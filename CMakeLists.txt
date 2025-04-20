# ======================================== CMAKE SETUP ========================================
cmake_minimum_required(VERSION 3.28)

# generate compile commands json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set c++ standards as global variables
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Build Config
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
project(Gatality)

# Set directories
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")

# Find source files
file(GLOB_RECURSE PROJECT_SOURCES
		"${SOURCE_DIR}/*.cpp"
		"${SOURCE_DIR}/*.ui"
		"${SOURCE_DIR}/*.qrc"
)

option(GATALITY_BUILD_APP "Build Gatality App" ON)
option(GATALITY_BUILD_TESTS "Build Gatality Tests" OFF)

# ======================================= SHARED DEPENDENCY SETUP ========================================

# CPPLocate (they have extreme cmake goofyness)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(OPTION_BUILD_TESTS OFF) # these are cpplocate's tests
add_subdirectory(external/cpplocate EXCLUDE_FROM_ALL)

# JSON
set(GATALITY_JSON_LIB_NAME "json")
add_library(${GATALITY_JSON_LIB_NAME} INTERFACE)
target_include_directories(${GATALITY_JSON_LIB_NAME} INTERFACE "${EXTERNAL_DIR}/json/")

if (GATALITY_BUILD_APP)

	# ======================================= APP DEPENDENCY SETUP =======================================
	# Vulkan
	find_package(Vulkan REQUIRED COMPONENTS glslc)

	# SDL
	if (GATALITY_BUILD_TESTS) # hack to allow SDL to build without window system on linux
		set(SDL_UNIX_CONSOLE_BUILD ON)
	endif()
	add_subdirectory("${EXTERNAL_DIR}/SDL" SYSTEM EXCLUDE_FROM_ALL)
	add_subdirectory("${EXTERNAL_DIR}/SDL_image" SYSTEM EXCLUDE_FROM_ALL)

	# STB Image
	set(GATALITY_STB_IMAGE_LIB_NAME "stb_image")
	add_library(${GATALITY_STB_IMAGE_LIB_NAME} INTERFACE)
	target_include_directories(${GATALITY_STB_IMAGE_LIB_NAME} INTERFACE "${EXTERNAL_DIR}/stb/")

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
	set(EXTERNAL_LINKS
		${Vulkan_LIBRARIES}
		freetype
		RmlUi::RmlUi
		RmlUi::Debugger
		SDL3::SDL3
		vk-bootstrap::vk-bootstrap
		cpplocate::cpplocate
		VulkanMemoryAllocator
		glm
		${GATALITY_JSON_LIB_NAME}
		${GATALITY_STB_IMAGE_LIB_NAME}
	)

	add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
	target_include_directories(${PROJECT_NAME} PRIVATE ${SOURCE_DIR} ${Vulkan_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} PRIVATE ${EXTERNAL_LINKS})
	target_precompile_headers(${PROJECT_NAME} PRIVATE "${SOURCE_DIR}/precompiled.h")

	# Platform specific business after add_executable
	if(APPLE) # MacOS
		set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE TRUE)

		# Bundle Properties 
		set_target_properties(${PROJECT_NAME} PROPERTIES
			MACOSX_BUNDLE TRUE
			# indentification
			MACOSX_BUNDLE_BUNDLE_NAME "Gatality"
			MACOSX_BUNDLE_GUI_IDENTIFIER "com.logic-graph-creator.Gatality"
			MACOSX_BUNDLE_NAME "Gatality"
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
		set(copied_resource "${CMAKE_CURRENT_BINARY_DIR}/resources/${resource_path_relative}")
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
	set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/resources/shaders")
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
	
endif()

# ==================================== CREATE TESTING EXECUTABLE ============================================
if (GATALITY_BUILD_TESTS)

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
		${GATALITY_JSON_LIB_NAME}
	)
	add_executable(${PROJECT_NAME}_tests ${TEST_FILES})
	target_include_directories(${PROJECT_NAME}_tests PRIVATE ${SOURCE_DIR} ${TEST_DIR})
	target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${EXTERNAL_LINKS_TESTS})
	target_precompile_headers(${PROJECT_NAME}_tests PRIVATE "${SOURCE_DIR}/precompiled.h")
endif()
