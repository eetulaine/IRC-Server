#include "../includes/Server.hpp"

void Server::registerCommands() {
	// command CAP will be ignored

	commands["NICK"] = [this](Client& client, const std::vector<std::string>& params) {
		handleNick(client, params);
	};

	commands["USER"] = [this](Client& client, const std::vector<std::string>& params) {
		handleUser(client, params);
	};

	commands["PASS"] = [this](Client& client, const std::vector<std::string>& params) {
		handlePass(client, params);
	};
}

void Server::handleNick(Client& client, const std::vector<std::string>& params) {

	for (const std::string& param : params) {
		std::cout << "- " << param << std::endl;
	}

	// No nickname given
	if (params.empty() || params[0].empty()) {
		std::cout << "Empty nick name" << std::endl;
		//client.appendSendBuffer(some msg); // error code 431
		return;
	}

	const std::string& newNick = params[0];

	// Nickname already in use
	if (isNickDuplicate(newNick)) {
		std::cout << "Duplicate nick name" << std::endl;
		//client.appendSendBuffer(some msg); // error code 433
		return;
	}

	// Valid nickname
	client.setNickname(newNick);
	std::cout << "Client FD " << client.getClientFD()
				<< " nickname set to: " << client.getNickname() << std::endl;

}


void Server::handleUser(Client& client, const std::vector<std::string>& params) {

	for (const std::string& param : params) {
		std::cout << "- " << param << std::endl;
	}

	if (params.empty() || params[0].empty()) {
		std::cout << "Empty user name" << std::endl;
		return;
	}

	const std::string& newUser = params[0];

	if (isUserDuplicate(params[0])) {
		std::cout << "Duplicate user name" << std::endl;
		return;
	}
	else if (params.size() < 4) {
		std::cout << "Invalid number of perams" << std::endl;
		return;
	}
	else
	{
		client.setUsername(newUser);
		client.setHostname(params[2]);
		client.setRealName(params[3]);

	}
	std::cout << "Client FD " << client.getClientFD()
				<< " username: " << client.getUsername()
				<< " Host: " << client.getHostname()
				<< " RealName: " << client.getRealName() << std::endl;

}



void Server::handlePass(Client& client, const std::vector<std::string>& params) {

	for (const std::string& param : params) {
		std::cout << "- " << param << std::endl;
	}

	if (params.empty() || params[0].empty()) {
		std::cout << "Empty Password" << std::endl;
		return;
	}

	const std::string& newPass = params[0];

	if (newPass != this->getPassword()) {
		std::cout << "Password Doesnot match" << std::endl;

		return;
	}

	// Do some authentication check ***
	client.setPassword(newPass);
	std::cout << "Client FD " << client.getClientFD()
				<< " password set to: " << client.getPassword() << std::endl;
}
