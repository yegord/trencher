#include <fstream>
#include <iostream>

#include <trench/Program.h>
#include <trench/NaiveParser.h>

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

			std::cout << "Number of threads parsed: " << program.threads().size() << std::endl;
			// Do something else.
		}
	} catch (const std::exception &exception) {
		std::cerr << "trencher: " << exception.what() << std::endl;
		return 1;
	}

	return 0;
}
