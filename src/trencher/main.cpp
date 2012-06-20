#include <fstream>
#include <iostream>

#include <trench/NaiveParser.h>
#include <trench/Program.h>
#include <trench/RobustnessChecking.h>

void help() {
	std::cout << "Usage: trencher file..." << std::endl;
}

int main(int argc, char **argv) {
	if (argc <= 1) {
		help();
		return 1;
	}

	try {
		for (int i = 1; i < argc; ++i) {
			trench::Program program;
			trench::NaiveParser parser;

			std::ifstream in(argv[i]);
			parser.parse(in, program);

			std::cerr << "checkIsRobust:         " << trench::checkIsRobust(program) << std::endl;
			std::cerr << "checkIsRobustParallel: " << trench::checkIsRobustParallel(program) << std::endl;
		}
	} catch (const std::exception &exception) {
		std::cerr << "trencher: " << exception.what() << std::endl;
		return 1;
	}

	return 0;
}
