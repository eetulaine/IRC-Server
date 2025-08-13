#pragma once

#include <string>
#include <iostream>
#include <sys/socket.h> //-> needed for socket
#include <sys/epoll.h> //-> needed for epoll
#include <netdb.h>      //-> needed for addrinfo
#include <cstring>		//-> needed for memset etc.
#include <unistd.h>		//-> needed for close etc.
#include <fcntl.h>		//-> needed for fcntl

class Server {

	private:
		int	port_;
		std::string password_;
		int serverSocket_;
		struct addrinfo hints_, *res_;
		//static bool isRunning_; // change the value to true when it start

	public:
		Server(int port, std::string password);
		~Server();

		void initAddrInfo(); 		//-> init addrinfo struct settings
		void createAddrInfo(); 		//-> call getaddrinfo
		void createServSocket(); 	//-> create socket
		void setNonBlocking(); 		//-> get & set socket status flags using fcntl
		void startServer();			//-> The loop, that will keep the server running and do diff actions

		int	getPort() const;
		int getServerSocket() const;
		std::string getPassword() const;
};
