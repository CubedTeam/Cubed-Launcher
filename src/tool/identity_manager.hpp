#pragma once

#include <QByteArray>
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QUrl>

class IdentityManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString identityPath READ identity_path CONSTANT)
    Q_PROPERTY(
        QString errorMessage READ error_message NOTIFY error_message_changed)

public:
    explicit IdentityManager(QObject* parent = nullptr);
    explicit IdentityManager(const QString& data_directory,
                             QObject* parent = nullptr);

    QString identity_path() const { return m_identity_path; }
    QString error_message() const { return m_error_message; }

    Q_INVOKABLE bool import_identity(const QUrl& source);
    Q_INVOKABLE bool export_identity(const QUrl& destination);

Q_SIGNALS:
    void error_message_changed();

private:
    bool has_identity_path();
    bool read_identity(const QString& path, QByteArray& contents);
    bool write_identity(const QString& path, const QByteArray& contents);
    void set_error(const QString& message);

    QString m_identity_path;
    QString m_error_message;
};
