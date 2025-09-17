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

	commands["WHO"] = [this](Client& client, const std::vector<std::string>& params) {
		(void)params;
		(void)this;
		logMessage(WARNING, "WHO", "WHO command ignored. ClientFD: " + std::to_string(client.getClientFD()));
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

bool Server::CheckInvitation(Client &client, Channel &channel) {
    if (channel.isInviteOnly() && !channel.isClientInvited(&client)) {
        messageHandle(ERR_INVITEONLYCHAN, client, "JOIN", {channel.getName(),
		":Cannot join channel (+i) - invite only"});
		logMessage(WARNING, "CHANNEL",
		"Client '" + client.getNickname() + "' attempted to join invite-only channel '" +
		channel.getName() + "' without an invitation.");
		return false;
	}
	return true;
}

bool checkChannelName(Client &client, const std::string& name) {
	if (isValidChannelName(name))
		return true;
	logMessage(ERROR, "JOIN",
	"Client '" + client.getNickname() + "' attempted to join with invalid channel name '" + name +
	"'. Channel names must start with '#' or '&' and contain only valid characters.");
	return false;
}

bool checkChannelLimit(Client &client, Channel &channel) {
    if (static_cast<int>(channel.getMembers().size()) < channel.getUserLimit())
        return true;
    //messageHandle(ERR_CHANNELISFULL, client, "JOIN", {channel.getName(), std::to_string(channel.getUserLimit())});
    logMessage(ERROR, "CHANNEL", "Client '" + client.getNickname() +
	"' attempted to join channel '" + channel.getName() + 
    "', but the channel is full (limit: " + std::to_string(channel.getUserLimit()) + ").");
    return false;
}


void Server::handleJoin(Client& client, const std::vector<std::string>& params) {
	if (params.empty() || params.size() < 1) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "JOIN", params);
		logMessage(ERROR, "JOIN", "Client '" + client.getNickname()
			+ "' sent JOIN command with insufficient parameters.");
		return;
	}

	std::vector<std::string> requestedChannels = split(params[0], ',');
	std::vector<std::string> providedKeys = (params.size() > 1) ? split(params[1], ',') : std::vector<std::string>{};

	for (size_t i = 0; i < requestedChannels.size(); i++) {
		const std::string& channelName = requestedChannels[i];
		const std::string& channelKey = (i < providedKeys.size()) ? providedKeys[i] : "";

		if (!checkChannelName(client, channelName)) {
			continue;
		}
		if (client.isInChannel(channelName)) {
			logMessage(WARNING, "JOIN", "Client '" + client.getNickname() + "' attempted to re-join channel '"
				+ channelName + "' but is already a member.");
			continue;
		}
		Channel* channel;
		if (channelExists(channelName)) {
			channel = getChannel(channelName);
			if (!CheckInvitation(client, *channel))
				continue;
			if (!channel->checkKey(channel, &client, channelKey)) {
				continue;
			}
			if (!checkChannelLimit(client, *channel)) {
				continue;
			}
		}
		else {
			channel = createChannel(&client, channelName, channelKey);
			if (!channel) {
				logMessage(ERROR, "CHANNEL", "Failed to create channel: '" + channelName + "'.");
				continue;
			}
		}
		channel->addChannelMember(&client);
		client.addToJoinedChannelList(channel->getName());
		logMessage(INFO, "CHANNEL", "Client '" + client.getNickname() + "' joined channel [" + channel->getName() + "]");
		
		
		messageBroadcast(*channel, client, "JOIN", ""); // need to set it if  above mehtods have no error
		if (channel->getTopic() != "") {
			messageHandle(RPL_TOPIC, client, "JOIN", {channel->getName() + " :" + channel->getTopic()});
		}

		std::string replyMsg2 = "= " + channel->getName() + " :";
		const std::set<Client*>& members = channel->getMembers();
		for (Client* member : members) {
			if (channel->isOperator(member))
				replyMsg2 += "@";
			replyMsg2 += member->getNickname() + " ";
		}
		messageHandle(RPL_NAMREPLY, client, "JOIN", {replyMsg2}); // what if list is longer then MAX_LEN
		messageHandle(RPL_ENDOFNAMES, client, "JOIN", {channel->getChannelName(), " :End of /NAMES list\r\n"});
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
	else if (!client.getIsPassValid()) {
		messageHandle(ERR_PASSWDMISMATCH, client, "NICK", params);
		logMessage(ERROR, "NICK", "Password is not set yet" + params[0]);
		return;
	}
	if (client.isAuthenticated())
	{
		std::string replyMsg = client.getClientIdentifier() + " NICK :" + params[0] + "\r\n";
		client.appendSendBuffer(replyMsg); // send msg to all client connected to same channel
		messageBroadcast(client, "NICK", replyMsg);
		logMessage(INFO, "NICK", "Nickname changed to " + params[0] + ". Old Nickname: " + client.getNickname());
		client.setNickname(params[0]);
	}
	else if (!client.getIsPassValid()) {
		messageHandle(ERR_ALREADYREGISTERED, client, "NICK", params);
	}
	else {
		client.setNickname(params[0]); // + std::to_string(client.getClientFD() - 4)
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
	else if (!client.getIsPassValid()) {
		messageHandle(ERR_PASSWDMISMATCH, client, "USER", params);
		logMessage(ERROR, "USER", "Password is not set yet" + params[0]);
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
		logMessage(ERROR, "MODE", "Client '" + client.getNickname()
			+ "' sent MODE command with insufficient parameters.");
		return;
	}
	Channel *channel;
	std::string target = params[0];
	if (target[0] != '#' && target[0] != '&') {
		Client* targetClient = getClient(target);
		if (targetClient) {
			if (targetClient->getNickname() != client.getNickname()) {
				messageHandle(ERR_USERSDONTMATCH, client, "MODE", params);
				logMessage(ERROR, "MODE", "Client " + client.getNickname()
				+ "' attempted MODE command for another user '" + targetClient->getNickname() + "'.");
				return;
			}
			messageHandle(ERR_UMODEUNKNOWNFLAG, client, "MODE", params);
			logMessage(WARNING, "MODE", "Client '" + client.getNickname() +
				"' attempted unsupported user MODE. User modes are not supported.");
			return;
		} else {
		messageHandle(ERR_NOSUCHNICK, client,"MODE",params);
		logMessage(ERROR, "MODE", "Client '" + client.getNickname() +  "' attempted MODE for nonexistent user '" + target + "'.");
		}
	}
	channel = getChannel(target);
	if (!channel) {
		//messageHandle(ERR_USERSDONTMATCH, client, params);
		logMessage(ERROR, "MODE", "Client '" + client.getNickname()
			+ "' attempted MODE on non-existent channel [" + target + "].");
		return;
	}
	if (params.size() == 1) {
		std::string currentModes = channel->getModeString();
		messageHandle(RPL_CHANNELMODEIS, client, "MODE", {target, currentModes});
		logMessage(INFO, "MODE", "Channel [" + channel->getName() + "] current modes: " + currentModes + ".");
		return;
	}

	if (!channel->isMember(&client)) {
		messageHandle(ERR_NOTONCHANNEL, client, "MODE", {channel->getName()});
		logMessage(ERROR, "MODE",
        	"Client '" + client.getNickname() + "' attempted MODE on channel '" 
        	+ channel->getName() + "', but is not a member.");
		return;
	}
	handleChannelMode(client, *channel, params);	
}

void Server::handleSingleMode(Client &client, Channel &channel, const char &operation, char &modeChar,
	const std::string &modeParam, const std::vector<std::string>& params) {

		switch (modeChar) {
		case 'i':
			inviteOnlyMode(client, channel, operation);
			break;
		case 't':
			topicRestrictionMode(client, channel, operation);
			break;
		case 'k':
			channelKeyMode(client, channel, operation, modeParam);
			break;
		case 'o':
			operatorMode(client, channel, operation, modeParam);
			break;
		case 'l':
			userLimitMode(client, channel, operation, modeParam);
			break;
		default:
			messageHandle(ERR_UNKNOWNMODE, client, "MODE", params);
			logMessage(WARNING, "MODE", "Client " + client.getNickname()
				+ " sent unknown mode character '" + modeChar + "' on channel " + channel.getName() + ".");
			break;
		}
}

bool Server::checkModeParam(const char modeChar, const char operation) {

	if ((modeChar == 'k' || modeChar == 'o' || modeChar == 'l') && operation == '+')
		return true;
	else if (modeChar == 'o' && operation == '-')
		return true;
	return false;
}

void Server::handleChannelMode(Client& client, Channel &channel, const std::vector<std::string>& params) {

	std::string modeString = params[1];
	if (modeString.size() < 2 || (modeString[0] != '+' && modeString[0] != '-')) {
		//messageHandle(ERR_UNKNOWNMODE, client, "MODE", params);
		logMessage(ERROR, "MODE", "Client '" + client.getNickname()
			+ "' used unkown mode character" + modeString[0] + " on channel '" + channel.getName() + "'.");
		return;
	}
	const char operation = modeString[0];
	size_t paramIndex = 2;
	for (size_t i = 1; i < modeString.size(); i++) {
		char modeChar = modeString[i];
		std::string modeParam = "";

		if (checkModeParam(modeChar, operation)) {
			if (paramIndex >= params.size()) {
				//messageHandle(ERR_NEEDMOREPARAMS, client, "MODE", params);
				logMessage(ERROR, "MODE", "Client '" + client.getNickname()
				+ "' sent MODE command with insufficient parameters for mode character "
				+ modeChar + ".");
				return;
			}
			modeParam = params[paramIndex++];
		}
		handleSingleMode(client, channel, operation, modeChar, modeParam, params);
	}
}

void Server::inviteOnlyMode(Client& client, Channel& channel, char operation) {
	if (!channel.isOperator(&client)) {
		//messageHandle(ERR_CHANOPRIVSNEEDED, client, "MODE", {channel.getName(), ":You're not a channel operator"});
		logMessage(WARNING, "MODE", "Client '" + client.getNickname() + "' attempted to change +i on '"
		+ channel.getName() + "' without operator privileges.");
		return;
	}
	if (operation == '+') {
		if (!channel.isInviteOnly()) {
			channel.setInviteOnly(true);
			messageBroadcast(channel, client, "MODE", channel.getName() + " +i");
			//messageHandle(RPL_MODECHANGE, client, "MODE", {channel.getName(), "+i", ":Invite-only mode enabled"});
			logMessage(INFO, "MODE", "Invite-only mode enabled on channel '" + channel.getName() + "' by client '"
			+ client.getNickname() + "'");
		} else {
			//messageHandle(RPL_MODECHANGE, client, "MODE", {channel.getName(), "+i", ":Invite-only mode already enabled"});
			logMessage(DEBUG, "MODE", "Invite-only mode already active on '" + channel.getName() + "'");
		}
	} else if (operation == '-') {
		if (channel.isInviteOnly()) {
			channel.setInviteOnly(false);
			messageBroadcast(channel, client, "MODE", channel.getName() + " -i");
			//messageHandle(RPL_MODECHANGE, client, "MODE", {channel.getName(), "-i", ":Invite-only mode disabled"});
			logMessage(INFO, "MODE", "Client '" + client.getNickname() + "' disabled invite-only mode on channel '"
			+ channel.getName() + "'");
		} else {
			//messageHandle(RPL_MODECHANGE, client, "MODE", {channel.getName(), "-i", ":Invite-only mode is not set"});
			logMessage(DEBUG, "MODE", "Invite-only mode already disabled on '" + channel.getName() + "'");
		}
	}
}

void Server::topicRestrictionMode(Client& client, Channel& channel, char operation) {
	if (!channel.isOperator(&client)) {
		messageHandle(ERR_CHANOPRIVSNEEDED, client, channel.getName(), {});
		return logMessage(ERROR, "MODE", "User not an operator");
	}
	if (operation == '+') {
		if (!channel.isTopicOperatorOnly()) {
			channel.setTopicOperatorOnly(true);
			logMessage(DEBUG, "MODE", "Topic settable by channel operator only");
			messageBroadcast(channel, client, "MODE", "+t");
		} else {
			logMessage(WARNING, "MODE", "Topic is already set as operator-only");
		}
	} else if (operation == '-') { // if '-
		if (channel.isTopicOperatorOnly()) {
			channel.setTopicOperatorOnly(false);
			logMessage(DEBUG, "MODE", "Topic settable by every channel member");
			messageBroadcast(channel, client, "MODE", "-t");
		} else {
			logMessage(WARNING, "MODE", "Topic is already possible to be set by all");
		}
	}
}

void Server::operatorMode(Client& client, Channel& channel, char operation, const std::string& user) {
	if (!channel.isOperator(&client)) {
		messageHandle(ERR_CHANOPRIVSNEEDED, client, channel.getName(), {});
		return logMessage(ERROR, "MODE", "User not an operator");
	}
	if (user.empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "MODE", {});
		return logMessage(ERROR, "MODE", "No target user specified");
	}
	Client* targetClient = nullptr;
	targetClient = getClient(user);
	if (!channel.isMember(targetClient)) {
		messageHandle(ERR_USERNOTINCHANNEL, client, channel.getName(), {"", user});
		return logMessage(ERROR, "MODE", "Target user not on channel");
	}
	if (operation == '+') {
		channel.setOperator(targetClient, true);
		messageBroadcast(channel, client, "MODE", "+o " + targetClient->getNickname());
		return logMessage(DEBUG, "MODE", "User " + user + " given operator rights by " + client.getNickname());
	}
	else if (operation == '-') {
		channel.setOperator(targetClient, false);
		messageBroadcast(channel, client, "MODE", "-o " + targetClient->getNickname());
		return logMessage(DEBUG, "MODE", "User " + user + " operator rights removed by " + client.getNickname());
	}

}

