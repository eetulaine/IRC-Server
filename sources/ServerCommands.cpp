#include "../includes/Server.hpp"

void Server::registerCommands() {
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
}

void Server::handleNick(Client& client, const std::vector<std::string>& params) {
	(void)client;
	std::cout << "Handling NICK command. Parameters: " << std::endl;
    for (const std::string& param : params) {
        std::cout << "- " << param << std::endl;
    }
};