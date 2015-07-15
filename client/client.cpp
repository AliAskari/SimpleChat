#include <cctype>
#include <algorithm>
#include <chrono>
#include <thread>
#include <sys/poll.h>
#include "chatClient.h"
#include "parentWin.h"
#include "childWin.h"

void getUserName(childWin& prompt, std::string& userInput);
void getPassWord(childWin& prompt, std::string& userInput);

bool handleInput(chatClient& aClient, childWin& prompt, childWin& chat, std::string userInput);
void handleSrvrMsg(const std::string& msgFrmSrvr, chatClient& aClient, childWin& chat, childWin& list);
void handleMsg(chatClient& aClient, childWin& chat, std::string& header, std::string& body);
void handleCmnd(chatClient& aClient, childWin& list, std::string& header, std::string& body);

void getPeerID(childWin& prompt, int& peerIDInt, int nUsers);
void split(const std::string &s, char delim, std::vector<std::string> &elems);
void splitMsg(const std::string& msg, std::string& header, std::string& body);

void getOnlineUsers(chatClient& aClient, std::string msg);

void printOnlineUsers(chatClient& aClient, childWin& list);

inline bool isInteger(const std::string& s);

int main(){
    // termWindow win;
    chatClient aClient;
    bool quit = false;

    aClient.connectToServer(5000);

    // create parent and child windows in terminal
    parentWin parent;

    childWin list(parent, 23, 16, 0, parent.x*0.8, FALSE);
    childWin chat(parent, 20, 60, 0, 0, TRUE);
    childWin prompt(parent, 1, 60, parent.y-2, 0, FALSE);
    childWin instruct(parent, 1, 80, parent.y-1, 0, FALSE);

    list.boxOn();
    // chat.boxOn();
    list.setTitle("Online Users");
    instruct.instructions();

    std::string userInput;
    std::string incomingMsg;
    std::string outgoingMsg;
    std::stringstream ss;

    while(true){
    	prompt.clearWin();
	    getUserName(prompt, userInput);  
	    aClient.userName = userInput;

	    getPassWord(prompt, userInput); 

	    // send username and password as a stream to the server
	    ss.str(std::string());
	    ss << aClient.userName << " " << userInput;
	    aClient.sendMsg(ss.str());
	    aClient.recvMsg(incomingMsg);
	    if(incomingMsg == "1"){
	        prompt.clearLine();
	        prompt.print("Welcome to Blah Blah...", FALSE);
	        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	        break;
	    }else if (incomingMsg == "0"){
	    	// incomingMsg == "0";
	    	// user already exist, as for password.
	    	prompt.clearLine();
	    	prompt.print("Wrong Credentials, Try Again!", FALSE);
	    	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	    }
	}

    // a vector for storing online users
    std::vector<std::string> onlineUsers;

    // setup structure for poll() | listening to stdin and socket
    struct pollfd ufds[2];
    int rv;
    ufds[0].fd = aClient.socketfd;
    ufds[0].events = POLLIN;
    ufds[1].fd = fileno(stdin);
    ufds[1].events = POLLIN;

    std::stringstream promptWinMsg;
    std::stringstream chatWinMsg;
    

	while(true){
	    promptWinMsg.str("");
	    promptWinMsg << aClient.userName << ">> ";
        prompt.moveCursor(0, 0);
        prompt.clearLine();
		prompt.print(promptWinMsg.str(), FALSE);
		// wait for events on 2 file descriptors
		rv = poll(ufds, 2, 1000);
		if (rv == -1){
			perror("poll"); // error occurred in poll()
		}else if(rv == 0){
			// std::cout << "\rWaiting..." << std::flush; 
		}else{
			if(ufds[0].revents & POLLIN){
				aClient.recvMsg(incomingMsg);
                handleSrvrMsg(incomingMsg, aClient, chat, list);
			}else if(ufds[1].revents & POLLIN){
				quit = handleInput(aClient, prompt, chat, userInput);
				if (quit){
					break;
				}
			}
		}
	}
	return 0;
}

void getUserName(childWin& prompt, std::string& userInput){
	prompt.clearLine();
	prompt.moveCursor(prompt.y, 0);
    prompt.print("Enter username:", FALSE);
    prompt.getStr(userInput);
}

void getPassWord(childWin& prompt, std::string& userInput){
    	prompt.clearLine();
    	prompt.print("Enter your password: ", FALSE);
	    prompt.getStr(userInput);
    	prompt.refreshWin();
}

