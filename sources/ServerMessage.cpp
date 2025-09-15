#include "../includes/Server.hpp"
#include "../includes/responseCodes.hpp"

std::string	Server::createMessage(int code, Client &client, std::string cmd, const std::vector<std::string>& params) {
	std::string message;
	std::string paramString;

	for (size_t i = 0; i < params.size(); ++i) {
		paramString += params[i];
		if (i != params.size() - 1) {
			paramString += " ";
		}
	}

	message = ":" + this->serverName_ + " ";
	if (code < 10) {
		message += "00";
	}
	message += std::to_string(code) + " ";
	if (client.getNickname().empty() && code == ERR_NICKNAMEINUSE)
		message += "* ";
	else
		message += client.getNickname() + " ";

	if (code == RPL_WELCOME) {
		message += ":Welcome to the " + this->getServerName() + " " + client.getClientIdentifier();
	} else if (code == RPL_YOURHOST) {
		message += "Your host is " + client.getHostname();
	} else if (code == RPL_CREATED) {
		message += this->getServerName() + " was created today";
	} else if (code == RPL_MYINFO) {
		message += this->getServerName() + ": Version 1.0";
	} else if (code == RPL_ISUPPORT) {
		message += this->getServerName() + " supports.......";
	} else if (code == ERR_NEEDMOREPARAMS) {
		message += cmd + " :Not enough parameters";
	} else if (code == ERR_PASSWDMISMATCH) {
		message += ":Password incorrect";
	} else if (code == ERR_ALREADYREGISTERED) {
		message += ":Unauthorized command (already registered)";
	} else if (code == ERR_NONICKNAMEGIVEN) {
		message += ":No nickname given";
	} else if (code == ERR_NICKNAMEINUSE) {
		message += params[0] + " :Nickname is already in use.";
	} else if (code == ERR_NOSUCHNICK) {
		message += paramString + " :No such nick/channel";
	} else if (code == ERR_CANNOTSENDTOCHAN) {
		message += paramString + " :Cannot send to channel";
	} else if (code == ERR_ERRONEUSNICKNAME) {
		message += paramString + " :Erroneous nickname";
	} else if (code == ERR_UNKNOWNCOMMAND) {
		message += cmd + " :Unknown command";
	} else if (code == ERR_NOTREGISTERED) {
		message += ":You have not registered";
	} else if (code == ERR_NOORIGIN) {
		message += ":No origin specified";
	} else if (code == ERR_NOSUCHSERVER) {
		message += paramString + " :No such server";
	} else if (code == ERR_NORECIPIENT) {
		message += ":No recipient given";
	} else if (code == ERR_UMODEUNKNOWNFLAG) {
		message += "Unknown MODE flag";
	} else if (code == RPL_WHOISUSER) {
		message += client.getUsername() + " " + client.getHostname() + " * :" + client.getRealName();
	} else if (code == RPL_ENDOFNAMES) {
		message += client.getUsername() + " " + client.getHostname() + " * :" + client.getRealName();
	} else if (code == RPL_PONG) {
		message = ":" + this->serverName_ + " PONG "+ this->serverName_;
		logMessage(DEBUG, "PONG", "PONG response to ping. Client FD: " + std::to_string(client.getClientFD()));
	} else if (code == ERR_ERRONEUSUSER) {
		message += paramString + " :Erroneous format";
	} else if (code == RPL_TOPIC) {
		message += paramString;
	} else if (code == RPL_NAMREPLY) {
		message += paramString;
	} else if (code == RPL_ENDOFNAMES) {
		message += paramString;
	} else {
		message += cmd + " " + paramString; // print all arguments
	}
	message += "\r\n";

	return (message);
}

void Server::messageHandle(int code, Client &client, std::string cmd, const std::vector<std::string>& params) {
	if (!code)
		return ;
	std::string message = createMessage(code, client, cmd, params);
	client.appendSendBuffer(message);
}

