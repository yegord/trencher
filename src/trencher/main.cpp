#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <trench/FenceInsertion.h>
#include <trench/NaiveParser.h>
#include <trench/Program.h>
#include <trench/RobustnessChecking.h>

void help() {
	std::cout << "Usage: trencher [-f] [-r] [-rp] file..." << std::endl;
}

int main(int argc, char **argv) {
	if (argc <= 1) {
		help();
		return 1;
	}

	try {
		enum {
			ROBUSTNESS,
			ROBUSTNESS_PARALLEL,
			FENCES
		} action = FENCES;

		for (int i = 1; i < argc; ++i) {
			std::string arg = argv[i];

			if (arg == "-f") {
				action = FENCES;
			} else if (arg == "-r") {
				action = ROBUSTNESS;
			} else if (arg == "-rp") {
				action = ROBUSTNESS_PARALLEL;
			} else if (arg.size() >= 1 && arg[1] == '-') {
				throw std::runtime_error("unknown option: " + arg);
			} else {
				trench::Program program;

				{
					trench::NaiveParser parser;
					std::ifstream in(argv[i]);
					parser.parse(in, program);
				}

				switch (action) {
					case ROBUSTNESS: {
						if (trench::checkIsRobust(program)) {
							std::cout << "Program IS robust.";
						} else {
							std::cout << "Program IS NOT robust.";
						}
						break;
					}
					case ROBUSTNESS_PARALLEL: {
						if (trench::checkIsRobustParallel(program)) {
							std::cout << "Program IS robust.";
						} else {
							std::cout << "Program IS NOT robust.";
						}
						break;
					}
					case FENCES: {
						std::cout << "Number of computed fences: " << trench::computeFences(program).size() << std::endl;
						break;
					}
					default: {
						assert(!"NEVER REACHED");
					}
				}
			}
		}
	} catch (const std::exception &exception) {
		std::cerr << "trencher: " << exception.what() << std::endl;
		return 1;
	}

	return 0;
}
