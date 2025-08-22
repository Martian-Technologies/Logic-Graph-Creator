# yosys
CPMAddPackage(
	GITHUB_REPOSITORY YosysHQ/yosys
	GIT_TAG 65109822b4db80cb740e31cfbeb27ff1159acdf9
	DOWNLOAD_ONLY YES
	SOURCE_DIR "${EXTERNAL_DIR}/yosys"
)

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
list(APPEND EXTERNAL_LINKS yosys-slang)

# link yosys-slang to yosys to have it loaded as a plugin
cmake_policy(SET CMP0079 NEW)

if(APPLE)
	set(yosys_slang_link_command "-Wl,-force_load,$<TARGET_FILE:yosys-slang>")
else()
	set(yosys_slang_link_command "-Wl,--push-state,--whole-archive $<TARGET_FILE:yosys-slang> -Wl,--pop-state")
endif()

# build yosys
execute_process(
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG := clang" > ${EXTERNAL_DIR}/yosys/Makefile.conf
    COMMAND ${CMAKE_COMMAND} -E echo "LINKFLAGS += ${yosys_slang_link_command}" >> ${EXTERNAL_DIR}/yosys/Makefile.conf
    COMMAND ${CMAKE_COMMAND} -E echo "ENABLE_TCL := 0" >> ${EXTERNAL_DIR}/yosys/Makefile.conf
    COMMAND ${CMAKE_COMMAND} -E echo "ENABLE_ABC := 0" >> ${EXTERNAL_DIR}/yosys/Makefile.conf
    COMMAND ${CMAKE_COMMAND} -E echo "ENABLE_GLOB := 1" >> ${EXTERNAL_DIR}/yosys/Makefile.conf
    COMMAND ${CMAKE_COMMAND} -E echo "ENABLE_PLUGINS := 1" >> ${EXTERNAL_DIR}/yosys/Makefile.conf
    COMMAND ${CMAKE_COMMAND} -E echo "ENABLE_READLINE := 0" >> ${EXTERNAL_DIR}/yosys/Makefile.conf
    COMMAND ${CMAKE_COMMAND} -E echo "ENABLE_VERIFIC_SYSTEMVERILOG := 0" >> ${EXTERNAL_DIR}/yosys/Makefile.conf
    COMMAND ${CMAKE_COMMAND} -E echo "ENABLE_VERIFIC_EDIF := 0" >> ${EXTERNAL_DIR}/yosys/Makefile.conf
    COMMAND ${CMAKE_COMMAND} -E echo "ENABLE_VERIFIC_LIBERTY := 0" >> ${EXTERNAL_DIR}/yosys/Makefile.conf
    COMMAND ${CMAKE_COMMAND} -E echo "ENABLE_COVER := 1" >> ${EXTERNAL_DIR}/yosys/Makefile.conf
    COMMAND ${CMAKE_COMMAND} -E echo "ENABLE_LIBYOSYS := 1" >> ${EXTERNAL_DIR}/yosys/Makefile.conf
    COMMAND ${CMAKE_COMMAND} -E echo "DISABLE_SPAWN := 1" >> ${EXTERNAL_DIR}/yosys/Makefile.conf
    COMMAND ${CMAKE_COMMAND} -E echo "ENABLE_ZLIB := 1" >> ${EXTERNAL_DIR}/yosys/Makefile.conf
	COMMAND make share
    WORKING_DIRECTORY ${EXTERNAL_DIR}/yosys
)

add_custom_target(build_yosys ALL
    BYPRODUCTS "${EXTERNAL_DIR}/yosys/libyosys.so"
    COMMAND make -j8 libyosys.so
    COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id "${EXTERNAL_DIR}/yosys/libyosys.so" "${EXTERNAL_DIR}/yosys/libyosys.so"
    WORKING_DIRECTORY ${EXTERNAL_DIR}/yosys
	COMMENT "Building Yosys..."
	USES_TERMINAL
)

# link it together
add_library(Yosys SHARED IMPORTED)
add_dependencies(Yosys build_yosys)
add_compile_definitions(YOSYS_DISABLE_SPAWN)
add_compile_definitions(_YOSYS_)
set_target_properties(Yosys PROPERTIES
	IMPORTED_LOCATION "${EXTERNAL_DIR}/yosys/libyosys.so"
	INTERFACE_INCLUDE_DIRECTORIES ${EXTERNAL_DIR}/yosys/share/include
)
list(APPEND EXTERNAL_LINKS Yosys)
