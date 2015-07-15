#include "server.h"
#include <signal.h>


int main(){

	server aServer;	
	std::cout << "Server is created" << std::endl;
	aServer.startServer();
	aServer.wait4Clients();

	return 0;
}
