#include <fstream>
#include <iostream>

#include <trench/NaiveParser.h>
#include <trench/Program.h>
#include <trench/RobustnessChecking.h>

#include "Examples.h"

void help() {
	std::cout << "Usage: trencher file..." << std::endl;
}

int main(int argc, char **argv) {
	if (argc <= 1) {
		help();
		return 1;
	}

	try {
		bool parallel = false;

		for (int i = 1; i < argc; ++i) {
			std::string arg = argv[i];

			if (arg == "-p") {
				parallel = true;
			} else {
				trench::Program program;

				if (arg == "dekker") {
					dekker(program, 7, false);
				} else if (arg == "dekker_fenced") {
					dekker(program, 7, true);
				} else {
					trench::NaiveParser parser;
					std::ifstream in(argv[i]);
					parser.parse(in, program);
				}

				if (parallel) {
					std::cout << "checkIsRobustParallel: " << trench::checkIsRobustParallel(program) << std::endl;
				} else {
					std::cout << "checkIsRobust: "         << trench::checkIsRobust(program) << std::endl;
				}
			}
		}
	} catch (const std::exception &exception) {
		std::cerr << "trencher: " << exception.what() << std::endl;
		return 1;
	}

	return 0;
}
