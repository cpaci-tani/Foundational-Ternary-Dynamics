// ============================================================================
// NdjsonParser.h — incremental line-based NDJSON parser for test subprocesses
// ============================================================================
//
// Consumes the protocol defined in engine/include/ftd/test_telemetry.h and
// emits one `eventParsed()` signal per decoded line. Falls back to regex
// parsing of "  PASS  name" / "  FAIL  name" lines so non-instrumented legacy
// tests still surface check-level results in the UI.
//
// Usage:
//
//   NdjsonParser parser;
//   connect(&parser, &NdjsonParser::eventParsed, this, &Foo::handleEvent);
//   parser.feed(process.readAllStandardOutput());
//   parser.flush();  // at end of subprocess
//
// Thread-safety: NONE. Create one parser per subprocess, live on the GUI
// thread, and route QProcess::readyReadStandardOutput into feed().
// ----------------------------------------------------------------------------

#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QVariantMap>

namespace ftd::testrunner {

class NdjsonParser : public QObject {
    Q_OBJECT
public:
    explicit NdjsonParser(QObject* parent = nullptr);
    ~NdjsonParser() override;

    // Append raw stdout bytes to the internal buffer and parse any complete
    // lines. Residual partial-line bytes remain buffered.
    void feed(const QByteArray& chunk);

    // Parse anything left in the buffer as if it terminated in a newline
    // (call this when the subprocess has exited).
    void flush();

    // Clear any buffered input. Called by owners that recycle a parser.
    void reset();

signals:
    // Emitted once per completed NDJSON line (or per synthesised PASS/FAIL
    // legacy line). The map always carries an "event" key.
    void eventParsed(const QVariantMap& evt);

    // Emitted for a stdout line that is neither valid JSON nor a legacy
    // PASS/FAIL line. Useful for surfacing crash traces in the output panel
    // without losing the data.
    void rawLine(const QString& line);

private:
    // Parse one complete line (no trailing newline) and dispatch the right
    // signal.
    void parseLine(const QByteArray& line);

    QByteArray m_buf;
};

}  // namespace ftd::testrunner
