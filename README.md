# Reverse Proxy Server

A high-performance C++ reverse proxy server with round-robin load balancing and comprehensive logging.

## Features

- **Load Balancing**: Round-robin distribution across multiple backend servers
- **Cross-Platform**: Windows and Linux support with native socket APIs
- **Logging**: File and console logging with timestamps
- **HTTP Support**: Basic HTTP request parsing and forwarding
- **Health Management**: Backend server health tracking
- **Thread-Safe**: Atomic operations for concurrent request handling

## Architecture

```
Client → Reverse Proxy (Port 8888) → Backend Servers (3000, 8000, 8080)
```

### Core Components

- **Server**: Main HTTP server handling client connections
- **LoadBalancer**: Round-robin algorithm with backend health management
- **Logger**: Thread-safe logging system with file and console output

## Quick Start

### Prerequisites

- GCC with C++17 support
- Windows: MinGW or Visual Studio
- Linux: Standard GCC installation

### Build

```cmd
# Windows
g++ -std=c++17 -I include src/Logger.cpp src/LoadBalancer.cpp src/Server.cpp src/main.cpp -lws2_32 -o reverse_proxy.exe

# Linux
g++ -std=c++17 -I include src/Logger.cpp src/LoadBalancer.cpp src/Server.cpp src/main.cpp -o reverse_proxy
```

### Run

```cmd
# Windows
.\reverse_proxy.exe

# Linux
./reverse_proxy
```

The server starts on `http://localhost:8888`

## Testing

### Start Test Backends

```cmd
# Terminal 1
python -c "import http.server; http.server.HTTPServer(('localhost', 3000), http.server.SimpleHTTPRequestHandler).serve_forever()"

# Terminal 2
python -c "import http.server; http.server.HTTPServer(('localhost', 8000), http.server.SimpleHTTPRequestHandler).serve_forever()"

# Terminal 3
python -c "import http.server; http.server.HTTPServer(('localhost', 8080), http.server.SimpleHTTPRequestHandler).serve_forever()"
```

### Test Requests

```cmd
curl http://localhost:8888/
curl http://localhost:8888/api/users
curl -X POST http://localhost:8888/api/login
```

## Configuration

Default backend servers (configured in `src/main.cpp`):
- `127.0.0.1:3000`
- `127.0.0.1:8000`
- `127.0.0.1:8080`

Proxy server port: `8888`

## Project Structure

```
├── include/
│   ├── Server.h         # Main server class
│   ├── LoadBalancer.h   # Load balancing logic
│   └── Logger.h         # Logging system
├── src/
│   ├── Server.cpp       # Server implementation
│   ├── LoadBalancer.cpp # Load balancer implementation
│   ├── Logger.cpp       # Logger implementation
│   └── main.cpp         # Application entry point
└── BUILD-AND-RUN.md     # Detailed build instructions
```

## Technical Details

### Load Balancing
- **Algorithm**: Round-robin with atomic counter
- **Health Checking**: Basic availability tracking
- **Failover**: Automatic backend selection

### Networking
- **Windows**: WinSock2 API
- **Linux**: POSIX sockets
- **Protocol**: HTTP/1.1 support
- **Concurrency**: Multi-threaded client handling

### Logging
- **Destinations**: File (`reverse_proxy.log`) and console
- **Format**: Timestamp + level + message
- **Thread Safety**: Mutex-protected operations

## Performance

- **Concurrent Connections**: Multiple simultaneous clients
- **Memory Management**: RAII principles with automatic cleanup
- **Error Handling**: Comprehensive error checking and logging

## License

MIT License - See LICENSE file for details

