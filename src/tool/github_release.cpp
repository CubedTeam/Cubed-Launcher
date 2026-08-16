#include "tool/github_release.hpp"

#include "tool/github_auth.hpp"
#include "tool/github_release_selector.hpp"
#include "tool/json_cache.hpp"
#include "tool/user_agent.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

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
                                 bool includePrereleases, Callback callback) {
    if (m_reply) {
        // AI-generated: replace stale checks when the channel changes.
        disconnect(m_reply, nullptr, this, nullptr);
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    m_callback = std::move(callback);
    const quint64 generation = ++m_request_generation;
    const QString cacheName =
        m_name + (includePrereleases ? QStringLiteral("-prerelease")
                                     : QStringLiteral("-stable"));

    constexpr qint64 CACHE_TTL_SECONDS = 3600;

    auto j = JsonCache::read(cacheName, CACHE_TTL_SECONDS);
    if (j) {
        Result result;
        result.ok = true;
        if (j->contains("download_url") && j->contains("version")) {
            result.downloadUrl = (*j)["download_url"].toString();
            result.version = (*j)["version"].toString();
            m_callback(result);
            return true;
        }
    }
    QUrl url(QString("https://api.github.com/repos/%1/%2/releases")
                 .arg(owner, repo));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("per_page"), QStringLiteral("100"));
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      buildUserAgent().toUtf8());
    GitHubAuth::apply_to_request(request);

    m_reply = m_manager->get(request);
    connect(m_reply, &QNetworkReply::finished, this,
            [this, assetRegex, includePrereleases, cacheName, generation]() {
                if (generation != m_request_generation) {
                    return;
                }
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
                if (doc.isNull() || !doc.isArray()) {
                    result.errorMessage = "Invalid JSON response";
                    cb(result);
                    return;
                }

                const auto selection = selectGithubRelease(
                    doc.array(), assetRegex, includePrereleases);
                if (selection) {
                    result.ok = true;
                    result.version = selection->version;
                    result.downloadUrl = selection->downloadUrl;
                    QJsonObject j;
                    j.insert("download_url", result.downloadUrl);
                    j.insert("version", result.version);
                    JsonCache::write(cacheName, j);
                    cb(result);
                    return;
                }

                result.errorMessage =
                    "No matching package with a valid semantic version found.";
                cb(result);
            });
    return true;
}
