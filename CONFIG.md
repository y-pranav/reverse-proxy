# Configuration Management

This document describes the configuration management features added to the reverse proxy server.

## Configuration File Format

The server uses JSON configuration files with the following structure:

```json
{
  "server": {
    "port": 8888,
    "max_connections": 100,
    "connection_timeout": 30,
    "keep_alive": true
  },
  "logging": {
    "file": "reverse_proxy.log",
    "level": "INFO",
    "console": true
  },
  "load_balancer": {
    "algorithm": "ROUND_ROBIN",
    "backends": [
      {
        "host": "127.0.0.1",
        "port": 3000,
        "weight": 1,
        "enabled": true
      }
    ]
  },
  "health_check": {
    "enabled": true,
    "interval": 30,
    "path": "/health",
    "timeout": 5
  }
}
```

## Configuration Sections

### Server Configuration
- `port`: Port number for the reverse proxy (1-65535)
- `max_connections`: Maximum concurrent connections
- `connection_timeout`: Connection timeout in seconds
- `keep_alive`: Enable HTTP keep-alive connections

### Logging Configuration
- `file`: Log file path (empty string disables file logging)
- `level`: Log level (`DEBUG`, `INFO`, `WARNING`, `ERROR`)
- `console`: Enable console output (true/false)

### Load Balancer Configuration
- `algorithm`: Load balancing algorithm
  - `ROUND_ROBIN`: Simple round-robin distribution
  - `WEIGHTED_ROUND_ROBIN`: Weighted round-robin based on server weights
  - `LEAST_CONNECTIONS`: Route to server with fewest active connections
  - `IP_HASH`: Consistent routing based on client IP hash

#### Backend Server Configuration
- `host`: Backend server hostname or IP address
- `port`: Backend server port number
- `weight`: Server weight for weighted algorithms (higher = more requests)
- `enabled`: Enable/disable this backend server

### Health Check Configuration (Future Feature)
- `enabled`: Enable health checking
- `interval`: Health check interval in seconds
- `path`: Health check endpoint path
- `timeout`: Health check timeout in seconds

## Usage Examples

### Basic Usage
```cmd
# Use default config.json
.\reverse_proxy.exe

# Use custom configuration file
.\reverse_proxy.exe config-weighted.json
```

### Sample Configurations

#### Weighted Round-Robin
Use `config-weighted.json` for weighted distribution:
- Server 1 (port 3000): weight 3 (gets 50% of requests)
- Server 2 (port 8000): weight 1 (gets 16.7% of requests)  
- Server 3 (port 8080): weight 2 (gets 33.3% of requests)

#### Least Connections
Use `config-least-connections.json` for connection-based routing:
- Routes requests to the backend with the fewest active connections
- Good for varying request processing times

### Testing Different Algorithms

1. **Round Robin Test**:
```cmd
.\reverse_proxy.exe config.json
curl http://localhost:8888/test1
curl http://localhost:8888/test2
curl http://localhost:8888/test3
```

2. **Weighted Round Robin Test**:
```cmd
.\reverse_proxy.exe config-weighted.json
# Server on port 3000 should get ~3x more requests than port 8000
```

3. **Least Connections Test**:
```cmd
.\reverse_proxy.exe config-least-connections.json
# Use concurrent requests to see connection-based routing
```

## Configuration Validation

The server validates configuration on startup:
- Port numbers must be valid (1-65535)
- At least one backend server must be configured
- Weights must be positive integers
- Log levels must be valid values
- Health check intervals must be positive

Invalid configurations fall back to default values with warnings.

## Default Configuration

If no configuration file is found, the server uses these defaults:
- Port: 8888
- Algorithm: Round Robin
- Backends: 127.0.0.1:3000, 127.0.0.1:8000, 127.0.0.1:8080
- Logging: INFO level, console + file
- Log file: reverse_proxy.log

## Build Instructions

The configuration system requires no additional dependencies. Build as usual:

```cmd
g++ -std=c++17 -I include src/Config.cpp src/Logger.cpp src/LoadBalancer.cpp src/Server.cpp src/main.cpp -lws2_32 -o reverse_proxy.exe
```

## Features Added

1. **JSON Configuration Parser**: Basic JSON parsing for configuration files
2. **Multiple Load Balancing Algorithms**: Support for 4 different algorithms
3. **Configurable Logging**: Log levels and destinations
4. **Server Configuration**: Port, connections, timeouts
5. **Backend Management**: Weights, enable/disable backends
6. **Validation**: Configuration validation with fallback to defaults
7. **Command Line Support**: Custom config file via command line argument

## Future Enhancements

- Real health checking implementation
- SSL/TLS configuration
- Advanced routing rules
- Metrics and monitoring configuration
- Hot configuration reloading
