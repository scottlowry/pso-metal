/* pso-metal P2-Z — Metal backend for the display-list slice.
 * Window/layer/readback/event-pump = W's proven P2-W pattern (gx_metal.m)
 * reproduced verbatim in shape; statics are private to W's closed file so
 * extension-by-copy is the only non-editing reuse available (deviation
 * documented in docs/p2-gx-dl1.md). Delta vs W: a real render pipeline —
 * vertex+fragment compiled at runtime from an embedded source string, an
 * RGBA8 texture, and a clear+draw pass replacing clear-only.
 */
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
/* No gx_dl.h include here on purpose: era types.h wants the waiver flag and
 * collides with objc BOOL; the gxh_dl_* prototypes are consumed by the C
 * side from gx_dl.h, and defined below with matching plain-C signatures. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static id<MTLDevice>           g_device;
static id<MTLCommandQueue>     g_queue;
static NSWindow               *g_window;
static CAMetalLayer           *g_layer;
static id<MTLCommandBuffer>    g_last_cb;
static id<CAMetalDrawable>     g_last_drawable;
static id<MTLRenderPipelineState> g_pso;
static id<MTLTexture>          g_tex;
static id<MTLSamplerState>     g_smp;
static unsigned                g_width, g_height;

/* 8 floats: ndc.xy, rgba, uv — matches the GP dispatch output layout. */
static const char *g_src =
"using namespace metal;\n"
/* VS *input* struct: raw fetch, fields matched positionally to the
 * MTLVertexDescriptor attributes (a [[position]] field is invalid as VS
 * input — that rejected the pre-storm variant). */
"struct VRaw { float2 pos [[attribute(0)]]; float4 col [[attribute(1)]]; float2 uv [[attribute(2)]]; };\n"
"struct VIn {\n"
"    float4 position [[position]];\n"
"    float4 col;\n"
"    float2 uv;\n"
"};\n"
"vertex VIn vmain(VRaw in[[stage_in]]) {\n"
"    VIn o; o.position = float4(in.pos, 0.0, 1.0);\n"
"    o.col = in.col; o.uv = in.uv; return o;\n"
"}\n"
/* TEV stage-0 GX_MODULATE (ScreenPrimitiveDraw.cpp:124): color0 x texture.
 * Exact-texel read() fetch: the gate demands deterministic texel hits,
 * and read() removes sampler state from the failure surface. */
"fragment float4 fmain(VIn in[[stage_in]],\n"
"    texture2d<float> tex[[texture(0)]]) {\n"
"    uint cx = min(uint(in.uv.x * 4.0), 3u);\n"
"    uint cy = min(uint(in.uv.y * 4.0), 3u);\n"
"    return tex.read(cx, cy) * in.col;\n"
"}\n";

