#include "launcher_update.hpp"

#include "settings.hpp"
#include "tool/log.hpp"
#include "tool/mirror.hpp"
#include "version.hpp"

#include <QDir>
#include <QFile>
#include <QProcess>

LauncherUpdate::LauncherUpdate()
    : m_fetcher(&m_manager, QStringLiteral("CubedLauncher"), this),
      m_downloader(&m_manager, this) {

    const QString appVersion(APP_VERSION);
    m_local_version = SemanticVersion::parse(
        appVersion == "dev" ? QStringView(u"0.0.1") : QStringView(appVersion));
    if (!m_local_version) {
        m_downloader.set_error_state("Failed to parse local version: " +
                                     appVersion);
    } else if (appVersion != "dev" && Settings::instance() &&
               Settings::instance()->auto_check_launcher_updates()) {
        check_update("CubedTeam", "Cubed-Launcher");
    }
    connect(&m_downloader, &FileDownloader::progress_changed, this,
            &LauncherUpdate::download_progress_changed);
    connect(&m_downloader, &FileDownloader::download_finished_changed, this,
            &LauncherUpdate::download_finish_changed);
    connect(&m_downloader, &FileDownloader::has_error_changed, this,
            &LauncherUpdate::has_error_changed);
    connect(&m_downloader, &FileDownloader::error_message_changed, this,
            &LauncherUpdate::error_message_changed);
    connect(&m_downloader, &FileDownloader::downloading_changed, this,
            &LauncherUpdate::downloading_changed);
    connect(&m_downloader, &FileDownloader::download_complete, this,
            &LauncherUpdate::on_download_complete);
}

bool LauncherUpdate::has_new_version() const { return m_new_version; }
bool LauncherUpdate::download_finish() const {
    return m_downloader.download_finished();
}
float LauncherUpdate::download_progress() const {
    return m_downloader.progress();
}
bool LauncherUpdate::downloading() const { return m_downloader.downloading(); }

QString LauncherUpdate::local_version() const {
    return m_local_version ? m_local_version->toString() : QString();
}
QString LauncherUpdate::remote_version() const {
    return m_remote_version ? m_remote_version->toString() : QString();
}

QString LauncherUpdate::error_message() const {
    return m_downloader.error_message();
}
bool LauncherUpdate::has_error() const { return m_downloader.has_error(); }

Q_INVOKABLE void LauncherUpdate::check_update(const QString& owner,
                                              const QString& repo) {
    m_downloader.clear_error_state();
    m_new_version = false;
    m_remote_version.reset();
    m_latest_launcher_link.clear();
    Q_EMIT new_version_changed();
    Q_EMIT remote_version_changed();
    if (!m_local_version) {
        m_downloader.set_error_state("Failed to parse local version: " +
                                     QString(APP_VERSION));
        return;
    }
    const Settings* settings = Settings::instance();
    const bool includePrereleases = settings && settings->prerelease_updates();
    m_fetcher.fetch(
        owner, repo,
        QRegularExpression(R"(CubedLauncher-.*-windows-x64-setup\.exe)"),
        includePrereleases, [this](GithubReleaseFetcher::Result r) {
            if (!r.ok) {
                m_downloader.set_error_state(r.errorMessage);
                return;
            }
            m_remote_version = SemanticVersion::parse(r.version);
            if (!m_remote_version) {
                m_downloader.set_error_state(
                    "Failed to parse remote version: " + r.version);
                return;
            }
            m_new_version = *m_remote_version > *m_local_version;
            m_latest_launcher_link = r.downloadUrl;
            Q_EMIT remote_version_changed();
            Q_EMIT new_version_changed();
        });
}

Q_INVOKABLE void LauncherUpdate::update_launcher_from_url(const QString& url) {
    update_launcher_internal(url);
}

Q_INVOKABLE void LauncherUpdate::update_launcher(int mirror_index) {

    if (m_latest_launcher_link.isEmpty()) {
        Logger::error("Download Url is Null");
        return;
    }
    QString download_url = m_latest_launcher_link;
    if (mirror_index > 0 && mirror_index < mirror_sources.size()) {
        const QString& prefix = mirror_sources.at(mirror_index).prefix;
        if (!prefix.isEmpty()) {
            download_url = prefix + download_url;
        }
    }
    update_launcher_internal(download_url);
}

void LauncherUpdate::update_launcher_internal(const QString& url) {
    const QString setup_path =
        QDir::temp().filePath("CubedLauncher-setup-latest.exe");
    m_downloader.start(url, setup_path);
}

void LauncherUpdate::on_download_complete(const QString& setup_path) {
    QFile check(setup_path);
    if (!check.open(QIODevice::ReadOnly)) {
        m_downloader.report_error(QStringLiteral("Can't open setup file"));
        return;
    }
    const bool valid = check.size() >= 100;
    check.close();
    if (!valid) {
        m_downloader.report_error(QStringLiteral("Downloaded file is invalid"));
        return;
    }

    Logger::info("Download Finish Start Installing...");
    if (!QProcess::startDetached(setup_path)) {
        m_downloader.report_error(
            QStringLiteral("Error can't start Installing Program"));
        return;
    }
    m_downloader.mark_succeeded();
    QCoreApplication::quit();
}

Q_INVOKABLE void LauncherUpdate::cancel_download() { m_downloader.cancel(); }
