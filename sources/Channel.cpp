#include "Channel.hpp"

Channel::Channel(const std::string &name, const std::string& key) {

	this->name_ = name;
	if (!key.empty())
		this->key_ = key;
	this->keyProtected_ = false;
	// this->setChannelKey(key);
	std::cout << GREEN "=== CHANNEL CREATED ===\n" END_COLOR;
	std::cout << "Channel name: " << name_ << "\n";
// 	if (!key_.empty())
// 		std::cout << "Channel key: " << key_ << "\n";
}

Channel::~Channel() {
	std::cout << RED "=== CHANNEL DESTROYED\n" END_COLOR;
}



//PUBLIC METHODS
void Channel::addMember(Client *client) {

	auto status = this->members_.insert(client);
	if (status.second)
		std::cout << "Client" << client->getNickname() << "added to channel successfully!\n";
	else
		std::cout << "Client" << client->getNickname() << "already exists in the channel\n";

}

void Channel::removeMember(Client *client) {

	size_t status =  members_.erase(client);
	if (status)
		std::cout << "Client" << client->getNickname() << "removed from channel\n";
	else
		std::cout << "Client was not found in channel\n";
}

bool Server::channelExists(const std::string& channelName) {
    return (channelMap_.find(channelName) != channelMap_.end());
}

Channel* Server::createChannel(const std::string& channelName, const std::string& channelKey) {

    if (channelExists(channelName))
        return channelMap_.at(channelName);

    Channel* newChannel = new Channel(channelName, channelKey);

	// if (!channelKey.empty()) {
    //         newChannel->setChannelKey(channelKey); 
    // }

    channelMap_[channelName] = newChannel;
    std::cout << "Channel " << newChannel->getName() << "was created!\n";
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



// ACCESSORS

void Channel::setChannelKey(const std::string& key) {

	this->key_ = key;
	keyProtected_ = true;
}

std::string Channel::getName() const {
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
