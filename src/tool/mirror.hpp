#pragma once
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QQmlEngine>
#include <QString>
#include <QStringList>
#include <QVector>
#include <qtmetamacros.h>

// AI-generated: index 0 is always direct GitHub. Order is part of the
// persisted Settings contract, so don't reorder existing entries.
struct MirrorEntry {
    QString name;
    QString prefix;
};

inline const QVector<MirrorEntry> mirror_sources{
    {QStringLiteral("Direct (GitHub)"), QStringLiteral("")},
    {QStringLiteral("gh-proxy.org"), QStringLiteral("https://gh-proxy.org/")},
    {QStringLiteral("v4.gh-proxy.org"),
     QStringLiteral("https://v4.gh-proxy.org/")},
    {QStringLiteral("v6.gh-proxy.org"),
     QStringLiteral("https://v6.gh-proxy.org/")},
    {QStringLiteral("cdn.gh-proxy.org"),
     QStringLiteral("https://cdn.gh-proxy.org/")},
    {QStringLiteral("gh.llkk.cc"), QStringLiteral("https://gh.llkk.cc/")},
    {QStringLiteral("ghproxy.cn"), QStringLiteral("https://ghproxy.cn/")},
    {QStringLiteral("ghproxy.net"), QStringLiteral("https://ghproxy.net/")},
    {QStringLiteral("gitproxy.click"),
     QStringLiteral("https://gitproxy.click/")},
    {QStringLiteral("github.tbedu.top"),
     QStringLiteral("https://github.tbedu.top/")},
    {QStringLiteral("github.moeyy.xyz"),
     QStringLiteral("https://github.moeyy.xyz/")},
    {QStringLiteral("gh.api.99988866.xyz"),
     QStringLiteral("https://gh.api.99988866.xyz/")},
    {QStringLiteral("mirror.ghproxy.com"),
     QStringLiteral("https://mirror.ghproxy.com/")},
};

// AI-generated: QML singleton exposing the mirror list and URL prefixing.
class MirrorSource : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QStringList names READ names CONSTANT)
public:
    explicit MirrorSource(QObject* parent = nullptr);
    QStringList names() const;
    Q_INVOKABLE QString apply(const QString& url, int index) const;
    // Emits latencyReady(index, ms) for each probe; ms < 0 means
    // timeout/failure. When force is false, re-emits cached results
    // from a previous round if still within TTL.
    Q_INVOKABLE void test_all_latency(bool force);
Q_SIGNALS:
    void latencyReady(int index, int ms);

private:
    QNetworkAccessManager m_manager;
    QJsonObject m_latency_cache;
    void probe(int index, const QString& url);
};