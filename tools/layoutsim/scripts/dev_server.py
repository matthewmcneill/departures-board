"""
Departures Board (c) 2025-2026 Gadec Software
Refactored for v3.0 by Matt McNeill 2026 CB Labs

https://github.com/gadec-uk/departures-board

This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/

Module: tools/layoutsim/scripts/dev_server.py
Description: Simple HTTP server to run the Layout Simulator locally with hot-reloading support.
"""

import http.server
import socketserver
import os
import sys
import json
import glob
import logging
from datetime import datetime

# Configuration Constants
PORT = 8000 # HTTP listener port for the development server
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
TOOLS_DIR = os.path.dirname(SCRIPT_DIR)
PROJECT_ROOT = os.path.dirname(os.path.dirname(TOOLS_DIR))
BOARD_DIR = os.path.join(PROJECT_ROOT, "modules", "displayManager", "boards")

# Logging Configuration
TIMESTAMP = datetime.now().strftime("%y%m%d-%H%M%S")
LOG_DIR = os.path.join(PROJECT_ROOT, "logs")
os.makedirs(LOG_DIR, exist_ok=True)

LOG_FILE = os.path.join(LOG_DIR, f"simserver-{TIMESTAMP}.log")
LATEST_LOG = os.path.join(LOG_DIR, "simserver_latest.log")

# Configure logging: File (Detailed) and Console (Minimal)
file_handler = logging.FileHandler(LOG_FILE)
latest_handler = logging.FileHandler(LATEST_LOG, mode='w')
console_handler = logging.StreamHandler(sys.stdout)

# Detailed format for files
file_formatter = logging.Formatter('%(asctime)s [%(levelname)s] %(message)s')
file_handler.setFormatter(file_formatter)
latest_handler.setFormatter(file_formatter)

# Simple format for console
console_formatter = logging.Formatter('%(message)s')
console_handler.setFormatter(console_formatter)
console_handler.setLevel(logging.WARNING) # Silence console spam by default

