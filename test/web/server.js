const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = 3000;
const PORTAL_DIR = path.resolve(__dirname, '../../web');

const server = http.createServer((req, res) => {
  let filePath = path.join(PORTAL_DIR, req.url === '/' ? 'index.html' : req.url);
  
  // Basic static file server
  fs.readFile(filePath, (err, content) => {
    if (err) {
      // If it's an API call, we return 404 and let Playwright mock it
      if (req.url.startsWith('/api')) {
        if (req.url === '/api/config/backup') {
          res.writeHead(200, { 
            'Content-Type': 'application/json',
            'Content-Disposition': 'attachment; filename=config.json'
          });
          res.end(JSON.stringify({ version: 2.5, hostname: "MockBoard" }));
          return;
        }
        if (req.url === '/api/config/restore' && req.method === 'POST') {
          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end(JSON.stringify({ status: "ok" }));
          return;
        }
        if (req.url === '/api/status') {
          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end(JSON.stringify({
            status: "ok",
            version: "v3.0",
            build: "B20260405092908-8b8b496+mod",
            heap: 55000,      // Amber (>50k, <80k)
            total_heap: 120000,
            min_heap: 35000,  // Red (<50k)
            max_alloc: 25000, // Red (<32k)
            temp: 72.5,       // Red (>70)
            storage_used: 450000,
            storage_total: 512000, // ~88% -> Amber
            connected: true,
            ssid: "MockWiFi",
            rssi: -60,
            ip: "192.168.1.50",
            uptime: 3600
          }));
          return;
        }
        res.writeHead(404);
        res.end();
        return;
      }
      res.writeHead(500);
      res.end(`Error: ${err.code}`);
      return;
    }
    res.writeHead(200, { 'Content-Type': 'text/html' });
    res.end(content, 'utf-8');
  });
});

server.listen(PORT, () => {
  console.log(`Server running at http://localhost:${PORT}/`);
});
