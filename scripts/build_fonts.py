"""
Departures Board (c) 2025-2026 Gadec Software

https://github.com/gadec-uk/departures-board

This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/

Module: scripts/build_fonts.py
Description: PlatformIO pre-script that automates the compilation of BDF font 
             sources into U8G2-compatible C++ arrays in fonts.cpp.
             Handles automatic compilation of the 'bdfconv' utility if missing.

Exported Functions:
- compile_bdfconv: Ensures the C-based bdfconv utility is compiled and ready.
- build_fonts: Coordinates the conversion of all .bdf files and merges them into fonts.cpp.
"""

import os
import subprocess
import glob

# PlatformIO 'env' instance is globally available when run as an extra_script
# But we need to import it if the environment is strict
try:
    from SCons.Script import Import
    Import("env")
except ImportError:
    # If run outside of SCons/PIO, we'd need a fallback, 
    # but this script is designed as an extra_script.
    pass

ROOT_DIR = env.get("PROJECT_DIR", os.getcwd())
BDFCONV_DIR = os.path.join(ROOT_DIR, "tools", "bdfconv")
BDFCONV_EXE = os.path.join(BDFCONV_DIR, "bdfconv")

FONTS_SOURCE_DIR = os.path.join(ROOT_DIR, "modules", "displayManager", "fonts", "source")
FONTS_DEST = os.path.join(ROOT_DIR, "modules", "displayManager", "fonts", "fonts.cpp")

def compile_bdfconv():
    """
    Checks for the presence of the bdfconv executable and compiles it from 
    source if necessary.
    
    Returns:
        bool: True if bdfconv is ready for use.
    """
    if not os.path.exists(BDFCONV_EXE):
        print("bdfconv executable not found. Compiling from source...")
        c_files = glob.glob(os.path.join(BDFCONV_DIR, "*.c"))
        if not c_files:
            print(f"Error: No bdfconv source files found in {BDFCONV_DIR}")
            return False
        
        # Determine the host compiler
        compiler = "gcc" if os.name == "posix" else "cl"
        
        cmd = [compiler] + c_files + ["-o", BDFCONV_EXE]
        try:
            subprocess.run(cmd, check=True)
            print("Successfully compiled bdfconv!")
            return True
        except subprocess.CalledProcessError as e:
            print(f"Error compiling bdfconv: {e}")
            return False
        except FileNotFoundError:
            print(f"Error: Compiler '{compiler}' not found. Cannot compile bdfconv.")
            return False
            
    return True

def build_fonts():
    """
    Scans the font source directory for BDF files and converts them into
    a single fonts.cpp destination. This involves invoking bdfconv for 
    each file and stripping/merging the results.
    """
    if not os.path.exists(FONTS_SOURCE_DIR):
        print(f"Notice: Font source directory {FONTS_SOURCE_DIR} not found. Skipping font build.")
        return
        
    bdf_files = glob.glob(os.path.join(FONTS_SOURCE_DIR, "*.bdf"))
    if not bdf_files:
        print("No .bdf files found. Skipping font build.")
        return
        
    # --- Step 1: Check if Rebuild is Required ---
    rebuild_needed = True
    if os.path.exists(FONTS_DEST):
        dest_mtime = os.path.getmtime(FONTS_DEST)
        newest_bdf_mtime = max(os.path.getmtime(f) for f in bdf_files)
        if newest_bdf_mtime <= dest_mtime:
            print("Fonts are up to date. Skipping rebuild.")
            rebuild_needed = False
            
    if not rebuild_needed:
        return

    # --- Step 2: Prepare bdfconv tool ---
    if not compile_bdfconv():
        return

    print("Rebuilding fonts.cpp from BDF sources...")
    
    # We will accumulate all generated arrays into one file
    temp_c_files = []
    
    try:
        # --- Step 3: Convert each BDF to a temp C array ---
        for bdf in bdf_files:
            font_name = os.path.splitext(os.path.basename(bdf))[0]
            temp_out = os.path.join(ROOT_DIR, f"temp_{font_name}.c")
            
            # bdfconv arguments:
            # -f 1       : Generate u8g2 font format
            # -m 0-65535 : Map all symbols (full coverage)
            # -n [name]  : C constant name
            cmd = [BDFCONV_EXE, "-f", "1", "-m", "0-65535", "-n", font_name, "-o", temp_out, bdf]
            subprocess.run(cmd, check=True, capture_output=True)
            temp_c_files.append(temp_out)
            print(f"Converted {font_name}...")
            
        # --- Step 4: Merge Temp Files into Final fonts.cpp ---
        with open(FONTS_DEST, "w") as out_f:
            out_f.write("/*\n")
            out_f.write(" * AUTOMATICALLY GENERATED FILE - DO NOT EDIT\n")
            out_f.write(" * Built from BDF sources by scripts/build_fonts.py\n")
            out_f.write(" */\n\n")
            out_f.write('#include <fonts/fonts.hpp>\n\n')
            
            for tf in temp_c_files:
                with open(tf, "r") as inf:
                    in_blocks = False
                    for line in inf:
                        # Extract only the uint8_t array definitions, skipping bdfconv headers
                        if "const uint8_t" in line:
                            in_blocks = True
                        if in_blocks:
                            out_f.write(line)
                out_f.write("\n")
                os.remove(tf) # cleanup temp file
                
        print(f"Successfully wrote {len(bdf_files)} fonts to {FONTS_DEST}")
        
    except Exception as e:
        print(f"Failed to convert fonts: {e}")
        # Ensure cleanup on failure
        for tf in temp_c_files:
            if os.path.exists(tf): os.remove(tf)

# Execute build
build_fonts()
