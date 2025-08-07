#include "Config.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

Config::Config() {
    loadDefaults();
}

bool Config::loadFromFile(const std::string& configFile) {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open config file: " << configFile << std::endl;
        std::cerr << "Using default configuration..." << std::endl;
        loadDefaults();
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonContent = buffer.str();
    file.close();
    
    if (!parseJson(jsonContent)) {
        std::cerr << "Error: Failed to parse config file: " << configFile << std::endl;
        std::cerr << "Using default configuration..." << std::endl;
        loadDefaults();
        return false;
    }
    
    if (!validate()) {
        std::cerr << "Error: Invalid configuration in file: " << configFile << std::endl;
        std::cerr << "Using default configuration..." << std::endl;
        loadDefaults();
        return false;
    }
    
    return true;
}

void Config::loadDefaults() {
    proxyPort = 8888;
    maxConnections = 100;
    connectionTimeout = 30;
    keepAlive = true;
    
    logFile = "reverse_proxy.log";
    logLevel = LogLevel::INFO;
    consoleLogging = true;
    
    algorithm = LoadBalancingAlgorithm::ROUND_ROBIN;
    backends.clear();
    backends.push_back(BackendConfig("127.0.0.1", 3000, 1, true));
    backends.push_back(BackendConfig("127.0.0.1", 8000, 1, true));
    backends.push_back(BackendConfig("127.0.0.1", 8080, 1, true));
    
    healthCheckEnabled = false;
    healthCheckInterval = 30;
    healthCheckPath = "/health";
    healthCheckTimeout = 5;
}

bool Config::parseJson(const std::string& jsonContent) {
    try {
        size_t serverPos = jsonContent.find("\"server\"");
        if (serverPos != std::string::npos) {
            size_t portPos = jsonContent.find("\"port\"", serverPos);
            if (portPos != std::string::npos) {
                size_t colonPos = jsonContent.find(":", portPos);
                size_t commaPos = jsonContent.find_first_of(",}", colonPos);
                if (colonPos != std::string::npos && commaPos != std::string::npos) {
                    std::string portStr = jsonContent.substr(colonPos + 1, commaPos - colonPos - 1);
                    portStr.erase(std::remove_if(portStr.begin(), portStr.end(), ::isspace), portStr.end());
                    proxyPort = std::stoi(portStr);
                }
            }
            
            size_t maxConnPos = jsonContent.find("\"max_connections\"", serverPos);
            if (maxConnPos != std::string::npos) {
                size_t colonPos = jsonContent.find(":", maxConnPos);
                size_t commaPos = jsonContent.find_first_of(",}", colonPos);
                if (colonPos != std::string::npos && commaPos != std::string::npos) {
                    std::string maxConnStr = jsonContent.substr(colonPos + 1, commaPos - colonPos - 1);
                    maxConnStr.erase(std::remove_if(maxConnStr.begin(), maxConnStr.end(), ::isspace), maxConnStr.end());
                    maxConnections = std::stoi(maxConnStr);
                }
            }
        }
        
        size_t loggingPos = jsonContent.find("\"logging\"");
        if (loggingPos != std::string::npos) {
            size_t levelPos = jsonContent.find("\"level\"", loggingPos);
            if (levelPos != std::string::npos) {
                size_t colonPos = jsonContent.find(":", levelPos);
                size_t quoteStart = jsonContent.find("\"", colonPos);
                size_t quoteEnd = jsonContent.find("\"", quoteStart + 1);
                if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                    std::string levelStr = jsonContent.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                    logLevel = parseLogLevel(levelStr);
                }
            }
            
            size_t filePos = jsonContent.find("\"file\"", loggingPos);
            if (filePos != std::string::npos) {
                size_t colonPos = jsonContent.find(":", filePos);
                size_t quoteStart = jsonContent.find("\"", colonPos);
                size_t quoteEnd = jsonContent.find("\"", quoteStart + 1);
                if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                    logFile = jsonContent.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                }
            }
        }
        
        size_t lbPos = jsonContent.find("\"load_balancer\"");
        if (lbPos != std::string::npos) {
            size_t algoPos = jsonContent.find("\"algorithm\"", lbPos);
            if (algoPos != std::string::npos) {
                size_t colonPos = jsonContent.find(":", algoPos);
                size_t quoteStart = jsonContent.find("\"", colonPos);
                size_t quoteEnd = jsonContent.find("\"", quoteStart + 1);
                if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                    std::string algoStr = jsonContent.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                    algorithm = parseAlgorithm(algoStr);
                }
            }
            
            size_t backendsPos = jsonContent.find("\"backends\"", lbPos);
            if (backendsPos != std::string::npos) {
                size_t arrayStart = jsonContent.find("[", backendsPos);
                size_t arrayEnd = jsonContent.find("]", arrayStart);
                if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
                    backends.clear();
                    
                    std::string backendsStr = jsonContent.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
                    size_t pos = 0;
                    
                    while (pos < backendsStr.length()) {
                        size_t objStart = backendsStr.find("{", pos);
                        if (objStart == std::string::npos) break;
                        
                        size_t objEnd = backendsStr.find("}", objStart);
                        if (objEnd == std::string::npos) break;
                        
                        std::string objStr = backendsStr.substr(objStart, objEnd - objStart + 1);
                        
                        BackendConfig backend;
                        
                        size_t hostPos = objStr.find("\"host\"");
                        if (hostPos != std::string::npos) {
                            size_t colonPos = objStr.find(":", hostPos);
                            size_t quoteStart = objStr.find("\"", colonPos);
                            size_t quoteEnd = objStr.find("\"", quoteStart + 1);
                            if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                                backend.host = objStr.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                            }
                        }
                        
                        size_t portPos = objStr.find("\"port\"");
                        if (portPos != std::string::npos) {
                            size_t colonPos = objStr.find(":", portPos);
                            size_t commaPos = objStr.find_first_of(",}", colonPos);
                            if (colonPos != std::string::npos && commaPos != std::string::npos) {
                                std::string portStr = objStr.substr(colonPos + 1, commaPos - colonPos - 1);
                                portStr.erase(std::remove_if(portStr.begin(), portStr.end(), ::isspace), portStr.end());
                                backend.port = std::stoi(portStr);
                            }
                        }
                        
                        size_t weightPos = objStr.find("\"weight\"");
                        if (weightPos != std::string::npos) {
                            size_t colonPos = objStr.find(":", weightPos);
                            size_t commaPos = objStr.find_first_of(",}", colonPos);
                            if (colonPos != std::string::npos && commaPos != std::string::npos) {
                                std::string weightStr = objStr.substr(colonPos + 1, commaPos - colonPos - 1);
                                weightStr.erase(std::remove_if(weightStr.begin(), weightStr.end(), ::isspace), weightStr.end());
                                backend.weight = std::stoi(weightStr);
                            }
                        }
                        
                        backends.push_back(backend);
                        pos = objEnd + 1;
                    }
                }
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false;
    }
}

