#include "Reduction.h"

#include <algorithm>
#include <cassert>

#include "Expression.h"
#include "Foreach.h"
#include "Instruction.h"
#include "Program.h"
#include "State.h"
#include "Transition.h"

namespace trench {

void reduce(const Program &program, Program &resultProgram, Thread *attacker, Read *attackRead, Write *attackWrite) {
	resultProgram.setMemorySize(std::max(program.memorySize(), 2));

	enum {
		DEFAULT_SPACE = Space(),
		BUFFER_SPACE,
		IS_BUFFERED_SPACE,
		HB_SPACE,
		SERVICE_SPACE
	};

	const std::shared_ptr<Constant>  zero(new Constant(0));
	const std::shared_ptr<Constant>  one(new Constant(1));

	const std::shared_ptr<Register>  is_buffered(resultProgram.makeRegister("_is_buffered"));

	const std::shared_ptr<Condition> check_is_buffered(
		new Condition(std::make_shared<BinaryOperator>(BinaryOperator::EQ, is_buffered, one)));
	const std::shared_ptr<Condition> check_is_not_buffered(
		new Condition(std::make_shared<BinaryOperator>(BinaryOperator::EQ, is_buffered, zero)));

	const std::shared_ptr<Constant>  attackAddr(new Constant(0));
	const std::shared_ptr<Constant>  successAddr(new Constant(1));

	resultProgram.setInterestingAddress(successAddr->value(), SERVICE_SPACE);

	const std::shared_ptr<Register>  addr(resultProgram.makeRegister("_addr"));

	const std::shared_ptr<Constant>  hb_nothing(new Constant(0));
	const std::shared_ptr<Constant>  hb_read(new Constant(1));
	const std::shared_ptr<Constant>  hb_write(new Constant(2));

	const std::shared_ptr<Register>  access_type(resultProgram.makeRegister("_access_type"));

	const std::shared_ptr<Condition> check_access_type_is_write(
		new Condition(std::make_shared<BinaryOperator>(BinaryOperator::EQ, access_type, hb_write)));
	const std::shared_ptr<Condition> check_access_type_is_not_write(
		new Condition(std::make_shared<BinaryOperator>(BinaryOperator::NEQ, access_type, hb_write)));
	const std::shared_ptr<Condition> check_access_type_is_read_or_write(
		new Condition(std::make_shared<BinaryOperator>(BinaryOperator::NEQ, access_type, hb_nothing)));

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
				State *attackerFrom = resultThread->makeState("att_" + transition->from()->name());
				State *attackerTo   = resultThread->makeState("att_" + transition->to()->name());

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
								std::make_shared<Write>(write->address(), attackAddr,  SERVICE_SPACE)
							)
						);
					}
				}

				/*
				 * Attacker's execution.
				 */
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

					/* ...or from memory and update HB. This is the final aim of attacker's existence. */
					if (read == attackRead || attackRead == NULL) {
						resultThread->makeTransition(
							attackerFrom,
							resultThread->makeState("final"),
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
					assert(!"Sorry, atomics in input programs are not supported.");
				} else {
					assert(!"NEVER REACHED");
				}
			}

			if (thread != attacker) {
				State *helperFrom = resultThread->makeState("hlp_" + transition->from()->name());
				State *helperTo   = resultThread->makeState("hlp_" + transition->to()->name());

				/*
				 * Becoming a helper.
				 */
				/* One can become a helper only on a read or a write. */
				if (Read *read = transition->instruction()->as<Read>()) {
					resultThread->makeTransition(
						originalFrom,
						helperTo,
						std::make_shared<Atomic>(
							std::make_shared<Read>(access_type, read->address(), HB_SPACE),
							check_access_type_is_write,
							transition->instruction()
						)
					);
				} else if (Write *write = transition->instruction()->as<Write>()) {
					resultThread->makeTransition(
						originalFrom,
						helperTo,
						std::make_shared<Atomic>(
							std::make_shared<Read>(access_type, write->address(), HB_SPACE),
							check_access_type_is_read_or_write,
							transition->instruction()
						)
					);
				}

				/*
				 * Helper's execution.
				 */
				if (Read *read = transition->instruction()->as<Read>()) {
					resultThread->makeTransition(
						helperFrom,
						helperTo,
						std::make_shared<Atomic>(
							std::make_shared<Read>(access_type, read->address(), HB_SPACE),
							check_access_type_is_write,
							transition->instruction()
						)
					);
					resultThread->makeTransition(
						helperFrom,
						helperTo,
						std::make_shared<Atomic>(
							std::make_shared<Read>(access_type, read->address(), HB_SPACE),
							check_access_type_is_not_write,
							transition->instruction(),
							std::make_shared<Write>(hb_read, read->address(), HB_SPACE)
						)
					);
					resultThread->makeTransition(
						helperFrom,
						helperTo,
						std::make_shared<Atomic>(
							std::make_shared<Read>(addr, attackAddr, SERVICE_SPACE),
							std::make_shared<Condition>(std::make_shared<BinaryOperator>(BinaryOperator::EQ, addr, read->address())),
							std::make_shared<Write>(one, successAddr, SERVICE_SPACE)
						)
					);
				} else if (Write *write = transition->instruction()->as<Write>()) {
					resultThread->makeTransition(
						helperFrom,
						helperTo,
						std::make_shared<Atomic>(
							transition->instruction(),
							std::make_shared<Write>(hb_read, write->address(), HB_SPACE)
						)
					);
					resultThread->makeTransition(
						helperFrom,
						helperTo,
						std::make_shared<Atomic>(
							std::make_shared<Read>(addr, attackAddr, SERVICE_SPACE),
							std::make_shared<Condition>(std::make_shared<BinaryOperator>(BinaryOperator::EQ, addr, write->address())),
							std::make_shared<Write>(one, successAddr, SERVICE_SPACE)
						)
					);
				} else if (transition->instruction()->as<Atomic>()) {
					assert(!"Sorry, atomics in input programs are not supported.");
				} else {
					resultThread->makeTransition(helperFrom, helperTo, transition->instruction());
				}
			}
		}
	}
}

} // namespace trench
