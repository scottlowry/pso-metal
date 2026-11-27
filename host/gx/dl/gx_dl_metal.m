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
static const char *g_vs =
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
"}\n";
/* TEV stage-0 GX_MODULATE (ScreenPrimitiveDraw.cpp:124): color0 x texture.
 * Exact-texel read() fetch: the gate demands deterministic texel hits,
 * and read() removes sampler state from the failure surface.
 * P2-AQ FIX (root cause of Z's 'cy>0 reads zeros', docs §2): the SCALAR
 * read(cx, cy) overload of texture2d<float> is MISCOMPILED by this M3 Ultra
 * runtime MSL compiler — literal read(2u,2u) on a texel-verified-correct 4x4
 * returns (0,0,0,0); dynamic coords return the diagonal at cy=0 and zeros
 * below. The vector-coordinate overload read(uint2(cx, cy)) lowers correctly
 * (16/16 texels exact; sample() is likewise exact). Keep the uint2 form;
 * do not "simplify" back to scalar coords.
 * P2-AQ debug: GX_DL_FSHOP swaps the fragment body:
 *  1 = emit uv, 2 = emit col, 3 = sample(sampler), 4 = scalar read no-col,
 *  5 = literal scalar read(2,2), 6 = uint2 read (shipped semantics). */
