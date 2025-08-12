#include <iostream>

int	main(int ac, char **av) {

	(void)av;
	if (ac != 3)
		std::cout << "Invalid arguments!\n";
	return 0;
}