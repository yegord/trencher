/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <calin@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <trench/config.h>

#include <istream>
#include <map>

namespace trench {

class Transition;
class Program;
class Attack;

class TrailParser {
  public:

//  void parse(std::istream &in, Program &program); 
  void parse(std::istream &in, Program &program, Program *augmented,
    const Attack *attack, const std::map<std::size_t,Transition*> &line2t);
}; 

} // namespace trench
