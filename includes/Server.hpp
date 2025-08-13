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

		// private member functions used for the server setup within the Server constructor
		void initAddrInfo(); 		//-> init addrinfo struct settings
		void createAddrInfo(); 		//-> call getaddrinfo
		void createServSocket(); 	//-> create socket
		void setNonBlocking(); 		//-> set socket status flags to non-blocking using fcntl
		void setSocketOption();		//-> set socket option to reuse address to avoid "address already in use" error
		void bindSocket();			//-> bind the socket to the address
		void initListen();			//-> prepare to listen for incoming connections

	public:
		Server(int port, std::string password);
		~Server();

		int	getPort() const;
		int getServerSocket() const;
		std::string getPassword() const;
};