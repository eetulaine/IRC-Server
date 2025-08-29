#include "../includes/Server.hpp"
#include "../includes/macros.hpp"
//#include "../includes/responseCodes.hpp"

bool Server::isNickUserValid(std::string cmd, std::string name) {
	if (cmd == "NICK") {
		std::regex nickName_regex(R"(^([A-Za-z\[\]\\`_^{}|])(?![$:#&~@+%])[-A-Za-z0-9\[\]\\`_^{}|]{0,8}$)");
		if (std::regex_match(name, nickName_regex) == false)
			return (FAIL);
	}
	else if (cmd == "USER") {
		std::regex userName_regex(R"(^[^\s@]{1,10}$)");
		if (std::regex_match(name, userName_regex) == false)
			return (FAIL);
	}
	return (SUCCESS);
}


void logMessage(logMsgType type, const std::string &action, const std::string &msg)
{
	// to skip the PING log
	if (msg.find("PING") != std::string::npos || msg.find("PONG") != std::string::npos)
		return;

	time_t currentTime = time(nullptr);
	tm *ltm = localtime(&currentTime);

	std::cout << "[" << std::put_time(ltm, "%d.%m.%Y %H:%M:%S") << "] ";
	switch (type)
	{
		case INFO:
			std::cout << GREEN;
			std::cout << "[INFO] ";
			break;
		case WARNING:
			std::cout << YELLOW;
			std::cout << "[WARNING] ";
			break;
		case ERROR:
			std::cout << RED;
			std::cout << "[ERROR]";
			break;
	}
	std::cout << END_COLOR;
	std::cout << "[" << action << "] " << msg << std::endl;
}
