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

# Configuration Constants
PORT = 8001 # HTTP listener port for the development server
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
TOOLS_DIR = os.path.dirname(SCRIPT_DIR)
PROJECT_ROOT = os.path.dirname(os.path.dirname(TOOLS_DIR))

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=PROJECT_ROOT, **kwargs)

    def end_headers(self):
        # Disable caching for hot-reloading
        self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate, max-age=0')
        self.send_header('Pragma', 'no-cache')
        self.send_header('Expires', '0')
        super().end_headers()

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
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            layouts = self.get_all_layouts()
            self.wfile.write(json.dumps(layouts).encode())
        elif self.path == '/api/latest-layout':
            self.send_response(200)
            self.send_header('Content-header', 'text/plain')
            self.end_headers()
            latest = self.get_latest_layout()
            self.wfile.write(latest.encode())
        else:
            # Default behavior: serve files from PROJECT_ROOT
            super().do_GET()

    def get_all_layouts(self):
        """
        Scans the project directory for custom JSON board layouts.
        Returns:
            list: A collection of layout dictionaries containing formatting names and paths.
        """
        pattern = os.path.join(PROJECT_ROOT, "modules/displayManager/boards/*/layouts/*.json")
        files = glob.glob(pattern)
        results = []
        for f in sorted(files):
            rel_path = os.path.relpath(f, PROJECT_ROOT)
            # Make a pretty name e.g. "nationalRailBoard - layoutDefault"
            parts = rel_path.split(os.sep)
            board = parts[3]
            layout = os.path.splitext(parts[5])[0]
            results.append({
                "name": f"{board} - {layout}",
                "path": rel_path
            })
        return results

    def get_latest_layout(self):
        """
        Identifies the most recently modified layout JSON file to auto-load in the IDE.
        Returns:
            str: Relative path to the latest JSON layout file.
        """
        pattern = os.path.join(PROJECT_ROOT, "modules/displayManager/boards/*/layouts/*.json")
        files = glob.glob(pattern)
        if not files:
            return "tools/layoutsim/layout.json"
        
        # Sort by mtime descending
        latest_file = max(files, key=os.path.getmtime)
        return os.path.relpath(latest_file, PROJECT_ROOT)

def run():
    """
    Initializes and blocks on the TCP socket server to run the development instance.
    """
    print(f"Starting Layout Simulator Dev Server on http://localhost:{PORT}")
    print(f"Project Root: {PROJECT_ROOT}")
    print("Open in browser: http://localhost:8000/")
    
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down server.")
            sys.exit(0)

if __name__ == "__main__":
    run()
