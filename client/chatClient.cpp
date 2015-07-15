#include "chatClient.h"


chatClient::chatClient(){
    //
    socketfd = -1;
}

chatClient::~chatClient(){
    //
    close(socketfd);
}

void chatClient::connectToServer(int portNumber){
    addrinfo hints;
    addrinfo *result;

    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;      // Allow IPv4 or IPv6 
    hints.ai_socktype = SOCK_STREAM;  // Stream socket 
    hints.ai_flags = AI_PASSIVE;      // The returned socket addresses will be suitable for use with connect(
    hints.ai_protocol = 0;            // Any protocol 

    //Either node or service, but not both, may be NULL.
    const char* node = "192.168.1.49";        // IP address of the server

    std::string port = std::to_string(portNumber);
    const char* service = port.c_str();
    int addrInfoStatus;
    if ((addrInfoStatus = getaddrinfo(node, service, &hints, &result)) != 0){
        std::cout << "getaddrinfo: " << gai_strerror(addrInfoStatus) << std::endl;
        exit(EXIT_FAILURE);
    }

    // getaddrinfo() returns a list of address structures.
    // Try each address until we successfully connect(2).
    // If socket(2) (or connect(2)) fails, we (close the socket
    // and) try the next address. 

    addrinfo *temp;
    int connectStatus;

    for(temp = result; temp; temp = result->ai_next){

        //get the file descriptor of the socket on the client side
        if((socketfd = socket(temp->ai_family, temp->ai_socktype, 0)) == -1)
            continue;

        // There is no need to bind the socket to a port on the client side

        // send connect request to the server and wait for a response
        if((connectStatus = connect(socketfd, temp->ai_addr, temp->ai_addrlen)) != -1)
            break;
        close(socketfd);
    }

    // No address succeeded
    if (temp == NULL){
        perror("Could not connect.\n");
    }
    freeaddrinfo(result);
}

int chatClient::sendMsg(std::string msg){
    int sentBytes;
    if((sentBytes = send(socketfd, msg.c_str(), strlen(msg.c_str()), 0)) == -1){
        perror("send");
    }
    return sentBytes;
}

int chatClient::recvMsg(std::string& msg){
    // receive message from client
    char fromClient[100];
    int recBytes;

    // per the following posts on stackoverflow one can use socket API calls send() recv()
    // instead of the read() and write() almost interchangebly if the flag is set to zero.
    // stackoverflow.com/q/1790750/2625036   ||| stackoverflow.com/q/4778043/2625036
    if((recBytes = recv(socketfd, fromClient, sizeof(fromClient), 0)) == -1){
        perror("recv");
    }
    fromClient[recBytes] = 0;
    msg = fromClient;
    return recBytes;
}