#include "tool/backup_crypto.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <array>
#include <memory>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace {
// AI-generated: Protect identity backups with authenticated encryption.
constexpr qsizetype kMaximumPlaintextSize = 512 * 1024;
constexpr qsizetype kMaximumEnvelopeSize = 1024 * 1024;
constexpr int kIterationCount = 600000;
constexpr int kKeySize = 32;
constexpr int kSaltSize = 16;
constexpr int kNonceSize = 12;
constexpr int kTagSize = 16;

using CipherContext =
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

struct KeyMaterial {
    std::array<unsigned char, kKeySize> bytes{};

    ~KeyMaterial() { OPENSSL_cleanse(bytes.data(), bytes.size()); }
};

bool derive_key(const QByteArray& passphrase, const QByteArray& salt,
                KeyMaterial& key) {
    return PKCS5_PBKDF2_HMAC(
               passphrase.constData(), static_cast<int>(passphrase.size()),
               reinterpret_cast<const unsigned char*>(salt.constData()),
               static_cast<int>(salt.size()), kIterationCount, EVP_sha256(),
               static_cast<int>(key.bytes.size()), key.bytes.data()) == 1;
}

QByteArray random_bytes(int size) {
    QByteArray bytes(size, Qt::Uninitialized);
    if (RAND_bytes(reinterpret_cast<unsigned char*>(bytes.data()), size) != 1) {
        return {};
    }
    return bytes;
}

QJsonObject create_envelope(const QByteArray& salt, const QByteArray& nonce,
                            const QByteArray& tag,
                            const QByteArray& ciphertext) {
    QJsonObject kdf;
    kdf.insert(QStringLiteral("name"), QStringLiteral("pbkdf2-hmac-sha256"));
    kdf.insert(QStringLiteral("iterations"), kIterationCount);
    kdf.insert(QStringLiteral("salt"), QString::fromLatin1(salt.toBase64()));

    QJsonObject cipher;
    cipher.insert(QStringLiteral("name"), QStringLiteral("aes-256-gcm"));
    cipher.insert(QStringLiteral("nonce"),
                  QString::fromLatin1(nonce.toBase64()));
    cipher.insert(QStringLiteral("tag"), QString::fromLatin1(tag.toBase64()));

    QJsonObject root;
    root.insert(QStringLiteral("format"),
                QStringLiteral("cubed-identity-backup"));
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("kdf"), kdf);
    root.insert(QStringLiteral("cipher"), cipher);
    root.insert(QStringLiteral("ciphertext"),
                QString::fromLatin1(ciphertext.toBase64()));
    return root;
}

bool decode_base64(const QJsonValue& value, QByteArray& output) {
    if (!value.isString()) {
        return false;
    }
    const auto result = QByteArray::fromBase64Encoding(
        value.toString().toLatin1(), QByteArray::AbortOnBase64DecodingErrors);
    if (result.decodingStatus != QByteArray::Base64DecodingStatus::Ok) {
        return false;
    }
    output = result.decoded;
    return true;
}
} // namespace

BackupCrypto::Result BackupCrypto::encrypt(const QByteArray& plaintext,
                                           const QByteArray& passphrase) {
    if (plaintext.isEmpty() || plaintext.size() > kMaximumPlaintextSize ||
        passphrase.isEmpty()) {
        return {{}, Error::InvalidInput};
    }

    const QByteArray salt = random_bytes(kSaltSize);
    const QByteArray nonce = random_bytes(kNonceSize);
    if (salt.size() != kSaltSize || nonce.size() != kNonceSize) {
        return {{}, Error::CryptoFailure};
    }

    KeyMaterial key;
    if (!derive_key(passphrase, salt, key)) {
        return {{}, Error::CryptoFailure};
    }

    CipherContext context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    if (!context ||
        EVP_EncryptInit_ex(context.get(), EVP_aes_256_gcm(), nullptr, nullptr,
                           nullptr) != 1 ||
        EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_SET_IVLEN, nonce.size(),
                            nullptr) != 1 ||
        EVP_EncryptInit_ex(
            context.get(), nullptr, nullptr, key.bytes.data(),
            reinterpret_cast<const unsigned char*>(nonce.constData())) != 1) {
        return {{}, Error::CryptoFailure};
    }

    QByteArray ciphertext(plaintext.size() + EVP_MAX_BLOCK_LENGTH,
                          Qt::Uninitialized);
    int written = 0;
    int final_written = 0;
    if (EVP_EncryptUpdate(
            context.get(), reinterpret_cast<unsigned char*>(ciphertext.data()),
            &written,
            reinterpret_cast<const unsigned char*>(plaintext.constData()),
            static_cast<int>(plaintext.size())) != 1 ||
        EVP_EncryptFinal_ex(
            context.get(),
            reinterpret_cast<unsigned char*>(ciphertext.data()) + written,
            &final_written) != 1) {
        return {{}, Error::CryptoFailure};
    }
    ciphertext.resize(written + final_written);

    QByteArray tag(kTagSize, Qt::Uninitialized);
    if (EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_GET_TAG, tag.size(),
                            tag.data()) != 1) {
        return {{}, Error::CryptoFailure};
    }

    const QByteArray envelope =
        QJsonDocument(create_envelope(salt, nonce, tag, ciphertext))
            .toJson(QJsonDocument::Compact);
    if (envelope.size() > kMaximumEnvelopeSize) {
        return {{}, Error::InvalidInput};
    }
    return {envelope, Error::None};
}

