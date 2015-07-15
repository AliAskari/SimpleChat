#include "messenger.h"

messenger::messenger(int socketfd){
	clientfd = socketfd;
	peerfd = -1;

	clientUserName = "";
	peerUserName = "";
}

messenger::~messenger(){
	//
	close(clientfd);
}

void messenger::setClient(const std::string& userName){
	//
	clientUserName = userName;
}

void messenger::setPeer(int fd, std::string userName){
	peerfd = fd;
	peerUserName = userName;
}

bool messenger::sendToPeer(std::string msg){
	// send message to the peer
	if (sendMsg(peerfd, msg)){
		// message was successfully sent
		return true;
	}else{
		// there was sth wrong with the connection
		return false;
	}
}

bool messenger::sendToClient(std::string msg){
	if (sendMsg(clientfd, msg)){
		return true;
	}else{
		return false;
	}
}

bool messenger::recvFromClient(std::string& msg){
	if (recvMsg(clientfd, msg)){
		return true;
	}else{
		return false;
	}
}

bool messenger::recvFromPeer(std::string& msg){
	if (recvMsg(peerfd, msg)){
		return true;
	}else{
		return false;
	}
}

int messenger::sendMsg(int fd, std::string msg){
	int sentBytes;
	if((sentBytes = send(fd, msg.c_str(), strlen(msg.c_str()), 0)) == -1){
	    perror("send");
	}
	return sentBytes;
}

int messenger::recvMsg(int fd, std::string& msg){
    char fromClient[1400];
    int recBytes;

    // per the following post on stackoverflow one can use socket API calls send() recv()
    // instead of the read() and write() almost interchangebly if the flag is et to zero.
    // stackoverflow.com/q/1790750/2625036   ||| stackoverflow.com/q/4778043/2625036
    if((recBytes = recv(fd, fromClient, sizeof(fromClient), 0)) == -1){
        perror("recv");
    }
    fromClient[recBytes] = 0;
    msg = fromClient;
    return recBytes;
}