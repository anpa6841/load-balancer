#include "RoundRobinMode.h"

// Default constructor implementation
RoundRobinMode::RoundRobinMode() : currentIndex(0) {};

// Round Robin Load Balancer Mode
std::string RoundRobinMode::selectBackend(LoadBalancer& lb, const std::string& /* clientIP */) {
    std::vector<std::string> backendServers = lb.getBackendServers();
    size_t index = currentIndex++ % backendServers.size();
    // std::cout << "Current Index: " << currentIndex << " ";
    return backendServers[index];
}
