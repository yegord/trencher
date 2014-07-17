/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <calin@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "ReachabilityChecking.h"

#include "Foreach.h"
#include "Program.h"
#include "State.h"
#include "Thread.h"
#include "Transition.h"
#include "SpinModelChecker.h"

namespace trench {

bool scReachable(const Program &program) {
  Program newProgram;
  int threadCount = (int) program.threads().size();
  newProgram.setMemorySize(std::max(program.memorySize(),threadCount));
  enum {
    DEFAULT_SPACE = Space(),
    SERVICE_SPACE
  };
  const std::shared_ptr<Constant> countAddr = newProgram.makeConstant(0);
  const std::shared_ptr<Constant> successAddr = newProgram.makeConstant(1);
  newProgram.setInteresting(successAddr->value(), SERVICE_SPACE);

  const std::shared_ptr<Constant> one = newProgram.makeConstant(1);
  const std::shared_ptr<Constant> max = newProgram.makeConstant(threadCount);
  const std::shared_ptr<Register> tmp = newProgram.makeRegister("_tmp");
  const std::shared_ptr<Condition> check_equals_max(new Condition(std::make_shared<BinaryOperator>(BinaryOperator::EQ, tmp, max)));
  
  /* copy the program */
  foreach (Thread *thread, program.threads()) {
    /* copy each thread */
    Thread *newThread = newProgram.makeThread(thread->name());
    /* use extra "final" state for each thread */
    newThread->makeTransition(
      newThread->makeState("new_final"),
      newThread->makeState("new_final"),
      std::make_shared<Atomic>(
        std::make_shared<Read>(tmp,countAddr,SERVICE_SPACE),
        check_equals_max,
        std::make_shared<Write>(one,successAddr,SERVICE_SPACE)
        )
      );
    if (thread->initialState()) {
      newThread->setInitialState(newThread->makeState("new_"+thread->initialState()->name()));
    }
    /* nondeterministically decide when final state has been reached in thread by increasing countAddr value */
    if (thread->finalState()) {
      newThread->makeTransition(
        newThread->makeState("new_"+thread->finalState()->name()),
        newThread->makeState("new_final"),
        std::make_shared<Atomic>(
          std::make_shared<Read>(tmp,countAddr,SERVICE_SPACE), // load into temporary register
          std::make_shared<Local>(tmp,std::make_shared<BinaryOperator>(BinaryOperator::ADD, tmp, one)), // add one
          std::make_shared<Write>(tmp,countAddr,SERVICE_SPACE) // write to countAddr
        )
      );
    }

    foreach (Transition *transition, thread->transitions()) {
      /* copy other transitions as they are */
      State *originalFrom = newThread->makeState("new_"+transition->from()->name());
      State *originalTo = newThread->makeState("new_"+transition->to()->name());

      newThread->makeTransition(originalFrom,originalTo,transition->instruction());
    }
  }

  /* reachability check */
  SpinModelChecker checker;
  bool reachable = (checker.check(newProgram,NULL) != NULL);
  
  return reachable;
}

} // namespace trench
