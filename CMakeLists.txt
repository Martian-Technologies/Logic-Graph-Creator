# ======================================== CMAKE SETUP ========================================
cmake_minimum_required(VERSION 3.10)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/apple.cmake)

if(APPLE)
	PreProjectAppleSetup()
endif()

set(CMAKE_POLICY_VERSION_MINIMUM 3.10)

# generate compile commands json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set c++ standards as global variables
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# ======================================== PROJECT SETUP ========================================
set(BUILD_SHARED_LIBS OFF) # Don't build shared libs
project(Connection_Machine VERSION 0.8.0)
if (APPLE)
	set(APP_NAME "Connection Machine") # name of the bundle
else()
	set(APP_NAME ${PROJECT_NAME})
endif()

# Set directories
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")

# Find source files
file(GLOB_RECURSE PROJECT_SOURCES
	"${SOURCE_DIR}/*.cpp"
)

option(CONNECTION_MACHINE_BUILD_APP "Build Connection Machine App" ON)
option(CONNECTION_MACHINE_DISTRIBUTE_APP "Distribute App" OFF)
option(CONNECTION_MACHINE_BUILD_TESTS "Build Connection Machine Tests" OFF)
option(CONNECTION_MACHINE_CODE_COVERAGE "Enable code coverage reporting" OFF)
option(RUN_TRACY_PROFILER "Enable runtime profiler" OFF)

# ====================================== SILENCE WARNINGS =======================================

if(MSVC)
	# C4244: 'conversion' conversion from 'type1' to 'type2', possible loss of data
	# C4267: 'var' conversion from 'size_t' to 'type', possible loss of data
	# C4018: 'var' signed/unsigned mismatch
	add_compile_options(/wd4244 /wd4267 /wd4018)
endif()

# ======================================= COVERAGE SETUP ========================================

if(CONNECTION_MACHINE_CODE_COVERAGE)
	if (CONNECTION_MACHINE_BUILD_TESTS)
		if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
			message(STATUS "Code coverage enabled")
			add_compile_options(--coverage -O0 -g)
			add_link_options(--coverage)
		elseif(MSVC)
		else()
			message(WARNING "Code coverage not working with: \"${CMAKE_CXX_COMPILER_ID}\"")
		endif()
	else()
		message(WARNING "Code coverage cant not be used without also enabling tests.")
	endif()
endif()

set(EXTERNAL_LINKS)

if (CONNECTION_MACHINE_BUILD_APP)
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/makeApp.cmake)
endif()

# ==================================== CREATE TESTING EXECUTABLE ============================================
if (CONNECTION_MACHINE_BUILD_TESTS)
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/makeTests.cmake)
endif()
