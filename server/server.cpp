#include "server.h"

server::server(){
    std::cout << "Inside server constructor" << std::endl;
    listenfd = -1;

    userDB = new sqlDB("userDB.sqlite");
    // create username database and connect to it    
    userDB->openConnection();
    userDB->createTable("users");
}

server::~server(){
    //
    close(listenfd);
    delete userDB;
}

void server::startServer(){

    addrinfo hints;
    addrinfo *result;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;      // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP sockets
    hints.ai_flags = AI_PASSIVE;      // For wildcard IP address 
    hints.ai_protocol = 0;            // Any protocol

    // Either node or service, but not both, may be NULL.
    const char* node = NULL;          // OR "127.0.0.1" OR NULL
    const char* service = "5000";
    int addrInfoStatus;
    if ((addrInfoStatus = getaddrinfo(node, service, &hints, &result)) != 0){
        std::cout << "getaddrinfo: " << gai_strerror(addrInfoStatus) << std::endl;
        exit(EXIT_FAILURE);
    }
    
    // declare listen socket file descriptor 
    addrinfo *temp;
    int bindStatus;

    for(temp = result; temp; temp = result->ai_next){
        //get the file descriptor of the socket on the server side
        if((listenfd = socket(temp->ai_family, temp->ai_socktype, 0)) == -1)
            continue;

        // associate the socket(listenfd) with a port on the Server side
        if((bindStatus = bind(listenfd, temp->ai_addr, temp->ai_addrlen)) == 0)
            break;
        close(listenfd);
    }

    // No address succeeded
    if (temp == NULL){
        perror("Could not connect.\n");
    }
    freeaddrinfo(result);

    // socket listens on specified port for incoming requests and it supports up to inConnectionNum
    int listenStatus;
    int inConnectionNum = 2;
    if((listenStatus = listen(listenfd, inConnectionNum)) != 0){
        std::cout << "Failed to listen\n";
    }else{
        std::cout << "Listening..." << std::endl;
    }
}

int server::connect2Client(){
    // accept will return a new socket(i.e. a different file descriptor). The previous socket(listenfd)
    // is still listening for incoming requests
    int currentfd = -1;
    char ipstr[INET6_ADDRSTRLEN];
    int port;

    struct sockaddr_storage clientInfo;
    socklen_t addrSize = sizeof(clientInfo);
    currentfd = accept(listenfd, (sockaddr*)&clientInfo, &addrSize); // accept awaiting request
    getpeername(currentfd, (struct sockaddr*)&clientInfo, &addrSize);

    // deal with both IPv4 and IPv6:
    if (clientInfo.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&clientInfo;
        port = ntohs(s->sin_port);
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
    } else { // AF_INET6
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&clientInfo;
        port = ntohs(s->sin6_port);
        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
    }
    std::cout << "Connection accepted from "  << ipstr <<  " using port " << port << std::endl;

    return currentfd;
}

void server::wait4Clients(){
    int currentfd;

    while(true){
        std::cout << "Waiting for incoming requests..." << std::endl;
        currentfd = -1;
        currentfd = connect2Client();
        if(currentfd>0){
            std::thread(&server::doOnThread, this, currentfd).detach();
        }else{
            std::cout << "Bad file descriptor." << std::endl;
        }
    }
}

