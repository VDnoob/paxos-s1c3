# Paxos

Paxos is a consensus algorithm used in many distributed networks. It provides a mechanism for the distributed system to continue in some predictable cases of server failures. I have done a basic and a simple implementation to explain the overall working and use of paxos. I have used server-client architecture for this.

## Installation

Download both the codes and open 4 different terminal windows in the folder where you downloaded the code as we are going to have 1 server and 3 clients. Write below lines in each of them.

```bash
g++ server.cpp -o server
./server
```

```bash
g++ client.cpp -o client1
./client1
```
```bash
g++ client.cpp -o client2
./client2
```
```bash
g++ client.cpp -o client3
./client3
```

Make sure you run the server first and then execute the second lines of client parts.
