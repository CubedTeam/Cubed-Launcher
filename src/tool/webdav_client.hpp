#pragma once

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QPointer>
#include <QUrl>
#include <memory>

class QAuthenticator;
class QNetworkReply;

class WebDavClient : public QObject {
    Q_OBJECT

public:
    enum class Operation { Test, Upload, Download };
    enum class Error {
        None,
        Busy,
        Authentication,
        DirectoryMissing,
        ResponseTooLarge,
        Network,
    };

    explicit WebDavClient(QObject* parent = nullptr);
    explicit WebDavClient(QNetworkAccessManager* manager,
                          QObject* parent = nullptr);
    ~WebDavClient() override;

    bool busy() const { return m_reply != nullptr; }
    bool test_directory(const QUrl& directory_url, const QString& username,
                        const QByteArray& password);
    bool upload(const QUrl& file_url, const QString& username,
                const QByteArray& password, const QByteArray& contents);
    bool download(const QUrl& file_url, const QString& username,
                  const QByteArray& password);

Q_SIGNALS:
    void completed(WebDavClient::Operation operation, WebDavClient::Error error,
                   const QByteArray& contents, const QString& detail);

private:
    bool start(Operation operation, const QByteArray& method, const QUrl& url,
               const QString& username, const QByteArray& password,
               const QByteArray& body = {});
    bool same_origin(const QUrl& url) const;
    void on_authentication_required(QNetworkReply* reply,
                                    QAuthenticator* authenticator);
    void on_finished();
    void clear_credentials();

    std::unique_ptr<QNetworkAccessManager> m_owned_manager;
    QNetworkAccessManager* m_manager;
    QPointer<QNetworkReply> m_reply;
    Operation m_operation{Operation::Test};
    QUrl m_origin;
    QString m_username;
    QByteArray m_password;
    QByteArray m_response;
    bool m_response_too_large{false};
};

Q_DECLARE_METATYPE(WebDavClient::Operation)
Q_DECLARE_METATYPE(WebDavClient::Error)
