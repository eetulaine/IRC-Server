#pragma once

// ****************REPLY CODES********** //

#define	RPL_WELCOME 001
// Welcome Reply

#define	RPL_YOURHOST 002
// Your host

#define	RPL_CREATED 003
// Server creation"

#define	RPL_MYINFO 004
// <servername> <version> <available user modes> <available channel modes>"

#define RPL_ISUPPORT 005

#define RPL_WHOISUSER 311

#define RPL_PONG 399

// ****************ERROR CODES************** //

#define ERR_NONICKNAMEGIVEN 431
// No nickname given

#define ERR_ERRONEUSNICKNAME 432
// Erroneous nickname (e.g., contains forbidden characters)

#define ERR_NICKNAMEINUSE 433
// Nickname is already in use

#define ERR_ERRONEUSUSER 434
// Erroneous username

#define ERR_NOTREGISTERED 451
// client tries to execute a command before completing registration

#define ERR_NEEDMOREPARAMS 461
// Not enough parameters

#define ERR_ALREADYREGISTRED 462
// Tries when already registered

#define ERR_PASSWDMISMATCH 464
// incorrect password

#define ERR_NOSUCHNICK 401
#define ERR_NOSUCHSERVER 402

#define ERR_UNKNOWNCOMMAND 421

#define ERR_NOORIGIN 409
#define ERR_NORECIPIENT 411

#define ERR_UMODEUNKNOWNFLAG 501

#define RPL_UMODEIS 221 // test

