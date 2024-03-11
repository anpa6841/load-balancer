#pragma once

#include "LoadBalancer.h"

// Weighted RoundRobin Load Balancer Mode
class WeightedRoundRobinMode: public LoadBalancerMode {
private:
    std::vector<std::string> servers;
    std::vector<int> weights;
    std::vector<int> originalWeights;
    size_t currentIndex;
public:
    WeightedRoundRobinMode();
    void add_server(const std::string& server, int weight);
    std::string selectBackend(LoadBalancer& lb, const std::string& /* clientIP */) override;
private:
    void reset_weights();
};