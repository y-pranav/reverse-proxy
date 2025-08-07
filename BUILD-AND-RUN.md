# Reverse Proxy - Windows Commands

## Build
```cmd
# Clean
taskkill /f /im reverse_proxy.exe 2>nul
del reverse_proxy.exe *.o 2>nul

# Build reverse proxy (port 8888)
g++ -std=c++17 -I include src/Logger.cpp src/LoadBalancer.cpp src/Server.cpp src/main.cpp -lws2_32 -o reverse_proxy.exe
```

## Run
```cmd
# Start reverse proxy (port 8888)
.\reverse_proxy.exe
```

## Test
```cmd
# Test the server
curl http://localhost:8888/
curl http://localhost:8888/api/users
curl -X POST http://localhost:8888/api/login
```

## Create Test Backends
```cmd
# For reverse proxy - backends on 3000, 8000, 8080
python -c "import http.server; http.server.HTTPServer(('localhost', 3000), http.server.SimpleHTTPRequestHandler).serve_forever()"
python -c "import http.server; http.server.HTTPServer(('localhost', 8000), http.server.SimpleHTTPRequestHandler).serve_forever()"
python -c "import http.server; http.server.HTTPServer(('localhost', 8080), http.server.SimpleHTTPRequestHandler).serve_forever()"
```
