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

    commands["JOIN"] = [this](Client& client, const std::vector<std::string>& params) {
	    handleJoin(client, params);

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

// CHANNEL ----- handle join command
/**
 * @breif Handles each requested channel from JOIN command.
 */
void Server::handleJoin(Client& client, const std::vector<std::string>& params) {

    //DEBUG
    std::cout << "DEBUG: Entered handleJoin\n";
    std::cout << "DEBUG: Client Nickname: " << client.getNickname() << "\n";

    if (params.empty()) {
        //sendReply();  ERR code + client name + "not enough parameters\n."
        std::cout << "empty parameters\n.";
        return ;
    }


    std::vector<std::string>  requestedChannels = split(params[0], ',');
    std::vector<std::string>  keys = (params.size() > 1) ? split(params[1], ',') : std::vector<std::string>{};
    
    for (int i = 0; i < requestedChannels.size(); i++) {
        const std::string& channelName = requestedChannels[i]; // -> Validate channel name !
        const std::string& channelKey = (i < keys.size()) ? keys[i] : ""; // -> Validate channel key !

        if (client.hasJoinedChannel(channelName)) {
                continue;
        }
                
        Channel* channel = channelExists(channelName)
                            ? getChannel(channelName)
                            : createChannel(channelName, channelKey);
                            
        // seperate getChannel and create Channel logic !!

        if (channel->requiresPassword()) {
            if (channel->getChannelKey() != channelKey) {
                std::cout << "cannot join channel " << channel->getName() << ".\n";
                continue;
                // return ;
            }
        }
       
        channel->addMember(&client);      // server-side  -> add client to channel
        client.joinChannel(channelName);  // client side  -> track joined channels
        //channel->broadcast(client.getNickname() + " has joined " + channelName);
    }

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

	std::cout << "Handling QUIT command. Parameters: " << std::endl;
    if (!client.isConnected() || !client.isAuthenticated()) {//no broadcasting from unconnected or unregistered clients
		closeClient(client);
		return;
	}
	std::string quitMessage = "Client quit";
	if (!params.empty())
		quitMessage = " QUIT :" + params[0];
	quitMessage = ":" + client.getNickname() + "!" + 
						client.getUsername() + "@" + 
						client.getHostname() + quitMessage + "\r\n";
	client.appendSendBuffer(quitMessage);
	client.sendData();
	closeClient(client);
};

void Server::closeClient(Client& client) {

	int clientfd = client.getClientFD();
	int epollfd = client.getEpollFd();
	client.setConnected(false);

	struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = clientfd;
	epoll_ctl(epollfd, EPOLL_CTL_DEL, clientfd, &ev);
	close(clientfd);
	clients_.erase(clientfd);
}
