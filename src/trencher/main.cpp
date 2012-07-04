#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <trench/FenceInsertion.h>
#include <trench/NaiveParser.h>
#include <trench/Program.h>
#include <trench/RobustnessChecking.h>

void help() {
	std::cout << "Usage: trencher [-r] [-f] file..." << std::endl;
}

int main(int argc, char **argv) {
	if (argc <= 1) {
		help();
		return 1;
	}

	try {
		enum {
			ROBUSTNESS,
			FENCES,
			TRIANGULAR_RACE_FREEDOM,
			TRF_FENCES
		} action = FENCES;

		for (int i = 1; i < argc; ++i) {
			std::string arg = argv[i];

			if (arg == "-r") {
				action = ROBUSTNESS;
			} else if (arg == "-f") {
				action = FENCES;
			} else if (arg == "-trf") {
				action = TRIANGULAR_RACE_FREEDOM;
			} else if (arg == "-ftrf") {
				action = TRF_FENCES;
			} else if (arg.size() >= 1 && arg[0] == '-') {
				throw std::runtime_error("unknown option: " + arg);
			} else {
				trench::Program program;

				{
					trench::NaiveParser parser;
					std::ifstream in(argv[i]);
					if (!in) {
						throw std::runtime_error("can't open file: " + arg);
					}
					parser.parse(in, program);
				}

				switch (action) {
					case ROBUSTNESS: {
						if (trench::isAttackFeasible(program, false)) {
							std::cout << "Program IS NOT robust." << std::endl;
						} else {
							std::cout << "Program IS robust." << std::endl;
						}
						break;
					}
					case FENCES: {
						std::cout << "Number of computed fences for enforcing robustness: "
						          << trench::computeFences(program, false).size() << std::endl;
						break;
					}
					case TRIANGULAR_RACE_FREEDOM: {
						if (trench::isAttackFeasible(program, true)) {
							std::cout << "Program IS NOT triangular data race-free." << std::endl;
						} else {
							std::cout << "Program IS triangular data race-free." << std::endl;
						}
						break;
					}
					case TRF_FENCES: {
						std::cout << "Number of computed fences for triangular race freedom: "
						          << trench::computeFences(program, true).size() << std::endl;
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
