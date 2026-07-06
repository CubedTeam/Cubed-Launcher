#include "tool/game_update.hpp"

#include "tool/game_path.hpp"
#include "tool/mirror.hpp"
#include "tool/user_agent.hpp"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <qmicroz.h>
GameUpdate::GameUpdate() { m_game_install_path = get_default_game_file_path(); }

Q_INVOKABLE void GameUpdate::check_update(const QString& local_version) {
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
                return;
            }

            if (replay->error() != QNetworkReply::NoError) {
                qDebug() << "Network Error:" << replay->errorString();
                m_has_error = true;
                m_error_message = replay->errorString();
                emit has_error_changed();
                emit error_message_changed();
                replay->deleteLater();
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
                return;
            }

            qDebug() << "Find latest Game url" << download_url;

            m_download_url = download_url;
            emit new_version_changed();
            emit remote_version_changed();
            emit local_version_changed();
        });
}

Q_INVOKABLE void GameUpdate::download_from_github(bool use_mirror) {
    m_has_error = false;
    m_error_message.clear();
    emit has_error_changed();
    emit error_message_changed();

    if (m_download_url.isEmpty()) {
        qDebug() << "Game Download Url is empty";
        m_has_error = true;
        m_error_message = "Game Download Url is empty";
        emit has_error_changed();
        emit error_message_changed();
        return;
    }
    QString download_url = m_download_url;
    if (use_mirror) {
        download_url.prepend(mirror[0]);
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
    if (download_url.isEmpty()) {
        qDebug() << "Error Download Url is empty";
        m_has_error = true;
        m_error_message = "Error Download Url is empty";
        emit has_error_changed();
        emit error_message_changed();
        m_download_progress = 1.0f;
        emit download_progress_changed();
        m_downloading = false;
        return;
    }
    qDebug() << "Download url" << download_url;

    QNetworkRequest download_request(download_url);
    download_request.setHeader(QNetworkRequest::UserAgentHeader,
                               buildUserAgent().toUtf8());
    download_request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                                  QNetworkRequest::NoLessSafeRedirectPolicy);
    auto* download_reply = m_manager.get(download_request);

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
        m_download_progress = 1.0f;
        emit download_progress_changed();
        m_downloading = false;
        return;
    }

    connect(
        download_reply, &QNetworkReply::readyRead, this,
        [download_reply, file]() { file->write(download_reply->readAll()); });

    connect(download_reply, &QNetworkReply::finished, this,
            [download_reply, file, zip_path, this]() {
                file->close();

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
                    return;
                } else {
                    qDebug() << "Install Game Success";
                }
                QFile::remove(zip_path);
                download_reply->deleteLater();
                m_download_progress = 1.0f;
                m_downloading = false;
                m_download_finish = true;
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
QString GameUpdate::local_version() const { return m_local_version.toString(); }
QString GameUpdate::remote_version() const {
    return m_remote_version.toString();
}
float GameUpdate::download_progress() const { return m_download_progress; }

bool GameUpdate::download_finish() const { return m_download_finish; }
QString GameUpdate::error_message() const { return m_error_message; }
bool GameUpdate::has_error() const { return m_has_error; }
