#include "Channel.hpp"
#include "../includes/macros.hpp"
#include "../includes/Server.hpp"



Channel::Channel(Client* client, const std::string &name, const std::string& key)
	: name_(name), key_(""), keyProtected_(false) {

	if(!key.empty())
		setChannelKey(key);

	logMessage(INFO, "CHANNEL " + this->getChannelName(),": New channel created, Name: [" 
        + this->getChannelName() + "], Key: [" + this->getChannelKey() + "]");
	setOperator(client, true); // set the client creating the channel as operator by default
	if (isOperator(client))
		std::cout << GREEN "Client " << client->getNickname() << " is operator" END_COLOR << std::endl;
}

Channel::~Channel() {
	logMessage(WARNING, "CHANNEL " + this->getChannelName(), ": Channel destroyed");
}


//PUBLIC METHODS
void Channel::addMember(Client *client) {
	
	//   std::cout <<  "HEEEEEEEEEE\n";
    // Check if client already exists using find()
    // if (members_.find(client) != members_.end()) {
    //     logMessage(WARNING, "CHANNEL " + this->getChannelName(), 
    //               ": Client '" + client->getNickname() + "' is already a member.");
    //     return;
    // }
    // std::cout <<  "HOOOOOOOOO\n";
    // Client doesn't exist - add them
    members_.insert(client);

	client->activeChannels(this->getChannelName());

    logMessage(INFO, "CHANNEL " + this->getChannelName(), 
              ": New member joined: " + client->getNickname());
}

void Channel::removeMember(Client *client) {

	size_t status =  members_.erase(client);
	if (status)
		std::cout << "Member <" << client->getNickname() << "> is removed from" << name_ << "channel\n";
	else
		std::cout << "Menber <" << client->getNickname() << "> was not found in channel\n";
}

Channel* Server::createChannel(Client* client, const std::string& channelName, const std::string& channelKey) {

    Channel* newChannel = new Channel(client, channelName, channelKey);

	// check for memory allocation!
    channelMap_[channelName] = newChannel; // server stores and keeps track of created channels
	
    return newChannel;
}

bool Channel::requiresPassword() {
	return (this->keyProtected_);
}

bool Channel::checkChannelKey(const std::string& providedKey) {
	if (!requiresPassword())
		return true;
	if (providedKey.empty())
		return false;
	return (this->key_ == providedKey);

}

bool Channel::isValidChannelName(const std::string& name) {

	if (name.empty() || name.size() > 50)
		return false;
	if (name[0] != '#' && name[0] != '&')
		return false;

	static const std::string inavalidChar = " ,\a:";
	for (size_t i = 0; i < name.size(); i++) {
		if (inavalidChar.find(name[i]) != std::string::npos)
			return false;
	}
	return true;
}


// ACCESSORS
void Channel::setChannelKey(const std::string& key) {

	key_ = key;
	keyProtected_ = !key.empty();
}

std::string Channel::getChannelKey() const {
	return this->key_;
}


std::string Channel::getChannelName() const {
	return this->name_;
}

bool Server::channeClientlExist(Client* client, const std::string& channelName) {
	if (channelMap_.find(channelName) != channelMap_.end() && client->activeChannels(channelName) == true)
		return true;
    return (false);
}


Channel* Server::getChannel(Client* client, const std::string& channelName) {
	(void)client;
    auto it = channelMap_.find(channelName);
    if (it != channelMap_.end()) {
		// logMessage(WARNING, "CHANNEL " + channelName, 
        //     ": Client '" + client->getNickname() + "' is already a member");
		return it->second;
	}
        
    return nullptr;

}

const std::set<Client*>& Channel::getMembers() const {
	return members_;
}

const std::set<Client*>& Channel::getOperators() const {
	return operators_;
}

bool Channel::isOperator(Client* client) const {
	return operators_.find(client) != operators_.end();
}

void Channel::setOperator(Client* client, bool isOperator) {
	if (isOperator)
		operators_.insert(client);
	else
		operators_.erase(client);

}

// std::string Channel::getTopic() {
// 	return this->topic_;
// }
