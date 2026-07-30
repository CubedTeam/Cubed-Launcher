#include "settings.hpp"

#include "game_path.hpp"

#include <QFileInfo>

Settings* Settings::s_instance = nullptr;

Settings::Settings(QObject* parent)
    : QObject(parent), m_settings(QSettings::IniFormat, QSettings::UserScope,
                                  "Cubed", "Launcher") {
    load();
    if (s_instance == nullptr) {
        s_instance = this;
    }
}

QString Settings::game_dir() const { return m_game_dir; }
QString Settings::player_name() const { return m_player_name; }

bool Settings::path_set() const { return !m_game_dir.isEmpty(); }
int Settings::mirror_index() const { return m_mirror_index; }
QString Settings::language() const { return m_language; }

QColor Settings::accent_color() const { return m_accent_color; }
bool Settings::card_colorful_border() const { return m_card_colorful_border; }
QString Settings::wrapper_command() const { return m_wrapper_command; }

Settings* Settings::instance() { return s_instance; }

void Settings::set_game_dir_url(const QUrl& path) {
    QString local = path.toLocalFile();
    if (!update_value(m_game_dir, local, "game_path")) {
        return;
    }

    emit game_dir_changed();
    emit path_set_changed();
}

void Settings::set_game_dir(const QString& path) {
    if (!update_value(m_game_dir, path, "game_path")) {
        return;
    }

    emit game_dir_changed();
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

void Settings::set_language(const QString& lang) {
    if (!update_value(m_language, lang, "language")) {
        return;
    }

    emit language_changed();
}

void Settings::set_accent_color(const QColor& color) {
    if (m_accent_color == color) {
        return;
    }

    m_accent_color = color;
    m_settings.setValue("accent_color", color.name());
    emit accent_color_changed();
}

void Settings::set_card_colorful_border(bool enabled) {
    if (!update_value(m_card_colorful_border, enabled,
                      "card_colorful_border")) {
        return;
    }

    emit card_colorful_border_changed();
}

void Settings::set_wrapper_command(const QString& command) {
    if (!update_value(m_wrapper_command, command, "wrapper_command")) {
        return;
    }

    emit wrapper_command_changed();
}

void Settings::load() {
    if (m_settings.contains("game_path")) {
        QString raw = m_settings.value("game_path").toString();
        // Migration: older versions stored the full file path. If the
        // basename matches the platform executable, strip it so the stored
        // value is the install directory.
        if (!raw.isEmpty()) {
            QFileInfo info(raw);
            if (info.fileName() == get_default_game_executable_name()) {
                raw = info.absolutePath();
            }
        }
        m_game_dir = raw;
    }
    qDebug() << "Settings Game Dir {" << m_game_dir << "}" << " empty "
             << m_game_dir.isEmpty();

    m_player_name = m_settings.value("player_name").toString();

    m_mirror_index = m_settings.value("mirror_index", -1).toInt();

    m_language = m_settings.value("language").toString();
    m_accent_color = QColor(
        m_settings.value("accent_color", QColor("#2196F3").name()).toString());
    m_card_colorful_border =
        m_settings.value("card_colorful_border", true).toBool();
    m_wrapper_command = m_settings.value("wrapper_command").toString();
}
void Settings::save(const QString& key, const QString& value) {
    m_settings.setValue(key, value);
}
