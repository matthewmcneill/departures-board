#!/usr/bin/env python3
import os
import subprocess
import sys
import shutil

# Configuration
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
TOOLS_DIR = os.path.join(PROJECT_ROOT, "tools", "layoutsim")
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
    ] + module_includes

    cpp_flags = ["-std=c++17"] + common_flags
    c_flags = ["-std=gnu99"] + common_flags

    # Identify sources
    cpp_sources = [
        os.path.join(SRC_DIR, "main.cpp"),
        os.path.join(SRC_DIR, "Arduino.cpp"),
        os.path.join(WIDGETS_DIR, "drawingPrimitives.cpp"),
        os.path.join(WIDGETS_DIR, "clockWidget.cpp"),
        os.path.join(WIDGETS_DIR, "headerWidget.cpp"),
        os.path.join(WIDGETS_DIR, "serviceListWidget.cpp"),
        os.path.join(WIDGETS_DIR, "scrollingTextWidget.cpp"),
        os.path.join(WIDGETS_DIR, "scrollingMessagePoolWidget.cpp"),
        os.path.join(WIDGETS_DIR, "labelWidget.cpp"),
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

    # Compile C++ files
    print("Compiling C++ files...")
    for src in cpp_sources:
        obj = os.path.join(BUILD_DIR, os.path.basename(src) + ".o")
        if not os.path.exists(obj) or os.path.getmtime(src) > os.path.getmtime(obj):
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
