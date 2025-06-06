#include <iostream>
#include "config.h"
#include "research/research.hpp"

int main(void)
{
	std::cout << "pIOn has started!\n" << std::endl;
	try {
		pIOn::Config config = pIOn::getConfig(ROOT_DIR "configs/config_pIOn.json");
		pIOn::validateConfig(config);

		pIOn::makeResearch(config);
		std::cout << "success!" << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	catch (...) {
		std::cerr << "Unknown exception!" << std::endl;
	}

	return 0;
}
