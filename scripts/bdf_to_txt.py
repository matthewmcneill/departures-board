import sys
import os

def bdf_to_txt(bdf_file, txt_file):
    with open(bdf_file, 'r') as f:
        lines = [line.strip() for line in f.readlines()]
    
    font_name = "Unknown"
    for line in lines:
        if line.startswith("FONT "):
            font_name = line.split(" ", 1)[1]
            break

    with open(txt_file, 'w') as out_f:
        out_f.write(f"Font: {font_name}\n\n")

        i = 0
        while i < len(lines):
            line = lines[i]
            if line.startswith("STARTCHAR"):
                char_name = line.split(" ", 1)[1]
                # Default values
                encoding = 0
                dx = 0
                w, h, x_off, y_off = 0, 0, 0, 0
                bitmap_lines = []
                
                # Parse char block
                while i < len(lines):
                    i += 1
                    sub_line = lines[i]
                    if sub_line.startswith("ENCODING"):
                        parts = sub_line.split()
                        if len(parts) > 1:
                            encoding = int(parts[1])
                    elif sub_line.startswith("DWIDTH"):
                        parts = sub_line.split()
                        if len(parts) > 1:
                            dx = int(parts[1])
                    elif sub_line.startswith("BBX"):
                        parts = sub_line.split()
                        if len(parts) >= 5:
                            w = int(parts[1])
                            h = int(parts[2])
                            x_off = int(parts[3])
                            y_off = int(parts[4])
                    elif sub_line == "BITMAP":
                        for _ in range(h):
                            i += 1
                            bitmap_lines.append(lines[i])
                    elif sub_line == "ENDCHAR":
                        break
                
                # Write to txt
                if encoding >= 0:
                    char_repr = repr(chr(encoding)) if encoding < 1114112 else '?'
                    out_f.write(f"Character U+{encoding:04X} (Char: {char_repr})\n")
                    out_f.write(f"Width: {w}, Height: {h}, X-Offset: {x_off}, Y-Offset: {y_off}, dX: {dx}\n")
                    
                    for hex_str in bitmap_lines:
                        # Convert hex to binary string
                        if hex_str:
                            # BDF pad to nearest byte (8 bits)
                            byte_len = len(hex_str) // 2
                            bin_str = ""
                            for j in range(byte_len):
                                byte_val = int(hex_str[j*2:j*2+2], 16)
                                bin_str += f"{byte_val:08b}"
                            
                            # Trim to width w
                            bin_str = bin_str[:w]
                            
                            # Convert to ASCII art
                            ascii_str = bin_str.replace('0', '.').replace('1', '#')
                            out_f.write(f"{ascii_str}\n")
                        else:
                            out_f.write("\n")
                    out_f.write("\n")
            i += 1

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python bdf_to_txt.py <input.bdf> <output.txt>")
        sys.exit(1)
    
    bdf_to_txt(sys.argv[1], sys.argv[2])
