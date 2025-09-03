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

	commands["MODE"] = [this](Client& client, const std::vector<std::string>& params) {
		handleMode(client, params);
	};

	commands["PRIVMSG"] = [this](Client& client, const std::vector<std::string>& params) {
		handlePrivMsg(client, params);
	};

	commands["KICK"] = [this](Client& client, const std::vector<std::string>& params) {
		handleKick(client, params);
	};
	commands["INVITE"] = [this](Client& client, const std::vector<std::string>& params) {
		handleInvite(client, params);
	};
	commands["TOPIC"] = [this](Client& client, const std::vector<std::string>& params) {
		handleTopic(client, params);
	};
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
        std::cout << "  Requires Password: " << (channel->isKeyProtected() ? "Yes" : "No") << std::endl;
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


void Server::handleJoin(Client& client, const std::vector<std::string>& params) {
	if (params.empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "JOIN", params);
		logMessage(ERROR, "CHANNEL", "Not enough parameters");
		return;
	}

	std::vector<std::string> requestedChannels = split(params[0], ',');
	std::vector<std::string> providedKeys = (params.size() > 1) ? split(params[1], ',') : std::vector<std::string>{};

	for (size_t i = 0; i < requestedChannels.size(); i++) {
		const std::string& channelName = requestedChannels[i];
		const std::string& channelKey = (i < providedKeys.size()) ? providedKeys[i] : "";

		if (!Channel::isValidChannelName(channelName)) {
			//messageHandle(ERR_BADCHANMASK, client, channelName, std::vector<std::string>{channelName});
			logMessage(ERROR, "CHANNEL " + channelName, "Invalid channel name");
			continue;
		}
		if (client.isInChannel(channelName)) {
			logMessage(WARNING, "CLIENT" + channelName,
				"Client '" + client.getUsername() + "' is already a member");
			continue;
		}

		Channel* channel = nullptr;
		if (channelExists(channelName)) {
			channel = getChannel(channelName);
			if (!channel->checkForChannelKey(channel, &client, channelKey)) {
				//messageHandle();
				continue;
			}
		}	// check for channel resterictions??
		else {
			channel = createChannel(&client, channelName, channelKey);
			if (!channel) {
				logMessage(ERROR, "CHANNEL", "Failed to create channel: " + channel->getChannelName());
				continue;
			}
		}
		channel->addChannelMember(&client);
		client.addToJoinedChannelList(channel->getChannelName());
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
		client.setNickname(params[0] + std::to_string(client.getClientFD() - 4));
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
		logMessage(INFO, "PASS", "Password validated for ClientFD: " + std::to_string(client.getClientFD()));
		//client.setAuthenticated(true); // check logic, usually pass always come first
	}
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

