cmake_minimum_required (VERSION 2.8.10)
project (neuromapp)

# The version number.
set(LAST_RELEASE 1.0.0)
set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_PATCH "0")
set(VERSION_ABI "1")

option (NEUROMAPP_CURSOR "Allow the use of cursors during input" OFF)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)
include(Compiler)
string(REPLACE "-std=c++11" " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

set(Boost_USE_STATIC_LIBS  ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_DIALECT_OPT_C99EXT}")


#find_package(Boost COMPONENTS thread date_time program_options filesystem system REQUIRED)
find_package(Boost 1.44 REQUIRED program_options unit_test_framework system filesystem atomic)
find_package(SLURM)
# boost include
include_directories(${Boost_INCLUDE_DIRS})

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g ")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ")

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if(NEUROMAPP_CURSOR)
    MESSAGE(STATUS "Using Cursors")
    add_definitions(-DNEUROMAPP_CURSOR)

    if(NOT READLINE_FOUND)
	find_path(READLINE_INCLUDE_DIR readline/readline.h
	    HINTS ${READLINE_ROOT_DIR} PATH_SUFFIXES include)
	find_library(READLINE_LIBRARY readline
	    HINTS ${READLINE_ROOT_DIR} PATH_SUFFIXES lib)
	find_library(NCURSES_LIBRARY ncurses)   # readline depends on libncurses
	mark_as_advanced(READLINE_INCLUDE_DIR READLINE_LIBRARY NCURSES_LIBRARY)

	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(Readline DEFAULT_MSG
	READLINE_LIBRARY NCURSES_LIBRARY READLINE_INCLUDE_DIR)

	set(READLINE_INCLUDE_DIRS ${READLINE_INCLUDE_DIR})
	set(READLINE_LIBRARIES ${READLINE_LIBRARY} ${NCURSES_LIBRARY})
    endif(NOT READLINE_FOUND)

endif(NEUROMAPP_CURSOR)


enable_testing()

include_directories(neuromapp)
add_subdirectory (neuromapp)
#test
add_subdirectory (test)

#prepare the input
execute_process(COMMAND unzip -o ${PROJECT_SOURCE_DIR}/neuromapp/coreneuron_1.0/common/data/bench.101392.zip
                              -d ${PROJECT_BINARY_DIR}/test/bench.101392)

#prepare the ref solution for the test
execute_process(COMMAND tar -xzvf ${PROJECT_SOURCE_DIR}/neuromapp/coreneuron_1.0/common/data/rhs_d_ref.tar.gz
                            -C    ${PROJECT_BINARY_DIR}/test/)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMake/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
                           ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
                           WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                           COMMENT "Generating API documentation with Doxygen" VERBATIM
                     )
endif()
