project(libbacktrace-cc)

cmake_minimum_required (VERSION 2.6)
if (NOT CMAKE_VERSION VERSION_LESS 2.8.4)
  cmake_policy (SET CMP0017 NEW)
endif()

option(WITH_BFD "Enable BFD library support for detailed symbol information gathering" ON)
option(WITH_UCONTEXT "Enable ucontext for getting current address from a signal handler" ON)

option(WITH_EXAMPLES "Enable example applications" ON)

include(CMakeParseArguments)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
message(STATUS "Project source dir = ${PROJECT_SOURCE_DIR}")
message(STATUS "Project build dir = ${CMAKE_BINARY_DIR}")

if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

include_directories(include)
include_directories(src)

# TODO(sergey): Perform some autotest to see if the things
# are actually awailable on the system.
if(WITH_BFD)
	add_definitions(-DWITH_BFD)
endif()
if(WITH_UCONTEXT)
	add_definitions(-DWITH_UCONTEXT)
endif()

add_library(backtrace
	src/backtrace/backtrace.cc
	src/backtrace/backtrace_util.cc
	src/backtrace/demangle.cc
	src/backtrace/stacktrace.cc
	src/backtrace/stacktrace_capture_stack_backtrace.cc
	src/backtrace/stacktrace_execinfo.cc
	src/backtrace/stacktrace_stack_walk.cc
	src/backtrace/stacktrace_stub.cc
	src/backtrace/symbolize_bfd.cc
	src/backtrace/symbolize.cc
	src/backtrace/symbolize_execinfo.cc
	src/backtrace/symbolize_stub.cc
	src/backtrace/symbolize_sym_from_addr.cc

	include/backtrace/backtrace.h
	src/backtrace/backtrace_util.h
	src/backtrace/demangle.h
	src/backtrace/stacktrace.h
	src/backtrace/symbolize.h
)

if(WITH_EXAMPLES)
	add_executable(print_backtrace examples/print_backtrace.c)
	target_link_libraries(print_backtrace backtrace)
	if(WITH_BFD)
		target_link_libraries(print_backtrace bfd dl)
	endif()
	if(MSVC)
		target_link_libraries(print_backtrace dbghelp psapi)
	endif()
endif()
