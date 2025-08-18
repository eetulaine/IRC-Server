#include "../includes/Client.hpp"

Client::Client(int clientFD, std::string clientIP) :
	clientFD_(clientFD), nickname_(""), username_(""), hostname_(clientIP), realName_(""), password_(""), isAuthenticated_(false) {
	std::cout << GREEN "\n=== CLIENT CREATED ===\n" END_COLOR;
	std::cout << "clientFD: " << clientFD_ << "\n";
	std::cout << "hostname: " << hostname_ << "\n";
}

Client::~Client() {
	std::cout << RED "CLIENT DESTROYED\n" END_COLOR;
}

// PUBLIC MEMBER FUNCTIONS
// =======================

ssize_t Client::receiveData(char* buffer, size_t bufferSize) {
    return recv(clientFD_, buffer, bufferSize, MSG_DONTWAIT);
}

// PRIVATE MEMBER FUNCTIONS
// ========================

void Client::authenticateClient() {
	if (realName_.empty() || username_.empty() || nickname_.empty() || password_.empty() || !clientFD_ || hostname_.empty())
		isAuthenticated_ = false;
	isAuthenticated_ = true;
}

// ACCESSORS
// =========

int Client::getClientFD () const {
	return clientFD_;
}

std::string Client::getHostname () const {
	return hostname_;
}

std::string Client::getNickname () const {
	return nickname_;
}

std::string Client::getUsername () const {
	return username_;
}

std::string Client::getRealName() const {
	return realName_;
}

std::string Client::getPassword() const {
	return password_;
}

void Client::setHostname(std::string hostname) {
	hostname_ = hostname;
}

void Client::setNickname(std::string nickname) {
	nickname_ = nickname;
}

void Client::setUsername(std::string username) {
	username_ = username;
}

void Client::setRealName(std::string realName) {
	realName_ = realName;
}

void Client::setPassword(std::string password) {
	password_ = password;
}