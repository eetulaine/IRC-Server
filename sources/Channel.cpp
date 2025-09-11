#include "Channel.hpp"
#include "../includes/macros.hpp"
#include "../includes/Server.hpp"
#include "../includes/responseCodes.hpp"


Channel::Channel(Client* client, const std::string &name, const std::string& key)
	: name_(name), key_(""), keyProtected_(false), inviteOnly_(false), topicOperatorOnly_(true), topic_("") {

	if(!key.empty())
		setChannelKey(key);

	logMessage(INFO, "CHANNEL", "Channel created. Name: ["
		+ this->getChannelName() + "], Key: [" + this->getChannelKey() + "]");
	setOperator(client, true); // set the client creating the channel as operator by default
	if (isOperator(client))
		logMessage(DEBUG, "CLIENT", "Client " + client->getNickname() + " is operator");
}

Channel::~Channel() {
	logMessage(WARNING, "CHANNEL", ": Channel destroyed");
}

//PUBLIC METHODS
bool Channel::isMember(Client* client) {
	auto it = members_.find(client);
	if (it == members_.end()) {
		return false;
	}
	return true;
}

void Channel::addChannelMember(Client *client) {

	members_.insert(client);
	logMessage(INFO, "CHANNEL", this->getChannelName() +
		": Client " +  client->getNickname() + " Joined");
}

void Channel::removeMember(Client *client) {

 	size_t status =  members_.erase(client);
 	if (status)
		logMessage(DEBUG, "CHANNEL", "Member <" + client->getNickname() + "> is removed from channel " + this->getChannelName());
 	else
		logMessage(DEBUG, "CHANNEL", "Member <" + client->getNickname() + "> not found on channel " + this->getChannelName());
 }

 void Channel::addInvite(Client *client) {
	invited_.insert(client);
	logMessage(DEBUG, "CHANNEL", this->getChannelName() +
		": Client " +  client->getNickname() + " added to invited list");
}

bool Channel::isKeyProtected() {
	return (this->keyProtected_);
}

bool Channel::isInviteOnly() {
	return inviteOnly_;
}

bool Channel::isClientInvited(Client* client) const {
	auto it = invited_.find(client);
	if (it != invited_.end())
		return true;
	return false;
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

// TOPIC ACCESSORS

bool Channel::isTopicOperatorOnly() const {
	return topicOperatorOnly_;
}

std::string Channel::getTopic() const {
	return topic_;
}

void Channel::setTopic(const std::string& topic) {
	topic_ = topic;
}

void Channel::setTopicOperatorOnly(bool topicOperatorOnly) {
	topicOperatorOnly_ = topicOperatorOnly;
}

// ACCESSORS
void Channel::setChannelKey(const std::string& key) {

	key_ = key;
	keyProtected_ = !key.empty();
}

void Channel::setInviteOnly(bool inviteOnly) {
	inviteOnly_ = inviteOnly;
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
