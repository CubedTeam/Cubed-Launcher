#pragma once

#include <QByteArray>

namespace BackupCrypto {

enum class Error {
    None,
    InvalidInput,
    InvalidEnvelope,
    AuthenticationFailed,
    CryptoFailure,
};

struct Result {
    QByteArray data;
    Error error{Error::None};

    explicit operator bool() const { return error == Error::None; }
};

Result encrypt(const QByteArray& plaintext, const QByteArray& passphrase);
Result decrypt(const QByteArray& envelope, const QByteArray& passphrase);

} // namespace BackupCrypto
