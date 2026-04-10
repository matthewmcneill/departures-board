#!/usr/bin/env python3
import sys
import json
import base64
import argparse
import urllib.request
import urllib.error

try:
    from PIL import Image
except ImportError:
    print("ERROR: Pillow library is not installed. Please run: python3 -m pip install Pillow", file=sys.stderr)
    sys.exit(1)

def decode_rgb565(data, width, height):
    """
    Decodes a raw byte array of RGB565 data into a Pillow Image.
    RGB565 stores a pixel in 2 bytes:
    Byte 0: RRRRRGGG
    Byte 1: GGGBBBBB
    NOTE: Endianness matters. ESP32 is usually little-endian, but SPI / RGB buffers
    are sometimes in network byte order. We will assume big-endian natively (standard 16-bit packed)
    where High Byte = RR RRR GG G, Low Byte = GG G BB BBB.
    """
    img = Image.new('RGB', (width, height))
    pixels = img.load()
    
    idx = 0
    for y in range(height):
        for x in range(width):
            if idx + 1 >= len(data):
                break
            
            # Read 16 bits (assuming little endian representation in memory commonly)
            # You might need to swap this to `(data[idx] << 8) | data[idx+1]` depending on hardware
            # Defaulting to Little Endian for ESP32 framebuffer
            pixel = (data[idx+1] << 8) | data[idx]
            
            # Extract RGB
            r = (pixel >> 11) & 0x1F
            g = (pixel >> 5) & 0x3F
            b = pixel & 0x1F
            
            # Scale to 8-bit (0-255)
            r = (r * 255) // 31
            g = (g * 255) // 63
            b = (b * 255) // 31
            
            pixels[x, y] = (r, g, b)
            idx += 2
            
    return img

def capture_display(ip_address, output_path):
    url = f"http://{ip_address}/mcp"
    
    payload = {
        "jsonrpc": "2.0",
        "method": "tools/call",
        "params": {"name": "get_display_buffer"},
        "id": 1
    }
    
    data = json.dumps(payload).encode('utf-8')
    req = urllib.request.Request(url, data=data, headers={'Content-Type': 'application/json'}, method='POST')
    
    print(f"Requesting framebuffer from {url}...")
    try:
        with urllib.request.urlopen(req, timeout=10) as response:
            res_body = response.read().decode('utf-8')
    except urllib.error.URLError as e:
        print(f"ERROR: Failed to connect to device: {e}", file=sys.stderr)
        sys.exit(1)
        
    try:
        res_json = json.loads(res_body)
        if "error" in res_json:
            print(f"RPC Error from device: {res_json['error']}", file=sys.stderr)
            sys.exit(1)
            
        b64_str = res_json['result']['content'][0]['text']
    except Exception as e:
        print(f"ERROR: Failed to parse valid MCP envelope. Response snippet: {res_body[:100]}", file=sys.stderr)
        sys.exit(1)
        
    print(f"Decoding {len(b64_str)} bytes of Base64...")
    raw_buffer = base64.b64decode(b64_str)
    
    buffer_len = len(raw_buffer)
    print(f"Decoded binary framebuffer size: {buffer_len} bytes")
    
    # Infer Display Dimensions based on known hardware matrices
    width, height = 256, 64 # HUB75 usually 256x64 (32768 bytes)
    
    if buffer_len == 32768:
        width, height = 256, 64
    elif buffer_len == 16384:
        width, height = 128, 64
    elif buffer_len == 8192:
        width, height = 128, 32
    elif buffer_len == 153600:
        width, height = 320, 240
    else:
        print(f"WARNING: Unknown buffer size ({buffer_len}). Assuming 256x64, output may be skewed.")

    print(f"Guessed dimensions: {width}x{height}")
    
    img = decode_rgb565(raw_buffer, width, height)
    img.save(output_path)
    print(f"Successfully saved display capture to: {output_path}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Capture ESP32 Display buffer via MCP")
    parser.add_argument("ip", help="Target ESP32 IP Address")
    parser.add_argument("output", help="Output PNG path", default="display_capture.png", nargs="?")
    args = parser.parse_args()
    
    capture_display(args.ip, args.output)
