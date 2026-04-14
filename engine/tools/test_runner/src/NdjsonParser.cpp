// ============================================================================
// NdjsonParser.cpp — implementation
// ============================================================================

#include "NdjsonParser.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QString>

namespace ftd::testrunner {

NdjsonParser::NdjsonParser(QObject* parent) : QObject(parent) {}

NdjsonParser::~NdjsonParser() = default;

void NdjsonParser::feed(const QByteArray& chunk) {
    if (chunk.isEmpty()) return;
    m_buf.append(chunk);

    // Drain complete lines. NDJSON guarantees one object per "\n"-terminated
    // line, but we accept "\r\n" too (Windows QProcess pipes occasionally
    // preserve Windows line endings when the subprocess writes in text mode).
    while (true) {
        const int nl = m_buf.indexOf('\n');
        if (nl < 0) break;

        QByteArray line = m_buf.left(nl);
        m_buf.remove(0, nl + 1);

        // Strip trailing \r if present.
        if (!line.isEmpty() && line.endsWith('\r')) {
            line.chop(1);
        }
        parseLine(line);
    }
}

void NdjsonParser::flush() {
    if (m_buf.isEmpty()) return;
    QByteArray line = m_buf;
    m_buf.clear();
    if (!line.isEmpty() && line.endsWith('\r')) {
        line.chop(1);
    }
    parseLine(line);
}

void NdjsonParser::reset() {
    m_buf.clear();
}

void NdjsonParser::parseLine(const QByteArray& line) {
    if (line.isEmpty()) {
        return;
    }

    // Fast path: NDJSON lines start with '{'.
    if (line.startsWith('{')) {
        QJsonParseError err{};
        const QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            const QVariantMap map = doc.object().toVariantMap();
            if (map.contains(QStringLiteral("event"))) {
                emit eventParsed(map);
                return;
            }
        }
        // Fall through to raw on parse error so diagnostics still reach UI.
    }

    // Legacy fallback: "  PASS  name" or "  FAIL  name".
    //
    // The old hand-written test helpers printed exactly two leading spaces
    // followed by PASS/FAIL (see test_telemetry.h non-NDJSON branch). We
    // accept 2+ leading spaces to be generous.
    const QString text = QString::fromUtf8(line);
    static const QRegularExpression kCheckRe(
        QStringLiteral(R"(^\s{2,}(PASS|FAIL)\s+(.+?)\s*$)"));
    const QRegularExpressionMatch m = kCheckRe.match(text);
    if (m.hasMatch()) {
        const bool pass = (m.captured(1) == QStringLiteral("PASS"));
        QVariantMap evt;
        evt.insert(QStringLiteral("event"), QStringLiteral("check"));
        evt.insert(QStringLiteral("name"), m.captured(2));
        evt.insert(QStringLiteral("pass"), pass);
        evt.insert(QStringLiteral("legacy"), true);
        emit eventParsed(evt);
        return;
    }

    emit rawLine(text);
}

}  // namespace ftd::testrunner
