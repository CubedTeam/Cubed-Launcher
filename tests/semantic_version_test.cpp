#include "tool/semantic_version.hpp"

#include <QtTest>

using namespace Qt::StringLiterals;

class SemanticVersionTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void parsesValidVersions();
    void rejectsInvalidVersions_data();
    void rejectsInvalidVersions();
    void followsSemverPrecedence();
    void ignoresBuildMetadataForPrecedence();
    void comparesLargeNumericIdentifiers();
};

void SemanticVersionTest::parsesValidVersions() {
    const auto prerelease = SemanticVersion::parse(u"v0.0.1-beta.1+build.007");
    QVERIFY(prerelease);
    QCOMPARE(prerelease->toString(), u"0.0.1-beta.1+build.007");
    QVERIFY(prerelease->isPrerelease());

    const auto stable = SemanticVersion::parse(u"V12.34.56");
    QVERIFY(stable);
    QCOMPARE(stable->toString(), u"12.34.56");
    QVERIFY(!stable->isPrerelease());
}

void SemanticVersionTest::rejectsInvalidVersions_data() {
    QTest::addColumn<QString>("version");
    const QStringList invalid = {
        QString(),           u"1"_s,        u"1.2"_s,    u"1.2.3.4"_s,
        u"01.2.3"_s,         u"1.02.3"_s,   u"1.2.03"_s, u"1.2.3-"_s,
        u"1.2.3-alpha..1"_s, u"1.2.3-01"_s, u"1.2.3+"_s, u"1.2.3+build..1"_s,
        u"1.2.3_foo"_s,      u" 1.2.3"_s,   u"1.2.3 "_s, u"vv1.2.3"_s,
        u"1.2.3+one+two"_s,
    };
    for (const QString& version : invalid) {
        QTest::newRow(version.toUtf8().constData()) << version;
    }
}

void SemanticVersionTest::rejectsInvalidVersions() {
    QFETCH(QString, version);
    QVERIFY(!SemanticVersion::parse(version));
}

void SemanticVersionTest::followsSemverPrecedence() {
    const QStringList ordered = {
        u"1.0.0-alpha"_s, u"1.0.0-alpha.1"_s, u"1.0.0-alpha.beta"_s,
        u"1.0.0-beta"_s,  u"1.0.0-beta.2"_s,  u"1.0.0-beta.11"_s,
        u"1.0.0-rc.1"_s,  u"1.0.0"_s,
    };
    for (qsizetype i = 1; i < ordered.size(); ++i) {
        const auto previous = SemanticVersion::parse(ordered[i - 1]);
        const auto current = SemanticVersion::parse(ordered[i]);
        QVERIFY(previous);
        QVERIFY(current);
        QVERIFY(*previous < *current);
    }
}

void SemanticVersionTest::ignoresBuildMetadataForPrecedence() {
    const auto left = SemanticVersion::parse(u"1.0.0+build.1");
    const auto right = SemanticVersion::parse(u"1.0.0+build.2");
    QVERIFY(left);
    QVERIFY(right);
    QCOMPARE(*left, *right);
}

void SemanticVersionTest::comparesLargeNumericIdentifiers() {
    const auto left = SemanticVersion::parse(
        u"999999999999999999999999999999.0.0-beta.99999999999999999999");
    const auto right = SemanticVersion::parse(
        u"1000000000000000000000000000000.0.0-beta.100000000000000000000");
    QVERIFY(left);
    QVERIFY(right);
    QVERIFY(*left < *right);
}

QTEST_APPLESS_MAIN(SemanticVersionTest)

#include "semantic_version_test.moc"
