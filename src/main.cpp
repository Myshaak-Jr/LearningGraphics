#include "app.h"
#include <iostream>


int runApp();

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	return runApp();
}

int runApp() {
	try {
		std::unique_ptr<App> app = std::make_unique<App>(600, 600);
		app->run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}