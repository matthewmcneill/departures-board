#!/usr/bin/env python3
import os
import gzip
import shutil
import re

# --- Configuration ---
PORTAL_DIR = "portal"
OUTPUT_FILE = "include/webServer/portalAssets.h" # Updated to match implementation plan's logical path
ASSETS = ["index.html"] # We start with index.html as per Phase 1

def minify_html(content):
    """
    Aggressively minify HTML by stripping comments and whitespace.
    """
    # 1. Remove HTML comments
    content = re.sub(r'<!--.*?-->', '', content, flags=re.DOTALL)
    
    # 2. Remove Multi-line comments in JS and CSS
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    
    # 3. Remove single-line comments (be careful with URLs)
    # Strategy: Remove // comments if they are at the start of a line or 
    # preceded by whitespace, but NOT prepended by a colon (to protect http://)
    content = re.sub(r'(?m)^\s*//.*$', '', content)
    content = re.sub(r'(?<!:)\/\/.*$', '', content, flags=re.MULTILINE)

    # 4. Strip whitespace from line ends and remove empty lines
    lines = [line.strip() for line in content.splitlines()]
    content = " ".join([line for line in lines if line])
    
    # 5. Replace multiple spaces with single space
    content = re.sub(r'\s+', ' ', content)
    
    return content.strip()

def generate_assets():
    print(f"--- Portal Builder: Generating {OUTPUT_FILE} ---")
    
    if not os.path.exists(PORTAL_DIR):
        print(f"Error: {PORTAL_DIR} directory not found!")
        return

    # Ensure output directory exists
    os.makedirs(os.path.dirname(OUTPUT_FILE), exist_ok=True)

    with open(OUTPUT_FILE, "w") as f:
        f.write("/*\n")
        f.write(" * Departures Board (c) 2025-2026 Gadec Software\n")
        f.write(" *\n")
        f.write(" * https://github.com/gadec-uk/departures-board\n")
        f.write(" *\n")
        f.write(" * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.\n")
        f.write(" * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/\n")
        f.write(" *\n")
        f.write(f" * Module: {OUTPUT_FILE}\n")
        f.write(" * Description: Automated Portal Assets - embedded gzipped binary data.\n *\n")
        f.write(" * Exported Functions/Classes:\n")
        for asset in ASSETS:
            var_name = asset.replace(".", "_") + "_gz"
            f.write(f" * - {var_name}: Gzipped content of {asset}\n")
            f.write(f" * - {var_name}_len: Length of {var_name}\n")
        f.write(" */\n\n")
        f.write("#pragma once\n\n")
        f.write("#include <Arduino.h>\n\n")

        for asset in ASSETS:
            asset_path = os.path.join(PORTAL_DIR, asset)
            if not os.path.exists(asset_path):
                print(f"Warning: Asset {asset_path} not found. Skipping.")
                continue

            print(f"Processing: {asset}...")
            
            # Minify and Gzip the content
            with open(asset_path, "r", encoding="utf-8") as f_in:
                content = f_in.read()
                minified_content = minify_html(content)
                gzipped_content = gzip.compress(minified_content.encode("utf-8"))

            # Generate C++ array name
            var_name = asset.replace(".", "_") + "_gz"
            
            # Write to header
            f.write(f"// Original: {asset} ({len(content)} bytes), Minified: {len(minified_content)} bytes, Gzipped: ({len(gzipped_content)} bytes)\n")
            f.write(f"const uint8_t {var_name}[] PROGMEM = {{\n    ")
            
            # Format as hex
            hex_data = [f"0x{b:02x}" for b in gzipped_content]
            for i, h in enumerate(hex_data):
                f.write(h)
                if i < len(hex_data) - 1:
                    f.write(", ")
                if (i + 1) % 12 == 0:
                    f.write("\n    ")
            
            f.write("\n};\n")
            f.write(f"const uint32_t {var_name}_len = {len(gzipped_content)};\n\n")

    print(f"Successfully generated {OUTPUT_FILE}")

if __name__ == "__main__":
    generate_assets()
