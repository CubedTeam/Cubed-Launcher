#include "game/game_update.hpp"

#include "tool/mirror.hpp"
#include "tool/path_tools.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <qmicroz.h>
#include <qtmetamacros.h>
#include <utility>

GameUpdate::GameUpdate()
    : m_fetcher(&m_manager, QStringLiteral("Cubed"), this),
      m_downloader(&m_manager, this) {
    m_game_install_path = DefaultDir::get_default_game_install_dir();
    connect(&m_downloader, &FileDownloader::progress_changed, this,
            &GameUpdate::download_progress_changed);
    connect(&m_downloader, &FileDownloader::download_finished_changed, this,
            &GameUpdate::download_finish_changed);
    connect(&m_downloader, &FileDownloader::has_error_changed, this,
            &GameUpdate::has_error_changed);
    connect(&m_downloader, &FileDownloader::error_message_changed, this,
            &GameUpdate::error_message_changed);
    connect(&m_downloader, &FileDownloader::downloading_changed, this,
            &GameUpdate::downloading_changed);
    connect(&m_downloader, &FileDownloader::download_complete, this,
            &GameUpdate::on_download_complete);
}

Q_INVOKABLE void GameUpdate::check_update(const QString& local_version) {
    if (std::exchange(m_checking_update, true)) {
        Q_EMIT checking_update_changed();
        return;
    }
    qDebug() << "Loacl Version: " << local_version;
    const bool installed = !local_version.isEmpty();
    if (local_version == "dev") {
        m_local_version = QVersionNumber::fromString("0.0.1");
    } else {
        m_local_version = QVersionNumber::fromString(local_version);
    }
    m_downloader.clear_error_state();

    auto finish_check = [this]() {
        m_checking_update = false;
        Q_EMIT checking_update_changed();
    };
    auto report_failure = [this, finish_check](const QString& message) {
        m_downloader.set_error_state(message);
        m_new_version = false;
        Q_EMIT new_version_changed();
        finish_check();
    };

    m_fetcher.fetch(
        "CubedTeam", "Cubed",
        QRegularExpression(R"(Cubed-.*-windows-x64\.zip)"),
        [this, installed, report_failure,
         finish_check](GithubReleaseFetcher::Result r) {
            if (!r.ok) {
                report_failure(r.errorMessage);
                return;
            }
            m_remote_version = QVersionNumber::fromString(r.version);
            if (m_remote_version.isNull()) {
                report_failure("Failed to parse remote version:" + r.version);
                return;
            }
            if (installed) {
                m_new_version = m_remote_version > m_local_version;
            } else {
                m_new_version = true;
            }
            m_download_url = r.downloadUrl;
            Q_EMIT new_version_changed();
            Q_EMIT remote_version_changed();
            Q_EMIT local_version_changed();
            finish_check();
        });
}

Q_INVOKABLE void GameUpdate::download_from_github(int mirror_index) {
    if (m_download_url.isEmpty()) {
        qDebug() << "Game Download Url is empty";
        m_downloader.start({}, QDir::temp().filePath("Cubed-latest.zip"));
        return;
    }
    QString download_url = m_download_url;
    if (mirror_index > 0 && mirror_index < mirror_sources.size()) {
        const QString& prefix = mirror_sources.at(mirror_index).prefix;
        if (!prefix.isEmpty()) {
            download_url = prefix + download_url;
        }
    }
    download_game(download_url);
}

Q_INVOKABLE void GameUpdate::download_game(const QString& download_url) {
    const QString zip_path = QDir::temp().filePath("Cubed-latest.zip");
    m_downloader.start(download_url, zip_path);
}

void GameUpdate::on_download_complete(const QString& zip_path) {
    QFile check(zip_path);
    if (!check.open(QIODevice::ReadOnly)) {
        m_downloader.report_error(QStringLiteral("Can't open zip"));
        return;
    }
    const QByteArray header = check.read(4);
    check.close();
    if (header.left(4) != QByteArray::fromHex("504b0304")) {
        m_downloader.report_error(QStringLiteral("Downloaded file is invalid"));
        return;
    }

    if (!QMicroz::extract(zip_path, m_game_install_path)) {
        QFile::remove(zip_path);
        m_downloader.report_error(QStringLiteral("Extract file error"));
        return;
    }

    qDebug() << "Install Game Success";
    QFile::remove(zip_path);
    m_downloader.mark_succeeded();
    m_new_version = false;
    m_local_version = m_remote_version;
    Q_EMIT new_version_changed();
    Q_EMIT local_version_changed();
}

Q_INVOKABLE void GameUpdate::cancel_download() { m_downloader.cancel(); }

void GameUpdate::set_game_install_path(const QString& game_dir) {
    if (game_dir.isEmpty()) {
        m_game_install_path = DefaultDir::get_default_game_install_dir();
    } else {
        m_game_install_path = game_dir;
    }
    qDebug() << "VersionUpdate: Change game dir" << m_game_install_path;
    Q_EMIT game_install_path_changed();
}
QString GameUpdate::game_install_path() const { return m_game_install_path; }

bool GameUpdate::has_new_version() const { return m_new_version; }
bool GameUpdate::checking_update() const { return m_checking_update; }
bool GameUpdate::downloading() const { return m_downloader.downloading(); }
QString GameUpdate::local_version() const { return m_local_version.toString(); }
QString GameUpdate::remote_version() const {
    return m_remote_version.toString();
}
float GameUpdate::download_progress() const { return m_downloader.progress(); }

bool GameUpdate::download_finish() const {
    return m_downloader.download_finished();
}
QString GameUpdate::error_message() const {
    return m_downloader.error_message();
}
bool GameUpdate::has_error() const { return m_downloader.has_error(); }
