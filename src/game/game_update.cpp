#include "game/game_update.hpp"

#include "settings.hpp"
#include "tool/log.hpp"
#include "tool/mirror.hpp"
#include "tool/path_tools.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <qmicroz.h>
#include <qtmetamacros.h>
#include <utility>

namespace {
#ifdef _WIN32
constexpr QStringView kAssetPattern = u"Cubed-.*-windows-x64\\.zip";
constexpr QStringView kTempFileName = u"Cubed-latest.zip";
#else
constexpr QStringView kAssetPattern = u"Cubed-.*-linux-x64\\.tar\\.gz";
constexpr QStringView kTempFileName = u"Cubed-latest.tar.gz";
#endif

QString temp_archive_path() {
    return QDir::temp().filePath(kTempFileName.toString());
}
} // namespace

GameUpdate::GameUpdate()
    : m_fetcher(&m_manager,
#ifdef _WIN32
                QStringLiteral("Cubed"),
#else
                QStringLiteral("Cubed-linux"),
#endif
                this),
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
    if (!std::exchange(m_checking_update, true)) {
        Q_EMIT checking_update_changed();
    }
    Logger::debug("Local Version: {}", local_version.toStdString());
    const bool installed = !local_version.isEmpty();
    m_downloader.clear_error_state();
    m_new_version = false;
    m_remote_version.reset();
    m_download_url.clear();
    Q_EMIT new_version_changed();
    Q_EMIT remote_version_changed();

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

    m_local_version = SemanticVersion::parse(local_version == "dev"
                                                 ? QStringView(u"0.0.1")
                                                 : QStringView(local_version));
    if (installed && !m_local_version) {
        report_failure("Failed to parse local version: " + local_version);
        return;
    }

    const Settings* settings = Settings::instance();
    const bool includePrereleases = settings && settings->prerelease_updates();

    m_fetcher.fetch(
        "CubedTeam", "Cubed", QRegularExpression(kAssetPattern.toString()),
        includePrereleases,
        [this, installed, report_failure,
         finish_check](GithubReleaseFetcher::Result r) {
            if (!r.ok) {
                report_failure(r.errorMessage);
                return;
            }
            m_remote_version = SemanticVersion::parse(r.version);
            if (!m_remote_version) {
                report_failure("Failed to parse remote version: " + r.version);
                return;
            }
            if (installed) {
                m_new_version = *m_remote_version > *m_local_version;
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
        Logger::warn("Game Download Url is empty");
        m_downloader.start({}, temp_archive_path());
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
    m_downloader.start(download_url, temp_archive_path());
}

void GameUpdate::on_download_complete(const QString& archive_path) {
    QFile check(archive_path);
    if (!check.open(QIODevice::ReadOnly)) {
        m_downloader.report_error(QStringLiteral("Can't open archive"));
        return;
    }
#ifdef _WIN32
    const QByteArray header = check.read(4);
    check.close();
    if (header.left(4) != QByteArray::fromHex("504b0304")) {
        m_downloader.report_error(QStringLiteral("Downloaded file is invalid"));
        return;
    }
    if (!QMicroz::extract(archive_path, m_game_install_path)) {
        QFile::remove(archive_path);
        m_downloader.report_error(QStringLiteral("Extract file error"));
        return;
    }
#else
    // AI-generated: Linux tarball has a top-level Cubed/ dir; strip it and
    // keep the executable bit via system tar.
    const QByteArray header = check.read(2);
    check.close();
    if (header.left(2) != QByteArray::fromHex("1f8b")) {
        m_downloader.report_error(QStringLiteral("Downloaded file is invalid"));
        return;
    }
    QDir().mkpath(m_game_install_path);
    QProcess tar;
    tar.setProgram(QStringLiteral("tar"));
    tar.setArguments({"xzf", archive_path, "-C", m_game_install_path,
                      "--strip-components", "1"});
    tar.start();
    if (!tar.waitForStarted() || !tar.waitForFinished(-1) ||
        tar.exitCode() != 0) {
        QFile::remove(archive_path);
        m_downloader.report_error(
            QStringLiteral("Extract file error: %1").arg(tar.errorString()));
        return;
    }
#endif

    Logger::info("Install Game Success");
    QFile::remove(archive_path);
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
    Logger::info("VersionUpdate: Change game dir {}",
                 m_game_install_path.toStdString());
    Q_EMIT game_install_path_changed();
}
QString GameUpdate::game_install_path() const { return m_game_install_path; }

bool GameUpdate::has_new_version() const { return m_new_version; }
bool GameUpdate::checking_update() const { return m_checking_update; }
bool GameUpdate::downloading() const { return m_downloader.downloading(); }
QString GameUpdate::local_version() const {
    return m_local_version ? m_local_version->toString() : QString();
}
QString GameUpdate::remote_version() const {
    return m_remote_version ? m_remote_version->toString() : QString();
}
float GameUpdate::download_progress() const { return m_downloader.progress(); }

bool GameUpdate::download_finish() const {
    return m_downloader.download_finished();
}
QString GameUpdate::error_message() const {
    return m_downloader.error_message();
}
bool GameUpdate::has_error() const { return m_downloader.has_error(); }
