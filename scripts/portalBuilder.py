#!/usr/bin/env python3
"""
Departures Board (c) 2025-2026 Gadec Software
Refactored for v3.0 by Matt McNeill 2026 CB Labs

https://github.com/gadec-uk/departures-board

This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/

Module: scripts/portalBuilder.py
Description: Aggressively minifies and gzips web portal assets for embedding into ESP32 firmware.
             Ensures that index.html and rss.json are separated into .h/.cpp to stay in Flash (.rodata).

Exported Functions/Classes:
- should_rebuild(): Determines if assets need regeneration based on file timestamps.
- minify_html(content): Strips comments and whitespace for aggressive size reduction.
- generate_assets(): Main entry point to orchestrate minification, compression, and global compilation files.
"""
import os
import gzip
import shutil
import re
import datetime

# --- Configuration ---
try:
    from SCons.Script import Import
    Import("env")
except ImportError:
    class MockEnv:
        def get(self, key, default):
            return os.environ.get(key, default)
    env = MockEnv()

ROOT_DIR = env.get("PROJECT_DIR", os.getcwd())
PORTAL_DIR = os.path.join(ROOT_DIR, "web")
OUTPUT_H = os.path.join(ROOT_DIR, "modules/webServer/portalAssets.h")
OUTPUT_CPP = os.path.join(ROOT_DIR, "modules/webServer/portalAssets.cpp")
ASSETS = ["index.html", "rss.json", "screenshot.html"] 

def should_rebuild():
    if not os.path.exists(OUTPUT_H) or not os.path.exists(OUTPUT_CPP):
        return True
    
    dest_mtime = min(os.path.getmtime(OUTPUT_H), os.path.getmtime(OUTPUT_CPP))
    for asset in ASSETS:
        asset_path = os.path.join(PORTAL_DIR, asset)
        if os.path.exists(asset_path) and os.path.getmtime(asset_path) > dest_mtime:
            return True
    return False

def minify_html(content):
    """
    Aggressively minify HTML by stripping comments and whitespace.
    """
    # 1. Remove HTML comments
    content = re.sub(r'<!--.*?-->', '', content, flags=re.DOTALL)
    
    # 2. Remove Multi-line comments in JS and CSS
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    
    # 3. Remove single-line comments (be careful with URLs)
    content = re.sub(r'(?m)^\s*//.*$', '', content)
    content = re.sub(r'(?<!:)\/\/.*$', '', content, flags=re.MULTILINE)

    # 4. Strip whitespace from line ends and remove empty lines
    lines = [line.strip() for line in content.splitlines()]
    content = " ".join([line for line in lines if line])
    
    # 5. Replace multiple spaces with single space
    content = re.sub(r'\s+', ' ', content)
    
    return content.strip()

def generate_assets():
    # Force rebuild so we test the new logic once
    if not should_rebuild() and os.environ.get("FORCE_REBUILD", "0") == "0":
        print("Portal assets are up to date. Skipping rebuild.")
        return

    print(f"--- Portal Builder: Generating {OUTPUT_H} and {OUTPUT_CPP} ---")
    
    if not os.path.exists(PORTAL_DIR):
        print(f"Error: {PORTAL_DIR} directory not found!")
        return

    os.makedirs(os.path.dirname(OUTPUT_H), exist_ok=True)

    h_file = open(OUTPUT_H, "w")
    cpp_file = open(OUTPUT_CPP, "w")

    header_comment = """/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: Portal Assets
 * Description: Automated Portal Assets - embedded gzipped binary data.
 * Exported cleanly via .h/.cpp to remain in ESP32 Flash (.rodata) and conserve RAM.
 */
"""
    h_file.write(header_comment)
    h_file.write("#pragma once\n\n#include <Arduino.h>\n\n")
    
    cpp_file.write(header_comment)
    cpp_file.write('#include "portalAssets.h"\n\n')

    for asset in ASSETS:
        asset_path = os.path.join(PORTAL_DIR, asset)
        if not os.path.exists(asset_path):
            print(f"Warning: Asset {asset_path} not found. Skipping.")
            continue

        print(f"Processing: {asset}...")
        
        with open(asset_path, "r", encoding="utf-8") as f_in:
            content = f_in.read()
            
            if asset == "index.html":
                serial = datetime.datetime.now().strftime("%Y-%m-%d")
                header_path = os.path.join(ROOT_DIR, "src/buildTime.hpp")
                if os.path.exists(header_path):
                    with open(header_path, "r") as hf:
                        hcontent = hf.read()
                        match = re.search(r'#define BUILD_TIME "(.*?)"', hcontent)
                        if match:
                            serial = match.group(1)
                
                content = re.sub(r'<p id="fw-build-text">Build: [^<]+</p>', 
                               f'<p id="fw-build-text">Build: {serial}</p>', 
                               content)
                print(f"Patched build serial in index.html to: {serial}")

            minified_content = minify_html(content)
            gzipped_content = gzip.compress(minified_content.encode("utf-8"))

        var_name = asset.replace(".", "_") + "_gz"
        
        h_file.write(f"// Original: {asset} ({len(content)} bytes), Minified: {len(minified_content)} bytes, Gzipped: ({len(gzipped_content)} bytes)\n")
        h_file.write(f"extern const uint8_t {var_name}[] __attribute__((section(\".rodata\")));\n")
        h_file.write(f"extern const uint32_t {var_name}_len __attribute__((section(\".rodata\")));\n\n")

        cpp_file.write(f"const uint8_t {var_name}[] __attribute__((section(\".rodata\"))) = {{\n    ")
        
        hex_data = [f"0x{b:02x}" for b in gzipped_content]
        for i, h in enumerate(hex_data):
            cpp_file.write(h)
            if i < len(hex_data) - 1:
                cpp_file.write(", ")
            if (i + 1) % 12 == 0:
                cpp_file.write("\n    ")
        
        cpp_file.write("\n};\n")
        cpp_file.write(f"const uint32_t {var_name}_len __attribute__((section(\".rodata\"))) = {len(gzipped_content)};\n\n")

    h_file.close()
    cpp_file.close()
    print(f"Successfully generated {OUTPUT_H} and {OUTPUT_CPP}")

generate_assets()

if __name__ == "__main__":
    pass
