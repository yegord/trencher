set(TRENCHER_SOURCES
	main.cpp
	Examples.cpp
	Examples.h
)

add_executable(trencher ${TRENCHER_SOURCES})
target_link_libraries(trencher trench ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
