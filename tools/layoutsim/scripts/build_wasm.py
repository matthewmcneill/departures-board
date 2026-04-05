#!/usr/bin/env python3
"""
Departures Board (c) 2025-2026 Gadec Software
Refactored for v3.0 by Matt McNeill 2026 CB Labs

https://github.com/gadec-uk/departures-board

This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/

Module: tools/layoutsim/scripts/build_wasm.py
Description: Compiles the C++ Layout Simulator engine into WebAssembly via Emscripten.
"""

import os
import subprocess
import sys
import shutil

# Configuration Constants
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..")) # Path to the main repository root
TOOLS_DIR = os.path.join(PROJECT_ROOT, "tools", "layoutsim") # Simulator sub-project root
SRC_DIR = os.path.join(TOOLS_DIR, "src")
WEB_DIR = os.path.join(TOOLS_DIR, "web")
DIST_DIR = os.path.join(WEB_DIR, "dist")
BUILD_DIR = os.path.join(TOOLS_DIR, "build")

WIDGETS_DIR = os.path.join(PROJECT_ROOT, "modules", "displayManager", "widgets")
FONTS_DIR = os.path.join(PROJECT_ROOT, "modules", "displayManager", "fonts")
U8G2_LIB_DIR = os.path.join(PROJECT_ROOT, ".pio", "libdeps", "esp32dev", "U8g2", "src")
EMSDK_DIR = os.path.join(TOOLS_DIR, "emsdk")

# Ensure directories exist
os.makedirs(DIST_DIR, exist_ok=True)
os.makedirs(BUILD_DIR, exist_ok=True)

def check_emcc():
    """
    Locates the Emscripten compiler executable on the host system.
    Returns:
        str: Path to the emcc executable, or None if not found.
    """
    try:
        subprocess.run(["emcc", "--version"], capture_output=True, check=True)
        return "emcc"
    except (subprocess.CalledProcessError, FileNotFoundError):
        pass
    local_emcc = os.path.join(EMSDK_DIR, "upstream", "emscripten", "emcc")
    if os.path.exists(local_emcc):
        return local_emcc
    return None