BackupCrypto::Result BackupCrypto::decrypt(const QByteArray& envelope,
                                           const QByteArray& passphrase) {
    if (envelope.isEmpty() || envelope.size() > kMaximumEnvelopeSize ||
        passphrase.isEmpty()) {
        return {{}, Error::InvalidInput};
    }

    QJsonParseError parse_error;
    const QJsonDocument document =
        QJsonDocument::fromJson(envelope, &parse_error);
    if (parse_error.error != QJsonParseError::NoError || !document.isObject()) {
        return {{}, Error::InvalidEnvelope};
    }

    const QJsonObject root = document.object();
    const QJsonObject kdf = root.value(QStringLiteral("kdf")).toObject();
    const QJsonObject cipher = root.value(QStringLiteral("cipher")).toObject();
    if (root.value(QStringLiteral("format")).toString() !=
            QStringLiteral("cubed-identity-backup") ||
        root.value(QStringLiteral("version")).toInt(-1) != 1 ||
        kdf.value(QStringLiteral("name")).toString() !=
            QStringLiteral("pbkdf2-hmac-sha256") ||
        kdf.value(QStringLiteral("iterations")).toInt(-1) != kIterationCount ||
        cipher.value(QStringLiteral("name")).toString() !=
            QStringLiteral("aes-256-gcm")) {
        return {{}, Error::InvalidEnvelope};
    }

    QByteArray salt;
    QByteArray nonce;
    QByteArray tag;
    QByteArray ciphertext;
    if (!decode_base64(kdf.value(QStringLiteral("salt")), salt) ||
        !decode_base64(cipher.value(QStringLiteral("nonce")), nonce) ||
        !decode_base64(cipher.value(QStringLiteral("tag")), tag) ||
        !decode_base64(root.value(QStringLiteral("ciphertext")), ciphertext) ||
        salt.size() != kSaltSize || nonce.size() != kNonceSize ||
        tag.size() != kTagSize || ciphertext.isEmpty() ||
        ciphertext.size() > kMaximumPlaintextSize) {
        return {{}, Error::InvalidEnvelope};
    }

    KeyMaterial key;
    if (!derive_key(passphrase, salt, key)) {
        return {{}, Error::CryptoFailure};
    }

    CipherContext context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    if (!context ||
        EVP_DecryptInit_ex(context.get(), EVP_aes_256_gcm(), nullptr, nullptr,
                           nullptr) != 1 ||
        EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_SET_IVLEN, nonce.size(),
                            nullptr) != 1 ||
        EVP_DecryptInit_ex(
            context.get(), nullptr, nullptr, key.bytes.data(),
            reinterpret_cast<const unsigned char*>(nonce.constData())) != 1) {
        return {{}, Error::CryptoFailure};
    }

    QByteArray plaintext(ciphertext.size(), Qt::Uninitialized);
    int written = 0;
    if (EVP_DecryptUpdate(
            context.get(), reinterpret_cast<unsigned char*>(plaintext.data()),
            &written,
            reinterpret_cast<const unsigned char*>(ciphertext.constData()),
            static_cast<int>(ciphertext.size())) != 1 ||
        EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_SET_TAG, tag.size(),
                            tag.data()) != 1) {
        return {{}, Error::CryptoFailure};
    }

    int final_written = 0;
    if (EVP_DecryptFinal_ex(context.get(),
                            reinterpret_cast<unsigned char*>(plaintext.data()) +
                                written,
                            &final_written) != 1) {
        return {{}, Error::AuthenticationFailed};
    }
    plaintext.resize(written + final_written);
    return {plaintext, Error::None};
}
