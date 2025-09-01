#include "Channel.hpp"
#include "../includes/macros.hpp"
#include "../includes/Server.hpp"
#include "../includes/responseCodes.hpp"


Channel::Channel(Client* client, const std::string &name, const std::string& key)
	: name_(name), key_(""), keyProtected_(false) {

	if(!key.empty())
		setChannelKey(key);

	logMessage(INFO, "CHANNEL", "Channel created. Name: [" 
		+ this->getChannelName() + "], Key: [" + this->getChannelKey() + "]");
	setOperator(client, true); // set the client creating the channel as operator by default
	if (isOperator(client))
		std::cout << GREEN "Client " << client->getNickname() << " is operator" END_COLOR << std::endl;
}

Channel::~Channel() {
	logMessage(WARNING, "CHANNEL", ": Channel destroyed");
}


//PUBLIC METHODS
void Channel::addChannelMember(Client *client) {
	
	members_.insert(client);
	logMessage(INFO, "CHANNEL", this->getChannelName() + 
		": Client " +  client->getNickname() + " Joined");
}

// void Channel::removeMember(Client *client) {

// 	size_t status =  members_.erase(client);
// 	if (status)
// 		std::cout << "Member <" << client->getNickname() << "> is removed from" << name_ << "channel\n";
// 	else
// 		std::cout << "Menber <" << client->getNickname() << "> was not found in channel\n";
// }

bool Channel::isKeyProtected() {
	return (this->keyProtected_);
}

bool Channel::isOperator(Client* client) const {
	return operators_.find(client) != operators_.end();
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

bool Channel::checkForChannelKey(Channel* channel, Client* client, const std::string& providedKey) {

	if (!channel->isKeyProtected())
		return true;
	if (providedKey.empty()) {
		logMessage(ERROR, "CHANNEL",
			"Key required: " + client->getNickname() + " failed to join");
		return false;
	}
	if (getChannelKey() != providedKey) {
		logMessage(ERROR, "CHANNEL",
			"Incorrect key: " + client->getNickname() + " failed to join");
		return false;
	}
	logMessage(INFO, "CHANNEL", "Joined key-protected channel: " + getChannelName());
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

Channel* Server::getChannel(const std::string& channelName) {
	auto it = channelMap_.find(channelName);
	if (it != channelMap_.end())
		return it->second;
	return nullptr;

}

const std::set<Client*>& Channel::getMembers() const {
	return members_;
}

const std::set<Client*>& Channel::getOperators() const {
	return operators_;
}

void Channel::setOperator(Client* client, bool isOperator) {
	if (isOperator)
		operators_.insert(client);
	else
		operators_.erase(client);

}
