#include "../includes/Server.hpp"
#include "../includes/responseCodes.hpp"
#include "../includes/macros.hpp"

void Server::registerCommands() {
	// command CAP will be ignored

	commands["CAP"] = [this](Client& client, const std::vector<std::string>& params) {
		(void)params;
		(void)this;
		logMessage(WARNING, "CAP", "CAP command ignored. ClientFD: " + std::to_string(client.getClientFD()));
	};

// 	commands["JOIN"] = [this](Client& client, const std::vector<std::string>& params) {
// 		(void)params;
// 		(void)this;
// 		logMessage(WARNING, "JOIN", "JOIN command ignored. ClientFD: " + std::to_string(client.getClientFD()));
// 	};

	commands["PING"] = [this](Client& client, const std::vector<std::string>& params) {
		handlePing(client, params);
	};

	commands["NICK"] = [this](Client& client, const std::vector<std::string>& params) {
		handleNick(client, params);
    };

    commands["JOIN"] = [this](Client& client, const std::vector<std::string>& params) {
	    handleJoin(client, params);

    };
	commands["QUIT"] = [this](Client& client, const std::vector<std::string>& params) {
		handleQuit(client, params);
    };

	commands["USER"] = [this](Client& client, const std::vector<std::string>& params) {
		handleUser(client, params);
	};

	commands["PASS"] = [this](Client& client, const std::vector<std::string>& params) {
		handlePass(client, params);
	};

	commands["MODE"] = [this](Client& client, const std::vector<std::string>& params) {
		handleMode(client, params);
	};

}

std::vector<std::string> split(const std::string& input, const char delmiter) {

    std::vector<std::string> tokens;
    std::stringstream ss(input);

    std::string token;
    while (std::getline(ss, token, delmiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

// CHANNEL ----- handle join command
/**
 * @breif Handles each requested channel from JOIN command.
 */
void Server::handleJoin(Client& client, const std::vector<std::string>& params) {

	if (params.empty()) {
    	//sendReply(client, "461 " + client.getNickname() + " JOIN :Not enough parameters\r\n");
    	return;
	}
	else if (!client.isAuthenticated())
	{
		messageHandle(ERR_NOTREGISTERED, client, "JOIN", params);
		return;
	}

    std::vector<std::string>  requestedChannels = split(params[0], ',');
    std::vector<std::string>  keys = (params.size() > 1) ? split(params[1], ',') : std::vector<std::string>{};

	// //for debugging
	// std::cout << "=== DEBUG ===\n";
	// for (size_t i = 0; i < keys.size(); i++) {
	// 	std::cout << RED << "KEYS[" << i << "] -> " << keys[i] << END_COLOR << "\n";
	// }
	// for (size_t i = 0; i < requestedChannels.size(); i++) {
	// 	std::cout << RED << "CHANNELS[" << i << "] -> " << requestedChannels[i] << END_COLOR << "\n";
	// }


    for (size_t i = 0; i < requestedChannels.size(); i++) {
        const std::string& channelName = requestedChannels[i];
        const std::string& channelKey = (i < keys.size()) ? keys[i] : "";


		if (Channel::isValidChannelName(channelName) == false) {
			std::cout << "Error: Invalid channel name: " << channelName << ">\n";
			logMessage(ERRORR, "CHANNEL", "Invalid name. Given Name[" + channelName + "]");
			continue; // return;
		}


		// -> How should we validate channel key..?

        if (client.hasJoinedChannel(channelName)) {
			std::cout << "Clinet <" << client.getNickname() << "> is already a member at channel <" << channelName << "\n";
			continue;
		}

		Channel* channel = getChannel(channelName);
		if (channel) {
			if (!channel->checkChannelKey(channelKey)) {
				std::cout << "Error: Keys do not match.\n";
				continue; //return;
			}
			std::cout << "Member " << client.getNickname() << " successfully entered key for <" << channelName << ">!\n";

		} else {
			channel = createChannel(channelName, channelKey);
		}

        channel->addMember(&client);      // server-side  -> add client to channel
        client.activeChannels(channelName);  // client side  -> track joined channels
        //channel->broadcast(client.getNickname() + " has joined " + channelName);
    }
}

void Server::handlePing(Client& client, const std::vector<std::string>& params) {
	// for (const std::string& param : params) {
	// 	std::cout << "- " << param << std::endl;
	// }
	(void)client;
	if (params.empty()) {
		messageHandle(ERR_NOORIGIN, client, "PING", params);
	}
	else if (params[0] != this->serverName_) {
		messageHandle(ERR_NOSUCHSERVER, client, "PING", params);
	}
	else
		messageHandle(RPL_PONG, client, "PING", params);
}

void Server::handleNick(Client& client, const std::vector<std::string>& params) {

	// No nickname given
	if (client.getNickname() == params[0]) {
		logMessage(WARNING, "NICK", "Nickname is same as existing Nickname: " + client.getNickname());
		return;
	}
	else if (params.empty() || params[0].empty()) {
		messageHandle(ERR_NONICKNAMEGIVEN, client, "NICK", params);
		logMessage(ERRORR, "NICK", "No nickname given. Client FD: " + std::to_string(client.getClientFD()));
		return;
	}
	else if (isNickUserValid("NICK", params[0])) {
		messageHandle(ERR_ERRONEUSNICKNAME, client, "NICK", params);
		logMessage(ERRORR, "NICK", "Invalid nickname format. Given Nickname: " + params[0]);
		return;
	}
	else if (isNickDuplicate(params[0])) {
		messageHandle(ERR_NICKNAMEINUSE, client, "NICK", params);
		logMessage(ERRORR, "NICK", "Nickname is already in use. Given Nickname: " + params[0]);
		return;
	}

	if (client.isAuthenticated())
	{
		std::string oldNick = client.getNickname();
		std::string replyMsg = client.getClientIdentifier() + " NICK :" + params[0] + "\r\n";
		client.setNickname(params[0]);
		logMessage(INFO, "NICK", "Nickname changed to " + client.getNickname() + ". Old Nickname: " + oldNick);
		client.appendSendBuffer(replyMsg); // send msg to all client connexted to same channel
	}
	else {
		client.setNickname(params[0]);
		logMessage(INFO, "NICK", "Nickname set to " + client.getNickname());
		if (client.isAuthenticated()) {
			messageHandle(client, "NICK", params);
			logMessage(INFO, "REGISTRATION", "Client registration is successful. Username: " + client.getUsername());
		}
	}
}

void Server::handleUser(Client& client, const std::vector<std::string>& params) {

	if (params.empty() || params[0].empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "USER", params); // what code to use??
		logMessage(ERRORR, "USER", "No username given. Client FD: " + std::to_string(client.getClientFD()));
		return;
	}
	else if (params.size() != 4) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "USER", params);
		logMessage(ERRORR, "USER", "Not enough parameters. Client FD: " + std::to_string(client.getClientFD()));
		return;
	}
	else if (isUserDuplicate(params[0])) {
		 // make unique?
		logMessage(WARNING, "USER", "Username is already in use. Given Username: " + params[0]);
		return;
	}
	else if (isNickUserValid("USER", params[0])) { // do we need to check real name, host?
		messageHandle(ERR_ERRONEUSUSER, client, "NICK", params);
		logMessage(ERRORR, "USER", "Invalid username format. Given Username: " + params[0]);
		return;
	}
	else
	{
		client.setUsername(params[0] + std::to_string(client.getClientFD()));
		client.setHostname(params[2]); // check index
		client.setRealName(params[3]);
		logMessage(INFO, "USER", "Username and details are set. Username: " + client.getUsername());
		if (client.isAuthenticated()) {
			messageHandle(client, "USER", params);
			logMessage(INFO, "REGISTRATION", "Client registration is successful. Username: " + client.getUsername());
		}
	}
}

