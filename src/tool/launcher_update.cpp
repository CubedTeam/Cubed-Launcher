#include "tool/launcher_update.hpp"

#include "tool/mirror.hpp"
#include "tool/user_agent.hpp"
#include "version.hpp"

#include <QProcess>
#include <qtmetamacros.h>

LauncherUpdate::LauncherUpdate() {

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
    QUrl url(QString("https://api.github.com/repos/%1/%2/releases/latest")
                 .arg(owner, repo));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      buildUserAgent().toUtf8());

    auto* replay = m_manager.get(request);

    connect(replay, &QNetworkReply::finished, this, [this, replay]() {
        if (!replay) {
            m_has_error = true;
            m_error_message = "Check Update Fail can't find reply";
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
            qDebug() << "Failed to parse remote version:" << latest_version_str;
            m_has_error = true;
            m_error_message =
                "Failed to parse remote version:" + latest_version_str;
            emit has_error_changed();
            emit error_message_changed();
            replay->deleteLater();
            return;
        }

        if (m_remote_version > m_local_version) {
            m_new_version = true;
            qDebug() << "New version available! Remote:" << latest_version_str
                     << "Local:" << m_local_version.toString();
        } else {
            qDebug() << "Already up to date. Local:"
                     << m_local_version.toString()
                     << "Remote:" << latest_version_str;
        }

        auto assets = json_obj["assets"].toArray();

        QRegularExpression regex(R"(CubedLauncher-.*-windows-x64-setup\.exe)");

        for (const auto& v : assets) {
            auto obj = v.toObject();

            QString name = obj["name"].toString();

            if (regex.match(name).hasMatch()) {
                m_latest_launcher_link = obj["browser_download_url"].toString();
                qDebug() << "Find Launcher Url " << m_latest_launcher_link;
                break;
            }
        }

        if (m_latest_launcher_link.isEmpty()) {
            m_has_error = true;
            m_error_message = "No Windows package found.";
            emit has_error_changed();
            emit error_message_changed();
            replay->deleteLater();
            return;
        }

        replay->deleteLater();

        emit remote_version_changed();
        emit new_version_changed();
    });
}

Q_INVOKABLE void LauncherUpdate::update_launcher(int mirror_index) {
    if (std::exchange(m_downloading, true)) {

        return;
    }

    if (m_latest_launcher_link.isEmpty()) {
        qDebug() << "Download Url is Null";
        m_has_error = true;
        m_error_message = "Download Url is Null.";
        emit has_error_changed();
        emit error_message_changed();
        m_downloading = false;
        m_download_progress = 1.0f;
        emit download_progress_changed();
        return;
    }
    QString download_url = m_latest_launcher_link;
    // AI-generated: prepend the selected mirror prefix (0 means direct).
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

    QString setup_path =
        QDir::temp().filePath("CubedLauncher-setup-latest.exe");
    auto file = std::make_shared<QFile>(setup_path);
    if (!file->open(QIODevice::WriteOnly)) {
        qDebug() << "Can't open file";
        download_reply->abort();
        download_reply->deleteLater();
        m_has_error = true;
        m_error_message = "Can't open file";
        emit has_error_changed();
        emit error_message_changed();
        m_download_progress = 1.0f;
        emit download_progress_changed();
        m_downloading = false;
        return;
    }

    connect(
        download_reply, &QNetworkReply::readyRead, this,
        [download_reply, file]() { file->write(download_reply->readAll()); });

    connect(download_reply, &QNetworkReply::finished, this,
            [download_reply, file, setup_path, this]() {
                file->close();

                if (download_reply->error() != QNetworkReply::NoError) {
                    qDebug() << download_reply->errorString();
                    download_reply->deleteLater();
                    m_has_error = true;
                    m_error_message = download_reply->errorString();
                    emit has_error_changed();
                    emit error_message_changed();
                    m_download_progress = 1.0f;
                    emit download_progress_changed();
                    m_downloading = false;
                    return;
                }

                QFile check(setup_path);

                if (!check.open(QIODevice::ReadOnly)) {
                    qDebug() << "Can't open setup file";
                    m_has_error = true;
                    m_error_message = "Can't open setup file";
                    emit has_error_changed();
                    emit error_message_changed();
                    m_download_progress = 1.0f;
                    emit download_progress_changed();
                    m_downloading = false;
                    return;
                }

                if (check.size() < 100) {
                    qDebug() << "Downloaded file is invalid";
                    m_has_error = true;
                    m_error_message = "Downloaded file is invalid";
                    emit has_error_changed();
                    emit error_message_changed();
                    check.close();
                    m_download_progress = 1.0f;
                    emit download_progress_changed();
                    m_downloading = false;
                    return;
                }
                qDebug() << "Download Finish Start Installing...";
                if (!QProcess::startDetached(setup_path)) {
                    qDebug() << "Error can't start Installing Program";
                    m_has_error = true;
                    m_error_message = "Error can't start Installing Program";
                    emit has_error_changed();
                    emit error_message_changed();
                    m_download_progress = 1.0f;
                    m_download_finish = true;
                    m_downloading = false;
                    emit download_finish_changed();
                    emit download_progress_changed();
                    return;
                }
                download_reply->deleteLater();
                m_download_progress = 1.0f;
                m_download_finish = true;
                emit download_finish_changed();
                emit download_progress_changed();
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