def build():
    """
    Executes the full WASM build pipeline, including registry generation, object compilation, and linking.
    """
    emcc_path = check_emcc()
    if not emcc_path:
        print("Error: 'emcc' not found.")
        sys.exit(1)

    print(f"Using emcc at: {emcc_path}")
    print("Building Layout Simulator WASM Engine...")

    # Generate the simulator registry
    print("Auto-generating Simulator Registry from C++ Headers...")
    subprocess.run([sys.executable, os.path.join(TOOLS_DIR, "scripts", "gen_sim_registry.py")], check=True)

    # Find all subdirectories in modules/ for include paths
    modules_root = os.path.join(PROJECT_ROOT, "modules")
    module_includes = [f"-I{modules_root}"]
    for root, dirs, files in os.walk(modules_root):
        if any(f.endswith('.hpp') or f.endswith('.h') for f in files):
            module_includes.append(f"-I{root}")

    common_flags = [
        "-O3",
        f"-I{SRC_DIR}",
        f"-I{U8G2_LIB_DIR}",
        f"-I{os.path.join(U8G2_LIB_DIR, 'clib')}",
        f"-I{os.path.join(PROJECT_ROOT, '.pio/libdeps/esp32dev/ArduinoJson/src')}",
        f"-I{os.path.join(PROJECT_ROOT, 'src')}",
        f"-I{WIDGETS_DIR}",
        f"-I{FONTS_DIR}",
        "-D__EMSCRIPTEN__",
        "-DARDUINO=100",
        "-DARDUINOJSON_ENABLE_ARDUINO_STRING=0",
        "-DARDUINOJSON_ENABLE_ARDUINO_STREAM=0",
        "-DARDUINOJSON_ENABLE_ARDUINO_PRINT=0",
    ] + module_includes + [f"-I{os.path.join(PROJECT_ROOT, 'test', 'mocks')}"]

    cpp_flags = ["-std=c++17"] + common_flags
    c_flags = ["-std=gnu99"] + common_flags

    # Identify sources
    cpp_sources = [
        os.path.join(SRC_DIR, "main.cpp"),
        os.path.join(SRC_DIR, "Arduino.cpp"),
        os.path.join(WIDGETS_DIR, "drawingPrimitives.cpp"),
        os.path.join(WIDGETS_DIR, "clockWidget.cpp"),
        os.path.join(WIDGETS_DIR, "serviceListWidget.cpp"),
        os.path.join(WIDGETS_DIR, "scrollingTextWidget.cpp"),
        os.path.join(WIDGETS_DIR, "scrollingMessagePoolWidget.cpp"),
        os.path.join(WIDGETS_DIR, "labelWidget.cpp"),
        os.path.join(WIDGETS_DIR, "trainFormationWidget.cpp"),
        os.path.join(WIDGETS_DIR, "locationAndFiltersWidget.cpp"),
        os.path.join(WIDGETS_DIR, "weatherWidget.cpp"),
        os.path.join(WIDGETS_DIR, "wifiStatusWidget.cpp"),
        os.path.join(PROJECT_ROOT, "modules/displayManager/messaging/messagePool.cpp"),
        os.path.join(FONTS_DIR, "fonts.cpp"),
        os.path.join(U8G2_LIB_DIR, "U8g2lib.cpp"),
        os.path.join(U8G2_LIB_DIR, "U8x8lib.cpp"),
    ]
    
    c_sources = []
    u8g2_clib = os.path.join(U8G2_LIB_DIR, "clib")
    for f in os.listdir(u8g2_clib):
        if f.endswith(".c"):
            c_sources.append(os.path.join(u8g2_clib, f))

    objects = []

    # Compile C files
    print("Compiling C files...")
    for src in c_sources:
        obj = os.path.join(BUILD_DIR, os.path.basename(src) + ".o")
        if not os.path.exists(obj) or os.path.getmtime(src) > os.path.getmtime(obj):
            cmd = [emcc_path] + c_flags + ["-c", src, "-o", obj]
            subprocess.run(cmd, check=True)
        objects.append(obj)

    # Collect max modification time of all headers to invalidate cache robustly 
    max_header_mtime = 0
    header_search_dirs = [modules_root, SRC_DIR, os.path.join(PROJECT_ROOT, "test", "mocks")]
    for search_dir in header_search_dirs:
        for root, dirs, files in os.walk(search_dir):
            for f in files:
                if f.endswith(('.hpp', '.h')):
                    mtime = os.path.getmtime(os.path.join(root, f))
                    if mtime > max_header_mtime:
                        max_header_mtime = mtime

    # Compile C++ files
    print("Compiling C++ files...")
    registry_hpp = os.path.join(SRC_DIR, "generated_registry.hpp")
    for src in cpp_sources:
        obj = os.path.join(BUILD_DIR, os.path.basename(src) + ".o")
        
        # Invalidate cache if the source file is updated, OR if any header was updated
        needs_build = not os.path.exists(obj) or os.path.getmtime(src) > os.path.getmtime(obj) or max_header_mtime > os.path.getmtime(obj)
        
        if needs_build:
            cmd = [emcc_path] + cpp_flags + ["-c", src, "-o", obj]
            print(f"Executing: {' '.join(cmd)}")
            subprocess.run(cmd, check=True)
        objects.append(obj)

    # Link everything
    print("Linking WASM Engine...")
    output = os.path.join(DIST_DIR, "layout_engine.js")
    
    link_cmd = [
        emcc_path,
        "-o", output,
        "-s", "MODULARIZE=1",
        "-s", "EXPORT_NAME='createLayoutEngine'",
        "-s", "EXPORTED_RUNTIME_METHODS=['ccall','cwrap','HEAPU8']",
        "-s", "EXPORTED_FUNCTIONS=['_initEngine','_renderFrame','_applyLayout','_applyMockData','_setDebugMode','_getLayoutMetadata','_malloc','_free']",
        "-s", "ALLOW_MEMORY_GROWTH=1",
        "-O3"
    ] + objects
    
    print(f"Executing link command...")
    subprocess.run(link_cmd, check=True)

    print("Build successful!")

if __name__ == "__main__":
    try:
        build()
    except subprocess.CalledProcessError as e:
        print(f"Error: Build failed during command execution.")
        sys.exit(1)
