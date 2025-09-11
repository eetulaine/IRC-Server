#include "../includes/Server.hpp"
#include "../includes/macros.hpp"
//#include "../includes/responseCodes.hpp"

bool Server::isNickUserValid(std::string cmd, std::string name) {
	if (cmd == "NICK") {
		std::regex nickName_regex(R"(^([A-Za-z\[\]\\`_^{}|])(?![$:#&~@+%])[-A-Za-z0-9\[\]\\`_^{}|]{0,15}$)");
		if (std::regex_match(name, nickName_regex) == false)
			return (true);
	}
	else if (cmd == "USER") {
		std::regex userName_regex(R"(^[^\s@]{1,10}$)");
		if (std::regex_match(name, userName_regex) == false)
			return (true);
	}
	return (false);
}

Channel* Server::createChannel(Client* client, const std::string& channelName, const std::string& channelKey) {

    Channel* newChannel = new Channel(client, channelName, channelKey);
    channelMap_[channelName] = newChannel;
    return newChannel;
}

// removes Client from all the Channels joined (both members within Channel and joinedChannels within Client)
void Server::leaveAllChannels(Client& client) {
	std::set<std::string> channelsToLeave = client.getJoinedChannels(); // copy the set first so we can safely modify it (leaveChannel())
	for (const std::string& channelName : channelsToLeave) {
		Channel* channel = getChannel(channelName);
		if (channel) {
			channel->removeMember(&client);
			client.leaveChannel(channelName);
		}
	}
}

bool Server::channelExists(const std::string& channelName) {

	return (channelMap_.find(channelName) != channelMap_.end());
}

void logMessage(logMsgType type, const std::string &action, const std::string &msg) {
	// to skip the PING log
	if (msg.find("PING") != std::string::npos || msg.find("PONG") != std::string::npos)
		return;

	time_t currentTime = time(nullptr);
	tm *ltm = localtime(&currentTime);

	std::cout << "[" << std::put_time(ltm, "%d.%m.%Y %H:%M:%S") << "] ";
	switch (type)
	{
		case INFO:
			std::cout << GREEN;
			std::cout << "[INFO] ";
			break;
		case WARNING:
			std::cout << YELLOW;
			std::cout << "[WARNING] ";
			break;
		case ERROR:
			std::cout << RED;
			std::cout << "[ERROR]";
			break;
		case DEBUG:
			std::cout << BLUE;
			std::cout << "[DEBUG]";
			break;
	}
	std::cout << END_COLOR;
	std::cout << "[" << action << "] " << msg << std::endl;
}
