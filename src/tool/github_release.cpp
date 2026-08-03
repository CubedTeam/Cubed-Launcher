#include "tool/github_release.hpp"

#include "tool/user_agent.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

GithubReleaseFetcher::GithubReleaseFetcher(QNetworkAccessManager* manager,
                                           QStringView name, QObject* parent)
    : QObject(parent), m_manager(manager), m_name(name) {}

GithubReleaseFetcher::~GithubReleaseFetcher() {
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
}

bool GithubReleaseFetcher::fetch(const QString& owner, const QString& repo,
                                 const QRegularExpression& assetRegex,
                                 Callback callback) {
    if (m_reply) {
        return false;
    }
    m_callback = std::move(callback);

    const QUrl url(QString("https://api.github.com/repos/%1/%2/releases/latest")
                       .arg(owner, repo));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      buildUserAgent().toUtf8());

    m_reply = m_manager->get(request);
    connect(m_reply, &QNetworkReply::finished, this, [this, assetRegex]() {
        auto* reply = m_reply;
        m_reply = nullptr;
        auto cb = std::move(m_callback);
        if (!cb) {
            return;
        }

        Result result;
        if (!reply) {
            result.errorMessage = "GitHub release check: reply is null";
            cb(result);
            return;
        }
        if (reply->error() != QNetworkReply::NoError) {
            result.errorMessage = reply->errorString();
            reply->deleteLater();
            cb(result);
            return;
        }

        const QByteArray data = reply->readAll();
        reply->deleteLater();

        const QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isObject()) {
            result.errorMessage = "Invalid JSON response";
            cb(result);
            return;
        }
        const QJsonObject obj = doc.object();
        if (!obj.contains("tag_name")) {
            result.errorMessage = "No tag_name found in response";
            cb(result);
            return;
        }
        QString tag = obj["tag_name"].toString();
        if (tag.startsWith('v', Qt::CaseInsensitive)) {
            tag.remove(0, 1);
        }
        if (tag.isEmpty()) {
            result.errorMessage = "No tag_name found in response";
            cb(result);
            return;
        }

        const auto assets = obj["assets"].toArray();
        for (const auto& v : assets) {
            const auto asset = v.toObject();
            if (assetRegex.match(asset["name"].toString()).hasMatch()) {
                result.ok = true;
                result.version = tag;
                result.downloadUrl = asset["browser_download_url"].toString();
                cb(result);
                return;
            }
        }

        result.errorMessage = "No matching package found.";
        cb(result);
    });
    return true;
}
