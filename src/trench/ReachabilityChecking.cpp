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

#include "Expression.h"
#include "Foreach.h"
#include "Instruction.h"
#include "Program.h"
#include "State.h"
#include "Thread.h"
#include "Transition.h"
#include "RobustnessChecking.h"
#include "SpinModelChecker.h"
#include "Benchmarking.h"

namespace trench {

bool scReachable(const Program &program) {
  trench::Program augmentedProgram;
  int threadCount = (int) program.threads().size();
  augmentedProgram.setMemorySize(std::max(program.memorySize(),1+threadCount));
  enum {
    DEFAULT_SPACE = Space(),
    SERVICE_SPACE
  };
  const std::shared_ptr<Register> tmp(augmentedProgram.makeRegister("_tmp"));
  const std::shared_ptr<Constant> one = augmentedProgram.makeConstant(1);
  const std::shared_ptr<Constant> successVar = augmentedProgram.makeConstant(1+threadCount);
  augmentedProgram.setInterestingAddress(successVar->value(), SERVICE_SPACE);

  const std::shared_ptr<Condition> check_equals_one(new Condition(std::make_shared<BinaryOperator>(BinaryOperator::EQ, tmp, one)));
  const std::shared_ptr<Condition> check_not_blocked(new Condition(std::make_shared<NotBlocked>()));
  // copy the program
  foreach (Thread *thread, program.threads()) {
    // copy each thread
    Thread *augmentedThread = augmentedProgram.makeThread(thread->name());
    // make final absorbing state
    augmentedThread->setFinalState(augmentedThread->makeState("final"));
    augmentedThread->makeTransition(
      augmentedThread->finalState(),
      augmentedThread->finalState(),
      std::make_shared<Atomic>(
        check_not_blocked,
        std::make_shared<Read>(tmp,successVar,SERVICE_SPACE),
        check_equals_one,
        std::make_shared<Write>(one,successVar,SERVICE_SPACE)
        )
      );

    if (thread->initialState()) {
      augmentedThread->setInitialState(augmentedThread->makeState("o_"+thread->initialState()->name()));
    }
    // nondeterministically decide when final state has been reached in thread by decreasing successVar
    if (thread->finalState()) {
      augmentedThread->makeTransition(
        augmentedThread->makeState("o_"+thread->finalState()->name()),
        augmentedThread->finalState(),
        std::make_shared<Atomic>(
          check_not_blocked,
          std::make_shared<Read>(tmp,successVar,SERVICE_SPACE), // load into register
          std::make_shared<Local>(tmp,std::make_shared<BinaryOperator>(BinaryOperator::SUB, tmp, one)), // substract one
          std::make_shared<Write>(tmp,successVar,SERVICE_SPACE) // write to successVar
        )
      );
    }

    foreach (Transition *transition, thread->transitions()) {
      // copy each transition
      State *originalFrom = augmentedThread->makeState("o_"+transition->from()->name());
      State *originalTo = augmentedThread->makeState("o_"+transition->to()->name());

      augmentedThread->makeTransition(
        originalFrom,
        originalTo,
        std::make_shared<Atomic>(
          check_not_blocked,
          transition->instruction()
        )
      );
    }
  }

  trench::SpinModelChecker checker;
  bool reachable = checker.check(augmentedProgram);

  return reachable;
}

// first SC reachability (see above) must work ...
namespace {
        
class Attack {
  const Program &program_;
  Thread *attacker_;
  Transition *write_;
  Transition *read_;
            
  bool feasible_;
  std::vector<State *> intermediary_;
            
  public:
    Attack(const Program &program, Thread *attacker, Transition *write, Transition *read):
      program_(program), attacker_(attacker), write_(write), read_(read), feasible_(false)
        {}
    
    const Program &program() const { return program_; }
    Thread *attacker() const { return attacker_; }
    Transition *write() const { return write_; }
    Transition *read() const { return read_; }
            
    bool feasible() const { return feasible_; }
    void setFeasible(bool value) { feasible_ = value; }
            
    const std::vector<State *> &intermediary() const { return intermediary_; }
            
    template<class T>
    void setIntermediary(const T &container) {
      intermediary_.clear();
      intermediary_.insert(intermediary_.end(), container.begin(), container.end());
    }
};
        
class AttackChecker {
  Attack &attack_;
  
  public:
    AttackChecker(Attack &attack):
      attack_(attack)
        {}
    void operator()() {
      if (isAttackFeasible(attack_.program(), false, attack_.attacker(), attack_.write(), attack_.read())) {
        attack_.setFeasible(true);
        
        boost::unordered_set<State *> visited;
        boost::unordered_set<State *> intermediary;
                    
        dfs(attack_.write()->to(), attack_.read()->from(), visited, intermediary);
        attack_.setIntermediary(intermediary);
      }
    }
        
