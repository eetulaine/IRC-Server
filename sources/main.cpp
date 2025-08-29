#include <iostream>
#include <string>
#include <cstdlib>
#include <regex>
#include "../includes/Server.hpp"
#include "../includes/macros.hpp"
//#include <exception>

bool isPasswordValid(const std::string &password)
{
	if (password.empty())
		return (false);

	std::regex passPattern("^[a-zA-Z0-9]{6,16}$");

	return (std::regex_match(password, passPattern));
}

int portValidation(const std::string &_port)
{
	int port = std::stoi(static_cast<std::string>(_port));

	// if server only allow one port, need to validation in the above method accordingly
	if (port < 6665 || port > 6669) // need to check actual port range
		throw std::runtime_error("Invalid listening port range(try 6665-6669)");
	return (port);
}

int main(int argc, char **argv)
{
	logMessage(INFO, "MAIN", "Program started");
	try
	{
		if (argc != 3)
			throw std::runtime_error("Invalid number of arguments");

		int port = portValidation(argv[1]);
		if (!isPasswordValid(argv[2]))
			throw std::runtime_error("Invalid password");
		logMessage(INFO, "MAIN", "Validation Done");
		Server ircserv(port, argv[2]);
		ircserv.startServer();
	}
	catch(const std::exception& e)
	{
		logMessage(ERROR, "MAIN", e.what());
		return (1);
	}
	logMessage(INFO, "MAIN", "Server activity finished");
	return (0);
}
