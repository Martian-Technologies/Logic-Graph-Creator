# ======================================== CMAKE SETUP ========================================
cmake_minimum_required(VERSION 3.16)

# generate compile commands json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set c++ standards as global variables
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# hack to avoid DNDEBUG which removes wires for some reason
if (MSVC)
	# also do it for MSVC, not sure if needed
	set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

# ======================================== PROJECT SETUP ========================================
project(Gatality)

# Set directories
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(UI_DIR "${CMAKE_SOURCE_DIR}/src")
set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")

# Find source files
file(GLOB_RECURSE PROJECT_SOURCES
		"${UI_DIR}/*.ui"
		"${UI_DIR}/*.qrc"
		"${SOURCE_DIR}/*.cpp"
)

# ============================================ OPTIONS =============================================
option(Gatality_BuildApp "Build main app" ON)
option(Gatality_BuildTests "Build tests" OFF)

# ======================================== DEPENDENCY SETUP ========================================
# Initialize QT
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${UI_DIR})
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# KDDockWidgets
set(KDDockWidgets_QT6 ON)
set(KDDockWidgets_STATIC ON)
add_subdirectory(external/KDDockWidgets)

# CPPLocate (they have extreme cmake goofyness)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(BUILD_SHARED_LIBS OFF)
set(OPTION_BUILD_TESTS OFF) # these are cpplocate's tests
add_subdirectory(external/cpplocate)


set(EXTERNAL_SOURCES
	
)
set(EXTERNAL_INCLUDES
	
)
set(EXTERNAL_LINKS
	cpplocate::cpplocate
	Qt6::Core Qt6::Gui Qt6::Widgets
	KDAB::kddockwidgets
)

# ======================================= CREATE EXECUTABLE ========================================
if (Gatality_BuildApp)
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
	add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${EXTERNAL_SOURCES})
	target_include_directories(${PROJECT_NAME} PRIVATE ${SOURCE_DIR} ${UI_DIR} ${EXTERNAL_INCLUDES} )
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
set(COPIED_RESOURCES_DIR $<TARGET_FILE_DIR:${PROJECT_NAME}>"/resources")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${RESOURCES_DIR} ${COPIED_RESOURCES_DIR}
	COMMENT "Copying resource directory"
)

endif()

# ============================================ GTESTING ============================================
if (Gatality_BuildTests)
	message("Doing Testing")
	add_subdirectory("${EXTERNAL_DIR}/googletest" SYSTEM)
	enable_testing()

	set(TEST_DIR "${CMAKE_SOURCE_DIR}/tests")
	set(TEST_FILES)
	# file(GLOB_RECURSE TEST_FILES
	# 	"${TEST_DIR}/*.cpp"
	# 	"${TEST_DIR}/*.h"
	# 	"${SOURCE_DIR}/logging/*"
	# 	"${SOURCE_DIR}/util/*"
	# )
	file(GLOB_RECURSE TEST_FILES
		# "${UI_DIR}/*.ui"
		# "${UI_DIR}/*.qrc"
		"${SOURCE_DIR}/backend/*.cpp"
		"${SOURCE_DIR}/gui/*.cpp"
		"${SOURCE_DIR}/computerAPI/*.cpp"
		"${SOURCE_DIR}/gui/*.cpp"
		"${SOURCE_DIR}/keybinds/*.cpp"
		"${SOURCE_DIR}/logging/*.cpp"
		"${SOURCE_DIR}/util/*.cpp"
		"${TEST_DIR}/*.cpp"
	)

	add_executable(${PROJECT_NAME}_tests ${TEST_FILES})

	# add_executable(${PROJECT_NAME}_tests ${TEST_FILES})
	target_include_directories(${PROJECT_NAME}_tests PRIVATE ${SOURCE_DIR} ${UI_DIR} ${TEST_DIR})
	target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${EXTERNAL_LINKS} gtest gtest_main)
	target_precompile_headers(${PROJECT_NAME}_tests PRIVATE "${SOURCE_DIR}/precompiled.h")
	
	add_test(NAME RunAllTests COMMAND ${PROJECT_NAME}_tests)
endif()
