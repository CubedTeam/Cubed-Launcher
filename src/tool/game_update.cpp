#include "tool/game_update.hpp"

#include "tool/game_path.hpp"
#include "tool/mirror.hpp"
#include "tool/user_agent.hpp"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <qmicroz.h>
#include <qtmetamacros.h>
#include <utility>

GameUpdate::GameUpdate() { m_game_install_path = get_default_game_file_path(); }

Q_INVOKABLE void GameUpdate::check_update(const QString& local_version) {
    if (std::exchange(m_checking_update, true)) {
        emit checking_update_changed();
        return;
    }
    qDebug() << "Loacl Version: " << local_version;
    bool installed = true;
    if (local_version.isEmpty()) {
        installed = false;
    } else if (local_version == "dev") {
        m_local_version = QVersionNumber::fromString("0.0.1");
    } else {
        m_local_version = QVersionNumber::fromString(local_version);
    }
    m_has_error = false;
    m_error_message.clear();
    emit has_error_changed();
    emit error_message_changed();

    QUrl url(QString("https://api.github.com/repos/%1/%2/releases/latest")
                 .arg("CubedTeam", "Cubed"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      buildUserAgent().toUtf8());

    auto* replay = m_manager.get(request);

    connect(
        replay, &QNetworkReply::finished, this, [this, installed, replay]() {
            if (!replay) {
                m_has_error = true;
                m_error_message = "Check update failed: reply is null";
                emit has_error_changed();
                emit error_message_changed();
                m_checking_update = false;
                m_new_version = false;
                emit new_version_changed();
                emit checking_update_changed();
                return;
            }

            if (replay->error() != QNetworkReply::NoError) {
                qDebug() << "Network Error:" << replay->errorString();
                m_has_error = true;
                m_error_message = replay->errorString();
                emit has_error_changed();
                emit error_message_changed();
                replay->deleteLater();
                m_checking_update = false;
                m_new_version = false;
                emit new_version_changed();
                emit checking_update_changed();
                return;
            }

            QByteArray response_data = replay->readAll();
            QJsonDocument json_doc = QJsonDocument::fromJson(response_data);
            if (json_doc.isNull() || !json_doc.isObject()) {
                qDebug() << "Invalid JSON response";
                m_has_error = true;
                m_error_message = "Invalid JSON response";
                emit has_error_changed();
                emit error_message_changed();
                replay->deleteLater();
                m_checking_update = false;
                m_new_version = false;
                emit new_version_changed();
                emit checking_update_changed();
                return;
            }

            QJsonObject json_obj = json_doc.object();

            if (!json_obj.contains("tag_name")) {
                qDebug() << "No tag_name found in response";
                m_has_error = true;
                m_error_message = "No tag_name found in response";
                emit has_error_changed();
                emit error_message_changed();
                replay->deleteLater();
                m_checking_update = false;
                m_new_version = false;
                emit new_version_changed();
                emit checking_update_changed();
                return;
            }

            QString latest_version_str = json_obj["tag_name"].toString();

            if (latest_version_str.startsWith('v', Qt::CaseInsensitive)) {
                latest_version_str.remove(0, 1);
            }

            if (latest_version_str.isEmpty()) {
                qDebug() << "No tag_name found in response";
                m_has_error = true;
                m_error_message = "No tag_name found in response";
                emit has_error_changed();
                emit error_message_changed();
                replay->deleteLater();
                m_checking_update = false;
                m_new_version = false;
                emit new_version_changed();
                emit checking_update_changed();
                return;
            }

            m_remote_version = QVersionNumber::fromString(latest_version_str);

            if (m_remote_version.isNull()) {
                qDebug() << "Failed to parse remote version:"
                         << latest_version_str;
                m_has_error = true;
                m_error_message =
                    "Failed to parse remote version:" + latest_version_str;
                emit has_error_changed();
                emit error_message_changed();
                replay->deleteLater();
                m_checking_update = false;
                m_new_version = false;
                emit new_version_changed();
                emit checking_update_changed();
                return;
            }

            if (installed) {
                if (m_remote_version > m_local_version) {
                    m_new_version = true;
                    qDebug() << "New Game version available! Remote:"
                             << latest_version_str
                             << "Local:" << m_local_version.toString();
                } else {
                    qDebug() << "Game Already up to date. Local:"
                             << m_local_version.toString()
                             << "Remote:" << latest_version_str;
                }
                emit new_version_changed();
            } else {
                if (m_has_error) {
                    m_new_version = false;
                } else {
                    m_new_version = true;
                }

                emit new_version_changed();
            }

            auto assets = json_obj["assets"].toArray();

            QRegularExpression regex(R"(Cubed-.*-windows-x64\.zip)");

            QString download_url;

            for (const auto& v : assets) {
                auto obj = v.toObject();

                QString name = obj["name"].toString();

                if (regex.match(name).hasMatch()) {
                    download_url = obj["browser_download_url"].toString();
                    break;
                }
            }

            replay->deleteLater();

            if (download_url.isEmpty()) {
                qDebug() << "No Windows package found.";
                m_has_error = true;
                m_error_message = "No Windows package found.";
                emit has_error_changed();
                emit error_message_changed();
                m_checking_update = false;
                m_new_version = false;
                emit new_version_changed();
                emit checking_update_changed();
                return;
            }

            qDebug() << "Find latest Game url" << download_url;

            m_download_url = download_url;
            emit new_version_changed();
            emit remote_version_changed();
            emit local_version_changed();
            m_checking_update = false;
            emit checking_update_changed();
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
        // AI-generated: drive the downloading lifecycle so the UI re-enables
        // buttons even when we bail out before a real download starts.
        m_downloading = true;
        emit downloading_changed();
        m_downloading = false;
        emit downloading_changed();
        return;
    }
    QString download_url = m_download_url;
    // AI-generated: prepend mirror prefix, 0 = direct.
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
    // AI-generated: reset progress so UI can re-trigger on finish/error.
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

    QString zip_path = QDir::temp().filePath("Cubed-latest.zip");
    auto file = std::make_shared<QFile>(zip_path);
    if (!file->open(QIODevice::WriteOnly)) {
        qDebug() << "Can't open file";
        m_has_error = true;
        m_error_message = "Can't open file";
        emit has_error_changed();
        emit error_message_changed();

        download_reply->disconnect();

        download_reply->abort();
        download_reply->deleteLater();
        m_download_reply = nullptr;
        m_download_progress = 1.0f;
        emit download_progress_changed();
        m_downloading = false;
        emit downloading_changed();
        return;
    }

    connect(
        download_reply, &QNetworkReply::readyRead, this,
        [download_reply, file]() { file->write(download_reply->readAll()); });

    connect(download_reply, &QNetworkReply::finished, this,
            [download_reply, file, zip_path, this]() {
                file->close();
                if (m_download_reply == download_reply) {
                    m_download_reply = nullptr;
                }

                // AI-generated: user-cancelled download, finish quietly.
                if (m_cancelling) {
                    m_cancelling = false;
                    download_reply->deleteLater();
                    QFile::remove(zip_path);
                    m_downloading = false;
                    m_download_progress = 0.0f;
                    emit downloading_changed();
                    emit download_progress_changed();
                    return;
                }

                if (download_reply->error() != QNetworkReply::NoError) {
                    qDebug() << download_reply->errorString();
                    m_has_error = true;
                    m_error_message = download_reply->errorString();
                    emit has_error_changed();
                    emit error_message_changed();
                    download_reply->deleteLater();
                    m_download_progress = 1.0f;
                    emit download_progress_changed();
                    m_downloading = false;
                    emit downloading_changed();
                    return;
                }

                qDebug() << "Download File completed! Zip Path" << zip_path;
                qDebug() << "Before extract zip_path" << zip_path
                         << "to game install path" << m_game_install_path;

                QFile check(zip_path);

                if (!check.open(QIODevice::ReadOnly)) {
                    qDebug() << "Can't open zip";
                    m_has_error = true;
                    m_error_message = "Can't open zip";
                    emit has_error_changed();
                    emit error_message_changed();
                    m_downloading = false;
                    emit downloading_changed();
                    return;
                }

                QByteArray header = check.read(4);
                qDebug() << header.toHex();

                if (header.left(4) != QByteArray::fromHex("504b0304")) {
                    qDebug() << "Downloaded file is invalid";
                    m_has_error = true;
                    m_error_message = "Downloaded file is invalid";
                    emit has_error_changed();
                    emit error_message_changed();
                    check.close();
                    m_downloading = false;
                    emit downloading_changed();
                    return;
                }

                check.close();

                QFile f(zip_path);
                qDebug() << "exists =" << f.exists();

                if (f.open(QIODevice::ReadOnly)) {
                    qDebug() << "QFile open OK";
                    qDebug() << "size =" << f.size();
                    f.close();
                } else {
                    qDebug() << "QFile open failed:" << f.errorString();
                    m_has_error = true;
                    m_error_message = "QFile open failed:" + f.errorString();
                    emit has_error_changed();
                    emit error_message_changed();
                    m_download_progress = 1.0f;
                    emit download_progress_changed();
                    QFile::remove(zip_path);
                    download_reply->deleteLater();
                    m_downloading = false;
                    emit downloading_changed();
                    return;
                }

                if (!QMicroz::extract(zip_path, m_game_install_path)) {
                    qDebug() << "Extract file error";
                    m_has_error = true;
                    m_error_message = "Extract file error";
                    emit has_error_changed();
                    emit error_message_changed();
                    m_download_progress = 1.0f;
                    emit download_progress_changed();
                    QFile::remove(zip_path);
                    download_reply->deleteLater();
                    m_downloading = false;
                    emit downloading_changed();
                    return;
                } else {
                    qDebug() << "Install Game Success";
                }
                QFile::remove(zip_path);
                download_reply->deleteLater();
                m_download_progress = 1.0f;
                m_downloading = false;
                emit downloading_changed();
                m_download_finish = true;
                m_new_version = false;
                m_local_version = m_remote_version;
                emit new_version_changed();
                emit local_version_changed();
                emit download_finish_changed();
                emit download_progress_changed();
            });

    connect(download_reply, &QNetworkReply::downloadProgress, this,
            [this](qint64 received, qint64 total) {
                if (total > 0) {
                    m_download_progress = float(received) / float(total);
                    emit download_progress_changed();
                }
            });
}

// AI-generated: abort the in-flight download; finished handler clears state.
void GameUpdate::cancel_download() {
    if (!m_downloading || m_cancelling || !m_download_reply) {
        return;
    }
    m_cancelling = true;
    m_download_reply->abort();
}

void GameUpdate::set_game_install_path(const QString& game_file_dir) {
    QFileInfo info;

    if (game_file_dir.isEmpty()) {
        info = QFileInfo(get_default_game_file_path());
    } else {
        info = QFileInfo(game_file_dir);
    }

    m_game_install_path = info.absolutePath();
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
