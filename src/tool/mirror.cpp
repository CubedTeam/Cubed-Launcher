#include "tool/mirror.hpp"

#include <QElapsedTimer>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

MirrorSource::MirrorSource(QObject* parent) : QObject(parent) {}

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
        emit latencyReady(index, ms);
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

void MirrorSource::test_all_latency() {
    for (int i = 0; i < mirror_sources.size(); ++i) {
        QString url = mirror_sources.at(i).prefix;
        if (url.isEmpty()) {
            url = QStringLiteral("https://github.com/");
        }
        probe(i, url);
    }
}