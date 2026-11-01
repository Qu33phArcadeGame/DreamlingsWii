/* Dream Core map data for the DreamlingsWii port (Phase 1: world content).
 *
 * SOURCE: https://raw.githubusercontent.com/Qu33phArcadeGame/DeepdreamlingsFusion/main/map.json
 *   (the live DeepdreamlingsFusion repo, branch main; re-read and verified
 *   row by row against the live file before this header was written).
 *   NOTE: the local copy at ~/workspace/dreamlings-3d/map.json is STALE --
 *   it is byte-identical to area3.json (Zentangle Meadow, 15x12) and is NOT
 *   Dream Core. It was not used. The live file above is the only source.
 *
 * CONVERSION (live JSON -> this header, done by script, not by hand):
 *   - "map"    (30x40 tile indices)  -> DC_TILES, verbatim, row-major.
 *   - "solid"  (30x40 0/1)           -> DC_SOLID, verbatim, row-major.
 *   - "encounter" (30x40, 0 / 2 / "1,2") -> DC_ENC: 0 = none, 1 = the "1,2"
 *     cells, 2 = the 2 cells. The original string-vs-int distinction is
 *     preserved as 1-vs-2; what each code MEANS for encounter triggers is
 *     TBD in the encounter-trigger phase (see REPORT.md) -- this header
 *     only carries the data.
 *   - "warps" / "spawns" -> DC_WARPS / DC_SPAWNS, verbatim.
 *   - map.json has NO "generator" key, so the generator rect comes from
 *     the web default in index.html GENERATORS["map"]: {x:33, y:27, w:3, h:2}.
 *     Those 6 cells are plain grass (tile 7) in the map; the generator is
 *     an overlay building drawn on top (see dreamcore_world.c).
 *   - Water slot-machine tiles are BAKED, not live-computed: the web
 *     flood-fills water tiles into 4-connected regions and puts one
 *     machine at the rounded centroid of each region >= 4 tiles. The same
 *     flood fill run over DC_TILES gives 3 regions (208 / 12 / 6 tiles)
 *     with rounded centroids (32,6), (10,28), (10,21) -- all on water
 *     tiles (30 / 32 / 31). DC_MACHINES holds those three tiles.
 *
 * COORDINATES (Wii world units, matching dio_build_wii_boxes):
 *   Tile (c,r) is centred at (c+0.5, r+0.5) and spans [c,c+1] x [r,r+1].
 *   The map spans X 0..40, Z 0..30. The web centres tiles at integers;
 *   that is a uniform 0.5 shift which does not affect layout, and the
 *   Wii convention is used here so the existing Wii player/camera code
 *   (tile = floor(px), floor(pz)) works unchanged.
 *   Warp/spawn/machine positions below are TILE coords; add 0.5 for
 *   world coords (e.g. start spawn -> world (4.5, 25.5)).
 *
 * VERIFIED BEFORE WRITING (host checks, all pass):
 *   - 30 rows x 40 cols for map/solid/encounter; every map value 0..33
 *     (palette indices 34..73, the 1000084*.png series, are UNUSED by
 *     the grid, so no colours are needed for them).
 *   - 38 solid cells; 50 encounter cells (32 code-1 + 18 code-2).
 *   - Both warps land on door tiles (16) that are NOT solid (walkable).
 *   - All three spawns are on non-solid cells.
 *   - The 6 generator cells are all grass (tile 7).
 *   - Tile-kind census: grass 835, water 226, dirt 60, trees 50,
 *     illusion 14, door/roof structures 15. Total 1200.
 */
#pragma once

#define DC_COLS 40
#define DC_ROWS 30

