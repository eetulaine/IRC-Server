#pragma once

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include "../includes/macros.hpp"

class Client {

	private:
		int clientFD_;
		std::string readBuffer_;
		std::string sendBuffer_;
		std::string nickname_;
		std::string username_;
		std::string hostname_;
		std::string realName_;
		std::string password_;
		bool isAuthenticated_;

		// PRIVATE MEMBER FUNCTIONS
		void authenticateClient();

	public:
		Client(int clientFD, std::string clientIP);
		~Client();

		// PUBLIC MEMBER FUNCTIONS
		int receiveData();

		// ACCESSORS
		int getClientFD() const;
		std::string getHostname() const;
		std::string getNickname() const;
		std::string getUsername() const;
		std::string getRealName() const;
		std::string getPassword() const;

		void setHostname(std::string hostname);
		void setNickname(std::string nickname);
		void setUsername(std::string username);
		void setRealName(std::string realName);
		void setPassword(std::string password);
};