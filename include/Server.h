#pragma once
#include <string>
#include <thread>
#include <atomic>
#include "Logger.h"
#include "LoadBalancer.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    typedef int SOCKET;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

class Server {
private:
    Logger& logger;
    LoadBalancer& loadBalancer;
    int port;
    SOCKET serverSocket;
    std::atomic<bool> running{false};
    
    // Initialize networking (Windows-specific)
    bool initializeNetworking();
    
    // Clean up networking
    void cleanupNetworking();
    
    // Handle a single client connection
    void handleClient(SOCKET clientSocket);
    
    // Parse HTTP request to extract method and path
    std::pair<std::string, std::string> parseHttpRequest(const std::string& request);
    
    // Forward request to backend server
    std::string forwardToBackend(const std::string& method, const std::string& path, const std::string& headers);
    
    // Create HTTP response
    std::string createHttpResponse(int statusCode, const std::string& body);

public:
    Server(int serverPort, Logger& log, LoadBalancer& lb);
    ~Server();
    
    // Start the server (blocking call)
    bool start();
    
    // Stop the server
    void stop();
    
    // Check if server is running
    bool isRunning() const { return running.load(); }
};
