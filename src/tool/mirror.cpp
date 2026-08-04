#include "tool/mirror.hpp"

#include "tool/json_cache.hpp"
#include "tool/log.hpp"

#include <QElapsedTimer>
#include <QMetaObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <optional>

MirrorSource::MirrorSource(QObject* parent) : QObject(parent) {}

namespace {
constexpr auto kLatencyCacheName = "MirrorLatency";
constexpr qint64 kLatencyCacheTtlSeconds = 3600;
} // namespace

QStringList MirrorSource::names() const {
    QStringList list;
    list.reserve(mirror_sources.size());
    for (const auto& entry : mirror_sources) {
        list.append(entry.name);
    }
    return list;
}

QString MirrorSource::apply(const QString& url, int index) const {
    if (index <= 0 || index >= mirror_sources.size()) {
        return url;
    }
    const QString& prefix = mirror_sources.at(index).prefix;
    if (prefix.isEmpty()) {
        return url;
    }
    return prefix + url;
}

// AI-generated: probe a single mirror. Emits latencyReady once with the
// elapsed ms on first bytes/headers, or -1 on timeout/network error.
void MirrorSource::probe(int index, const QString& url) {
    QNetworkRequest req(url);
    req.setRawHeader(
        "User-Agent",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
        "(KHTML, like Gecko) Chrome/120.0 Safari/537.36");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    req.setTransferTimeout(8000);
    auto* reply = m_manager.get(req);
    auto start = std::make_shared<QElapsedTimer>();
    start->start();
    auto measured = std::make_shared<bool>(false);

    auto finish = [this, reply, index, measured](int ms) {
        if (*measured) {
            return;
        }
        *measured = true;
        if (reply) {
            reply->disconnect();
            reply->deleteLater();
        }
        // AI-generated: only persist successful latencies; timeouts would
        // otherwise freeze as "failed" for the full TTL on every relaunch.
        if (ms >= 0) {
            m_latency_cache.insert(QString::number(index), ms);
            JsonCache::write(kLatencyCacheName, m_latency_cache);
        }
        emitLatencyReady(index, ms);
    };

    auto* watchdog = new QTimer(reply);
    watchdog->setSingleShot(true);
    watchdog->setInterval(8000);
    connect(watchdog, &QTimer::timeout, reply, [reply, finish]() {
        reply->abort();
        finish(-1);
    });
    watchdog->start();

    connect(reply, &QNetworkReply::metaDataChanged, reply,
            [start, finish]() { finish(int(start->elapsed())); });
    connect(reply, &QNetworkReply::readyRead, reply,
            [start, finish]() { finish(int(start->elapsed())); });
    connect(reply, &QNetworkReply::errorOccurred, reply,
            [finish](QNetworkReply::NetworkError) { finish(-1); });
    connect(reply, &QNetworkReply::finished, reply,
            [start, finish]() { finish(int(start->elapsed())); });
}

void MirrorSource::test_all_latency(bool force) {
    // AI-generated: replay valid cached entries first, then probe only the
    // indices that are missing or failed on the previous round.
    m_latency_cache = {};
    QVector<int> pending;
    std::optional<QJsonObject> cached;
    if (!force) {
        cached = JsonCache::read(kLatencyCacheName, kLatencyCacheTtlSeconds);
    }
    for (int i = 0; i < mirror_sources.size(); ++i) {
        bool ok = false;
        const int ms =
            cached ? cached->value(QString::number(i)).toVariant().toInt(&ok)
                   : 0;
        if (cached && ok && ms >= 0) {
            m_latency_cache.insert(QString::number(i), ms);
            emitLatencyReady(i, ms);
        } else {
            pending.append(i);
        }
    }
    Logger::info("Mirror Pending sum {}", pending.size());
    for (int i : pending) {
        QString url = mirror_sources.at(i).prefix;
        if (url.isEmpty()) {
            url = QStringLiteral("https://github.com/");
        }
        probe(i, url);
    }
}

// AI-generated: queued emission so the receiver (e.g. QML Connections) is
// always connected, even when callers fire synchronously during component
// completion.
void MirrorSource::emitLatencyReady(int index, int ms) {
    QMetaObject::invokeMethod(
        this, [this, index, ms]() { Q_EMIT latencyReady(index, ms); },
        Qt::QueuedConnection);
}