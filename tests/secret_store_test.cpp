#include "tool/secret_store.hpp"

#include <QCoreApplication>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>

class SecretStoreTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void save_load_and_remove_without_application_metadata();
};

void SecretStoreTest::save_load_and_remove_without_application_metadata() {
    QTemporaryDir settings_directory;
    QVERIFY(settings_directory.isValid());
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                       settings_directory.path());

    const QString previous_organization = QCoreApplication::organizationName();
    const QString previous_application = QCoreApplication::applicationName();
    QCoreApplication::setOrganizationName({});
    QCoreApplication::setApplicationName({});

    const QString key = QStringLiteral("test_") +
                        QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QByteArray secret("webdav-password");
    QVERIFY(SecretStore::save(key, secret));
    QCOMPARE(SecretStore::load(key), std::optional<QByteArray>(secret));
    QVERIFY(SecretStore::remove(key));
    QVERIFY(!SecretStore::load(key).has_value());

    QCoreApplication::setOrganizationName(previous_organization);
    QCoreApplication::setApplicationName(previous_application);
}

QTEST_MAIN(SecretStoreTest)

#include "secret_store_test.moc"
