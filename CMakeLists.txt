# ======================================== CMAKE SETUP ========================================
cmake_minimum_required(VERSION 3.16)

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

# ============================================ OPTIONS =============================================
option(Gatality_BuildApp "Build main app" ON)
option(Gatality_BuildTests "Build tests" OFF)

# ======================================== DEPENDENCY SETUP ========================================
# Initialize QT
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${SOURCE_DIR})
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Initialize Vulkan
find_package(Vulkan REQUIRED COMPONENTS glslc)
add_subdirectory("${EXTERNAL_DIR}/VulkanMemoryAllocator" SYSTEM) # and vma
# GLM
add_subdirectory("${EXTERNAL_DIR}/glm" SYSTEM)

# KDDockWidgets
set(KDDockWidgets_QT6 ON)
set(KDDockWidgets_STATIC ON)
add_subdirectory(external/KDDockWidgets)

set(EXTERNAL_SOURCES
	
)
set(EXTERNAL_INCLUDES
	${Vulkan_INCLUDE_DIRS}
)
set(EXTERNAL_LINKS
	Qt6::Core Qt6::Gui Qt6::Widgets
	KDAB::kddockwidgets
	${Vulkan_LIBRARIES}
	VulkanMemoryAllocator
	glm
)
set(EXTERNAL_DEFINES
	"GLM_FORCE_RADIANS"
	"GLM_FORCE_DEPTH_ZERO_TO_ONE"
)

if (Gatality_BuildApp)
# ======================================= CREATE EXECUTABLE ========================================
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

# ===================================== SHADER COMPILATION =========================================
	set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
	file(GLOB_RECURSE SHADER_SOURCE_FILES
		"${SOURCE_DIR}/*.vert"
		"${SOURCE_DIR}/*.frag"
	)

	set(SHADER_COMMANDS)
	set(SHADER_PRODUCTS)

	foreach(SHADER_SOURCE IN LISTS SHADER_SOURCE_FILES)
		cmake_path(GET SHADER_SOURCE FILENAME SHADER_NAME)
	
		# Build command
		list(APPEND SHADER_COMMANDS COMMAND)
		list(APPEND SHADER_COMMANDS Vulkan::glslc)
		list(APPEND SHADER_COMMANDS "${SHADER_SOURCE}")
		list(APPEND SHADER_COMMANDS "-o")
		list(APPEND SHADER_COMMANDS "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.spv")
	
		# Add product
		list(APPEND SHADER_PRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.spv")
	endforeach()

	add_custom_target("Shaders" ALL
		${SHADER_COMMANDS}
		COMMENT "Compiling Shaders"
		SOURCES ${SHADER_SOURCE_FILES}
		BYPRODUCTS ${SHADER_PRODUCTS}
	)
	
# ======================================= RESOURCE EMBEDDING =======================================
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

	# Add compiled shader to qt
	qt_add_resources(${PROJECT_NAME} "shaders"
			PREFIX
			"/shaders"
			FILES
			${SHADER_PRODUCTS}
			BASE ${CMAKE_CURRENT_BINARY_DIR}
	)

endif() # Gatality_BuildApp

if (Gatality_BuildTests)
# ============================================ GTESTING ============================================
	message("HI")
	add_subdirectory("${EXTERNAL_DIR}/googletest" SYSTEM)
	enable_testing()

	set(TEST_DIR "${CMAKE_SOURCE_DIR}/tests")
	set(TEST_FILES)
	file(GLOB_RECURSE TEST_FILES
		"${TEST_DIR}/*.cpp"
		"${TEST_DIR}/*.h"
		"${SOURCE_DIR}/backend/*"
	)

	add_executable(${PROJECT_NAME}_tests ${TEST_FILES})
	target_include_directories(${PROJECT_NAME}_tests PRIVATE ${SOURCE_DIR} ${TEST_DIR})
	target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${EXTERNAL_LINKS} gtest gtest_main)
	target_precompile_headers(${PROJECT_NAME}_tests PRIVATE "${SOURCE_DIR}/precompiled.h")
	
	add_test(NAME RunAllTests COMMAND ${PROJECT_NAME}_tests)
	
endif() # Gatality_BuildTests
