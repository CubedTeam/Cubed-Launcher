#include "settings.hpp"

#include "tool/json_cache.hpp"
#include "tool/log.hpp"
#include "tool/secret_store.hpp"

#include <QFileInfo>

namespace {
constexpr QLatin1StringView kGithubTokenKey("github_token");
} // namespace

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
QString Settings::frp_install_path() const { return m_frp_install_path; }
QString Settings::easytier_install_path() const {
    return m_easytier_install_path;
}
int Settings::easytier_public_server_index() const {
    return m_easytier_public_server_index;
}
QString Settings::github_token() const { return m_github_token; }

Settings* Settings::instance() { return s_instance; }

void Settings::set_game_dir_url(const QUrl& path) {
    QString local = path.toLocalFile();
    if (!update_value(m_game_dir, local, "game_path")) {
        return;
    }

    Q_EMIT game_dir_changed();
    Q_EMIT path_set_changed();
}

void Settings::set_game_dir(const QString& path) {
    if (!update_value(m_game_dir, path, "game_path")) {
        return;
    }

    Q_EMIT game_dir_changed();
    Q_EMIT path_set_changed();
}

void Settings::set_player_name(const QString& name) {
    if (!update_value(m_player_name, name, "player_name")) {
        return;
    }

    Q_EMIT player_name_changed();
}

void Settings::set_mirror_index(int index) {
    if (!update_value(m_mirror_index, index, "mirror_index")) {
        return;
    }

    Q_EMIT mirror_index_changed();
}

void Settings::set_language(const QString& lang) {
    if (!update_value(m_language, lang, "language")) {
        return;
    }

    Q_EMIT language_changed();
}

void Settings::set_accent_color(const QColor& color) {
    if (m_accent_color == color) {
        return;
    }

    m_accent_color = color;
    m_settings.setValue("accent_color", color.name());
    Q_EMIT accent_color_changed();
}

void Settings::set_card_colorful_border(bool enabled) {
    if (!update_value(m_card_colorful_border, enabled,
                      "card_colorful_border")) {
        return;
    }

    Q_EMIT card_colorful_border_changed();
}

void Settings::set_wrapper_command(const QString& command) {
    if (!update_value(m_wrapper_command, command, "wrapper_command")) {
        return;
    }

    Q_EMIT wrapper_command_changed();
}

void Settings::set_frp_install_path_url(const QUrl& path) {
    QString local = path.toLocalFile();
    if (!update_value(m_frp_install_path, local, "frp_path")) {
        return;
    }

    Q_EMIT frp_install_path_changed();
}

void Settings::set_frp_install_path(const QString& path) {
    if (!update_value(m_frp_install_path, path, "frp_path")) {
        return;
    }

    Q_EMIT frp_install_path_changed();
}

void Settings::set_easytier_install_path_url(const QUrl& path) {
    QString local = path.toLocalFile();
    if (!update_value(m_easytier_install_path, local, "easytier_path")) {
        return;
    }

    Q_EMIT easytier_install_path_changed();
}

void Settings::set_easytier_install_path(const QString& path) {
    if (!update_value(m_easytier_install_path, path, "easytier_path")) {
        return;
    }

    Q_EMIT easytier_install_path_changed();
}

void Settings::set_easytier_public_server_index(int index) {
    if (!update_value(m_easytier_public_server_index, index,
                      "easytier_public_server_index")) {
        return;
    }

    Q_EMIT easytier_public_server_index_changed();
}

void Settings::set_github_token(const QString& token) {
    const QString trimmed = token.trimmed();
    if (trimmed.isEmpty()) {
        SecretStore::remove(kGithubTokenKey);
        m_github_token.clear();
    } else {
        if (!SecretStore::save(kGithubTokenKey, trimmed.toUtf8())) {
            m_github_token.clear();
        } else {
            m_github_token = trimmed;
        }
    }
    Q_EMIT github_token_changed();
}

void Settings::clear_cache() { JsonCache::clear_all(); }

void Settings::load() {
    if (m_settings.contains("game_path")) {
        QString raw = m_settings.value("game_path").toString();
        // Migration: older versions stored the full file path. If the
        // basename matches the platform executable, strip it so the stored
        // value is the install directory.
        if (!raw.isEmpty()) {
            QFileInfo info(raw);
            if (info.isFile()) {
                raw = info.absolutePath();
                save("game_path", raw);
            }
        }
        m_game_dir = raw;
    }
    Logger::info("Settings Game Dir {{{}}} empty {}", m_game_dir.toStdString(),
                 m_game_dir.isEmpty());

    m_player_name = m_settings.value("player_name").toString();

    m_mirror_index = m_settings.value("mirror_index", -1).toInt();

    m_language = m_settings.value("language").toString();
    m_accent_color = QColor(
        m_settings.value("accent_color", QColor("#2196F3").name()).toString());
    m_card_colorful_border =
        m_settings.value("card_colorful_border", true).toBool();
    m_wrapper_command = m_settings.value("wrapper_command").toString();
    m_frp_install_path = m_settings.value("frp_path").toString();
    m_easytier_install_path = m_settings.value("easytier_path").toString();
    m_easytier_public_server_index =
        m_settings.value("easytier_public_server_index", -1).toInt();

    // AI-generated: migrate legacy plaintext token to the OS keyring, then
    // drop the old key.
    if (m_settings.contains(kGithubTokenKey)) {
        const QString legacy =
            m_settings.value(kGithubTokenKey).toString().trimmed();
        if (!legacy.isEmpty()) {
            SecretStore::save(kGithubTokenKey, legacy.toUtf8());
        }
        m_settings.remove(kGithubTokenKey);
    }

    if (auto stored = SecretStore::load(kGithubTokenKey)) {
        m_github_token = QString::fromUtf8(*stored);
    }
}
void Settings::save(const QString& key, const QString& value) {
    m_settings.setValue(key, value);
}
