# Reverse Proxy Server

A high-performance C++ reverse proxy server with configurable load balancing algorithms and comprehensive logging.

## Features

- **Configuration Management**: JSON-based configuration with hot reloading support
- **Multiple Load Balancing Algorithms**: Round-robin, weighted round-robin, least connections, and IP hash
- **Cross-Platform**: Windows and Linux support with native socket APIs
- **Advanced Logging**: Configurable log levels with file and console output
- **HTTP Support**: HTTP/1.1 request parsing and forwarding
- **Health Management**: Backend server health tracking and monitoring
- **Thread-Safe**: Atomic operations for concurrent request handling

## Version History

This project demonstrates incremental development with multiple versions available:

- **[`v1`](../../tree/v1)** - Simple reverse proxy with basic round-robin load balancing
- **[`v2`](../../tree/v2)** - Configuration management with JSON configs and multiple algorithms
- **[`main`](../../tree/main)** - Stable release branch (latest stable features)
- **[`dev`](../../tree/dev)** - Active development branch (experimental features)

## Architecture

```
Client → Reverse Proxy (Port 8888) → Backend Servers (3000, 8000, 8080)
```

### Core Components

- **Server**: Main HTTP server with configuration management and client handling
- **LoadBalancer**: Multiple algorithms (round-robin, weighted, least connections, IP hash)
- **Logger**: Configurable logging system with multiple levels and destinations
- **Config**: JSON configuration parser with validation and defaults

## Quick Start

### Prerequisites

- GCC with C++17 support
- Windows: MinGW or Visual Studio
- Linux: Standard GCC installation

### Build

```cmd
# Windows
g++ -std=c++17 -I include src/Logger.cpp src/LoadBalancer.cpp src/Config.cpp src/Server.cpp src/main.cpp -lws2_32 -o reverse_proxy.exe

# Linux
g++ -std=c++17 -I include src/Logger.cpp src/LoadBalancer.cpp src/Config.cpp src/Server.cpp src/main.cpp -o reverse_proxy
```

### Run

```cmd
# Windows (with default config)
.\reverse_proxy.exe

# Windows (with custom config)
.\reverse_proxy.exe config-weighted.json

# Linux (with default config)
./reverse_proxy

# Linux (with custom config)
./reverse_proxy config-least-connections.json
```

The server starts on `http://localhost:8888` (configurable)

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

The server uses JSON configuration files for all settings. A default `config.json` file is provided.

For detailed configuration options and examples, see [CONFIG.md](CONFIG.md).

## Project Structure

```
├── include/
│   ├── Server.h         # Main server class
│   ├── LoadBalancer.h   # Load balancing algorithms
│   ├── Logger.h         # Logging system
│   └── Config.h         # Configuration management
├── src/
│   ├── Server.cpp       # Server implementation
│   ├── LoadBalancer.cpp # Load balancer implementation
│   ├── Logger.cpp       # Logger implementation
│   ├── Config.cpp       # Configuration parser
│   └── main.cpp         # Application entry point
├── config.json          # Default configuration
├── config-weighted.json # Weighted round-robin example
├── config-least-connections.json # Least connections example
└── BUILD-AND-RUN.md     # Detailed build instructions
```

## Technical Details

### Load Balancing
- **Algorithms**: Round-robin, weighted round-robin, least connections, IP hash
- **Health Checking**: Configurable backend health monitoring
- **Failover**: Automatic backend selection with connection tracking
- **Configuration**: JSON-based backend server configuration

### Networking
- **Windows**: WinSock2 API
- **Linux**: POSIX sockets
- **Protocol**: HTTP/1.1 support
- **Concurrency**: Multi-threaded client handling

### Logging
- **Levels**: DEBUG, INFO, WARNING, ERROR
- **Destinations**: File and console output (configurable)
- **Format**: Timestamp + level + message
- **Thread Safety**: Mutex-protected operations

### Configuration
- **Format**: JSON with comprehensive validation
- **Fallback**: Automatic defaults on parse errors
- **Hot Reload**: Runtime configuration updates
- **Validation**: Input validation with error reporting

## Performance

- **Concurrent Connections**: Multiple simultaneous clients
- **Memory Management**: RAII principles with automatic cleanup
- **Error Handling**: Comprehensive error checking and logging

## License

MIT License - See LICENSE file for details

