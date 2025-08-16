add_main_dependencies()

# Google Testing

CPMAddPackage(
	NAME gtest
	GITHUB_REPOSITORY google/googletest
	GIT_TAG v1.17.0
	SOURCE_DIR "${EXTERNAL_DIR}/gtest"
)

set(TEST_DIR "${CMAKE_SOURCE_DIR}/tests")
set(TEST_FILES)
file(GLOB_RECURSE TEST_FILES
	"${SOURCE_DIR}/backend/*.cpp"
	"${SOURCE_DIR}/computerAPI/*.cpp"
	"${SOURCE_DIR}/logging/*.cpp"
	"${SOURCE_DIR}/util/*.cpp"
	"${TEST_DIR}/*.cpp"
)

set(EXTERNAL_LINKS ${EXTERNAL_LINKS} gtest gtest_main)

message("EXTERNAL_LINKS: ${EXTERNAL_LINKS}")

if(APPLE)
	# Link CoreFoundation explicitly on macOS
	list(APPEND EXTERNAL_LINKS "-framework CoreFoundation")
endif()

add_executable(${PROJECT_NAME}_tests ${TEST_FILES})
target_include_directories(${PROJECT_NAME}_tests PRIVATE ${SOURCE_DIR} ${TEST_DIR} "${EXTERNAL_DIR}/wasmtime")
target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${EXTERNAL_LINKS})

if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
target_compile_options(${PROJECT_NAME}_tests PRIVATE --coverage -O0 -g)
target_link_options(${PROJECT_NAME}_tests PRIVATE --coverage)
endif()

target_precompile_headers(${PROJECT_NAME}_tests PRIVATE "${SOURCE_DIR}/precompiled.h")

if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	find_program(GCOVR_PATH gcovr)
	if(NOT GCOVR_PATH)
		message(FATAL_ERROR "gcovr not found! Please install it via pip: pip install gcovr")
	endif()

	add_custom_target(coverage
		COMMAND ${GCOVR_PATH}
			--root ${CMAKE_SOURCE_DIR}
			--filter ${SOURCE_DIR}
			--exclude-unreachable-branches
			--print-summary
			--html --html-details -o coverage.html
			--gcov-executable gcov
			--exclude '${EXTERNAL_DIR}'
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		COMMENT "Generating code coverage report with gcovr"
	)
endif()