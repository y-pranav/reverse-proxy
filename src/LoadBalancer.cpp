#include "LoadBalancer.h"
#include <algorithm>
#include <iostream>
#include <functional>
#include <climits>

LoadBalancer::LoadBalancer(LoadBalancingAlgorithm algo) 
    : currentIndex(0), weightedIndex(0), algorithm(algo), totalWeight(0) {
}

void LoadBalancer::configure(const Config& config) {
    backends.clear();
    currentWeights.clear();
    totalWeight.store(0);
    
    algorithm = config.getAlgorithm();
    
    for (const auto& backendConfig : config.getBackends()) {
        if (backendConfig.enabled) {
            addBackend(backendConfig.host, backendConfig.port, backendConfig.weight);
        }
    }
}

void LoadBalancer::addBackend(const std::string& host, int port, int weight) {
    backends.push_back(BackendServer(host, port, weight));
    currentWeights.push_back(0);
    totalWeight.fetch_add(weight);
}

BackendServer* LoadBalancer::getNextBackend(const std::string& clientIP) {
    if (backends.empty()) return nullptr;
    
    switch (algorithm) {
        case LoadBalancingAlgorithm::ROUND_ROBIN:
            return getRoundRobinBackend();
        case LoadBalancingAlgorithm::WEIGHTED_ROUND_ROBIN:
            return getWeightedRoundRobinBackend();
        case LoadBalancingAlgorithm::LEAST_CONNECTIONS:
            return getLeastConnectionsBackend();
        case LoadBalancingAlgorithm::IP_HASH:
            return getIPHashBackend(clientIP);
        default:
            return getRoundRobinBackend();
    }
}

BackendServer* LoadBalancer::getRoundRobinBackend() {
    size_t startIndex = currentIndex.fetch_add(1) % backends.size();
    
    if (backends[startIndex].isHealthy) {
        return &backends[startIndex];
    }
    
    for (size_t i = 1; i < backends.size(); i++) {
        size_t index = (startIndex + i) % backends.size();
        if (backends[index].isHealthy) {
            return &backends[index];
        }
    }
    
    return nullptr;
}

BackendServer* LoadBalancer::getWeightedRoundRobinBackend() {
    if (backends.empty()) return nullptr;
    
    int maxWeight = 0;
    int selectedIndex = -1;
    
    for (size_t i = 0; i < backends.size(); i++) {
        if (!backends[i].isHealthy) continue;
        
        currentWeights[i] += backends[i].weight;
        
        if (selectedIndex == -1 || currentWeights[i] > maxWeight) {
            maxWeight = currentWeights[i];
            selectedIndex = static_cast<int>(i);
        }
    }
    
    if (selectedIndex == -1) return nullptr;
    
    currentWeights[selectedIndex] -= totalWeight.load();
    
    return &backends[selectedIndex];
}

BackendServer* LoadBalancer::getLeastConnectionsBackend() {
    BackendServer* selected = nullptr;
    int minConnections = INT_MAX;
    
    for (auto& backend : backends) {
        if (!backend.isHealthy) continue;
        
        int connections = backend.activeConnections.load();
        if (connections < minConnections) {
            minConnections = connections;
            selected = &backend;
        }
    }
    
    return selected;
}

BackendServer* LoadBalancer::getIPHashBackend(const std::string& clientIP) {
    if (backends.empty()) return nullptr;
    
    std::hash<std::string> hasher;
    size_t hash = hasher(clientIP);
    
    std::vector<size_t> healthyIndices;
    for (size_t i = 0; i < backends.size(); i++) {
        if (backends[i].isHealthy) {
            healthyIndices.push_back(i);
        }
    }
    
    if (healthyIndices.empty()) return nullptr;
    
    size_t selectedIndex = healthyIndices[hash % healthyIndices.size()];
    return &backends[selectedIndex];
}

void LoadBalancer::incrementConnections(const std::string& host, int port) {
    for (auto& backend : backends) {
        if (backend.host == host && backend.port == port) {
            backend.activeConnections.fetch_add(1);
            break;
        }
    }
}

void LoadBalancer::decrementConnections(const std::string& host, int port) {
    for (auto& backend : backends) {
        if (backend.host == host && backend.port == port) {
            int current = backend.activeConnections.load();
            if (current > 0) {
                backend.activeConnections.fetch_sub(1);
            }
            break;
        }
    }
}

void LoadBalancer::markUnhealthy(const std::string& host, int port) {
    auto it = std::find_if(backends.begin(), backends.end(),
        [&host, port](const BackendServer& server) {
            return server.host == host && server.port == port;
        }
    );
    
    if (it != backends.end()) {
        it->isHealthy = false;
        std::cout << "Backend " << host << ":" << port << " marked as unhealthy" << std::endl;
    }
}

void LoadBalancer::markHealthy(const std::string& host, int port) {
    auto it = std::find_if(backends.begin(), backends.end(),
        [&host, port](const BackendServer& server) {
            return server.host == host && server.port == port;
        });
    
    if (it != backends.end()) {
        it->isHealthy = true;
        std::cout << "Backend " << host << ":" << port << " marked as healthy" << std::endl;
    }
}

size_t LoadBalancer::getBackendCount() const {
    return backends.size();
}

size_t LoadBalancer::getHealthyBackendCount() const {
    return std::count_if(backends.begin(), backends.end(),
        [](const BackendServer& server) { return server.isHealthy; });
}

void LoadBalancer::printStatus() const {
    std::cout << "\n=== Load Balancer Status ===" << std::endl;
    std::cout << "Algorithm: ";
    switch (algorithm) {
        case LoadBalancingAlgorithm::ROUND_ROBIN:
            std::cout << "Round Robin";
            break;
        case LoadBalancingAlgorithm::WEIGHTED_ROUND_ROBIN:
            std::cout << "Weighted Round Robin";
            break;
        case LoadBalancingAlgorithm::LEAST_CONNECTIONS:
            std::cout << "Least Connections";
            break;
        case LoadBalancingAlgorithm::IP_HASH:
            std::cout << "IP Hash";
            break;
    }
    std::cout << std::endl;
    
    std::cout << "Total Backends: " << backends.size() << std::endl;
    std::cout << "Healthy Backends: " << getHealthyBackendCount() << std::endl;
    
    std::cout << "\nBackend Details:" << std::endl;
    for (size_t i = 0; i < backends.size(); i++) {
        const auto& backend = backends[i];
        std::cout << "  " << (i + 1) << ". " << backend.host << ":" << backend.port
                  << " (weight: " << backend.weight
                  << ", connections: " << backend.activeConnections.load()
                  << ", " << (backend.isHealthy ? "healthy" : "unhealthy") << ")" << std::endl;
    }
    std::cout << "===========================\n" << std::endl;
}

void LoadBalancer::setAlgorithm(LoadBalancingAlgorithm algo) {
    algorithm = algo;
    
    currentIndex.store(0);
    weightedIndex.store(0);
    
    if (algo == LoadBalancingAlgorithm::WEIGHTED_ROUND_ROBIN) {
        std::fill(currentWeights.begin(), currentWeights.end(), 0);
    }
}