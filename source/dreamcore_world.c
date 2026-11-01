/* Dream Core world renderer -- see dreamcore_world.h.
 *
 * Geometry follows the web voxel diorama (voxel-engine.js) exactly
 * where the web defines numbers, and flat Wii colours where the web
 * uses textures (colours live in dreamcore_map.h with provenance):
 *
 *   ground (grass/illusion/flat door)  height 0.35 (DIORAMA.groundHeight)
 *   water                              height 0.22 (DIORAMA.waterHeight)
 *   dirt path (tree7/tree8)            height 0.10 (sunken, web tileHeight)
 *   solid door/roof structures         height 1.00 (DIORAMA.solidHeight)
 *   trees: trunk 0.5 x 1.8, canopy layers 1.5 x 0.7 @1.65,
 *          1.1 x 0.6 @2.2, 0.7 x 0.5 @2.65 (web pine renderer)
 *   generator: walls 2.2 + trim/roof/peak stack (web addBuildingTo)
 *   water machines: pedestal + gold trim + body + marquee, simplified
 *          static markers (web addWaterSlots; reels/lever live on the
 *          slot screen, not in the world)
 *
 * Tiles with NO ground box (matching the web): tree tiles (trunk +
 * canopy instead) and the 6 generator-footprint tiles (building
 * instead). Everything else gets exactly one ground/structure box.
 * Skipping those ground boxes also avoids coplanar front-face ties
 * with the walls/trunks standing on them.
 */
#include <string.h>
#include "dreamcore_world.h"
#include "dreamcore_map.h"
#include "diorama.h"

static GXRModeObj *dc_mode = NULL;
static float dc_cam_x = 20.0f, dc_cam_z = 15.0f;

void dc_wii_set_mode(GXRModeObj *mode) {
    dc_mode = mode;
}

static void dc_quad(float x0, float y0, float z0,
                    float x1, float y1, float z1,
                    float x2, float y2, float z2,
                    float x3, float y3, float z3,
                    unsigned char r, unsigned char g, unsigned char b) {
    GX_Position3f32(x0, y0, z0); GX_Color3u8(r, g, b);
    GX_Position3f32(x1, y1, z1); GX_Color3u8(r, g, b);
    GX_Position3f32(x2, y2, z2); GX_Color3u8(r, g, b);
    GX_Position3f32(x3, y3, z3); GX_Color3u8(r, g, b);
}

/* Top face at full colour, front face (max Z) at 3/4 colour -- the
 * diorama.c shading convention, extended to floating (y0) boxes. */
static void dc_draw_box(const DcBox *b) {
    float x0 = b->x - b->w / 2, x1 = b->x + b->w / 2;
    float z0 = b->z - b->d / 2, z1 = b->z + b->d / 2;
    float y0 = b->y0, y1 = b->y0 + b->h;
    unsigned char fr = (unsigned char)(b->r * 3 / 4);
    unsigned char fg = (unsigned char)(b->g * 3 / 4);
    unsigned char fb = (unsigned char)(b->b * 3 / 4);
    GX_Begin(GX_QUADS, GX_VTXFMT0, 8);
    dc_quad(x0, y1, z0, x1, y1, z0, x1, y1, z1, x0, y1, z1,
            b->r, b->g, b->b);
    dc_quad(x0, y0, z1, x1, y0, z1, x1, y1, z1, x0, y1, z1,
            fr, fg, fb);
    GX_End();
}

static int dc_is_dirt(const char *name) {
    return strstr(name, "tree7") != NULL || strstr(name, "tree8") != NULL;
}

static int dc_is_tree(const char *name) {
    /* tree1-6 are foliage; tree7/8 are dirt (checked first). */
    if (dc_is_dirt(name)) return 0;
    return strstr(name, "tree") != NULL;
}