int gxh_dl_open(unsigned width, unsigned height) {
    @autoreleasepool {
        g_device = MTLCreateSystemDefaultDevice();
        if (!g_device) { fprintf(stderr, "gxh_dl: no Metal device\n"); return 2; }
        g_queue = [g_device newCommandQueue];
        if (!g_queue) { fprintf(stderr, "gxh_dl: no queue\n"); return 2; }

        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
        g_width = width; g_height = height;
        NSRect frame = NSMakeRect(100, 100, width, height);
        g_window = [[NSWindow alloc]
            initWithContentRect:frame
                      styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable)
                        backing:NSBackingStoreBuffered defer:NO];
        g_window.title = @"pso-metal P2-Z display-list quad";
        [g_window setReleasedWhenClosed:NO];

        g_layer = [CAMetalLayer layer];
        g_layer.device = g_device;
        g_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        g_layer.framebufferOnly = NO;         /* drawable getBytes-readable */
        g_layer.contentsScale = 1.0;
        g_layer.drawableSize = CGSizeMake(width, height);
        g_window.contentView.layer = g_layer;
        g_window.contentView.wantsLayer = YES;
        [g_window makeKeyAndOrderFront:nil];

        NSError *err = nil;
        id<MTLLibrary> lib = [g_device newLibraryWithSource:@(g_src)
                                                    options:nil error:&err];
        if (!lib) { fprintf(stderr, "gxh_dl: shader compile: %s\n",
                            err.localizedDescription.UTF8String); return 3; }
        MTLRenderPipelineDescriptor *pd = [MTLRenderPipelineDescriptor new];
        pd.vertexFunction   = [lib newFunctionWithName:@"vmain"];
        pd.fragmentFunction = [lib newFunctionWithName:@"fmain"];
        pd.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        MTLVertexDescriptor *vd = [MTLVertexDescriptor vertexDescriptor];
        vd.attributes[0].format = MTLVertexFormatFloat2;   /* ndc pos  */
        vd.attributes[0].offset = 0;  vd.attributes[0].bufferIndex = 0;
        vd.attributes[1].format = MTLVertexFormatFloat4;   /* color0   */
        vd.attributes[1].offset = 8;  vd.attributes[1].bufferIndex = 0;
        vd.attributes[2].format = MTLVertexFormatFloat2;   /* texcoord0*/
        vd.attributes[2].offset = 24; vd.attributes[2].bufferIndex = 0;
        vd.layouts[0].stride = 32;
        pd.vertexDescriptor = vd;
        g_pso = [g_device newRenderPipelineStateWithDescriptor:pd error:&err];
        if (!g_pso) { fprintf(stderr, "gxh_dl: pipeline: %s\n",
                              err.localizedDescription.UTF8String); return 3; }
        MTLSamplerDescriptor *sd = [MTLSamplerDescriptor new];
        sd.minFilter = sd.magFilter = MTLSamplerMinMagFilterNearest; /* exact-texel gate */
        sd.sAddressMode = sd.tAddressMode = MTLSamplerAddressModeClampToEdge;
        g_smp = [g_device newSamplerStateWithDescriptor:sd];
        printf("gxh_dl: window+pipeline open '%s' %ux%u, device=%s\n",
               g_window.title.UTF8String, width, height, g_device.name.UTF8String);
        return 0;
    }
}

void gxh_dl_set_texture(unsigned w, unsigned h, const unsigned char *rgba8) {
    @autoreleasepool {
        MTLTextureDescriptor *td = [MTLTextureDescriptor
            texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                         width:w height:h mipmapped:NO];
        td.usage = MTLTextureUsageShaderRead;
        g_tex = [g_device newTextureWithDescriptor:td];
        [g_tex replaceRegion:MTLRegionMake3D(0, 0, 0, w, h, 1)
                 mipmapLevel:0 withBytes:rgba8 bytesPerRow:w * 4];
        if (getenv("GX_DL_TEXP")) {   /* upload roundtrip verify */
            unsigned char rb[64];
            [g_tex getBytes:rb bytesPerRow:w * 4
                 fromRegion:MTLRegionMake2D(0, 0, w, h) mipmapLevel:0];
            printf("g_tex %ux%u fmt=%lu full%u:", w, h, (unsigned long)g_tex.pixelFormat, w*h*4u);
            for (unsigned i = 0; i < w*h*4u; i++) printf(" %02x", rb[i]);
            printf("\n");
        }
    }
}

int gxh_dl_draw(const float *vc, unsigned nverts,
                double r, double g, double b, double a) {
    @autoreleasepool {
        if (!g_layer || !g_pso || !g_tex) return 3;
        id<CAMetalDrawable> d = [g_layer nextDrawable];
        if (!d) { fprintf(stderr, "gxh_dl: nextDrawable nil\n"); return 5; }
        MTLRenderPassDescriptor *rpd = [MTLRenderPassDescriptor new];
        rpd.colorAttachments[0].texture     = d.texture;
        rpd.colorAttachments[0].loadAction  = MTLLoadActionClear;
        rpd.colorAttachments[0].storeAction = MTLStoreActionStore;
        rpd.colorAttachments[0].clearColor  = MTLClearColorMake(r, g, b, a);

        id<MTLCommandBuffer> cb = [g_queue commandBuffer];
        id<MTLRenderCommandEncoder> enc = [cb renderCommandEncoderWithDescriptor:rpd];
        if (!enc) return 3;
        [enc setRenderPipelineState:g_pso];
        [enc setFragmentTexture:g_tex atIndex:0];
        [enc setFragmentSamplerState:g_smp atIndex:0];
        if (nverts) {
            id<MTLBuffer> vb = [g_device newBufferWithBytes:vc
                length:nverts * 8 * sizeof(float)
               options:MTLResourceStorageModeShared];
            [enc setVertexBuffer:vb offset:0 atIndex:0];
            [enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0
                    vertexCount:nverts];
        }
        [enc endEncoding];
        [d present];
        [cb commit];
        g_last_cb = cb; g_last_drawable = d;
        return 0;
    }
}

