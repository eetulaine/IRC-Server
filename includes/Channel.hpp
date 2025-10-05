#pragma once

#include <string>
#include "../includes/Client.hpp"
#include <vector>
#include <map>
#include <set>


class Client;

class Channel {
	private:
		std::string name_;
		std::string key_;  				//password for joining protected channels

		bool keyProtected_;
		bool inviteOnly_;				// channel is invite only
		bool topicOperatorOnly_;		// if only operator can change channel topic
		int userLimit_;					// user limit, modifiable by mode -l, default is -1

		std::string topic_;				// channel topic
		std::set<Client*> members_;     // pointer to store clients
		std::set<Client*> operators_;	// keep track of operator rights
		std::set<Client*> invited_;		// list of invitees of the channel

	public:
		Channel(Client* client, const std::string &name, const std::string& );
		~Channel();

		//METHODS
		bool isMember(Client* client);
		void addChannelMember(Client *client);
		void addInvite(Client* client);
		void removeMember(Client *client);

		bool isOperator(Client* client) const;
		std::string getModeString();


		// ACCESSORS
		void setChannelKey(const std::string& key);
		void setKeyProtected(bool keyProtected);
		void setOperator(Client* client, bool isOperator);
		bool isKeyProtected();
		bool isInviteOnly();
		bool isClientInvited(Client* client) const;
		void setInviteOnly(bool inviteOnly);
		std::string getName() const;

		std::string getChannelKey() const;
		bool checkKey(Channel* channel, Client* client, const std::string& providedKey);
		const std::set<Client *> &getMembers() const; 			// list all clients in the channel
		const std::set<Client*> &getOperators() const;			// list all operators
		int getUserLimit() const;
		void setUserLimit(int userLimit);
		bool checkChannelLimit(Client &client, Channel &channel);

		// CHANNEL TOPIC
		bool isTopicOperatorOnly() const;
		std::string getTopic() const;
		void setTopic(const std::string& topic);
		void setTopicOperatorOnly(bool topicOperatorOnly);
};
bool isValidChannelName(const std::string& name);
bool isValidChannelKey(const std::string &key);
