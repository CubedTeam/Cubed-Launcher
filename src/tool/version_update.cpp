#include "tool/version_update.hpp"

#include "version.hpp"
VersionUpdate::VersionUpdate() {
    if (QString(APP_VERSION) == "dev") {
        m_local_version = QVersionNumber::fromString("0.0.1");
    } else {
        m_local_version = QVersionNumber::fromString(APP_VERSION);
        check_update("CubedTeam", "Cubed-Launcher");
    }
}

bool VersionUpdate::has_new_version() const { return m_new_version; }
float VersionUpdate::download_progress() const { return m_download_progress; }
QString VersionUpdate::local_version() const {
    return m_local_version.toString();
}
QString VersionUpdate::remote_version() const {
    return m_remote_version.toString();
}

Q_INVOKABLE void VersionUpdate::check_update(QString onwer, QString repo) {
    QUrl url(QString("https://api.github.com/repos/%1/%2/releases/latest")
                 .arg(onwer, repo));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      buildUserAgent().toUtf8());

    auto* replay = m_manager.get(request);
    connect(replay, &QNetworkReply::finished, this,
            &VersionUpdate::on_reply_finished);
}

Q_INVOKABLE void VersionUpdate::download_and_install_game(QString game_path) {

    if (std::exchange(m_downloading, true)) {
        return;
    }

    QFileInfo info(game_path);
    QString parentDir = info.absolutePath();

    QUrl url("https://api.github.com/repos/CubedTeam/Cubed/releases/latest");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      buildUserAgent().toUtf8());

    auto* reply = m_manager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, parentDir]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            reply->deleteLater();
            return;
        }

        auto doc = QJsonDocument::fromJson(reply->readAll());

        if (!doc.isObject()) {
            reply->deleteLater();
            return;
        }

        auto assets = doc.object()["assets"].toArray();

        QRegularExpression regex(R"(Cubed-.*-windows-x64\.zip)");

        QString downloadUrl;

        for (const auto& v : assets) {
            auto obj = v.toObject();

            QString name = obj["name"].toString();

            if (regex.match(name).hasMatch()) {
                downloadUrl = obj["browser_download_url"].toString();
                break;
            }
        }

        reply->deleteLater();

        if (downloadUrl.isEmpty()) {
            qDebug() << "No Windows package found.";
            return;
        }

        QNetworkRequest downloadRequest(downloadUrl);
        downloadRequest.setHeader(QNetworkRequest::UserAgentHeader,
                                  buildUserAgent().toUtf8());

        auto* downloadReply = m_manager.get(downloadRequest);

        connect(downloadReply, &QNetworkReply::finished, this,
                [downloadReply, parentDir]() {
                    if (downloadReply->error() != QNetworkReply::NoError) {
                        qDebug() << downloadReply->errorString();
                        downloadReply->deleteLater();
                        return;
                    }

                    QString zipPath = QDir::temp().filePath("Cubed-latest.zip");

                    auto* file = new QFile(zipPath);

                    if (!file->open(QIODevice::WriteOnly)) {
                        qDebug() << "Can't open file";
                        return;
                    }

                    connect(downloadReply, &QNetworkReply::readyRead,
                            [downloadReply, file]() {
                                file->write(downloadReply->readAll());
                            });

                    connect(downloadReply, &QNetworkReply::finished,
                            [downloadReply, file, zipPath, parentDir]() {
                                file->close();
                                delete file;

                                if (downloadReply->error() ==
                                    QNetworkReply::NoError) {
                                    QMicroz::extract(zipPath, parentDir);
                                    QFile::remove(zipPath);
                                }

                                downloadReply->deleteLater();
                            });
                });

        connect(downloadReply, &QNetworkReply::downloadProgress, this,
                [this](qint64 received, qint64 total) {
                    if (total > 0) {
                        m_download_progress = float(received) / float(total);
                        emit download_progress_changed();
                    }
                });
    });
}

QString VersionUpdate::buildUserAgent() {
    QString app_name = QCoreApplication::applicationName();
    if (app_name.isEmpty())
        app_name = "Qt-UpdateChecker";

    QString app_ver = QCoreApplication::applicationVersion();
    if (app_ver.isEmpty())
        app_ver = "1.0";

    return QString("%1/%2 (Qt/%3; %4; %5)")
        .arg(app_name)
        .arg(app_ver)
        .arg(qVersion())
        .arg(QSysInfo::prettyProductName())
        .arg(QSysInfo::currentCpuArchitecture());
}

void VersionUpdate::on_reply_finished() {
    auto* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network Error:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray response_data = reply->readAll();
    QJsonDocument json_doc = QJsonDocument::fromJson(response_data);
    if (json_doc.isNull() || !json_doc.isObject()) {
        qDebug() << "Invalid JSON response";
        reply->deleteLater();
        return;
    }

    QJsonObject json_obj = json_doc.object();
    QString latest_version_str = json_obj["tag_name"].toString();

    if (latest_version_str.startsWith('v', Qt::CaseInsensitive)) {
        latest_version_str.remove(0, 1);
    }

    if (latest_version_str.isEmpty()) {
        qDebug() << "No tag_name found in response";
        reply->deleteLater();
        return;
    }

    m_remote_version = QVersionNumber::fromString(latest_version_str);

    if (m_remote_version.isNull()) {
        qDebug() << "Failed to parse remote version:" << latest_version_str;
        reply->deleteLater();
        return;
    }

    if (m_remote_version > m_local_version) {
        m_new_version = true;
        qDebug() << "New version available! Remote:" << latest_version_str
                 << "Local:" << m_local_version.toString();
    } else {
        qDebug() << "Already up to date. Local:" << m_local_version.toString()
                 << "Remote:" << latest_version_str;
    }

    reply->deleteLater();

    emit remote_version_changed();
    emit new_version_changed();
}