void Server::handlePass(Client& client, const std::vector<std::string>& params) {

	// need to ensure at beginning pass argument
	if (params.empty() || params[0].empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "PASS", params);
		logMessage(ERRORR, "PASS", "Empty password");
		return;
	}
	else if (params[0] != this->getPassword()) {
		messageHandle(ERR_PASSWDMISMATCH, client, "PASS", params);
		logMessage(ERRORR, "PASS", "Password mismatch. Given Password: " + params[0]);
		return;
	}
	else if (client.getIsAuthenticated()) {
		messageHandle(ERR_ALREADYREGISTRED, client, "PASS", params);
		logMessage(WARNING, "PASS", "Authentication done already");
	}
	else {
		client.setPassword(params[0]);
		client.setIsPassValid(true);
		client.setAuthenticated(true); // check logic
	}
	logMessage(INFO, "PASS", "Client authentication completed. ClientFD: " + std::to_string(client.getClientFD()));
}

void Server::handleQuit(Client& client, const std::vector<std::string>& params) {

	std::cout << "Handling QUIT command. Parameters: " << std::endl;
    if (!client.isConnected() || !client.isAuthenticated()) {//no broadcasting from unconnected or unregistered clients
		closeClient(client);
		return;

	}
	std::string quitMessage = "Client quit";
	if (!params.empty())
		quitMessage = " QUIT :" + params[0];
	quitMessage = ":" + client.getNickname() + "!" +
						client.getUsername() + "@" +
						client.getHostname() + quitMessage + "\r\n";
	client.appendSendBuffer(quitMessage);
	client.sendData();
	closeClient(client);
}


void Server::handleMode(Client& client, const std::vector<std::string>& params) {
	(void)params;
	(void)this;
	std::string msg = ":" + serverName_ + " 221 " + client.getNickname() + " +i" + "\r\n";
	client.appendSendBuffer(msg);
	logMessage(WARNING, "MODE", "Testing mode command. ClientFD: " + std::to_string(client.getClientFD()));
}

void Server::closeClient(Client& client) {

	int clientfd = client.getClientFD();
	int epollfd = client.getEpollFd();
	client.setConnected(false);

	struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = clientfd;
	epoll_ctl(epollfd, EPOLL_CTL_DEL, clientfd, &ev);
	close(clientfd);
	if (clients_[clientfd])
		clients_.erase(clientfd);
}

