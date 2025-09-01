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
		// printChannelMap();
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

	// commands["MODE"] = [this](Client& client, const std::vector<std::string>& params) {
	// 	handleMode(client, params);
	// };

	commands["KICK"] = [this](Client& client, const std::vector<std::string>& params) {
		handleKick(client, params);
	};
	/* commands["INVITE"] = [this](Client& client, const std::vector<std::string>& params) {
		handleKick(client, params);
	}; */
}

void Server::printChannelMap() {
    // Iterate through the map and print each channel's name and details
    for (const auto& entry : channelMap_) {
        const std::string& channelName = entry.first;  // Channel name
        Channel* channel = entry.second;               // Channel object (pointer)

        // Print the channel name
        std::cout << "Channel Name: " << channelName << std::endl;

        // Access specific properties of the channel
        std::cout << "  Channel Key: " << channel->getChannelKey() << std::endl;
        std::cout << "  Requires Password: " << (channel->requiresPassword() ? "Yes" : "No") << std::endl;
    }
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

/**
 * @breif Handles each requested channel from JOIN command.
 */
void Server::handleJoin(Client& client, const std::vector<std::string>& params) {

	if (params.empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "JOIN", params);
		logMessage(ERROR, "CHANNEL", "Not enough parameters");
		return ;
	}

	/* else if (!client.isAuthenticated())
	{
		messageHandle(ERR_NOTREGISTERED, client, "JOIN", params);
		return;
	}
 */
    std::vector<std::string>  requestedChannels = split(params[0], ',');
    std::vector<std::string>  keys = (params.size() > 1) ? split(params[1], ',') : std::vector<std::string>{};

    for (size_t i = 0; i < requestedChannels.size(); i++) {
        const std::string& channelName = requestedChannels[i];
        const std::string& channelKey = (i < keys.size()) ? keys[i] : ""; 	// -> How should we validate channel key..?

		if (Channel::isValidChannelName(channelName) == false) {
			messageHandle(ERR_BADCHANMASK, client, "JOIN", params);
			logMessage(ERROR, "CHANNEL " + channelName, "Invalid channel name");
			continue;
		}

		if (channeClientlExist(&client, channelName)) {
			Channel* channel = getChannel(&client, channelName);
			if (channel->requiresPassword()) {
				if (channelKey.empty()) {  // check with multiple keys, not always empty
	                messageHandle(ERR_BADCHANNELKEY, client, "JOIN", params);
	                logMessage(ERROR, "CHANNEL " + channel->getChannelName(),
	                    "Key required to join the channel: Client " + client.getNickname() + " failed to join");
            		continue;;
				}
				else {
					if (channel->getChannelKey() != channelKey) {
						messageHandle(ERR_BADCHANNELKEY, client, "JOIN", params);
                		logMessage(ERROR, "CHANNEL " + channel->getChannelName(),
                           "Keys do not match: " + client.getNickname() + " failed to join");
                		continue;
					}
					else
						logMessage(ERROR, "CHANNEL", ": joined key protected channel!: " + channelName);

				}
			}
			logMessage(WARNING, "CHANNEL " + channelName, 
            	": Client '" + client.getNickname() + "' is already a member");
			continue;
		}
        Channel* channel = getChannel(&client, channelName);
        if (!channel) {
			channel = createChannel(&client, channelName, channelKey);
			channel->addMember(&client);
			if (!channel)
				logMessage(ERROR, "SERVER", ": Failed to create channel: " + channelName);
		}
        else {
			if (channel->requiresPassword()) {
				if (channelKey.empty()) {  // check with multiple keys, not always empty
	                messageHandle(ERR_BADCHANNELKEY, client, "JOIN", params);
	                logMessage(ERROR, "CHANNEL " + channel->getChannelName(),
	                    "Key required to join the channel: Client " + client.getNickname() + " failed to join");
            		continue;;
				}
				else {
					if (channel->getChannelKey() != channelKey) {
						messageHandle(ERR_BADCHANNELKEY, client, "JOIN", params);
                		logMessage(ERROR, "CHANNEL " + channel->getChannelName(),
                           "Keys do not match: " + client.getNickname() + " failed to join");
                		continue;
					}
					else
						logMessage(ERROR, "CHANNEL", ": joined key protected channel!: " + channelName);

				}
			}
        }
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
		logMessage(ERROR, "NICK", "No nickname given. Client FD: " + std::to_string(client.getClientFD()));
		return;
	}
	else if (isNickUserValid("NICK", params[0])) {
		messageHandle(ERR_ERRONEUSNICKNAME, client, "NICK", params);
		logMessage(ERROR, "NICK", "Invalid nickname format. Given Nickname: " + params[0]);
		return;
	}
	else if (isNickDuplicate(params[0])) {
		messageHandle(ERR_NICKNAMEINUSE, client, "NICK", params);
		logMessage(ERROR, "NICK", "Nickname is already in use. Given Nickname: " + params[0]);
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
		logMessage(ERROR, "USER", "No username given. Client FD: " + std::to_string(client.getClientFD()));
		return;
	}
	else if (params.size() != 4) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "USER", params);
		logMessage(ERROR, "USER", "Not enough parameters. Client FD: " + std::to_string(client.getClientFD()));
		return;
	}
	else if (isUserDuplicate(params[0])) {
		 // make unique?
		logMessage(WARNING, "USER", "Username is already in use. Given Username: " + params[0]);
		return;
	}
	else if (isNickUserValid("USER", params[0])) { // do we need to check real name, host?
		messageHandle(ERR_ERRONEUSUSER, client, "NICK", params);
		logMessage(ERROR, "USER", "Invalid username format. Given Username: " + params[0]);
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
		logMessage(ERROR, "PASS", "Empty password");
		return;
	}
	else if (params[0] != this->getPassword()) {
		messageHandle(ERR_PASSWDMISMATCH, client, "PASS", params);
		logMessage(ERROR, "PASS", "Password mismatch. Given Password: " + params[0]);
		return;
	}
	else if (client.getIsAuthenticated()) {
		messageHandle(ERR_ALREADYREGISTRED, client, "PASS", params);
		logMessage(WARNING, "PASS", "Authentication done already");
	}
	else {
		client.setPassword(params[0]);
		client.setIsPassValid(true);
		//client.setAuthenticated(true); // check logic
	}
	// logMessage(INFO, "PASS", "Client authentication completed. ClientFD: " + std::to_string(client.getClientFD()));
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

