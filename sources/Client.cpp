#include "../includes/Client.hpp"

Client::Client(int clientFD, std::string clientIP) :
	clientFD_(clientFD), nickname_(""), username_(""), hostname_(clientIP), realName_(""), password_(""), isAuthenticated_(false) {
	std::cout << GREEN "\n=== CLIENT CREATED ===\n" END_COLOR;
	std::cout << "clientFD: " << clientFD_ << "\n";
	std::cout << "hostname: " << hostname_ << "\n";
}

Client::~Client() {
	close(clientFD_);
	std::cout << RED "CLIENT DESTROYED\n" END_COLOR;
}

// PUBLIC MEMBER FUNCTIONS
// =======================

int Client::receiveData() {
	char buffer[BUF_SIZE];

	ssize_t bytesRead = recv(clientFD_, buffer, BUF_SIZE, MSG_DONTWAIT);
    if (bytesRead > 0) {
		std::string received(buffer, bytesRead);
		addToBuffer(received);
		std::cout << buffer_ << "\n";
		return SUCCESS;
	}
	else if (!bytesRead) {
		std::cout << "Client " << clientFD_ << " disconnected." << std::endl;
	}
	else {
		std::cerr << "recv failed\n";
	}
	return FAIL;
}

// PRIVATE MEMBER FUNCTIONS
// ========================

void Client::authenticateClient() {
	if (realName_.empty() || username_.empty() || nickname_.empty() || password_.empty() || !clientFD_ || hostname_.empty())
		return;
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

std::string	Client::getBuffer() const {
	return (buffer_);
}

void Client::addToBuffer(const std::string& received) {buffer_.append(received);}

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