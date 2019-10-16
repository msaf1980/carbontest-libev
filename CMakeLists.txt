cmake_minimum_required( VERSION 3.5 )

# Options can be turned on and off by providing cmake with a -DOPTIONNAME=ON (or OFF) argument.
# For example, to turn off benchmarks, and use sanitize flags in a DEBUG build:
#    cmake .. -DCMAKE_BUILD_TYPE=DEBUG -DBENCH=OFF -DSANITIZE=ON

# Add test files with filenames <NAME>_test.cpp or <NAME>_test.c into test dir
# Add bench files with filenames <NAME>_bench.cpp or <NAME>_bench.c into test dir

# Override install prefix with CMAKE_INSTALL_PREFIX
# Override bin dir with BINDIR
# Override lib dir with LIBDIR

option(TEST      "Build tests" ON)
option(BENCH     "Build benchmarks" OFF)
option(ASAN      "Adds sanitize flags" OFF)
option(TSAN      "Adds thread sanitize flags" OFF)
option(DEBUGINFO   "Add debug info" ON)
#option(USE_COTIRE "Enable cutire build" ON)

if(ASAN)
  set(TSAN OFF)
endif()

if(BENCH)
  set(TEST ON)
endif()

set( DIR_SOURCES src )
set( DIR_INCLUDES include )
#set( DIR_TESTS test )
#set( DIR_TESTS_INTEGRATION test_integration )
set( DIR_TESTS_TOOLS tools )
#set( DIR_BENCHS bench )
set( DIR_DEP dep )
set( DIR_PRESCRIPT cmake_pre )
set( DIR_SCRIPT cmake )

if (NOT DEFINED BINDIR)
set( BINDIR bin )
endif()
if (NOT DEFINED LIBDIR)
set( LIBDIR lib )
endif()

set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

# Include cmake script from cmake dir
file(GLOB cmakeFiles ${DIR_PRESCRIPT}/*.cmake)
foreach(cmakeFile ${cmakeFiles})
    message("INCLUDE ${cmakeFile}")
    INCLUDE(${cmakeFile})
endforeach(cmakeFile)

#list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

set ( PROJECT carbontest-libev )
set ( BINARY ${PROJECT} )

project( ${PROJECT} )

################################################################################
# Conan
if(EXISTS "${CMAKE_SOURCE_DIR}/conan_load.cmake")
include(${CMAKE_SOURCE_DIR}/conan_load.cmake)
endif()
################################################################################

file(GLOB cmakeFiles ${DIR_SCRIPT}/*.cmake)
foreach(cmakeFile ${cmakeFiles})
    message("INCLUDE ${cmakeFile}")
    INCLUDE(${cmakeFile})
endforeach(cmakeFile)

set ( LIBRARIES
    pthread
    ${CONAN_LIBS_LIBEV}
    ${CONAN_LIBS_FMT}
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if ( DEFINED DIR_INCLUDES )
    # Includes in separate directory
    include_directories( ${DIR_INCLUDES} ${fmt_INCLUDE_DIRS} contrib/concurrentqueue contrib/plog/include contrib/c_procs/include contrib/cpp_procs/include )
endif()

#Scan dir for standart source files
aux_source_directory( ${DIR_SOURCES} SOURCES )

list(APPEND SOURCES
	contrib/c_procs/src/daemonutils.c
	contrib/c_procs/src/fileutils.c
	contrib/cpp_procs/src/threads/spinning_barrier.cpp
)

#Add sources from dir
#set( SOURCES
#    ${DIR_SOURCES}/main.cpp
#    ${DIR_SOURCES}/palindrom.cpp
#)

# Add executable target
add_executable( ${BINARY} ${SOURCES} )
#target_include_directories( ${BINARY} ${DIR_INCLUDES} )
if(LIBRARIES)
    target_link_libraries ( ${BINARY} ${LIBRARIES} )
endif()
#if (FMT_HEADER_ONLY)
#    target_compile_definitions(${BINARY} PRIVATE FMT_HEADER_ONLY=1)
#endif(FMT_HEADER_ONLY)
set_target_properties( ${BINARY} PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE )
#cotire( ${BINARY} )

if(DEFINED ENV{DEVENV})
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

INSTALL( TARGETS ${BINARY} DESTINATION ${BINDIR} )

enable_testing()

if (TEST)

if ( DEFINED DIR_TESTS )
	#set enable testing
	foreach ( dir IN LISTS DIR_TESTS )
		message("add test ${dir}")
		add_subdirectory( ${dir} )
	endforeach()
endif()

if ( DEFINED DIR_TESTS_INTEGRATION )
	#set enable testing
	foreach ( dir IN LISTS DIR_TESTS_INTEGRATION )
		message("add integration test ${dir}")
		add_subdirectory( ${dir} )
	endforeach()
endif()

if ( DEFINED DIR_TESTS_TOOLS )
	#set enable testing
	foreach ( dir IN LISTS DIR_TESTS_TOOLS )
		message("add tools from ${dir}")
		file(GLOB cmakeFiles ${dir}/*.cmake)
		foreach(cmakeFile ${cmakeFiles})
			message("INCLUDE ${cmakeFile}")
			INCLUDE(${cmakeFile})
		endforeach(cmakeFile)
	endforeach()
endif()

endif() # END TEST


message(STATUS "")
message(STATUS "BUILD SUMMARY")
message(STATUS "  CMAKE_GENERATOR       : ${CMAKE_GENERATOR}")
message(STATUS "  Compiler ID           : ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  Build type            : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build tests           : ${TEST}")
message(STATUS "  Build benchmarks      : ${BENCH}")
message(STATUS "  Sanitize flags        : ${SANITIZE}")
message(STATUS "  Thread Sanitize flags : ${TSANITIZE}")
message(STATUS "")
message(STATUS "  Install prefix : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Binary dir     : ${BINDIR}")
message(STATUS "  Lib dir        : ${LIBDIR}")
message(STATUS "")
message(STATUS "CMAKE_CXX_FLAGS            : ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_C_FLAGS              : ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_SHARED_LINKER_FLAGS  : ${CMAKE_SHARED_LINKER_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS     : ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "")
