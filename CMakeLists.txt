# ======================================== CMAKE SETUP ========================================
cmake_minimum_required(VERSION 3.16)

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
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")

# Find source files
file(GLOB_RECURSE PROJECT_SOURCES
		"${UI_DIR}/*.ui"
		"${UI_DIR}/*.qrc"
		"${SOURCE_DIR}/*.cpp"
)

# ======================================== DEPENDENCY SETUP ========================================
# Initialize QT
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${UI_DIR})
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# KDDockWidgets
set(KDDockWidgets_QT6 ON)
add_subdirectory(external/KDDockWidgets)

set(EXTERNAL_SOURCES
	
)
set(EXTERNAL_INCLUDES
	
)
set(EXTERNAL_LINKS
	Qt6::Core Qt6::Gui Qt6::Widgets
	KDAB::kddockwidgets
)

# ======================================== CREATE EXECUTABLE ========================================
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
elseif (WIN32) # Windows
	if (CMAKE_BUILD_TYPE MATCHES Release) # If release build
		# Set WIN32_EXECUTABLE (Disables Console)
		set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
	endif ()
endif()

# ======================================== RESOURCE EMBEDDING ========================================

# find resources
file(GLOB_RECURSE IMG_SOURCES "${RESOURCES_DIR}/*.png" "${RESOURCES_DIR}/*.ico")
file(GLOB_RECURSE JSON_SOURCES "${RESOURCES_DIR}/*.json")

# add resources to QT
qt_add_resources(${PROJECT_NAME} "mainResources"
		PREFIX
		"/"
		FILES
		${IMG_SOURCES}
		${JSON_SOURCES}
		BASE ${RESOURCES_DIR}
)
