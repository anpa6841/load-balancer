#pragma once

#include<random>
#include "LoadBalancer.h"

class RandomMode : public LoadBalancerMode {
public:
    std::string selectBackend(LoadBalancer& lb, const std::string& clientIP) override;
};
