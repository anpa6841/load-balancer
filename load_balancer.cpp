#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


// Number of backend servers
const int NUM_BACKEND_SERVERS = 3;

class LoadBalancer;


class LoadBalancerMode {
public:
    virtual std::string selectBackend(LoadBalancer &lb) = 0;
    virtual ~LoadBalancerMode() {};
};


class LoadBalancer {
private:
    std::vector<std::string> backendServers;
    LoadBalancerMode *mode;
    std::mutex mutex;

public:
    LoadBalancer(LoadBalancerMode *mode) : mode(mode) {}

    void addBackendServer(const std::string &server) {
        backendServers.push_back(server);
    }

    std::string selectBackend() {
        // Lock to ensure thread safety
        std::lock_guard<std::mutex> lock(mutex);
        return mode -> selectBackend(*this);
    }

    std::vector<std::string> getBackendServers() {
        return backendServers;
    }
};


// Round Robin Load Balancer Mode
class RoundRobin: public LoadBalancerMode {
private:
    size_t currentIndex;

public:
    RoundRobin() : currentIndex(-1) {}

    std::string selectBackend(LoadBalancer &lb) {
        std::vector<std::string> backendServers = lb.getBackendServers();
        size_t index = currentIndex++ % backendServers.size();
        return backendServers[index];
    }
};


// Weighted Round Robin Load Balance Mode
class WeightedRoundRobin: public LoadBalancerMode {
private:
    std::vector<std::string> servers;
    std::vector<int> weights;
    std::vector<int> originalWeights;
    size_t currentIndex;

public:
    WeightedRoundRobin() : currentIndex(0) {}

    void add_server(const std::string& server, int weight) {
        servers.push_back(server);
        weights.push_back(weight);
        originalWeights.push_back(weight);
    }

    std::string selectBackend(LoadBalancer &lb) {
        if (servers.empty()) {
            return "No server available"; // No servers added
        }

        // Find the next available server with non-zero weight
        for (size_t i = 0; i < servers.size(); i++) {
            currentIndex = (currentIndex + 1) % servers.size();
            if (weights[currentIndex] > 0) {
                --weights[currentIndex]; // Decrement weight
                return servers[currentIndex]; // Return server name
            }
        }

        // If all servers have weight 0, reset the weights and return the first server
        reset_weights();
        return servers[currentIndex];
    }

private:
    void reset_weights() {
        std::copy(originalWeights.begin(), originalWeights.end(), weights.begin());
    }
};


// Thread function for simulating client requests
void clientThread(LoadBalancer &lb, int threadId) {
    std::string server = lb.selectBackend();
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate processing time
    std::cout << "Thread " << threadId << ": Selected backend server: " << server << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [mode]" << std::endl;
        std::cout << "Available modes: RR (Round Robin), WRR (Weighted Round Robin)" << std::endl;
        return 1;
    }

    std::string modeStr = argv[1];
    LoadBalancerMode *mode = nullptr;

    if (modeStr == "RR") {
        mode = new RoundRobin();
    } else if (modeStr == "WRR") {
        mode = new WeightedRoundRobin();
        // Add weights for the backend servers
        dynamic_cast<WeightedRoundRobin *>(mode) -> add_server("Server A", 3);
        dynamic_cast<WeightedRoundRobin *>(mode) -> add_server("Server B", 1);
        dynamic_cast<WeightedRoundRobin *>(mode) -> add_server("Server C", 2);
    } else {
        std:: cerr << "Invalid mode. Available modes: RR (Round Robin), WRR (Weighted Round Robin)" << std::endl;
        return 1;
    }

    // Create Load Balancer with specified mode
    LoadBalancer lb(mode);

    // Add backend servers
    lb.addBackendServer("Server A");
    lb.addBackendServer("Server B");
    lb.addBackendServer("Server C");

    // Create client threads to simulate requests
    std::vector<std::thread> threads;
    for (int i = 0; i < 7; i++) {
        threads.emplace_back(clientThread, std::ref(lb), i + 1);
    }

    // Wait for all threads to finish
    for (auto &thread: threads) {
        thread.join();
    }

    delete mode; // Free allocated memory

    return 0;
}