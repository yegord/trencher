/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "Expression.h"

#include <cassert>

namespace trench {

const char *UnaryOperator::getOperatorSign() const {
	switch (kind()) {
		case NOT:
			return "!";
		default:
			assert(!"NEVER REACHED");
			return NULL;
	}
}

const char *BinaryOperator::getOperatorSign() const {
	switch (kind()) {
		case EQ:
			return "==";
		case NEQ:
			return "!=";
		case LT:
			return "<";
		case LEQ:
			return "<=";
		case GT:
			return ">";
		case GEQ:
			return ">=";
		case AND:
			return "&&";
		case OR:
			return "||";
		case ADD:
			return "+";
		case SUB:
			return "-";
		case MUL:
			return "*";
		case BIN_AND:
			return "&";
		default:
			assert(!"NEVER REACHED");
			return NULL;
	}
}

} // namespace trench
