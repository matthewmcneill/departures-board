# Font Reverse Engineering and Build Chain

This document provides a technical deep-dive into the custom fonts used by the Departures Board project, the U8G2 compression format, and the automated build pipeline used to maintain them.

## Overview

The Project uses authentic UK railway and TfL station board fonts. These were originally sourced as binary arrays from the `U8G2` library and custom-edited to match real-world display hardware.

## Font Pipeline

The font system follows a fully reversible "Round Trip" lifecycle:

1.  **Original Arrays**: Authentic font data is stored in [fonts_recovered.h](file:///Users/mcneillm/Documents/Projects/departures-board/docs/fonts/fonts_recovered.h).
2.  **Decompilation**: [font_decompiler.c](file:///Users/mcneillm/Documents/Projects/departures-board/scripts/font_decompiler.c) extracts these arrays into human-readable `.txt` dumps in `docs/fonts/dumps/`.
3.  **BDF Generation**: [txt_to_bdf.py](file:///Users/mcneillm/Documents/Projects/departures-board/scripts/txt_to_bdf.py) parses the dumps into standard `.bdf` source files in `modules/displayManager/fonts/source/`.
4.  **C++ Compilation**: [build_fonts.py](file:///Users/mcneillm/Documents/Projects/departures-board/scripts/build_fonts.py) (via PlatformIO) uses the `bdfconv` utility to generate the final [fonts.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/fonts/fonts.cpp).

---

## Technical Deep-Dive: U8G2 Format

The U8G2 font format is optimized for embedded systems with low memory. It uses a bit-level RLE (Run-Length Encoding) compression scheme.

### 1. The Global Header
Every font array starts with a 23-byte header defining:
- **Glyph Count**: Number of characters in the font.
- **BBX Mode**: Bounding box strategy.
- **RLE Lengths (b0, b1)**: The number of bits used to store background (0) and foreground (1) run lengths.
- **Bit-field Lengths**: Bits used for width, height, and offsets in character records.

### 2. Character Records
Each glyph consists of:
- **Unicode**: 1 or 2 bytes.
- **Size**: Total bytes for this glyph's data.
- **Metadata**: Bits for Width (W), Height (H), X-Offset, Y-Offset, and Horizontal Advance (dX).
- **Bitmap Stream**: The RLE-encoded pixel data.

### 3. Bitstream and RLE Decoding
U8G2 uses a **Least Significant Bit (LSB) first** bitstream within each byte.

**Decoding Logic**:
1. Read `b0` bits for the background run (length of '0's).
2. Read `b1` bits for the foreground run (length of '1's).
3. Read 1 "stop bit".
   - If the stop bit is `1`, the sequence continues (repeat steps 1-2).
   - If the stop bit is `0`, the glyph ends (or next row starts depending on context).

---

## Editing Fonts

Because the pipeline is reversible, you can edit the fonts using any standard BDF font editor.

### Recommended Software
- **[FontForge](https://fontforge.org/)**: A powerful open-source font editor (Mac, Linux, Windows). Great for precise pixel editing.
- **[Fony](http://hukka.ututil.fi/fony/)**: A lightweight Windows-based pixel font editor (works well under Wine on Mac).
- **[GIMP](https://www.gimp.org/)**: Can import/export BDF files as bitmap images.

### How to apply changes:
1. Edit the `.bdf` file in `modules/displayManager/fonts/source/`.
2. Save the file.
3. Run `pio run` (or `python scripts/build_fonts.py`) to regenerate `fonts.cpp`.
4. The decompiler and `txt_to_bdf.py` can then be used to "back-propagate" changes to the documentation dumps if desired.

---

## Restoration Success (March 2026)

The font pipeline was restored after discovering corrupted metadata in intermediate dumps. By cross-referencing the official `u8g2_font.c` source code, a character-accurate decompiler was developed to recover the authentic UK rail assets from legacy git history. 

Verification confirms that the current `fonts.cpp` is 100% binary-compatible with the original authentic source arrays.
