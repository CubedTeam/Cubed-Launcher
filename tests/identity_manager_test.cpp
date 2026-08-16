#include "tool/identity_manager.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

class IdentityManagerTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void import_creates_data_directory();
    void import_replaces_existing_identity();
    void invalid_import_preserves_existing_identity();
    void import_rejects_missing_directory_and_remote_sources();
    void export_copies_identity_exactly();
    void export_failure_preserves_existing_destination();
};

namespace {
bool write_file(const QString& path, const QByteArray& contents) {
    QFile file(path);
    return file.open(QIODevice::WriteOnly) &&
           file.write(contents) == contents.size();
}

QByteArray read_file(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return file.readAll();
}
} // namespace

void IdentityManagerTest::import_creates_data_directory() {
    QTemporaryDir temporary_directory;
    QVERIFY(temporary_directory.isValid());
    const QString source =
        QDir(temporary_directory.path()).filePath("source.json");
    const QByteArray contents = R"({"player":"alice","token":"secret"})";
    QVERIFY(write_file(source, contents));

    const QString data_directory =
        QDir(temporary_directory.path()).filePath("nested/Cubed");
    IdentityManager manager(data_directory);

    QVERIFY(manager.import_identity(QUrl::fromLocalFile(source)));
    QCOMPARE(read_file(manager.identity_path()), contents);
    QVERIFY(manager.error_message().isEmpty());
}

void IdentityManagerTest::import_replaces_existing_identity() {
    QTemporaryDir temporary_directory;
    QVERIFY(temporary_directory.isValid());
    const QString source =
        QDir(temporary_directory.path()).filePath("source.json");
    const QByteArray contents = R"({"player":"bob"})";
    QVERIFY(write_file(source, contents));

    IdentityManager manager(temporary_directory.path());
    QVERIFY(write_file(manager.identity_path(), R"({"player":"old"})"));

    QVERIFY(manager.import_identity(QUrl::fromLocalFile(source)));
    QCOMPARE(read_file(manager.identity_path()), contents);
}

void IdentityManagerTest::invalid_import_preserves_existing_identity() {
    QTemporaryDir temporary_directory;
    QVERIFY(temporary_directory.isValid());
    IdentityManager manager(temporary_directory.path());
    const QByteArray original = R"({"player":"original"})";
    QVERIFY(write_file(manager.identity_path(), original));

    const QString invalid =
        QDir(temporary_directory.path()).filePath("invalid.json");
    QVERIFY(write_file(invalid, "not json"));
    QVERIFY(!manager.import_identity(QUrl::fromLocalFile(invalid)));
    QCOMPARE(read_file(manager.identity_path()), original);

    const QString array =
        QDir(temporary_directory.path()).filePath("array.json");
    QVERIFY(write_file(array, "[]"));
    QVERIFY(!manager.import_identity(QUrl::fromLocalFile(array)));
    QCOMPARE(read_file(manager.identity_path()), original);
}

void IdentityManagerTest::
    import_rejects_missing_directory_and_remote_sources() {
    QTemporaryDir temporary_directory;
    QVERIFY(temporary_directory.isValid());
    IdentityManager manager(temporary_directory.path());

    const QString missing =
        QDir(temporary_directory.path()).filePath("missing.json");
    QVERIFY(!manager.import_identity(QUrl::fromLocalFile(missing)));
    QVERIFY(
        !manager.import_identity(QUrl("https://example.com/identity.json")));

    const QString directory =
        QDir(temporary_directory.path()).filePath("identity-directory");
    QVERIFY(QDir().mkpath(directory));
    QVERIFY(!manager.import_identity(QUrl::fromLocalFile(directory)));
    QVERIFY(!QFileInfo::exists(manager.identity_path()));
}

void IdentityManagerTest::export_copies_identity_exactly() {
    QTemporaryDir temporary_directory;
    QVERIFY(temporary_directory.isValid());
    IdentityManager manager(temporary_directory.path());
    const QByteArray contents = "{\n  \"player\": \"alice\"\n}\n";
    QVERIFY(write_file(manager.identity_path(), contents));

    const QString destination =
        QDir(temporary_directory.path()).filePath("backup.json");
    QVERIFY(manager.export_identity(QUrl::fromLocalFile(destination)));
    QCOMPARE(read_file(destination), contents);
}

void IdentityManagerTest::export_failure_preserves_existing_destination() {
    QTemporaryDir temporary_directory;
    QVERIFY(temporary_directory.isValid());
    IdentityManager manager(temporary_directory.path());
    const QString destination =
        QDir(temporary_directory.path()).filePath("backup.json");
    const QByteArray original = "existing backup";
    QVERIFY(write_file(destination, original));

    QVERIFY(!manager.export_identity(QUrl::fromLocalFile(destination)));
    QCOMPARE(read_file(destination), original);
    QVERIFY(!manager.export_identity(QUrl("https://example.com/backup.json")));
}

QTEST_MAIN(IdentityManagerTest)
#include "identity_manager_test.moc"
