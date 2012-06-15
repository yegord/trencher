set(TRENCH_SOURCES
	Expression.h
	Foreach.h
	Instruction.h
	Kinds.h
	NaiveParser.cpp
	NaiveParser.h
	NaivePrinter.cpp
	NaivePrinter.h
	Printer.h
	Program.cpp
	Program.h
	SpinPrinter.h
	SpinPrinter.cpp
	State.h
	Thread.cpp
	Thread.h
	Transition.cpp
	Transition.h
	config.h
)

add_library(trench ${TRENCH_SOURCES})
