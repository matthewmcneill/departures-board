#!/usr/bin/env python3
import sys
import json
import argparse
import urllib.request
import urllib.error

def mcp_request(ip_address, method_name):
    url = f"http://{ip_address}/mcp"
    payload = {
        "jsonrpc": "2.0",
        "method": "tools/call",
        "params": {"name": method_name},
        "id": 1
    }
    
    data = json.dumps(payload).encode('utf-8')
    req = urllib.request.Request(url, data=data, headers={'Content-Type': 'application/json'}, method='POST')
    
    try:
        with urllib.request.urlopen(req, timeout=5) as response:
            res_body = response.read().decode('utf-8')
    except urllib.error.URLError as e:
        return {"error": f"Connection Failed: {e}"}
        
    try:
        res_json = json.loads(res_body)
        if "error" in res_json:
            return {"error": res_json['error']}
            
        inner_text = res_json['result']['content'][0]['text']
        # The telemetry and network endpoints return a stringified JSON in the text field
        return json.loads(inner_text)
    except Exception as e:
        return {"error": f"Parse Error: {e}\nRaw Response: {res_body[:100]}"}

def run_diagnostics(ip_address):
    print(f"==========================================")
    print(f" HARDWARE DIAGNOSTICS: {ip_address}")
    print(f"==========================================\n")
    
    # 1. Telemetry
    print(">>> 1. SYSTEM TELEMETRY (get_system_telemetry)")
    telemetry = mcp_request(ip_address, "get_system_telemetry")
    if "error" in telemetry:
        print(f"    [FAIL] {telemetry['error']}\n")
    else:
        print(f"    - Free Heap: {telemetry.get('heap', 0) / 1024:.2f} KB")
        print(f"    - Max Alloc Block: {telemetry.get('max_alloc', 0) / 1024:.2f} KB")
        print(f"    - Core Temp: {telemetry.get('temp', 'N/A')} °C")
        print(f"    - Uptime: {telemetry.get('uptime', 0)} seconds\n")
        
    # 2. Configuration
    print(">>> 2. BOARD CONFIGURATION (get_configuration)")
    config = mcp_request(ip_address, "get_configuration")
    if "error" in config:
        print(f"    [FAIL] {config['error']}\n")
    else:
        print(f"    - Hostname: {config.get('hostname', 'Unknown')}")
        print(f"    - Target API: {config.get('targetApi', 'Unknown')}")
        print(f"    - Brightness: {config.get('brightness', 'Unknown')} / 255\n")
        
    # 3. Network
    print(">>> 3. NETWORK STATUS (get_network_status)")
    network = mcp_request(ip_address, "get_network_status")
    if "error" in network:
        print(f"    [FAIL] {network['error']}\n")
    else:
        print(f"    - SSID: {network.get('ssid', 'N/A')}")
        print(f"    - RSSI: {network.get('rssi', 'N/A')} dBm")
        print(f"    - Local IP: {network.get('ip', 'N/A')}")
        print(f"    - Connected: {'YES' if network.get('connected') else 'NO'}\n")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Query ESP32 diagnostic tree via MCP")
    parser.add_argument("ip", help="Target ESP32 IP Address")
    args = parser.parse_args()
    
    run_diagnostics(args.ip)
