#include "Reduction.h"

#include "Expression.h"
#include "Foreach.h"
#include "Instruction.h"
#include "Program.h"
#include "State.h"
#include "Transition.h"

namespace trench {

void reduce(const Program &program, Thread *attacker, Program &resultProgram) {
	enum {
		DEFAULT_SPACE = Space(),
		BUFFER_SPACE,
		IS_BUFFERED_SPACE
	};

	std::shared_ptr<Constant>  zero(new Constant(0));
	std::shared_ptr<Constant>  one(new Constant(1));

	std::shared_ptr<Register>  is_buffered(resultProgram.makeRegister("_is_buffered"));
	std::shared_ptr<Condition> check_is_buffered(new Condition(std::make_shared<BinaryOperator>(BinaryOperator::EQ, is_buffered, one)));
	std::shared_ptr<Condition> check_is_not_buffered(new Condition(std::make_shared<BinaryOperator>(BinaryOperator::EQ, is_buffered, zero)));

	std::shared_ptr<Register>  attacking = resultProgram.makeRegister("_attacking");
	std::shared_ptr<Condition> check_not_attacking(new Condition(std::make_shared<BinaryOperator>(BinaryOperator::EQ, attacking, zero)));
	std::shared_ptr<Local>     make_attacking(new Local(attacking, one));

//	std::shared_ptr<Constant>  writeVar(new std::shared_ptr<Constant>(0));

	resultProgram.setMemorySize(program.memorySize());

	foreach (Thread *thread, program.threads()) {
		Thread *resultThread = resultProgram.makeThread(thread->name());

		foreach (Transition *transition, thread->transitions()) {
			/*
			 * Original code.
			 */
			State *originalFrom = resultThread->makeState("orig_" + transition->from()->name());
			State *originalTo = resultThread->makeState("orig_" + transition->to()->name());

			resultThread->makeTransition(originalFrom, originalTo, transition->instruction());

			if (thread == attacker) {
				/*
				 * Attacker's execution.
				 */
				State *attackerFrom = resultThread->makeState("att_" + transition->from()->name());
				State *attackerTo   = resultThread->makeState("att_" + transition->to()->name());

				if (Write *write = transition->instruction()->as<Write>()) {
					/* Writes write to the buffer. */
					resultThread->makeTransition(
						attackerFrom,
						attackerTo,
						std::make_shared<Atomic>(
							std::make_shared<Write>(write->value(), write->address(), BUFFER_SPACE),
							std::make_shared<Write>(one,            write->address(), IS_BUFFERED_SPACE)
						)
					);
				} else if (Read *read = transition->instruction()->as<Read>()) {
					/* Reads either read from buffer... */
					resultThread->makeTransition(
						attackerFrom,
						attackerTo,
						std::make_shared<Atomic>(
							std::make_shared<Read>(is_buffered, read->address(), IS_BUFFERED_SPACE),
							check_is_buffered,
							std::make_shared<Read>(read->reg(), read->address(), BUFFER_SPACE)
						)
					);

					/* ...or from memory. */
					resultThread->makeTransition(
						attackerFrom,
						attackerTo,
						std::make_shared<Atomic>(
							std::make_shared<Read>(is_buffered, read->address(), IS_BUFFERED_SPACE),
							check_is_not_buffered,
							std::make_shared<Read>(read->reg(), read->address())
							// TODO: update HB variable
						)
					);
				} else if (transition->instruction()->as<Mfence>()) {
					/* No transition: attacker can't execute fences. */
				} else {
					resultThread->makeTransition(
						attackerFrom,
						attackerTo,
						transition->instruction()
					);
				}

				/*
				 * Becoming an attacker.
				 */
				if (Write *write = transition->instruction()->as<Write>()) {
					/* First write going into the buffer. */
					resultThread->makeTransition(
						originalFrom,
						attackerTo,
						std::make_shared<Atomic>(
							check_not_attacking,
							make_attacking,
							// TODO: remember writeVar
							std::make_shared<Write>(write->value(), write->address(), BUFFER_SPACE),
							std::make_shared<Write>(one,            write->address(), IS_BUFFERED_SPACE)
						)
					);
				}
			}
		}
	}
}

} // namespace trench
