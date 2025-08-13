#include "Server.hpp"

void Server::initAddrInfo() {
	std::memset(&hints_, 0, sizeof(hints_));
	hints_.ai_family = AF_UNSPEC;       // Allow IPv4 or IPv6
	hints_.ai_socktype = SOCK_STREAM;   // TCP socket
	hints_.ai_flags = AI_PASSIVE;       // suitable for server use with bind()
}

void Server::createAddrInfo() {
	int error = getaddrinfo(NULL, std::to_string(port_).c_str(), &hints_, &res_);
	if (error) {
		freeaddrinfo(res_);
		throw std::runtime_error("Error: getaddrinfo: " + std::string(gai_strerror(error)));
	}
}

void Server::createServSocket() {
	serverSocket_ = socket(res_->ai_family, res_->ai_socktype, 0);
	if (serverSocket_ < 0) {
		freeaddrinfo(res_);
		throw std::runtime_error("Error: failed to create socket");
	}
}
void Server::setNonBlocking() {
	int socketStatusFlags = fcntl(serverSocket_, F_GETFL, 0);
	if (socketStatusFlags == -1) {
		close(serverSocket_);
		freeaddrinfo(res_);
		throw std::runtime_error("Error: failed to get socket status flags");
	}
	if (fcntl(serverSocket_, F_SETFL, socketStatusFlags | O_NONBLOCK) == -1) {
		close(serverSocket_);
		freeaddrinfo(res_);
		throw std::runtime_error("Error: failed to set non-blocking");
	}
}

void Server::startServer()
{
	int epollFd = epoll_create1(1); // check later if we need this EPOLL_CLOEXEC flag(1)
	if (epollFd < 0) {
		throw std::runtime_error("epoll fd creating failed");
	}

	struct epoll_event serverEvent; // epoll event for Listening socket (new connections monitoring)
	serverEvent.events = EPOLLIN;
	serverEvent.data.fd = serverSocket_;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverEvent.data.fd, &serverEvent) < 0) {
		close (epollFd);
		throw std::runtime_error("Adding server socket to epoll failed");
	}

	const int MAX_EVENTS = 42; // max events to handle at once
	struct epoll_event epEventList[MAX_EVENTS];

	while(true) {
		int epActiveSockets = epoll_wait(epollFd, epEventList, MAX_EVENTS, 4200); // timeout time?
		// handle SIGINT;
		if (epActiveSockets < 0) {
			throw std::runtime_error("Epoll waiting failed");
		}
		if (epActiveSockets > 0) {
			for (int i = 0; i < epActiveSockets; i++)
			{
				if (epEventList[i].data.fd == serverSocket_) {
					// method to receive new connection;
				}

				else if (epEventList[i].events && EPOLLIN) {
					// method to receive data from client;
				}

				else if (epEventList[i].events && EPOLLOUT) {
					// method to send data to specific client;
				}

			}
		}
	}
}

Server::Server(int port, std::string password)
: port_(port), password_(password), serverSocket_(-1){
	try {
		initAddrInfo();
		createAddrInfo();
		createServSocket();
		setNonBlocking();
	} catch (std::exception& e) {
		if (serverSocket_ >= 0) {
			close(serverSocket_);
		}
		throw std::runtime_error("Error: " + std::string(e.what()));
	}
}

Server::~Server() {
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
