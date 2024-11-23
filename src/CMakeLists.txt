cmake_minimum_required(VERSION 3.16)
project(LogicGraphCreator)

find_package(Qt6
    REQUIRED COMPONENTS Core Gui Widgets
)

qt_standard_project_setup()

set(CMAKE_AUTOUIC ON)

file(GLOB SOURCES "*.cpp")
file(GLOB UIS "ui/*.ui")

qt_add_executable(${PROJECT_NAME} ${SOURCES} ${UIS})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

set(images_resource_files
    "logicTiles.png"
)

qt_add_resources(${PROJECT_NAME} "images"
    PREFIX
        "/"
    FILES
        ${images_resource_files}
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