void Server::messageHandle(Client &client, std::string cmd, const std::vector<std::string>& params) {

	std::vector<int> responseCodes = {
		RPL_WELCOME,
		RPL_YOURHOST,
		RPL_CREATED,
		RPL_MYINFO,
		RPL_ISUPPORT // optional
	};

	for (int code : responseCodes) {
		std::string message = createMessage(code, client, cmd, params);
		client.appendSendBuffer(message);
	}
}

void Server::messageToClient(Client &targetClient, Client &fromClient, std::string command, const std::string msgToSend) {
	// check conditions
	// std::string finalMsg;
	// if (command == "NICK") {
	// 	finalMsg = msgToSend;
	// }
	// else
	std::string	finalMsg = fromClient.getClientIdentifier() + " " + command + " " + targetClient.getNickname() + " " + msgToSend + "\r\n";

	targetClient.appendSendBuffer(finalMsg);
}

void Server::messageToClient(Client &targetClient, Client &fromClient, std::string command, const std::string msgToSend, std::string channelName) {

	// check conditions
	std::string finalMsg;
	if (command == "NICK") {
		finalMsg = msgToSend;
		//std::cout << "NICK CHANGE: ToClient: " << targetClient.getNickname() << " MSG: " << finalMsg << std::endl;
	}
	else
		finalMsg = fromClient.getClientIdentifier() + " " + command + " " + channelName + " " + msgToSend + "\r\n";
	std::cout << "SEND MSG: ToClient: " << targetClient.getNickname() << " MSG: " << finalMsg << std::endl;
	targetClient.appendSendBuffer(finalMsg);
}

void Server::messageBroadcast(Channel &targetChannel, Client &fromClient, std::string command, const std::string msgToSend) {
	// check conditions
	if (!isClientChannelMember(&targetChannel, fromClient)) {
		messageHandle(ERR_NOTONCHANNEL, fromClient, command, {msgToSend});
		logMessage(ERROR, "PRIVMSG", "Cleint: " + fromClient.getNickname() + " not in channel: " + targetChannel.getChannelName());
		return ;
	}

	const std::set<Client*>& clients = targetChannel.getMembers();

	for (Client* targetClient : clients) {
		if (command == "PRIVMSG" || command == "NICK") {
			if (targetClient->getClientFD() != fromClient.getClientFD()) {
				if (isClientChannelMember(&targetChannel, *targetClient)) {
					messageToClient(*targetClient, fromClient, command, msgToSend, targetChannel.getChannelName());
				}
			}
		}
		else {
			if (isClientChannelMember(&targetChannel, *targetClient)) {
				messageToClient(*targetClient, fromClient, command, msgToSend, targetChannel.getChannelName());
			}
		}
	}
}

void Server::messageBroadcast(Client &fromClient, std::string command, const std::string msgToSend)
{

	std::set<std::string> channels = fromClient.getJoinedChannels();
	for (const std::string& channelName : channels) {
		Channel* targetChannel = getChannel(channelName);
		if (targetChannel) {
			messageBroadcast(*targetChannel, fromClient, command, msgToSend);
		}
	}


	// if (command == "NICK") {
	// 	for (auto &client : this->clients_) {
	// 		Client *targetClient = client.second.get();
	// 		if (targetClient && (targetClient->getClientFD() != fromClient.getClientFD())) //(targetClient->getClientFD() != fromClient.getClientFD())
	// 		{
	// 			messageToClient(*targetClient, fromClient, command, msgToSend);
	// 		}
	// 	}
	// }
	// else {
	// 	std::set<std::string> channels = fromClient.getJoinedChannels();
	// 	for (const std::string& channelName : channels) {
	// 		Channel* targetChannel = getChannel(channelName);
	// 		if (targetChannel) {
	// 			messageBroadcast(*targetChannel, fromClient, command, msgToSend);
	// 		}
	// 	}
	// }

}


		// else if (command == "NICK") {
		// 	if (targetClient->getClientFD() != fromClient.getClientFD()) {
		// 		if (isClientChannelMember(&targetChannel, *targetClient)) {
		// 			messageToClient(*targetClient, fromClient, command, msgToSend);
		// 		}
		// 	}
		// }
