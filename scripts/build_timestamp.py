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
target_dir = os.path.join(os.getcwd(), "modules/systemManager")
target_file = os.path.join(target_dir, "build_time.h")

# Ensure directory exists
os.makedirs(target_dir, exist_ok=True)

# Write the header file
with open(target_file, "w") as f:
    f.write("/*\n")
    f.write(" * Automated Build Timestamp Header\n")
    f.write(f" * Generated on: {now.strftime('%Y-%m-%d %H:%M:%S')}\n")
    f.write(" */\n\n")
    f.write("#pragma once\n\n")
    f.write(f'#define BUILD_TIME "{full_serial}"\n')
    f.write(f'#define BUILD_DATE_PRETTY "{now.strftime("%Y-%m-%d")}"\n')

print(f"--- Build Serial: {full_serial} ---")
