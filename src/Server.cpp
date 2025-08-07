#include "Server.h"
#include <iostream>
#include <sstream>
#include <chrono>

Server::Server(Logger& log, LoadBalancer& lb) 
    : logger(log), loadBalancer(lb), serverSocket(INVALID_SOCKET) {
    logger.info("Server instance created");
}

Server::~Server() {
    stop();
    cleanupNetworking();
}

bool Server::configure(const std::string& configFile) {
    logger.info("Loading configuration from: " + configFile);
    
    if (!config.loadFromFile(configFile)) {
        logger.warning("Failed to load config file, using defaults");
    }
    
    logger.configure(config);
    logger.info("Logger configured successfully");
    
    loadBalancer.configure(config);
    logger.info("Load balancer configured successfully");
    
    config.printConfiguration();
    loadBalancer.printStatus();
    
    logger.info("Server configured on port " + std::to_string(config.getProxyPort()));
    return true;
}

bool Server::initializeNetworking() {
#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        logger.error("WSAStartup failed with error: " + std::to_string(result));
        return false;
    }
    logger.info("Windows Winsock initialized successfully");
#endif
    return true;
}

void Server::cleanupNetworking() {
#ifdef _WIN32
    WSACleanup();
    logger.info("Windows Winsock cleaned up");
#endif
}

bool Server::start() {
    if (!initializeNetworking()) {
        return false;
    }
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        logger.error("Failed to create socket");
        cleanupNetworking();
        return false;
    }
    
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        logger.warning("Failed to set SO_REUSEADDR");
    }
    
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(config.getProxyPort());
    
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        logger.error("Failed to bind socket on port " + std::to_string(config.getProxyPort()));
        closesocket(serverSocket);
        cleanupNetworking();
        return false;
    }
    
    if (listen(serverSocket, config.getMaxConnections()) == SOCKET_ERROR) {
        logger.error("Failed to listen on socket");
        closesocket(serverSocket);
        cleanupNetworking();
        return false;
    }
    
    running.store(true);
    logger.info("Server started successfully on port " + std::to_string(config.getProxyPort()));
    std::cout << "Reverse Proxy Server listening on port " << config.getProxyPort() << std::endl;
    std::cout << "Algorithm: " << config.algorithmToString() << std::endl;
    std::cout << "Backend servers: " << loadBalancer.getBackendCount() << std::endl;
    std::cout << "Send HTTP requests to test the load balancing!" << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    
    while (running.load()) {
        sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            if (running.load()) {
                logger.warning("Failed to accept client connection");
            }
            continue;
        }
        
        handleClient(clientSocket);
        closesocket(clientSocket);
    }
    
    return true;
}

void Server::handleClient(SOCKET clientSocket) {
    char buffer[4096];
    
    std::string clientIP = getClientIP(clientSocket);
    
    // Receive HTTP request
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        logger.warning("Failed to receive data from client " + clientIP);
        return;
    }
    
    buffer[bytesReceived] = '\0';
    std::string request(buffer);
    
    logger.debug("Received HTTP request from " + clientIP + " (" + std::to_string(bytesReceived) + " bytes)");
    
    std::pair<std::string, std::string> parsedRequest = parseHttpRequest(request);
    std::string method = parsedRequest.first;
    std::string path = parsedRequest.second;
    
    if (method.empty() || path.empty()) {
        logger.warning("Invalid HTTP request format from " + clientIP);
        std::string response = createHttpResponse(400, "Bad Request");
        send(clientSocket, response.c_str(), response.length(), 0);
        return;
    }
    
    logger.info("Request: " + method + " " + path + " from " + clientIP);
    
    std::string backendResponse = forwardToBackend(method, path, request, clientIP);
    
    send(clientSocket, backendResponse.c_str(), backendResponse.length(), 0);
    logger.debug("Response sent to client " + clientIP);
}

std::string Server::getClientIP(SOCKET clientSocket) {
    sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    
    if (getpeername(clientSocket, (sockaddr*)&clientAddr, &addrLen) == 0) {
#ifdef _WIN32
        return std::string(inet_ntoa(clientAddr.sin_addr));
#else
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, INET_ADDRSTRLEN);
        return std::string(ipStr);
#endif
    }
    
    return "unknown";
}

std::pair<std::string, std::string> Server::parseHttpRequest(const std::string& request) {
    std::istringstream iss(request);
    std::string method, path, version;
    
    if (iss >> method >> path >> version) {
        return {method, path};
    }
    
    return {"", ""};
}

std::string Server::forwardToBackend(const std::string& method, const std::string& path, 
                                    const std::string& headers, const std::string& clientIP) {
    BackendServer* backend = loadBalancer.getNextBackend(clientIP);
    
    if (backend == nullptr) {
        logger.error("No healthy backend servers available");
        return createHttpResponse(503, "Service Unavailable - No backend servers");
    }
    
    std::string backendUrl = backend->host + ":" + std::to_string(backend->port);
    logger.info("Forwarding " + method + " " + path + " to backend: " + backendUrl + 
                " (algorithm: " + config.algorithmToString() + ")");
    
    loadBalancer.incrementConnections(backend->host, backend->port);
    
    std::string responseBody = "{\n";
    responseBody += "  \"message\": \"Request processed successfully\",\n";
    responseBody += "  \"method\": \"" + method + "\",\n";
    responseBody += "  \"path\": \"" + path + "\",\n";
    responseBody += "  \"backend\": \"" + backendUrl + "\",\n";
    responseBody += "  \"client_ip\": \"" + clientIP + "\",\n";
    responseBody += "  \"algorithm\": \"" + config.algorithmToString() + "\",\n";
    responseBody += "  \"backend_weight\": " + std::to_string(backend->weight) + ",\n";
    responseBody += "  \"backend_connections\": " + std::to_string(backend->activeConnections.load()) + ",\n";
    responseBody += "  \"timestamp\": \"" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) + "\"\n";
    responseBody += "}";
    
    loadBalancer.decrementConnections(backend->host, backend->port);
    
    logger.info("Backend " + backendUrl + " processed request successfully");
    return createHttpResponse(200, responseBody);
}

std::string Server::createHttpResponse(int statusCode, const std::string& body) {
    std::string statusText;
    switch (statusCode) {
        case 200: statusText = "OK"; break;
        case 400: statusText = "Bad Request"; break;
        case 503: statusText = "Service Unavailable"; break;
        default: statusText = "Unknown"; break;
    }
    
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "Server: ReverseProxy/1.0\r\n";
    response << "\r\n";
    response << body;
    
    return response.str();
}

void Server::stop() {
    if (running.load()) {
        running.store(false);
        logger.info("Server stopping...");
        
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
            serverSocket = INVALID_SOCKET;
        }
        
        logger.info("Server stopped successfully");
    }
}