/* W's ±2/255 tolerance corner check, generalized to N sample points. */
int gxh_dl_verify(const unsigned *xs, const unsigned *ys,
                  const unsigned *want_rgba, int npoints, int frame_no) {
    @autoreleasepool {
        if (!g_last_cb || !g_last_drawable) return 6;
        [g_last_cb waitUntilCompleted];
        if (g_last_cb.error) {
            fprintf(stderr, "gxh_dl f%d: cb error: %s\n", frame_no,
                    g_last_cb.error.localizedDescription.UTF8String);
            return 3;
        }
        id<MTLTexture> tex = g_last_drawable.texture;
        NSUInteger bpr = tex.width * 4;
        unsigned char *px = (unsigned char *)malloc(bpr * tex.height);
        [tex getBytes:px bytesPerRow:bpr
           fromRegion:MTLRegionMake2D(0, 0, tex.width, tex.height) mipmapLevel:0];
        int bad = 0;
        for (int i = 0; i < npoints; i++) {
            const unsigned char *p = px + (size_t)ys[i] * bpr + (size_t)xs[i] * 4;
            /* drawable BGRA vs wanted RGBA */
            if (abs((int)p[2] - (int)(want_rgba[i] >> 24 & 0xFF)) > 2 ||
                abs((int)p[1] - (int)(want_rgba[i] >> 16 & 0xFF)) > 2 ||
                abs((int)p[0] - (int)(want_rgba[i] >>  8 & 0xFF)) > 2) {
                fprintf(stderr, "gxh_dl f%d: pt(%u,%u) got BGRA(%u,%u,%u,%u) want RGBA(0x%08x)\n",
                        frame_no, xs[i], ys[i], p[0], p[1], p[2], p[3], want_rgba[i]);
                bad |= 1 << i;
            }
        }
        free(px);
        return bad;
    }
}

void gxh_dl_flush(void) {
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

/* debug: dump the last drawable as a PPM (8x nearest per output pixel). */
void gxh_dl_dump(const char *path) {
    @autoreleasepool {
        if (!g_last_cb || !g_last_drawable) return;
        [g_last_cb waitUntilCompleted];
        id<MTLTexture> tex = g_last_drawable.texture;
        NSUInteger bpr = tex.width * 4;
        unsigned char *px = (unsigned char *)malloc(bpr * tex.height);
        [tex getBytes:px bytesPerRow:bpr
           fromRegion:MTLRegionMake2D(0, 0, tex.width, tex.height) mipmapLevel:0];
        FILE *fp = fopen(path, "wb");
        fprintf(fp, "P6\n%lu %lu\n255\n", (unsigned long)tex.width, (unsigned long)tex.height);
        unsigned char *row = (unsigned char *)malloc(tex.width * 3);
        for (NSUInteger y = 0; y < tex.height; y++) {
            for (NSUInteger x = 0; x < tex.width; x++) {
                row[x*3+0] = px[y*bpr + x*4 + 2];
                row[x*3+1] = px[y*bpr + x*4 + 1];
                row[x*3+2] = px[y*bpr + x*4 + 0];
            }
            fwrite(row, 3, tex.width, fp);
        }
        free(row); free(px); fclose(fp);
    }
}

void gxh_dl_close(void) {
    @autoreleasepool {
        if (g_window) { [g_window close]; g_window = nil; }
        g_layer = nil; g_last_cb = nil; g_last_drawable = nil;
        g_pso = nil; g_tex = nil; g_smp = nil;
    }
}
