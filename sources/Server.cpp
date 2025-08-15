#include "../includes/Server.hpp"

Server::Server(int port, std::string password) : port_(port), password_(password), serverSocket_(-1) {
	initAddrInfo();
	createAddrInfo();
	createServSocket();
	setNonBlocking();
	setSocketOption();
	bindSocket();
	initListen();
	std::cout << GREEN "\n=== SERVER CREATED ===\n" END_COLOR;
	std::cout << "Port: " << port_ << "\n";
	std::cout << "Pass: " << password_ << "\n";
	std::cout << "Sock: " << serverSocket_ << "\n\n";
}

Server::~Server() {
	if (serverSocket_ >= 0)
		close(serverSocket_);
	if (res_)
		freeaddrinfo(res_);
}

// PRIVATE MEMBER FUNCTIONS USED WITHIN THE SERVER CONSTRUCTOR
// ===========================================================

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

void Server::startServer()
{
	int epollFd = epoll_create1(EPOLL_CLOEXEC); // check later if we need this EPOLL_CLOEXEC flag(1)
	std::cout << GREEN "=== SERVER STARTED ===" END_COLOR << "\nepoll fd: " << epollFd << "\n\n";
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

	std::cout << GREEN "Waiting for events...\n" END_COLOR;
	while(true) {
		int epActiveSockets = epoll_wait(epollFd, epEventList, MAX_EVENTS, 4200); // timeout time?
		// handle SIGINT;
		std::cout << epActiveSockets << " active sockets\n";
		if (epActiveSockets < 0) {
			throw std::runtime_error("Epoll waiting failed");
		}
		if (epActiveSockets > 0) {
			for (int i = 0; i < epActiveSockets; i++)
			{
				if (epEventList[i].data.fd == serverSocket_) {
					// method to receive new connection;
				}

				else if (epEventList[i].events & EPOLLIN) {
					// method to receive data from client;
				}

				else if (epEventList[i].events & EPOLLOUT) {
					// method to send data to specific client;
				}

			}
		}
		std::cout << YELLOW "still waiting...\n" END_COLOR;
	}
}

// we set socket option for all sockets (SOL_SOCKET) to SO_REUSEADDR which enables us to reuse local addresses
// to avoid "address already in use" error
void Server::setSocketOption() {
	int opt = 1;
	if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw std::runtime_error("failed to set socket option: SO_REUSEADDR");
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

// ACCESSORS
// =========

int Server::getPort() const {
	return port_;
}

std::string Server::getPassword() const {
	return password_;
}

int Server::getServerSocket() const {
	return serverSocket_;
}
