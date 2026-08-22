#include "tool/webdav_client.hpp"

#include <QAuthenticator>
#include <QBuffer>
#include <QNetworkReply>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>

class FakeReply : public QNetworkReply {
    Q_OBJECT

public:
    FakeReply(const QNetworkRequest& request, int status, QByteArray contents,
              QNetworkReply::NetworkError error, QObject* parent)
        : QNetworkReply(parent), m_contents(std::move(contents)),
          m_error(error) {
        setRequest(request);
        setUrl(request.url());
        setOperation(QNetworkAccessManager::CustomOperation);
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, status);
        open(QIODevice::ReadOnly | QIODevice::Unbuffered);
        QTimer::singleShot(1, this, [this]() {
            if (m_error != QNetworkReply::NoError) {
                setError(m_error, QStringLiteral("simulated network error"));
                Q_EMIT errorOccurred(m_error);
            }
            if (!m_contents.isEmpty()) {
                Q_EMIT readyRead();
            }
            setFinished(true);
            Q_EMIT finished();
        });
    }

    void abort() override { setError(OperationCanceledError, "aborted"); }
    qint64 bytesAvailable() const override {
        return m_contents.size() - m_offset + QNetworkReply::bytesAvailable();
    }

protected:
    qint64 readData(char* data, qint64 maximum_size) override {
        const qint64 remaining = m_contents.size() - m_offset;
        const qint64 count = std::min(maximum_size, remaining);
        if (count <= 0) {
            return -1;
        }
        std::copy_n(m_contents.constData() + m_offset, count, data);
        m_offset += count;
        return count;
    }

private:
    QByteArray m_contents;
    QNetworkReply::NetworkError m_error;
    qint64 m_offset{0};
};

class FakeNetworkAccessManager : public QNetworkAccessManager {
    Q_OBJECT

public:
    int response_status{207};
    QByteArray response_body;
    QNetworkReply::NetworkError response_error{QNetworkReply::NoError};
    QByteArray method;
    QNetworkRequest captured_request;
    QByteArray request_body;
    QNetworkReply* last_reply{nullptr};

protected:
    QNetworkReply* createRequest(Operation operation,
                                 const QNetworkRequest& request,
                                 QIODevice* outgoing_data) override {
        captured_request = request;
        method = request.attribute(QNetworkRequest::CustomVerbAttribute)
                     .toByteArray();
        if (operation == PutOperation) {
            method = "PUT";
        } else if (operation == GetOperation) {
            method = "GET";
        }
        if (outgoing_data) {
            request_body = outgoing_data->readAll();
        }
        last_reply = new FakeReply(request, response_status, response_body,
                                   response_error, this);
        return last_reply;
    }
};

class WebDavClientTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void propfind_request_and_success();
    void authentication_challenge_uses_configured_credentials();
    void upload_request_and_success();
    void download_returns_contents();
    void http_errors_are_mapped_data();
    void http_errors_are_mapped();
    void network_errors_are_mapped_data();
    void network_errors_are_mapped();
    void oversized_download_is_rejected();
};

void WebDavClientTest::propfind_request_and_success() {
    FakeNetworkAccessManager manager;
    WebDavClient client(&manager);
    QSignalSpy completed(&client, &WebDavClient::completed);
    const QUrl url("https://dav.example.com/backups/");

    QVERIFY(client.test_directory(url, "alice", "secret"));
    QTRY_COMPARE(completed.size(), 1);
    QCOMPARE(manager.method, QByteArray("PROPFIND"));
    QCOMPARE(manager.captured_request.url(), url);
    QCOMPARE(manager.captured_request.rawHeader("Depth"), QByteArray("0"));
    QCOMPARE(manager.captured_request.attribute(
                 QNetworkRequest::RedirectPolicyAttribute),
             QVariant::fromValue(QNetworkRequest::SameOriginRedirectPolicy));
    QVERIFY(manager.request_body.contains("resourcetype"));
    QCOMPARE(qvariant_cast<WebDavClient::Error>(completed.at(0).at(1)),
             WebDavClient::Error::None);
}

void WebDavClientTest::authentication_challenge_uses_configured_credentials() {
    FakeNetworkAccessManager manager;
    WebDavClient client(&manager);
    QSignalSpy completed(&client, &WebDavClient::completed);

    QVERIFY(client.test_directory(QUrl("https://dav.example.com/backups/"),
                                  "alice", "secret"));
    QAuthenticator authenticator;
    Q_EMIT manager.authenticationRequired(manager.last_reply, &authenticator);
    QCOMPARE(authenticator.user(), QString("alice"));
    QCOMPARE(authenticator.password(), QString("secret"));
    QTRY_COMPARE(completed.size(), 1);
}

