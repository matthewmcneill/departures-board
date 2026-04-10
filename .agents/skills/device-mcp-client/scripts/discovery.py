#!/usr/bin/env python3
import os
import re
import sys

def discover_ip():
    """
    Parses the active pio-manager spool cache to locate the dynamically
    assigned DHCP IP address from the ESP32's boot sequence.
    """
    # Use repository-relative path to ensure portability
    log_path = "logs/latest-monitor.log"
    
    if not os.path.exists(log_path):
        print("ERROR: Spool log not found at logs/latest-monitor.log", file=sys.stderr)
        sys.exit(1)
        
    with open(log_path, "r", encoding="utf-8", errors="ignore") as f:
        # Read lines in reverse order since we want the most recent connection
        lines = f.readlines()
        
    # Regex to match Private IPv4 addresses (192.168.x.x, 10.x.x.x, 172.16-31.x.x)
    # The ESP32 usually prints something like "STA IP: 192.168.1.10" or "IPv4: ..."
    private_ip_pattern = re.compile(
        r"IP:\s*(192\.168\.\d{1,3}\.\d{1,3}|10\.\d{1,3}\.\d{1,3}\.\d{1,3}|172\.(?:1[6-9]|2\d|3[0-1])\.\d{1,3}\.\d{1,3})"
    )
    
    for line in reversed(lines):
        # We only want to match lines where the IP is explicitly logged as the device IP
        if "WIFI" in line and "IP:" in line or "Connected" in line:
            matches = private_ip_pattern.findall(line)
            if matches:
                # Return the first match from the end
                print(matches[-1])
                return matches[-1]
            
    print("ERROR: No valid private IP address found in logs.", file=sys.stderr)
    sys.exit(1)

if __name__ == "__main__":
    discover_ip()
