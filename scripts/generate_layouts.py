#!/usr/bin/env python3
import os
import subprocess
import glob

Import("env")

def generate_layouts(source, target, env):
    print("Auto-generating Layout C++ from JSON...")
    
    # Paths to search for layout JSONs
    search_paths = [
        "modules/displayManager/boards/nationalRailBoard/layouts/*.json",
        "modules/displayManager/boards/tflBoard/layouts/*.json",
        "modules/displayManager/boards/busBoard/layouts/*.json",
        "modules/displayManager/boards/systemBoard/layouts/*.json"
    ]
    
    script_path = "tools/layoutsim/scripts/gen_layout_cpp.py"
    
    for path_glob in search_paths:
        for json_file in glob.glob(path_glob):
            print(f"  Processing {json_file}...")
            subprocess.run(["python3", script_path, "--input", json_file], check=True)

# Add to the build loop
generate_layouts(None, None, env)
