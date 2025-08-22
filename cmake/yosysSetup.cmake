# yosys
CPMAddPackage(
	GITHUB_REPOSITORY YosysHQ/yosys
	GIT_TAG 65109822b4db80cb740e31cfbeb27ff1159acdf9
	DOWNLOAD_ONLY YES
	SOURCE_DIR "${EXTERNAL_DIR}/yosys"
)

# link yosys-slang to yosys to have it loaded as a plugin
if(MSVC)
    set(YOSYS_LIB_SUFFIX ".dll")
else()
    set(YOSYS_LIB_SUFFIX ".so")
endif()
set(YOSYS_BIN_LOCATION "${yosys_BINARY_DIR}/libyosys${YOSYS_LIB_SUFFIX}")

file(WRITE "${CMAKE_BINARY_DIR}/cmake/FindYosys.cmake"
	"include(CMakeFindDependencyMacro)
	add_library(yosys::yosys INTERFACE IMPORTED)
	add_compile_definitions(YOSYS_DISABLE_SPAWN)
	add_compile_definitions(_YOSYS_)
	target_include_directories(yosys::yosys INTERFACE
		${EXTERNAL_DIR}/yosys/share/include
	)\n"
)

# Tell CMake where to look when someone calls find_package(Yosys)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/cmake")
# yosys slang
set(YOSYS_CONFIG "${EXTERNAL_DIR}/yosys/misc/yosys-config.in" CACHE STRING "Location of yosys-config utility" FORCE)
CPMAddPackage(
	NAME yosys-slang
	GITHUB_REPOSITORY ItchyTrack/yosys-slang
	GIT_TAG b8edd6bceed8d1097132409700ac6d76069a4b82
	OPTIONS
		"BUILD_AS_PLUGIN OFF"
	SOURCE_DIR "${EXTERNAL_DIR}/yosys-slang"
)
set(YOSYS_SLANG_BIN_LOCATION "${CMAKE_BINARY_DIR}/libyosys-slang.a")
# list(APPEND EXTERNAL_LINKS yosys-slang)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
    set(YOSYS_CONFIG_COMPILER "clang")
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(YOSYS_CONFIG_COMPILER "gcc")
elseif(MSVC)
    set(YOSYS_CONFIG_COMPILER "none")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(YOSYS_CONFIG_COMPILER "msys2-64")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(YOSYS_CONFIG_COMPILER "msys2-32")
else()
    set(YOSYS_CONFIG_COMPILER "none")
endif()

if(MSVC)
    set(YOSYS_LINK_FLAGS "")
else()
	if(APPLE)
		set(YOSYS_LINK_FLAGS "-Wl,-force_load,${YOSYS_SLANG_BIN_LOCATION}")
	else()
		set(YOSYS_LINK_FLAGS "-Wl,--push-state,--whole-archive ${YOSYS_SLANG_BIN_LOCATION} -Wl,--pop-state")
	endif()
endif()

# build yosys
file(WRITE "${yosys_BINARY_DIR}/Makefile.conf" "CONFIG := ${YOSYS_CONFIG_COMPILER}\n")
file(APPEND "${yosys_BINARY_DIR}/Makefile.conf" "LINKFLAGS += ${YOSYS_LINK_FLAGS}\n")
file(APPEND "${yosys_BINARY_DIR}/Makefile.conf" "ENABLE_TCL := 0\n")
file(APPEND "${yosys_BINARY_DIR}/Makefile.conf" "ENABLE_ABC := 0\n")
file(APPEND "${yosys_BINARY_DIR}/Makefile.conf" "ENABLE_GLOB := 1\n")
file(APPEND "${yosys_BINARY_DIR}/Makefile.conf" "ENABLE_PLUGINS := 1\n")
file(APPEND "${yosys_BINARY_DIR}/Makefile.conf" "ENABLE_READLINE := 0\n")
file(APPEND "${yosys_BINARY_DIR}/Makefile.conf" "ENABLE_VERIFIC_SYSTEMVERILOG := 0\n")
file(APPEND "${yosys_BINARY_DIR}/Makefile.conf" "ENABLE_VERIFIC_EDIF := 0\n")
file(APPEND "${yosys_BINARY_DIR}/Makefile.conf" "ENABLE_VERIFIC_LIBERTY := 0\n")
file(APPEND "${yosys_BINARY_DIR}/Makefile.conf" "ENABLE_COVER := 1\n")
file(APPEND "${yosys_BINARY_DIR}/Makefile.conf" "ENABLE_LIBYOSYS := 1\n")
file(APPEND "${yosys_BINARY_DIR}/Makefile.conf" "DISABLE_SPAWN := 1\n")
file(APPEND "${yosys_BINARY_DIR}/Makefile.conf" "ENABLE_ZLIB := 1\n")

find_program(MAKE_EXE make)
if(NOT MAKE_EXE)
	if (WIN32)
		message(FATAL_ERROR "GNU Make not found. On Windows, install MSYS2 or MinGW and ensure 'make' is in PATH.")
	elseif()
		message(FATAL_ERROR "Make not found! ensure 'make' is in PATH.")
	endif()
endif()

execute_process(
	COMMAND ${MAKE_EXE} -f "${EXTERNAL_DIR}/yosys/Makefile" share
    WORKING_DIRECTORY ${EXTERNAL_DIR}/yosys
)

if (APPLE)
	add_custom_target(build_yosys ALL
		BYPRODUCTS ${YOSYS_BIN_LOCATION}
		COMMAND ${MAKE_EXE} -f ${EXTERNAL_DIR}/yosys/Makefile -j8 libyosys${YOSYS_LIB_SUFFIX}
		COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id ${YOSYS_BIN_LOCATION} ${YOSYS_BIN_LOCATION}
		WORKING_DIRECTORY ${yosys_BINARY_DIR}
		COMMENT "Building Yosys..."
		USES_TERMINAL
	)
else()
	add_custom_target(build_yosys ALL
		BYPRODUCTS ${YOSYS_BIN_LOCATION}
		COMMAND ${MAKE_EXE} -f ${EXTERNAL_DIR}/yosys/Makefile -j8 libyosys${YOSYS_LIB_SUFFIX}
		WORKING_DIRECTORY ${yosys_BINARY_DIR}
		COMMENT "Building Yosys..."
		USES_TERMINAL
	)
endif()
add_dependencies(build_yosys yosys-slang)

# link it together
add_library(Yosys SHARED IMPORTED)
add_dependencies(Yosys build_yosys)
add_compile_definitions(YOSYS_DISABLE_SPAWN)
add_compile_definitions(_YOSYS_)
set_target_properties(Yosys PROPERTIES
	IMPORTED_LOCATION ${YOSYS_BIN_LOCATION}
	INTERFACE_INCLUDE_DIRECTORIES ${EXTERNAL_DIR}/yosys/share/include
)
list(APPEND EXTERNAL_LINKS Yosys)
