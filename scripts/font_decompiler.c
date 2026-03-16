/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: scripts/font_decompiler.c
 * Description: Specialized tool to decompress and decompile U8G2 formatted font 
 *              arrays back into human-readable text dumps and ASCII art grids.
 *              Used for reverse-engineering and authenticity verification.
 *
 * Exported Functions:
 * - get_bits: Reads N bits from the LSB-first U8G2 bitstream.
 * - get_signed_bits: Reads N bits and converts to a signed integer.
 * - decompile_font: Main loop for parsing a U8G2 font array.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief State container for the bit-level reader.
 */
typedef struct {
    const uint8_t *font; // Pointer to the start of the font array
    uint32_t bit_pos;    // Current bit offset in the stream
} bitstream_t;

/**
 * @brief Reads a specific number of bits from the bitstream.
 * @param bs Pointer to the bitstream state
 * @param count Number of bits to read (0-32)
 * @return The unsigned integer value of the bits read
 */
uint32_t get_bits(bitstream_t *bs, uint8_t count) {
    uint32_t val = 0;
    for (uint8_t i = 0; i < count; i++) {
        uint32_t byte_idx = bs->bit_pos / 8;
        uint8_t bit_idx = bs->bit_pos % 8;
        
        // U8G2 bits are stored LSB-first within each byte
        if (bs->font[byte_idx] & (1 << bit_idx)) {
            val |= (1 << i);
        }
        bs->bit_pos++;
    }
    return val;
}

/**
 * @brief Reads a specific number of bits and interprets them as a signed offset.
 * @param bs Pointer to the bitstream state
 * @param count Number of bits to read
 * @return Signed integer value (U8G2 uses a specific offset-bias for signs)
 */
int32_t get_signed_bits(bitstream_t *bs, uint8_t count) {
    if (count == 0) return 0;
    int32_t v = (int32_t)get_bits(bs, count);
    int32_t d = 1 << (count - 1); // Mid-point bias for signed values
    return v - d;
}

/**
 * @brief Decompiles a single U8G2 font array into a .txt dump.
 * @param name The name of the font (used for output filename)
 * @param font Pointer to the raw U8G2 binary data
 * @param size Total size of the font array in bytes
 */
void decompile_font(const char *name, const uint8_t *font, uint32_t size) {
    char out_filename[256];
    snprintf(out_filename, sizeof(out_filename), "%s.txt", name);
    printf("Decompiling %s -> %s...\n", name, out_filename);
    
    FILE *f = fopen(out_filename, "w");
    if (!f) return;
    
    fprintf(f, "Font: %s\n", name);
    
    // --- Phase 1: Parse Global Header ---
    // The U8G2 header contains global metadata and bit-field lengths for glyphs.
    uint8_t glyph_cnt = font[0];
    uint8_t bbx_mode = font[1];
    uint8_t b0 = font[2]; // Background RLE bit count
    uint8_t b1 = font[3]; // Foreground RLE bit count
    uint8_t bits_w = font[4];
    uint8_t bits_h = font[5];
    uint8_t bits_x = font[6];
    uint8_t bits_y = font[7];
    uint8_t bits_dx = font[8];
    
    fprintf(f, "Glyph count: %d\n", glyph_cnt);
    fprintf(f, "b0: %d, b1: %d\n", b0, b1);
    
    uint32_t pos = 23; // First glyph always starts after the 23-byte header
    uint16_t unicode_prefix = 0;
    
    // --- Phase 2: Iterate Glyphs ---
    while (pos < size) {
        uint16_t unicode = font[pos]; 
        uint8_t sz = font[pos+1]; // Size of the glyph record in bytes
        
        // Handle Unicode block markers (size 0)
        if (sz == 0) {
            pos += 2;
            if (pos >= size) break;
            unicode_prefix = (font[pos] << 8) | font[pos+1];
            if (unicode_prefix == 0) break; // Terminating null prefix
            pos += 2;
            continue;
        }
        
        unicode |= (unicode_prefix & 0xFF00);
        
        bitstream_t bs;
        bs.font = font;
        bs.bit_pos = (pos + 2) * 8; // Bitstream starts after Unicode and size bytes
        
        // Read glyph-specific bounding box and advance data
        int32_t w = get_bits(&bs, bits_w);
        int32_t h = get_bits(&bs, bits_h);
        int32_t x = get_signed_bits(&bs, bits_x);
        int32_t y = get_signed_bits(&bs, bits_y);
        int32_t dx = get_signed_bits(&bs, bits_dx);
        
        fprintf(f, "\nCharacter U+%04X (Char: '%c')\n", unicode, (unicode >= 32 && unicode <= 126) ? (char)unicode : ' ');
        fprintf(f, "Width: %d, Height: %d, X-Offset: %d, Y-Offset: %d, dX: %d\n", w, h, x, y, dx);
        
        // --- Phase 3: Decode Bitmap RLE ---
        if (w > 0 && h > 0) {
            char *grid = malloc(w * h);
            memset(grid, '.', w * h);
            
            int curr_x = 0;
            int curr_y = 0;
            
            while (curr_y < h) {
                uint32_t a = get_bits(&bs, b0); // Length of background run
                uint32_t b = get_bits(&bs, b1); // Length of foreground run
                
                do {
                    // Fill background
                    for (uint32_t j = 0; j < a; j++) {
                        curr_x++;
                        if (curr_x >= w) { curr_x = 0; curr_y++; }
                    }
                    
                    // Fill foreground (the actual pixels)
                    for (uint32_t j = 0; j < b; j++) {
                        if (curr_y < h && curr_x < w) {
                            grid[curr_y * w + curr_x] = '#';
                        }
                        curr_x++;
                        if (curr_x >= w) { curr_x = 0; curr_y++; }
                    }
                    // A '1' bit indicates a continuation of RLE runs for this specific sequence
                } while (get_bits(&bs, 1) != 0 && curr_y < h);
            }
            
            // Write ASCII art to file
            for (int ty = 0; ty < h; ty++) {
                for (int tx = 0; tx < w; tx++) {
                    fputc(grid[ty * w + tx], f);
                }
                fputc('\n', f);
            }
            free(grid);
        }
        
        pos += sz; // Advance to next glyph record
    }
    
    fclose(f);
}

// Incorporate recovered font assets from git history
#include "../docs/fonts/fonts_recovered.h"

/**
 * @brief Entry point: decompiles all known authentic fonts for validation.
 */
int main() {
    decompile_font("NatRailSmall9", NatRailSmall9, sizeof(NatRailSmall9));
    decompile_font("NatRailTall12", NatRailTall12, sizeof(NatRailTall12));
    decompile_font("NatRailClockSmall7", NatRailClockSmall7, sizeof(NatRailClockSmall7));
    decompile_font("NatRailClockLarge9", NatRailClockLarge9, sizeof(NatRailClockLarge9));
    decompile_font("Underground10", Underground10, sizeof(Underground10));
    decompile_font("UndergroundClock8", UndergroundClock8, sizeof(UndergroundClock8));
    return 0;
}
