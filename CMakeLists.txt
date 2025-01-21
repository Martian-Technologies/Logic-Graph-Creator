cmake_minimum_required(VERSION 3.16)
set(CXX_STANDARD 20)
project(Gatality)

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

# Set directories
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(LIBRARY_DIR "${CMAKE_SOURCE_DIR}/lib")
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")

# Set Qt moc and uic
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${SOURCE_DIR})

# Initialize QT
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
# Initialize Vulkan
find_package(Vulkan REQUIRED COMPONENTS glslc)

# Find source files
set(SOURCE_FILES)
file(GLOB_RECURSE SOURCE_FILES
		"${SOURCE_DIR}/*.cpp"
		"${SOURCE_DIR}/*.ui"
		"${SOURCE_DIR}/*.qrc"
)

set(LIB_SOURCE

)
set(LIB_INCLUDE
	${Vulkan_INCLUDE_DIRS}
)

file(GLOB_RECURSE IMG_SOURCES "${RESOURCES_DIR}/*.png" "${RESOURCES_DIR}/*.ico")
file(GLOB_RECURSE JSON_SOURCES "${RESOURCES_DIR}/*.json")
file(GLOB_RECURSE SHADER_SOURCES "")

if(APPLE) # MacOS specific (Before add_executable)
	# Icon
	set(ICON_PATH "${RESOURCES_DIR}/gateIcon.icns")
	set(MACOSX_BUNDLE_ICON_FILE "gateIcon.icns")
	set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	list(APPEND SOURCE_FILES ${ICON_PATH})
elseif (WIN32) # Windows specific (Before add_executable)
	# Icon
	set(ICON_PATH "${RESOURCES_DIR}/icon.rc")
	list(APPEND SOURCE_FILES ${ICON_PATH})
endif()


# Add executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${LIB_SOURCE})
target_include_directories(${PROJECT_NAME} PRIVATE ${SOURCE_DIR} ${LIB_INCLUDE})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets ${Vulkan_LIBRARIES})
target_precompile_headers(${PROJECT_NAME} PRIVATE "${SOURCE_DIR}/precompiled.h")

if(APPLE) # MacOS specific (After add_executable)
	set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE TRUE)
elseif (WIN32) # Windows specific (After add_executable)
	if (CMAKE_BUILD_TYPE MATCHES Release) # If release build
		# Set WIN32_EXECUTABLE (Disables Console)
		set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
	endif ()
endif()


# Compile shaders -----
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
file(GLOB_RECURSE SHADER_SOURCE_FILES
	"${SOURCE_DIR}/*.vert"
	"${SOURCE_DIR}/*.frag")

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

# Add resources ------
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
