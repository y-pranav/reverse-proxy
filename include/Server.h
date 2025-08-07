#pragma once
#include <string>
#include <thread>
#include <atomic>
#include "Logger.h"
#include "LoadBalancer.h"
#include "Config.h"

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
    Config config;
    SOCKET serverSocket;
    std::atomic<bool> running{false};
    
    bool initializeNetworking();
    void cleanupNetworking();
    void handleClient(SOCKET clientSocket);
    std::pair<std::string, std::string> parseHttpRequest(const std::string& request);
    std::string getClientIP(SOCKET clientSocket);
    std::string forwardToBackend(const std::string& method, const std::string& path, 
                                const std::string& headers, const std::string& clientIP);
    std::string createHttpResponse(int statusCode, const std::string& body);

public:
    Server(Logger& log, LoadBalancer& lb);
    ~Server();
    
    bool configure(const std::string& configFile);
    bool start();
    void stop();
    bool isRunning() const { return running.load(); }
    const Config& getConfig() const { return config; }
};