void Server::channelKeyMode(Client& client, Channel& channel, char operation, const std::string& key) {
	if (!channel.isOperator(&client)) {
		messageHandle(ERR_CHANOPRIVSNEEDED, client, "MODE", {channel.getName(), ":You're not a channel operator"});
		logMessage(WARNING, "MODE", "Unauthorized key mode change attempt on " + channel.getName());
		return;
	}
	if (operation == '+') {
		if (key.empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "MODE", {channel.getName(), "+k", ":Key required for +k mode"});
		logMessage(WARNING, "MODE", "Missing key in +k mode on " + channel.getName());
		return;
	}
	channel.setChannelKey(key);
	messageBroadcast(channel, client, "MODE", channel.getName() + " +k");
	/*messageHandle(RPL_MODECHANGE, client, "MODE", {channel.getName(), "+k", ":Channel key set"});*/
	logMessage(INFO, "MODE", "+k set on " + channel.getName());
    } else if (operation == '-') {
		channel.setChannelKey("");
		messageBroadcast(channel, client, "MODE", channel.getName() + " -k");
		/*messageHandle(RPL_MODECHANGE, client, "MODE", {channel.getName(), "-k", ":Channel key removed"});*/
		logMessage(INFO, "MODE", "+k removed from " + channel.getName());
	}
}

