const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = 3000;
const PORTAL_DIR = path.resolve(__dirname, '../../portal');

const server = http.createServer((req, res) => {
  let filePath = path.join(PORTAL_DIR, req.url === '/' ? 'index.html' : req.url);
  
  // Basic static file server
  fs.readFile(filePath, (err, content) => {
    if (err) {
      // If it's an API call, we return 404 and let Playwright mock it
      if (req.url.startsWith('/api')) {
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
