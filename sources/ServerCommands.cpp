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

	commands["PONG"] = [this](Client& client, const std::vector<std::string>& params) {
		handlePong(client, params);
	};

	commands["NICK"] = [this](Client& client, const std::vector<std::string>& params) {
		handleNick(client, params);
    };

	commands["QUIT"] = [this](Client& client, const std::vector<std::string>& params) {
		handleQuit(client, params);
    };
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
	else if (params[0] != "IRCS") {
		messageHandle(ERR_NOSUCHSERVER, client, "PING", params);
	}
	else
		messageHandle(RPL_PONG, client, "PING", params);
}

void Server::handlePong(Client& client, const std::vector<std::string>& params) {
	if (params.empty()) {
		messageHandle(ERR_NOORIGIN, client, "PING", params);
	}
	else if (params[0] != client.getClientIdentifier()) {
		messageHandle(ERR_NOSUCHSERVER, client, "PING", params);
	}
}

void Server::handleNick(Client& client, const std::vector<std::string>& params) {

	// No nickname given
	if (params.empty() || params[0].empty()) {
		messageHandle(ERR_NONICKNAMEGIVEN, client, "NICK", params);
	}

	const std::string& newNick = params[0];

	// Nickname already in use
	if (isNickDuplicate(newNick)) {
		messageHandle(ERR_NICKNAMEINUSE, client, "NICK", params);
	}

	// Valid nickname
	client.setNickname(newNick);
	std::cout << "Client FD " << client.getClientFD()
				<< ", nickname set to: " << client.getNickname() << std::endl;

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
		client.setUsername(newUser);
		client.setHostname(params[2]);
		client.setRealName(params[3]);

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
		client.authenticateClient();
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
	/* === BROADCAST MESSAGE??===
	quitMessage = ":" + client.getNickname() + "!" + 
						client.getUsername() + "@" + 
						client.getHostname() + quitMessage + "\r\n"; */
};

