#include "tool/version_update.hpp"

#include "tool/game_path.hpp"
#include "version.hpp"

#include <QProcess>
#include <qtmetamacros.h>

namespace {
const QStringList mirror{"https://v4.gh-proxy.org/"};
}

VersionUpdate::VersionUpdate() {

    m_game_dir = get_default_game_file_path();

    if (QString(APP_VERSION) == "dev") {
        m_local_version = QVersionNumber::fromString("0.0.1");
    } else {
        m_local_version = QVersionNumber::fromString(APP_VERSION);
        check_update("CubedTeam", "Cubed-Launcher");
    }
}

bool VersionUpdate::has_new_version() const { return m_new_version; }
bool VersionUpdate::download_finish() const { return m_download_finish; }
float VersionUpdate::download_progress() const { return m_download_progress; }
QString VersionUpdate::local_version() const {
    return m_local_version.toString();
}
QString VersionUpdate::remote_version() const {
    return m_remote_version.toString();
}
QString VersionUpdate::game_install_path() const { return m_game_dir; }

bool VersionUpdate::launcher_finish() const { return m_launcher_finish; }
float VersionUpdate::launcher_progress() const { return m_launcher_progress; }

Q_INVOKABLE void VersionUpdate::check_update(const QString& onwer,
                                             const QString& repo) {
    QUrl url(QString("https://api.github.com/repos/%1/%2/releases/latest")
                 .arg(onwer, repo));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      buildUserAgent().toUtf8());

    auto* replay = m_manager.get(request);

    connect(replay, &QNetworkReply::finished, this, [this]() {
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

        reply->deleteLater();

        emit remote_version_changed();
        emit new_version_changed();
    });
}

Q_INVOKABLE void VersionUpdate::download_from_github(bool use_mirror) {

    QString raw_url(
        "https://api.github.com/repos/CubedTeam/Cubed/releases/latest");

    QUrl url(raw_url);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      buildUserAgent().toUtf8());

    auto* reply = m_manager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, use_mirror]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            reply->deleteLater();
            m_download_progress = 1.0f;
            emit download_progress_changed();
            return;
        }

        auto doc = QJsonDocument::fromJson(reply->readAll());

        if (!doc.isObject()) {
            reply->deleteLater();
            m_download_progress = 1.0f;
            emit download_progress_changed();
            return;
        }

        auto assets = doc.object()["assets"].toArray();

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

        reply->deleteLater();

        if (download_url.isEmpty()) {
            qDebug() << "No Windows package found.";
            m_download_progress = 1.0f;
            emit download_progress_changed();
            return;
        }

        if (use_mirror) {
            download_url.prepend(mirror[0]);
        }

        download_game(download_url);
    });
}