/* Per-tile flat colour for ground/structure boxes. */
static void dc_tile_colour(int tile, int solid,
                           unsigned char *r, unsigned char *g, unsigned char *b) {
    switch (tile) {
        case 16: /* door.png */
            *r = DC_COL_DOOR_R; *g = DC_COL_DOOR_G; *b = DC_COL_DOOR_B; return;
        case 19: /* roof1.png */
            *r = DC_COL_ROOF1_R; *g = DC_COL_ROOF1_G; *b = DC_COL_ROOF1_B; return;
        case 22: /* roof2.png */
            *r = DC_COL_ROOF2_R; *g = DC_COL_ROOF2_G; *b = DC_COL_ROOF2_B; return;
        case 20: /* illusion1.png */
            *r = DC_COL_ILL1_R; *g = DC_COL_ILL1_G; *b = DC_COL_ILL1_B; return;
        case 17: /* illusion2.png */
            *r = DC_COL_ILL2_R; *g = DC_COL_ILL2_G; *b = DC_COL_ILL2_B; return;
        case 18: /* illusion3.png */
            *r = DC_COL_ILL3_R; *g = DC_COL_ILL3_G; *b = DC_COL_ILL3_B; return;
        case 21: /* illusion4.png */
            *r = DC_COL_ILL4_R; *g = DC_COL_ILL4_G; *b = DC_COL_ILL4_B; return;
        case 23: /* illusion5.png */
            *r = DC_COL_ILL5_R; *g = DC_COL_ILL5_G; *b = DC_COL_ILL5_B; return;
        case 24: /* illusion6.png */
            *r = DC_COL_ILL6_R; *g = DC_COL_ILL6_G; *b = DC_COL_ILL6_B; return;
        case 27: /* illusion7.png */
            *r = DC_COL_ILL7_R; *g = DC_COL_ILL7_G; *b = DC_COL_ILL7_B; return;
        case 26: /* illusion8.png */
            *r = DC_COL_ILL8_R; *g = DC_COL_ILL8_G; *b = DC_COL_ILL8_B; return;
        case 28: /* illusion9.png */
            *r = DC_COL_ILL9_R; *g = DC_COL_ILL9_G; *b = DC_COL_ILL9_B; return;
        default:
            (void)solid;
            *r = DC_COL_GRASS_R; *g = DC_COL_GRASS_G; *b = DC_COL_GRASS_B; return;
    }
}

static void dc_push(DcBox *out, int max_boxes, int *n,
                    float x, float y0, float z,
                    float w, float h, float d,
                    unsigned char r, unsigned char g, unsigned char b) {
    DcBox *box;
    if (*n >= max_boxes) return;
    box = &out[*n];
    box->x = x; box->y0 = y0; box->z = z;
    box->w = w; box->h = h; box->d = d;
    box->r = r; box->g = g; box->b = b;
    (*n)++;
}

