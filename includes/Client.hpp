#pragma once

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdint> // 'uint32_t'
#include "../includes/macros.hpp"
#include "../includes/Server.hpp"

class Server;

class Client {

	private:
		int clientFD_;
		int epollFd_;
		std::string readBuffer_;
		std::string sendBuffer_;
		std::string nickname_;
		std::string username_;
		std::string hostname_;
		std::string realName_;
		std::string password_;
		bool authenticated_;
		bool connected_;

		// PRIVATE MEMBER FUNCTIONS
		bool isSocketValid() const;

	public:
		Client(int clientFD, std::string clientIP, int epollFd);
		~Client();

		// PUBLIC MEMBER FUNCTIONS
		int receiveData();
		bool sendData();
		void addReadToBuffer(const std::string& received);

		// ACCESSORS
		int getClientFD() const;
		int getEpollFd() const;
		std::string getHostname() const;
		std::string getNickname() const;
		std::string getUsername() const;
		std::string getRealName() const;
		std::string getPassword() const;
		std::string getReadBuffer() const;
		bool isConnected() const;
		bool isAuthenticated();

		void setHostname(std::string hostname);
		void setNickname(std::string nickname);
		void setUsername(std::string username);
		void setRealName(std::string realName);
		void setPassword(std::string password);
		void setBuffer(std::string buffer);
		void setConnected(bool connected);
		void setAuthenticated(bool authenticated);
		void appendSendBuffer(std::string sendMsg);
		void epollEventChange(uint32_t eventType); // any better name??
};
