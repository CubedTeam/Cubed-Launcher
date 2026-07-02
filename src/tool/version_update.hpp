#pragma once
#include "version.hpp"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QQmlEngine>
class VersionUpdate : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(
        bool hasNewVersion READ has_new_version NOTIFY new_version_changed)
    Q_PROPERTY(QString localVersion READ local_version CONSTANT)
    Q_PROPERTY(
        QString remoteVersion READ remote_version NOTIFY remote_version_changed)

public:
    VersionUpdate() {
        if (QString(APP_VERSION) == "dev") {
            m_local_version = QVersionNumber::fromString("0.0.1");
        } else {
            m_local_version = QVersionNumber::fromString(APP_VERSION);
            check_update();
        }
    }

    bool has_new_version() const { return m_new_version; }
    QString local_version() const { return m_local_version.toString(); }
    QString remote_version() const { return m_remote_version.toString(); }

    Q_INVOKABLE void check_update() {
        QUrl url(QString("https://api.github.com/repos/%1/%2/releases/latest")
                     .arg("CubedTeam", "Cubed-Launcher"));
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::UserAgentHeader,
                          buildUserAgent().toUtf8());

        QNetworkAccessManager* manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished, this,
                &VersionUpdate::on_reply_finished);
        manager->get(request);
    }

signals:

    void new_version_changed();
    void remote_version_changed();

private:
    bool m_new_version{false};

    QVersionNumber m_local_version;
    QVersionNumber m_remote_version;

    QString buildUserAgent() {
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

    void on_reply_finished(QNetworkReply* reply) {
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

        reply->deleteLater();

        emit remote_version_changed();
        emit new_version_changed();
    }
};