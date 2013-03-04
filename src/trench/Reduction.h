/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <trench/config.h>

#include <boost/unordered_set.hpp>

namespace trench {

class Program;
class State;
class Thread;
class Transition;

void reduce(const Program &program, Program &result, bool searchForTdrOnly,
            Thread *attacker = NULL, Transition *attackWrite = NULL, Transition *attackRead = NULL,
	    const boost::unordered_set<State *> &fenced = boost::unordered_set<State *>());

} // namespace trench
