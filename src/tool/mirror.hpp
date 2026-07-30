#pragma once
#include <QNetworkAccessManager>
#include <QQmlEngine>
#include <QString>
#include <QStringList>
#include <QVector>
#include <qtmetamacros.h>

// AI-generated: one mirror source. name is the UI label, prefix is prepended
// to the GitHub URL (empty for direct).
struct MirrorEntry {
    QString name;
    QString prefix;
};

// AI-generated: built-in mirrors. Index 0 is always direct GitHub. Order is
// part of the persisted Settings contract, so don't reorder existing entries.
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
    // AI-generated: prepend mirror prefix at index; 0 or out-of-range ==
    // direct.
    Q_INVOKABLE QString apply(const QString& url, int index) const;
    // AI-generated: probe every mirror and report round-trip ms per index.
    // Emits latencyReady(index, ms) for each; ms < 0 means timeout/failure.
    Q_INVOKABLE void test_all_latency();
signals:
    // AI-generated: fired once per mirror probe. ms<0 = unreachable.
    void latencyReady(int index, int ms);

private:
    // AI-generated: holds the probe request until it completes.
    QNetworkAccessManager m_manager;
    void probe(int index, const QString& url);
};