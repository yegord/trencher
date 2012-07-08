#include "SpinModelChecker.h"

#include <chrono>
#include <cstdlib> /* system */
#include <cstring> /* strerror */
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "Benchmarking.h"
#include "SpinPrinter.h"

namespace trench {

namespace {

boost::filesystem::path makeTempDir() {
	boost::filesystem::path path = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	if (!boost::filesystem::create_directory(path)) {
		throw std::runtime_error("could not create temporary directory: " + path.string());
	}
	return path.string();
}

long run(const std::string &commandLine) {
	using std::chrono::system_clock;

	auto startTime = std::chrono::system_clock::now();
	int status = system(commandLine.c_str());
	auto endTime = std::chrono::system_clock::now();

	if (status == -1) {
		throw std::runtime_error((boost::format("could not run '%s': %s") % commandLine % strerror(errno)).str());
	} else if (status != 0) {
		throw std::runtime_error((boost::format("'%s' finished with non-zero exit code: %d") % commandLine % status).str());
	}

	return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
}

} // anonymous namespace

SpinModelChecker::SpinModelChecker() {
	setSpinCommandLine("cd \"%1%\" && spin -a \"%2%\"");
	setCompilerCommandLine("clang -DSAFETY -DVECTORSZ=4444 -DBITSTATE -o \"%2%\" \"%3%\"");
	setVerifierCommandLine("cd \"%1%\" && \"%2%\" > \"%2%.stdout\" 2> \"%2%.stderr\"");
}

bool SpinModelChecker::check(const Program &program) {
	boost::filesystem::path temp_dir = makeTempDir();
	boost::filesystem::path program_pml = temp_dir/"program.pml";
	boost::filesystem::path verifier_c = temp_dir/"pan.c";
	boost::filesystem::path verifier = temp_dir/"pan";
	boost::filesystem::path trail = temp_dir/"program.pml.trail";

	std::ofstream out(program_pml.c_str());

	SpinPrinter printer;
	printer.print(out, program);
	if (!out) {
		throw std::runtime_error("could not create file: " + program_pml.string());
	}
	out.close();

	Statistics::instance().addSpinTime    (run((boost::format(spinCommandLine())     % temp_dir % program_pml).str()));
	Statistics::instance().addCompilerTime(run((boost::format(compilerCommandLine()) % temp_dir % verifier % verifier_c).str()));
	Statistics::instance().addVerifierTime(run((boost::format(verifierCommandLine()) % temp_dir % verifier).str()));

	bool result = boost::filesystem::exists(trail);

	boost::filesystem::remove_all(temp_dir);

	return result;
}

} // namespace trench