logger = logging.getLogger("simserver")
logger.setLevel(logging.INFO)
logger.addHandler(file_handler)
logger.addHandler(latest_handler)
logger.addHandler(console_handler)

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=PROJECT_ROOT, **kwargs)

    def end_headers(self):
        """
        Add CORS headers to all responses.
        """
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'X-Requested-With, Content-Type')
        super().end_headers()

    def do_OPTIONS(self):
        """
        Handle preflight requests.
        """
        self.send_response(200)
        self.end_headers()

    def do_GET(self):
        """
        Intercepts GET requests to provide mock API routes for the simulator frontend.
        """
        if self.path == '/' or self.path == '/index.html':
            self.send_response(302)
            self.send_header('Location', '/tools/layoutsim/web/index.html')
            self.end_headers()
            return
        elif self.path == '/api/layouts':
            layouts = self.get_all_layouts()
            logger.info(f"[API] Found {len(layouts)} layouts")
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate, max-age=0')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Expires', '0')
            self.end_headers()
            self.wfile.write(json.dumps(layouts).encode())
            return
        elif self.path == '/api/integrity-check':
            current_hash = self.get_current_headers_hash()
            stored_hash = self.get_stored_registry_hash()
            status = "OK" if current_hash == stored_hash else "OUT_OF_SYNC"
            
            result = {
                "status": status,
                "current": current_hash,
                "stored": stored_hash
            }
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate, max-age=0')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Expires', '0')
            self.end_headers()
            self.wfile.write(json.dumps(result).encode())
            return
        elif self.path.startswith('/api/mock-data'):
            import urllib.parse
            query = urllib.parse.urlparse(self.path).query
            params = urllib.parse.parse_qs(query)
            layout_path = params.get('layout', [''])[0]
            
            # Map layout directory to mock data file
            mock_file = "nationalRailBoard.json"
            if "busBoard" in layout_path: mock_file = "busBoard.json"
            elif "tflBoard" in layout_path: mock_file = "tflBoard.json"
            elif "systemBoard" in layout_path: mock_file = "nationalRailBoard.json" # Fallback
            
            full_mock_path = os.path.join(PROJECT_ROOT, "tools/layoutsim/mock_data", mock_file)
            
            # Default to NR if no specific mock found
            if not os.path.exists(full_mock_path):
                full_mock_path = os.path.join(PROJECT_ROOT, "tools/layoutsim/mock_data", "nationalRailBoard.json")


            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate, max-age=0')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Expires', '0')
            self.end_headers()
            
            with open(full_mock_path, 'rb') as f:
                self.wfile.write(f.read())
            return
        elif self.path == '/api/latest-layout':
            latest = self.get_latest_layout()
            logger.info(f"[API] Latest layout: {latest}")
            self.send_response(200)
            self.send_header('Content-type', 'text/plain')
            self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate, max-age=0')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Expires', '0')
            self.end_headers()
            self.wfile.write(latest.encode())
            return
        else:
            # Default behavior: serve files from PROJECT_ROOT
            # logger.info(f"[HTTP] GET {self.path}")
            super().do_GET()

    def log_message(self, format, *args):
        """
        Redirect HTTP server logs to the file instead of stderr.
        """
        logger.info("%s - - [%s] %s" %
                         (self.address_string(),
                          self.log_date_time_string(),
                          format%args))

    def get_all_layouts(self):
        """
        Scans layout directories for JSON files.
        """
        layouts = []
        for board in os.listdir(BOARD_DIR):
            board_path = os.path.join(BOARD_DIR, board)
            if not os.path.isdir(board_path):
                continue
            layout_dir = os.path.join(board_path, "layouts")
            if os.path.exists(layout_dir):
                for f in os.listdir(layout_dir):
                    if f.endswith(".json"):
                        layouts.append({
                            "name": f"{board} - {f.replace('.json', '')}",
                            "path": os.path.join(layout_dir, f)
                        })
        return sorted(layouts, key=lambda x: x["name"])

    def get_current_headers_hash(self):
        """
        Calculates the MD5 hash of all layout headers and the registry script.
        Must match the logic in gen_sim_registry.py.
        """
        import hashlib
        import glob
        headers = sorted(glob.glob(os.path.join(BOARD_DIR, "*", "i*Layout.hpp")))
        hashes = []
        import re
        for header in headers:
            with open(header, "r") as f:
                content = f.read()
            # Match logic in gen_sim_registry.py: only hash if it contains a layout class
            if re.search(r"class\s+(i[A-Za-z0-9_]+Layout)\s*:", content):
                hashes.append(content)
        
        script_path = os.path.join(PROJECT_ROOT, "tools/layoutsim/scripts/gen_sim_registry.py")
        if os.path.exists(script_path):
            with open(script_path, "r") as f:
                hashes.append(f.read())
                
        combined = "".join(hashes).encode()
        return hashlib.md5(combined).hexdigest()

    def get_stored_registry_hash(self):
        """
        Reads the embedded hash from the generated C++ header.
        """
        import re
        registry_path = os.path.join(PROJECT_ROOT, "tools/layoutsim/src/generated_registry.hpp")
        if not os.path.exists(registry_path):
            return "MISSING"
            
        with open(registry_path, "r") as f:
            content = f.read()
            match = re.search(r'#define SIM_REGISTRY_HASH "([a-f0-9]+)"', content)
            if match:
                return match.group(1)
        return "UNKNOWN"

    def get_latest_layout(self):
        """
        Identifies the most recently modified layout JSON file to auto-load in the IDE.
        Returns:
            str: Relative path to the latest JSON layout file.
        """
        pattern = os.path.join(PROJECT_ROOT, "modules/displayManager/boards/*/layouts/*.json")
        files = glob.glob(pattern)
        if not files:
            return "modules/displayManager/boards/nationalRailBoard/layouts/layoutDefault.json"

        
        # Sort by mtime descending
        latest_file = max(files, key=os.path.getmtime)
        return os.path.relpath(latest_file, PROJECT_ROOT)

def run():
    """
    Initializes and blocks on the TCP socket server to run the development instance.
    """
    print(f"Starting Layout Simulator Dev Server on http://localhost:{PORT}")
    print(f"Project Root: {PROJECT_ROOT}")
    logger.info(f"--- Session Start: {TIMESTAMP} ---")
    logger.info(f"Logging to: {LOG_FILE}")
    
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down Layout Simulator server.")
            logger.info("Server shutdown via KeyboardInterrupt.")
            sys.exit(0)

if __name__ == "__main__":
    run()
