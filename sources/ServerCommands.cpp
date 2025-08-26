#include "../includes/Server.hpp"
#include "../includes/responseCodes.hpp"

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
		return;
	}
	else if (params.empty() || params[0].empty()) {
		messageHandle(ERR_NONICKNAMEGIVEN, client, "NICK", params);
		return;
	}
	else if (isNickUserValid("NICK", params[0])) {
		messageHandle(ERR_ERRONEUSNICKNAME, client, "NICK", params);
		return;
	}
	else if (isNickDuplicate(params[0])) {
		messageHandle(ERR_NICKNAMEINUSE, client, "NICK", params);
		return;
	}

	if (client.isAuthenticated())
	{
		std::string oldNick = client.getNickname();
		client.setNickname(params[0]);
		std::string replyMsg = std::to_string(client.getClientFD()) + ":" + oldNick + "!user@host NICK :" + client.getNickname();
		client.appendSendBuffer(replyMsg);
	}
	else
		client.setNickname(params[0]);

	std::cout << "Client FD " << client.getClientFD()
				<< ", nickname set to: " << client.getNickname() << std::endl;

}


bool Server::isNickUserValid(std::string cmd, std::string name) {
	if (cmd == "NICK") {
		std::regex nickName_regex(R"(^([A-Za-z\[\]\\`_^{}|])(?![$:#&~@+%])[-A-Za-z0-9\[\]\\`_^{}|]{0,8}$)");
		if (std::regex_match(name, nickName_regex) == false)
			return (FAIL);
	}
	else if (cmd == "USER") {
		std::regex userName_regex(R"(^[^\s@]{1,10}$)");
		if (std::regex_match(name, userName_regex) == false)
			return (FAIL);
	}
	return (SUCCESS);
}

void Server::handleUser(Client& client, const std::vector<std::string>& params) {

	if (params.empty() || params[0].empty()) {
		messageHandle(ERR_NICKNAMEINUSE, client, "USER", params);
		return;
	}

	const std::string& newUser = params[0];

	if (isUserDuplicate(params[0])) {
		std::cout << "Duplicate user name" << std::endl;
		return;
	}
	else if (params.size() < 4) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "USER", params);
	}
	else
	{
		client.setUsername(newUser + std::to_string(client.getClientFD()));
		client.setHostname(params[2]);
		client.setRealName(params[3]);
		messageHandle(RPL_WELCOME, client, "USER", params);

	}
	std::cout << "Client FD " << client.getClientFD()
				<< ", username: " << client.getUsername()
				<< ", Host: " << client.getHostname()
				<< ", RealName: " << client.getRealName() << std::endl;

}

void Server::handlePass(Client& client, const std::vector<std::string>& params) {

	if (params.empty() || params[0].empty()) {
		messageHandle(ERR_NEEDMOREPARAMS, client, "PASS", params);
		return;
	}

	const std::string& newPass = params[0];

	if (newPass != this->getPassword()) {
		messageHandle(ERR_PASSWDMISMATCH, client, "PASS", params);
	}
	else if (client.getIsAuthenticated()) {
		messageHandle(ERR_ALREADYREGISTRED, client, "PASS", params);
	}
	else {
		client.setPassword(newPass);
		client.setIsPassValid(true);
		client.setAuthenticated(true);
	}

	std::cout << "Client FD " << client.getClientFD()
				<< ", password set to: " << client.getPassword() << std::endl;
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