void server::doOnThread(int fd){
    // create a messenger to handle connection between 2 client
    messenger aMessenger(fd);        

    std::unique_lock<std::recursive_mutex> guard(mutex, std::defer_lock);
    // get the client's desired username and password then check if username
    // already exists or not if exists check if the password is right

    std::string incomingMsg;
    std::string username;
    std::string password;
    bool quit = false;
    while(!quit){
        aMessenger.recvFromClient(incomingMsg);    
        splitMsg(incomingMsg, username, password);
        if (username.empty() || password.empty()){
            // sth has gone terribly wrong
            quit = true;
            break;
        }
        aMessenger.setClient(username);
        std::cout << "Username: '" << username << "', Password: '" << password << "'" << std::endl;

        std::cout << "User is online: " << userDB->isOnline(username) << std::endl;
        if(userDB->isUser(username) && !userDB->isOnline(username)){
            std::cout << "Already a User." << std::endl;
            if(userDB->validatePassword(aMessenger, password)){  
                userDB->setUserOnline(aMessenger);
                aMessenger.sendToClient("1");  
                break;   
            }else{
                std::cout << "Wrong password" << std::endl;
                aMessenger.sendToClient("0");    
            }  
        }else if(!userDB->isOnline(username)){
            std::cout << "New User." << std::endl;      
            aMessenger.sendToClient("1");
            userDB->addUser(aMessenger, password);
            break;
        }else if(userDB->isOnline(username)){
            aMessenger.sendToClient("0");
        }
    }

    int nOnlineUsersLocal = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // setup structure for poll()
    struct pollfd ufds[1];
    int rv;
    ufds[0].fd = fd;
    ufds[0].events = POLLIN;

    while(!quit){
        // each thread starts thinking there are no other online users 
        // here, I get the number of global online users and check it against the local `nOnlineUsersLocal` variable
        // and if they are not the same, it means the global `nOnlineUsers` variable is changed on some other thread
        // so I update the local variable here
        guard.lock();
        nOnlineUsers = userDB->getNmbrOfOnlineUsers();
        if (nOnlineUsersLocal != nOnlineUsers){
            std::cout << aMessenger.clientUserName << " nClients: " << nOnlineUsers << " local: " << nOnlineUsersLocal << std::endl;
            // there is a new user connected to the server 
            // send the updated list to the client
            updateUserList(aMessenger);
            nOnlineUsersLocal = nOnlineUsers;
        }
        guard.unlock();

        // wait for events on the sockets, timeout 100ms
        rv = poll(ufds, 1, 100);
        if (rv == -1){
            perror("poll"); // error occurred in poll()
        }else if (rv == 0){
            // std::cout << "Waiting..." << std::endl; 
        }else{  
            if(ufds[0].revents & POLLIN){  
                if(aMessenger.recvFromClient(incomingMsg)){
                    if(!handleRequest(aMessenger, incomingMsg)){
                        std::cout << "Disconnect request." << std::endl;
                        break;   
                    }
                }else{
                    // if recv() returns 0 it means the connection is closed!!
                    std::cout << "Client disconnected." << std::endl;
                    break;
                }
            }
        }
    };

    guard.lock();
    if (userDB->isUser(aMessenger.clientUserName)){
        userDB->setUserOffline(aMessenger.clientUserName);
    }
    guard.unlock();

    std::cout << "Number of online clients: " << userDB->getNmbrOfOnlineUsers() << std::endl;
}

bool server::handleRequest(messenger& aMessenger, std::string msgFrmClient){
    std::string header;
    std::string body;

    splitMsg(msgFrmClient, header, body);

    if(header == "/msg"){
        return handleMsg(aMessenger, body);
    }else if(header == "/cmnd"){
        return handleCmnd(aMessenger, body);
    }else{
        // non compatible header!!
        return false;
    }
}

bool server::handleMsg(messenger& aMessenger, std::string body){
    std::stringstream ss;
    std::string peerUserName;
    std::string msg = body;
    int fd;
    splitMsg(msg, peerUserName, body);

    std::cout << "Message is: " << body << std::endl;
    // find the fd of the peer in database
    fd = userDB->findFD(peerUserName);

    if (fd > 0){
        aMessenger.setPeer(fd, peerUserName);
        std::cout << "UserName is: " << aMessenger.peerUserName << std::endl;
        std::cout << "fd is: " << aMessenger.peerfd << std::endl;
        // send the received message to peer
        ss << "/msg " << aMessenger.clientUserName << " " << body;
        aMessenger.sendToPeer(ss.str());
        return true;

    }else{
        std::cout << "Not a valid peer fd." << std::endl;
        return false;
    }
}

bool server::handleCmnd(messenger& aMessenger, const std::string& msg){
    std::string header;
    std::string body;
    splitMsg(msg, header, body);

    if(header == "/X"){
        std::cout << "/X detected" << std::endl;
        return false;

    }else{
        return true;
    }
}

void server::updateUserList(messenger& aMessenger){
    // get a vector of online users
    onlineUsers = userDB->getOnlineUsers();

    // get number of users
    nOnlineUsers = userDB->getNmbrOfOnlineUsers();

    // make a stringstream of online users seperated by a " "(space)
    std::stringstream ss;
    ss << "/cmnd " << "/updateList " ;
    for(int i=0; i < nOnlineUsers; i++){
        ss << onlineUsers[i] << " ";
    }
    // send usernames of online users, as a stream, to the client
    aMessenger.sendToClient(ss.str());

    std::cout << aMessenger.clientUserName << ": " << ss.str() << std::endl;
}

void splitMsg(const std::string& msg, std::string& header, std::string& body){
    int headerEnd = msg.find(' ');
    header = msg.substr(0, headerEnd);  
    body = msg.substr(headerEnd+1, msg.size());
}