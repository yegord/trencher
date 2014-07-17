/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <calin@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "ReachabilityChecking.h"

#include <boost/range/adaptor/map.hpp>
#include <boost/threadpool.hpp>

#include <vector>
#include <unordered_map>

#include "Foreach.h"
#include "Instruction.h"
#include "Program.h"
#include "Thread.h"
#include "Transition.h"
#include "RobustnessChecking.h"
#include "Benchmarking.h"

#include "DotPrinter.h"

namespace trench {

namespace {

class AttackChecker {
  Attack *attack_;

  public:

  AttackChecker(Attack *attack):
    attack_(attack) {}

  void operator()() {
    Program *instrumented = isAttackFeasible(attack_->program(), attack_);
    if (instrumented != NULL) {
      attack_->setFeasible(true);
      attack_->setInstrumented(instrumented);
    }
  }
};

class ProgramChecker {
  Program *program_;

  public:
  
  ProgramChecker(Program *program): program_(program) {}

  void operator()() {
    if (scReachable(*program_)) {
      program_->setReachable(true);
    }
  }
};

} // anonymous namespace

void tsoReachable(std::queue<Program*> queue, int &reachable) {
  while (!queue.empty()) {
    Program* program = queue.front(); queue.pop();
    std::vector<Attack> attacks;
    
    foreach (Thread *thread, program->threads()) {
      std::vector<Transition *> reads;
      std::vector<Transition *> writes;

      foreach (Transition *transition, thread->transitions()) {
        if (transition->instruction()->is<Read>()) {
          reads.push_back(transition);
        } else if (transition->instruction()->is<Write>()) {
          writes.push_back(transition);
        }
      }

      foreach (Transition *read, reads) {
        foreach (Transition *write, writes) {
          attacks.push_back(Attack(*program,thread,write,read));
        }
      }
    }

    boost::threadpool::pool pool(boost::thread::hardware_concurrency());
    foreach (Attack &attack, attacks) {
      pool.schedule(AttackChecker(&attack));
    }
    pool.wait();
    foreach (Attack &attack, attacks) {
      if (attack.feasible()) {
        pool.schedule(ProgramChecker(attack.instrumented()));
      }
    }
    pool.wait();
    foreach (Attack &attack, attacks) {
      if (attack.feasible() && (attack.instrumented()->reachable())) {
        reachable = 1; 
        // instrumented program which provides the witness ... 
        // trench::DotPrinter printer;
        // printer.print(std::cerr,*(attack.instrumented()));
        break;
      }
    }

    if (reachable != 1) {
      foreach (Attack attack, attacks) {
        if (attack.feasible()) {
          queue.push(attack.instrumented());
        }
      }
    } else {
      break; // a bug was found 
    }
  }
  if (reachable == -1) reachable = 0; // program robust
}

} // namespace trench
