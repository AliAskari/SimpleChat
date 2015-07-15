#ifndef SQLDB_SQLDB_H
#define SQLDB_SQLDB_H

#include <vector>
#include <sqlite3.h>
#include "messenger.h"

class sqlDB{
    sqlite3* database;
    sqlite3_stmt *statement;
    std::string dbName;
    std::string tableName;
    std::string errMsg;
    int sqlStatus;
    bool isOpen;

public:
    sqlDB(std::string);
    ~sqlDB();
    bool openConnection();
    bool createTable(const std::string& tableName);
    bool addUser(const messenger& aMessenger, const std::string& password);
    bool isUser(const std::string& aUserName);
    bool isOnline(const std::string& aUserName);
    bool validatePassword(const messenger& aMessenger, const std::string& password);
    int findFD(const std::string& aUserName);
    bool setUserOffline(const std::string& aUserName);
    bool setUserOnline(const messenger& aMessenger);
    std::vector<std::string> getOnlineUsers();
    int getNmbrOfOnlineUsers();
    void close();
};


#endif //SQLDB_SQLDB_HPP
