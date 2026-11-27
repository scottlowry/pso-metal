/* pso-metal P2-W — real Metal window: NSWindow + CAMetalLayer clear+present
 * + drawable readback. Device/queue/render-pass/readback code paths are the
 * spike/metal_clear.m proven ones (same clear-encode + getBytes + ±2/255
 * tolerance); the delta here is the on-screen layer (framebufferOnly=NO so
 * CI can read back the DRAWABLE itself, not an offscreen twin) and a manual
 * event pump (the driver has no NSApp run loop).
 */
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include "gx_metal.h"

static id<MTLDevice>            g_device;
static id<MTLCommandQueue>      g_queue;
static NSWindow                *g_window;
static CAMetalLayer            *g_layer;
static id<MTLCommandBuffer>     g_last_cb;
static id<CAMetalDrawable>      g_last_drawable;
static unsigned                 g_width, g_height;

int gxh_open(unsigned width, unsigned height) {
    @autoreleasepool {
        g_device = MTLCreateSystemDefaultDevice();
        if (!g_device) {
            fprintf(stderr, "gxh: MTLCreateSystemDefaultDevice() nil — no Metal device\n");
            return 2;
        }
        g_queue = [g_device newCommandQueue];
        if (!g_queue) { fprintf(stderr, "gxh: newCommandQueue nil\n"); return 2; }

        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];

        g_width = width; g_height = height;
        NSRect frame = NSMakeRect(100, 100, width, height);
        g_window = [[NSWindow alloc]
            initWithContentRect:frame
                      styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable)
                        backing:NSBackingStoreBuffered defer:NO];
        g_window.title = @"pso-metal P2-W GX window";
        [g_window setReleasedWhenClosed:NO];

        g_layer = [CAMetalLayer layer];
        g_layer.device = g_device;
        g_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        g_layer.framebufferOnly = NO;          /* drawable must be getBytes-readable */
        g_layer.contentsScale = 1.0;
        g_layer.drawableSize = CGSizeMake(width, height);
        g_window.contentView.layer = g_layer; /* implicitly layer-backed */
        g_window.contentView.wantsLayer = YES;

        [g_window makeKeyAndOrderFront:nil];
        printf("gxh: window open '%s' %ux%u BGRA8Unorm layer-backed, device=%s\n",
               g_window.title.UTF8String, width, height, g_device.name.UTF8String);
        return 0;
    }
}

int gxh_present_clear(double r, double g, double b, double a) {
    @autoreleasepool {
        if (!g_layer) return 3;
        id<CAMetalDrawable> d = [g_layer nextDrawable];
        if (!d) { fprintf(stderr, "gxh: nextDrawable nil (drawable pool exhausted)\n"); return 5; }

        MTLRenderPassDescriptor *rpd = [MTLRenderPassDescriptor new];
        rpd.colorAttachments[0].texture     = d.texture;
        rpd.colorAttachments[0].loadAction  = MTLLoadActionClear;
        rpd.colorAttachments[0].storeAction = MTLStoreActionStore;
        rpd.colorAttachments[0].clearColor  = MTLClearColorMake(r, g, b, a);

        id<MTLCommandBuffer> cb = [g_queue commandBuffer];
        id<MTLRenderCommandEncoder> enc = [cb renderCommandEncoderWithDescriptor:rpd];
        if (!enc) { fprintf(stderr, "gxh: renderCommandEncoder nil\n"); return 3; }
        [enc endEncoding];
        [d present];
        [cb commit];
        g_last_cb = cb; g_last_drawable = d;
        return 0;
    }
}

/* BGRA8Unorm bytes [B,G,R,A]; ±2/255 headroom, same tolerance as the spike. */
static int corner_ok(const unsigned char *px, double r, double g, double b) {
    double B = px[0] / 255.0, G = px[1] / 255.0, R = px[2] / 255.0;
    return fabs(R - r) <= 2.0 / 255.0 && fabs(G - g) <= 2.0 / 255.0 &&
           fabs(B - b) <= 2.0 / 255.0;
}

int gxh_wait_verify(double r, double g, double b, int frame_no) {
    @autoreleasepool {
        if (!g_last_cb || !g_last_drawable) return 6;
        [g_last_cb waitUntilCompleted];
        if (g_last_cb.error) {
            fprintf(stderr, "gxh f%d: command buffer error: %s\n", frame_no,
                    g_last_cb.error.localizedDescription.UTF8String);
            return 3;
        }
        id<MTLTexture> tex = g_last_drawable.texture;
        NSUInteger bpr = tex.width * 4;
        unsigned char *px = (unsigned char *)malloc(bpr * tex.height);
        if (!px) return 3;
        [tex getBytes:px bytesPerRow:bpr
           fromRegion:MTLRegionMake2D(0, 0, tex.width, tex.height) mipmapLevel:0];
        const NSUInteger xs[4] = {0, tex.width - 1, 0, tex.width - 1};
        const NSUInteger ys[4] = {0, 0, tex.height - 1, tex.height - 1};
        int bad = 0;
        unsigned char sample[4] = {0};
        for (int i = 0; i < 4; i++) {
            const unsigned char *p = px + ys[i] * bpr + xs[i] * 4;
            if (i == 0) { sample[0]=p[0]; sample[1]=p[1]; sample[2]=p[2]; sample[3]=p[3]; }
            if (!corner_ok(p, r, g, b)) bad++;
        }
        free(px);
        if (bad) {
            fprintf(stderr, "gxh f%d: %d/4 corners MISMATCH (TL=BGRA %u,%u,%u,%u, wanted rgb %.3f,%.3f,%.3f)\n",
                    frame_no, bad, sample[0], sample[1], sample[2], sample[3], r, g, b);
            return 4;
        }
        printf("gxh f%02d: drawable %lux%lu TL-corner BGRA(%u,%u,%u,%u) MATCH rgb(%.3f,%.3f,%.3f) [4/4 corners]\n",
               frame_no, (unsigned long)tex.width, (unsigned long)tex.height,
               sample[0], sample[1], sample[2], sample[3], r, g, b);
        return 0;
    }
}

void gxh_flush(void) {
    @autoreleasepool {
        [CATransaction flush];
        NSEvent *ev;
        while ((ev = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:nil inMode:NSDefaultRunLoopMode
                                        dequeue:YES]) != nil) {
            [NSApp sendEvent:ev];
        }
    }
}

void gxh_close(void) {
    @autoreleasepool {
        if (g_window) { [g_window close]; g_window = nil; }
        g_layer = nil; g_last_cb = nil; g_last_drawable = nil;
    }
}
