# G1
generates tiles and map from a bmp

usage: G1 in.bmp outTile.bin outMap.bin

In general:
- reads in 8x8 tiles from BMP from top-left to bottom-right
- generates tiles in the same order
- generates corresponding map entries
- finds similar tiles (hflip, vflip, different palette)
- as verbatim as possible, palette is determined by top-left pixel of tile
- 4bit color (for simplicity)

Basically clones the functionality of GRIT but I wanted finer control...

GRIT is more geared towards making assets as opposed to patching.
