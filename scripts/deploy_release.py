#!/usr/bin/env python3
# scripts/deploy_release.py
# 
# Departures Board (c) 2025-2026 Gadec Software
# Refactored for v3.0 by Matt McNeill 2026 CB Labs
#
# Generates firmware binaries, securely signs them locally via OpenSSL, 
# and synchronizes the updates with the public GitHub repository.

import os
import sys
import subprocess

# Set target environment globally based on context_bridge
BUILD_ENV = "esp32dev"

def run_cmd_capture(args):
    """Runs a command and returns the stripped output, exiting on failure."""
    res = subprocess.run(args, capture_output=True, text=True)
    if res.returncode != 0:
        # Ignore minor errors if we're just checking versions, 
        # but returning empty is safer for the caller to handle.
        return ""
    return res.stdout.strip()

def get_latest_github_release():
    print("Fetching latest release from GitHub...")
    # Assumes gh cli is installed and authenticated
    out = run_cmd_capture(['gh', 'release', 'view', '--json', 'tagName', '-q', '.tagName'])
    if not out:
        print("Could not fetch remote version, perhaps no releases exist yet? Assuming v0.0.0")
        return "v0.0.0"
    return out

def main():
    if not os.path.exists("private_key.pem"):
        print("Error: private_key.pem not found. Run ./scripts/generate_keys.sh first.")
        sys.exit(1)

    latest_tag = get_latest_github_release()
    print(f"Latest GitHub Release: {latest_tag}")
    
    new_tag = input("Enter the new version tag (e.g. v1.1): ")
    if new_tag == latest_tag or not new_tag.startswith('v'):
        print(f"Error: New tag ({new_tag}) must be strictly incremental and formatted with a 'v'.")
        sys.exit(1)

    print(f"\nBuilding firmware via PlatformIO targeting {BUILD_ENV}...")
    subprocess.run(['pio', 'run', '-e', BUILD_ENV], check=True)
    
    bin_path = f".pio/build/{BUILD_ENV}/firmware.bin"
    if not os.path.exists(bin_path):
        print(f"Error: {bin_path} not found.")
        sys.exit(1)
        
    print("\nSigning firmware.bin...")
    print("Please enter the AES-256 password you selected during key generation:")
    
    # We do not capture output here so the terminal can natively prompt for the openssl pass phrase
    cmd = ['openssl', 'dgst', '-sha256', '-sign', 'private_key.pem', '-out', 'firmware.sig', bin_path]
    subprocess.run(cmd, check=True)
    
    if not os.path.exists('firmware.sig'):
        print("Error: Firmware signing failed.")
        sys.exit(1)
        
    print(f"\nCreating GitHub Release {new_tag} and uploading assets...")
    subprocess.run(['gh', 'release', 'create', new_tag, '--generate-notes', bin_path, 'firmware.sig'], check=True)
    
    print("\nRelease deployment successful! Remote edge nodes will fetch this during their next quiet-hour synchronization.")

if __name__ == "__main__":
    main()
