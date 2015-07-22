# SimpleChat

SimpleChat is a simple chat (duh!!) program for local networks. 
It has been written for learning purposes, and there are probably
many things wrong with it!! 
I am not sure how many simultaneous users it can support, but
I have tried it with up to 20 different clients, and it seems
to work fine without any problems.
It uses Berkeley Socket API for transferring messages using TCP.

# Tips(!)

Find the IP address of the machine running the server. Then go to
chatClient.cpp file , line 25, and change the IP address, so the clients
are connecting to the right server!


# Compiling

For the server make sure to add pthread and lsqlite3 flags, and compile it:

    g++ chatServer.cpp server.cpp sqlDB.cpp messenger.cpp -o chatServer -std=c++11 -pthreads -lsqlite3

For the client I am using ncurses library, so just add the ncurses flag:

    g++ client.cpp chatClient.cpp parentWin.cpp childWin.cpp -o client -std=c++11 -lncurses
