#!/usr/bin/env python3
import os
import gzip
import shutil

# --- Configuration ---
PORTAL_DIR = "portal"
OUTPUT_FILE = "include/webServer/portalAssets.h" # Updated to match implementation plan's logical path
ASSETS = ["index.html"] # We start with index.html as per Phase 1

def generate_assets():
    print(f"--- Portal Builder: Generating {OUTPUT_FILE} ---")
    
    if not os.path.exists(PORTAL_DIR):
        print(f"Error: {PORTAL_DIR} directory not found!")
        return

    # Ensure output directory exists
    os.makedirs(os.path.dirname(OUTPUT_FILE), exist_ok=True)

    with open(OUTPUT_FILE, "w") as f:
        f.write("/* Automated Portal Assets - DO NOT EDIT MANUALLY */\n")
        f.write("#pragma once\n\n")
        f.write("#include <Arduino.h>\n\n")

        for asset in ASSETS:
            asset_path = os.path.join(PORTAL_DIR, asset)
            if not os.path.exists(asset_path):
                print(f"Warning: Asset {asset_path} not found. Skipping.")
                continue

            print(f"Processing: {asset}...")
            
            # Gzip the content
            with open(asset_path, "rb") as f_in:
                content = f_in.read()
                gzipped_content = gzip.compress(content)

            # Generate C++ array name
            var_name = asset.replace(".", "_") + "_gz"
            
            # Write to header
            f.write(f"// Original: {asset} ({len(content)} bytes), Gzipped: ({len(gzipped_content)} bytes)\n")
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