int dc_build_boxes(DcBox *out, int max_boxes) {
    int n = 0;
    int c, r, i;
    /* Wooden diorama base, one unit larger than the map on each side,
     * top at Y=0 (same convention as the arena base). */
    dc_push(out, max_boxes, &n, 20.0f, -0.8f, 15.0f,
            42.0f, 0.8f, 32.0f,
            DC_COL_BASE_R, DC_COL_BASE_G, DC_COL_BASE_B);
    /* Ground / water / dirt / structure boxes, one per tile except
     * tree tiles and generator-footprint tiles (see file comment). */
    for (r = 0; r < DC_ROWS; r++) {
        for (c = 0; c < DC_COLS; c++) {
            int tile = DC_TILES[r][c];
            int solid = DC_SOLID[r][c];
            const char *name = DC_TILE_NAMES[tile];
            unsigned char cr, cg, cb;
            float h;
            float x = c + 0.5f, z = r + 0.5f;
            int in_gen = (c >= DC_GEN_X && c < DC_GEN_X + DC_GEN_W &&
                          r >= DC_GEN_Y && r < DC_GEN_Y + DC_GEN_H);
            if (dc_is_tree(name) || in_gen) continue;
            if (strstr(name, "water") != NULL) {
                h = 0.22f;
                cr = DC_COL_WATER_R; cg = DC_COL_WATER_G; cb = DC_COL_WATER_B;
            } else if (dc_is_dirt(name)) {
                h = 0.10f;
                cr = DC_COL_DIRT_R; cg = DC_COL_DIRT_G; cb = DC_COL_DIRT_B;
            } else if ((strstr(name, "door") != NULL ||
                        strstr(name, "roof") != NULL) && solid) {
                h = 1.00f;
                dc_tile_colour(tile, solid, &cr, &cg, &cb);
            } else {
                h = 0.35f;
                dc_tile_colour(tile, solid, &cr, &cg, &cb);
            }
            dc_push(out, max_boxes, &n, x, 0.0f, z, 1.0f, h, 1.0f, cr, cg, cb);
        }
    }
    /* Trees: trunk + 3 canopy layers (web pine renderer). */
    for (r = 0; r < DC_ROWS; r++) {
        for (c = 0; c < DC_COLS; c++) {
            const char *name = DC_TILE_NAMES[DC_TILES[r][c]];
            float x, z;
            if (!dc_is_tree(name)) continue;
            x = c + 0.5f; z = r + 0.5f;
            dc_push(out, max_boxes, &n, x, 0.0f, z,
                    0.5f, 1.8f, 0.5f,
                    DC_COL_TRUNK_R, DC_COL_TRUNK_G, DC_COL_TRUNK_B);
            dc_push(out, max_boxes, &n, x, 1.65f, z,
                    1.5f, 0.7f, 1.5f,
                    DC_COL_CANOPY_R, DC_COL_CANOPY_G, DC_COL_CANOPY_B);
            dc_push(out, max_boxes, &n, x, 2.20f, z,
                    1.1f, 0.6f, 1.1f,
                    DC_COL_CANOPY_R, DC_COL_CANOPY_G, DC_COL_CANOPY_B);
            dc_push(out, max_boxes, &n, x, 2.65f, z,
                    0.7f, 0.5f, 0.7f,
                    DC_COL_CANOPY_R, DC_COL_CANOPY_G, DC_COL_CANOPY_B);
        }
    }
    /* Generator building (web addBuildingTo, style "generator").
     * Footprint rect is the tile rect itself: x 33..36, z 27..29. */
    {
        float cx = DC_GEN_X + DC_GEN_W / 2.0f;   /* 34.5 */
        float cz = DC_GEN_Y + DC_GEN_H / 2.0f;   /* 28.0 */
        static const float pilx[4] = { 33.45f, 35.55f, 33.45f, 35.55f };
        static const float pilz[4] = { 27.45f, 27.45f, 28.55f, 28.55f };
        dc_push(out, max_boxes, &n, cx, 0.0f, cz,
                3.0f, 2.2f, 2.0f,
                DC_COL_GENWALL_R, DC_COL_GENWALL_G, DC_COL_GENWALL_B);
        dc_push(out, max_boxes, &n, cx, 2.20f, cz,
                3.10f, 0.07f, 2.10f,
                DC_COL_GENGLOW_R, DC_COL_GENGLOW_G, DC_COL_GENGLOW_B);
        dc_push(out, max_boxes, &n, cx, 2.25f, cz,
                3.15f, 0.14f, 2.15f,
                DC_COL_GENTRIM_R, DC_COL_GENTRIM_G, DC_COL_GENTRIM_B);
        dc_push(out, max_boxes, &n, cx, 2.39f, cz,
                3.50f, 0.35f, 2.50f,
                DC_COL_GENROOF_R, DC_COL_GENROOF_G, DC_COL_GENROOF_B);
        dc_push(out, max_boxes, &n, cx, 2.74f, cz,
                1.50f, 0.25f, 1.00f,
                DC_COL_GENTRIM_R, DC_COL_GENTRIM_G, DC_COL_GENTRIM_B);
        for (i = 0; i < 4; i++) {
            dc_push(out, max_boxes, &n, pilx[i], 0.0f, pilz[i],
                    0.22f, 2.5f, 0.22f,
                    DC_COL_GENPILL_R, DC_COL_GENPILL_G, DC_COL_GENPILL_B);
        }
        /* Orb: small box standing in for the web's glowing sphere. */
        dc_push(out, max_boxes, &n, cx, 2.88f, cz,
                0.84f, 0.84f, 0.84f,
                DC_COL_GENORB_R, DC_COL_GENORB_G, DC_COL_GENORB_B);
        /* Door glow on the front face (max Z side). */
        dc_push(out, max_boxes, &n, cx, 0.0f, 29.04f,
                0.6f, 1.1f, 0.08f,
                DC_COL_GENDOOR_R, DC_COL_GENDOOR_G, DC_COL_GENDOOR_B);
    }
    /* Water slot machines: static markers at the baked centroids. */
    for (i = 0; i < 3; i++) {
        float mx = DC_MACHINES[i].x + 0.5f;
        float mz = DC_MACHINES[i].y + 0.5f;
        dc_push(out, max_boxes, &n, mx, 0.0f, mz,
                1.10f, 0.25f, 0.90f,
                DC_COL_MACHPED_R, DC_COL_MACHPED_G, DC_COL_MACHPED_B);
        dc_push(out, max_boxes, &n, mx, 0.25f, mz,
                1.14f, 0.06f, 0.94f,
                DC_COL_MACHGOLD_R, DC_COL_MACHGOLD_G, DC_COL_MACHGOLD_B);
        dc_push(out, max_boxes, &n, mx, 0.31f, mz,
                0.92f, 1.35f, 0.64f,
                DC_COL_MACHBODY_R, DC_COL_MACHBODY_G, DC_COL_MACHBODY_B);
        dc_push(out, max_boxes, &n, mx, 1.66f, mz,
                0.92f, 0.15f, 0.66f,
                DC_COL_MACHMARQ_R, DC_COL_MACHMARQ_G, DC_COL_MACHMARQ_B);
    }
    return n;
}

