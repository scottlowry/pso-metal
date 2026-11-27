/* pso-metal P2-W — the Metal window backend called by gx_shims.c.
 * Exit-code convention inherited from spike/metal_clear.m: 2 no device,
 * 3 encode/window failure, 4 readback mismatch. */
#ifndef PSO_GX_METAL_H
#define PSO_GX_METAL_H

/* NSWindow + backing CAMetalLayer (640x480 logical), device/queue from the
 * proven spike path. 0 ok, 2 no Metal device, 3 window/layer failure. */
int gxh_open(unsigned width, unsigned height);

/* Acquire next drawable, encode a clear-only render pass (loadAction=Clear,
 * colors rgba 0..1), commit + present. 0 ok, 3 failure, 5 no drawable. */
int gxh_present_clear(double r, double g, double b, double a);

/* waitUntilCompleted on the last command buffer, then getBytes the drawable
 * texture and assert every sampled corner matches the expected color within
 * ±2/255. 0 match, 4 mismatch, 6 no outstanding present. */
int gxh_wait_verify(double r, double g, double b, int frame_no);

/* Pump window server events + flush CoreAnimation so the window actually
 * composites (no NSApp run loop in the driver). */
void gxh_flush(void);

/* Close window, release layer. Idempotent. */
void gxh_close(void);

#endif /* PSO_GX_METAL_H */
