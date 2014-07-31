/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <trench/AutomatonPrinting.h>
#include <trench/Benchmarking.h>
#include <trench/FenceInsertion.h>
#include <trench/NaiveParser.h>
#include <trench/Program.h>
#include <trench/ProgramPrinting.h>
#include <trench/Reduction.h>
#include <trench/RobustnessChecking.h>
#include <trench/SCSemantics.h>
#include <trench/State.h>

void help() {
	std::cout << "Usage: trencher [-b|-nb] [-r|-f|-trf|-ftrf|-dot|-rdot] file..." << std::endl
	<< std::endl
	<< "Options:" << std::endl
	<< "  -b     Switch benchmarking mode on (print only execution statistics)." << std::endl
	<< "  -nb    Switch benchmarking mode off." << std::endl
	<< "  -r     Check robustness." << std::endl
	<< "  -f     Do fence insertion for enforcing robustness." << std::endl
	<< "  -trf   Check triangular data race freedom." << std::endl
	<< "  -ftrf  Do fence insertion for enforcing triangular data race freedom." << std::endl
	<< "  -dot   Print the example in dot format." << std::endl
	<< "  -rdot  Print the example instrumented for robustness checking in dot format." << std::endl
	<< "  -scdot Print the SC semantics automaton for the program." << std::endl;
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
			TRF_FENCES,
			PRINT_DOT,
			PRINT_ROBUSTNESS_DOT,
			PRINT_SC_DOT,
		} action = FENCES;

		bool benchmarking = false;

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
			} else if (arg == "-b") {
				benchmarking = true;
			} else if (arg == "-nb") {
				benchmarking = false;
			} else if (arg == "-dot") {
				action = PRINT_DOT;
			} else if (arg == "-rdot") {
				action = PRINT_ROBUSTNESS_DOT;
			} else if (arg == "-scdot") {
				action = PRINT_SC_DOT;
			} else if (arg.size() >= 1 && arg[0] == '-') {
				throw std::runtime_error("unknown option: " + arg);
			} else {
				trench::Program program;

				trench::Statistics::instance().reset();
				{
					trench::NaiveParser parser;
					std::ifstream in(argv[i]);
					if (!in) {
						throw std::runtime_error("can't open file: " + arg);
					}
					parser.parse(in, program);

					trench::Statistics::instance().incThreadsCount(program.threads().size());
					for (trench::Thread *thread : program.threads()) {
						trench::Statistics::instance().incStatesCount(thread->states().size());
						trench::Statistics::instance().incTransitionsCount(thread->transitions().size());
					}
				}

				auto startTime = std::chrono::steady_clock::now();
				clock_t startClock = clock();

				switch (action) {
					case ROBUSTNESS: {
						bool feasible = trench::isAttackFeasible(program, false);
						if (!benchmarking) {
							if (feasible) {
								std::cout << "Program IS NOT robust." << std::endl;
							} else {
								std::cout << "Program IS robust." << std::endl;
							}
						}
						break;
					}
					case FENCES: {
						auto fences = trench::computeFences(program, false);
						if (!benchmarking) {
							std::cout << "Computed fences for enforcing robustness (" << fences.size() << " total):";
							for (const auto &fence : fences) {
								std::cout << " (" << fence.first->name() << "," << fence.second->name() << ')';
							}
							std::cout << std::endl;
						}
						break;
					}
					case TRIANGULAR_RACE_FREEDOM: {
						bool feasible = trench::isAttackFeasible(program, true);
						if (!benchmarking) {
							if (feasible) {
								std::cout << "Program IS NOT free from triangular data races." << std::endl;
							} else {
								std::cout << "Program IS free from triangular data races." << std::endl;
							}
						}
						break;
					}
					case TRF_FENCES: {
						auto fences = trench::computeFences(program, true);
						if (!benchmarking) {
							std::cout << "Computed fences for enforcing triangular race freedom (" << fences.size() << " total):";
							for (const auto &fence : fences) {
								std::cout << " (" << fence.first->name() << "," << fence.second->name() << ')';
							}
							std::cout << std::endl;
						}
						break;
					}
					case PRINT_SC_DOT: {
						trench::printAsDot(trench::SCSemantics(program), std::cout);
						break;
					}
					case PRINT_DOT: {
						trench::printAsDot(std::cout, program);
						break;
					}
					case PRINT_ROBUSTNESS_DOT: {
						trench::printAsDot(std::cout, trench::reduce(program, false));
						break;
					}
					default: {
						assert(!"NEVER REACHED");
					}
				}

				clock_t endClock = clock();
				auto endTime = std::chrono::steady_clock::now();

				trench::Statistics::instance().addCpuTime((endClock - startClock) * 1000 / CLOCKS_PER_SEC);
				trench::Statistics::instance().addRealTime(
					std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count());

				if (benchmarking) {
					std::cout << "filename " << arg << " action " << action <<
						trench::Statistics::instance() << std::endl;
				}
			}
		}
	} catch (const std::exception &exception) {
		std::cerr << "trencher: " << exception.what() << std::endl;
		return 1;
	}

	return 0;
}
