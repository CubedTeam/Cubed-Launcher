#include "tool/semantic_version.hpp"

#include <QChar>
#include <algorithm>
#include <utility>

namespace {
bool isAsciiDigit(QChar c) { return c >= u'0' && c <= u'9'; }

bool isIdentifierCharacter(QChar c) {
    return isAsciiDigit(c) || (c >= u'A' && c <= u'Z') ||
           (c >= u'a' && c <= u'z') || c == u'-';
}

bool isNumeric(QStringView value) {
    if (value.isEmpty()) {
        return false;
    }
    for (const QChar c : value) {
        if (!isAsciiDigit(c)) {
            return false;
        }
    }
    return true;
}

bool isValidNumericIdentifier(QStringView value) {
    return isNumeric(value) && (value.size() == 1 || value.front() != u'0');
}

bool isValidIdentifier(QStringView value) {
    if (value.isEmpty()) {
        return false;
    }
    for (const QChar c : value) {
        if (!isIdentifierCharacter(c)) {
            return false;
        }
    }
    return true;
}

std::optional<QStringList> parseIdentifiers(QStringView value,
                                            bool rejectLeadingZero) {
    if (value.isEmpty()) {
        return std::nullopt;
    }

    QStringList result;
    const auto identifiers = value.split(u'.');
    result.reserve(identifiers.size());
    for (const QStringView identifier : identifiers) {
        if (!isValidIdentifier(identifier) ||
            (rejectLeadingZero && isNumeric(identifier) &&
             identifier.size() > 1 && identifier.front() == u'0')) {
            return std::nullopt;
        }
        result.append(identifier.toString());
    }
    return result;
}

std::strong_ordering compareNumeric(QStringView left, QStringView right) {
    // AI-generated: compare numeric identifiers without integer conversion.
    if (left.size() != right.size()) {
        return left.size() < right.size() ? std::strong_ordering::less
                                          : std::strong_ordering::greater;
    }
    const int result = left.compare(right, Qt::CaseSensitive);
    if (result < 0) {
        return std::strong_ordering::less;
    }
    if (result > 0) {
        return std::strong_ordering::greater;
    }
    return std::strong_ordering::equal;
}
} // namespace

std::optional<SemanticVersion> SemanticVersion::parse(QStringView version) {
    if (version.startsWith(u'v') || version.startsWith(u'V')) {
        version = version.sliced(1);
    }
    if (version.isEmpty()) {
        return std::nullopt;
    }

    QStringView build;
    const qsizetype buildSeparator = version.indexOf(u'+');
    if (buildSeparator >= 0) {
        if (version.indexOf(u'+', buildSeparator + 1) >= 0) {
            return std::nullopt;
        }
        build = version.sliced(buildSeparator + 1);
        version = version.first(buildSeparator);
    }

    QStringView prerelease;
    const qsizetype prereleaseSeparator = version.indexOf(u'-');
    if (prereleaseSeparator >= 0) {
        prerelease = version.sliced(prereleaseSeparator + 1);
        version = version.first(prereleaseSeparator);
    }

    const auto core = version.split(u'.');
    if (core.size() != 3 || !isValidNumericIdentifier(core[0]) ||
        !isValidNumericIdentifier(core[1]) ||
        !isValidNumericIdentifier(core[2])) {
        return std::nullopt;
    }

    SemanticVersion result;
    result.m_major = core[0].toString();
    result.m_minor = core[1].toString();
    result.m_patch = core[2].toString();

    if (prereleaseSeparator >= 0) {
        auto identifiers = parseIdentifiers(prerelease, true);
        if (!identifiers) {
            return std::nullopt;
        }
        result.m_prerelease = std::move(*identifiers);
    }
    if (buildSeparator >= 0) {
        auto identifiers = parseIdentifiers(build, false);
        if (!identifiers) {
            return std::nullopt;
        }
        result.m_build = std::move(*identifiers);
    }
    return result;
}

QString SemanticVersion::toString() const {
    QString result = m_major + u'.' + m_minor + u'.' + m_patch;
    if (!m_prerelease.isEmpty()) {
        result += u'-' + m_prerelease.join(u'.');
    }
    if (!m_build.isEmpty()) {
        result += u'+' + m_build.join(u'.');
    }
    return result;
}

bool SemanticVersion::isPrerelease() const { return !m_prerelease.isEmpty(); }

std::strong_ordering
SemanticVersion::comparePrecedence(const SemanticVersion& other) const {
    for (const auto& pair :
         {std::pair{QStringView(m_major), QStringView(other.m_major)},
          std::pair{QStringView(m_minor), QStringView(other.m_minor)},
          std::pair{QStringView(m_patch), QStringView(other.m_patch)}}) {
        if (const auto result = compareNumeric(pair.first, pair.second);
            result != std::strong_ordering::equal) {
            return result;
        }
    }

    if (m_prerelease.isEmpty() || other.m_prerelease.isEmpty()) {
        if (m_prerelease.isEmpty() == other.m_prerelease.isEmpty()) {
            return std::strong_ordering::equal;
        }
        return m_prerelease.isEmpty() ? std::strong_ordering::greater
                                      : std::strong_ordering::less;
    }

    const qsizetype commonSize =
        std::min(m_prerelease.size(), other.m_prerelease.size());
    for (qsizetype i = 0; i < commonSize; ++i) {
        const QStringView left(m_prerelease[i]);
        const QStringView right(other.m_prerelease[i]);
        const bool leftNumeric = isNumeric(left);
        const bool rightNumeric = isNumeric(right);
        if (leftNumeric != rightNumeric) {
            return leftNumeric ? std::strong_ordering::less
                               : std::strong_ordering::greater;
        }

        std::strong_ordering result = std::strong_ordering::equal;
        if (leftNumeric) {
            result = compareNumeric(left, right);
        } else {
            const int lexical = left.compare(right, Qt::CaseSensitive);
            if (lexical < 0) {
                result = std::strong_ordering::less;
            } else if (lexical > 0) {
                result = std::strong_ordering::greater;
            }
        }
        if (result != std::strong_ordering::equal) {
            return result;
        }
    }

    if (m_prerelease.size() == other.m_prerelease.size()) {
        return std::strong_ordering::equal;
    }
    return m_prerelease.size() < other.m_prerelease.size()
               ? std::strong_ordering::less
               : std::strong_ordering::greater;
}

bool SemanticVersion::operator==(const SemanticVersion& other) const {
    return comparePrecedence(other) == std::strong_ordering::equal;
}

std::strong_ordering
SemanticVersion::operator<=>(const SemanticVersion& other) const {
    return comparePrecedence(other);
}
