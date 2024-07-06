#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h> // Include for memset
#include <cstdlib> // Include for rand() and srand()
#include <ctime> // Include for time()
#include <netinet/in.h>

const int PORT = 8080;

struct PaxosMessage {
    char type[10]; // Use fixed-size char array for serialization
    int proposalID;
};

void startClient() {
    static int lastProposalID = 0; // Initialize static variable to keep track of last proposal ID sent
    int proposalID = ++lastProposalID; // Increment last proposal ID and use it as the current proposal ID

    srand(time(NULL)); // Seed the random number generator with the current time
    proposalID = rand() % 100 + 1; // Generate a random proposal ID between 1 and 100

    int sock = 0;
    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr)); // Initialize serv_addr with zeros

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "\n Socket creation error \n";
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        std::cout << "\nInvalid address/ Address not supported \n";
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "\nConnection Failed \n";
        return;
    }

    // Send proposal ID to server
    PaxosMessage proposalMessage;
    strcpy(proposalMessage.type, "PROPOSE");
    proposalMessage.proposalID = proposalID;
    if(send(sock, &proposalMessage, sizeof(proposalMessage), 0) < 0) {
        std::cout << "Send failed\n";
        return;
    }
    std::cout << "Proposal sent: " << proposalMessage.proposalID << std::endl;

    // Receive accept request from server
    PaxosMessage acceptRequest;
    if(recv(sock, &acceptRequest, sizeof(acceptRequest), 0) < 0) {
        std::cout << "Receive failed\n";
        return;
    }
    std::cout << "Received accept request for ID: " << acceptRequest.proposalID << std::endl;

    // Check if accept request matches proposed ID
    if (acceptRequest.proposalID == proposalID) {
        // Send acceptance to server
        PaxosMessage acceptedMessage;
        strcpy(acceptedMessage.type, "ACCEPTED");
        acceptedMessage.proposalID = proposalID;
        if(send(sock, &acceptedMessage, sizeof(acceptedMessage), 0) < 0) {
            std::cout << "Send failed\n";
            return;
        }
    }

    close(sock);
}

int main() {
    startClient();
    return 0;
}
