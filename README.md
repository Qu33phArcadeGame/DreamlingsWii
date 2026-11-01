# Deepdreamlings Wii — full diorama

A Wii homebrew port of the DS Deepdreamlings game, with the live tiny CNN
(24×24, 16.16 fixed-point, 3 conv layers, 20 displayed gradient-ascent
steps) and a full GX voxel diorama world. Not a web/TensorFlow port, and
not pre-dreamed playback: the dreaming happens on the Wii.

## What is in this version

- Six screens, same as DS: Title → Select → World → Dream → Slot/Fusion → Gallery
- World: GX diorama — wooden base, 192 voxel floor boxes, 3 tall gate
  boxes, slot cabinet, 4 trees, player as a pixel billboard. Camera locked
  straight-on front 2.5D, smooth-following the player, never rotating.
  Visible triangle budget (host-verified, see `../dreamlings-diorama-shared/test_arena.c`):
  436 triangles including the player billboard.
- Dream: step into a gate, the CNN picks its best channel and morphs the
  sprite for 20 displayed steps, shown as a large billboard.
- Slots: 3 reels, A to spin, A to stop each reel, three-of-a-kind fuses
  (average of sprites a, a+3, a+7) into the gallery.
- Save: `sd:/apps/dreamlings/dreamlings.sav` via libfat. SD only, never NAND.
- Controls: D-pad / Nunchuk stick walk, A confirm, B back/run, − gallery,
  HOME back to the Homebrew Channel. Classic/GameCube D-pad + A/B also work.

## Building

Same route as QU33PH Wii, whose Makefile this is adapted from:

- Locally: install devkitPro `wii-dev` (`dkp-pacman -S wii-dev`), then
  `make`. Output is `boot.dol`.
- Or GitHub Actions: push this folder to a repo and the workflow in
  `.github/workflows/build-wii.yml` builds it in devkitPro's devkitPPC
  container and uploads a `dreamlings-wii-sd-card` artifact laid out as:

```
apps/
  dreamlings/
    boot.dol
    meta.xml
    icon.png
```

## Putting it on the Wii

1. Wii needs the Homebrew Channel (https://wii.hacks.guide/get-started.html).
2. FAT32 SD/SDHC card, 32 GB or less. Copy the `apps` folder to the root
   (`SD:/apps/dreamlings/boot.dol`).
3. Homebrew Channel → Deepdreamlings → Load. HOME returns to the channel.
4. Dolphin test: File → Open `boot.dol` directly.

## Files

- `source/main.c` — state machine, controls, SD save (DS port)
- `source/diorama.c/.h` — GX diorama renderer
- `source/diorama_arena.h` — shared arena box list (also used by the DS lite version)
- `source/cnn.c/.h`, `source/cnn_weights.h`, `source/dreamlings.h` — verbatim from the DS version

## Known limits (this version)

- No text rendering: screens are visual (billboards, coloured highlights).
  Controls are in this README and in `meta.xml`.
- No audio yet, no pointer picking yet, no P12–P22 yet (11 Dream Core dreamlings).
- No web tile maps yet: this is the DS-port arena in 3D, not Dream Core 40×30.
