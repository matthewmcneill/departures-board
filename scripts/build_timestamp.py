#!/usr/bin/env python3
import datetime
import os
import subprocess

# Get current time in ByyyyMMddHHmmss format
now = datetime.datetime.now()
timestamp = "B" + now.strftime("%Y%m%d%H%M%S")

# Try to get git hash and check for local modifications (dirty tree)
try:
    git_hash = subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD'], stderr=subprocess.STDOUT).decode('ascii').strip()
    
    # Check for uncommitted changes
    # git diff --quiet returns 1 if there are changes
    is_dirty = subprocess.call(['git', 'diff', '--quiet']) != 0 or \
               subprocess.call(['git', 'diff', '--cached', '--quiet']) != 0
    
    dirty_suffix = "+mod" if is_dirty else ""
    full_serial = f"{timestamp}-{git_hash}{dirty_suffix}"
except Exception:
    full_serial = timestamp

# Define target paths
target_dir = os.path.join(os.getcwd(), "src")
target_file = os.path.join(target_dir, "buildTime.hpp")

# Ensure directory exists
os.makedirs(target_dir, exist_ok=True)

# Write the header file
with open(target_file, "w") as f:
    f.write("/*\n")
    f.write(" * Departures Board (c) 2025-2026 Gadec Software\n")
    f.write(" * Refactored for v3.0 by Matt McNeill 2026 CB Labs\n")
    f.write(" *\n")
    f.write(" * https://github.com/gadec-uk/departures-board\n")
    f.write(" *\n")
    f.write(" * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.\n")
    f.write(" * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/\n")
    f.write(" *\n")
    f.write(" * Module: src/buildTime.hpp\n")
    f.write(" * Description: Automated Build Timestamp Header generated during PlatformIO pre-build.\n")
    f.write(" *\n")
    f.write(" * Exported Functions/Classes:\n")
    f.write(" * - BUILD_TIME: String literal containing the B+YYYYMMDDHHMMSS+Hash format.\n")
    f.write(" * - BUILD_DATE_PRETTY: String literal with human-readable date block.\n")
    f.write(" */\n\n")
    f.write("#pragma once\n\n")
    f.write(f'#define BUILD_TIME "{full_serial}"\n')
    f.write(f'#define BUILD_DATE_PRETTY "{now.strftime("%Y-%m-%d")}"\n')

print(f"--- Build Serial: {full_serial} ---")
