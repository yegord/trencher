#pragma once

#include "config.h"

#include <boost/unordered_set.hpp>

namespace trench {

class Program;
class State;
class Thread;
class Transition;

void reduce(const Program &program, Program &result,
            Thread *attacker = NULL, Transition *attackWrite = NULL, Transition *attackRead = NULL,
	    const boost::unordered_set<State *> &fenced = boost::unordered_set<State *>());

} // namespace trench
