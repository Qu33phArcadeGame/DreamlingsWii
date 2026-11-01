/* Deepdreamlings Wii -- live tiny-CNN port, full GX diorama world.
 * Port of the DS BlocksDS game (source/main.c in ds-blocksds): same
 * CNN, same six screens, same slot/fusion rule. The DS 256x192 bitmap
 * world is replaced by the GX diorama (diorama.c); the dream itself is
 * still the 2D CNN morph, shown as a large billboard.
 *
 * Controls (Wiimote, like QU33PH Wii): D-pad or Nunchuk stick to walk /
 * select, A confirm / dream / spin / stop reel, B back / run, + pause
 * (no-op in this version), - gallery, HOME back to the Homebrew Channel.
 * Classic / GameCube pads: D-pad + A/B work through WPAD/PAD polling
 * in the same buttonsDown/held helpers below (GameCube via PAD_ScanPads).
 *
 * Save: sd:/apps/dreamlings/dreamlings.sav (fat). Never writes to NAND.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include "cnn.h"
#include "dreamlings.h"
#include "diorama.h"
#include "dreamcore_map.h"
#include "dreamcore_world.h"

enum { ST_TITLE, ST_SELECT, ST_WORLD, ST_DREAM, ST_SLOT, ST_GALLERY };

static int state = ST_TITLE;
static int selected = 0;
static int collected[11];
static int ncollected = 0;
static unsigned char dream_img[576];
static unsigned char fused[11][576];
static int nfused = 0;
static float px_x = 4.5f, pz_z = 25.5f;
static int frame = 0;
static u32 rng_state = 12345;

static u32 xrnd(void) {
    rng_state = rng_state * 1103515245 + 12345;
    return (rng_state >> 16) & 0x7FFF;
}

/* Dream Core triggers (Phase 1): generator building -> dream screen,
 * water-machine tiles -> slot screen. Warp doors are inert for now
 * (room1/hall1 do not exist on Wii yet); DC_ENC zone encounters
 * arrive in the encounter phase. */
static const float gen_rect[4] = {33.0f, 27.0f, 36.0f, 29.0f};

static int near_machine(float x, float z) {
    int i;
    for (i = 0; i < 3; i++) {
        float mx = DC_MACHINES[i].x + 0.5f, mz = DC_MACHINES[i].y + 0.5f;
        if (x + 0.75f > mx - 0.5f && x - 0.75f < mx + 0.5f &&
            z + 0.75f > mz - 0.5f && z - 0.75f < mz + 0.5f) return 1;
    }
    return 0;
}

static int near_rect(float x, float z, const float *r) {
    return x + 0.75f > r[0] && x - 0.75f < r[2] &&
           z + 0.75f > r[1] && z - 0.75f < r[3];
}

static void save_game(void) {
    FILE *f = fopen("sd:/apps/dreamlings/dreamlings.sav", "wb");
    if (!f) return;
    fwrite("DLSV", 1, 4, f);
    { unsigned char v = 1; fwrite(&v, 1, 1, f); }
    fwrite(collected, 1, 11, f);
    { unsigned char v = (unsigned char)nfused; fwrite(&v, 1, 1, f); }
    fwrite(fused, 1, 11 * 576, f);
    { unsigned char v = (unsigned char)selected; fwrite(&v, 1, 1, f); }
    fclose(f);
}

static void load_game(void) {
    FILE *f = fopen("sd:/apps/dreamlings/dreamlings.sav", "rb");
    unsigned char hdr[4], v;
    int i;
    if (!f) return;
    if (fread(hdr, 1, 4, f) != 4 || memcmp(hdr, "DLSV", 4) != 0) { fclose(f); return; }
    if (fread(&v, 1, 1, f) != 1 || v != 1) { fclose(f); return; }
    if (fread(collected, 1, 11, f) != 11) { fclose(f); return; }
    if (fread(&v, 1, 1, f) != 1) { fclose(f); return; }
    nfused = v; if (nfused > 11) nfused = 11;
    if (fread(fused, 1, 11 * 576, f) != 11 * 576) { fclose(f); return; }
    if (fread(&v, 1, 1, f) == 1 && v < 11) selected = v;
    ncollected = 0;
    for (i = 0; i < 11; i++) if (collected[i]) ncollected++;
    fclose(f);
}

