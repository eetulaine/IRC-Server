#include "../includes/Server.hpp"
#include "../includes/responseCodes.hpp"
#include "../includes/macros.hpp"

void Server::registerCommands() {
	// command CAP will be ignored

	commands["CAP"] = [this](Client& client, const std::vector<std::string>& params) {
		(void)client;
		std::cout << this->getPort() << ", CAP Ignored" << std::endl;
		for (const std::string& param : params) {
			std::cout << "- " << param << std::endl;
		}
	};

	commands["JOIN"] = [this](Client& client, const std::vector<std::string>& params) {
		(void)client;
		std::cout  << this->getPort() << ", JOIN Ignored" << std::endl;
		for (const std::string& param : params) {
			std::cout << "- " << param << std::endl;
		}
	};

	commands["PING"] = [this](Client& client, const std::vector<std::string>& params) {
		handlePing(client, params);
	};

	commands["NICK"] = [this](Client& client, const std::vector<std::string>& params) {
		handleNick(client, params);
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
		logMessage(ERRORR, "NICK", "No nickname given. Client FD: " + client.getClientFD());
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
		client.setNickname(params[0]);
		logMessage(INFO, "NICK", "Nickname changed to " + client.getNickname() + ". Old Nickname: " + oldNick);
		std::string replyMsg = std::to_string(client.getClientFD()) + ":" + oldNick + "!user@host NICK :" + client.getNickname();
		client.appendSendBuffer(replyMsg); // send msg to all client connexted to same channel
	}
	else {
		client.setNickname(params[0]);
		logMessage(INFO, "NICK", "Nickname set to " + client.getNickname());
		if (client.isAuthenticated()) {
			messageHandle(client, "USER", params);
			logMessage(INFO, "Registration", "Client registration is successful. Username: " + client.getUsername());
		}
	}
}

void Server::handleUser(Client& client, const std::vector<std::string>& params) {

	if (params.empty() || params[0].empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "USER", params); // what code to use??
		logMessage(ERRORR, "USER", "No username given. Client FD: " + client.getClientFD());
		return;
	}
	else if (params.size() != 4) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "USER", params);
		logMessage(ERRORR, "USER", "Not enough parameters. Client FD: " + client.getClientFD());
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
			logMessage(INFO, "Registration", "Client registration is successful. Username: " + client.getUsername());
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
	logMessage(INFO, "PASS", "Client authentication completed. ClientFD: " + client.getClientFD());
}

void Server::handleQuit(Client& client, const std::vector<std::string>& params) {
	(void)client;
	std::cout << "Handling QUIT command. Parameters: " << std::endl;
	std::string quitMessage = "Client quit\r\n";
	if (!params.empty())
		quitMessage = " QUIT :" + params[0] + "\r\n";
    if (!client.isConnected() || !client.isAuthenticated()) //no broadcasting from unconnected or unregistered clients
		return;
	quitMessage = ":" + client.getNickname() + "!" +
						client.getUsername() + "@" +
						client.getHostname() + quitMessage + "\r\n";
	client.appendSendBuffer(quitMessage);
	clients_.erase(client.getClientFD());
	closeServer();
};

