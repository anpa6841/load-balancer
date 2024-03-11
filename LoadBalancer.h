#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <functional>

#include "LoadBalancerMode.h"

class LoadBalancer {
private:
    std::vector<std::string> backendServers;
    std::unique_ptr<LoadBalancerMode> mode;
    std::mutex mutex;

public:
    LoadBalancer(std::unique_ptr<LoadBalancerMode> mode);

    std::string selectBackend(LoadBalancer& lb, const std::string& clientIP);
    std::vector<std::string> getBackendServers();
    std::string getNextBackend(const std::string& clientIP);
    void addBackendServer(const std::string &server);
};
