#include <fstream>
#include <iostream>

#include <trench/NaiveParser.h>
#include <trench/NaivePrinter.h>
#include <trench/SpinPrinter.h>
#include <trench/Program.h>
#include <trench/Reduction.h>

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

#if 0
			trench::NaivePrinter naivePrinter;
			naivePrinter.print(std::cout, program);
#endif

			trench::Program augmentedProgram;
			trench::reduce(program, program.threads().front(), augmentedProgram);

			trench::SpinPrinter spinPrinter;
			spinPrinter.print(std::cout, augmentedProgram);
		}
	} catch (const std::exception &exception) {
		std::cerr << "trencher: " << exception.what() << std::endl;
		return 1;
	}

	return 0;
}
