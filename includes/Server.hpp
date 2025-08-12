#pragma once

#include <string>
#include <iostream>
#include <sys/socket.h> //-> needed for socket
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

	public:
		Server(int port, std::string password);
		~Server();

		void initAddrInfo();
		void createAddrInfo();
		void createServSocket();
		void setNonBlocking();

		int	getPort() const;
		int getServerSocket() const;
		std::string getPassword() const;
};