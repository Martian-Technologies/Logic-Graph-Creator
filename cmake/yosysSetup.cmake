# yosys
CPMAddPackage(
	NAME yosys_shared
	GITHUB_REPOSITORY ItchyTrack/yosys
	GIT_TAG c84e773982d4fba951c033e40d6bb8edfa5efeca
	OPTIONS
		"ENABLE_LIBYOSYS ON"
		"ENABLE_READLINE OFF"
		"DISABLE_SPAWN ON"
		"ENABLE_ABC OFF"
		"LINK_ABC OFF"
	SOURCE_DIR "${EXTERNAL_DIR}/yosys"
)
target_compile_options(yosys_shared PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>)
list(APPEND EXTERNAL_LINKS yosys_shared)
file(WRITE "${CMAKE_BINARY_DIR}/cmake/FindYosys.cmake"
	"include(CMakeFindDependencyMacro)
	add_library(yosys::yosys INTERFACE IMPORTED)
	target_include_directories(yosys::yosys INTERFACE
		${EXTERNAL_DIR}/yosys
	)
	add_compile_definitions(_YOSYS_)
	set(YOSYS_BINDIR ${YOSYS_BINDIR} PARENT_SCOPE)
	set(YOSYS_DATDIR ${CMAKE_BINARY_DIR}/yosys_plugins PARENT_SCOPE)\n"
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

# link yosys-slang to yosys to have it loaded as a plugin
cmake_policy(SET CMP0079 NEW)
set(yosys_slang_binary "${CMAKE_BINARY_DIR}/libyosys-slang.a")
# get_target_property(yosys_link_flags yosys_shared LINK_FLAGS)
if(APPLE)
	target_link_libraries(yosys_shared PRIVATE "-Wl,-force_load,$<TARGET_FILE:yosys-slang>")
else()
	target_link_libraries(yosys_shared PRIVATE "-Wl,--push-state,--whole-archive $<TARGET_FILE:yosys-slang> -Wl,--pop-state")
endif()