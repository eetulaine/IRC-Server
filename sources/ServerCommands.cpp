#include "../includes/Server.hpp"

void Server::registerCommands() {
    commands["NICK"] = [this](Client& client, const std::vector<std::string>& params) {
		handleNick(client, params);
    };
	commands["QUIT"] = [this](Client& client, const std::vector<std::string>& params) {
		handleQuit(client, params);
    };
}

void Server::handleNick(Client& client, const std::vector<std::string>& params) {
	(void)client;
	std::cout << "Handling NICK command. Parameters: " << std::endl;
    for (const std::string& param : params) {
        std::cout << "- " << param << std::endl;
    }
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