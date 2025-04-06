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

option(GATALITY_BUILD_TESTS OFF) # these are cpplocate's tests

# ======================================== DEPENDENCY SETUP ========================================

# Vulkan
find_package(Vulkan REQUIRED COMPONENTS glslc)

# SDL
add_subdirectory("${EXTERNAL_DIR}/SDL" SYSTEM EXCLUDE_FROM_ALL)

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

# CPPLocate (they have extreme cmake goofyness)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(OPTION_BUILD_TESTS OFF) # these are cpplocate's tests
add_subdirectory(external/cpplocate EXCLUDE_FROM_ALL)

# JSON
set(GATALITY_JSON_LIB_NAME "json")
add_library(${GATALITY_JSON_LIB_NAME} INTERFACE)
target_include_directories(${GATALITY_JSON_LIB_NAME} INTERFACE "${EXTERNAL_DIR}/json/")

# Google Testing
add_subdirectory("${EXTERNAL_DIR}/googletest" SYSTEM EXCLUDE_FROM_ALL)

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
	vk-bootstrap::vk-bootstrap
	freetype
	RmlUi::RmlUi
	SDL3::SDL3
	cpplocate::cpplocate
	VulkanMemoryAllocator
	glm
	${GATALITY_JSON_LIB_NAME}
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

# ==================================== CREATE TESTING EXECUTABLE ============================================
if (GATALITY_BUILD_TESTS)
	set(TEST_DIR "${CMAKE_SOURCE_DIR}/tests")
	set(TEST_FILES)
	file(GLOB_RECURSE TEST_FILES
		"${SOURCE_DIR}/backend/*.cpp"
		"${SOURCE_DIR}/computerAPI/*.cpp"
		"${SOURCE_DIR}/keybinds/*.cpp"
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

# ======================================= RESOURCE COPYING =========================================
# We copy the resources directory to the location of the output executable. This is our solution for "resource installing" until we fully set up cpack solution
set(COPIED_RESOURCES_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${RESOURCES_DIR}" "${COPIED_RESOURCES_DIR}"
	COMMENT "Copying resource directory"
)

# ===================================== SHADER COMPILATION =========================================
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
file(GLOB_RECURSE SHADER_SOURCE_FILES
	"${SOURCE_DIR}/*.vert"
	"${SOURCE_DIR}/*.frag"
)

# list of commands to execute to compile all shaders
set(SHADER_COMMANDS)
set(SHADER_PRODUCTS)
list(APPEND SHADER_COMMANDS COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR})

foreach(SHADER_SOURCE IN LISTS SHADER_SOURCE_FILES)
	cmake_path(GET SHADER_SOURCE FILENAME SHADER_NAME)
	
	# Build command
	list(APPEND SHADER_COMMANDS COMMAND)
	list(APPEND SHADER_COMMANDS Vulkan::glslc)
	list(APPEND SHADER_COMMANDS "${SHADER_SOURCE}")
	list(APPEND SHADER_COMMANDS "-o")
	list(APPEND SHADER_COMMANDS "${SHADER_BINARY_DIR}/${SHADER_NAME}.spv")
	
	# Add product
	list(APPEND SHADER_PRODUCTS "${SHADER_BINARY_DIR}/${SHADER_NAME}.spv")
endforeach()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	${SHADER_COMMANDS}
	DEPENDS ${SHADER_SOURCE_FILES}
	BYPRODUCTS ${SHADER_PRODUCTS}
	COMMENT "Compiling Shaders"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${SHADER_BINARY_DIR} "${COPIED_RESOURCES_DIR}/shaders"
	COMMENT "Copying shader directory"
)