void WebDavClientTest::upload_request_and_success() {
    FakeNetworkAccessManager manager;
    manager.response_status = 201;
    WebDavClient client(&manager);
    QSignalSpy completed(&client, &WebDavClient::completed);
    const QByteArray payload = R"({"ciphertext":"abc"})";

    QVERIFY(client.upload(
        QUrl("https://dav.example.com/backups/cubed-identity-backup.dat"),
        "alice", "secret", payload));
    QTRY_COMPARE(completed.size(), 1);
    QCOMPARE(manager.method, QByteArray("PUT"));
    QCOMPARE(manager.request_body, payload);
    QCOMPARE(manager.captured_request.header(QNetworkRequest::ContentTypeHeader)
                 .toString(),
             QString("application/vnd.cubed.identity-backup+json"));
}

void WebDavClientTest::download_returns_contents() {
    FakeNetworkAccessManager manager;
    manager.response_status = 200;
    manager.response_body = "encrypted-backup";
    WebDavClient client(&manager);
    QSignalSpy completed(&client, &WebDavClient::completed);

    QVERIFY(client.download(QUrl("https://dav.example.com/backup.dat"), "alice",
                            "secret"));
    QTRY_COMPARE(completed.size(), 1);
    QCOMPARE(manager.method, QByteArray("GET"));
    QCOMPARE(completed.at(0).at(2).toByteArray(),
             QByteArray("encrypted-backup"));
}

void WebDavClientTest::http_errors_are_mapped_data() {
    QTest::addColumn<int>("status");
    QTest::addColumn<WebDavClient::Error>("expected");
    QTest::newRow("unauthorized") << 401 << WebDavClient::Error::Authentication;
    QTest::newRow("forbidden") << 403 << WebDavClient::Error::Authentication;
    QTest::newRow("not found") << 404 << WebDavClient::Error::DirectoryMissing;
    QTest::newRow("missing parent")
        << 409 << WebDavClient::Error::DirectoryMissing;
    QTest::newRow("server error") << 500 << WebDavClient::Error::Network;
}

void WebDavClientTest::http_errors_are_mapped() {
    QFETCH(int, status);
    QFETCH(WebDavClient::Error, expected);
    FakeNetworkAccessManager manager;
    manager.response_status = status;
    WebDavClient client(&manager);
    QSignalSpy completed(&client, &WebDavClient::completed);

    QVERIFY(client.test_directory(QUrl("https://dav.example.com/backups/"),
                                  "alice", "secret"));
    QTRY_COMPARE(completed.size(), 1);
    QCOMPARE(qvariant_cast<WebDavClient::Error>(completed.at(0).at(1)),
             expected);
}

void WebDavClientTest::network_errors_are_mapped_data() {
    QTest::addColumn<QNetworkReply::NetworkError>("network_error");
    QTest::newRow("timeout") << QNetworkReply::TimeoutError;
    QTest::newRow("tls") << QNetworkReply::SslHandshakeFailedError;
}

void WebDavClientTest::network_errors_are_mapped() {
    QFETCH(QNetworkReply::NetworkError, network_error);
    FakeNetworkAccessManager manager;
    manager.response_status = 0;
    manager.response_error = network_error;
    WebDavClient client(&manager);
    QSignalSpy completed(&client, &WebDavClient::completed);

    QVERIFY(client.test_directory(QUrl("https://dav.example.com/backups/"),
                                  "alice", "secret"));
    QTRY_COMPARE(completed.size(), 1);
    QCOMPARE(qvariant_cast<WebDavClient::Error>(completed.at(0).at(1)),
             WebDavClient::Error::Network);
}

void WebDavClientTest::oversized_download_is_rejected() {
    FakeNetworkAccessManager manager;
    manager.response_status = 200;
    manager.response_body = QByteArray(1024 * 1024 + 1, 'x');
    WebDavClient client(&manager);
    QSignalSpy completed(&client, &WebDavClient::completed);

    QVERIFY(client.download(QUrl("https://dav.example.com/backup.dat"), "alice",
                            "secret"));
    QTRY_COMPARE(completed.size(), 1);
    QCOMPARE(qvariant_cast<WebDavClient::Error>(completed.at(0).at(1)),
             WebDavClient::Error::ResponseTooLarge);
    QVERIFY(completed.at(0).at(2).toByteArray().isEmpty());
}

QTEST_MAIN(WebDavClientTest)
#include "webdav_client_test.moc"
