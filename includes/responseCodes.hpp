#pragma once

// ****************REPLY CODES********** //

#define	RPL_WELCOME 			001 // Welcome Reply
#define	RPL_YOURHOST	 		002 // Your host
#define	RPL_CREATED 			003 // Server creation"
#define	RPL_MYINFO 				004 // <servername> <version> <available user modes> <available channel modes>"
#define RPL_ISUPPORT	 		005
#define RPL_UMODEIS 			221 // test
#define RPL_WHOISUSER 			311
#define RPL_NOTOPIC				331
#define RPL_TOPIC				332
#define RPL_INVITING			341
#define RPL_NAMREPLY			353
#define RPL_ENDOFNAMES			366
#define RPL_PONG 				399


// ****************ERROR CODES************** //

#define ERR_NOSUCHNICK 			401
#define ERR_NOSUCHSERVER 		402
#define ERR_NOSUCHCHANNEL 		403
#define ERR_CANNOTSENDTOCHAN	404 // When client cannot send msg (mode +m, +b, etc..)
#define ERR_TOOMANYTARGETS		407 // When to many target client(user) or channel
#define ERR_NOORIGIN 			409
#define ERR_NORECIPIENT 		411
#define ERR_NOTEXTTOSEND		412 // when PRIVMSG has no text to send
#define ERR_UNKNOWNCOMMAND 		421
#define ERR_NONICKNAMEGIVEN 	431 // No nickname given
#define ERR_ERRONEUSNICKNAME	432 // Erroneous nickname (e.g., contains forbidden characters)
#define ERR_NICKNAMEINUSE 		433 // Nickname is already in use
#define ERR_ERRONEUSUSER 		434 // Erroneous username
#define ERR_USERNOTINCHANNEL	441 // target user of a command not on given channel
#define ERR_NOTONCHANNEL		442 // client attempting a channel affecting command not a member on the channel
#define ERR_USERONCHANNEL		443 // user already on the channel they were invited to
#define ERR_NOTREGISTERED 		451 // client tries to execute a command before completing registration
#define ERR_NEEDMOREPARAMS 		461 // Not enough parameters
#define ERR_ALREADYREGISTERED	462 // Tries when already registered
#define ERR_PASSWDMISMATCH		464 // incorrect password
#define ERR_CHANNELISFULL		471 // channel is full (user limit reached)
#define ERR_INVITEONLYCHAN		473 // trying to join an invite-only channel without invitation
#define ERR_BADCHANNELKEY		475
#define ERR_BADCHANMASK			479 // channel name does not match the proper syntax
#define	ERR_CHANOPRIVSNEEDED	482 // user doesn't have the required channel operator rights
#define ERR_UMODEUNKNOWNFLAG	501
#define ERR_UNKNOWNMODE			472
