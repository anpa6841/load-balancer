#include "RandomMode.h"

std::string RandomMode::selectBackend(LoadBalancer& lb, const std::string& /* clientIP */) {
    std::vector<std::string> backendServers = lb.getBackendServers();
    if (backendServers.empty()) {
        return ""; // No server available
    }

    // Seed the random number generator with current time
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(static_cast<unsigned int>(seed));

    // Randomly select a backend server index
    std::uniform_int_distribution<size_t> dist(0, backendServers.size() - 1);
    size_t index = dist(gen);

    // Return the random selected backend server
    return backendServers[index];
}
