#include "Server.h"
#include "Logger.h"
#include "LoadBalancer.h"
#include <iostream>

int main() {
    std::cout << "=== Real Networking Reverse Proxy Test ===" << std::endl;
    
    // Initialize logging
    Logger logger("real_networking_test.log", true);
    logger.info("=== Real Networking Test Started ===");
    
    // Initialize load balancer
    LoadBalancer loadBalancer;
    
    // Add some test backend servers
    loadBalancer.addBackend("127.0.0.1", 3000);
    loadBalancer.addBackend("127.0.0.1", 8000);
    loadBalancer.addBackend("127.0.0.1", 8080);
    
    logger.info("Configured " + std::to_string(loadBalancer.getBackendCount()) + " backend servers");
    std::cout << "Backend Servers Configured:" << std::endl;
    std::cout << "   127.0.0.1:3000" << std::endl;
    std::cout << "   127.0.0.1:8000" << std::endl;
    std::cout << "   127.0.0.1:8080" << std::endl;
    
    // Create the reverse proxy server
    const int PROXY_PORT = 8888;
    Server proxyServer(PROXY_PORT, logger, loadBalancer);
    
    std::cout << "\nStarting Real Reverse Proxy Server..." << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << "Server will listen on: http://localhost:" << PROXY_PORT << std::endl;
    std::cout << "Test with commands like:" << std::endl;
    std::cout << "   curl http://localhost:" << PROXY_PORT << std::endl;
    std::cout << "   curl http://localhost:" << PROXY_PORT << "/api/users" << std::endl;
    std::cout << "   curl -X POST http://localhost:" << PROXY_PORT << "/api/login" << std::endl;
    std::cout << "\nNote: Backend servers on ports 3000, 8000, 8080 should be running" << std::endl;
    std::cout << "    If no backends are available, you'll get 503 Service Unavailable" << std::endl;
    std::cout << "\nPress Ctrl+C to stop the server" << std::endl;
    std::cout << "================================================" << std::endl;
    
    // Start the server (this is a blocking call)
    bool success = proxyServer.start();
    
    if (!success) {
        logger.error("Failed to start reverse proxy server");
        std::cout << "Failed to start server. Check logs for details." << std::endl;
        return 1;
    }
    
    logger.info("=== Real Networking Test Completed ===");
    std::cout << "\nServer shutdown complete. Check 'real_networking_test.log' for detailed logs." << std::endl;
    
    return 0;
}
