#include "tool/webdav_client.hpp"

#include "tool/user_agent.hpp"

#include <QAuthenticator>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <algorithm>

namespace {
// AI-generated: Limit credential use and downloaded backup size.
constexpr qsizetype kMaximumResponseSize = 1024 * 1024;

int effective_port(const QUrl& url) {
    return url.port(
        url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0
            ? 443
            : -1);
}
} // namespace

WebDavClient::WebDavClient(QObject* parent)
    : QObject(parent),
      m_owned_manager(std::make_unique<QNetworkAccessManager>()),
      m_manager(m_owned_manager.get()) {
    m_owned_manager->setParent(this);
    connect(m_manager, &QNetworkAccessManager::authenticationRequired, this,
            &WebDavClient::on_authentication_required);
}

WebDavClient::WebDavClient(QNetworkAccessManager* manager, QObject* parent)
    : QObject(parent), m_manager(manager) {
    Q_ASSERT(m_manager);
    connect(m_manager, &QNetworkAccessManager::authenticationRequired, this,
            &WebDavClient::on_authentication_required);
}

WebDavClient::~WebDavClient() {
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
    }
    clear_credentials();
}

bool WebDavClient::test_directory(const QUrl& directory_url,
                                  const QString& username,
                                  const QByteArray& password) {
    static const QByteArray body =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:propfind xmlns:d=\"DAV:\"><d:prop><d:resourcetype/>"
        "</d:prop></d:propfind>";
    return start(Operation::Test, QByteArrayLiteral("PROPFIND"), directory_url,
                 username, password, body);
}

bool WebDavClient::upload(const QUrl& file_url, const QString& username,
                          const QByteArray& password,
                          const QByteArray& contents) {
    return start(Operation::Upload, QByteArrayLiteral("PUT"), file_url,
                 username, password, contents);
}

bool WebDavClient::download(const QUrl& file_url, const QString& username,
                            const QByteArray& password) {
    return start(Operation::Download, QByteArrayLiteral("GET"), file_url,
                 username, password);
}

bool WebDavClient::start(Operation operation, const QByteArray& method,
                         const QUrl& url, const QString& username,
                         const QByteArray& password, const QByteArray& body) {
    if (busy()) {
        return false;
    }

    m_operation = operation;
    m_origin = url;
    m_username = username;
    m_password = password;
    m_response.clear();
    m_response_too_large = false;

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      buildUserAgent().toUtf8());
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::SameOriginRedirectPolicy);
    request.setTransferTimeout(30000);
    if (operation == Operation::Test) {
        request.setRawHeader(QByteArrayLiteral("Depth"),
                             QByteArrayLiteral("0"));
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("application/xml; charset=utf-8"));
    } else if (operation == Operation::Upload) {
        request.setHeader(
            QNetworkRequest::ContentTypeHeader,
            QStringLiteral("application/vnd.cubed.identity-backup+json"));
    }

    m_reply = m_manager->sendCustomRequest(request, method, body);
    QNetworkReply* reply = m_reply;
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        if (m_operation != Operation::Download || m_response_too_large) {
            reply->readAll();
            return;
        }
        m_response.append(reply->readAll());
        if (m_response.size() > kMaximumResponseSize) {
            m_response_too_large = true;
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::finished, this, &WebDavClient::on_finished);
    return true;
}

bool WebDavClient::same_origin(const QUrl& url) const {
    return url.scheme().compare(m_origin.scheme(), Qt::CaseInsensitive) == 0 &&
           url.host().compare(m_origin.host(), Qt::CaseInsensitive) == 0 &&
           effective_port(url) == effective_port(m_origin);
}

void WebDavClient::on_authentication_required(QNetworkReply* reply,
                                              QAuthenticator* authenticator) {
    if (reply != m_reply || !same_origin(reply->url())) {
        return;
    }
    authenticator->setUser(m_username);
    authenticator->setPassword(QString::fromUtf8(m_password));
}

void WebDavClient::on_finished() {
    QPointer<QNetworkReply> reply = m_reply;
    m_reply = nullptr;
    if (!reply) {
        clear_credentials();
        return;
    }
    if (m_operation == Operation::Download && !m_response_too_large) {
        m_response.append(reply->readAll());
    }

    const int status =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    Error error = Error::None;
    QString detail;
    if (m_response_too_large) {
        error = Error::ResponseTooLarge;
    } else if (status == 401 || status == 403) {
        error = Error::Authentication;
    } else if (status == 404 || status == 409) {
        error = Error::DirectoryMissing;
    } else if (reply->error() != QNetworkReply::NoError || status < 200 ||
               status >= 300) {
        error = Error::Network;
        detail = reply->errorString();
        if (detail.isEmpty() && status > 0) {
            detail = QStringLiteral("HTTP %1").arg(status);
        }
    }

    const Operation operation = m_operation;
    const QByteArray response =
        error == Error::None ? m_response : QByteArray{};
    reply->deleteLater();
    clear_credentials();
    Q_EMIT completed(operation, error, response, detail);
}

void WebDavClient::clear_credentials() {
    std::fill(m_password.begin(), m_password.end(), '\0');
    m_password.clear();
    m_username.clear();
    m_origin.clear();
    m_response.clear();
    m_response_too_large = false;
}
