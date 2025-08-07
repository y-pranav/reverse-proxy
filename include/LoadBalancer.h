#pragma once
#include <vector>
#include <string>
#include <atomic>

/**
 * Represents a backend server
 * Like a simple data class/record in Java
 */
struct BackendServer {
    std::string host;
    int port;
    bool isHealthy;
    
    BackendServer(const std::string& h, int p) : host(h), port(p), isHealthy(true) {}
};

/**
 * LoadBalancer class - manages backend servers and routing
 * Implements round-robin algorithm
 * Thread-safe for concurrent requests
 */
class LoadBalancer {
private:
    std::vector<BackendServer> backends;
    std::atomic<size_t> currentIndex;  // Thread-safe counter

public:
    LoadBalancer();
    
    // Add a backend server to the pool
    void addBackend(const std::string& host, int port);
    
    // Get next backend using round-robin
    BackendServer* getNextBackend();
    
    // Health check methods (for future phases)
    void markUnhealthy(const std::string& host, int port);
    void markHealthy(const std::string& host, int port);
    
    // Get backend count
    size_t getBackendCount() const;
};
