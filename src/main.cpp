#include "app.h"
#include <iostream>


int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	try {
		std::unique_ptr<App> app = std::make_unique<App>(600, 600);
		app->loadScene();
		app->run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	
	return 0;
}
