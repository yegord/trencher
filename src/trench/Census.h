#pragma once

#include "Expression.h"
#include "Instruction.h"

namespace trench {

class Program;
class Thread;

class Census {
	mutable std::vector<Instruction *> instructions_;
	mutable std::vector<Expression *> expressions_;
	mutable std::vector<Space> spaces_;
	mutable bool unique_;

	public:

	Census(): unique_(false) {}

	void visit(const Program &program);
	void visit(const Thread *thread);
	void visit(const std::shared_ptr<Instruction> &instruction);
	void visit(const std::shared_ptr<Expression> &expression);

	const std::vector<Instruction *> &instructions() const { unique(); return instructions_; }
	const std::vector<Expression *> &expressions() const { unique(); return expressions_; }
	const std::vector<Space> &spaces() const { unique(); return spaces_; }
	
	private:

	void unique() const;
};

} // namespace trench
