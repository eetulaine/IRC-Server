#pragma once

#include <string>
#include "Client.hpp"
#include <vector>
#include <map>
#include <set>


class Client;
class Channel {
	private:
		std::string name_;
		std::string topic_;
		// std::string topic_setter_;  // is it needed? 
		std::string key_;  //password for joining protected channels
		std::set<Client*> clients_;     // pointer to store clients

	public:
		Channel(const std::string &name);
		~Channel();

		
		//METHODS
		void addClinet(Client *client);
		void removeClient(Client *client);
		// bool hasKey(Client *client) const; // check for protected channel






		// ACCESSORS
		std::string getName() const;
		std::string getTopic(); // make it const?
		std::string getKey() const;
		const std::set<Client *> &getClients() const; // list all clients in the channel

};

