cmake_minimum_required(VERSION 2.6)
project(TRENCHER C CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif(NOT CMAKE_BUILD_TYPE)

if(CMAKE_COMPILER_IS_GNUCXX)
	add_definitions(-std=c++0x)
	add_definitions(-Wall -Wextra)
	add_definitions(-fshow-column)
endif(CMAKE_COMPILER_IS_GNUCXX)

find_package(Boost 1.46.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

include_directories(${TRENCHER_SOURCE_DIR})

add_subdirectory(trench)
add_subdirectory(trencher)
