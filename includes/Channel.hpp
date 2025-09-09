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
		bool inviteOnly_;				// channel is invite only
		bool topicOperatorOnly_;		// if only operator can change channel topic
		std::string topic_;				// channel topic
		std::set<Client*> members_;     // pointer to store clients
		std::set<Client*> operators_;	// keep track of operator rights
		std::set<Client*> invited_;		// list of invitees of the channel
		bool keyProtected_;
		bool inviteOnly_;				// channel is invite only

	public:
		Channel(Client* client, const std::string &name, const std::string& );
		~Channel();


		//METHODS
		bool isMember(Client* client);
		void addChannelMember(Client *client);
		void addInvite(Client* client);
		void removeMember(Client *client);
		static bool isValidChannelName(const std::string& name);
		bool isOperator(Client* client) const;
		//void removeOperator(Client* client);

		// ACCESSORS
		void setChannelKey(const std::string& key);
		void setOperator(Client* client, bool isOperator);
		bool isKeyProtected();
		bool isInviteOnly();
		bool isClientInvited(Client* client) const;
		void setInviteOnly(bool inviteOnly);
		std::string getChannelName() const;
		// std::string getTopic(); // make it const?
		std::string getChannelKey() const;
		bool checkForChannelKey(Channel* channel, Client* client, const std::string& providedKey);
		const std::set<Client *> &getMembers() const; 			// list all clients in the channel
		const std::set<Client*> &getOperators() const;			// list all operators
		//size_t getOperatorCount() const;

		// CHANNEL TOPIC
		bool isTopicOperatorOnly() const;
		std::string getTopic() const;
		void setTopic(const std::string& topic);
		void setTopicOperatorOnly(bool topicOperatorOnly);
};
