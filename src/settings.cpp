#include "settings.hpp"

Settings* Settings::s_instance = nullptr;

Settings::Settings(QObject* parent)
    : QObject(parent), m_settings(QSettings::IniFormat, QSettings::UserScope,
                                  "Cubed", "Launcher") {
    load();
    if (s_instance == nullptr) {
        s_instance = this;
    }
}

QString Settings::game_path() const { return m_game_path; }
QString Settings::player_name() const { return m_player_name; }

bool Settings::path_set() const { return !m_game_path.isEmpty(); }
int Settings::mirror_index() const { return m_mirror_index; }
QString Settings::language() const { return m_language; }

QColor Settings::accent_color() const { return m_accent_color; }
bool Settings::card_colorful_border() const { return m_card_colorful_border; }

Settings* Settings::instance() { return s_instance; }

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

void Settings::load() {
    if (m_settings.contains("game_path")) {
        m_game_path = m_settings.value("game_path").toString();
    }
    qDebug() << "Settings Game Path {" << m_game_path << "}" << " empty "
             << m_game_path.isEmpty();

    m_player_name = m_settings.value("player_name").toString();

    m_mirror_index = m_settings.value("mirror_index", -1).toInt();

    m_language = m_settings.value("language").toString();
    m_accent_color = QColor(
        m_settings.value("accent_color", QColor("#2196F3").name()).toString());
    m_card_colorful_border =
        m_settings.value("card_colorful_border", true).toBool();
}
void Settings::save(const QString& key, const QString& value) {
    m_settings.setValue(key, value);
}