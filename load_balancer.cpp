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
    virtual std::string selectBackend(LoadBalancer& lb) = 0;
    virtual ~LoadBalancerMode() {};
};

class LoadBalancer {
private:
    std::vector<std::string> backendServers;
    std::unique_ptr<LoadBalancerMode> mode;
    std::mutex mutex;

public:
    LoadBalancer(std::unique_ptr<LoadBalancerMode> mode) : mode(std::move(mode)) {}

    void addBackendServer(const std::string &server) {
        backendServers.push_back(server);
    }

    std::string selectBackend(LoadBalancer& lb) {
        // Lock to ensure thread safety
        std::lock_guard<std::mutex> lock(mutex);
        return mode -> selectBackend(*this);
    }

    std::vector<std::string> getBackendServers() {
        return backendServers;
    }

    std::string getNextBackend() {
        return selectBackend(*this);
    }
};


// Round Robin Load Balancer Mode
class RoundRobin: public LoadBalancerMode {
private:
    size_t currentIndex;

public:
    RoundRobin() : currentIndex(-1) {}

    std::string selectBackend(LoadBalancer& lb) override {
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
    WeightedRoundRobin() : currentIndex(-1) {}

    void add_server(const std::string& server, int weight) {
        servers.push_back(server);
        weights.push_back(weight);
        originalWeights.push_back(weight);
    }

    std::string selectBackend(LoadBalancer& lb) override {
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


void handleConnection(int clientSocket, LoadBalancer& lb) {
    std::string backendServer = lb.getNextBackend();
    std::cout << "Routing request to backend server: " << backendServer << std::endl;
    close(clientSocket);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [mode]" << std::endl;
        std::cout << "Available modes: RR (Round Robin), WRR (Weighted Round Robin)" << std::endl;
        return 1;
    }

    std::string modeStr = argv[1];
    std::unique_ptr<LoadBalancerMode> mode;

    if (modeStr == "RR") {
        mode = std::make_unique<RoundRobin>();
    } else if (modeStr == "WRR") {
        mode = std::make_unique<WeightedRoundRobin>();
        // Add weights for the backend servers
        dynamic_cast<WeightedRoundRobin *>(mode.get()) -> add_server("127.0.0.1:8081", 3);
        dynamic_cast<WeightedRoundRobin *>(mode.get()) -> add_server("127.0.0.1:8082", 1);
        dynamic_cast<WeightedRoundRobin *>(mode.get()) -> add_server("127.0.0.1:8083", 2);
    } else {
        std:: cerr << "Invalid mode. Available modes: RR (Round Robin), WRR (Weighted Round Robin)" << std::endl;
        return 1;
    }

    // Create Load Balancer with specified mode
    LoadBalancer lb(std::move(mode));

    // Add backend servers
    lb.addBackendServer("127.0.0.1:8081");
    lb.addBackendServer("127.0.0.1:8082");
    lb.addBackendServer("127.0.0.1:8083");

    // Create a socket for accepting incoming client connections
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error: Failed to create server socket" << std::endl;
        return 1;
    }

    // Bind the server socket to a port
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);
    int opt = 1;

    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options\n";
        return 1;
    }

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        std::cerr << "Error: Failed to bind server socket" << std::endl;
        close(serverSocket);
        return 1;
    }

    // Listen for incoming client connections
    if (listen(serverSocket, SOMAXCONN) == -1) {
        std::cerr << "Error failed to listen for client connections" << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Load balancer server running..." << std::endl;

    // Accept incoming connections and handle them in separate thread
    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
        if (clientSocket == -1) {
            std::cerr << "Error: Failed to accept client connection" << std::endl;
            continue;
        }

        // Handle the client connection in a separate thread
        std::thread(handleConnection, clientSocket, std::ref(lb)).detach();
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}