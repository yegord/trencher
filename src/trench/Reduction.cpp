#include "Reduction.h"

#include <cassert>

#include "Expression.h"
#include "Foreach.h"
#include "Instruction.h"
#include "Program.h"
#include "State.h"
#include "Transition.h"

namespace trench {

void reduce(const Program &program, Program &resultProgram, Thread *attacker, Read *attackRead, Write *attackWrite) {
	enum {
		DEFAULT_SPACE = Space(),
		BUFFER_SPACE,
		IS_BUFFERED_SPACE,
		HB_SPACE,
		SERVICE_SPACE
	};

	const std::shared_ptr<Constant>  zero(new Constant(0));
	const std::shared_ptr<Constant>  one(new Constant(1));

	const std::shared_ptr<Constant>  hb_nothing(new Constant(0));
	const std::shared_ptr<Constant>  hb_read(new Constant(1));
	const std::shared_ptr<Constant>  hb_write(new Constant(2));

	const std::shared_ptr<Register>  is_buffered(resultProgram.makeRegister("_is_buffered"));
	const std::shared_ptr<Condition> check_is_buffered(new Condition(std::make_shared<BinaryOperator>(BinaryOperator::EQ, is_buffered, one)));
	const std::shared_ptr<Condition> check_is_not_buffered(new Condition(std::make_shared<BinaryOperator>(BinaryOperator::EQ, is_buffered, zero)));

	const std::shared_ptr<Constant>  attackWriteAddr(new Constant(0));

	const std::shared_ptr<Noop>      noop(new Noop());

	foreach (Thread *thread, program.threads()) {
		Thread *resultThread = resultProgram.makeThread(thread->name());

		foreach (Transition *transition, thread->transitions()) {
			/*
			 * Original code.
			 */
			State *originalFrom = resultThread->makeState("orig_" + transition->from()->name());
			State *originalTo = resultThread->makeState("orig_" + transition->to()->name());

			resultThread->makeTransition(originalFrom, originalTo, transition->instruction());

			if (thread == attacker || attacker == NULL) {
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
					/* Reads either read from the buffer... */
					resultThread->makeTransition(
						attackerFrom,
						attackerTo,
						std::make_shared<Atomic>(
							std::make_shared<Read>(is_buffered, read->address(), IS_BUFFERED_SPACE),
							check_is_buffered,
							std::make_shared<Read>(read->reg(), read->address(), BUFFER_SPACE)
						)
					);

					/* ...or from memory... */
					resultThread->makeTransition(
						attackerFrom,
						attackerTo,
						std::make_shared<Atomic>(
							std::make_shared<Read>(is_buffered, read->address(), IS_BUFFERED_SPACE),
							check_is_not_buffered,
							std::make_shared<Read>(read->reg(), read->address())
						)
					);

					/* ...or from memory and update HB. */
					if (read == attackRead || attackRead == NULL) {
						resultThread->makeTransition(
							attackerFrom,
							attackerTo,
							std::make_shared<Atomic>(
								std::make_shared<Read> (is_buffered, read->address(), IS_BUFFERED_SPACE),
								check_is_not_buffered,
								std::make_shared<Read> (read->reg(), read->address()),
								std::make_shared<Write>(hb_read,     read->address(), HB_SPACE)
							)
						);
					}
				} else if (transition->instruction()->as<Mfence>()) {
					/* No transition: attacker can't execute fences. */
				} else if (transition->instruction()->as<Local>() ||
				           transition->instruction()->as<Condition>() ||
					   transition->instruction()->as<Noop>()) {
					resultThread->makeTransition(
						attackerFrom,
						attackerTo,
						transition->instruction()
					);
				} else if (transition->instruction()->as<Atomic>()) {
					assert(!"Sorry, atomics are not supported in input programs.");
				} else {
					assert(!"NEVER REACHED");
				}

				/*
				 * Becoming an attacker.
				 */
				if (Write *write = transition->instruction()->as<Write>()) {
					if (write == attackWrite || attackWrite == NULL) {
						/* First write going into the buffer. */
						resultThread->makeTransition(
							originalFrom,
							attackerTo,
							std::make_shared<Atomic>(
								std::make_shared<Write>(write->value(),   write->address(), BUFFER_SPACE),
								std::make_shared<Write>(one,              write->address(), IS_BUFFERED_SPACE),
								std::make_shared<Write>(write->address(), attackWriteAddr,  SERVICE_SPACE)
							)
						);
					}
				}
			}

			if (thread != attacker) {
				/*
				 * Helper's execution.
				 */
				// TODO

				/*
				 * Becoming a helper.
				 */
				// TODO
			}
		}

	}
}

} // namespace trench
