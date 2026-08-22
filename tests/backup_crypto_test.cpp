#include "tool/backup_crypto.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

class BackupCryptoTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void round_trip_preserves_identity();
    void encryption_uses_random_parameters();
    void wrong_passphrase_is_rejected();
    void tampering_is_rejected();
    void malformed_and_unknown_envelopes_are_rejected();
    void oversized_input_is_rejected();
};

void BackupCryptoTest::round_trip_preserves_identity() {
    const QByteArray identity = R"({"player":"alice","token":"very-secret"})";
    const QByteArray passphrase = "correct horse battery staple";

    const BackupCrypto::Result encrypted =
        BackupCrypto::encrypt(identity, passphrase);
    QVERIFY(encrypted);
    QVERIFY(!encrypted.data.contains("very-secret"));

    const BackupCrypto::Result decrypted =
        BackupCrypto::decrypt(encrypted.data, passphrase);
    QVERIFY(decrypted);
    QCOMPARE(decrypted.data, identity);
}

void BackupCryptoTest::encryption_uses_random_parameters() {
    const QByteArray identity = R"({"player":"alice"})";
    const QByteArray passphrase = "correct horse battery staple";

    const auto first = BackupCrypto::encrypt(identity, passphrase);
    const auto second = BackupCrypto::encrypt(identity, passphrase);
    QVERIFY(first);
    QVERIFY(second);
    QVERIFY(first.data != second.data);
}

void BackupCryptoTest::wrong_passphrase_is_rejected() {
    const auto encrypted = BackupCrypto::encrypt(
        R"({"player":"alice"})", "correct horse battery staple");
    QVERIFY(encrypted);

    const auto decrypted =
        BackupCrypto::decrypt(encrypted.data, "different secret phrase");
    QCOMPARE(decrypted.error, BackupCrypto::Error::AuthenticationFailed);
    QVERIFY(decrypted.data.isEmpty());
}

void BackupCryptoTest::tampering_is_rejected() {
    const QByteArray passphrase = "correct horse battery staple";
    const auto encrypted =
        BackupCrypto::encrypt(R"({"player":"alice"})", passphrase);
    QVERIFY(encrypted);

    QJsonDocument document = QJsonDocument::fromJson(encrypted.data);
    QJsonObject root = document.object();
    QByteArray ciphertext =
        QByteArray::fromBase64(root.value("ciphertext").toString().toLatin1());
    ciphertext[0] = static_cast<char>(ciphertext[0] ^ 0x01);
    root.insert("ciphertext", QString::fromLatin1(ciphertext.toBase64()));

    const auto decrypted = BackupCrypto::decrypt(
        QJsonDocument(root).toJson(QJsonDocument::Compact), passphrase);
    QCOMPARE(decrypted.error, BackupCrypto::Error::AuthenticationFailed);
}

void BackupCryptoTest::malformed_and_unknown_envelopes_are_rejected() {
    QCOMPARE(BackupCrypto::decrypt("not json", "long enough passphrase").error,
             BackupCrypto::Error::InvalidEnvelope);

    const QByteArray passphrase = "correct horse battery staple";
    const auto encrypted =
        BackupCrypto::encrypt(R"({"player":"alice"})", passphrase);
    QVERIFY(encrypted);
    QJsonDocument document = QJsonDocument::fromJson(encrypted.data);
    QJsonObject root = document.object();
    root.insert("version", 2);
    QCOMPARE(BackupCrypto::decrypt(
                 QJsonDocument(root).toJson(QJsonDocument::Compact), passphrase)
                 .error,
             BackupCrypto::Error::InvalidEnvelope);
}

void BackupCryptoTest::oversized_input_is_rejected() {
    const QByteArray oversized(512 * 1024 + 1, 'x');
    QCOMPARE(BackupCrypto::encrypt(oversized, "long enough passphrase").error,
             BackupCrypto::Error::InvalidInput);
    QCOMPARE(BackupCrypto::decrypt(QByteArray(1024 * 1024 + 1, 'x'),
                                   "long enough passphrase")
                 .error,
             BackupCrypto::Error::InvalidInput);
}

QTEST_MAIN(BackupCryptoTest)
#include "backup_crypto_test.moc"
