#pragma once

#include <string>
#include <iostream>
#include <sys/socket.h> //-> needed for socket
#include <sys/epoll.h>	//-> needed for epoll
#include <netdb.h>      //-> needed for addrinfo
#include <cstring>		//-> needed for memset etc.
#include <unistd.h>		//-> needed for close etc.
#include <fcntl.h>		//-> needed for fcntl
#include <arpa/inet.h>	// Client IP  log "inet_ntop()"
#include <map>
#include <memory>		// for std::unique_ptr
#include "../includes/macros.hpp"
#include "../includes/Client.hpp"

class Client;
class Server {

	private:
		int	port_;
		std::string password_;
		int serverSocket_;
		struct addrinfo hints_, *res_;
		//static bool isRunning_; // change the value to true when it start

		// private member functions used for the server setup within the Server constructor
		void initAddrInfo(); 		//-> init addrinfo struct settings
		void createAddrInfo(); 		//-> call getaddrinfo
		void createServSocket(); 	//-> create socket
		void setNonBlocking(); 		//-> set socket status flags to non-blocking using fcntl
		void setSocketOption();		//-> set socket option to reuse address to avoid "address already in use" error
		void bindSocket();			//-> bind the socket to the address
		void initListen();			//-> prepare to listen for incoming connections

		// dependant methods for "ServerActivity"
		void acceptNewClient(int epollFd); //-> accept new client request
		std::string getClientIP(struct sockaddr_in clientSocAddr);
		void receiveData(int currentFD, int epollFD);
		void sendData(int currentFD);
		std::map<int, std::unique_ptr<Client>> clients_;

	public:
		Server(int port, std::string password);
		~Server();


		// ServerActivity: activity response loop for running server
		void startServer();			//-> The loop, that will keep the server running and do diff actions

		int	getPort() const;
		int getServerSocket() const;
		std::string getPassword() const;
};
