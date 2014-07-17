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
#include <vector>
#include <map>

namespace trench {

class Program;
class Attack;
class State;
class Thread;
class Transition;

void instrument(Program &program, const Attack *attack, const std::vector<std::size_t> lines, const std::map<std::size_t,Transition*> &line2t);

} // namespace trench
