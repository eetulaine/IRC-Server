#include <iostream>
#include <string>
#include <cstdlib>
#include <regex>
//#include <exception>

bool isPasswordValid(const std::string &password)
{
	if (password.empty())
		return (false);

	std::regex passPattern("^[a-zA-Z0-9]{6,16}$");

	return (std::regex_match(password, passPattern));
}

void argsValidation(int argc, char **argv)
{
	if (argc != 3)
		throw std::runtime_error("Invalid number of arguments");

	int port = std::stoi(static_cast<std::string>(argv[1]));

	if (port < 6665 || port > 6669) // need to check actual port range
		throw std::runtime_error("Invalid listening port range(try 6665-6669)");
	// if server only allow one port, need to validation in the above method accordingly

	if (!isPasswordValid(argv[2]))
		throw std::runtime_error("Invalid password");
}

int main(int argc, char **argv)
{
	try
	{
		argsValidation(argc, argv);
		std::cout << "ValidationSuccess" << std::endl;
	}
	catch(const std::exception& e)
	{
		std::cerr << "\033[0;31m" << "[Error] " << "\033[0;37m";
		std::cerr << e.what() << std::endl;
		return (1);
	}
	std::cout << "Star Sarver init" << std::endl;
	return (0);
}
