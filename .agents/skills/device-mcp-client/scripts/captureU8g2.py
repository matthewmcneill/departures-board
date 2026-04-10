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

def decode_u8g2_monochrome(data, width, height):
    """
    Decodes a U8G2 tile-based monochrome buffer into a Pillow Image.
    Each tile is 8x8 pixels.
    Each byte in a tile is one column (8 vertical pixels).
    Bit 0 is the top pixel of the column.
    Layout: Tiles are stored row-major (Left-to-Right, then Top-to-Bottom).
    """
    img = Image.new('1', (width, height), 0) # '1' is 1-bit monochrome
    pixels = img.load()
    
    tiles_x = width // 8
    tiles_y = height // 8
    
    idx = 0
    # Iterate through tiles in row-major order
    for ty in range(tiles_y):
        for tx in range(tiles_x):
            # Each tile is 8 bytes (8 columns)
            for col in range(8):
                if idx >= len(data):
                    break
                byte = data[idx]
                idx += 1
                
                # Each bit in the byte is a pixel in the vertical column
                for row_bit in range(8):
                    pixel_val = (byte >> row_bit) & 0x01
                    # Global coordinates
                    px = (tx * 8) + col
                    py = (ty * 8) + row_bit
                    
                    if px < width and py < height:
                        pixels[px, py] = pixel_val
                        
    return img

def capture_u8g2(ip_address, output_path):
    url = f"http://{ip_address}/mcp"
    
    payload = {
        "jsonrpc": "2.0",
        "method": "tools/call",
        "params": {"name": "get_display_buffer"},
        "id": 1
    }
    
    data = json.dumps(payload).encode('utf-8')
    req = urllib.request.Request(url, data=data, headers={'Content-Type': 'application/json'}, method='POST')
    
    print(f"Requesting U8G2 framebuffer from {url}...")
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
    
    # 2048 bytes is the expected size for 256x64 monochrome (256*64/8)
    if buffer_len == 2048:
        width, height = 256, 64
    elif buffer_len == 1024:
        width, height = 128, 64
    else:
        # Fallback to 256x64 and warning
        width, height = 256, 64
        print(f"WARNING: Unexpected buffer size ({buffer_len}). Defaulting to 256x64.")

    print(f"Dimensions: {width}x{height}")
    
    img = decode_u8g2_monochrome(raw_buffer, width, height)
    img.save(output_path)
    print(f"Successfully saved U8G2 display capture to: {output_path}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Capture ESP32 U8G2 Display buffer via MCP")
    parser.add_argument("ip", help="Target ESP32 IP Address")
    parser.add_argument("output", help="Output PNG path", default="display_capture_u8g2.png", nargs="?")
    args = parser.parse_args()
    
    capture_u8g2(args.ip, args.output)
