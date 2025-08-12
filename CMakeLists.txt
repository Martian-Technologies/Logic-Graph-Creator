# ======================================== CMAKE SETUP ========================================
cmake_minimum_required(VERSION 3.28)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies.cmake)

if(APPLE)
	set(CMAKE_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk")
	if (CONNECTION_MACHINE_DISTRIBUTE_APP)
		set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)
		set(Vulkan_LIBRARY "${CMAKE_SOURCE_DIR}/lib-binary/libMoltenVK.dylib")
	endif()
endif()

# generate compile commands json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set c++ standards as global variables
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# ======================================== PROJECT SETUP ========================================
set(BUILD_SHARED_LIBS OFF) # Don't build shared libs
project(Connection_Machine VERSION 0.8.0)
if (APPLE)
	set(APP_NAME "Connection Machine")
else()
	set(APP_NAME Connection_Machine)
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

# ======================================= COVERAGE SETUP ========================================

if(CONNECTION_MACHINE_CODE_COVERAGE)
	if (CONNECTION_MACHINE_BUILD_TESTS)
		if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
			message(STATUS "Code coverage enabled")
			add_compile_options(--coverage -O0 -g)
			add_link_options(--coverage)
		elseif(MSVC)
			message(WARNING "Code coverage not supported with MSVC. Use Clang-cl or OpenCppCoverage.")
		else()
			message(WARNING "Code coverage not working with: \"${CMAKE_CXX_COMPILER_ID}\"")
		endif()
	else()
		message(WARNING "Code coverage cant not be used without also enabling tests.")
	endif()
	
endif()

# parallel hashmap
set(CONNECTION_MACHINE_PARALLEL_HASHMAP_LIB_NAME "parallel_hashmap")
add_library(${CONNECTION_MACHINE_PARALLEL_HASHMAP_LIB_NAME} INTERFACE)
target_include_directories(${CONNECTION_MACHINE_PARALLEL_HASHMAP_LIB_NAME} INTERFACE "${EXTERNAL_DIR}/parallel_hashmap/")

# tracy
if (RUN_TRACY_PROFILER)
	message("tracy is ON")
	option(TRACY_ENABLE "" ON)
	option(TRACY_ON_DEMAND "" ON)
	add_subdirectory("${EXTERNAL_DIR}/tracy")
endif()

if (CONNECTION_MACHINE_BUILD_APP)
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/makeApp.cmake)
endif()

# ==================================== CREATE TESTING EXECUTABLE ============================================
if (CONNECTION_MACHINE_BUILD_TESTS)
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/makeTests.cmake)
endif()
