cmake_minimum_required(VERSION 3.16)
project(LogicGraphCreator)

# hack to avoid DNDEBUG which removes wires for some reason
if (MSVC)
  # also do it for MSVC, not sure if needed
  set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

# Set directories
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")

# Set Qt moc and uic
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${SOURCE_DIR})

# Initialize QT
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
# Initialize Vulkan
find_package(Vulkan REQUIRED)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

# Find source files
set(SOURCE_FILES)
file(GLOB_RECURSE SOURCE_FILES
        "${SOURCE_DIR}/*.cpp"
        "${SOURCE_DIR}/*.ui"
        "${SOURCE_DIR}/*.qrc"
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
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${SOURCE_DIR} ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets ${Vulkan_LIBRARIES})

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
file(GLOB_RECURSE SHADERS_SOURCE
  "${SOURCE_DIR}/*.vert"
  "${SOURCE_DIR}/*.frag")
# Create shader binary directory
add_custom_target(make-shader-dir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR})
# Compile each shader
foreach(source IN LISTS SHADERS_SOURCE)
  get_filename_component(FILENAME ${source} NAME)
  add_custom_command(
    COMMAND
      ${glslc_executable}
      #      -MD -MF ${SHADER_BINARY_DIR}/${FILENAME}.d
      -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
      ${source}
    OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
    DEPENDS ${source} ${SHADER_BINARY_DIR}
    COMMENT "Compiling ${FILENAME}"
  )
  list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()
# Shader compilation dependency and directory
add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS} make-shader-dir)


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
        ${SPV_SHADERS}
        BASE ${SHADER_BINARY_DIR}
)
