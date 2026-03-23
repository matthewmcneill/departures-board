#!/usr/bin/env python3
"""
Departures Board (c) 2025-2026 Gadec Software
Refactored for v3.0 by Matt McNeill 2026 CB Labs

https://github.com/gadec-uk/departures-board

This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/

Module: tools/layoutsim/scripts/setup_wasm_env.py
Description: Clones and configures a local Emscripten SDK environment for compiling the simulator.
"""

import os
import subprocess
import sys

# Configuration
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
EMSDK_DIR = os.path.join(PROJECT_ROOT, "tools", "layoutsim", "emsdk")

def run_cmd(cmd, cwd=None):
    """
    Executes a shell command synchronously and exits the script on error.
    Args:
        cmd (list): Arguments array to pass to subprocess.
        cwd (str, optional): Target working directory for execution.
    """
    print(f"Executing: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd)
    if result.returncode != 0:
        print(f"Error: Command failed with exit code {result.returncode}")
        sys.exit(result.returncode)

def setup():
    """
    Orchestrates the git clone, source installation, and activation steps for the Emscripten SDK.
    """
    print("Setting up local Emscripten SDK environment...")
    
    # 1. Clone emsdk if missing
    if not os.path.exists(EMSDK_DIR):
        print(f"Cloning emsdk into {EMSDK_DIR}...")
        run_cmd(["git", "clone", "https://github.com/emscripten-core/emsdk.git", EMSDK_DIR])
    else:
        print("emsdk already exists. Updating...")
        run_cmd(["git", "pull"], cwd=EMSDK_DIR)

    # 2. Install latest
    print("Installing latest emscripten...")
    run_cmd([os.path.join(EMSDK_DIR, "emsdk"), "install", "latest"], cwd=EMSDK_DIR)

    # 3. Activate latest
    print("Activating latest emscripten...")
    run_cmd([os.path.join(EMSDK_DIR, "emsdk"), "activate", "latest"], cwd=EMSDK_DIR)

    print("\nSetup complete!")
    print("You can now run the build script. It will automatically detect this local installation.")

if __name__ == "__main__":
    setup()