Q_INVOKABLE void VersionUpdate::download_game(const QString& download_url) {
    if (std::exchange(m_downloading, true)) {
        return;
    }
    m_download_finish = false;
    emit download_finish_changed();
    if (download_url.isEmpty()) {
        qDebug() << "Error Download Url is empty";
        m_download_progress = 1.0f;
        emit download_progress_changed();
        return;
    }
    qDebug() << "Download url" << download_url;

    QNetworkRequest download_request(download_url);
    download_request.setHeader(QNetworkRequest::UserAgentHeader,
                               buildUserAgent().toUtf8());

    auto* download_reply = m_manager.get(download_request);

    QString zip_path = QDir::temp().filePath("Cubed-latest.zip");
    auto* file = new QFile(zip_path);
    if (!file->open(QIODevice::WriteOnly)) {
        qDebug() << "Can't open file";
        download_reply->abort();
        download_reply->deleteLater();
        delete file;
        m_download_progress = 1.0f;
        emit download_progress_changed();
        return;
    }

    connect(
        download_reply, &QNetworkReply::readyRead, this,
        [download_reply, file]() { file->write(download_reply->readAll()); });

    connect(download_reply, &QNetworkReply::finished, this,
            [download_reply, file, zip_path, this]() {
                file->close();
                delete file;

                if (download_reply->error() != QNetworkReply::NoError) {
                    qDebug() << download_reply->errorString();
                    download_reply->deleteLater();
                    m_download_progress = 1.0f;
                    emit download_progress_changed();
                    return;
                }

                qDebug() << "Download File completed! Zip Path" << zip_path;
                qDebug() << "Before extract zip_path" << zip_path
                         << "to game path" << m_game_dir;

                QFile check(zip_path);

                if (!check.open(QIODevice::ReadOnly)) {
                    qDebug() << "Can't open zip";
                    return;
                }

                if (check.size() < 100) {
                    qDebug() << "Downloaded file is invalid";
                    check.close();
                    return;
                }

                QByteArray header = check.read(4);
                qDebug() << header.toHex();

                check.close();

                QFile f(zip_path);
                qDebug() << "exists =" << f.exists();

                if (f.open(QIODevice::ReadOnly)) {
                    qDebug() << "QFile open OK";
                    qDebug() << "size =" << f.size();
                    f.close();
                } else {
                    qDebug() << "QFile open failed:" << f.errorString();
                }

                if (!QMicroz::extract(zip_path, m_game_dir)) {
                    qDebug() << "Extract file error";
                } else {
                    qDebug() << "Install Game Sucess";
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

Q_INVOKABLE void VersionUpdate::set_game_dir(const QString& game_file_dir) {
    QFileInfo info;

    if (game_file_dir.isEmpty()) {
        info = QFileInfo(get_default_game_file_path());
    } else {
        info = QFileInfo(game_file_dir);
    }

    m_game_dir = info.absolutePath();
    qDebug() << "VersionUpdate: Change game dir" << m_game_dir;
    emit game_install_path_changed();
}

Q_INVOKABLE void VersionUpdate::update_launcher(bool use_mirror) {
    if (std::exchange(m_launcher_downloading, true)) {

        return;
    }

    if (m_latest_launcher_link.isEmpty()) {
        qDebug() << "Download Url is Null";
        m_launcher_downloading = false;
        m_download_progress = 1.0f;
        emit download_progress_changed();
        return;
    }
    QString download_url = m_latest_launcher_link;
    if (use_mirror) {
        download_url.prepend(mirror[0]);
    }

    QNetworkRequest download_request(download_url);
    download_request.setHeader(QNetworkRequest::UserAgentHeader,
                               buildUserAgent().toUtf8());

    auto* download_reply = m_manager.get(download_request);

    QString setup_path =
        QDir::temp().filePath("CubedLauncher-setup-latest.exe");
    auto* file = new QFile(setup_path);
    if (!file->open(QIODevice::WriteOnly)) {
        qDebug() << "Can't open file";
        download_reply->abort();
        download_reply->deleteLater();
        delete file;

        m_launcher_progress = 1.0f;
        emit launcher_progress_changed();
        m_launcher_downloading = false;
        return;
    }

    connect(
        download_reply, &QNetworkReply::readyRead, this,
        [download_reply, file]() { file->write(download_reply->readAll()); });

    connect(download_reply, &QNetworkReply::finished, this,
            [download_reply, file, setup_path, this]() {
                file->close();
                delete file;

                if (download_reply->error() != QNetworkReply::NoError) {
                    qDebug() << download_reply->errorString();
                    download_reply->deleteLater();
                    m_launcher_progress = 1.0f;
                    emit launcher_progress_changed();
                    m_launcher_downloading = false;
                    return;
                }

                QFile check(setup_path);

                if (!check.open(QIODevice::ReadOnly)) {
                    qDebug() << "Can't open setup file";
                    m_launcher_progress = 1.0f;
                    emit launcher_progress_changed();
                    m_launcher_downloading = false;
                    return;
                }

                if (check.size() < 100) {
                    qDebug() << "Downloaded file is invalid";
                    check.close();
                    m_launcher_progress = 1.0f;
                    emit launcher_progress_changed();
                    m_launcher_downloading = false;
                    return;
                }
                qDebug() << "Download Finish Start Installing...";
                if (!QProcess::startDetached(setup_path)) {
                    qDebug() << "Error can't start Installing Program";
                }
                download_reply->deleteLater();
                m_launcher_progress = 1.0f;
                m_launcher_finish = true;
                emit launcher_finish_changed();
                emit launcher_progress_changed();
                QCoreApplication::quit();
            });

    connect(download_reply, &QNetworkReply::downloadProgress, this,
            [this](qint64 received, qint64 total) {
                if (total > 0) {
                    m_launcher_progress = float(received) / float(total);
                    emit launcher_progress_changed();
                }
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