bool handleInput(chatClient& aClient, childWin& prompt, childWin& chat, std::string userInput){
	// return false if you still want to get input from client
	// true will break the loop(exit program).
    std::stringstream ss;
    prompt.getStr(userInput);
    if(!userInput.empty() && !aClient.peerUserName.empty() && (userInput != "/X" && userInput != "/S")){
            ss << "/msg " << aClient.peerUserName << " " << userInput; 
            aClient.sendMsg(ss.str());

            ss.str(std::string());
            ss << aClient.userName << ">> " << userInput;
            chat.moveCursor(chat.y, 1);
            chat.print(ss.str(), TRUE);
            ss.str(std::string());
            return false;

    }else if(userInput == "/S"){
        int nUsers = aClient.onlineUsers.size();
        int peerID = 0;
        if (nUsers != 0){

            //wait for user to enter the peer userName to chat
            getPeerID(prompt, peerID, nUsers);

            // set peer username using peerID
            aClient.peerUserName = aClient.onlineUsers[peerID-1];

        }else{
            prompt.print("There are no users online!", FALSE);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        return false;

    }else if(userInput == "/X"){
        prompt.clearLine();
        prompt.print("Quiting...\n", FALSE);
        prompt.refreshWin();

        ss << "/cmnd " << userInput;
        aClient.sendMsg(ss.str());
        ss.str(std::string());
        return true;
    }else{
        return false;
    }
}

void handleSrvrMsg(const std::string& msg, chatClient& aClient, childWin& chat, childWin& list){
    std::string header;
    std::string body;
    splitMsg(msg, header, body);

    if(header == "/msg"){
        handleMsg(aClient, chat, header, body);
    }else if(header == "/cmnd"){
        handleCmnd(aClient, list, header, body);
    }
}

void handleMsg(chatClient& aClient, childWin& chat, std::string& header, std::string& body){
    std::stringstream chatWinMsg;
    std::string msg = body;
    splitMsg(msg, header, body);
    chatWinMsg << header << ">> " << body;
    chat.moveCursor(chat.y, 1);
    chat.print(chatWinMsg.str(), TRUE);
}

void handleCmnd(chatClient& aClient, childWin& list, std::string& header, std::string& body){
    std::string msg = body;
    splitMsg(msg, header, body);

    if(header == "/updateList"){
        // update online user list here
        getOnlineUsers(aClient, body);
        printOnlineUsers(aClient, list);
    }
}

void getPeerID(childWin& prompt, int& peerIDInt, int nUsers){
    std::string peerIDStr;
    while(peerIDInt == 0){
        prompt.print("Enter user number: ", FALSE);
        prompt.getStr(peerIDStr);
        if(isInteger(peerIDStr)){
            peerIDInt = std::stoi(peerIDStr);
            if(peerIDInt > 0 && peerIDInt <= nUsers){
                prompt.clearLine();
            }else{
                peerIDInt = 0;
                prompt.clearLine();
                prompt.print("Not a valid user.", FALSE);
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            }
        }else{
            peerIDInt = 0;
            prompt.clearLine();
            prompt.print("Not a valid number.", FALSE);
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
    }
}

void getOnlineUsers(chatClient& aClient, std::string msg){
    aClient.onlineUsers.clear();
    // split online usernames into the provided vector
    split(msg, ' ', aClient.onlineUsers);
    // remove current user from the vector
    aClient.onlineUsers.erase(std::remove(aClient.onlineUsers.begin(), aClient.onlineUsers.end(), aClient.userName), aClient.onlineUsers.end());

    if(std::find(aClient.onlineUsers.begin(), aClient.onlineUsers.end(), aClient.peerUserName) != aClient.onlineUsers.end()){
    	// peerUserName is online
    }else{
    	// peerUserName is not online
    	aClient.peerUserName.clear();
    }
}

void printOnlineUsers(chatClient& aClient, childWin& list){
	list.clearWin();
	list.boxOn();
	list.setTitle("Online Users");	
    std::stringstream ss;
    int nUsers = aClient.onlineUsers.size();

	list.moveCursor(1, 1);
    for(int i=0; i<nUsers; i++){
        ss << (i+1) << ". " << aClient.onlineUsers[i];
        list.moveCursor(list.y, 1);
        list.print(ss.str(), TRUE);
        // clear the stream
        ss.str(std::string());
    }
}

// helper functions
bool isInteger(const std::string& s){
   if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

   char* p ;
   strtol(s.c_str(), &p, 10) ;

   return (*p == 0) ;
}

void splitMsg(const std::string& msg, std::string& header, std::string& body){
    int headerEnd = msg.find(' ');
    header = msg.substr(0, headerEnd);  
    body = msg.substr(headerEnd+1, msg.size());
}

void split(const std::string &s, char delim, std::vector<std::string> &elems){
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}