int Server::handleKickParams(Client& client, const std::vector<std::string>& params) {
	if (params.empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "KICK", params);
		logMessage(ERROR, "KICK", "No channel or user specified");
		return ERR;
	}
	if (params[0].empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "KICK", params);
		logMessage(ERROR, "KICK", "No channel specified");
		return ERR;
	}
	if (params[1].empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "KICK", params);
		logMessage(ERROR, "KICK", "No user specified");
		return ERR;
	}
	return SUCCESS;
}

void Server::handleKick(Client& client, const std::vector<std::string>& params) {
	
	if (handleKickParams(client, params) == ERR)
		return;
	std::string channel = params[0];
	std::string userToKick = params[1];
	std::string kickReason = (params.size() > 2) ? params[2] : "No reason given"; //optional reason for KICK
	auto it = channelMap_.find(channel);
	if (it == channelMap_.end()) {
		messageHandle(ERR_NOSUCHCHANNEL, client, channel, params);
		logMessage(ERROR, "KICK", "Channel " + channel + " does not exist");
		return;
	}
	Channel* targetChannel = it->second;
	const std::set<Client*>& members = targetChannel->getMembers();
	if (members.find(&client) == members.end()) {
		messageHandle(ERR_NOTONCHANNEL, client, channel, params);
		logMessage(ERROR, "KICK", "User " + client.getNickname() + " not on channel " + channel);
		return;
	}
	if (!targetChannel->isOperator(&client)) { // check whether the user has operator rights on the channel
		messageHandle(ERR_CHANOPRIVSNEEDED, client, channel, params);
		logMessage(ERROR, "KICK", "User " + client.getNickname() + " doesn't have operator rights on channel " + channel);
	}
	Client* clientToKick = nullptr;
	for (Client* member : members) {
		if (member->getNickname() == userToKick) {
			clientToKick = member;
			break;
		}
	}
	if (clientToKick == &client) { // do we want the user to be able to kick themselves out of the channel????
		logMessage(WARNING, "KICK", "User " + client.getNickname() + " attempted to kick themselves out of channel " + channel);
		return; //comment this out if user can self-kick
	}
	if (clientToKick == nullptr) {
		messageHandle(ERR_USERNOTINCHANNEL, client, "KICK", params);
		logMessage(ERROR, "KICK", "User " + userToKick + " not found on channel " + channel);
		return;
	}
	std::string kickMessage = ":" + client.getNickname() + "!" + client.getUsername() +
							"@" + client.getHostname() + " KICK " + channel + " " +
							userToKick + " :" + kickReason;
	for (Client* member : members) {
		member->appendSendBuffer(kickMessage);
	}
	targetChannel->removeMember(clientToKick);
	logMessage(INFO, "KICK", "User " + userToKick + " kicked from " + channel + " by " + client.getNickname() + " (reason: " + kickReason + ")");
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

