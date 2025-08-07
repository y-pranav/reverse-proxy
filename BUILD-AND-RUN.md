# Reverse Proxy - Build and Run Guide

## Build Commands

### Windows
```cmd
# Clean up previous builds
taskkill /f /im reverse_proxy.exe 2>nul
del reverse_proxy.exe *.o 2>nul

# Build with configuration management
g++ -std=c++17 -I include src/Logger.cpp src/LoadBalancer.cpp src/Config.cpp src/Server.cpp src/main.cpp -lws2_32 -o reverse_proxy.exe
```

### Linux
```bash
# Clean up previous builds
pkill reverse_proxy 2>/dev/null
rm -f reverse_proxy *.o

# Build with configuration management
g++ -std=c++17 -I include src/Logger.cpp src/LoadBalancer.cpp src/Config.cpp src/Server.cpp src/main.cpp -o reverse_proxy
```

## Run Commands

### Basic Usage
```cmd
# Windows - use default config.json
.\reverse_proxy.exe

# Linux - use default config.json
./reverse_proxy

# Windows - use custom config
.\reverse_proxy.exe config-weighted.json

# Linux - use custom config
./reverse_proxy config-least-connections.json
```

## Configuration Files

The server supports multiple configuration files for different load balancing scenarios:

- `config.json` - Round-robin load balancing
- `config-weighted.json` - Weighted round-robin
- `config-least-connections.json` - Least connections algorithm

## Test Commands

### Basic Testing
```cmd
# Test basic functionality
curl http://localhost:8888/
curl http://localhost:8888/api/users
curl -X POST http://localhost:8888/api/login

# Test multiple requests to see load balancing
for /l %i in (1,1,10) do curl http://localhost:8888/test%i
```

### Load Testing
```bash
# Linux load testing
for i in {1..20}; do curl http://localhost:8888/test$i & done; wait
```

## Backend Server Setup

### Simple Python Backends
```cmd
# Terminal 1 - Backend on port 3000
python -c "import http.server; http.server.HTTPServer(('localhost', 3000), http.server.SimpleHTTPRequestHandler).serve_forever()"

# Terminal 2 - Backend on port 8000
python -c "import http.server; http.server.HTTPServer(('localhost', 8000), http.server.SimpleHTTPRequestHandler).serve_forever()"

# Terminal 3 - Backend on port 8080
python -c "import http.server; http.server.HTTPServer(('localhost', 8080), http.server.SimpleHTTPRequestHandler).serve_forever()"
```

### Node.js Express Backends
```bash
# Install express globally
npm install -g express

# Create simple backend servers
node -e "const express = require('express'); const app = express(); app.get('*', (req, res) => res.json({server: 'backend-3000', path: req.path})); app.listen(3000, () => console.log('Backend running on port 3000'));"
```

## Troubleshooting

### Build Issues
- Ensure GCC supports C++17
- On Windows, install MinGW or use Visual Studio
- Check that all source files are present

### Runtime Issues
- Verify configuration file syntax (valid JSON)
- Check that proxy port is not in use
- Ensure backend servers are running and accessible
- Check firewall settings if external access needed

### Log Files
- Check `reverse_proxy.log` for detailed error messages
- Adjust log level in configuration file for more/less detail
