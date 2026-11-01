/* Dream Core world renderer for DreamlingsWii (Phase 1).
 *
 * Draws the real Dream Core map (dreamcore_map.h) as a GX diorama,
 * replacing dio_build_wii_boxes / dio_wii_draw_world's 16x12 test
 * arena. dc_draw_world has the same signature as dio_wii_draw_world
 * so the main.c world screen can swap to it with one line (plus the
 * bounds/collision changes listed in REPORT.md -- those are NOT done
 * here).
 *
 * Conventions kept verbatim from source/diorama.c:
 *   - No textures: flat colours, top face full colour, front face
 *     (max Z, facing the camera) at 3/4 colour.
 *   - Camera locked straight-on front 2.5D, smooth-follow 0.15,
 *     eye at (tx+1.5, 4.2, tz+9.0), look-at (tx, 0.5, tz),
 *     45-degree perspective, near 0.1, far 100. Never rotates.
 *   - Player: black blob shadow quad at y=0.36, then the 24x24
 *     grayscale sprite billboard at height 1.1, scale 1.5.
 *   - Assumes dio_wii_init has run (GX vertex format, TEV, Z mode).
 *
 * One deliberate difference from DioBox: this renderer uses its own
 * DcBox with a y0 (base height) field. DioBox forces every box to sit
 * on Y=0, which cannot express the web's trees (trunk 0..1.8 with
 * canopy layers floating at 1.65/2.2/2.65) or the generator's stacked
 * trim/roof/peak. Growing DioBox would touch the shared header the
 * DS lite renderer also uses, so this file stays self-contained.
 *
 * Box budget: 1368 boxes (see dc_build_boxes) x top+front (4 tris)
 * = 5472 tris, plus the player billboard (<=1152) = <=6624 tris
 * per world frame. Wii GX handles this trivially; the DS 2048-tri
 * limit does not apply here.
 */
#pragma once

#include <gccore.h>

#ifdef __cplusplus
extern "C" {
#endif

/* A diorama box. Unlike DioBox, y0 lets boxes float above the ground
 * (canopy layers, generator trim/roof/peak, machine marquee). */
typedef struct {
    float x, y0, z;     /* centre x/z, base height y0, world units */
    float w, h, d;      /* size: width, height (in Y), depth */
    unsigned char r, g, b;
} DcBox;

/* Static box buffer size for dc_draw_world. 1368 boxes are emitted;
 * 2048 leaves headroom for later areas' props. */
#define DC_MAX_BOXES 2048

/* Fill out[] with the Dream Core box list (up to max_boxes).
 * Returns the number of boxes written (1368 if max_boxes allows). */
int dc_build_boxes(DcBox *out, int max_boxes);

/* Draw one frame of the Dream Core world. px/pz are the player
 * position in Wii world units (tile + 0.5): X 0.5..39.5, Z 0.5..29.5.
 * sprite is the 24x24 grayscale player sprite (0 = transparent). */
void dc_draw_world(float px, float pz, const unsigned char *sprite);

/* Give the renderer the video mode for the camera aspect ratio.
 * Call once after dio_wii_init (the integration step adds one line
 * in main.c). Until then a 4:3 fallback is used. */
void dc_wii_set_mode(GXRModeObj *mode);

#ifdef __cplusplus
}
#endif
