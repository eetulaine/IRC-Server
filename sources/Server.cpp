#include "Server.hpp"

// add default settings to addrinfo struct
void Server::initAddrInfo() {
	std::memset(&hints_, 0, sizeof(hints_));
	hints_.ai_family = AF_UNSPEC;       // Allow IPv4 or IPv6
	hints_.ai_socktype = SOCK_STREAM;   // TCP socket
	hints_.ai_flags = AI_PASSIVE;       // suitable for server use with bind()
}

// translates service location/name to a set of socket addresses in the addrinfo struct
void Server::createAddrInfo() {
	int error = getaddrinfo(NULL, std::to_string(port_).c_str(), &hints_, &res_);
	if (error)
		throw std::runtime_error("getaddrinfo: " + std::string(gai_strerror(error)));
}

// creates a new TCP socket
void Server::createServSocket() {
	serverSocket_ = socket(res_->ai_family, res_->ai_socktype, 0);
	if (serverSocket_ < 0)
		throw std::runtime_error("failed to create socket");
}
void Server::setNonBlocking() {
	if (fcntl(serverSocket_, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("fcntl failed to set non-blocking");
}

// we set socket option for all sockets (SOL_SOCKET) to SO_REUSEADDR which enables us to reuse local addresses
// to avoid "address already in use" error
void Server::setSocketOption() {
	int opt = 1;
	if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw std::runtime_error("failed to set socket option SO_REUSEADDR)");
}

// we bind the socket to the address
void Server::bindSocket() {
	if (bind(serverSocket_, res_->ai_addr, res_->ai_addrlen) == -1)
		throw(std::runtime_error("failed to bind the socket"));
}

// prepare to listen for incoming connections on socket fd. we set the amount of connection requests to max (SOMAXCONN)
void Server::initListen() {
	if (listen(serverSocket_, SOMAXCONN) == -1)
		throw(std::runtime_error("failed to init listen()"));
}

Server::Server(int port, std::string password) : port_(port), password_(password), serverSocket_(-1) {
	initAddrInfo();
	createAddrInfo();
	createServSocket();
	setNonBlocking();
	setSocketOption();
	bindSocket();
	initListen();
}

Server::~Server() {
	if (serverSocket_ >= 0)
		close(serverSocket_);
	if (res_)
		freeaddrinfo(res_);
}

int Server::getPort() const {
	return port_;
}

std::string Server::getPassword() const {
	return password_;
}

int Server::getServerSocket() const {
	return serverSocket_;
}