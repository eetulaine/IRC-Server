#pragma once
#include <string>
#include <iostream>

class Server {

	private:
		int	port_;
		std::string password_;

	public:
		Server(int port, std::string password);
		~Server();

		int	getPort() const;
		std::string getPassword() const;

};