void Server::handlePrivMsg(Client& client, const std::vector<std::string>& params) {
	for (const std::string& param : params) {
		std::cout << "- " << param << std::endl;
	}

	// ERR_NOSUCHNICK  ERR_TOOMANYTARGETS ERR_CANNOTSENDTOCHAN
	if (params.empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "PRIVMSG", params);
		logMessage(ERROR, "PRIVMSG", "No parameter provided");
	}
	else if (params[0].empty()) {
		messageHandle(ERR_NORECIPIENT, client, "PRIVMSG", params);
		logMessage(ERROR, "PRIVMSG", "No recipient to send msg");
	}
	else if (params[1].empty()) {
		messageHandle(ERR_NOTEXTTOSEND, client, "PRIVMSG", params);
		logMessage(ERROR, "PRIVMSG", "No text to send");
	}

	bool isChannel = false;
	std::string target = params[0];
	Client *clientTo;
		Channel *channelTo;
	if (target[0] == '#') {
		isChannel = true;
		//sendTo.erase(0,1);
		channelTo = getChannelShahnaj(target); // check the method
		if (channelTo == nullptr) {
			//messageHandle(ERR_CANNOTSENDTOCHAN, client, "PRIVMSG", params);
			logMessage(ERROR, "PRIVMSG", "Channel: \"" + target + "\" does not exist");
			return ;
		}
		else if (!isClientChannelMember(channelTo, client)) {
			messageHandle(ERR_CANNOTSENDTOCHAN, client, "PRIVMSG", params);
			logMessage(ERROR, "PRIVMSG", "Client is not a member of channel: \"" + target + "\"");
			return ;
		}
		logMessage(DEBUG, "PRIVMSG", "End of channel");
	}
	else {
		clientTo = getClient(target);
		if (clientTo == nullptr || (clientTo && !clientTo->isAuthenticated())) {
			messageHandle(ERR_NOSUCHNICK, client, "PRIVMSG", params);
			logMessage(ERROR, "PRIVMSG", "No such nickname: \"" + target + "\"");
			return ;
		}
		logMessage(DEBUG, "PRIVMSG", "End of client");
	}
	std::string msgToSend;
	if (params[1].length() > MAX_MSG_LEN) {
		msgToSend = params[1].substr(0, MAX_MSG_LEN);
	}
	else
		msgToSend = params[1];
	logMessage(DEBUG, "PRIVMSG", "MSG: " + msgToSend);
	if (isChannel) {
		logMessage(DEBUG, "PRIVMSG", "Msg to client goes here");
	}
	else {
		logMessage(DEBUG, "PRIVMSG", "Sending msg to client");
		//clientTo->appendSendBuffer(msgToSend + "\r\n");
		std::string finalMsg = client.getClientIdentifier() + " PRIVMSG " + clientTo->getNickname() + " " + msgToSend + "\r\n";
		//messageHandle(5, clientTo, "PRIVMSG", finalMsg);
		logMessage(DEBUG, "PRIVMSG", "FULL MSG: " + finalMsg);
		//send(clientTo->getClientFD(), finalMsg.c_str(), finalMsg.size(), 0);
		clientTo->appendSendBuffer(finalMsg);
	}
}


int Server::handleInviteParams(Client& client, const std::vector<std::string>& params) {
	if (params.empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "INVITE", params);
		logMessage(ERROR, "INVITE", "No nickname or channel specified");
		return ERR;
	}
	if (params[0].empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "INVITE", params);
		logMessage(ERROR, "INVITE", "No nickname specified");
		return ERR;
	}
	if (params[1].empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "INVITE", params);
		logMessage(ERROR, "INVITE", "No channel specified");
		return ERR;
	}
	return SUCCESS;
}

void Server::handleInvite(Client& client, const std::vector<std::string>& params) {
	if (handleKickParams(client, params) == ERR)
		return;
	std::string userToBeInvited = params[0];
	std::string channelInvitedTo = params[1];
	auto it = channelMap_.find(channelInvitedTo);
	if (it == channelMap_.end()) {
		logMessage(WARNING, "INVITE", "Channel " + channelInvitedTo + " does not exist");
		return;
	}
	Channel* targetChannel = it->second;
	const std::set<Client*>& members = targetChannel->getMembers();
	if (members.find(&client) == members.end()) {
		messageHandle(ERR_NOTONCHANNEL, client, "INVITE", params);
		logMessage(ERROR, "INVITE", "User " + client.getNickname() + " not on channel " + channelInvitedTo);
		return;
	}
	if (targetChannel->isInviteOnly() && !targetChannel->isOperator(&client)) {
		messageHandle(ERR_CHANOPRIVSNEEDED, client, "INVITE", params);
		logMessage(ERROR, "INVITE", "User " + client.getNickname() + " does not have operator rights for invite-only channel " + channelInvitedTo);
		return;
	}
	Client* clientToBeInvited = nullptr;
	for (auto& pair : clients_) {
		if (pair.second && pair.second->getNickname() == userToBeInvited) {
			clientToBeInvited = pair.second.get();
			break;
		}
	}
	if (clientToBeInvited == nullptr) {
		messageHandle(ERR_NOSUCHNICK, client, "INVITE", params);
		logMessage(ERROR, "INVITE", "User " + userToBeInvited + " does not exist");
		return;
	}
	if (clientToBeInvited == &client) { // user can't invite themselves
		logMessage(WARNING, "INVITE", "User " + client.getNickname() + " attempted to invite themselves to channel " + channelInvitedTo);
		return;
	}
	if (members.find(clientToBeInvited) != members.end()) { // user to be invited already a member of the channel
		messageHandle(ERR_USERONCHANNEL, *clientToBeInvited, "INVITE", params);
		logMessage(ERROR, "INVITE", "User " + client.getNickname() + " already on channel " + channelInvitedTo);
		return;
	}
	targetChannel->addInvite(clientToBeInvited); // add client to the invited_ list for the channel
	std::string confirmMessage = ":" + getServerName() + " " + std::to_string(RPL_INVITING) + " " +
                                client.getNickname() + " " + userToBeInvited + " " + channelInvitedTo;
    client.appendSendBuffer(confirmMessage);
    logMessage(INFO, "INVITE", "User " + userToBeInvited + " invited to join channel " + channelInvitedTo + " by " + client.getNickname());
}

