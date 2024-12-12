cmake_minimum_required(VERSION 3.16)
project(LogicGraphCreator)

# Set directories
set(FORMS_DIR "${CMAKE_SOURCE_DIR}/src/gui/resources")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

# Set Qt moc and uic
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${FORMS_DIR})

# Initialize QT
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Find source files
set(SOURCE_FILES)
file(GLOB_RECURSE SOURCE_FILES
        "${FORMS_DIR}/*.ui"
        "${FORMS_DIR}/*.qrc"
        "${SOURCE_DIR}/*.cpp"
)

file(GLOB_RECURSE IMG_SOURCES "${FORMS_DIR}/*.png" "${FORMS_DIR}/*.ico")

if(APPLE) # MacOS specific (Before add_executable)
    # Icon
    set(ICON_PATH "${FORMS_DIR}/gateIcon.icns")
    set(MACOSX_BUNDLE_ICON_FILE "gateIcon.icns")
    set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    list(APPEND SOURCE_FILES ${ICON_PATH})
elseif (WIN32) # Windows specific (Before add_executable)
    # Icon
    set(ICON_PATH "${FORMS_DIR}/icon.rc")
    list(APPEND SOURCE_FILES ${ICON_PATH})
endif()

# Add executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${SOURCE_DIR} ${FORMS_DIR} )
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

if(APPLE) # MacOS specific (After add_executable)
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE TRUE)
elseif (WIN32) # Windows specific (After add_executable)
    if (CMAKE_BUILD_TYPE MATCHES Release) # If release build
        # Set WIN32_EXECUTABLE (Disables Console)
        set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    endif ()
endif()

# Add resources
qt_add_resources(${PROJECT_NAME} "images"
        PREFIX
        "/"
        FILES
        ${IMG_SOURCES}
        BASE ${FORMS_DIR}
)
