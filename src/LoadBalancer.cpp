#include "LoadBalancer.h"
#include <algorithm>  

LoadBalancer::LoadBalancer() : currentIndex(0) {
}

void LoadBalancer::addBackend(const std::string& host, int port) {
    backends.push_back(BackendServer(host, port));
}

BackendServer* LoadBalancer::getNextBackend() {
    if (backends.empty()) return nullptr;
    
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

void LoadBalancer::markUnhealthy(const std::string& host, int port) {
    auto it = std::find_if(backends.begin(), backends.end(),
        [&host, port](const BackendServer& server) {
            return server.host == host && server.port == port;
        }
    );
    
    if (it != backends.end()) {
        it->isHealthy = false;
    }
}

void LoadBalancer::markHealthy(const std::string& host, int port) {
    auto it = std::find_if(backends.begin(), backends.end(),
        [&host, port](const BackendServer& server) {
            return server.host == host && server.port == port;
        });
    
    if (it != backends.end()) {
        it->isHealthy = true;
    }
}

size_t LoadBalancer::getBackendCount() const {
    return backends.size();
}