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
find_package(OpenSSL 3 REQUIRED COMPONENTS Crypto SSL)

qt_standard_project_setup(REQUIRES 6.8)

add_library(CubedSemanticVersion STATIC
    src/tool/semantic_version.cpp
    src/tool/semantic_version.hpp
)
target_include_directories(CubedSemanticVersion PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(CubedSemanticVersion PUBLIC Qt6::Core)

add_library(CubedReleaseSelection STATIC
    src/tool/github_release_selector.cpp
    src/tool/github_release_selector.hpp
)
target_include_directories(CubedReleaseSelection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(CubedReleaseSelection PUBLIC CubedSemanticVersion Qt6::Core)

qt_add_executable(${PROJECT_NAME}
    src/main.cpp
)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

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
        qml/Theme.qml
        qml/Sidebar.qml
        qml/Component/Card.qml
        qml/Component/MdIcon.qml
        qml/Component/MdButton.qml
        qml/Component/MdIconButton.qml
        qml/Component/MdTextField.qml
        qml/Component/MdComboBox.qml
        qml/Component/MdSwitch.qml
        qml/Component/MdToolTip.qml
        qml/Component/MdProgressBar.qml
        qml/Component/MdDialog.qml
        qml/Component/StatusChip.qml
        qml/Component/InfoBanner.qml
        qml/Component/SettingRow.qml
        qml/Component/PageScaffold.qml
        qml/Component/SectionHeader.qml
        qml/Component/FirstLaunchDialog.qml
        qml/Component/LogCard.qml
        qml/Component/InstallPathCard.qml
        qml/Tab/Launch.qml
        qml/Tab/Setting.qml
        qml/Tab/About.qml
        qml/Tab/Multiplayer.qml
        qml/Tool/MirrorSelect.qml
        qml/Tool/CheckUpdate.qml
        qml/Tab/Manager.qml
        qml/Tool/DownloadGame.qml
        qml/Tool/FrpManagement.qml
        qml/Tool/EasyTierManagement.qml
        qml/Tool/ServiceInstallCard.qml
        qml/Tool/FrpSection.qml
        qml/Tool/EasyTierSection.qml
        qml/Tool/PublicServerSelect.qml
        qml/pages/SpecialThanks.qml
    RESOURCES
        resources/logo.png
        resources/CubedLauncher.png
        resources/MaterialSymbols-LICENSE.txt

    SOURCES
        src/game/cubed_game.cpp
        src/game/cubed_game.hpp
        src/tool/system_info.hpp
        src/settings.cpp
        src/settings.hpp
        src/tool/github_release.hpp
        src/tool/github_release.cpp
        src/tool/file_downloader.hpp
        src/tool/file_downloader.cpp
        src/launcher_update.hpp
        src/launcher_update.cpp
        src/tool/side_tool.hpp
        src/tool/mirror.hpp
        src/tool/mirror.cpp
        src/game/game_update.hpp
        src/game/game_update.cpp
        src/multiplayer/binary_service_base.hpp
        src/multiplayer/binary_service_base.cpp
        src/tool/json_cache.hpp
        src/tool/json_cache.cpp
        src/tool/github_auth.hpp
        src/tool/github_auth.cpp
        src/tool/secret_store.hpp
        src/tool/secret_store.cpp
        src/tool/identity_manager.hpp
        src/tool/identity_manager.cpp
        src/tool/backup_crypto.hpp
        src/tool/backup_crypto.cpp
        src/tool/webdav_client.hpp
        src/tool/webdav_client.cpp
        src/tool/webdav_backup_manager.hpp
        src/tool/webdav_backup_manager.cpp
        src/tool/ansi_text_formatter.hpp
        src/tool/ansi_text_formatter.cpp
        src/multiplayer/frp_manager.hpp
        src/multiplayer/frp_manager.cpp
        src/multiplayer/easytier_manager.hpp
        src/multiplayer/easytier_manager.cpp

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
        ${CMAKE_CURRENT_SOURCE_DIR}/src/game
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tool
        ${CMAKE_CURRENT_SOURCE_DIR}/src/multiplayer
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    CubedSemanticVersion
    CubedReleaseSelection
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    OpenSSL::Crypto
    OpenSSL::SSL
    qmicroz
)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE Crypt32 shell32)
endif()

include(CTest)
if (BUILD_TESTING)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Test)
    qt_add_executable(SemanticVersionTest
        tests/semantic_version_test.cpp
    )
    target_link_libraries(SemanticVersionTest PRIVATE
        CubedSemanticVersion
        Qt6::Test
    )
    add_test(NAME SemanticVersionTest COMMAND SemanticVersionTest)

    qt_add_executable(GithubReleaseSelectorTest
        tests/github_release_selector_test.cpp
    )
    target_link_libraries(GithubReleaseSelectorTest PRIVATE
        CubedReleaseSelection
        Qt6::Test
    )
    add_test(NAME GithubReleaseSelectorTest COMMAND GithubReleaseSelectorTest)

    qt_add_executable(IdentityManagerTest
        tests/identity_manager_test.cpp
        src/tool/identity_manager.cpp
        src/tool/identity_manager.hpp
    )
    target_include_directories(IdentityManagerTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_link_libraries(IdentityManagerTest PRIVATE
        Qt6::Core
        Qt6::Qml
        Qt6::Test
    )
    add_test(NAME IdentityManagerTest COMMAND IdentityManagerTest)

    qt_add_executable(BackupCryptoTest
        tests/backup_crypto_test.cpp
        src/tool/backup_crypto.cpp
        src/tool/backup_crypto.hpp
    )
    target_include_directories(BackupCryptoTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_link_libraries(BackupCryptoTest PRIVATE
        OpenSSL::Crypto
        Qt6::Core
        Qt6::Test
    )
    add_test(NAME BackupCryptoTest COMMAND BackupCryptoTest)

    qt_add_executable(WebDavClientTest
        tests/webdav_client_test.cpp
        src/tool/webdav_client.cpp
        src/tool/webdav_client.hpp
    )
    target_include_directories(WebDavClientTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_link_libraries(WebDavClientTest PRIVATE
        Qt6::Network
        Qt6::Test
    )
    add_test(NAME WebDavClientTest COMMAND WebDavClientTest)

    qt_add_executable(AnsiTextFormatterTest
        tests/ansi_text_formatter_test.cpp
        src/tool/ansi_text_formatter.cpp
        src/tool/ansi_text_formatter.hpp
    )
    target_include_directories(AnsiTextFormatterTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_link_libraries(AnsiTextFormatterTest PRIVATE
        Qt6::Gui
        Qt6::Qml
        Qt6::Test
    )
    add_test(NAME AnsiTextFormatterTest COMMAND AnsiTextFormatterTest)
    set_tests_properties(AnsiTextFormatterTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORMTHEME="
    )
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SECRET REQUIRED IMPORTED_TARGET libsecret-1)
    target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::SECRET)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE QT_NO_KEYWORDS)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

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
