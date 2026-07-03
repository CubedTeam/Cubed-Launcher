#pragma once
#include <QStandardPaths>

inline QString get_default_game_file_path() {
    QString game_file_path{
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) +
        "/game/"};
#ifdef _WIN32
    game_file_path.append("Cubed.exe");
#else
    game_file_path.append("Cubed");
#endif
    return game_file_path;
}