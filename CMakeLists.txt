# ======================================== CMAKE SETUP ========================================
cmake_minimum_required(VERSION 3.28)

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

if (MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DDEBUG")
	# hack to avoid DNDEBUG which removes wires for some reason
	set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
	# hack to avoid DNDEBUG which removes wires for some reason
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

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

# ======================================= SHARED DEPENDENCY SETUP ========================================

# CPPLocate (they have extreme cmake goofyness)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(OPTION_BUILD_TESTS OFF) # these are cpplocate's tests
add_subdirectory("${EXTERNAL_DIR}/cpplocate" EXCLUDE_FROM_ALL)

# wasmtime
if (APPLE AND CONNECTION_MACHINE_DISTRIBUTE_APP)
	add_library(wasmtime STATIC IMPORTED GLOBAL)
	set_target_properties(wasmtime PROPERTIES
		IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib-binary/libwasmtime.a"
		INTERFACE_INCLUDE_DIRECTORIES "${EXTERNAL_DIR}/wasmtime/crates/c-api"  # where .h file is
	)
else()
	add_subdirectory("${EXTERNAL_DIR}/wasmtime/crates/c-api")
endif()

# JSON
set(CONNECTION_MACHINE_JSON_LIB_NAME "json")
add_library(${CONNECTION_MACHINE_JSON_LIB_NAME} INTERFACE)
target_include_directories(${CONNECTION_MACHINE_JSON_LIB_NAME} INTERFACE "${EXTERNAL_DIR}/json/")

# tracy
if (RUN_TRACY_PROFILER)
	message("tracy is ON")
	option(TRACY_ENABLE "" ON)
	option(TRACY_ON_DEMAND "" ON)
	add_subdirectory("${EXTERNAL_DIR}/tracy")
endif()

if (CONNECTION_MACHINE_BUILD_APP)
	# ======================================= APP DEPENDENCY SETUP =======================================
	# Find vulkan (we don't actually use the headers, we just want shaderc compiler (won't be needed when we switch to slang hopefully))
	find_package(Vulkan REQUIRED COMPONENTS glslc)

	# Volk vulkan meta loader
	add_subdirectory("${EXTERNAL_DIR}/volk" SYSTEM EXCLUDE_FROM_ALL)

	# SDL
	if (CONNECTION_MACHINE_BUILD_TESTS) # hack to allow SDL to build without window system on linux
		set(SDL_UNIX_CONSOLE_BUILD ON)
	endif()
	set(SDL_STATIC ON)
	add_subdirectory("${EXTERNAL_DIR}/SDL" SYSTEM EXCLUDE_FROM_ALL)

	# STB Image
	set(CONNECTION_MACHINE_STB_IMAGE_LIB_NAME "stb_image")
	add_library(${CONNECTION_MACHINE_STB_IMAGE_LIB_NAME} INTERFACE)
	target_include_directories(${CONNECTION_MACHINE_STB_IMAGE_LIB_NAME} INTERFACE "${EXTERNAL_DIR}/stb/")
	
	# Freetype
	add_subdirectory("${EXTERNAL_DIR}/freetype" SYSTEM EXCLUDE_FROM_ALL)
	add_library(Freetype::Freetype ALIAS freetype)

	# RmlUi
	set(RMLUI_BACKEND native)
	add_subdirectory("${EXTERNAL_DIR}/RmlUi" SYSTEM EXCLUDE_FROM_ALL)

	# VMA
	add_subdirectory("${EXTERNAL_DIR}/VulkanMemoryAllocator" SYSTEM EXCLUDE_FROM_ALL)

	# Vk Bootstrap
	add_subdirectory("${EXTERNAL_DIR}/vk-bootstrap" SYSTEM EXCLUDE_FROM_ALL)

	# GLM
	add_subdirectory("${EXTERNAL_DIR}/glm" SYSTEM EXCLUDE_FROM_ALL)
	
	# ===================================== CREATE APP EXECUTABLE ========================================
	
	# Platform specific business before add_executable
	if(APPLE) # MacOS
		# Icon
		set(ICON_PATH "${RESOURCES_DIR}/gateIcon.icns")
		set(MACOSX_BUNDLE_ICON_FILE "gateIcon.icns")
		set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
		list(APPEND PROJECT_SOURCES ${ICON_PATH})

	elseif (WIN32) # Windows
		# Icon
		set(ICON_PATH "${RESOURCES_DIR}/icon.rc")
		list(APPEND PROJECT_SOURCES ${ICON_PATH})
	endif()

	# Add executable
	if (RUN_TRACY_PROFILER)
		set(EXTERNAL_LINKS
			freetype
			RmlUi::RmlUi
			RmlUi::Debugger
			SDL3::SDL3
			volk
			vk-bootstrap::vk-bootstrap
			cpplocate::cpplocate
			VulkanMemoryAllocator
			glm
			${CONNECTION_MACHINE_JSON_LIB_NAME}
			${CONNECTION_MACHINE_STB_IMAGE_LIB_NAME}
			wasmtime
			Tracy::TracyClient
		)
	else()
		set(EXTERNAL_LINKS
			freetype
			RmlUi::RmlUi
			RmlUi::Debugger
			SDL3::SDL3
			volk
			vk-bootstrap::vk-bootstrap
			cpplocate::cpplocate
			VulkanMemoryAllocator
			glm
			${CONNECTION_MACHINE_JSON_LIB_NAME}
			${CONNECTION_MACHINE_STB_IMAGE_LIB_NAME}
			wasmtime
		)
	endif()

	add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
	target_include_directories(${PROJECT_NAME} PRIVATE ${SOURCE_DIR} PUBLIC ${Vulkan_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} PRIVATE ${EXTERNAL_LINKS})
	target_precompile_headers(${PROJECT_NAME} PRIVATE "${SOURCE_DIR}/precompiled.h")

	if (RUN_TRACY_PROFILER)
		target_compile_definitions(${PROJECT_NAME} PRIVATE VK_NO_PROTOTYPES "TRACY_PROFILER")
	else()
		target_compile_definitions(${PROJECT_NAME} PRIVATE VK_NO_PROTOTYPES)
	endif()

	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${APP_NAME}")

	# Platform specific business after add_executable
	if(APPLE) # MacOS
		# set(CMAKE_MACOSX_BUNDLE YES)

		set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE TRUE)
		# Bundle Properties 
		set_target_properties(${PROJECT_NAME} PROPERTIES
			MACOSX_BUNDLE TRUE
			# indentification
			MACOSX_BUNDLE_BUNDLE_NAME "${APP_NAME}"
			MACOSX_BUNDLE_GUI_IDENTIFIER "com.connection-machine.app"
			MACOSX_BUNDLE_NAME "${APP_NAME}"
			MACOSX_BUNDLE_COPYRIGHT "MIT License" # this needs changing
			# version info
			MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
			MACOSX_BUNDLE_VERSION "${PROJECT_VERSION}"
			MACOSX_BUNDLE_INFO_VERSION "6.0"
			# Dev Region
			MACOSX_BUNDLE_DEVELOPMENT_REGION "English"
			# Doc Types that macos can handle
			MACOSX_BUNDLE_DOCUMENT_TYPES "json;JSON File;Editor;${MACOSX_BUNDLE_GUI_IDENTIFIER}.json;txt;Text File;Editor;${MACOSX_BUNDLE_GUI_IDENTIFIER}.txt;circuit;Circuit File;Editor;${MACOSX_BUNDLE_GUI_IDENTIFIER}.cir;circuit;Circuit File;Editor;${MACOSX_BUNDLE_GUI_IDENTIFIER}.circuit;"
			# Application category
			MACOSX_BUNDLE_INFO_STRING "Logic Gate Sandbox Application"
			MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_VERSION}"
		)
		set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "")
	elseif (WIN32) # Windows
		if (CMAKE_BUILD_TYPE MATCHES Release) # If release build
			# Set WIN32_EXECUTABLE (Disables Console)
			set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
		endif ()
	endif()
	
	# ======================================= RESOURCE COPYING =========================================
	# We copy the resources directory to the location of the output executable. This is our solution for "resource installing" until we fully set up cpack solution
	
	# Get resource files relative to resources dir
	file(GLOB_RECURSE RESOURCE_FILES RELATIVE "${RESOURCES_DIR}" "${RESOURCES_DIR}/*")
	set(RESOURCE_FILES_COPIED)

	foreach(resource_path_relative IN LISTS RESOURCE_FILES)
		set(original_resource "${RESOURCES_DIR}/${resource_path_relative}")
		if(APPLE)
			set(copied_resource "${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.app/Contents/MacOS/resources/${resource_path_relative}")
		else()
			set(copied_resource "${CMAKE_CURRENT_BINARY_DIR}/resources/${resource_path_relative}")
		endif()
		get_filename_component(copied_resource_dir "${copied_resource}" DIRECTORY)

		# Custom command which makes the parent directory and copies the resource
		add_custom_command(
			OUTPUT "${copied_resource}"
			COMMAND "${CMAKE_COMMAND}" -E make_directory "${copied_resource_dir}"
			COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${original_resource}" "${copied_resource}"
			DEPENDS "${original_resource}"
			COMMENT "Copying resource: ${resource_path_relative}"
		)

		list(APPEND RESOURCE_FILES_COPIED "${copied_resource}")
	endforeach()
	
	add_custom_target(copy-resources DEPENDS ${RESOURCE_FILES_COPIED})
	add_dependencies(${PROJECT_NAME} copy-resources)

	# ===================================== SHADER COMPILATION =========================================
	if(APPLE)
		set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.app/Contents/MacOS/resources/shaders")
	else()
		set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/resources/shaders")
	endif()
	file(GLOB_RECURSE SHADER_SOURCE_FILES
		"${SOURCE_DIR}/*.vert"
		"${SOURCE_DIR}/*.frag"
	)
	set(SHADER_PRODUCTS)

	foreach(shader_source IN LISTS SHADER_SOURCE_FILES)
		cmake_path(GET shader_source FILENAME shader_name)
		set(compiled_shader "${SHADER_BINARY_DIR}/${shader_name}.spv")

		# Custom command which makes the parent directory and compiles the shader
		add_custom_command(
			OUTPUT "${compiled_shader}"
			COMMAND "${CMAKE_COMMAND}" -E make_directory "${SHADER_BINARY_DIR}"
			COMMAND Vulkan::glslc "${shader_source}" "-o" "${compiled_shader}"
			DEPENDS "${shader_source}"
			COMMENT "Compiling shader: ${shader_source}"
		)

		list(APPEND SHADER_PRODUCTS "${compiled_shader}")
	endforeach()
	
	add_custom_target(compile-shaders DEPENDS ${SHADER_PRODUCTS})
	add_dependencies(${PROJECT_NAME} compile-shaders)

	if (APPLE AND CONNECTION_MACHINE_DISTRIBUTE_APP)
		set(TEAM_ID "" CACHE STRING "The development team ID for code signing")
		execute_process(
			COMMAND bash -c "security find-identity -v -p codesigning | grep 'Developer ID Application' | sed -n 's/.*(\\(.*\\)).*/\\1/p'"
			OUTPUT_VARIABLE TEAM_ID
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		if(TEAM_ID STREQUAL "")
			message("If you are trying distribute this build you need a Developer ID Application. If not ignore this.")
		else()
			message("Found TEAM_ID")
			set(XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Manual")
			set(XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${TEAM_ID})
			set(XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Developer ID Application")
			set(XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED TRUE)

			set(CMAKE_MACOSX_BUNDLE YES)
			set_target_properties(${PROJECT_NAME} PROPERTIES
				XCODE_ATTRIBUTE_CODE_SIGN_STYLE ${XCODE_ATTRIBUTE_CODE_SIGN_STYLE}
				XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}
				XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY}
				XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED ${XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED}
				XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "--timestamp=http://timestamp.apple.com/ts01  --options=runtime,library"
				XCODE_ATTRIBUTE_CODE_SIGN_INJECT_BASE_ENTITLEMENTS "NO"
				MACOSX_BUNDLE TRUE
			)

			set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})
			set(DIRS ${CMAKE_BINARY_DIR})
			
			install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION ".")

			set(CPACK_PACKAGE_FILE_NAME "Connection-Machine-${PROJECT_VERSION}-Mac-universal")

			install(CODE "
				include(BundleUtilities)
				fixup_bundle(\"${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.app\" \"\" \"${DIRS}\")
				execute_process(COMMAND codesign --force --deep --sign ${TEAM_ID} \"${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.app\")
				execute_process(COMMAND codesign --verify --verbose=2 \"${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.app\")
				message(\"To sign the .dgm run\")
				message(\"codesign --force --verbose=2 --sign $(security find-identity -v -p codesigning | grep 'Developer ID Application' | sed -n 's/.*(\\\\(.*\\\\)).*/\\\\1/p') '${CMAKE_CURRENT_BINARY_DIR}/${CPACK_PACKAGE_FILE_NAME}.dmg'\")
				message(\"To verify the .dgm is signed run\")
				message(\"codesign --verify --verbose=2 '${CMAKE_CURRENT_BINARY_DIR}/${CPACK_PACKAGE_FILE_NAME}.dmg'\")
			")

			set(CPACK_GENERATOR "DragNDrop")
			include(CPack)
		endif()
	endif()
endif()

# ==================================== CREATE TESTING EXECUTABLE ============================================
if (CONNECTION_MACHINE_BUILD_TESTS)
	# Google Testing
	add_subdirectory("${EXTERNAL_DIR}/googletest" SYSTEM EXCLUDE_FROM_ALL)
	
	set(TEST_DIR "${CMAKE_SOURCE_DIR}/tests")
	set(TEST_FILES)
	file(GLOB_RECURSE TEST_FILES
		"${SOURCE_DIR}/backend/*.cpp"
		"${SOURCE_DIR}/computerAPI/*.cpp"
		"${SOURCE_DIR}/logging/*.cpp"
		"${SOURCE_DIR}/util/*.cpp"
		"${TEST_DIR}/*.cpp"
	)

	set(EXTERNAL_LINKS_TESTS
		cpplocate::cpplocate
		gtest gtest_main
		${CONNECTION_MACHINE_JSON_LIB_NAME}
		wasmtime

	)

	if(APPLE)
		# Link CoreFoundation explicitly on macOS
		list(APPEND EXTERNAL_LINKS_TESTS "-framework CoreFoundation")
	endif()

	add_executable(${PROJECT_NAME}_tests ${TEST_FILES})
	target_include_directories(${PROJECT_NAME}_tests PRIVATE ${SOURCE_DIR} ${TEST_DIR} "${EXTERNAL_DIR}wasmtime")
	target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${EXTERNAL_LINKS_TESTS})

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
endif()
