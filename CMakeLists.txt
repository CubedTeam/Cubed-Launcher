cmake_minimum_required(VERSION 3.21)

project(CubedLauncher LANGUAGES C CXX)

if(NOT DEFINED APP_VERSION)
    set(APP_VERSION "dev")
endif()
message(STATUS "App Version: ${APP_VERSION}")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CONFIGURATION_TYPES
   AND NOT CMAKE_BUILD_TYPE)

    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)

endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_AUTOMOC ON)

configure_file(
    src/version.hpp.in
    ${CMAKE_BINARY_DIR}/generated/version.hpp
    @ONLY
)

add_subdirectory(third_party/qmicroz)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Quick QuickControls2 Network LinguistTools)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(${PROJECT_NAME}
    src/main.cpp
)


if (WIN32)
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            VERSION ${APP_VERSION}
            WIN32_EXECUTABLE $<CONFIG:Release>
    )
endif()


qt_add_qml_module(${PROJECT_NAME}
    URI ${PROJECT_NAME}
    VERSION 1.0
    QML_FILES 
        qml/Main.qml
        qml/Sidebar.qml
        qml/Tab/Launch.qml
        qml/Tab/Setting.qml
        qml/Tab/About.qml
        qml/Tool/GamePathSet.qml
        qml/Tool/MirrorSelect.qml
        qml/Tool/CheckUpdate.qml
        qml/Tab/Manager.qml
        qml/Tool/DownloadGame.qml
    RESOURCES
        resources/logo.png

    SOURCES
        src/game-mgmt/cubed_instance.cpp
        src/game-mgmt/cubed_instance.hpp
        src/tool/system_info.hpp
        src/settings.cpp
        src/settings.hpp
        src/tool/launcher_update.hpp
        src/tool/launcher_update.cpp
        src/tool/side_tool.hpp
        src/tool/mirror.hpp
        src/tool/mirror.cpp
        src/tool/game_update.hpp
        src/tool/game_update.cpp
    
)

qt_add_translations(${PROJECT_NAME}
    TS_FILES
        i18n/CubedLauncher_zh_CN.ts
)


if (MSVC)
target_sources(${PROJECT_NAME} PRIVATE CubedLauncher.rc)
endif()

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_BINARY_DIR}/generated
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/game-mgmt
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tool
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    qmicroz
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    FILES resources/CubedLauncher.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(
    FILES resources/CubedLauncher.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
)

endif()