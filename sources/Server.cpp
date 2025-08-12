#include "Server.hpp"

Server::Server(int port, std::string password) : port_(port), password_(password) {

}

Server::~Server() {}

int Server::getPort() const {
	return port_;
}

std::string Server::getPassword() const {
	return password_;
}