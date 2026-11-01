/* Wii full diorama renderer -- see diorama.h.
 * No textures: sprites are drawn pixel-by-pixel as coloured quads, so
 * there is no GX texture-tiling format to get wrong, and the pixel-art
 * silhouette is preserved exactly. Boxes are drawn as top + front
 * faces only (the camera never sees the back or bottom), which is why
 * the visible triangle count in the host test is 436, not 205*12.
 */
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "diorama.h"
#include "diorama_arena.h"

#define FIFO_SIZE (256 * 1024)

static unsigned char gp_fifo[FIFO_SIZE] __attribute__((aligned(32)));
static void *frameBuffer[2] = {NULL, NULL};
static GXRModeObj *rmode = NULL;
static int which_fb = 0;
static float cam_target_x = 8.0f, cam_target_z = 6.0f;

static void draw_quad(float x0, float y0, float z0,
                      float x1, float y1, float z1,
                      float x2, float y2, float z2,
                      float x3, float y3, float z3,
                      unsigned char r, unsigned char g, unsigned char b) {
    GX_Position3f32(x0, y0, z0); GX_Color3u8(r, g, b);
    GX_Position3f32(x1, y1, z1); GX_Color3u8(r, g, b);
    GX_Position3f32(x2, y2, z2); GX_Color3u8(r, g, b);
    GX_Position3f32(x3, y3, z3); GX_Color3u8(r, g, b);
}

static void draw_box_top_front(const DioBox *b) {
    float x0 = b->x - b->w / 2, x1 = b->x + b->w / 2;
    float z0 = b->z - b->d / 2, z1 = b->z + b->d / 2;
    float y0 = 0.0f, y1 = b->h;
    /* Base slab sits below Y=0: shift it down so its top is at Y=0. */
    if (b->kind == DIO_BASE) { y0 = -b->h; y1 = 0.0f; }
    unsigned char tr = b->r, tg = b->g, tb = b->b;
    unsigned char fr = (unsigned char)(b->r * 3 / 4);
    unsigned char fg = (unsigned char)(b->g * 3 / 4);
    unsigned char fb_ = (unsigned char)(b->b * 3 / 4);
    GX_Begin(GX_QUADS, GX_VTXFMT0, 8);
    /* Top face (brighter). Winding chosen so it faces up. */
    draw_quad(x0, y1, z0, x1, y1, z0, x1, y1, z1, x0, y1, z1, tr, tg, tb);
    /* Front face (darker): the face at max Z, facing the camera. */
    draw_quad(x0, y0, z1, x1, y0, z1, x1, y1, z1, x0, y1, z1, fr, fg, fb_);
    GX_End();
}

int dio_wii_init(GXRModeObj *mode) {
    rmode = mode;
    frameBuffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    frameBuffer[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(frameBuffer[which_fb]);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

    GX_Init(gp_fifo, FIFO_SIZE);
    GX_SetCopyClear((GXColor){10, 5, 18, 255}, GX_MAX_Z24);
    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    {
        f32 yscale = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
        u32 xfbHeight = GX_SetDispCopyYScale(yscale);
        GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
        GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
        GX_SetDispCopyDst(rmode->fbWidth, xfbHeight);
        GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
        GX_SetFieldMode(rmode->field_rendering,
                        (rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE);
    }
    GX_SetPixelFmt(rmode->aa ? GX_PF_RGB565_Z16 : GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    GX_SetCullMode(GX_CULL_NONE);
    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_TRUE);
    GX_CopyDisp(frameBuffer[which_fb], GX_TRUE);
    GX_SetDispCopyGamma(GX_GM_1_0);

    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGB8, 0);
    GX_SetNumChans(1);
    GX_SetNumTexGens(0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_InvalidateTexAll();
    return 0;
}

static void set_camera(float tx, float tz) {
    /* Smooth follow in X/Z, clamped so the camera never leaves the arena. */
    if (tx < 4.0f) tx = 4.0f; if (tx > 12.0f) tx = 12.0f;
    if (tz < 3.0f) tz = 3.0f; if (tz > 9.0f) tz = 9.0f;
    cam_target_x += (tx - cam_target_x) * 0.15f;
    cam_target_z += (tz - cam_target_z) * 0.15f;
    {
        Mtx44 proj;
        Mtx view;
        guPerspective(proj, 45, (f32)rmode->fbWidth / (f32)rmode->efbHeight, 0.1f, 100.0f);
        GX_LoadProjectionMtx(proj, GX_PERSPECTIVE);
        /* guLookAt(mt, cam, up, look) -- up is the third argument. */
        guLookAt(view,
                 (guVector){cam_target_x + 1.5f, 4.2f, cam_target_z + 9.0f},
                 (guVector){0.0f, 1.0f, 0.0f},
                 (guVector){cam_target_x, 0.5f, cam_target_z});
        GX_LoadPosMtxImm(view, GX_PNMTX0);
    }
}

void dio_wii_clear_colour(unsigned char r, unsigned char g, unsigned char b) {
    GX_SetCopyClear((GXColor){r, g, b, 255}, GX_MAX_Z24);
    set_camera(8.0f, 6.0f);
}

void dio_wii_draw_billboard(float cx, float cy, float cz, float scale,
                            const unsigned char *sprite) {
    int count = 0;
    int x, y;
    for (y = 0; y < 24; y++)
        for (x = 0; x < 24; x++)
            if (sprite[y * 24 + x]) count++;
    if (!count) return;
    GX_Begin(GX_QUADS, GX_VTXFMT0, count * 4);
    for (y = 0; y < 24; y++) {
        for (x = 0; x < 24; x++) {
            unsigned char v = sprite[y * 24 + x];
            if (!v) continue;
            float px0 = cx - scale / 2 + x * scale / 24.0f;
            float px1 = cx - scale / 2 + (x + 1) * scale / 24.0f;
            float py1 = cy + scale / 2 - y * scale / 24.0f;
            float py0 = cy + scale / 2 - (y + 1) * scale / 24.0f;
            draw_quad(px0, py0, cz, px1, py0, cz, px1, py1, cz, px0, py1, cz, v, v, v);
        }
    }
    GX_End();
}

void dio_wii_draw_quad(float x0, float y0, float x1, float y1, float z,
                       unsigned char r, unsigned char g, unsigned char b) {
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    draw_quad(x0, y0, z, x1, y0, z, x1, y1, z, x0, y1, z, r, g, b);
    GX_End();
}

void dio_wii_draw_world(float px, float pz, const unsigned char *sprite) {
    DioBox boxes[256];
    int n = dio_build_wii_boxes(boxes, 256);
    int i;
    set_camera(px, pz);
    for (i = 0; i < n; i++) draw_box_top_front(&boxes[i]);
    /* Blob shadow under the player, then the player billboard. */
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    draw_quad(px - 0.8f, 0.36f, pz - 0.8f, px + 0.8f, 0.36f, pz - 0.8f,
              px + 0.8f, 0.36f, pz + 0.8f, px - 0.8f, 0.36f, pz + 0.8f, 0, 0, 0);
    GX_End();
    dio_wii_draw_billboard(px, 1.1f, pz, 1.5f, sprite);
}

void dio_wii_present(void) {
    GX_DrawDone();
    GX_CopyDisp(frameBuffer[which_fb], GX_TRUE);
    VIDEO_SetNextFramebuffer(frameBuffer[which_fb]);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    which_fb ^= 1;
}
