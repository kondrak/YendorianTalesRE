# Yendorian Tales - reverse engineering
Done using HxD editor and DosBox with debugger feature enabled to track all assembly calls - reverse engineering old games is really easy these days!

## Graphics
Both games store image date in a dedicated, uncompressed `PICTURES.VGA` file which is a concatenated stream of bytes - each one representing a single pixel referencing a color index in proper DOS palette. There's no size nor color information stored in this file, all data is fetched directly from the game's .EXE file. Graphics are grouped by size ranging from the largest (fullscreen images) to smallest (8x8 pixel icons used throughout the game). Palette data was extracted using DosBox's screen capture feature and saved as `.pal` files. `PICTURES.VGA` is kept open during runtime and required images are fetched as memory offsets by the game when necessary. Most animation effects (enhanced weapons, texture shimmering) is done using good old palette cycling and different enemy variantions are also achieved by using different colors - extraction program only uses the primary palette for simplicity.
Game runs in mode 13h at all times, one exception being the Pacific splash screen which is 640x480 and stored in the .EXE as a PCX file. For some reason both games fetch this data and save it on the disk, only to remove it after exiting the game.

Last color in the palette is used for transparency colorkeying - the extracted palette modifies that value to magenta for better distinction. Sample extraction program can also export palette colors to RGBA format with the final palette color being treated as transparency level.

<p align="center"><img src="bariag.png"></p>

All enemies consist of 10 frames in total:
Idle animation - first 6 frames (looped in a ping-pong manner)
Attack animation - next 3 frames
Pain animation - last 1 frame

This is true even for creatures that don't have dedicated Idle/Attack/Pain animations (like Dwarf Towers) - the images are simply duplicated. For different variants of the same enemy (for example Centipede and Millipede) different palette colors are being used.

Scene objects are duplicated in flipped and unflipped variants (used depending on player position in the world) - wasteful in terms of space but it seems this approach was easier for the developers. All sky and ground textures are represented by two images - flipped and unflipped, even if there's only one variant used in the game (like Astral Plane sky texture). Again, this is wasteful but apparently it was easier for developers to program when assuming fixed number of frames for specific items.

## Sounds
Yendorian Tales uses Creative Voice File (.VOC) for digitized audio. Sound files are stored uncompressed in the game's `WORLD.DAT` file. Size information and file offsets are stored in the .EXE file - for Yendorian Tales 2, sound data starts at offset `0x2EBF1` and `0x2D057` for Yendorian Tales 3. In both cases, audio data is followed by file sizes, so all information is there for data extraction. These files are supported by most modern sound players - VLC being my tool of choice for verification.

## Music
Yedndorian Tales uses Creative Music File (.CMF) which is similar to standard MIDI. Unlike the latter format, instrument banks are stored directly in the file, this way the music sounds exactly the same on every system. Just like digitized sounds, all CMF files are stored uncompressed in `WORLD.DAT` with size and offset information being stored in the game's .EXE file. For Yendorian Tales 2, music data starts at offset `0x2EB73` and `0x2CFC7` for Yendorian Tales 3. Music data is followed by file sizes, following the same rule as with .VOC files. Unlike the latter though, these files are hardly supported by modern audio players, so your best bet is to record it in DosBox to .WAV files and save it in a widely supported compressed format.