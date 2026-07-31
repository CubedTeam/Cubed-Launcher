#include "tool/game_update.hpp"

#include "tool/game_path.hpp"
#include "tool/mirror.hpp"
#include "tool/user_agent.hpp"

#include <QDir>
#include <QFileInfo>
#include <QNetworkReply>
#include <qmicroz.h>
#include <qtmetamacros.h>
#include <utility>

GameUpdate::GameUpdate() : m_fetcher(&m_manager, this) {
    m_game_install_path = get_default_game_install_dir();
}

Q_INVOKABLE void GameUpdate::check_update(const QString& local_version) {
    if (std::exchange(m_checking_update, true)) {
        emit checking_update_changed();
        return;
    }
    qDebug() << "Loacl Version: " << local_version;
    const bool installed = !local_version.isEmpty();
    if (local_version == "dev") {
        m_local_version = QVersionNumber::fromString("0.0.1");
    } else {
        m_local_version = QVersionNumber::fromString(local_version);
    }
    m_has_error = false;
    m_error_message.clear();
    emit has_error_changed();
    emit error_message_changed();

    auto finish_check = [this]() {
        m_checking_update = false;
        emit checking_update_changed();
    };
    auto report_failure = [this, finish_check](const QString& message) {
        m_has_error = true;
        m_error_message = message;
        emit has_error_changed();
        emit error_message_changed();
        m_new_version = false;
        emit new_version_changed();
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
            emit new_version_changed();
            emit remote_version_changed();
            emit local_version_changed();
            finish_check();
        });
}

Q_INVOKABLE void GameUpdate::download_from_github(int mirror_index) {
    m_has_error = false;
    m_error_message.clear();
    emit has_error_changed();
    emit error_message_changed();

    if (m_download_url.isEmpty()) {
        qDebug() << "Game Download Url is empty";
        m_has_error = true;
        m_error_message = "Game Download Url is empty";
        m_download_progress = 1.0f;
        emit has_error_changed();
        emit error_message_changed();
        // AI-generated: pulse downloading so the UI re-enables controls.
        m_downloading = true;
        emit downloading_changed();
        m_downloading = false;
        emit downloading_changed();
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
    m_has_error = false;
    m_error_message.clear();
    emit has_error_changed();
    emit error_message_changed();

    if (std::exchange(m_downloading, true)) {
        return;
    }
    m_download_finish = false;
    emit download_finish_changed();
    m_download_progress = 0.0f;
    emit download_progress_changed();
    m_cancelling = false;
    m_has_error = false;
    m_error_message.clear();
    emit has_error_changed();
    emit error_message_changed();
    emit downloading_changed();
    if (download_url.isEmpty()) {
        qDebug() << "Error Download Url is empty";
        m_has_error = true;
        m_error_message = "Error Download Url is empty";
        emit has_error_changed();
        emit error_message_changed();
        m_download_progress = 1.0f;
        emit download_progress_changed();
        m_downloading = false;
        emit downloading_changed();
        return;
    }
    qDebug() << "Download url" << download_url;

    QNetworkRequest download_request(download_url);
    download_request.setHeader(QNetworkRequest::UserAgentHeader,
                               buildUserAgent().toUtf8());
    download_request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                                  QNetworkRequest::NoLessSafeRedirectPolicy);
    auto* download_reply = m_manager.get(download_request);
    m_download_reply = download_reply;

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

    const QString zip_path = QDir::temp().filePath("Cubed-latest.zip");
    auto file = std::make_shared<QFile>(zip_path);
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
            [download_reply, file, zip_path, fail, cancel, finish, this]() {
                file->close();
                if (m_download_reply == download_reply) {
                    m_download_reply = nullptr;
                }

                if (m_cancelling) {
                    download_reply->deleteLater();
                    cancel(zip_path);
                    return;
                }

                if (download_reply->error() != QNetworkReply::NoError) {
                    qDebug() << download_reply->errorString();
                    download_reply->deleteLater();
                    fail(download_reply->errorString());
                    return;
                }

                qDebug() << "Download File completed! Zip Path" << zip_path;
                qDebug() << "Before extract zip_path" << zip_path
                         << "to game install path" << m_game_install_path;

                QFile check(zip_path);
                if (!check.open(QIODevice::ReadOnly)) {
                    fail("Can't open zip");
                    return;
                }
                const QByteArray header = check.read(4);
                check.close();
                if (header.left(4) != QByteArray::fromHex("504b0304")) {
                    fail("Downloaded file is invalid");
                    return;
                }

                if (!QMicroz::extract(zip_path, m_game_install_path)) {
                    QFile::remove(zip_path);
                    download_reply->deleteLater();
                    fail("Extract file error");
                    return;
                }

                qDebug() << "Install Game Success";
                QFile::remove(zip_path);
                download_reply->deleteLater();
                finish();
                m_new_version = false;
                m_local_version = m_remote_version;
                emit new_version_changed();
                emit local_version_changed();
            });

    connect(download_reply, &QNetworkReply::downloadProgress, this,
            [this](qint64 received, qint64 total) {
                if (total > 0) {
                    m_download_progress = float(received) / float(total);
                    emit download_progress_changed();
                }
            });
}

// AI-generated: abort in-flight download; finished handler clears state.
void GameUpdate::cancel_download() {
    if (!m_downloading || m_cancelling || !m_download_reply) {
        return;
    }
    m_cancelling = true;
    m_download_reply->abort();
}

void GameUpdate::set_game_install_path(const QString& game_dir) {
    if (game_dir.isEmpty()) {
        m_game_install_path = get_default_game_install_dir();
    } else {
        m_game_install_path = game_dir;
    }
    qDebug() << "VersionUpdate: Change game dir" << m_game_install_path;
    emit game_install_path_changed();
}
QString GameUpdate::game_install_path() const { return m_game_install_path; }

bool GameUpdate::has_new_version() const { return m_new_version; }
bool GameUpdate::checking_update() const { return m_checking_update; }
bool GameUpdate::downloading() const { return m_downloading; }
QString GameUpdate::local_version() const { return m_local_version.toString(); }
QString GameUpdate::remote_version() const {
    return m_remote_version.toString();
}
float GameUpdate::download_progress() const { return m_download_progress; }

bool GameUpdate::download_finish() const { return m_download_finish; }
QString GameUpdate::error_message() const { return m_error_message; }
bool GameUpdate::has_error() const { return m_has_error; }
