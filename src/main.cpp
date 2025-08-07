#include "Server.h"
#include "Logger.h"
#include "LoadBalancer.h"
#include "Config.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "=== Reverse Proxy Server with Configuration Management ===" << std::endl;
    
    std::string configFile = "config.json";
    
    if (argc > 1) {
        configFile = argv[1];
    }
    
    Logger logger("", true, LogLevel::INFO);
    logger.info("=== Reverse Proxy Server Starting ===");
    logger.info("Using configuration file: " + configFile);
    
    LoadBalancer loadBalancer(LoadBalancingAlgorithm::ROUND_ROBIN);
    
    Server proxyServer(logger, loadBalancer);
    
    if (!proxyServer.configure(configFile)) {
        logger.error("Failed to configure server");
        return 1;
    }
    
    const Config& config = proxyServer.getConfig();
    
    std::cout << "\nStarting Reverse Proxy Server..." << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << "Server will listen on: http://localhost:" << config.getProxyPort() << std::endl;
    std::cout << "Load Balancing Algorithm: " << config.algorithmToString() << std::endl;
    std::cout << "Backend Servers: " << config.getBackends().size() << std::endl;
    
    for (size_t i = 0; i < config.getBackends().size(); i++) {
        const auto& backend = config.getBackends()[i];
        std::cout << "   " << (i + 1) << ". " << backend.host << ":" << backend.port 
                  << " (weight: " << backend.weight << ")" << std::endl;
    }
    
    std::cout << "\nTest with commands like:" << std::endl;
    std::cout << "   curl http://localhost:" << config.getProxyPort() << std::endl;
    std::cout << "   curl http://localhost:" << config.getProxyPort() << "/api/users" << std::endl;
    std::cout << "   curl -X POST http://localhost:" << config.getProxyPort() << "/api/login" << std::endl;
    std::cout << "\nNote: Backend servers should be running on configured ports" << std::endl;
    std::cout << "    If no backends are available, you'll get 503 Service Unavailable" << std::endl;
    std::cout << "\nPress Ctrl+C to stop the server" << std::endl;
    std::cout << "================================================" << std::endl;
    
    bool success = proxyServer.start();
    
    if (!success) {
        logger.error("Failed to start reverse proxy server");
        std::cout << "Failed to start server. Check logs for details." << std::endl;
        return 1;
    }
    
    logger.info("=== Reverse Proxy Server Shutdown Complete ===");
    std::cout << "\nServer shutdown complete. Check '" << config.getLogFile() << "' for detailed logs." << std::endl;
    
    return 0;
}
