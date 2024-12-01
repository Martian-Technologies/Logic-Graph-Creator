cmake_minimum_required(VERSION 3.16)
project(LogicGraphCreator)

find_package(Qt6
    REQUIRED COMPONENTS Core Gui Widgets
)

qt_standard_project_setup()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/gui/resources)

file(GLOB_RECURSE SOURCES "*.cpp")

# qt_add_executable(${PROJECT_NAME} ${SOURCES})

if(WIN32)
    set(ICON_PATH "${CMAKE_SOURCE_DIR}/gui/resources/gateIcon.ico")
    set(RESOURCE_FILE "${CMAKE_BINARY_DIR}/icon.rc")
    file(WRITE ${RESOURCE_FILE} "IDI_ICON1 ICON \"${ICON_PATH}\"")
    qt_add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${RESOURCE_FILE})
endif()

# Set icon for macOS
if(APPLE)
    set(ICON_PATH ${CMAKE_SOURCE_DIR}/gui/resources/gateIcon.icns)
    set(MACOSX_BUNDLE_ICON_FILE "gateIcon.icns")
    set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    qt_add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${ICON_PATH})    
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

file(GLOB_RECURSE IMG_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.png")

qt_add_resources(${PROJECT_NAME} "images"
    PREFIX
        "/"
    FILES
        ${IMG_SOURCES}
)

install(TARGETS ${PROJECT_NAME}
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ${PROJECT_NAME}
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})