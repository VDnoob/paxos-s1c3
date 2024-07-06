#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <algorithm>
#include <set>
#include <cstdlib>
#include <ctime>

const int PORT = 8080;
const int CLIENT_COUNT = 3;
const int TIMEOUT_MS = 5000; // 5 seconds

struct PaxosMessage {
    char type[10]; // Use fixed-size char array for serialization
    int proposalID;
};

void handlePaxos(std::vector<int>& clientSockets,int mainProposalID) {
    std::cout << "Handling Paxos..." << std::endl;
    int promiseCount = 0;
    int acceptedCount = 0;
    int proposalID = mainProposalID; // Initialize proposal ID to a random value between 1 and 100
    std::set<int> uniqueProposalIDs; // Set to store unique proposal IDs

    // Send proposal ID to clients
    PaxosMessage proposalMessage;
    strcpy(proposalMessage.type, "PROPOSE");
    proposalMessage.proposalID = proposalID;
    for (int client_socket : clientSockets) {
        std::cout << "Sending proposal to client" << std::endl;
        send(client_socket, &proposalMessage, sizeof(proposalMessage), 0);
        std::cout << "Proposal sent: " << proposalMessage.proposalID << std::endl;
    }

    // Receive promises from clients
    for (int i = 0; i < CLIENT_COUNT; ++i) {
        PaxosMessage response;
        std::cout << "Receiving promise from client " << i+1 << std::endl;
        int bytes_received = recv(clientSockets[i], &response, sizeof(response), 0);
        if (bytes_received < 0) {
            std::cerr << "Error receiving promise from client " << i+1 << std::endl;
            return;
        } else if (bytes_received == 0) {
            std::cerr << "Connection closed by client " << i+1 << std::endl;
            return;
        }
        std::cout << "Received " << bytes_received << " bytes from client " << i+1 << ": " << response.type << ", " << response.proposalID << std::endl;
        if (bytes_received != sizeof(response)) {
            std::cerr << "Incomplete message received from client " << i+1 << std::endl;
            return;
        }
        std::cout << "Received promise from client " << i+1 << ": " << response.type << ", " << response.proposalID << std::endl;
        if (strcmp(response.type, "PROMISE") == 0) {
            // Check if proposal ID is unique
            if (uniqueProposalIDs.find(response.proposalID) == uniqueProposalIDs.end()) {
                uniqueProposalIDs.insert(response.proposalID);
                promiseCount++;
            }
        }
    }

    // If majority of clients haven't promised any ID, proceed
    if (promiseCount >= (CLIENT_COUNT / 2) + 1) {
        std::cout << "Majority promise received. Proceeding to accept phase..." << std::endl;
        // Send accept request to clients
        PaxosMessage acceptMessage;
        strcpy(acceptMessage.type, "ACCEPT");
        acceptMessage.proposalID = proposalID;
        for (int client_socket : clientSockets) {
            std::cout << "Sending accept request to client" << std::endl;
            send(client_socket, &acceptMessage, sizeof(acceptMessage), 0);
        }

        // Receive acceptances from clients
        for (int i = 0; i < CLIENT_COUNT; ++i) {
            PaxosMessage response;
            std::cout << "Receiving acceptance from client " << i+1 << std::endl;
            if (recv(clientSockets[i], &response, sizeof(response), 0) < 0) {
                std::cerr << "Error receiving acceptance from client " << i+1 << std::endl;
                return;
            }
            std::cout << "Received acceptance from client " << i+1 << ": " << response.type << ", " << response.proposalID << std::endl;
            if (strcmp(response.type, "ACCEPTED") == 0 && response.proposalID == proposalID) {
                acceptedCount++;
            }
        }

        // If majority of clients accepted the proposal, proceed to learn phase
        if (acceptedCount >= (CLIENT_COUNT / 2) + 1) {
            std::cout << "Majority acceptance received. Proceeding to learn phase..." << std::endl;
            // Send learn message to clients
            PaxosMessage learnMessage;
            strcpy(learnMessage.type, "LEARN");
            learnMessage.proposalID = proposalID;
            for (int client_socket : clientSockets) {
                std::cout << "Sending learn message to client" << std::endl;
                send(client_socket, &learnMessage, sizeof(learnMessage), 0);
            }
        }
    }
}

void startServer() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, CLIENT_COUNT) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server started. Waiting for connections...\n";

    // Wait for all clients to connect
    PaxosMessage res;
    std::vector<int> clientSockets;
    int mainProposalID = -1;
    while (clientSockets.size() < CLIENT_COUNT) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) >= 0) {
            clientSockets.push_back(new_socket);
            int res_recv = recv(clientSockets[clientSockets.size()-1], &res, sizeof(res), 0);
            mainProposalID = std::max(mainProposalID,res.proposalID);
            std::cout << "Client " << clientSockets.size() << " connected.\n";
        }
    }

    // Start Paxos process
    std::cout << "Starting Paxos...\n";
    handlePaxos(clientSockets,mainProposalID);

    if (new_socket < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
}

int main() {
    srand(time(NULL)); // Seed the random number generator
    startServer();
    return 0;
}
