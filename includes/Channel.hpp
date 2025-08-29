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
		// std::string topic_;
		// std::string topic_setter_;  // is it needed? 
		std::string key_;  				//password for joining protected channels
		bool keyProtected_;
		std::set<Client*> members_;     // pointer to store clients

	public:
		Channel(const std::string &name, const std::string& );
		~Channel();

		
		//METHODS
		bool isMember(Client* client);
		void addMember(Client *client);
		void removeMember(Client *client);
		// bool hasKey(Client *client) const; 		// check for protected channel
		static bool isValidChannelName(const std::string& name);


		// ACCESSORS
		void setChannelKey(const std::string& key);
		bool requiresPassword();
		std::string getChannelName() const;
		std::string getTopic(); // make it const?
		std::string getChannelKey() const;
		bool checkChannelKey(const std::string& providedKey);
		const std::set<Client *> &getMembers() const; 			// list all clients in the channel

};

