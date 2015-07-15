#include <iostream>
#include <sstream>
#include "sqlDB.h"

sqlDB::sqlDB(std::string dbName){
    this->dbName = dbName;
    isOpen = false;
}

bool sqlDB::openConnection(){
    if((sqlStatus = sqlite3_open(dbName.c_str(), &database)) != SQLITE_OK){
        std::cout << "Cannot open database: " << sqlite3_errmsg(database) << std::endl;
        sqlite3_close(database);
        isOpen = false;
        return false;
    }

	isOpen = true;

    // TO BE DELETED
    errMsg = sqlite3_errmsg(database);
    if(errMsg != "not an error"){
    	std::cout << "After DB open: " << errMsg << std::endl;
    	return false;
    }
    // TO BE DELETED

    return true;
}

bool sqlDB::createTable(const std::string& tableName){
	this->tableName = tableName;
	std::stringstream query;
	query << "CREATE TABLE " << tableName << " (fd INTEGER, username TEXT, password TEXT, UNIQUE (username));";

	if(isOpen){
		if(sqlite3_prepare_v2(database, query.str().c_str(), -1, &statement, 0) == SQLITE_OK){
		    sqlStatus = sqlite3_step(statement);
		    sqlite3_finalize(statement);
		    errMsg = sqlite3_errmsg(database);
		    if(errMsg != "not an error"){
		    	std::cout << errMsg << std::endl;	
		    	return false;
		    } 
		}
	}
	return true;
}

bool sqlDB::addUser(const messenger& aMessenger, const std::string& password){
	std::stringstream query;
    query << "INSERT INTO " << tableName << " VALUES (" << aMessenger.clientfd 
    											<< ",'" << aMessenger.clientUserName << "'"
										        << ",'" << password << "');";

    if(sqlite3_prepare_v2(database, query.str().c_str(), -1, &statement, 0) == SQLITE_OK){
        if ((sqlite3_step(statement) != SQLITE_DONE) && (sqlite3_step(statement) != SQLITE_CONSTRAINT)){
            std::cout << "SQL error: " << sqlite3_errmsg(database) << std::endl;
            close();
            return false;
        }else if(sqlite3_step(statement) == SQLITE_CONSTRAINT){
        	return false;
        }
    }else{
	    errMsg = sqlite3_errmsg(database);
	    if(errMsg != "not an error") std::cout << "Error: " << errMsg << std::endl;
		close();
		return false;
    }

    sqlite3_finalize(statement);
    return true;
}

bool sqlDB::validatePassword(const messenger& aMessenger, const std::string& password){
	std::stringstream query;
	query << "SELECT password FROM " << tableName << " WHERE username = '" << aMessenger.clientUserName << "';";

	std::string clientPassword;
  	int result = -1;

	if(isOpen){
	    if(sqlite3_prepare_v2(database, query.str().c_str(), -1, &statement, 0) == SQLITE_OK){
    		result = sqlite3_step(statement);
	        if (result == SQLITE_ROW) {
        		clientPassword = (const char*)(sqlite3_column_text(statement, 0)); 
    			if (clientPassword == password){
    				return true;
    			}else{
    				return false;
    			}
	        }
	    }
	}
	return false;
}

std::vector<std::string> sqlDB::getOnlineUsers(){
	std::vector<std::string> onlineUsers;
	char *err_msg = 0;
	std::stringstream query;
	query << "SELECT username FROM " << tableName << " WHERE fd > 0;";

  	std::string aUser;
  	int result = 1;
  	int nRows = 0;

	if(isOpen){
	    if(sqlite3_prepare_v2(database, query.str().c_str(), -1, &statement, 0) == SQLITE_OK){

	    	while(true){
	    		result = sqlite3_step(statement);
		        if (result == SQLITE_ROW) {
	        		aUser = (const char*)(sqlite3_column_text(statement, 0)); 
	    			if (!aUser.empty()){
	    				onlineUsers.push_back(aUser);
	    				nRows++;
	    			}
		        }else{
		        	// no more valid result
		        	break;
		        }
		    }
	    }
	}
	return onlineUsers;
}

