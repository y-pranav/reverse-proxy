#pragma once
#include <string>
#include <vector>
#include <map>

enum class LogLevel;

struct BackendConfig {
    std::string host;
    int port;
    int weight;
    bool enabled;
    
    BackendConfig() : host(""), port(0), weight(1), enabled(true) {}
    BackendConfig(const std::string& h, int p, int w = 1, bool e = true) 
        : host(h), port(p), weight(w), enabled(e) {}
};

enum class LoadBalancingAlgorithm {
    ROUND_ROBIN,
    WEIGHTED_ROUND_ROBIN,
    LEAST_CONNECTIONS,
    IP_HASH
};

class Config {
private:
    int proxyPort;
    std::string logFile;
    LogLevel logLevel;
    bool consoleLogging;
    
    LoadBalancingAlgorithm algorithm;
    std::vector<BackendConfig> backends;
    
    bool healthCheckEnabled;
    int healthCheckInterval;
    std::string healthCheckPath;
    int healthCheckTimeout;
    
    int maxConnections;
    int connectionTimeout;
    bool keepAlive;
    
    bool parseJson(const std::string& jsonContent);
    LoadBalancingAlgorithm parseAlgorithm(const std::string& algo);
    LogLevel parseLogLevel(const std::string& level);
    
public:
    Config();
    
    bool loadFromFile(const std::string& configFile);
    void loadDefaults();
    
    int getProxyPort() const { return proxyPort; }
    const std::string& getLogFile() const { return logFile; }
    LogLevel getLogLevel() const { return logLevel; }
    bool isConsoleLoggingEnabled() const { return consoleLogging; }
    
    LoadBalancingAlgorithm getAlgorithm() const { return algorithm; }
    const std::vector<BackendConfig>& getBackends() const { return backends; }
    
    bool isHealthCheckEnabled() const { return healthCheckEnabled; }
    int getHealthCheckInterval() const { return healthCheckInterval; }
    const std::string& getHealthCheckPath() const { return healthCheckPath; }
    int getHealthCheckTimeout() const { return healthCheckTimeout; }
    
    int getMaxConnections() const { return maxConnections; }
    int getConnectionTimeout() const { return connectionTimeout; }
    bool isKeepAliveEnabled() const { return keepAlive; }
    
    std::string algorithmToString() const;
    std::string logLevelToString() const;
    
    bool validate() const;
    void printConfiguration() const;
};
