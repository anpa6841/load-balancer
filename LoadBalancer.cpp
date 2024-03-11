#include <thread>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "LoadBalancer.h"
#include "RoundRobinMode.h"
#include "WeightedRoundRobinMode.h"
#include "IPHashingMode.h"
#include "RandomMode.h"

LoadBalancer::LoadBalancer(std::unique_ptr<LoadBalancerMode> mode) : mode(std::move(mode)) {
    // Parametrized Contructor implementation
}

void LoadBalancer::addBackendServer(const std::string &server) {
    backendServers.push_back(server);
}

std::string LoadBalancer::selectBackend(LoadBalancer& lb, const std::string& clientIP) {
    // Lock to ensure thread safety
    std::lock_guard<std::mutex> lock(mutex);
    return mode->selectBackend(*this, clientIP);
}

std::vector<std::string> LoadBalancer::getBackendServers() {
    return backendServers;
}

std::string LoadBalancer::getNextBackend(const std::string& clientIP) {
    return selectBackend(*this, clientIP);
}


void handleConnection(int clientSocket, LoadBalancer& lb, const std::string& clientIP) {
    std::string backendServer = lb.getNextBackend(clientIP);
    std::cout << "Routing request to backend server: " << backendServer << std::endl;
    close(clientSocket);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [mode]" << std::endl;
        std::cout << "Available modes: RR (Round Robin), WRR (Weighted Round Robin), IPH (IP Hashing), Random (RA)" << std::endl;
        return 1;
    }

    std::string modeStr = argv[1];
    std::unique_ptr<LoadBalancerMode> mode;

    if (modeStr == "RR") {
        mode = std::make_unique<RoundRobinMode>();
    } else if (modeStr == "WRR") {
        mode = std::make_unique<WeightedRoundRobinMode>();
        // Add weights for the backend servers
        dynamic_cast<WeightedRoundRobinMode *>(mode.get()) -> add_server("127.0.0.1:8081", 3);
        dynamic_cast<WeightedRoundRobinMode *>(mode.get()) -> add_server("127.0.0.1:8082", 1);
        dynamic_cast<WeightedRoundRobinMode *>(mode.get()) -> add_server("127.0.0.1:8083", 2);
    } else if (modeStr == "IPH") {
        mode = std::make_unique<IPHashingMode>();
    } else if (modeStr == "RA") {
        mode = std::make_unique<RandomMode>();
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
        std::thread(handleConnection, clientSocket, std::ref(lb), inet_ntoa(clientAddr.sin_addr)).detach();
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}