  private:
    static void dfs(State *state, State *target, boost::unordered_set<State *> &visited, boost::unordered_set<State *> &intermediary) {
      if (state == target) {
        intermediary.insert(state);
          return;
      }
      
      if (visited.find(state) != visited.end()) {
        return;
      }
                
      visited.insert(state);
                
      foreach (Transition *transition, state->out()) {
        switch (transition->instruction()->mnemonic()) {
          case Instruction::READ:
          case Instruction::WRITE:
          case Instruction::LOCAL:
          case Instruction::CONDITION:
          case Instruction::NOOP:
            dfs(transition->to(), target, visited, intermediary);
                            
            if (intermediary.find(transition->to()) != intermediary.end()) {
              intermediary.insert(state);
            }
            break;
          case Instruction::MFENCE:
            break;
          default: {
            assert(!"NEVER REACHED");
          }
        }
      }
    }
};
        
class Attacker {
  std::vector<const Attack *> attacks_;
  std::vector<State *> fences_;

  public:

    const std::vector<const Attack *> &attacks() const { return attacks_; }
    void addAttack(const Attack *attack) { assert(attack->feasible()); attacks_.push_back(attack); }
            
    const std::vector<State *> &fences() const { return fences_; }
            
    template<class T>
    void setFences(const T &container) {
      fences_.clear();
      fences_.insert(fences_.end(), container.begin(), container.end());
    }
};

void reduce(const Program &program, Program &resultProgram, std::unordered_map<Thread *,Attacker> attacks) {
  resultProgram.setMemorySize(std::max(program.memorySize(), 2));
  
  enum {
    DEFAULT_SPACE = Space(),
    ATTACK_SPACE
  };
  const std::shared_ptr<Condition> check_not_blocked(
    new Condition(std::make_shared<NotBlocked>()));

  foreach (Thread *thread, program.threads()) {
    Thread *resultThread = resultProgram.makeThread(thread->name());

    if (thread->initialState()) {
      resultThread->setInitialState(resultThread->makeState("o_"+thread->initialState()->name()));
    }

    foreach (Transition *transition, thread->transitions()) {
      /*
       * Original code.
       */
      State *originalFrom = resultThread->makeState("o_"+transition->from()->name());
      State *originalTo = resultThread->makeState("o_"+transition->to()->name());

      if (transition->instruction()->is<Read>() || transition->instruction()->is<Write>()) {
        resultThread->makeTransition(
          originalFrom,
          originalTo,
          std::make_shared<Atomic>(
            check_not_blocked,
            transition->instruction()
          )
        );
      } else {
        resultThread->makeTransition(originalFrom, originalTo, transition->instruction());
      }
    }
    // encode the Attacker delayed writes ...
    //foreach (const auto &item, attacks) {
    //  foreach (Attacker attacker, item.second) {
        // ... encode each attacker's feasible attacks
        // fence the writes of each attack
        // branch into attack copy

    //  }
   // }

  }
}

} // anonymous namespace

int tsoReachable(const Program &program) {
  std::vector<Attack> attacks;
  std::unordered_map<Thread*,Attacker> feasibleAttacks;
  trench::Program augmentedProgram;
  reduce(program,augmentedProgram,feasibleAttacks);
  int reachable = -1;

  int iter = 0;
  while (iter <= 3) {
    // build all attacks for the "new" instrumented program
    foreach (Thread *thread, program.threads()) {
      std::vector<Transition *> reads;
      std::vector<Transition *> writes;

      foreach (Transition *transition, thread->transitions()) {
        if (transition->instruction()->is<Read>()) {
          reads.push_back(transition);
        } else if (transition->instruction()->is<Write>()) {
          writes.push_back(transition);
        }
      }

      attacks.clear();
    
      foreach (Transition *read, reads) {
        foreach (Transition *write, writes) {
          attacks.push_back(Attack(program,thread,write,read));
        }
      }
    }
  
    // out of those, find all feasible attacks ...
    boost::threadpool::pool pool(boost::thread::hardware_concurrency());
    foreach (Attack &attack, attacks) {
      pool.schedule(AttackChecker(attack));
    }
    pool.wait();

    feasibleAttacks.clear();

    foreach (Attack &attack, attacks) {
      if (attack.feasible()) {
        feasibleAttacks[attack.attacker()].addAttack(&attack);
      }
    }

    // instrument the feasible attacks in the given program ... 
    if (feasibleAttacks.empty()) {
      // reachable = // check if final state is SC reachable in instrumented program ...
    } else {
      reduce(augmentedProgram,augmentedProgram,feasibleAttacks);
      // if final state is SC reachable in instrumented program ...
      // reachable = 1;
    }
    
    if (reachable != -1) {
      break;
    }

    iter++; 
    std::cout << "loop" << iter << std::endl;
  }
  return reachable;
}

} // namespace trench
