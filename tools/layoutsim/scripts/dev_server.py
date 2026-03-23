import http.server
import socketserver
import os
import sys
import json
import glob

# Configuration
PORT = 8000
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
        if self.path == '/api/layouts':
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
        pattern = os.path.join(PROJECT_ROOT, "modules/displayManager/boards/*/layouts/*.json")
        files = glob.glob(pattern)
        if not files:
            return "tools/layoutsim/layout.json"
        
        # Sort by mtime descending
        latest_file = max(files, key=os.path.getmtime)
        return os.path.relpath(latest_file, PROJECT_ROOT)

def run():
    print(f"Starting Layout Simulator Dev Server on http://localhost:{PORT}")
    print(f"Project Root: {PROJECT_ROOT}")
    print("Direct link: http://localhost:8000/tools/layoutsim/web/index.html")
    
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down server.")
            sys.exit(0)

if __name__ == "__main__":
    run()
