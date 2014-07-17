/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "SpinModelChecker.h"

#include <cstdlib> /* system */
#include <cstring> /* strerror */
#include <fstream>

#include <boost/chrono.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "Benchmarking.h"
#include "SpinPrinter.h"
#include "DotPrinter.h"
#include "TrailParser.h"

namespace trench {

namespace {

boost::filesystem::path makeTempDir() {
	boost::filesystem::path path = boost::filesystem::temp_directory_path() / "trencher" / boost::filesystem::unique_path();
	if (!boost::filesystem::create_directories(path)) {
		throw std::runtime_error("could not create temporary directory: " + path.string());
	}
  //std::cout << "Temp DIR path ... " << path.string() << std::endl;
	return path.string();
}

long run(const std::string &commandLine, bool catch0) {
	auto startTime = boost::chrono::system_clock::now();
	int status = system(commandLine.c_str());
	auto endTime = boost::chrono::system_clock::now();

	if (status == -1) {
		throw std::runtime_error((boost::format("could not run '%s': %s") % commandLine % strerror(errno)).str());
	} else if ((status != 0) && catch0) {
	  // std::cout << commandLine << std::endl;	
    throw std::runtime_error((boost::format("'%s' finished with non-zero exit code: %d") % commandLine % status).str());
	}

	return boost::chrono::duration_cast<boost::chrono::milliseconds>(endTime - startTime).count();
}

} // anonymous namespace

SpinModelChecker::SpinModelChecker() {
	std::string spin = "spin";
	if (boost::filesystem::exists("spin.exe")) {
		spin = boost::filesystem::absolute("spin.exe").string();
	}
	setSpinCommandLine("cd \"%1%\" && \"" + spin + "\" -a \"%2%\"");
	setCompilerCommandLine("cc -DSAFETY -DVECTORSZ=4444 -DBITSTATE -o \"%2%\" \"%3%\" 2> \"%3%.cc.stderr\"");
	setVerifierCommandLine("cd \"%1%\" && \"%2%\" > \"%2%.stdout\" 2> \"%2%.stderr\"");
  setTrailCommandLine("cd \"%1%\" && \"" + spin + "\" -t -p \"%2%\" > \"%2%.other\" 2> \"%2%.trail.stderr\"");
}

Program* SpinModelChecker::check(Program &instrumented, const Attack *attack) {
	boost::filesystem::path temp_dir = makeTempDir();
	boost::filesystem::path program_pml = temp_dir/"program.pml";
	boost::filesystem::path verifier_c = temp_dir/"pan.c";
	boost::filesystem::path verifier = temp_dir/"pan";
	boost::filesystem::path trail = temp_dir/"program.pml.trail";
  boost::filesystem::path other = temp_dir/"program.pml.other";
//  boost::filesystem::path dot = temp_dir/"program.dot";
//  boost::filesystem::path png = temp_dir/"program.png";

  std::map<std::size_t,Transition*> line2t;
	std::ofstream out(program_pml.string().c_str());
	SpinPrinter printer; printer.print(instrumented, out, line2t);

	if (!out) {
		throw std::runtime_error("could not create file: " + program_pml.string());
	}
	out.close();

	Statistics::instance().addSpinTime    (run((boost::format(spinCommandLine())     % temp_dir % program_pml).str(),true));
	Statistics::instance().addCompilerTime(run((boost::format(compilerCommandLine()) % temp_dir % verifier % verifier_c).str(),true));
	Statistics::instance().addVerifierTime(run((boost::format(verifierCommandLine()) % temp_dir % verifier).str(),true));

  Program* result = NULL;
	if (boost::filesystem::exists(trail)) {
//    std::cout << "Temp DIR path ... " << temp_dir.string() << std::endl; 
//    << " attacker ... " << attack->attacker()->name()
//    << " store from ... " << attack->write()->from()->name() 
//    << " load to ... " << attack->read()->to()->name() << std::endl;
 
    // "spin -t -p program.pml" returns 1 for some strange/unknown reason !!! ...
    Statistics::instance().addTrailTime(run((boost::format(trailCommandLine()) %temp_dir %program_pml).str(),false));
    if (attack != NULL) { // use new AttackChecker
      // parse trail and instrument it in result // return the trail suffix instead of instrumenting it here ?
      result = new Program(attack->program().memorySize());
      
      TrailParser parser; std::ifstream in(other.string().c_str());
      if (!in) {
        throw std::runtime_error("can't open file: " + other.string());
      }
      // std::cout << "Temp dir ... " << temp_dir.string() << std::endl;
      parser.parse(in, *result, &instrumented, attack, line2t);
//      std::ofstream dotfile(dot.string().c_str());
//      trench::DotPrinter another; another.print(dotfile,*result);
//      run((boost::format("cd \"%1%\" && \"dot\" -Tpng \"program.dot\" > \"program.png\"") %temp_dir %home).str(),false);
  	  //boost::filesystem::remove_all(temp_dir);
    } else { // use old AttackChecker // used by non-reachability at the moment
      result = new Program(-1);
    }
  }
  
  boost::filesystem::remove_all(temp_dir);
  return result;
}

} // namespace trench
