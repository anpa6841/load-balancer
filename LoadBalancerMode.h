#pragma once

#include <string>

class LoadBalancer; // Forward declaration

class LoadBalancerMode {
public:
    virtual ~LoadBalancerMode() = default;
    virtual std::string selectBackend(LoadBalancer& lb, const std::string& clientIP) = 0;
};
