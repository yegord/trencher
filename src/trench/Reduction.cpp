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

void reduce(const Program &program, Program &resultProgram, bool searchForTdrOnly, Thread *attacker, Transition *attackWrite, Transition *attackRead, const boost::unordered_set<State *> &fenced) {
	assert(attackWrite == NULL || attackWrite->instruction()->is<Write>());
	assert(attackRead == NULL || attackRead->instruction()->is<Read>());

	resultProgram.setMemorySize(std::max(program.memorySize(), 2));

	enum {
		DEFAULT_SPACE = Space(),
		BUFFER_SPACE,
		IS_BUFFERED_SPACE,
		HB_SPACE,
		SERVICE_SPACE
	};

	const std::shared_ptr<Constant>  zero = resultProgram.makeConstant(0);
	const std::shared_ptr<Constant>  one  = resultProgram.makeConstant(1);

	const std::shared_ptr<Register>  is_buffered(resultProgram.makeRegister("_is_buffered"));

	const std::shared_ptr<Condition> check_is_buffered(
		new Condition(std::make_shared<BinaryOperator>(BinaryOperator::EQ, is_buffered, one)));
	const std::shared_ptr<Condition> check_is_not_buffered(
		new Condition(std::make_shared<BinaryOperator>(BinaryOperator::EQ, is_buffered, zero)));

	const std::shared_ptr<Constant>  attackAddrVar = resultProgram.makeConstant(0);
	const std::shared_ptr<Constant>  nattackersVar = resultProgram.makeConstant(1);
	const std::shared_ptr<Constant>  successVar    = resultProgram.makeConstant(2);

	resultProgram.setInterestingAddress(successVar->value(), SERVICE_SPACE);

	const std::shared_ptr<Register>  addr(resultProgram.makeRegister("_addr"));
	const std::shared_ptr<Register>  nattackers(resultProgram.makeRegister("_attacking"));

	const std::shared_ptr<Constant>  hb_nothing = resultProgram.makeConstant(0);
	const std::shared_ptr<Constant>  hb_read    = resultProgram.makeConstant(1);
	const std::shared_ptr<Constant>  hb_write   = resultProgram.makeConstant(2);

	const std::shared_ptr<Register>  access_type(resultProgram.makeRegister("_access_type"));

	const std::shared_ptr<Condition> check_access_type_is_write(
		new Condition(std::make_shared<BinaryOperator>(BinaryOperator::EQ, access_type, hb_write)));
	const std::shared_ptr<Condition> check_access_type_is_not_write(
		new Condition(std::make_shared<BinaryOperator>(BinaryOperator::NEQ, access_type, hb_write)));
	const std::shared_ptr<Condition> check_access_type_is_read_or_write(
		new Condition(std::make_shared<BinaryOperator>(BinaryOperator::NEQ, access_type, hb_nothing)));

	const std::shared_ptr<Register>  tmp(resultProgram.makeRegister("_tmp"));

	const std::shared_ptr<Condition> check_not_blocked(
		new Condition(std::make_shared<NotBlocked>()));

	foreach (Thread *thread, program.threads()) {
		Thread *resultThread = resultProgram.makeThread(thread->name());

		if (thread->initialState()) {
			resultThread->setInitialState(resultThread->makeState("orig_" + thread->initialState()->name()));
		}

		foreach (Transition *transition, thread->transitions()) {
			/*
			 * Original code.
			 */
			State *originalFrom = resultThread->makeState("orig_" + transition->from()->name());
			State *originalTo = resultThread->makeState("orig_" + transition->to()->name());

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

			if (thread == attacker || attacker == NULL) {
				State *attackerFrom = resultThread->makeState("att_" + transition->from()->name());
				State *attackerTo   = resultThread->makeState("att_" + transition->to()->name());

				/*
				 * Becoming an attacker.
				 */
				if (Write *write = transition->instruction()->as<Write>()) {
					if (transition == attackWrite || attackWrite == NULL) {
						/* First write going into the buffer. */
						resultThread->makeTransition(
							originalFrom,
							attackerTo,
							std::make_shared<Atomic>(
								std::make_shared<Read> (nattackers,       nattackersVar,    SERVICE_SPACE),
								std::make_shared<Condition>(std::make_shared<BinaryOperator>(BinaryOperator::EQ, nattackers, zero)),
								std::make_shared<Write>(write->value(),   write->address(), BUFFER_SPACE),
								std::make_shared<Write>(one,              write->address(), IS_BUFFERED_SPACE),
								std::make_shared<Write>(write->address(), attackAddrVar,    SERVICE_SPACE),
								std::make_shared<Write>(one,              nattackersVar,    SERVICE_SPACE)
							)
						);
					}
				}

				/*
				 * Attacker's execution.
				 */

				if (fenced.find(transition->from()) != fenced.end()) {
					/* No transition from an extra fenced state. */
				} else if (Write *write = transition->instruction()->as<Write>()) {
					if (!searchForTdrOnly) {
						/* Writes write to the buffer. */
						resultThread->makeTransition(
							attackerFrom,
							attackerTo,
							std::make_shared<Atomic>(
								std::make_shared<Write>(write->value(), write->address(), BUFFER_SPACE),
								std::make_shared<Write>(one,            write->address(), IS_BUFFERED_SPACE)
							)
						);
					}
				} else if (Read *read = transition->instruction()->as<Read>()) {
					/* Reads either read from the buffer... */
					resultThread->makeTransition(
						attackerFrom,
						attackerTo,
						std::make_shared<Atomic>(
							check_not_blocked,
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
							check_not_blocked,
							std::make_shared<Read>(is_buffered, read->address(), IS_BUFFERED_SPACE),
							check_is_not_buffered,
							transition->instruction()
						)
					);

					/* ...or from memory and update HB. This is the final aim of attacker's existence. */
					if (transition == attackRead || attackRead == NULL) {
						resultThread->makeTransition(
							attackerFrom,
							resultThread->makeState("final"),
							std::make_shared<Atomic>(
								check_not_blocked,
								std::make_shared<Read> (is_buffered, read->address(), IS_BUFFERED_SPACE),
								check_is_not_buffered,
								transition->instruction(),
								std::make_shared<Write>(hb_read,     read->address(), HB_SPACE)
							)
						);
					}
				} else if (transition->instruction()->as<Mfence>()) {
					/* No transition: attacker can't execute fences. */
				} else if (transition->instruction()->as<Lock>() ||
				           transition->instruction()->as<Unlock>()) {
					/* No transition: attacker can't execute locked instructions. */
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

				if (searchForTdrOnly) {
					if (Write *write = transition->instruction()->as<Write>()) {
						resultThread->makeTransition(
							originalFrom,
							helperTo,
							std::make_shared<Atomic>(
								check_not_blocked,
								std::make_shared<Read>(access_type, write->address(), HB_SPACE),
								check_access_type_is_read_or_write,
								std::make_shared<Write>(one, successVar, SERVICE_SPACE)
							)
						);
					}
				} else {
					/*
					 * Becoming a helper.
					 */
					/* One can become a helper only on a read or a write. */
					if (Read *read = transition->instruction()->as<Read>()) {
						resultThread->makeTransition(
							originalFrom,
							helperTo,
							std::make_shared<Atomic>(
								check_not_blocked,
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
								check_not_blocked,
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
								check_not_blocked,
								std::make_shared<Read>(access_type, read->address(), HB_SPACE),
								check_access_type_is_write,
								transition->instruction()
							)
						);
						resultThread->makeTransition(
							helperFrom,
							helperTo,
							std::make_shared<Atomic>(
								check_not_blocked,
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
								check_not_blocked,
								std::make_shared<Read>(addr, attackAddrVar, SERVICE_SPACE),
								std::make_shared<Condition>(std::make_shared<BinaryOperator>(BinaryOperator::EQ, addr, read->address())),
								std::make_shared<Write>(one, successVar, SERVICE_SPACE)
							)
						);
					} else if (Write *write = transition->instruction()->as<Write>()) {
						resultThread->makeTransition(
							helperFrom,
							helperTo,
							std::make_shared<Atomic>(
								check_not_blocked,
								transition->instruction(),
								std::make_shared<Write>(hb_read, write->address(), HB_SPACE)
							)
						);
						resultThread->makeTransition(
							helperFrom,
							helperTo,
							std::make_shared<Atomic>(
								check_not_blocked,
								std::make_shared<Read>(addr, attackAddrVar, SERVICE_SPACE),
								std::make_shared<Condition>(std::make_shared<BinaryOperator>(BinaryOperator::EQ, addr, write->address())),
								std::make_shared<Write>(one, successVar, SERVICE_SPACE)
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
}

} // namespace trench