LoadBalancingAlgorithm Config::parseAlgorithm(const std::string& algo) {
    if (algo == "ROUND_ROBIN") return LoadBalancingAlgorithm::ROUND_ROBIN;
    if (algo == "WEIGHTED_ROUND_ROBIN") return LoadBalancingAlgorithm::WEIGHTED_ROUND_ROBIN;
    if (algo == "LEAST_CONNECTIONS") return LoadBalancingAlgorithm::LEAST_CONNECTIONS;
    if (algo == "IP_HASH") return LoadBalancingAlgorithm::IP_HASH;
    return LoadBalancingAlgorithm::ROUND_ROBIN;
}

LogLevel Config::parseLogLevel(const std::string& level) {
    if (level == "DEBUG") return LogLevel::DEBUG;
    if (level == "INFO") return LogLevel::INFO;
    if (level == "WARNING") return LogLevel::WARNING;
    if (level == "ERROR") return LogLevel::ERROR;
    return LogLevel::INFO;
}

std::string Config::algorithmToString() const {
    switch (algorithm) {
        case LoadBalancingAlgorithm::ROUND_ROBIN: return "ROUND_ROBIN";
        case LoadBalancingAlgorithm::WEIGHTED_ROUND_ROBIN: return "WEIGHTED_ROUND_ROBIN";
        case LoadBalancingAlgorithm::LEAST_CONNECTIONS: return "LEAST_CONNECTIONS";
        case LoadBalancingAlgorithm::IP_HASH: return "IP_HASH";
        default: return "UNKNOWN";
    }
}

std::string Config::logLevelToString() const {
    switch (logLevel) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

bool Config::validate() const {
    if (proxyPort <= 0 || proxyPort > 65535) {
        std::cerr << "Invalid proxy port: " << proxyPort << std::endl;
        return false;
    }
    
    if (backends.empty()) {
        std::cerr << "No backend servers configured" << std::endl;
        return false;
    }
    
    for (const auto& backend : backends) {
        if (backend.host.empty()) {
            std::cerr << "Backend host cannot be empty" << std::endl;
            return false;
        }
        if (backend.port <= 0 || backend.port > 65535) {
            std::cerr << "Invalid backend port: " << backend.port << std::endl;
            return false;
        }
        if (backend.weight <= 0) {
            std::cerr << "Backend weight must be positive: " << backend.weight << std::endl;
            return false;
        }
    }
    
    if (healthCheckEnabled) {
        if (healthCheckInterval <= 0) {
            std::cerr << "Health check interval must be positive" << std::endl;
            return false;
        }
        if (healthCheckTimeout <= 0) {
            std::cerr << "Health check timeout must be positive" << std::endl;
            return false;
        }
    }
    
    return true;
}

void Config::printConfiguration() const {
    std::cout << "\n=== Reverse Proxy Configuration ===" << std::endl;
    std::cout << "Server:" << std::endl;
    std::cout << "  Port: " << proxyPort << std::endl;
    std::cout << "  Max Connections: " << maxConnections << std::endl;
    std::cout << "  Connection Timeout: " << connectionTimeout << "s" << std::endl;
    std::cout << "  Keep-Alive: " << (keepAlive ? "Enabled" : "Disabled") << std::endl;
    
    std::cout << "\nLogging:" << std::endl;
    std::cout << "  File: " << logFile << std::endl;
    std::cout << "  Level: " << logLevelToString() << std::endl;
    std::cout << "  Console: " << (consoleLogging ? "Enabled" : "Disabled") << std::endl;
    
    std::cout << "\nLoad Balancer:" << std::endl;
    std::cout << "  Algorithm: " << algorithmToString() << std::endl;
    std::cout << "  Backend Servers:" << std::endl;
    for (size_t i = 0; i < backends.size(); i++) {
        const auto& backend = backends[i];
        std::cout << "    " << (i + 1) << ". " << backend.host << ":" << backend.port 
                  << " (weight: " << backend.weight 
                  << ", " << (backend.enabled ? "enabled" : "disabled") << ")" << std::endl;
    }
    
    std::cout << "\nHealth Check:" << std::endl;
    std::cout << "  Enabled: " << (healthCheckEnabled ? "Yes" : "No") << std::endl;
    if (healthCheckEnabled) {
        std::cout << "  Interval: " << healthCheckInterval << "s" << std::endl;
        std::cout << "  Path: " << healthCheckPath << std::endl;
        std::cout << "  Timeout: " << healthCheckTimeout << "s" << std::endl;
    }
    std::cout << "==================================\n" << std::endl;
}
