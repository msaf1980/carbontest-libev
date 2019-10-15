#include <atomic>
#include <iostream>
#include <string>

#include "config.hpp"
#include "runner.hpp"

int main(int argc, char *argv[]) {
	try {
		parseArgs(config, argc, argv);
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return (1);
	}

	return runClients();
}