static const char *g_fs[] = {
"fragment float4 fmain(VIn in[[stage_in]],\n"
"    texture2d<float> tex[[texture(0)]]) {\n"
"    uint cx = min(uint(in.uv.x * 4.0), 3u);\n"
"    uint cy = min(uint(in.uv.y * 4.0), 3u);\n"
"    return tex.read(uint2(cx, cy)) * in.col;\n"
"}\n",
"fragment float4 fmain(VIn in[[stage_in]],\n"
"    texture2d<float> tex[[texture(0)]]) {\n"
"    return float4(fract(in.uv * 4.0), floor(in.uv.y * 4.0) / 4.0, 1.0);\n"
"}\n",
"fragment float4 fmain(VIn in[[stage_in]],\n"
"    texture2d<float> tex[[texture(0)]]) {\n"
"    return in.col;\n"
"}\n",
"fragment float4 fmain(VIn in[[stage_in]],\n"
"    texture2d<float> tex[[texture(0)]],\n"
"    sampler smp[[sampler(0)]]) {\n"
"    return tex.sample(smp, in.uv) * in.col;\n"
"}\n",
"fragment float4 fmain(VIn in[[stage_in]],\n"
"    texture2d<float> tex[[texture(0)]]) {\n"
"    uint cx = min(uint(in.uv.x * 4.0), 3u);\n"
"    uint cy = min(uint(in.uv.y * 4.0), 3u);\n"
"    return tex.read(cx, cy);\n"
"}\n",
"fragment float4 fmain(VIn in[[stage_in]],\n"
"    texture2d<float> tex[[texture(0)]]) {\n"
"    return tex.read(2u, 2u) * in.col;\n"
"}\n",
"fragment float4 fmain(VIn in[[stage_in]],\n"
"    texture2d<float> tex[[texture(0)]]) {\n"
"    uint cx = min(uint(in.uv.x * 4.0), 3u);\n"
"    uint cy = min(uint(in.uv.y * 4.0), 3u);\n"
"    return tex.read(uint2(cx, cy)) * in.col;\n"
"}\n"};

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
        int fshop = getenv("GX_DL_FSHOP") ? atoi(getenv("GX_DL_FSHOP")) : 0;
        if (fshop < 0 || fshop > 6) fshop = 0;
        NSString *src;
        const char *texn_env = getenv("GX_DL_TEXN");   /* P2-BA V3: N x N texel grid */
        if (texn_env && fshop == 0) {
            /* P2-AQ R2/§2.5 binding: vector read(uint2(cx, cy)) ONLY — the
             * scalar overload is miscompiled on this machine (docs/p2-gx-dl2.md
             * §2). This runtime-formatted body keeps the shipped 4x4 path
             * byte-identical (TEXN unset == today) and generalizes the
             * floor/last-texel constants for the 32x32 PAL8 logo texture. */
            int tn = atoi(texn_env);
            if (tn < 1 || tn > 1024) tn = 4;
            static char body[512];
            snprintf(body, sizeof body,
                "fragment float4 fmain(VIn in[[stage_in]],\n"
                "    texture2d<float> tex[[texture(0)]]) {\n"
                "    uint cx = min(uint(in.uv.x * %d.0), %du);\n"
                "    uint cy = min(uint(in.uv.y * %d.0), %du);\n"
                "    return tex.read(uint2(cx, cy)) * in.col;\n"
                "}\n", tn, tn - 1, tn, tn - 1);
            printf("gxh_dl: TEXN=%d uint2-fragment variant: '%.*s%s",
                   tn, 60, body, "...'\n");
            src = [NSString stringWithFormat:@"%s%s", g_vs, body];
        } else {
            src = [NSString stringWithFormat:@"%s%s", g_vs, g_fs[fshop]];
        }
        id<MTLLibrary> lib = [g_device newLibraryWithSource:src
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

/* read back the last presented drawable at N points -> RGBA8 packed u32.
 * Returns 0 ok, 6 no-frame. (P2-AQ: raw sampler for the texel diagnostic.) */
int gxh_dl_sample(const unsigned *xs, const unsigned *ys, unsigned *out_rgba, int n) {
    @autoreleasepool {
        if (!g_last_cb || !g_last_drawable) return 6;
        [g_last_cb waitUntilCompleted];
        id<MTLTexture> tex = g_last_drawable.texture;
        NSUInteger bpr = tex.width * 4;
        unsigned char *px = (unsigned char *)malloc(bpr * tex.height);
        [tex getBytes:px bytesPerRow:bpr
           fromRegion:MTLRegionMake2D(0, 0, tex.width, tex.height) mipmapLevel:0];
        for (int i = 0; i < n; i++) {
            const unsigned char *p = px + (size_t)ys[i] * bpr + (size_t)xs[i] * 4;
            out_rgba[i] = ((unsigned)p[2] << 24) | ((unsigned)p[1] << 16) |
                          ((unsigned)p[0] << 8)  |  p[3];      /* BGRA -> RGBA */
        }
        free(px);
        return 0;
    }
}

/* P2-AQ diagnostic: CPU-side getBytes roundtrip of the CURRENT gpu texture,
 * printed as a 4-value-per-row table so it can be diffed against the
 * GPU-sampled texel map without env-gating. */
void gxh_dl_tex_roundtrip(unsigned w, unsigned h) {
    @autoreleasepool {
        if (!g_tex) { printf("tex_rt: no texture\n"); return; }
        unsigned char *rb = (unsigned char *)malloc((size_t)w * h * 4);
        [g_tex getBytes:rb bytesPerRow:w * 4
             fromRegion:MTLRegionMake2D(0, 0, w, h) mipmapLevel:0];
        printf("tex_rt GPU-texture-memory %ux%u (row-major):\n", w, h);
        for (unsigned y = 0; y < h; y++) {
            printf("  row%u:", y);
            for (unsigned x = 0; x < w; x++)
                printf(" [%u,%u]=(%u,%u,%u,%u)", x, y,
                       rb[(y*w+x)*4], rb[(y*w+x)*4+1], rb[(y*w+x)*4+2], rb[(y*w+x)*4+3]);
            printf("\n");
        }
        printf("  tex dims: %lu x %lu fmt=%lu levels=%lu usage=%lu\n",
               (unsigned long)g_tex.width, (unsigned long)g_tex.height,
               (unsigned long)g_tex.pixelFormat, (unsigned long)g_tex.mipmapLevelCount,
               (unsigned long)g_tex.usage);
        free(rb);
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

/* P2-SEQ: PNG artifact dump of the last drawable (BGRA -> RGB8 rep -> PNG
 * data through AppKit; zero new dependencies, zero shader/pipeline change —
 * the shipped gate paths are untouched). Returns 0 on a non-empty write. */
int gxh_dl_dump_png(const char *path) {
    @autoreleasepool {
        if (!g_last_cb || !g_last_drawable) return 6;
        [g_last_cb waitUntilCompleted];
        id<MTLTexture> tex = g_last_drawable.texture;
        NSUInteger bpr = tex.width * 4;
        unsigned char *px = (unsigned char *)malloc(bpr * tex.height);
        [tex getBytes:px bytesPerRow:bpr
           fromRegion:MTLRegionMake2D(0, 0, tex.width, tex.height) mipmapLevel:0];
        NSBitmapImageRep *rep = [[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes:NULL pixelsWide:(NSInteger)tex.width
            pixelsHigh:(NSInteger)tex.height bitsPerSample:8 samplesPerPixel:3
            hasAlpha:NO isPlanar:NO colorSpaceName:NSDeviceRGBColorSpace
            bytesPerRow:(NSInteger)(tex.width * 3) bitsPerPixel:24];
        unsigned char *dp = [rep bitmapData];
        for (NSUInteger y = 0; y < tex.height; y++)
            for (NSUInteger x = 0; x < tex.width; x++) {
                dp[y*tex.width*3 + x*3 + 0] = px[y*bpr + x*4 + 2];
                dp[y*tex.width*3 + x*3 + 1] = px[y*bpr + x*4 + 1];
                dp[y*tex.width*3 + x*3 + 2] = px[y*bpr + x*4 + 0];
            }
        NSData *d = [rep representationUsingType:NSBitmapImageFileTypePNG
                                      properties:@{}];
        free(px);
        if (!d.length) return 3;
        BOOL ok = [d writeToFile:[NSString stringWithUTF8String:path] atomically:YES];
        return ok ? 0 : 3;
    }
}

void gxh_dl_close(void) {
    @autoreleasepool {
        if (g_window) { [g_window close]; g_window = nil; }
        g_layer = nil; g_last_cb = nil; g_last_drawable = nil;
        g_pso = nil; g_tex = nil; g_smp = nil;
    }
}