static int buttons_down(int chan, u32 mask) {
    return (WPAD_ButtonsDown(chan) & mask) != 0;
}
static int buttons_held(int chan, u32 mask) {
    return (WPAD_ButtonsHeld(chan) & mask) != 0;
}

int main(int argc, char **argv) {
    GXRModeObj *rmode;
    int slot_reels[3] = {0, 0, 0};
    int slot_spinning = 0, slot_stopped = 0;
    int dream_iter = 0, dream_ch = 0, dream_total = 20;
    int i;

    (void)argc; (void)argv;
    VIDEO_Init();
    WPAD_Init();
    PAD_Init();
    fatInitDefault();
    load_game();

    rmode = VIDEO_GetPreferredMode(NULL);
    dio_wii_init(rmode);
    dc_wii_set_mode(rmode);

    while (1) {
        WPAD_ScanPads();
        PAD_ScanPads();
        frame++;

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
            SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);

        if (state == ST_TITLE) {
            dio_wii_clear_colour(10, 0, 16);
            dc_draw_world(8.0f, 6.0f, DREAMLINGS[selected]);
            dio_wii_draw_billboard(8.0f, 3.0f, 7.5f, 5.0f, DREAMLINGS[selected]);
            if (buttons_down(0, WPAD_BUTTON_A)) state = ST_SELECT;
        } else if (state == ST_SELECT) {
            dio_wii_clear_colour(5, 0, 10);
            dc_draw_world(8.0f, 6.0f, DREAMLINGS[selected]);
            for (i = 0; i < 11; i++) {
                float gx = 2.0f + (i % 6) * 2.4f;
                float gy = 4.5f - (i / 6) * 3.0f;
                if (i == selected)
                    dio_wii_draw_quad(gx - 1.1f, gy - 1.1f, gx + 1.1f, gy + 1.1f, 7.4f, 255, 255, 0);
                dio_wii_draw_billboard(gx, gy, 7.5f, 2.0f, DREAMLINGS[i]);
            }
            if (buttons_down(0, WPAD_BUTTON_LEFT)) selected = (selected + 10) % 11;
            if (buttons_down(0, WPAD_BUTTON_RIGHT)) selected = (selected + 1) % 11;
            if (buttons_down(0, WPAD_BUTTON_UP)) selected = (selected + 5) % 11;
            if (buttons_down(0, WPAD_BUTTON_DOWN)) selected = (selected + 6) % 11;
            if (buttons_down(0, WPAD_BUTTON_A)) {
                if (!collected[selected]) { collected[selected] = 1; ncollected++; save_game(); }
                state = ST_WORLD;
            }
            if (buttons_down(0, WPAD_BUTTON_B)) state = ST_TITLE;
        } else if (state == ST_WORLD) {
            float speed = buttons_held(0, WPAD_BUTTON_B) ? 0.30f : 0.15f;
            float old_x = px_x, old_z = pz_z;
            /* Nunchuk stick, if present, moves like the D-pad. */
            {
                WPADData *wd = WPAD_Data(0);
                if (wd && wd->exp.type == WPAD_EXP_NUNCHUK) {
                    float jx = wd->exp.nunchuk.js.pos.x - 128.0f;
                    float jy = wd->exp.nunchuk.js.pos.y - 128.0f;
                    if (jx < -32) px_x -= speed;
                    if (jx > 32) px_x += speed;
                    if (jy > 32) pz_z -= speed;
                    if (jy < -32) pz_z += speed;
                }
            }
            if (buttons_held(0, WPAD_BUTTON_LEFT)) px_x -= speed;
            if (buttons_held(0, WPAD_BUTTON_RIGHT)) px_x += speed;
            if (buttons_held(0, WPAD_BUTTON_UP)) pz_z -= speed;
            if (buttons_held(0, WPAD_BUTTON_DOWN)) pz_z += speed;
            if (px_x < 0.75f) px_x = 0.75f; if (px_x > 39.25f) px_x = 39.25f;
            if (pz_z < 0.75f) pz_z = 0.75f; if (pz_z > 29.25f) pz_z = 29.25f;
            /* Solid tiles block: step back out if we entered one. */
            if (DC_SOLID[(int)pz_z][(int)px_x]) { px_x = old_x; pz_z = old_z; }

            dc_draw_world(px_x, pz_z, DREAMLINGS[selected]);

            if (buttons_down(0, WPAD_BUTTON_A)) {
                if (near_rect(px_x, pz_z, gen_rect)) {
                    memcpy(dream_img, DREAMLINGS[selected], 576);
                    cnn_forward(dream_img);
                    dream_ch = cnn_best_channel();
                    dream_iter = 0;
                    state = ST_DREAM;
                } else if (near_machine(px_x, pz_z)) {
                    slot_spinning = 0; slot_stopped = 0;
                    state = ST_SLOT;
                }
            }
            if (buttons_down(0, WPAD_BUTTON_MINUS)) state = ST_GALLERY;
        } else if (state == ST_DREAM) {
            dc_draw_world(px_x, pz_z, dream_img);
            if (dream_iter < dream_total) {
                cnn_dream_step(dream_img, dream_ch, 16);
                dream_iter++;
            }
            dio_wii_draw_billboard(8.0f, 3.0f, 7.5f, 6.0f, dream_img);
            if (dream_iter >= dream_total && buttons_down(0, WPAD_BUTTON_A))
                state = ST_WORLD;
        } else if (state == ST_SLOT) {
            dc_draw_world(px_x, pz_z, DREAMLINGS[selected]);
            if (!slot_spinning && buttons_down(0, WPAD_BUTTON_A)) {
                slot_spinning = 1; slot_stopped = 0;
            }
            if (slot_spinning && buttons_down(0, WPAD_BUTTON_A)) {
                slot_stopped++;
                if (slot_stopped >= 3) {
                    slot_spinning = 0;
                    if (slot_reels[0] == slot_reels[1] && slot_reels[1] == slot_reels[2] && nfused < 11) {
                        int a = slot_reels[0], b = (a + 3) % 11, c = (a + 7) % 11;
                        for (i = 0; i < 576; i++)
                            fused[nfused][i] = (unsigned char)((DREAMLINGS[a][i] + DREAMLINGS[b][i] + DREAMLINGS[c][i]) / 3);
                        nfused++; ncollected++; save_game();
                    }
                }
            }
            if (slot_spinning) {
                for (i = slot_stopped; i < 3; i++) slot_reels[i] = (int)(xrnd() % 11);
            }
            for (i = 0; i < 3; i++) {
                int reel = (!slot_spinning || i < slot_stopped) ? slot_reels[i] : (int)((frame + i * 3) % 11);
                dio_wii_draw_quad(4.0f + i * 3.5f - 1.6f, 1.4f, 4.0f + i * 3.5f + 1.6f, 4.6f, 7.4f, 255, 255, 255);
                dio_wii_draw_billboard(4.0f + i * 3.5f, 3.0f, 7.5f, 3.0f, DREAMLINGS[reel]);
            }
            if (buttons_down(0, WPAD_BUTTON_B) && !slot_spinning) state = ST_WORLD;
        } else if (state == ST_GALLERY) {
            int idx = 0;
            dc_draw_world(px_x, pz_z, DREAMLINGS[selected]);
            for (i = 0; i < 11; i++) {
                if (!collected[i]) continue;
                dio_wii_draw_billboard(2.0f + (idx % 6) * 2.4f, 4.5f - (idx / 6) * 3.0f, 7.5f, 2.0f, DREAMLINGS[i]);
                idx++;
            }
            for (i = 0; i < nfused; i++) {
                dio_wii_draw_billboard(2.0f + (idx % 6) * 2.4f, 4.5f - (idx / 6) * 3.0f, 7.5f, 2.0f, fused[i]);
                idx++;
            }
            if (buttons_down(0, WPAD_BUTTON_B)) state = ST_WORLD;
        }

        dio_wii_present();
    }
    return 0;
}
