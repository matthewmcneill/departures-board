"""
Departures Board (c) 2025-2026 Gadec Software
Refactored for v3.0 by Matt McNeill 2026 CB Labs

https://github.com/gadec-uk/departures-board

This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/

Module: scripts/build_fonts.py
Description: PlatformIO pre-script that automates the compilation of BDF font 
             sources into U8G2-compatible C++ arrays in fonts.cpp.
             Handles automatic compilation of the 'bdfconv' utility if missing.
"""

import os
import glob
import sys
import subprocess
from txt_to_bdf import parse_font_txt, write_bdf

# PlatformIO 'env' instance is globally available when run as an extra_script
try:
    from SCons.Script import Import
    Import("env")
except ImportError:
    class MockEnv:
        def get(self, key, default):
            return os.environ.get(key, default)
    env = MockEnv()

ROOT_DIR = env.get("PROJECT_DIR", os.getcwd())
BDFCONV_DIR = os.path.join(ROOT_DIR, "tools", "bdfconv")
BDFCONV_EXE = os.path.join(BDFCONV_DIR, "bdfconv")

FONTS_SOURCE_DIR = os.path.join(ROOT_DIR, "modules", "displayManager", "fonts", "source")
FONTS_DEST = os.path.join(ROOT_DIR, "modules", "displayManager", "fonts", "fonts.cpp")

def compile_bdfconv():
    if not os.path.exists(BDFCONV_EXE):
        print("bdfconv executable not found. Compiling from source...")
        c_files = glob.glob(os.path.join(BDFCONV_DIR, "*.c"))
        if not c_files:
            print(f"Error: No bdfconv source files found in {BDFCONV_DIR}")
            return False
        
        compiler = "gcc" if os.name == "posix" else "cl"
        cmd = [compiler] + c_files + ["-o", BDFCONV_EXE]
        try:
            subprocess.run(cmd, check=True)
            print("Successfully compiled bdfconv!")
            return True
        except Exception as e:
            print(f"Error compiling bdfconv: {e}")
            return False
    return True

def build_fonts():
    if not os.path.exists(FONTS_SOURCE_DIR):
        print(f"Notice: Font source directory {FONTS_SOURCE_DIR} not found.")
        return

    # --- Step 1: Convert Visual Dumps (.txt) to BDF ---
    print("Checking for visual font dumps (.txt) to convert to BDF...")
    txt_files = glob.glob(os.path.join(FONTS_SOURCE_DIR, "*.txt"))
    for txt_file in txt_files:
        font_name = os.path.splitext(os.path.basename(txt_file))[0]
        bdf_path = os.path.join(FONTS_SOURCE_DIR, f"{font_name}.bdf")
        
        if not os.path.exists(bdf_path) or os.path.getmtime(txt_file) > os.path.getmtime(bdf_path):
            print(f"Converting dump {os.path.basename(txt_file)} -> BDF")
            res = parse_font_txt(txt_file)
            if res:
                fname, chars = res
                write_bdf(fname, chars, bdf_path)
            else:
                print(f"Error parsing {txt_file}")

    # --- Step 2: Check if Rebuild of fonts.cpp is Required ---
    bdf_files = glob.glob(os.path.join(FONTS_SOURCE_DIR, "*.bdf"))
    if not bdf_files:
        print("No .bdf files found. Skipping font build.")
        return

    dest_mtime = os.path.getmtime(FONTS_DEST) if os.path.exists(FONTS_DEST) else 0
    newest_bdf_mtime = max(os.path.getmtime(f) for f in bdf_files)
    
    if newest_bdf_mtime <= dest_mtime:
        print("Fonts are up to date. Skipping fonts.cpp rebuild.")
        return

    # --- Step 3: Prepare bdfconv tool ---
    if not compile_bdfconv():
        return

    print("Rebuilding fonts.cpp from BDF sources...")
    temp_c_files = []
    
    try:
        for bdf in bdf_files:
            font_name = os.path.splitext(os.path.basename(bdf))[0]
            temp_out = os.path.join(ROOT_DIR, f"temp_{font_name}.c")
            cmd = [BDFCONV_EXE, "-f", "1", "-m", "0-65535", "-n", font_name, "-o", temp_out, bdf]
            subprocess.run(cmd, check=True, capture_output=True)
            temp_c_files.append(temp_out)
            print(f"  -> Compiled {font_name}")
            
        with open(FONTS_DEST, "w") as out_f:
            out_f.write("/* AUTOMATICALLY GENERATED FILE - DO NOT EDIT */\n")
            out_f.write('#include <fonts/fonts.hpp>\n\n')
            for tf in temp_c_files:
                with open(tf, "r") as inf:
                    in_blocks = False
                    for line in inf:
                        if "const uint8_t" in line: in_blocks = True
                        if in_blocks: out_f.write(line)
                out_f.write("\n")
                os.remove(tf)
        print(f"Successfully wrote {len(bdf_files)} fonts to {FONTS_DEST}")
    except Exception as e:
        print(f"Failed to convert fonts: {e}")
        for tf in temp_c_files:
            if os.path.exists(tf): os.remove(tf)

if __name__ == "__main__":
    build_fonts()
