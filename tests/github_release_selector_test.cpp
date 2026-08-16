#include "tool/github_release_selector.hpp"

#include <QJsonObject>
#include <QtTest>

namespace {
QJsonObject release(QStringView version, bool prerelease,
                    QStringView assetName = u"Cubed-setup.exe") {
    QJsonObject asset;
    asset.insert("name", assetName.toString());
    asset.insert("browser_download_url",
                 QStringLiteral("https://example.invalid/") +
                     assetName.toString());

    QJsonObject result;
    result.insert("tag_name", version.toString());
    result.insert("prerelease", prerelease);
    result.insert("draft", false);
    result.insert("assets", QJsonArray{asset});
    return result;
}
} // namespace

class GithubReleaseSelectorTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void stableChannelRejectsPrereleases();
    void prereleaseChannelSelectsHighestVersion();
    void skipsInvalidAndUnusableReleases();
};

void GithubReleaseSelectorTest::stableChannelRejectsPrereleases() {
    const QJsonArray releases = {
        release(u"v2.0.0-beta.1", false),
        release(u"v1.9.0", true),
        release(u"v1.8.0", false),
    };
    const auto selected = selectGithubRelease(
        releases, QRegularExpression(QStringLiteral("Cubed-setup\\.exe")),
        false);
    QVERIFY(selected);
    QCOMPARE(selected->version, u"1.8.0");
}

void GithubReleaseSelectorTest::prereleaseChannelSelectsHighestVersion() {
    const QJsonArray releases = {
        release(u"v1.9.0", false),
        release(u"v2.0.0-beta.2", true),
        release(u"v2.0.0-beta.11", true),
        release(u"v2.0.0-alpha.1", true),
    };
    const auto selected = selectGithubRelease(
        releases, QRegularExpression(QStringLiteral("Cubed-setup\\.exe")),
        true);
    QVERIFY(selected);
    QCOMPARE(selected->version, u"2.0.0-beta.11");
}

void GithubReleaseSelectorTest::skipsInvalidAndUnusableReleases() {
    QJsonObject draft = release(u"v9.0.0", false);
    draft.insert("draft", true);
    const QJsonArray releases = {
        release(u"latest", false),
        draft,
        release(u"v3.0.0", false, u"Other.zip"),
        release(u"v2.1.0", false),
    };
    const auto selected = selectGithubRelease(
        releases, QRegularExpression(QStringLiteral("Cubed-setup\\.exe")),
        true);
    QVERIFY(selected);
    QCOMPARE(selected->version, u"2.1.0");
}

QTEST_APPLESS_MAIN(GithubReleaseSelectorTest)

#include "github_release_selector_test.moc"
