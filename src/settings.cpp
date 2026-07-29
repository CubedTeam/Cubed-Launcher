#include "settings.hpp"

Settings::Settings(QObject* parent)
    : QObject(parent), m_settings(QSettings::IniFormat, QSettings::UserScope,
                                  "Cubed", "Launcher") {
    load();
}

QString Settings::game_path() const { return m_game_path; }
QString Settings::player_name() const { return m_player_name; }

bool Settings::path_set() const { return !m_game_path.isEmpty(); }
int Settings::mirror_index() const { return m_mirror_index; }

void Settings::set_game_path_url(const QUrl& path) {
    QString local = path.toLocalFile();
    if (!update_value(m_game_path, local, "game_path")) {
        return;
    }

    emit game_path_changed();
    emit path_set_changed();
}

void Settings::set_game_path(const QString& path) {
    if (!update_value(m_game_path, path, "game_path")) {
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

void Settings::set_mirror_index(int index) {
    if (!update_value(m_mirror_index, index, "mirror_index")) {
        return;
    }

    emit mirror_index_changed();
}

void Settings::load() {
    if (m_settings.contains("game_path")) {
        m_game_path = m_settings.value("game_path").toString();
    }
    qDebug() << "Settings Game Path {" << m_game_path << "}" << " empty "
             << m_game_path.isEmpty();

    m_player_name = m_settings.value("player_name").toString();

    // AI-generated: load persisted mirror index; -1 means "never set".
    m_mirror_index = m_settings.value("mirror_index", -1).toInt();
}
void Settings::save(const QString& key, const QString& value) {
    m_settings.setValue(key, value);
}