/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <calin@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "TrailParser.h"

#include <sstream>
#include <stdexcept>
#include <iostream>
#include <map>

#include "Program.h"
#include "Instrumentation.h"

namespace trench {

// parse a human readable SPIN trail according to @attack and @line2t and instrument it in @program
void TrailParser::parse(std::istream &in, Program &program, Program *augmented, 
  const Attack *attack, const std::map<std::size_t,Transition*> &line2t) {
      
  // trail lines that make the counterexample
	std::vector<std::size_t> lines;
  
  in >> std::skipws; 
  std::string token;
  std::stringstream ss; 
  ss << augmented->getThread(attack->attacker()->name());
	
  while (in >> token) {
    if (token == "spin:") {
      if (!(in >> token)) {
        throw std::runtime_error("expected `trail' or `warning', not EOF");
      }
      if (token == "trail") {
        break; // trail ends ...
      }
    } else if (token.back() == ':') {
      if (!(in >> token)) {
        throw std::runtime_error("expected `proc', not EOF");
      } else if (!(in >> token)) {
        throw std::runtime_error("expected `proc' identifier, not EOF");
      } else if (!(in >> token)) {
        throw std::runtime_error("expected thread pointer or `terminates', not EOF");
      }
      // check if the token contains the pointer to the attacker thread
      if ((token != "terminates") && (token.front() == '(') && (token.back() == ')')) {
        token.pop_back(); token.replace(0,5,""); // discard `(PTR_' and `)'
        // std::cout << attack->attacker()->name() << "=" << token << " ? ";
        std::size_t pos = token.find(":");
        if (pos != std::string::npos) token = token.replace(token.begin()+pos,token.end(),"");
        if (ss.str() == token) { // this simple check is not sufficient on MacOS
//        if (ss.str() == token.replace(token.begin()+token.find(":"),token.end(),"")) {
          if (!(in >> token)) {
            throw std::runtime_error("expected path:line identifier, not EOF");
          }
          // split token over `:' and take the latter path indicating the line
          std::size_t line = std::stoi(token.substr(token.find(":")+1)); //atoi(token.substr(token.find(":")+1).c_str());
          if ((lines.empty() || (lines.back() != line)) && (line != 0)) {
            lines.push_back(line);
          }
        }
      }
    }
    // skip the rest of the line
    in >> std::noskipws;
    char c;
    while (in >> c) {
      if (c == '\n' || c == '\r') {
        break;
      }
    }
    in >> std::skipws;
  }

  /* for checking the trail parsing ... 
  std::cout << "Attacker " << attack->attacker()->name() << " i.e. " << ss.str() << " : ";
  foreach (int i, lines) {
    std::cout << i << " ";
  }
  std::cout << std::endl;
  */
  // copy attack->program() and instrument the `lines'-indicated transitions from line2t
  instrument(program, attack, lines, line2t);
}

} // namespace trench
