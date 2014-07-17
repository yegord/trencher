set(TRENCH_SOURCES
	Kinds.h
	Expression.h
	Expression.cpp
#  MPrinter.h
#  MPrinter.cpp
#  CPrinter.h
#  CPrinter.cpp
  DotPrinter.h
	DotPrinter.cpp
  SpinPrinter.h
  SpinPrinter.cpp
  NaiveParser.h
  NaiveParser.cpp
  TrailParser.h
  TrailParser.cpp
  Benchmarking.cpp
	Benchmarking.h
	Census.cpp
	Census.h
	FenceInsertion.cpp
	FenceInsertion.h
	Foreach.h
	Instruction.h
	Program.cpp
	Program.h
	Reduction.cpp
	Reduction.h
	RobustnessChecking.cpp
	RobustnessChecking.h
	SpinModelChecker.cpp
	SpinModelChecker.h
	State.h
	Thread.cpp
	Thread.h
	Transition.cpp
	Transition.h
	config.h
  ReachabilityCheckingTSO.cpp
  ReachabilityCheckingSC.cpp
  ReachabilityChecking.h
  Instrumentation.cpp
  Instrumentation.h
)

add_library(trench ${TRENCH_SOURCES})
