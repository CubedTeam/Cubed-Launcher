#include "settings.hpp"

Settings::Settings(QObject* parent)
    : QObject(parent), m_settings(QSettings::IniFormat, QSettings::UserScope,
                                  "Cubed", "Launcher") {
    load();
}

QString Settings::game_path() const { return m_game_path; }
QString Settings::player_name() const { return m_player_name; }

bool Settings::path_set() const { return !m_game_path.isEmpty(); }

void Settings::set_game_path(const QUrl& path) {
    QString local = path.toLocalFile();
    if (!update_value(m_game_path, local, "game_path")) {
        return;
    }

    emit game_path_changed();
    emit path_set_changed();
}

void Settings::set_player_name(const QString& name) {
    if (!update_value(m_player_name, name, "player_name")) {
        return;
    }
    emit player_name_changed();
}

void Settings::load() {
    m_game_path = m_settings.value("game_path").toString();
    m_player_name = m_settings.value("player_name").toString();
}
void Settings::save(const QString& key, const QString& value) {
    m_settings.setValue(key, value);
}