int Server::handleTopicParams(Client& client, const std::vector<std::string>& params) {
	if (params.empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "TOPIC", params);
		logMessage(ERROR, "TOPIC", "No channel/topic specified");
		return ERR;
	}
	if (params[0].empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "TOPIC", params);
		logMessage(ERROR, "TOPIC", "No channel specified");
		return ERR;
	}
	return SUCCESS;
}

void Server::handleTopic(Client& client, const std::vector<std::string>& params) {
	if (handleTopicParams(client, params) == ERR)
		return;
	std::string channel = params[0];
	bool topicGiven = true;
	if (params[1].empty()) {
		topicGiven = false;
	}
	auto it = channelMap_.find(channel); // confirm the channel exists
	if (it == channelMap_.end()) {
		logMessage(WARNING, "TOPIC", "Channel " + channel + " does not exist");
		return;
	}
	Channel* targetChannel = it->second;
	logMessage(DEBUG, "TOPIC", "CURRENT TOPIC: " + targetChannel->getTopic());
	if (!topicGiven && targetChannel->getTopic().empty()) { // if topic not given and channel topic has not been set, print "no topic"
		messageHandle(RPL_NOTOPIC, client, "TOPIC", params);
		logMessage(WARNING, "TOPIC", "No topic set for channel " + channel);
		return;
	}
	else if (!topicGiven) { // if topic not given as argument and topic is already set for channel, print the topic
		messageHandle(RPL_TOPIC, client, "TOPIC", params);
		logMessage(DEBUG, "TOPIC", targetChannel->getTopic());
		return;
	}
	const std::set<Client*>& members = targetChannel->getMembers();
	if (members.find(&client) == members.end()) { // if user wanting to set the topic has not joined the channel they can't set the topic
		messageHandle(ERR_NOTONCHANNEL, client, "TOPIC", params);
		logMessage(ERROR, "TOPIC", "User " + client.getNickname() + " not on channel " + channel);
		return;
	}
	if (topicGiven && !targetChannel->isTopicOperatorOnly()) { // if topic is given and the mode +t has not been set we can set the topic
		targetChannel->setTopic(params[1]);
		logMessage(DEBUG, "TOPIC", "User " + client.getNickname() + " set new topic: " + params[1] + " for channel " + channel);
		return;
	}
	else if (topicGiven && targetChannel->isTopicOperatorOnly()) { // if topic can be set by operators only (mode +t), either set the topic if user is operator or print error
		if (!targetChannel->isOperator(&client)) {
			logMessage(DEBUG, "TOPIC", "User " + client.getNickname() + " unable to set topic for channel " + channel + " (NOT AN OPERATOR)");
			return;
		}
	}
	targetChannel->setTopic(params[1]);
	logMessage(DEBUG, "TOPIC", "User " + client.getNickname() + " set new topic: " + params[1] + " for channel " + channel);
}