bool Server::isValidUserLimit(const std::string& str, int& userLimit) {
	std::istringstream iss(str);
	int temp;
	char remain;

	if (!(iss >> temp) || (iss >> remain) || temp <= 0)
		return false;
	userLimit = temp;
	return true;
}

void Server::userLimitMode(Client& client, Channel& channel, char operation, const std::string& userLimitStr) {
	if (operation == '-') {
		channel.setUserLimit(CHAN_USER_LIMIT);
		messageBroadcast(channel, client, "MODE", "-l");
		return logMessage(DEBUG, "MODE", "User limit removed (defaulted back to 100)");
	}
	if (userLimitStr.empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "MODE", {});
		return logMessage(ERROR, "MODE", "No user limit specified for the channel");
	}
	if (!channel.isOperator(&client)) {
		messageHandle(ERR_CHANOPRIVSNEEDED, client, channel.getName(), {});
		return logMessage(ERROR, "MODE", "User does not have operator rights");
	}
	int userLimit = 0;
	if (!isValidUserLimit(userLimitStr, userLimit))
		return logMessage(ERROR, "MODE", "Faulty user limit");
	if (userLimit > 0 && userLimit <= CHAN_USER_LIMIT) {
		channel.setUserLimit(userLimit);
		messageBroadcast(channel, client, "MODE", "+l " + std::to_string(userLimit));
		return logMessage(DEBUG, "MODE", "User limit set to: " + std::to_string(userLimit));
	}
	else if (userLimit > CHAN_USER_LIMIT)
		return logMessage(ERROR, "MODE", "User limit set too high > 100");
	else
		logMessage(ERROR, "MODE", "Faulty user limit");
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
		return logMessage(ERROR, "KICK", "Channel " + channel + " does not exist");
	}
	Channel* targetChannel = it->second;
	const std::set<Client*>& members = targetChannel->getMembers();
	if (members.find(&client) == members.end()) {
		messageHandle(ERR_NOTONCHANNEL, client, channel, params);
		return logMessage(ERROR, "KICK", "User " + client.getNickname() + " not on channel " + channel);
	}
	if (!targetChannel->isOperator(&client)) { // check whether the user has operator rights on the channel
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
	if (clientToKick == &client) {
		return logMessage(WARNING, "KICK", "User " + client.getNickname() + " attempted to kick themselves out of channel " + channel);
	}
	if (clientToKick == nullptr) {
		// MESSAGE client the user they tried to kick is not on the channel
		messageHandle(ERR_USERNOTINCHANNEL, client, channel, params);
		return logMessage(ERROR, "KICK", "User " + userToKick + " not found on channel " + channel);
	}
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
		messageHandle(ERR_NOSUCHCHANNEL, client, channelInvitedTo, params);
		return logMessage(WARNING, "INVITE", "Channel " + channelInvitedTo + " does not exist");
	}
	Channel* targetChannel = getChannel(channelInvitedTo);
	if (!targetChannel->isMember(&client)) {
		messageHandle(ERR_NOTONCHANNEL, client, targetChannel->getName(), params);
		return logMessage(ERROR, "INVITE", "User " + client.getNickname() + " not on channel " + channelInvitedTo);
	}
	if (targetChannel->isInviteOnly() && !targetChannel->isOperator(&client)) {
		messageHandle(ERR_CHANOPRIVSNEEDED, client, channelInvitedTo, params);
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
		messageHandle(ERR_NOSUCHNICK, client, "INVITE", params);
		return logMessage(ERROR, "INVITE", "User " + userToBeInvited + " does not exist");
	}
	if (clientToBeInvited == &client) { // user can't invite themselves
		return logMessage(WARNING, "INVITE", "User " + client.getNickname() + " attempted to invite themselves to channel " + channelInvitedTo);
	}
	if (targetChannel->isMember(clientToBeInvited)) { // user to be invited already a member of the channel
		messageHandle(ERR_USERONCHANNEL, client, channelInvitedTo, params);
		return logMessage(ERROR, "INVITE", "User " + client.getNickname() + " already on channel " + channelInvitedTo);
	}
	targetChannel->addInvite(clientToBeInvited); // add client to the invited_ list for the channel
	messageHandle(RPL_INVITING, client, channelInvitedTo, params);
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
		messageHandle(ERR_NOSUCHCHANNEL, client, channel, params);
		return logMessage(WARNING, "TOPIC", "Channel " + channel + " does not exist");
	}
	Channel* targetChannel = getChannel(channel);
	logMessage(DEBUG, "TOPIC", "CURRENT TOPIC: " + targetChannel->getTopic());
	if (!topicGiven && targetChannel->getTopic().empty()) { // if topic not given and channel topic has not been set, print "no topic"
		messageHandle(RPL_NOTOPIC, client, channel, params);
		return logMessage(WARNING, "TOPIC", "No topic set for channel " + channel);
	}
	else if (!topicGiven) { // if topic not given as argument and topic is already set for channel, print the topic
		messageHandle(RPL_TOPIC, client, channel, {"", targetChannel->getTopic()});
		return logMessage(DEBUG, "TOPIC", targetChannel->getTopic());
	}
	const std::set<Client*>& members = targetChannel->getMembers();
	if (members.find(&client) == members.end()) { // if user wanting to set the topic has not joined the channel they can't set the topic
		messageHandle(ERR_NOTONCHANNEL, client, channel, params);
		return logMessage(ERROR, "TOPIC", "User " + client.getNickname() + " not on channel " + channel);
	}
	if (topicGiven && !targetChannel->isTopicOperatorOnly()) { // if topic is given and the mode +t has not been set we can set the topic
		targetChannel->setTopic(params[1]);
		messageBroadcast(*targetChannel, client, "TOPIC", params[1]);
		return logMessage(DEBUG, "TOPIC", "User " + client.getNickname() + " set new topic: " + params[1] + " for channel " + channel);
	}
	else if (topicGiven && targetChannel->isTopicOperatorOnly()) { // if topic can be set by operators only (mode +t), either set the topic if user is operator or print error
		if (!targetChannel->isOperator(&client)) {
			messageHandle(ERR_CHANOPRIVSNEEDED, client, channel, params);
			return logMessage(DEBUG, "TOPIC", "User " + client.getNickname() + " unable to set topic for channel " + channel + " (NOT AN OPERATOR)");
		}
	}
	targetChannel->setTopic(params[1]);
	messageHandle(RPL_TOPIC, client, channel, {"", targetChannel->getTopic()});
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
	if (getClient(params[0]) == nullptr) {
		messageHandle(ERR_NOSUCHNICK, client, "WHOIS", params);
	}
	messageHandle(RPL_ENDOFWHOIS, client, "WHOIS", {" :End of /WHOIS list"});
	//messageHandle(RPL_WHOISUSER, client, "WHOIS", params); // {client.getNickname() + " " + client.getUsername() + " " + client.getHostname() + " * :" + client.getRealName()}
}