/* Tile palette, in live-file order (indices 0..73). Only 0..33 are used. */
static const char *DC_TILE_NAMES[74] = {
    "tree7.png",
    "tree4.png",
    "tree5.png",
    "tree3.png",
    "tree2.png",
    "tree8.png",
    "tree1.png",
    "grass9.png",
    "grass8.png",
    "grass7.png",
    "grass6.png",
    "grass5.png",
    "grass4.png",
    "grass3.png",
    "grass1.png",
    "grass2.png",
    "door.png",
    "illusion2.png",
    "illusion3.png",
    "roof1.png",
    "illusion1.png",
    "illusion4.png",
    "roof2.png",
    "illusion5.png",
    "illusion6.png",
    "water1.png",
    "illusion8.png",
    "illusion7.png",
    "illusion9.png",
    "water5.png",
    "water3.png",
    "water4.png",
    "water6.png",
    "water2.png",
    "1000084751.png",
    "1000084750.png",
    "1000084726.png",
    "1000084749.png",
    "1000084747.png",
    "1000084748.png",
    "1000084727.png",
    "1000084728.png",
    "1000084723.png",
    "1000084725.png",
    "1000084721.png",
    "1000084722.png",
    "1000084716.png",
    "1000084724.png",
    "1000084720.png",
    "1000084718.png",
    "1000084715.png",
    "1000084714.png",
    "1000084717.png",
    "1000084719.png",
    "1000084711.png",
    "1000084708.png",
    "1000084707.png",
    "1000084702.png",
    "1000084712.png",
    "1000084703.png",
    "1000084713.png",
    "1000084706.png",
    "1000084705.png",
    "1000084672.png",
    "1000084675.png",
    "1000084704.png",
    "1000084674.png",
    "1000084673.png",
    "1000084671.png",
    "1000084670.png",
    "1000084669.png",
    "1000084667.png",
    "1000084668.png",
    "1000084657.png"
};

static const unsigned char DC_TILES[DC_ROWS][DC_COLS] = {
    {15,15,15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10,10,10,10,10,10,29,29,29,30,30,30,31,31,31,32,32,32,33,33,33,33},
    {15,15,15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10,0,0,0,0,0,29,29,29,30,30,30,31,31,31,32,32,32,33,33,33,33},
    {15,15,15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10,0,21,18,21,0,29,29,29,30,30,30,31,31,31,32,32,32,33,33,33,33},
    {15,15,15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10,0,21,18,21,0,29,29,29,30,30,30,31,31,31,32,32,32,33,33,33,33},
    {15,15,15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10,0,21,18,21,0,29,29,29,30,30,30,25,25,25,25,25,25,25,25,33,33},
    {15,15,15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10,0,21,18,21,0,29,29,29,30,30,30,25,25,25,25,25,25,25,25,33,33},
    {7,7,7,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,5,5,16,5,5,29,29,29,30,30,30,25,25,25,25,25,25,25,25,33,33},
    {7,7,7,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,12,14,5,14,12,29,29,29,30,30,30,25,25,25,25,25,25,25,25,33,33},
    {7,7,7,14,14,4,14,14,14,14,14,14,14,4,14,14,14,14,14,12,14,5,14,12,29,29,29,30,30,30,31,31,31,32,32,32,33,33,33,33},
    {8,8,8,14,4,6,4,14,14,6,14,14,4,6,4,14,14,14,14,12,14,5,14,12,29,29,29,30,30,30,31,31,31,32,32,32,33,33,33,33},
    {8,8,8,14,14,2,14,14,6,4,6,14,14,1,14,14,14,14,14,12,14,5,14,12,29,29,29,30,30,30,31,31,31,32,32,32,33,33,33,33},
    {8,8,8,14,14,2,14,14,14,3,14,14,14,1,14,14,14,14,14,12,12,5,12,12,29,29,29,30,30,30,31,31,31,32,32,32,33,33,33,33},
    {9,9,9,14,14,14,14,14,14,3,14,14,14,14,14,14,14,14,14,11,11,5,10,10,29,29,29,30,30,30,31,31,31,32,32,32,33,33,33,33},
    {9,9,9,6,14,14,14,4,14,14,14,6,14,14,14,4,14,14,14,11,11,5,10,10,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14},
    {9,9,6,6,6,14,4,4,4,14,6,6,6,14,4,4,4,14,14,11,11,5,10,10,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14},
    {10,10,6,6,6,14,4,4,4,14,6,6,6,14,4,4,4,14,14,12,12,5,9,9,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14},
    {10,10,10,5,14,14,14,0,14,14,14,5,14,14,14,0,14,14,14,12,12,5,9,9,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14},
    {10,10,10,5,14,14,14,0,14,14,14,5,14,14,14,0,14,14,14,12,12,5,9,9,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14},
    {11,11,11,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,13,13,5,8,8,14,14,14,14,14,14,14,14,14,14,14,14,14,14,19,22},
    {11,11,11,14,14,14,14,14,14,14,14,14,14,6,14,14,14,14,13,13,5,8,8,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,16,22},
    {11,11,11,14,14,14,14,14,14,33,30,25,14,14,6,6,6,14,14,14,13,13,5,8,8,14,14,14,14,14,14,14,14,14,14,14,14,14,19,22},
    {12,12,12,14,14,14,14,14,29,32,31,14,14,14,28,14,14,14,14,15,15,5,7,7,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14},
    {12,12,12,14,14,14,14,14,14,14,14,14,14,14,28,14,14,14,14,15,15,5,7,7,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14},
    {12,12,12,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,15,15,5,7,7,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14},
    {13,13,13,16,16,16,14,14,14,14,14,14,14,14,14,14,14,14,14,15,15,5,7,7,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14},
    {13,13,13,16,5,16,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,7,7,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14},
    {13,13,13,16,16,16,14,14,14,14,14,14,14,14,14,14,14,14,14,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14},
    {15,15,15,12,12,12,11,11,25,33,29,30,10,9,9,9,8,8,8,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7},
    {15,15,15,12,12,12,11,11,31,25,32,33,10,9,9,9,8,8,8,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7},
    {15,15,15,12,12,12,11,11,30,29,31,25,10,9,9,9,8,8,8,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7},
};