bool sqlDB::isOnline(const std::string& aUserName){
	int fd = findFD(aUserName);
	std::cout << "the fd is: " << fd << std::endl;
	if (fd > 0){
		return true;
	}else{
		return false;
	}
}

bool sqlDB::isUser(const std::string& aUserName){
	char *err_msg = 0;
	std::stringstream query;
	query << "SELECT username FROM " << tableName << " WHERE username = '" << aUserName << "';";

	if(isOpen){
	    if(sqlite3_prepare_v2(database, query.str().c_str(), -1, &statement, 0) == SQLITE_OK){
	        if (sqlite3_step(statement) == SQLITE_ROW){
	        	std::cout << "is a user." << std::endl;
	        	return true;
		    }else{
		    	std::cout << "Not a user." << std::endl;
		    	return false;
		    }
	    }
	    std::cout << "Error: " << sqlite3_errmsg(database);
	    return false;
	}
	std::cout << "No valid connection." << std::endl;
	return false;
}

int sqlDB::findFD(const std::string& aUserName){
 	std::string fd;
	char *err_msg = 0;
	int status;
	std::stringstream query;
	query << "SELECT fd FROM " << tableName << " WHERE username = '" << aUserName << "';";

	int cols;
	if(isOpen){
	    if(sqlite3_prepare_v2(database, query.str().c_str(), -1, &statement, 0) == SQLITE_OK){
	    	status = sqlite3_step(statement);
	        if (status == SQLITE_ROW){
	        	fd = (const char*)(sqlite3_column_text(statement, 0));
		    }else{
		    	return -1;
		    }
	    }
	}
	return std::stoi(fd);
}

bool sqlDB::setUserOffline(const std::string& aUserName){
	std::stringstream query;
	query << "UPDATE " << tableName << " SET " << "fd = -1 WHERE username = '" << aUserName << "';";

	if(isOpen){
	    if(sqlite3_prepare_v2(database, query.str().c_str(), -1, &statement, 0) == SQLITE_OK){
	    	if (sqlite3_step(statement) != SQLITE_DONE){
	    	    std::cout << "SQL error: " << sqlite3_errmsg(database) << std::endl;
	    	    close();
	    	    return false;
		    }else{
		    	return true;
		    }
		    std::cout << "Error: " << sqlite3_errmsg(database);
		    return false;
		}
	}
	std::cout << "No valid connection." << std::endl;
	return false;
}

bool sqlDB::setUserOnline(const messenger& aMessenger){
	std::stringstream query;
	query << "UPDATE " << tableName << " SET " << "fd = " << aMessenger.clientfd << " WHERE username = '" << aMessenger.clientUserName << "';";

	if(isOpen){
	    if(sqlite3_prepare_v2(database, query.str().c_str(), -1, &statement, 0) == SQLITE_OK){
	    	if (sqlite3_step(statement) != SQLITE_DONE){
	    	    std::cout << "SQL error: " << sqlite3_errmsg(database) << std::endl;
	    	    close();
	    	    return false;
		    }else{
		    	return true;
		    }
		    std::cout << "Error: " << sqlite3_errmsg(database);
		    return false;
		}
	}
	std::cout << "No valid connection." << std::endl;
	return false;
}

int sqlDB::getNmbrOfOnlineUsers(){
	std::stringstream query;
	query << "SELECT username FROM " << tableName << " WHERE fd > 0;";

  	int result = 1;
  	int nRows = 0;

	if(isOpen){
	    if(sqlite3_prepare_v2(database, query.str().c_str(), -1, &statement, 0) == SQLITE_OK){
	    	while(true){
	    		result = sqlite3_step(statement);
		        if (result == SQLITE_ROW) {
		        	nRows++;
		        }else{
		        	// no more valid result
		        	break;
		        }
		    }
	    }
	}
	return nRows;
}

void sqlDB::close(){
	isOpen = false;
    sqlite3_close(database);
}

sqlDB::~sqlDB(){
	//
    close();
}