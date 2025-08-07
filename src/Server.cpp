#include "Server.h"
#include <iostream>
#include <sstream>
#include <chrono>

Server::Server(int serverPort, Logger& log, LoadBalancer& lb) 
    : port(serverPort), logger(log), loadBalancer(lb), serverSocket(INVALID_SOCKET) {
    logger.info("Server created on port " + std::to_string(port));
}

Server::~Server() {
    stop();
    cleanupNetworking();
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
    
    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        logger.error("Failed to create socket");
        cleanupNetworking();
        return false;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        logger.warning("Failed to set SO_REUSEADDR");
    }
    
    // Setup server address
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    // Bind socket
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        logger.error("Failed to bind socket on port " + std::to_string(port));
        closesocket(serverSocket);
        cleanupNetworking();
        return false;
    }
    
    // Listen for connections
    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        logger.error("Failed to listen on socket");
        closesocket(serverSocket);
        cleanupNetworking();
        return false;
    }
    
    running.store(true);
    logger.info("Server started successfully on port " + std::to_string(port));
    std::cout << "Reverse Proxy Server listening on port " << port << std::endl;
    std::cout << "Send HTTP requests to test the load balancing!" << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    
    // Main server loop
    while (running.load()) {
        sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        // Accept client connection
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            if (running.load()) {
                logger.warning("Failed to accept client connection");
            }
            continue;
        }
        
        // Handle client in same thread
        handleClient(clientSocket);
        closesocket(clientSocket);
    }
    
    return true;
}

void Server::handleClient(SOCKET clientSocket) {
    char buffer[4096];
    
    // Receive HTTP request
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        logger.warning("Failed to receive data from client");
        return;
    }
    
    buffer[bytesReceived] = '\0';
    std::string request(buffer);
    
    logger.info("Received HTTP request (" + std::to_string(bytesReceived) + " bytes)");
    
    // Parse HTTP request
    std::pair<std::string, std::string> parsedRequest = parseHttpRequest(request);
    std::string method = parsedRequest.first;
    std::string path = parsedRequest.second;
    
    if (method.empty() || path.empty()) {
        logger.warning("Invalid HTTP request format");
        std::string response = createHttpResponse(400, "Bad Request");
        send(clientSocket, response.c_str(), response.length(), 0);
        return;
    }
    
    logger.info("Parsed: " + method + " " + path);
    
    // Forward to backend
    std::string backendResponse = forwardToBackend(method, path, request);
    
    // Send response back to client
    send(clientSocket, backendResponse.c_str(), backendResponse.length(), 0);
    logger.info("Response sent to client");
}

std::pair<std::string, std::string> Server::parseHttpRequest(const std::string& request) {
    std::istringstream iss(request);
    std::string method, path, version;
    
    if (iss >> method >> path >> version) {
        return {method, path};
    }
    
    return {"", ""};
}

std::string Server::forwardToBackend(const std::string& method, const std::string& path, const std::string& headers) {
    // Get backend server from load balancer
    BackendServer* backend = loadBalancer.getNextBackend();
    
    if (backend == nullptr) {
        logger.error("No healthy backend servers available");
        return createHttpResponse(503, "Service Unavailable - No backend servers");
    }
    
    std::string backendUrl = backend->host + ":" + std::to_string(backend->port);
    logger.info("Forwarding " + method + " " + path + " to backend: " + backendUrl);
    
    std::string responseBody = "{\n";
    responseBody += "  \"message\": \"Request processed successfully\",\n";
    responseBody += "  \"method\": \"" + method + "\",\n";
    responseBody += "  \"path\": \"" + path + "\",\n";
    responseBody += "  \"backend\": \"" + backendUrl + "\",\n";
    responseBody += "  \"timestamp\": \"" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) + "\"\n";
    responseBody += "}";
    
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
