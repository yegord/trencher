/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <calin@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "ReachabilityChecking.h"

#include "Expression.h"
#include "ExpressionsCache.h"
#include "Instruction.h"
#include "Program.h"
#include "State.h"
#include "Thread.h"
#include "Transition.h"
#include "SCSemantics.h"
#include "Reachability.h"
#include "AutomatonPrinting.h"

namespace trench {

namespace {

Program encode(const Program &program) {
  ExpressionsCache cache;
  Program newProgram;
  int threadCount = (int) program.threads().size();
  enum {
    DEFAULT_SPACE = Space(),
    SERVICE_SPACE
  };
  auto countAddr = cache.makeConstant(0);
  auto successAddr = cache.makeConstant(1);
  newProgram.setInterestingAddress(successAddr->value(), SERVICE_SPACE);
  auto one = cache.makeConstant(1);
  auto max = cache.makeConstant(threadCount);
  auto tmp = cache.makeRegister("_tmp");
  auto check_equals_max = std::make_shared<Condition>(std::make_shared<BinaryOperator>(BinaryOperator::EQ, tmp, max));

  /* copy the program */
  for (Thread *thread : program.threads()) {
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
          std::make_shared<Write>(std::make_shared<BinaryOperator>(BinaryOperator::ADD, tmp, one),countAddr,SERVICE_SPACE) // m[countAddr]<-tmp+1
        )
      );
    }
    for (Transition *transition : thread->transitions()) {
      /* copy other transitions as they are */
      State *originalFrom = newThread->makeState("new_"+transition->from()->name());
      State *originalTo = newThread->makeState("new_"+transition->to()->name());

      newThread->makeTransition(originalFrom,originalTo,transition->instruction());
    }
  }
  return newProgram;
}

} // anonymous namespace

bool scReachable(const Program &program) {
  Program newProgram = encode(program); 
  bool reachable = dfsFinalStateReachable(SCSemantics(newProgram));
  return reachable;
}

} // namespace trench
