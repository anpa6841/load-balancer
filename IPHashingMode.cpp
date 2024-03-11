#include "IPHashingMode.h"

// Hash function for IPv4 addresses
size_t IPHashingMode::hashIPv4(const std::string& ipAddress) const {
    std::hash<std::string> hashFunction;
    return hashFunction(ipAddress);
}

std::string IPHashingMode::selectBackend(LoadBalancer& lb, const std::string& clientIP) {
    std::vector<std::string> backendServers = lb.getBackendServers();

    // std::cout << "Client IP: " << clientIP << std::endl;

    // If client IP is already mapped, return the corresponding backend server
    if (clientToServerMap.find(clientIP) != clientToServerMap.end()) {
        size_t index = clientToServerMap[clientIP];
        return backendServers[index];
    }

    // Calculate hash value from client IP
    size_t hashValue = hashIPv4(clientIP);

    // Determine backend server index based on hash value
    // Save the index in the clientToServerMap for subsequent
    // requests lookup originating from the same client
    size_t index = hashValue % backendServers.size();
    clientToServerMap[clientIP] = index;

    // Return backend server corresponding to the index
    return backendServers[index];
}
