#pragma once

#include "LoadBalancer.h"

// IPHashing Load Balancer Mode
class IPHashingMode: public LoadBalancerMode {
private:
    // Maps client IP to backend server index
    std::unordered_map<std::string, size_t> clientToServerMap;
public:
    size_t hashIPv4(const std::string& ipAddress) const;
    std::string selectBackend(LoadBalancer& lb, const std::string& /* clientIP */) override;
};