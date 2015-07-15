#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <iostream>
#include <vector>
#include <poll.h>
#include <mutex>
#include <atomic>
#include <sqlite3.h>
#include "messenger.h"
#include "sqlDB.h"

void splitMsg(const std::string& msg, std::string& header, std::string& body);

class server{
    std::recursive_mutex mutex;
    
    sqlDB* userDB;
    int nOnlineUsers;
    std::vector<std::string> onlineUsers;

    int connect2Client();
    void doOnThread(int fd);
    bool handleRequest(messenger& aMessenger, std::string msgFrmClient);
    bool handleMsg(messenger& aMessenger, std::string body);
    bool handleCmnd(messenger& aMessenger, const std::string& msg);
    void updateUserList(messenger& aMessenger);

public:
    int listenfd;
    server();
    
    void startServer();
    void wait4Clients();

    ~server();
};


#endif //SERVER_SERVER_H
