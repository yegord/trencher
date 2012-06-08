set(TRENCH_SOURCES
	Expression.h
	Foreach.h
	Instruction.h
	Kinds.h
	Program.h
	State.h
	Thread.h
	Transition.h
	Transition.cpp
	Program.cpp
	Thread.cpp
	config.h
)

add_library(trench ${TRENCH_SOURCES})
