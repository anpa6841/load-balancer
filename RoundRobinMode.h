#pragma once

#include "LoadBalancer.h"

class RoundRobinMode : public LoadBalancerMode {
private:
    size_t currentIndex;
public:
    RoundRobinMode();
    std::string selectBackend(LoadBalancer& lb, const std::string& clientIP) override;
};
