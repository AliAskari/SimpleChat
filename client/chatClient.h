#ifndef CLIENT_CHATCLIENT_H
#define CLIENT_CHATCLIENT_H

#include <iostream>
#include <arpa/inet.h>  // for inet_ntop function
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <thread>
#include <sstream>
#include <vector>
#include <netdb.h>


class chatClient{

public:
    int socketfd;
    std::string userName;
    std::string peerUserName;
    std::vector<std::string> onlineUsers;

    chatClient();
    ~chatClient();
    void connectToServer(int portNumber);
    int sendMsg(std::string msg);
    int recvMsg(std::string& msg);
};

#endif //CLIENT_CHATCLIENT_H
