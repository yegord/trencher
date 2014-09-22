/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "RobustnessChecking.h"

#include "Benchmarking.h"
#include "Program.h"
#include "Reduction.h"
#include "State.h"
#include "Transition.h"
#include "SCSemantics.h"
#include "Reachability.h"

#include <iostream>
#include "Instruction.h"
#include "Expression.h"
#include "ExpressionsCache.h"

namespace trench {

namespace {

bool isReachable(State *state, State *target, boost::unordered_set<State *> &visited) {
	if (state == target) {
		return true;
	}

	if (visited.find(state) != visited.end()) {
		return false;
	}

	visited.insert(state);

	for (Transition *transition : state->out()) {
		switch (transition->instruction()->mnemonic()) {
			case Instruction::READ:
			case Instruction::WRITE:
			case Instruction::LOCAL:
			case Instruction::CONDITION:
			case Instruction::NOOP:
				if (isReachable(transition->to(), target, visited)) {
					return true;
				}
				break;
			case Instruction::MFENCE:
			case Instruction::LOCK:
			case Instruction::UNLOCK:
				break;
			default: {
				assert(!"NEVER REACHED");
			}
		}
	}

	return false;
}

  State* awrite(Thread *thread, State *from, const std::string link, const std::string name, const int count, Write* write) {
    ExpressionsCache cache; 
    auto one = cache.makeConstant(1);
    auto adr_reg = cache.makeRegister(link + "_adr_" + std::to_string(count));
    auto val_reg = cache.makeRegister(link + "_val_" + std::to_string(count));

    // buffer the write using the newly added registers
    State *toadr = thread->makeState("adr_" + std::to_string(count) + "_" + link + name);
    State *toval = thread->makeState("val_" + std::to_string(count) + "_" + link + name);
    thread->makeTransition(from,toadr,std::make_shared<Local>(adr_reg,write->address()));
    thread->makeTransition(toadr,toval,std::make_shared<Local>(val_reg,write->value()));

    return toval;
  }

  void flush(Thread *thread, State *from, State *to, const std::string link, const std::string name, const int count) {
    ExpressionsCache cache;
    for (int i=1; i<=count; i++) {
      auto adr_reg = cache.makeRegister(link + "_adr_" + std::to_string(i));
      auto val_reg = cache.makeRegister(link + "_val_" + std::to_string(i));

      State *flush = NULL;
      if (i==count) flush = to; else flush = thread->makeState("flush_" + std::to_string(i) + "_" + link + "_" + name);
      thread->makeTransition(from, flush, std::make_shared<Write>(val_reg,adr_reg,Space()));
      from = flush;
    }
  }

