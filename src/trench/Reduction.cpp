/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "Reduction.h"

#include <algorithm>
#include <cassert>

#include "Expression.h"
#include "ExpressionsCache.h"
#include "Instruction.h"
#include "Program.h"
#include "State.h"
#include "Transition.h"

namespace trench {

void reduce(const Program &program, Program &resultProgram, bool searchForTdrOnly, Thread *attacker, Transition *attackWrite, Transition *attackRead, const boost::unordered_set<State *> &fenced) {
	assert(attackWrite == NULL || attackWrite->instruction()->is<Write>());
	assert(attackRead == NULL || attackRead->instruction()->is<Read>());

	ExpressionsCache cache;

	resultProgram.setMemorySize(std::max(program.memorySize(), 3));

	enum {
		DEFAULT_SPACE = Space(),
		BUFFER_SPACE,
		IS_BUFFERED_SPACE,
		HB_SPACE,
		SERVICE_SPACE
	};

	auto zero = cache.makeConstant(0);
	auto one  = cache.makeConstant(1);

	auto is_buffered(cache.makeRegister("_is_buffered"));

	auto check_is_buffered = std::make_shared<Condition>(std::make_shared<BinaryOperator>(BinaryOperator::EQ, is_buffered, one));
	auto check_is_not_buffered = std::make_shared<Condition>(std::make_shared<BinaryOperator>(BinaryOperator::EQ, is_buffered, zero));

	auto attackAddrVar = cache.makeConstant(0);
	auto nattackersVar = cache.makeConstant(1);
	auto successVar    = cache.makeConstant(2);

	resultProgram.setInterestingAddress(successVar->value(), SERVICE_SPACE);

	auto addr = cache.makeRegister("_addr");
	auto nattackers = cache.makeRegister("_attacking");

	auto hb_nothing = cache.makeConstant(0);
	auto hb_read    = cache.makeConstant(1);
	auto hb_write   = cache.makeConstant(2);

	auto access_type = cache.makeRegister("_access_type");

	auto check_access_type_is_write = std::make_shared<Condition>(std::make_shared<BinaryOperator>(BinaryOperator::EQ, access_type, hb_write));
	auto check_access_type_is_not_write = std::make_shared<Condition>(std::make_shared<BinaryOperator>(BinaryOperator::NEQ, access_type, hb_write));
	auto check_access_type_is_read_or_write = std::make_shared<Condition>(std::make_shared<BinaryOperator>(BinaryOperator::NEQ, access_type, hb_nothing));

	auto tmp = cache.makeRegister("_tmp");

	auto check_can_access_memory = std::make_shared<Condition>(std::make_shared<CanAccessMemory>());

	for (Thread *thread : program.threads()) {
		Thread *resultThread = resultProgram.makeThread(thread->name());

		if (thread->initialState()) {
			resultThread->setInitialState(resultThread->makeState("orig_" + thread->initialState()->name()));
		}

		for (Transition *transition : thread->transitions()) {
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
						check_can_access_memory,
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
							check_can_access_memory,
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
							check_can_access_memory,
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
								check_can_access_memory,
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
								check_can_access_memory,
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
								check_can_access_memory,
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
								check_can_access_memory,
								std::make_shared<Read>(access_type, write->address(), HB_SPACE),
								check_access_type_is_read_or_write,
								transition->instruction(),
								std::make_shared<Write>(hb_write,   write->address(), HB_SPACE)
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
								check_can_access_memory,
								std::make_shared<Read>(access_type, read->address(), HB_SPACE),
								check_access_type_is_write,
								transition->instruction()
							)
						);
						resultThread->makeTransition(
							helperFrom,
							helperTo,
							std::make_shared<Atomic>(
								check_can_access_memory,
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
								check_can_access_memory,
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
								check_can_access_memory,
								transition->instruction(),
								std::make_shared<Write>(hb_write, write->address(), HB_SPACE)
							)
						);
						resultThread->makeTransition(
							helperFrom,
							helperTo,
							std::make_shared<Atomic>(
								check_can_access_memory,
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
