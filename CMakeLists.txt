cmake_minimum_required(VERSION 3.16)
project(LogicGraphCreator)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set directories
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(UI_DIR "${CMAKE_SOURCE_DIR}/src")
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")

# Set Qt moc and uic
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${UI_DIR})

# Initialize QT
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Find source files
set(SOURCE_FILES)
file(GLOB_RECURSE SOURCE_FILES
        "${UI_DIR}/*.ui"
        "${UI_DIR}/*.qrc"
        "${SOURCE_DIR}/*.cpp"
)

file(GLOB_RECURSE IMG_SOURCES "${RESOURCES_DIR}/*.png" "${RESOURCES_DIR}/*.ico")
file(GLOB_RECURSE JSON_SOURCES "${RESOURCES_DIR}/*.json")

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
target_include_directories(${PROJECT_NAME} PRIVATE ${SOURCE_DIR} ${UI_DIR} )
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
target_precompile_headers(${PROJECT_NAME} PRIVATE "${SOURCE_DIR}/precompiled.h")

if(APPLE) # MacOS specific (After add_executable)
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE TRUE)
elseif (WIN32) # Windows specific (After add_executable)
    if (CMAKE_BUILD_TYPE MATCHES Release) # If release build
        # Set WIN32_EXECUTABLE (Disables Console)
        set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    endif ()
endif()

# Add resources
qt_add_resources(${PROJECT_NAME} "mainResources"
        PREFIX
        "/"
        FILES
        ${IMG_SOURCES}
        ${JSON_SOURCES}
        BASE ${RESOURCES_DIR}
)