static const unsigned char DC_SOLID[DC_ROWS][DC_COLS] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

static const unsigned char DC_ENC[DC_ROWS][DC_COLS] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

/* Warps: {tile x, tile y, destination map, entry spawn name}. */
typedef struct { int x, y; const char *to_map; const char *entry; } DcWarp;
static const DcWarp DC_WARPS[2] = {
    { 21,  6, "room1", "door1" },
    { 38, 19, "hall1", "door"  },
};

/* Spawns: {tile x, tile y, name}. World coords = tile + 0.5. */
typedef struct { int x, y; const char *name; } DcSpawn;
static const DcSpawn DC_SPAWNS[3] = {
    {  4, 25, "start"    },
    { 21,  7, "door1"    },
    { 37, 19, "fromhall" },
};

/* Generator building rect, in tiles (web GENERATORS["map"] default). */
#define DC_GEN_X 33
#define DC_GEN_Y 27
#define DC_GEN_W 3
#define DC_GEN_H 2

/* Water slot-machine tiles (baked flood-fill centroids, see header note). */
typedef struct { int x, y; } DcMachine;
static const DcMachine DC_MACHINES[3] = {
    { 32,  6 },   /* big north-east water, 208 tiles */
    { 10, 28 },   /* south pond, 12 tiles */
    { 10, 21 },   /* middle pond, 6 tiles */
};

/* ---- Colours (flat Wii approximations of the web diorama) ----
 * Provenance is stated per colour because the Wii renderer is flat
 * (no textures) while the web renders textured tiles:
 *   WEB  = the web voxel-engine.js constant itself (authoritative).
 *   SAMP = average of the matching local 64x64 tile PNG in
 *          ~/workspace/dreamlings-3d/ (a downscale of the live 256x256
 *          art; averages survive downscaling). Green-dominant pixels
 *          only for canopy, blue-dominant only for water.
 *   APPROX = no usable sample exists; flagged for the visual check.
 */
#define DC_COL_GRASS_R 0x5a
#define DC_COL_GRASS_G 0xa8
#define DC_COL_GRASS_B 0x4a  /* WEB: procedural Dream Core grass */
#define DC_COL_BASE_R  0x5a
#define DC_COL_BASE_G  0x3a
#define DC_COL_BASE_B  0x28  /* WEB: DIORAMA.baseColor (diorama slab) */
#define DC_COL_TRUNK_R 0x6b
#define DC_COL_TRUNK_G 0x4a
#define DC_COL_TRUNK_B 0x2f  /* WEB: tree trunk material */
#define DC_COL_CANOPY_R 0x37
#define DC_COL_CANOPY_G 0x8f
#define DC_COL_CANOPY_B 0x41 /* SAMP: tree1-5 green avg (55,143,65) */
#define DC_COL_WATER_R 0x41
#define DC_COL_WATER_G 0x7b
#define DC_COL_WATER_B 0xc9 /* SAMP: water1-6 blue avg (65,123,201) */
#define DC_COL_DIRT_R 0x8b
#define DC_COL_DIRT_G 0x6b
#define DC_COL_DIRT_B 0x48 /* APPROX: live tree7.png is real 256x256 dirt
 * art (8751 bytes) that could not be pixel-sampled from this VM, and
 * the local tree7/tree8 PNGs are green placeholders, NOT dirt. This
 * warm brown is a placeholder to be confirmed against the first
 * Dolphin screenshot and adjusted in this one place if it reads wrong. */
