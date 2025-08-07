#pragma once
#include <vector>
#include <string>
#include <atomic>
#include <map>
#include "Config.h"

/**
 * Represents a backend server
 * Enhanced with weight and connection tracking
 */
struct BackendServer {
    std::string host;
    int port;
    
    int weight;
    bool isHealthy;
    std::atomic<int> activeConnections;
    
    BackendServer(const std::string& h, int p, int w = 1) 
        : host(h), port(p), weight(w), isHealthy(true), activeConnections(0) {}
    
    // Custom copy constructor and assignment operator for atomic
    BackendServer(const BackendServer& other) 
        : host(other.host), port(other.port), weight(other.weight), 
          isHealthy(other.isHealthy), activeConnections(other.activeConnections.load()) {}
    
    BackendServer& operator=(const BackendServer& other) {
        if (this != &other) {
            host = other.host;
            port = other.port;
            weight = other.weight;
            isHealthy = other.isHealthy;
            activeConnections.store(other.activeConnections.load());
        }
        return *this;
    }
};

/**
 * LoadBalancer class - manages backend servers and routing
 * Supports multiple load balancing algorithms
 * Thread-safe for concurrent requests
 */
class LoadBalancer {
private:
    std::vector<BackendServer> backends;
    std::atomic<size_t> currentIndex;  // For round-robin
    std::atomic<size_t> weightedIndex; // For weighted round-robin
    LoadBalancingAlgorithm algorithm;
    
    // Weighted round-robin state
    std::vector<int> currentWeights;
    std::atomic<int> totalWeight;

public:
    LoadBalancer(LoadBalancingAlgorithm algo = LoadBalancingAlgorithm::ROUND_ROBIN);
    
    // Configure from Config object
    void configure(const Config& config);
    
    // Add a backend server to the pool
    void addBackend(const std::string& host, int port, int weight = 1);
    
    // Get next backend using configured algorithm
    BackendServer* getNextBackend(const std::string& clientIP = "");
    
    // Algorithm-specific methods
    BackendServer* getRoundRobinBackend();
    BackendServer* getWeightedRoundRobinBackend();
    BackendServer* getLeastConnectionsBackend();
    BackendServer* getIPHashBackend(const std::string& clientIP);
    
    // Connection tracking
    void incrementConnections(const std::string& host, int port);
    void decrementConnections(const std::string& host, int port);
    
    // Health check methods
    void markUnhealthy(const std::string& host, int port);
    void markHealthy(const std::string& host, int port);
    
    // Utility methods
    size_t getBackendCount() const;
    size_t getHealthyBackendCount() const;
    void printStatus() const;
    
    // Algorithm management
    void setAlgorithm(LoadBalancingAlgorithm algo);
    LoadBalancingAlgorithm getAlgorithm() const { return algorithm; }
};
