/* Wii full diorama renderer (libogc GX).
 * Consumes the shared arena box list (diorama_arena.h) -- the same layout
 * the DS lite renderer and the host budget test use. Only rendering is
 * Wii-specific; game logic and the CNN are unchanged.
 *
 * Camera: locked straight-on front 2.5D, like the web diorama -- a
 * perspective camera slightly above and in front of the arena, looking
 * at its centre, smooth-following the player in X only. It never rotates.
 * Lighting: GX vertex lighting is approximated by baking a top shade
 * (brighter) and a front shade (darker) per face, plus a dark blob
 * shadow quad under the player. No shadow maps on Wii in this version.
 */
#pragma once
#include <gccore.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Call once at boot, after VIDEO is configured. rmode is the mode
 * VIDEO_GetPreferredMode returned. Returns 0 on success. */
int dio_wii_init(GXRModeObj *rmode);

/* Draw one frame of the world diorama. px/pz are the player position in
 * arena units (0..16, 0..12). sprite is the 24x24 grayscale player sprite
 * (0 = transparent), drawn as a billboard facing the camera. */
void dio_wii_draw_world(float px, float pz, const unsigned char *sprite);

/* Draw a full-screen coloured quad (title / select / gallery backdrops). */
void dio_wii_clear_colour(unsigned char r, unsigned char g, unsigned char b);

/* Draw a sprite as a screen-space billboard centred at (cx, cy) in
 * arena-world coordinates projected by the current camera -- used for
 * the dream screen (large, centred) and slot reels. scale is in arena
 * units (player = 1.5, dream = 6.0, slot reel = 3.0). */
void dio_wii_draw_billboard(float cx, float cy, float cz, float scale,
                            const unsigned char *sprite);

/* Draw a flat coloured quad in the billboard plane (selection highlight,
 * blob shadows, UI backdrops). */
void dio_wii_draw_quad(float x0, float y0, float x1, float y1, float z,
                       unsigned char r, unsigned char g, unsigned char b);

/* Swap buffers / present the frame. Call once per frame, after drawing. */
void dio_wii_present(void);

#ifdef __cplusplus
}
#endif
