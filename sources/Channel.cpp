#include "Channel.hpp"

Channel::Channel(const std::string &name, const std::string& key)
	: name_(name), key_(), keyProtected_(false) {
	
	std::cout << GREEN "=== CHANNEL CREATED ===\n" END_COLOR;

	std::cout << "Name: " << name_ << "\n";
	if(!key.empty()) {
		setChannelKey(key);
		std::cout << "Password: " << key_ << "\n";
	}
	else
		std::cout << "Password: Not provided\n";
		
}

Channel::~Channel() {

	std::cout << RED "=== CHANNEL DESTROYED\n" END_COLOR;
}


//PUBLIC METHODS
void Channel::addMember(Client *client) {

	auto status = members_.insert(client);
	if (status.second)
		std::cout << GREEN << "Member <" << client->getNickname() << "> is successfully added to channel< " << name_ << ">\n" << END_COLOR;
	else
		std::cout << "Member <" << client->getNickname() << "> already exists in the channel\n";

}

void Channel::removeMember(Client *client) {

	size_t status =  members_.erase(client);
	if (status)
		std::cout << "Member <" << client->getNickname() << "> is removed from" << name_ << "channel\n";
	else
		std::cout << "Menber <" << client->getNickname() << "> was not found in channel\n";
}

bool Server::channelExists(const std::string& channelName) {
    return (channelMap_.find(channelName) != channelMap_.end());
}

Channel* Server::createChannel(const std::string& channelName, const std::string& channelKey) {


    Channel* newChannel = new Channel(channelName, channelKey);

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
	if ((name[0] != '#' && name[0] != '&'))
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
	keyProtected_ = true;
}

std::string Channel::getChannelName() const {
	return this->name_;
}

// std::string Channel::getTopic() {
// 	return this->topic_;
// }

std::string Channel::getChannelKey() const {
	return this->key_;
}

Channel* Server::getChannel(const std::string& channelName) {
    auto it = channelMap_.find(channelName);
    if (it != channelMap_.end())
        return it->second;
    return nullptr;

}