static void dc_set_camera(float tx, float tz) {
    /* Same convention as diorama.c set_camera, with Dream Core
     * clamps: the 40x30 map, inset by the arena's 4 / 3 margins. */
    Mtx44 proj;
    Mtx view;
    float aspect;
    if (tx < 4.0f) tx = 4.0f; if (tx > 36.0f) tx = 36.0f;
    if (tz < 3.0f) tz = 3.0f; if (tz > 27.0f) tz = 27.0f;
    dc_cam_x += (tx - dc_cam_x) * 0.15f;
    dc_cam_z += (tz - dc_cam_z) * 0.15f;
    aspect = dc_mode ? (float)dc_mode->fbWidth / (float)dc_mode->efbHeight
                     : 640.0f / 480.0f;
    guPerspective(proj, 45, aspect, 0.1f, 100.0f);
    GX_LoadProjectionMtx(proj, GX_PERSPECTIVE);
    guLookAt(view,
             &(guVector){dc_cam_x + 1.5f, 4.2f, dc_cam_z + 9.0f},
             &(guVector){0.0f, 1.0f, 0.0f},
             &(guVector){dc_cam_x, 0.5f, dc_cam_z});
    GX_LoadPosMtxImm(view, GX_PNMTX0);
}

void dc_draw_world(float px, float pz, const unsigned char *sprite) {
    static DcBox boxes[DC_MAX_BOXES];
    int n = dc_build_boxes(boxes, DC_MAX_BOXES);
    int i;
    dc_set_camera(px, pz);
    for (i = 0; i < n; i++) dc_draw_box(&boxes[i]);
    /* Blob shadow under the player, then the player billboard --
     * identical to dio_wii_draw_world. */
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    dc_quad(px - 0.8f, 0.36f, pz - 0.8f, px + 0.8f, 0.36f, pz - 0.8f,
            px + 0.8f, 0.36f, pz + 0.8f, px - 0.8f, 0.36f, pz + 0.8f,
            0, 0, 0);
    GX_End();
    dio_wii_draw_billboard(px, 1.1f, pz, 1.5f, sprite);
}
