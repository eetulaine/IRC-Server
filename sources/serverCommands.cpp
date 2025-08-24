#include "../includes/Server.hpp"

void Server::registerCommands() {
    commands["NICK"] = [this](Client& client, const std::vector<std::string>& params) {
		handleNick(client, params);
    };
}

void Server::handleNick(Client& client, const std::vector<std::string>& params) {
	(void)client;
	std::cout << "Handling NICK command. Parameters: " << std::endl;
    for (const std::string& param : params) {
        std::cout << "- " << param << std::endl;
    }
};
