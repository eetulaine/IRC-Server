#include "Channel.hpp"

Channel::Channel(const std::string &name) : name_(name), topic_("") {
	std::cout << GREEN "=== CHANNEL CREATED ===\n" END_COLOR;
	std::cout << "Channel name: " << name_ << "\n";
	//std::cout << "Channel topic: " << topic_ << "\n";
}

Channel::~Channel() {
	std::cout << RED "=== CHANNEL DESTROYED\n" END_COLOR;
}


std::string Channel::getName() const {
	return this->name_;
}


std::string Channel::getTopic() {
	return this->topic_;
}

std::string Channel::getKey() const {
	return this->key_;
}


void Channel::addClinet(Client *client) {

	auto status = this->clients_.insert(client);
	if (status.second)
		std::cout << "Client" << client->getNickname() << "added to channel successfully!\n";
	else
		std::cout << "Client" << client->getNickname() << "already exists in the channel\n";

}

void Channel::removeClient(Client *client) {

	size_t status =  clients_.erase(client);
	if (status)
		std::cout << "Client" << client->getNickname() << "removed from channel\n";
	else
		std::cout << "Client was not found in channel\n";
}