#define DC_COL_DOOR_R 0x86
#define DC_COL_DOOR_G 0x5c
#define DC_COL_DOOR_B 0x31 /* SAMP: door.png avg (134,92,49) */
#define DC_COL_ROOF1_R 0xa6
#define DC_COL_ROOF1_G 0x2d
#define DC_COL_ROOF1_B 0xa9 /* SAMP: roof1.png avg (166,45,169) */
#define DC_COL_ROOF2_R 0x6e
#define DC_COL_ROOF2_G 0xa1
#define DC_COL_ROOF2_B 0x6f /* SAMP: roof2.png avg (110,161,111) */
/* Illusion tiles: per-tile SAMP averages from the local PNGs. */
#define DC_COL_ILL1_R 0x58
#define DC_COL_ILL1_G 0xbb
#define DC_COL_ILL1_B 0xd2 /* illusion1.png (88,187,210) */
#define DC_COL_ILL2_R 0x09
#define DC_COL_ILL2_G 0x82
#define DC_COL_ILL2_B 0xc2 /* illusion2.png (9,130,194) */
#define DC_COL_ILL3_R 0xbb
#define DC_COL_ILL3_G 0x7c
#define DC_COL_ILL3_B 0xd9 /* illusion3.png (187,124,217) */
#define DC_COL_ILL4_R 0x3c
#define DC_COL_ILL4_G 0x77
#define DC_COL_ILL4_B 0x2e /* illusion4.png (60,119,46) */
#define DC_COL_ILL5_R 0x26
#define DC_COL_ILL5_G 0x4c
#define DC_COL_ILL5_B 0x5e /* illusion5.png (38,76,94) */
#define DC_COL_ILL6_R 0x71
#define DC_COL_ILL6_G 0x43
#define DC_COL_ILL6_B 0x1b /* illusion6.png (113,67,27) */
#define DC_COL_ILL7_R 0xc1
#define DC_COL_ILL7_G 0xcb
#define DC_COL_ILL7_B 0x9a /* illusion7.png (193,203,154) */
#define DC_COL_ILL8_R 0x4d
#define DC_COL_ILL8_G 0x40
#define DC_COL_ILL8_B 0x94 /* illusion8.png (77,64,148) */
#define DC_COL_ILL9_R 0xc1
#define DC_COL_ILL9_G 0x8e
#define DC_COL_ILL9_B 0xdd /* illusion9.png (193,142,221) */
/* Generator building (WEB: addBuildingTo, style "generator"). */
#define DC_COL_GENWALL_R 0x3d
#define DC_COL_GENWALL_G 0x26
#define DC_COL_GENWALL_B 0x70
#define DC_COL_GENTRIM_R 0x6a
#define DC_COL_GENTRIM_G 0x4a
#define DC_COL_GENTRIM_B 0x9a
#define DC_COL_GENGLOW_R 0xb3
#define DC_COL_GENGLOW_G 0x88
#define DC_COL_GENGLOW_B 0xff
#define DC_COL_GENROOF_R 0x1a
#define DC_COL_GENROOF_G 0x0f
#define DC_COL_GENROOF_B 0x2e
#define DC_COL_GENPILL_R 0x5a
#define DC_COL_GENPILL_G 0x3a
#define DC_COL_GENPILL_B 0x8a
#define DC_COL_GENORB_R 0xd0
#define DC_COL_GENORB_G 0xa0
#define DC_COL_GENORB_B 0xff
#define DC_COL_GENDOOR_R 0x8a
#define DC_COL_GENDOOR_G 0xff
#define DC_COL_GENDOOR_B 0xea
/* Water slot machines (WEB: addWaterSlots). */
#define DC_COL_MACHPED_R 0x1a
#define DC_COL_MACHPED_G 0x0a
#define DC_COL_MACHPED_B 0x2e
#define DC_COL_MACHGOLD_R 0xd4
#define DC_COL_MACHGOLD_G 0xa0
#define DC_COL_MACHGOLD_B 0x17
#define DC_COL_MACHBODY_R 0x5a
#define DC_COL_MACHBODY_G 0x1a
#define DC_COL_MACHBODY_B 0x8a
#define DC_COL_MACHMARQ_R 0xff
#define DC_COL_MACHMARQ_G 0x4a
#define DC_COL_MACHMARQ_B 0xd8

