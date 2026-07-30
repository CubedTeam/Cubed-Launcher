#include "tool/launcher_update.hpp"

#include "tool/mirror.hpp"
#include "tool/user_agent.hpp"
#include "version.hpp"

#include <QProcess>
#include <QUrl>
#include <qtmetamacros.h>

LauncherUpdate::LauncherUpdate() : m_fetcher(&m_manager, this) {

    if (QString(APP_VERSION) == "dev") {
        m_local_version = QVersionNumber::fromString("0.0.1");
    } else {
        m_local_version = QVersionNumber::fromString(APP_VERSION);
        check_update("CubedTeam", "Cubed-Launcher");
    }
}

bool LauncherUpdate::has_new_version() const { return m_new_version; }
bool LauncherUpdate::download_finish() const { return m_download_finish; }
float LauncherUpdate::download_progress() const { return m_download_progress; }
bool LauncherUpdate::downloading() const { return m_downloading; }

QString LauncherUpdate::local_version() const {
    return m_local_version.toString();
}
QString LauncherUpdate::remote_version() const {
    return m_remote_version.toString();
}

QString LauncherUpdate::error_message() const { return m_error_message; }
bool LauncherUpdate::has_error() const { return m_has_error; }

Q_INVOKABLE void LauncherUpdate::check_update(const QString& owner,
                                              const QString& repo) {
    m_fetcher.fetch(
        owner, repo,
        QRegularExpression(R"(CubedLauncher-.*-windows-x64-setup\.exe)"),
        [this](GithubReleaseFetcher::Result r) {
            if (!r.ok) {
                m_has_error = true;
                m_error_message = r.errorMessage;
                emit has_error_changed();
                emit error_message_changed();
                return;
            }
            m_remote_version = QVersionNumber::fromString(r.version);
            if (m_remote_version.isNull()) {
                m_has_error = true;
                m_error_message = "Failed to parse remote version:" + r.version;
                emit has_error_changed();
                emit error_message_changed();
                return;
            }
            m_new_version = m_remote_version > m_local_version;
            m_latest_launcher_link = r.downloadUrl;
            emit remote_version_changed();
            emit new_version_changed();
        });
}

Q_INVOKABLE void LauncherUpdate::update_launcher(int mirror_index) {
    if (std::exchange(m_downloading, true)) {

        return;
    }
    m_download_progress = 0.0f;
    emit download_progress_changed();
    m_download_finish = false;
    emit download_finish_changed();
    m_cancelling = false;
    m_has_error = false;
    m_error_message.clear();
    emit has_error_changed();
    emit error_message_changed();
    emit downloading_changed();

    auto fail = [this](const QString& message) {
        m_has_error = true;
        m_error_message = message;
        emit has_error_changed();
        emit error_message_changed();
        m_download_progress = 1.0f;
        emit download_progress_changed();
        m_downloading = false;
        emit downloading_changed();
    };
    auto cancel = [this](const QString& path) {
        m_cancelling = false;
        QFile::remove(path);
        m_download_progress = 0.0f;
        m_downloading = false;
        emit downloading_changed();
        emit download_progress_changed();
    };
    auto finish = [this]() {
        m_download_progress = 1.0f;
        m_downloading = false;
        m_download_finish = true;
        emit downloading_changed();
        emit download_finish_changed();
        emit download_progress_changed();
    };

    if (m_latest_launcher_link.isEmpty()) {
        qDebug() << "Download Url is Null";
        fail("Download Url is Null.");
        return;
    }
    QString download_url = m_latest_launcher_link;
    if (mirror_index > 0 && mirror_index < mirror_sources.size()) {
        const QString& prefix = mirror_sources.at(mirror_index).prefix;
        if (!prefix.isEmpty()) {
            download_url = prefix + download_url;
        }
    }

    QNetworkRequest download_request(download_url);
    download_request.setHeader(QNetworkRequest::UserAgentHeader,
                               buildUserAgent().toUtf8());
    download_request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                                  QNetworkRequest::NoLessSafeRedirectPolicy);
    auto* download_reply = m_manager.get(download_request);
    m_download_reply = download_reply;

    const QString setup_path =
        QDir::temp().filePath("CubedLauncher-setup-latest.exe");
    auto file = std::make_shared<QFile>(setup_path);
    if (!file->open(QIODevice::WriteOnly)) {
        qDebug() << "Can't open file";
        download_reply->abort();
        download_reply->deleteLater();
        m_download_reply = nullptr;
        fail("Can't open file");
        return;
    }

    connect(
        download_reply, &QNetworkReply::readyRead, download_reply,
        [download_reply, file]() { file->write(download_reply->readAll()); });

    connect(download_reply, &QNetworkReply::finished, this,
            [download_reply, file, setup_path, fail, cancel, finish, this]() {
                file->close();
                m_download_reply = nullptr;

                if (m_cancelling) {
                    download_reply->deleteLater();
                    cancel(setup_path);
                    return;
                }

                if (download_reply->error() != QNetworkReply::NoError) {
                    qDebug() << download_reply->errorString();
                    download_reply->deleteLater();
                    fail(download_reply->errorString());
                    return;
                }

                QFile check(setup_path);
                if (!check.open(QIODevice::ReadOnly)) {
                    fail("Can't open setup file");
                    return;
                }
                const bool valid = check.size() >= 100;
                check.close();
                if (!valid) {
                    fail("Downloaded file is invalid");
                    return;
                }

                qDebug() << "Download Finish Start Installing...";
                if (!QProcess::startDetached(setup_path)) {
                    fail("Error can't start Installing Program");
                    finish();
                    return;
                }
                download_reply->deleteLater();
                finish();
                QCoreApplication::quit();
            });

    connect(download_reply, &QNetworkReply::downloadProgress, this,
            [this](qint64 received, qint64 total) {
                if (total > 0) {
                    m_download_progress = float(received) / float(total);
                    emit download_progress_changed();
                }
            });
}

// AI-generated: abort in-flight update; finished handler clears state.
void LauncherUpdate::cancel_download() {
    if (!m_downloading || m_cancelling || !m_download_reply) {
        return;
    }
    m_cancelling = true;
    m_download_reply->abort();
}
