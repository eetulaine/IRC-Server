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
	commands["WHOIS"] = [this](Client& client, const std::vector<std::string>& params) {
		handleWhois(client, params);
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
			if (channel && channel->isInviteOnly()) {
				logMessage(ERROR, "CHANNEL", "Channel " + channel->getChannelName() +" is invite-only. Please request an invite from the operator");
				continue;
			}
			if (channel && !channel->checkForChannelKey(channel, &client, channelKey)) {
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
		messageBroadcast(*channel, client, "JOIN", ""); // need to set it if  above mehtods have no error
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
	else {
		messageHandle(RPL_PONG, client, "PING", params);
		logMessage(DEBUG, "PING", "PING received. Client FD: " + std::to_string(client.getClientFD()));
	}

}

void Server::handleNick(Client& client, const std::vector<std::string>& params) {

	// No nickname given
	logMessage(DEBUG, "NICK:NICKNAME", " NICK1: " + params[0]);
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
		logMessage(DEBUG, "NICK:NICKNAME", " NICK2: " + params[0]);
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
		messageHandle(RPL_WHOISUSER, client, "WHOIS", params);
		logMessage(INFO, "NICK", "Nickname changed to " + client.getNickname() + ". Old Nickname: " + oldNick);
		client.appendSendBuffer(replyMsg); // send msg to all client connexted to same channel
	}
	else if (!client.getIsPassValid()) {
		messageHandle(ERR_ALREADYREGISTERED, client, "NICK", params);
	}
	else {
		client.setNickname(params[0]); // + std::to_string(client.getClientFD() - 4)
	//	messageHandle(RPL_WHOISUSER, client, "WHOIS", params);
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
	// else if (isUserDuplicate(params[0])) {
	// 	 // make unique?
	// 	logMessage(WARNING, "USER", "Username is already in use. Given Username: " + params[0]);
	// 	return;
	// }
	else if (isNickUserValid("USER", params[0])) { // do we need to check real name, host?
		messageHandle(ERR_ERRONEUSUSER, client, "NICK", params);
		logMessage(ERROR, "USER", "Invalid username format. Given Username: " + params[0]);
		return;
	}
	else
	{
		if (params[0][0] != '~')
			client.setUsername("~" + params[0]); // + std::to_string(client.getClientFD())
		else
			client.setUsername(params[0]);
		client.setHostname(params[2]); // check index
		client.setRealName(params[3]);
		//messageHandle(RPL_WHOISUSER, client, "WHOIS", params);
		logMessage(INFO, "USER", "Username and details are set. Username: " + client.getUsername());
		if (client.isAuthenticated()) {
			messageHandle(RPL_WHOISUSER, client, "WHOIS", params);
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
		messageHandle(ERR_ALREADYREGISTERED, client, "PASS", params);
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
    if (!client.isConnected() || !client.isAuthenticated()) //no broadcasting from unconnected or unregistered clients
		return closeClient(client);
	std::string reason = "Client quit";
	if (!params.empty())
		reason = params[0];
	// BROADCAST the quitMessage to all the channels the client quitting is a member of
	 for (const std::string& channelName : client.getJoinedChannels()) {
        Channel* channel = getChannel(channelName);
        if (channel) {
            messageBroadcast(*channel, client, "QUIT", " :" + reason);
        }
    }
	//client.appendSendBuffer(quitMessage);
	logMessage(INFO, "QUIT", "User " + client.getNickname() + " quit (reason: " + reason + ")");
	closeClient(client);
}

void Server::handleMode(Client& client, const std::vector<std::string>& params) {

	if (params.empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "MODE", params);
        return;
	}

	std::string targetChannel = params[0];
	if (targetChannel[0] == '#' || targetChannel[0] == '&')
		handleChannelMode(client, params);
	else
		messageHandle(ERR_NEEDMOREPARAMS, client, "MODE", params);
}

void Server::handleChannelMode(Client& client, const std::vector<std::string>& params) {

	std::string channelName = params[0];
	Channel *channel = getChannel(channelName);
	if (!channel) {
		messageHandle(ERR_NOSUCHCHANNEL, client, "MODE", params);
		return;
	}
	if (!channel->isMember(&client)) {
		messageHandle(ERR_NOTONCHANNEL, client, "MODE", params);
		return;
	}
	std::string modeString = params[1];
	if (modeString.size() < 2 || (modeString[0] != '+' && modeString[0] != '-')) {
		messageHandle(ERR_UNKNOWNMODE, client, "MODE", params);
		return;
	}

	char operation = modeString[0];
	char modeChar = modeString[1];
	std::string modeParam = (params.size() > 2) ? params[2] : "";

	switch (modeChar) {
		case 'i':
			inviteOnlyMode(client, *channel, operation);
			break;
		case 't':
			//TopicRestrictionMode(client, *channel, operation);
			//break;
		case 'k':
			channelKeyMode(client, *channel, operation, modeParam);
			break;
		case 'o':
			//OperatorMode(client, *channel, operation, modeParam);
			//break
		case 'l':
			// userLimitMode(client, *channel, operation, modeParam);
			//break;
		default:
			messageHandle(ERR_UNKNOWNMODE, client, "MODE", params);
			break;
	}
}

void Server::inviteOnlyMode(Client& client, Channel& channel, char operation) {

	if (!channel.isOperator(&client)) {
		// messageHandle(ERR_CHANOPRIVSNEEDED, client, "MODE", params);
		return;
	}
	if (operation == '+') {
		if (!channel.isInviteOnly()) {
			channel.setInviteOnly(true);
			logMessage(DEBUG, "MODE", "Channel is set to invite only");
			//broadcast();
	} else
		logMessage(WARNING, "MODE", "Channel is already set to invite only");
	} else {
		if (channel.isInviteOnly()) {
			channel.setInviteOnly(false);
			logMessage(DEBUG, "MODE", "Channel invite-only mode removed");
			//broadcast();
		} else 
		logMessage(WARNING, "MODE", "channel is not invite-only mode");
	}
}

void Server::channelKeyMode(Client& client, Channel& channel, char operation, const std::string& key) {
	
	if (!channel.isOperator(&client)) {
		// messageHandle(ERR_CHANOPRIVSNEEDED, client, "MODE", params);
		return;
	}
	if (operation == '+') {
		if (key.empty())
			// messageHandle(ERR_CHANOPRIVSNEEDED, client, "MODE", params);
			return;
		channel.setChannelKey(key);
		logMessage(INFO, "MODE", "Channel key set to: " + key + " for channel: " + channel.getChannelName());
		//broadcast();
	}
	else {
		channel.setChannelKey("");
		logMessage(INFO, "MODE", "Channel key removed from channel: " + channel.getChannelName());
		//broadcast();
	}
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
		// MESSAGE client that the channel does not exist
		messageHandle(ERR_NOSUCHCHANNEL, client, channel, params);
		return logMessage(ERROR, "KICK", "Channel " + channel + " does not exist");
	}
	Channel* targetChannel = it->second;
	const std::set<Client*>& members = targetChannel->getMembers();
	if (members.find(&client) == members.end()) {
		// MESSAGE client that they are not on the channel themselves
		messageHandle(ERR_NOTONCHANNEL, client, channel, params);
		return logMessage(ERROR, "KICK", "User " + client.getNickname() + " not on channel " + channel);
	}
	if (!targetChannel->isOperator(&client)) { // check whether the user has operator rights on the channel
		// MESSAGE client they don't have the operator rights to kick another user
		messageHandle(ERR_CHANOPRIVSNEEDED, client, channel, params);
		return logMessage(ERROR, "KICK", "User " + client.getNickname() + " doesn't have operator rights on channel " + channel);
	}
	Client* clientToKick = nullptr;
	for (Client* member : members) {
		if (member->getNickname() == userToKick) {
			clientToKick = member;
			break;
		}
	}
	if (clientToKick == &client) { // do we want the user to be able to kick themselves out of the channel????
		// MESSAGE client they can't kick themselves off the channel
		return logMessage(WARNING, "KICK", "User " + client.getNickname() + " attempted to kick themselves out of channel " + channel);
	}
	if (clientToKick == nullptr) {
		// MESSAGE client the user they tried to kick is not on the channel
		messageHandle(ERR_USERNOTINCHANNEL, client, "KICK", params);
		return logMessage(ERROR, "KICK", "User " + userToKick + " not found on channel " + channel);
	}
    // std::string kickMessage = ":" + client.getNickname() + "!" + client.getUsername() +
    // 						"@" + client.getHostname() + " KICK " + channel + " " +
    // 						userToKick + " :" + kickReason;
    // for (Client* member : members) {
    // 	member->appendSendBuffer(kickMessage);
    // }

	// BROADCAST to all the channel members that a user is kicked from the channel
	
	messageBroadcast(*targetChannel, client, "KICK", clientToKick->getNickname() + " :" + kickReason);
	targetChannel->removeMember(clientToKick);
	clientToKick->leaveChannel(channel);
	logMessage(INFO, "KICK", "User " + userToKick + " kicked from " + channel + " by " + client.getNickname() + " (reason: " + kickReason + ")");
}

void Server::closeClient(Client& client) {

	int clientfd = client.getClientFD();
	int epollfd = client.getEpollFd();
	leaveAllChannels(client); // remove client from Channel member lists and clear joinedChannels
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
	Client *targetClient;
	Channel *targetChannel;
	if (target[0] == '#') {
		isChannel = true;
		//sendTo.erase(0,1);
		targetChannel = getChannelShahnaj(target); // check the method
		if (targetChannel == nullptr) {
			//messageHandle(ERR_CANNOTSENDTOCHAN, client, "PRIVMSG", params);
			logMessage(ERROR, "PRIVMSG", "Channel: \"" + target + "\" does not exist");
			return ;
		}
		else if (!isClientChannelMember(targetChannel, client)) {
			messageHandle(ERR_CANNOTSENDTOCHAN, client, "PRIVMSG", params);
			logMessage(ERROR, "PRIVMSG", "Client is not a member of channel: \"" + target + "\"");
			return ;
		}
		logMessage(DEBUG, "PRIVMSG", "End of channel");
	}
	else {
		targetClient = getClient(target);
		if (targetClient == nullptr || (targetClient && !targetClient->isAuthenticated())) {
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
		logMessage(DEBUG, "PRIVMSG", "Sending Channel Msg");
		messageBroadcast(*targetChannel, client, "PRIVMSG", msgToSend);
	}
	else {
		logMessage(DEBUG, "PRIVMSG", "Sending msg to client");
		logMessage(DEBUG, "PRIVMSG", "MSG: " + msgToSend);
		messageToClient(*targetClient, client, "PRIVMSG", msgToSend);
	//	std::string finalMsg = client.getClientIdentifier() + " PRIVMSG " + targetClient->getNickname() + " " + msgToSend + "\r\n";

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
	if (!channelExists(channelInvitedTo)) {
		// MESSAGE client the channel they wanted to invite to doesn't exist
		return logMessage(WARNING, "INVITE", "Channel " + channelInvitedTo + " does not exist");
	}
	Channel* targetChannel = getChannel(channelInvitedTo);
	if (!targetChannel->isMember(&client)) {
		// MESSAGE client they are not joined to the channel themselves
		messageHandle(ERR_NOTONCHANNEL, client, "INVITE", params);
		return logMessage(ERROR, "INVITE", "User " + client.getNickname() + " not on channel " + channelInvitedTo);
	}
	if (targetChannel->isInviteOnly() && !targetChannel->isOperator(&client)) {
		// MESSAGE client they don't have operator rights for invite-only channel
		messageHandle(ERR_CHANOPRIVSNEEDED, client, "INVITE", params);
		return logMessage(ERROR, "INVITE", "User " + client.getNickname() + " does not have operator rights for invite-only channel " + channelInvitedTo);
	}
	Client* clientToBeInvited = nullptr;
	for (auto& pair : clients_) {
		if (pair.second && pair.second->getNickname() == userToBeInvited) {
			clientToBeInvited = pair.second.get();
			break;
		}
	}
	if (clientToBeInvited == nullptr) {
		// MESSAGE client that the user they wanted to invite doesn't exist
		messageHandle(ERR_NOSUCHNICK, client, "INVITE", params);
		return logMessage(ERROR, "INVITE", "User " + userToBeInvited + " does not exist");
	}
	if (clientToBeInvited == &client) { // user can't invite themselves
		// MESSAGE client that they can't invite themselves
		return logMessage(WARNING, "INVITE", "User " + client.getNickname() + " attempted to invite themselves to channel " + channelInvitedTo);
	}
	if (targetChannel->isMember(clientToBeInvited)) { // user to be invited already a member of the channel
		// MESSAGE client that the user invited is already on the channel
		messageHandle(ERR_USERONCHANNEL, *clientToBeInvited, "INVITE", params);
		return logMessage(ERROR, "INVITE", "User " + client.getNickname() + " already on channel " + channelInvitedTo);
	}
	// MESSAGE/BROADCAST? clientToBeInvited & client: User client inviting userToBeInvited to channelInvitedTo
	
	targetChannel->addInvite(clientToBeInvited); // add client to the invited_ list for the channel
	/* std::string confirmMessage = ":" + getServerName() + " 341 " +
                                client.getNickname() + " " + userToBeInvited + " " + channelInvitedTo;
    client.appendSendBuffer(confirmMessage); */
    logMessage(INFO, "INVITE", "User " + client.getNickname() + " inviting " + userToBeInvited + " to " + channelInvitedTo);
}

int Server::handleTopicParams(Client& client, const std::vector<std::string>& params) {
	if (params.empty() || params[0].empty()) {
		// MESSAGE client that they didn't include any parameters
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
	if (!channelExists(channel)) {
		// MESSAGE client the channel is invalid
		return logMessage(WARNING, "TOPIC", "Channel " + channel + " does not exist");
	}
	Channel* targetChannel = getChannel(channel);
	logMessage(DEBUG, "TOPIC", "CURRENT TOPIC: " + targetChannel->getTopic());
	if (!topicGiven && targetChannel->getTopic().empty()) { // if topic not given and channel topic has not been set, print "no topic"
		// MESSAGE client that there's no topic set for the Channel yet
		messageHandle(RPL_NOTOPIC, client, "TOPIC", params);
		return logMessage(WARNING, "TOPIC", "No topic set for channel " + channel);
	}
	else if (!topicGiven) { // if topic not given as argument and topic is already set for channel, print the topic
		// MESSAGE client the topic already set for the Channel
		messageHandle(RPL_TOPIC, client, "TOPIC", params);
		return logMessage(DEBUG, "TOPIC", targetChannel->getTopic());
	}
	const std::set<Client*>& members = targetChannel->getMembers();
	if (members.find(&client) == members.end()) { // if user wanting to set the topic has not joined the channel they can't set the topic
		// MESSAGE client that they are not on the channel and thus cannot change topic
		messageHandle(ERR_NOTONCHANNEL, client, "TOPIC", params);
		return logMessage(ERROR, "TOPIC", "User " + client.getNickname() + " not on channel " + channel);
	}
	if (topicGiven && !targetChannel->isTopicOperatorOnly()) { // if topic is given and the mode +t has not been set we can set the topic
		targetChannel->setTopic(params[1]);
		// BROADCAST to every channel member that user has set a new Channel topic
		return logMessage(DEBUG, "TOPIC", "User " + client.getNickname() + " set new topic: " + params[1] + " for channel " + channel);
	}
	else if (topicGiven && targetChannel->isTopicOperatorOnly()) { // if topic can be set by operators only (mode +t), either set the topic if user is operator or print error
		if (!targetChannel->isOperator(&client)) {
			// MESSAGE client: unable to set topic without operator rights
			return logMessage(DEBUG, "TOPIC", "User " + client.getNickname() + " unable to set topic for channel " + channel + " (NOT AN OPERATOR)");
		}
	}
	// BROADCAST to every channel member that the Channel topic has been set
	targetChannel->setTopic(params[1]);
	messageBroadcast(*targetChannel, client, "TOPIC", params[1]);
	logMessage(DEBUG, "TOPIC", "User " + client.getNickname() + " set new topic: " + params[1] + " for channel " + channel);
}

void Server::handleWhois(Client& client, const std::vector<std::string>& params) {
	//(void)client;
	(void)this;
	std::cout << "WHOIS: Client ID: " << client.getClientFD()  << std::endl;
	for (const std::string& param : params) {
		std::cout << "- " << param << std::endl;
	}
	if (params[0].empty()) {
		messageHandle(ERR_NONICKNAMEGIVEN, client, "WHOIS", params);
	}
	messageHandle(RPL_WHOISUSER, client, "WHOIS", params);
}
