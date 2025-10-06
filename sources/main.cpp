#include <iostream>
#include <string>
#include <cstdlib>
#include <regex>
#include "../includes/Server.hpp"
#include "../includes/macros.hpp"

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

	if (port < 6665 || port > 6669)
		throw std::runtime_error("Invalid listening port range(try 6665-6669)");
	return (port);
}

int main(int argc, char **argv)
{
	try
	{
		if (argc != 3)
			throw std::runtime_error("Invalid number of arguments");

		int port = portValidation(argv[1]);
		if (!isPasswordValid(argv[2]))
			throw std::runtime_error("Invalid password");
		Server ircserv(port, argv[2]);
		ircserv.startServer();
	}
	catch(const std::exception& e)
	{
		logMessage(ERROR, "MAIN", e.what());
		return (1);
	}
	return (0);
}
