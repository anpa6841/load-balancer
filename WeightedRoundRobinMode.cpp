#include "WeightedRoundRobinMode.h"

// Default constructor implmentation
WeightedRoundRobinMode::WeightedRoundRobinMode() : currentIndex(-1) {};

// Weighted Round Robin Load Balance Mode
void WeightedRoundRobinMode::add_server(const std::string& server, int weight) {
    servers.push_back(server);
    weights.push_back(weight);
    originalWeights.push_back(weight);
}

std::string WeightedRoundRobinMode::selectBackend(LoadBalancer& lb, const std::string& /* clientIP */) {
    if (servers.empty()) {
        return "No server available"; // No servers added
    }

    // Find the next available server with non-zero weight
    for (size_t i = 0; i < servers.size(); i++) {
        currentIndex = (currentIndex + 1) % servers.size();
        if (weights[currentIndex] > 0) {
            --weights[currentIndex]; // Decrement 
            // std::cout << "Current Index: " << currentIndex << " ";
            return servers[currentIndex]; // Return server name
        }
    }

    // If all servers have weight 0, reset the weights and return the first server
    reset_weights();
    return servers[currentIndex];
}

void WeightedRoundRobinMode::reset_weights() {
    std::copy(originalWeights.begin(), originalWeights.end(), weights.begin());
}