#pragma once

#include <string>
#include <iostream>
#include <algorithm> // transform
#include <sys/socket.h> //-> needed for socket
#include <sys/epoll.h>	//-> needed for epoll
#include <netdb.h>      //-> needed for addrinfo
#include <cstring>		//-> needed for memset etc.
#include <unistd.h>		//-> needed for close etc.
#include <fcntl.h>		//-> needed for fcntl
#include <arpa/inet.h>	// Client IP  log "inet_ntop()"
#include <map>			// for map
#include <memory>		// for std::unique_ptr
#include <vector>		// for vector
#include <sstream>		// for istringstream
#include <functional>   // for std::function
#include <algorithm>	// for transform
#include <csignal>		// for signal
#include <regex>
#include <ctime>
#include <iomanip>  // put_time
#include "../includes/macros.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"


class Client;
class Channel;

class Server {

	private:
		int			port_;
		std::string	password_;
		int			serverSocket_;
		int			epollFd;
		static volatile sig_atomic_t isRunning_;
		struct addrinfo		hints_, *res_;
		const std::string	serverName_ = "IRCS_SERV";

		std::map<int, std::unique_ptr<Client>> clients_; //-> List of clients
		std::map<std::string, Channel*>  channelMap_; //-> List of created channels

		using CommandHandler = std::function<void(Client& client, const std::vector<std::string>& params)>;
		std::map<std::string, CommandHandler> commands;

		// private member functions used for the server setup within the Server constructor
		void		initAddrInfo(); 		//-> init addrinfo struct settings
		void		createAddrInfo(); 		//-> call getaddrinfo
		void		createServSocket(); 	//-> create socket
		void		setNonBlocking(); 		//-> set socket status flags to non-blocking using fcntl
		void		setSocketOption();		//-> set socket option to reuse address to avoid "address already in use" error
		void		bindSocket();			//-> bind the socket to the address
		void		initListen();			//-> prepare to listen for incoming connections
		static void	stop(int signum);		//-> signal handler
		void		customSignals(bool customSignals);

		// dependent methods for "ServerActivity"
		void		acceptNewClient(int epollFd);
		std::string getClientIP(struct sockaddr_in clientSocAddr);
		void		receiveData(int currentFD);
		void		sendData(int currentFD);

		std::pair<std::string, std::vector<std::string>> parseCommand(const std::string& line);

	public:
		Server(int port, std::string password);
		~Server();

		void		startServer();
		void		processBuffer(Client& client);
		void		registerCommands();
		void		closeServer();
		void		closeClient(Client& client);

		int			getPort() const;
		int			getServerSocket() const;
		std::string	getPassword() const;
		std::string	getServerName() const;

		// CLIENT
		Client* 	getClient(const std::string& nickName);

		// CHANNEL
		bool 		channelExists(const std::string& channelName);
		Channel*	getChannel(const std::string& channelName);
		Channel*	createChannel(Client* client, const std::string& channelName, const std::string& channelKey);
		bool		isClientChannelMember(Channel *channel, Client& client);
		bool 		checkChannelName(Client &client, const std::string& name);
		void		leaveAllChannels(Client& client);

		// COMMAND Parsing Methods
		int			handleNickParams(Client& client, const std::vector<std::string>& params);
		int			handleUserParams(Client& client, const std::vector<std::string>& params);
		int			handleKickParams(Client& client, const std::vector<std::string>& params);
		int			handlePrivMsgParams(Client& client, const std::vector<std::string>& params);
		int			handleInviteParams(Client& client, const std::vector<std::string>& params);
		int			handleTopicParams(Client& client, const std::vector<std::string>& params);
		bool 		checkModeParam(const char modeChar, const char operation);

		// COMMAND Handle Methods
		void		handleNick(Client& client, const std::vector<std::string>& params);
		void		handleUser(Client& client, const std::vector<std::string>& params);
		void		handlePass(Client& client, const std::vector<std::string>& params);
		void		handlePing(Client& client, const std::vector<std::string>& params);
		void		handleQuit(Client& client, const std::vector<std::string>& params);
		void		handleMode(Client& client, const std::vector<std::string>& params);
		void 		handleChannelMode(Client& client, Channel &channel, const std::vector<std::string>& params);
		void		handleKick(Client& client, const std::vector<std::string>& params);
		void		handleJoin(Client& client, const std::vector<std::string>& params);
		void		handlePrivMsg(Client& client, const std::vector<std::string>& params);
		void		handleInvite(Client& client, const std::vector<std::string>& params);
		void		handleTopic(Client& client, const std::vector<std::string>& params);
		void		handleWhois(Client& client, const std::vector<std::string>& params);
		void		handleSingleMode(Client &client, Channel &channel, const char &operation, char &modeChar,
						const std::string &modeParam, const std::vector<std::string>& params);

		/// COMMANDs Util Methods
		void		inviteOnlyMode(Client& client, Channel& channel, char operation);
		void 		userLimitMode(Client& client, Channel& channel, char operation, const std::string& limit);
		void 		channelKeyMode(Client& client, Channel& channel, char operation, const std::string& key);
		void		topicRestrictionMode(Client& client, Channel& channel, char operation);
		void		operatorMode(Client& client, Channel& channel, char operation, const std::string& user);
		bool		isValidUserLimit(const std::string& str, int& userLimit);
		bool 		checkInvitation(Client &client, Channel &channel);
		bool		checkChannelLimit(Client &client, Channel &channel);
		bool		stringCompCaseIgnore(const std::string &str1, const std::string &str2);
		bool		isNickDuplicate(std::string  userName);
		bool		isNickUserValid(std::string cmd, std::string name);

		// MESSAGE Handle Methods
		void		messageHandle(int code, Client &client, std::string cmd, const std::vector<std::string>& params);
		void		messageHandle(Client &client, std::string cmd, const std::vector<std::string>& params);
		std::string	createMessage(int code, Client &client, std::string cmd, const std::vector<std::string>& params);
		void		messageToClient(Client &targetClient, Client &fromClient, std::string command, const std::string msgToSend);
		void		messageToClient(Client &targetClient, Client &fromClient, std::string command, const std::string msgToSend, std::string channelName);
		void		messageBroadcast(Channel &targetChannel, Client &fromClient, std::string command, const std::string msgToSend);
		void		messageBroadcast(Client &fromClient, std::string command, const std::string msgToSend);
};

// To track server activity
void	logMessage(logMsgType type, const std::string &action, const std::string &msg);