  State* aread(Thread *thread, State *from, State *to, const std::string link, const std::string name, const int count, Read* read) {
    ExpressionsCache cache;
    for (int i=count; i>=1; i--) {
      auto areg = cache.makeRegister(link + "_adr_" + std::to_string(i));
      auto vreg = cache.makeRegister(link + "_val_" + std::to_string(i));
      auto eq = std::make_shared<Condition>(std::make_shared<BinaryOperator>(BinaryOperator::EQ,areg,read->address()));
      auto neq = std::make_shared<Condition>(std::make_shared<BinaryOperator>(BinaryOperator::NEQ,areg,read->address()));

      State *buf = thread->makeState("buf_" + std::to_string(i) + "_" + link + name);
      State *next = thread->makeState("next_" + std::to_string(i) + "_" + link + name);
      thread->makeTransition(from, buf, eq); // read address matches areg value
      thread->makeTransition(from, next, neq);
      thread->makeTransition(buf, to, std::make_shared<Local>(read->reg(),vreg));
      from = next;
    }
    thread->makeTransition(from, to, std::make_shared<Read>(read->reg(),read->address(),Space()));
    return to;
  }

} // anonymous namespace

bool isAttackFeasible(const Program &program, bool searchForTdrOnly, Thread *attacker, Transition *attackWrite, Transition *attackRead, 
  const boost::unordered_set<State *> &fenced) {

	Statistics::instance().incPotentialAttacksCount();

	if (attackWrite && attackRead) {
		boost::unordered_set<State *> visited(fenced);
		if (!isReachable(attackWrite->to(), attackRead->from(), visited)) {
			Statistics::instance().incInfeasibleAttacksCount1();
			return false;
		}
	}

	auto augmentedProgram = reduce(program, searchForTdrOnly, attacker, attackWrite, attackRead, fenced);

	bool feasible = dfsFinalStateReachable(SCSemantics(augmentedProgram));

	if (feasible) {
		Statistics::instance().incFeasibleAttacksCount();
	} else {
		Statistics::instance().incInfeasibleAttacksCount2();
	}

	return feasible;
}

Program* isAttackFeasible(const Program &program, const Attack *attack, const boost::unordered_set<State *> &fenced) {
  Statistics::instance().incPotentialAttacksCount();
  
  boost::unordered_set<State *> visited(fenced);
	if (!isReachable(attack->write()->to(), attack->read()->from(), visited)) {
	  Statistics::instance().incInfeasibleAttacksCount1();
		return NULL;
	}

	auto augmentedProgram = reduce(program, false, attack->attacker(), attack->write(), attack->read(), fenced);
	auto witness = isFinalStateReachable(SCSemantics(augmentedProgram));

  if (witness.first) {
		Statistics::instance().incFeasibleAttacksCount();
    // instrument program using the witness
    Program *result = new Program();
    for (auto thread: attack->program().threads()) {
      // copy the original program
      Thread *resultThread = result->makeThread(thread->name());
      if (thread->initialState()) resultThread->setInitialState(resultThread->makeState(thread->initialState()->name()));
      if (thread->finalState()) resultThread->setFinalState(resultThread->makeState(thread->finalState()->name()));
      for (auto transition: thread->transitions()) {
        State *from = resultThread->makeState(transition->from()->name());
        State *to = resultThread->makeState(transition->to()->name());
        resultThread->makeTransition(from, to, transition->instruction());
      }
      // instrument the attack witness
      if (thread == attack->attacker()) {
        State *from = resultThread->makeState(attack->write()->from()->name());
        std::stringstream ss; ss << attack->attacker(); std::string link = ss.str();
        //std::cout << "Instrumenting attacker for thread " << attack->attacker()->name() << " i.e. PTR_" << ss.str() << " ... " << std::endl;
        
        int count = 0; // track how many writes were buffered so far
        for (auto transition: witness.second) {
          if (transition.thread()->name() == thread->name()) {
            // destination control state of the transition in the augmentedProgram
            std::string name = transition.destination().controlStates().get(transition.thread())->name();
            if (name.substr(0,3) == "att") {
              auto instruction = transition.instruction();
              if (instruction->mnemonic() == Instruction::ATOMIC) {
                auto atomic = instruction->as<Atomic>();
                if (Write *write = (atomic->instructions().at(0))->as<Write>()) { // attacker write to buffer
                  count++;
                  from = awrite(resultThread, from, link, name.substr(3), count, write);
                  // nondeterministically flush and return to normal control flow // NEEDED ?
                  // State *to = resultThread->makeState(name.substr(4));
                  // flush(program, resultThread, from, to, link, name.substr(4), count);
                } else if (Write *write = (atomic->instructions().at(2))->as<Write>()) { // FIRST attacker write to buffer
                  count++;
                  from = awrite(resultThread, from, link, name.substr(3), count, write);
                  // nondeterministically flush and return to normal control flow: NOT NEEDED !
                  // State *to = resultThread->makeState(name.substr(4));
                  // flush(program, resultThread, from, to, link, name.substr(4), count, zero);
                } else if (Read *read = (atomic->instructions().at(3))->as<Read>()) { // attacker read
                  State *last = resultThread->makeState("last_" + link + name.substr(3)); // insert count?
                  from = aread(resultThread, from, last, link, name.substr(3), count, read);
                  // flush and return to normal program flow // NEEDED ?
                  // State *to = resultThread->makeState(name.substr(4));
                  // flush(program, resultThread, from, to, link, name.substr(4), count);
                }
              } else if (auto local = instruction->as<Local>()) {
                State *to = resultThread->makeState("lcn_" + std::to_string(count) + "_" + link + name.substr(3));
                resultThread->makeTransition(from,to,std::make_shared<Local>(local->reg(),local->value())); from = to;
              } else if (auto condition = instruction->as<Condition>()) {
                State *to = resultThread->makeState("lcn_" + std::to_string(count) + "_" + link + name.substr(3));
                resultThread->makeTransition(from,to,std::make_shared<Condition>(condition->expression())); from = to;
              } else if (auto noop = instruction->as<Noop>()) {
                State *to = resultThread->makeState("lcn_" + std::to_string(count) + "_" + link + name.substr(3));
                resultThread->makeTransition(from,to,std::make_shared<Noop>()); from = to; 
              } else { /* attacker doesn't execute fences nor locked instructios */
                assert(!"No fences or locked instructions.");
              }
            }
          }
        }
        // read instruction closes the hb cycle
        from = aread(resultThread, from, resultThread->makeState(link), link, "", count, attack->read()->instruction()->as<Read>());
        // flush the buffer
        State *to = resultThread->makeState(attack->read()->to()->name());
        flush(resultThread, from, to, link, attack->read()->to()->name(), count);
      }
    }
    return result;
	} else {
		Statistics::instance().incInfeasibleAttacksCount2();
	}

	return NULL; 
}

} // namespace trench
