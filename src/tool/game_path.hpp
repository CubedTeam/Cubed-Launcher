#pragma once
#include <QStandardPaths>
#include <QString>

inline QString get_default_game_install_dir() {
    return QStandardPaths::writableLocation(
               QStandardPaths::AppLocalDataLocation) +
           "/game";
}

inline QString get_default_game_executable_name() {
#ifdef _WIN32
    return "Cubed.exe";
#else
    return "Cubed";
#endif
}

inline QString get_default_game_file_path() {
    return get_default_game_install_dir() + "/" +
           get_default_game_executable_name();
}

inline QString get_default_frp_install_dir() {
    return QStandardPaths::writableLocation(
               QStandardPaths::AppLocalDataLocation) +
           "/frp";
}
