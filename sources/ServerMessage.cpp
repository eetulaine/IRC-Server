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
	message += std::to_string(code) + " " + client.getNickname() + " ";

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
	} else if (code == ERR_ALREADYREGISTRED) {
		message += ":Unauthorized command (already registered)";
	} else if (code == ERR_NONICKNAMEGIVEN) {
		message += ":No nickname given";
	} else if (code == ERR_NICKNAMEINUSE) {
		message += paramString + " :Nickname is already in use";
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
		message += paramString;
	} else if (code == RPL_PONG) {
		message = ":" + this->serverName_ + " PONG "+ this->serverName_;
	} else if (code == ERR_ERRONEUSUSER) {
		message += paramString + " :Erroneous format";
	//last
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

// for channel msg

// void Server::messageHandle(int code, Client &client, std::string cmd, const std::vector<std::string>& params) {
// 	if (!code)
// 		return ;
// 	std::string message = createMessage(code, client, cmd, params);
// 	client.appendSendBuffer(message);
// }

