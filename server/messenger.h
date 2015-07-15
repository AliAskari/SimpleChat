#ifndef MESSENGER_MESSENGER_H
#define MESSENGER_MESSENGER_H

#include <iostream>
#include <arpa/inet.h>  // for inet_ntop function
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <thread>
#include <sstream>
#include <netdb.h>


class messenger{
public:    
    int clientfd;
    std::string clientUserName;

    int peerfd;    
    std::string peerUserName;

    messenger(int clientfd);
    ~messenger();

    // probably these should be private!
    int sendMsg(int fd, std::string msg);
    int recvMsg(int fd, std::string& msg);

    void setClient(const std::string& userName);
    void setPeer(int fd, std::string userName);

    bool sendToPeer(std::string msg);
    bool sendToClient(std::string msg);
    bool recvFromPeer(std::string& msg);
    bool recvFromClient(std::string& msg);
};

#endif //MESSENGER_MESSENGER_H
