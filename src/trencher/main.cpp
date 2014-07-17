/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include <queue>
#include <ctime>
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <boost/chrono.hpp>

#include <trench/Benchmarking.h>
#include <trench/DotPrinter.h>
//#include <trench/CPrinter.h>
//#include <trench/MPrinter.h>
#include <trench/NaiveParser.h>
#include <trench/Foreach.h>
#include <trench/State.h>
#include <trench/Program.h>
#include <trench/Reduction.h>
#include <trench/FenceInsertion.h>
#include <trench/RobustnessChecking.h>
#include <trench/ReachabilityChecking.h>

void help() {
	std::cout << "Usage: trencher [-b|-nb] [-r|-f|-trf|-ftrf|-dot|-rdot] file..." << std::endl
	<< std::endl
	<< "Options:" << std::endl
	<< "  -b     Switch benchmarking mode on (print only execution statistics)." << std::endl
	<< "  -nb    Switch benchmarking mode off." << std::endl
	<< "  -r     Check robustness." << std::endl
	<< "  -f     Do fence insertion for enforcing robustness." << std::endl
	<< "  -dot   Print the example in dot format." << std::endl
//  << "  -c     Print the example as a C program." << std::endl 
//  << "  -m     Print the example as a Memorax program." << std::endl 
	<< "  -rdot  Print the example instrumented for robustness checking in dot format." << std::endl
  << "  -rsc   Check reachability under SC." << std::endl
  << "  -rtso  Check reachability under TSO using robustness." << std::endl;
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
      REACHABILITY_SC,
      REACHABILITY_TSO,
			PRINT_DOT,
//      PRINT_C,
//      PRINT_M,
      PRINT_ROBUSTNESS_DOT
		} action = FENCES;

		bool benchmarking = false;

		for (int i = 1; i < argc; ++i) {
			std::string arg = argv[i];
      
      if (arg == "-rsc") {
        action = REACHABILITY_SC;
			} else if (arg == "-rtso") {
        action = REACHABILITY_TSO;
      } else if (arg == "-r") {
				action = ROBUSTNESS;
			} else if (arg == "-f") {
				action = FENCES;
			} else if (arg == "-b") {
				benchmarking = true;
			} else if (arg == "-dot") {
				action = PRINT_DOT;
//      } else if (arg == "-c") {
//        action = PRINT_C;
//      } else if (arg == "-m") {
//        action = PRINT_M;
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
					foreach (trench::Thread *thread, program.threads()) {
						trench::Statistics::instance().incStatesCount(thread->states().size());
						trench::Statistics::instance().incTransitionsCount(thread->transitions().size());
					}
				}

				auto startTime = boost::chrono::system_clock::now();
				clock_t startClock = clock();

				switch (action) {
          case REACHABILITY_SC: {
            bool reachable = trench::scReachable(program);
            if (!benchmarking) {
              if (reachable) {
                std::cout << "Final state IS reachable under SC." << std::endl;
              } else {
                std::cout << "Final state IS NOT reachable under SC." << std::endl;
              }
            }
            break;
          }
          case REACHABILITY_TSO: {
            if (trench::scReachable(program)) {
              if (!benchmarking) {
                std::cout << "Final state IS reachable under SC, therefore under TSO too." << std::endl;
              }
              break;
            } else {
              std::queue<trench::Program*> queue; queue.push(&program);
              int reachable = -1; trench::tsoReachable(queue,reachable);
              if (!benchmarking) {
                if (reachable == 1) {
                  std::cout << "Final state IS reachable under TSO." << std::endl;
                } else if (reachable == 0) {
                  std::cout << "Final state IS NOT reachable under TSO." << std::endl;
                } else {
                  std::cout << "Never reached ... semidecision procedure!" << std::endl;
                }
              }
              break;
            }
          }
					case ROBUSTNESS: {
						bool feasible = trench::isAttackFeasible(program);
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
						auto fences = trench::computeFences(program);
						if (!benchmarking) {
							std::cout << "Computed fences for enforcing robustness (" << fences.size() << " total):";
							foreach (const auto &fence, fences) {
								std::cout << " (" << fence.first->name() << "," << fence.second->name() << ')';
							}
							std::cout << std::endl;
						}
						break;
					}
					case PRINT_DOT: {
						trench::DotPrinter printer;
						printer.print(std::cout, program);
						break;
					}
/*          case PRINT_C: {
            trench::CPrinter printer;
            printer.print(std::cout, program);
            break;
          }
          case PRINT_M: {
            trench::MPrinter printer;
            printer.print(std::cout, program);
            break;
          }
*/					case PRINT_ROBUSTNESS_DOT: {
						trench::Program instrumentedProgram;
						trench::reduce(program, instrumentedProgram);

						trench::DotPrinter printer;
						printer.print(std::cout, instrumentedProgram);
						break;
					}
					default: {
						assert(!"NEVER REACHED");
					}
				}

				clock_t endClock = clock();
				auto endTime = boost::chrono::system_clock::now();

				trench::Statistics::instance().addCpuTime((endClock - startClock) * 1000 / CLOCKS_PER_SEC);
				trench::Statistics::instance().addRealTime(
					boost::chrono::duration_cast<boost::chrono::milliseconds>(endTime - startTime).count());

				if (benchmarking) {
					std::cout << "filename " << arg << // " action " << action <<
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
