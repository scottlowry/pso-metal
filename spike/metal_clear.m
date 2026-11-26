// P1-K Metal offscreen spike: prove the host can acquire a Metal device and
// round-trip a 640x480 BGRA8Unorm render target through a clear + readback,
// headless (no window, no CAMetalLayer, no shaders). This is the minimum
// substrate the port's GX->Metal layer needs before any draw semantics:
// PSO boot renders 640x480 (GXNtsc480IntDf, docs/boot-spine.md S6/S7/S9),
// and the efB-copy-into-framebuffer path becomes a clear/copy on this texture.
//
// Exit codes (CI): 0 pass, 2 no Metal device/queue, 3 texture/encode failure,
// 4 readback mismatch.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

static const NSUInteger kWidth  = 640;
static const NSUInteger kHeight = 480;

// Known non-trivial clear color (RGBA floats).
static const double kR = 0.25, kG = 0.5, kB = 0.75, kA = 1.0;
// 8-bit quantization/rounding headroom (0.5*255=127.5 rounds differently per impl).
static const double kTolerance = 2.0 / 255.0;

static void fail(const char *msg) { fprintf(stderr, "FAIL: %s\n", msg); }

int main(void) {
    @autoreleasepool {
        // 1. Device
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (!device) {
            fprintf(stderr, "FAIL: MTLCreateSystemDefaultDevice() returned nil "
                            "— no Metal device available headless\n");
            return 2;
        }
        printf("Metal device: %s (lowPower=%d)\n",
               device.name.UTF8String, (int)device.isLowPower);

        id<MTLCommandQueue> queue = [device newCommandQueue];
        if (!queue) { fail("newCommandQueue returned nil"); return 2; }

        // 2. Render target: 640x480 BGRA8Unorm, host-readable
        MTLTextureDescriptor *td = [MTLTextureDescriptor
            texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                         width:kWidth height:kHeight
                                     mipmapped:NO];
        // MTLTextureUsageHostReadback is iOS-only; on macOS any texture is
        // -[MTLTexture getBytes:] readable.
        td.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
        id<MTLTexture> fb = [device newTextureWithDescriptor:td];
        if (!fb) { fail("newTextureWithDescriptor failed"); return 3; }
        printf("Render target: %lu x %lu BGRA8Unorm\n",
               (unsigned long)fb.width, (unsigned long)fb.height);

        // 3. Encode a render pass whose loadAction is the clear.
        //    No pipeline, no draws: the clear alone is valid.
        MTLRenderPassDescriptor *rpd = [MTLRenderPassDescriptor new];
        rpd.colorAttachments[0].texture     = fb;
        rpd.colorAttachments[0].loadAction  = MTLLoadActionClear;
        rpd.colorAttachments[0].storeAction = MTLStoreActionStore;
        rpd.colorAttachments[0].clearColor  = MTLClearColorMake(kR, kG, kB, kA);

        id<MTLCommandBuffer> cb = [queue commandBuffer];
        id<MTLRenderCommandEncoder> enc = [cb renderCommandEncoderWithDescriptor:rpd];
        if (!enc) { fail("renderCommandEncoderWithDescriptor returned nil"); return 3; }
        [enc endEncoding];
        [cb commit];
        [cb waitUntilCompleted];
        if (cb.error) {
            fprintf(stderr, "FAIL: command buffer error: %s\n",
                    cb.error.localizedDescription.UTF8String);
            return 3;
        }
        printf("Render pass (clear to RGBA %.2f,%.2f,%.2f,%.2f): OK\n",
               kR, kG, kB, kA);

        // 4. Read back into host memory.
        const NSUInteger bpr = kWidth * 4; // BGRA8 = 4 bytes/pixel, tightly packed
        unsigned char *pixels = (unsigned char *)malloc(bpr * kHeight);
        if (!pixels) { fail("malloc readback buffer"); return 3; }
        [fb getBytes:pixels bytesPerRow:bpr
          fromRegion:MTLRegionMake2D(0, 0, kWidth, kHeight)
             mipmapLevel:0];

        // 5. Assert sampled pixels + full-frame scan against the clear color.
        //    BGRA8Unorm byte order: [B, G, R, A].
        int rc = 0;
        const NSUInteger pts[5][2] = {{0,0}, {639,479}, {319,240}, {0,479}, {639,0}};
        for (int i = 0; i < 5; i++) {
            NSUInteger x = pts[i][0], y = pts[i][1];
            const unsigned char *px = pixels + y * bpr + x * 4;
            double b = px[0] / 255.0, g = px[1] / 255.0,
                   r = px[2] / 255.0, a = px[3] / 255.0;
            int ok = fabs(r - kR) <= kTolerance && fabs(g - kG) <= kTolerance &&
                     fabs(b - kB) <= kTolerance && fabs(a - kA) <= kTolerance;
            printf("pixel(%3lu,%3lu) = BGRA(%u,%u,%u,%u) -> rgba(%.4f,%.4f,%.4f,%.4f) %s\n",
                   (unsigned long)x, (unsigned long)y, px[0], px[1], px[2], px[3],
                   r, g, b, a, ok ? "MATCH" : "MISMATCH");
            if (!ok) rc = 4;
        }
        NSUInteger bad = 0;
        for (NSUInteger y = 0; y < kHeight && rc == 0; y++) {
            for (NSUInteger x = 0; x < kWidth; x++) {
                const unsigned char *px = pixels + y * bpr + x * 4;
                if (px[0] < 189 || px[0] > 193 || px[1] < 126 || px[1] > 130 ||
                    px[2] <  62 || px[2] >  66 || px[3] != 255) { bad++; }
            }
        }
        free(pixels);
        if (bad) {
            fprintf(stderr, "FAIL: %lu of %lu pixels outside clear-color tolerance\n",
                    (unsigned long)bad, (unsigned long)(kWidth * kHeight));
            rc = 4;
        } else if (rc == 0) {
            printf("Full-frame check: all %lu pixels within tolerance (±%.4f)\n",
                   (unsigned long)(kWidth * kHeight), kTolerance);
        }
        if (rc) return rc;
        printf("PASS: offscreen 640x480 clear + readback round-trip\n");
        return 0;
    }
}
