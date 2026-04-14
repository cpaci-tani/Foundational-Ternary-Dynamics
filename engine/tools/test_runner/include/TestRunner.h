// ============================================================================
// TestRunner.h — QProcess-per-test subprocess launcher for FTD Test Bench
// ============================================================================
//
// Spawns each test binary as a child QProcess, routes its stdout through a
// dedicated NdjsonParser, and re-emits structured events as Qt signals.
// Handles both instrumented tests (FTD_TEST_TELEMETRY=1 NDJSON stream) and
// legacy tests (PASS/FAIL regex fallback).
//
// The runner owns the QProcess / NdjsonParser pair for each active test.
// `stopAll()` kills every running subprocess (used by the Stop button in
// MainWindow). Individual subprocess failures are surfaced as `testFinished`
// events with a synthesised failure count, so the dispatcher can keep draining
// the queue without special-case plumbing.
// ----------------------------------------------------------------------------

#pragma once

#include <QHash>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QVariantMap>

namespace ftd::testrunner {

class NdjsonParser;

struct RunningTest {
    QProcess* process = nullptr;
    NdjsonParser* parser = nullptr;
    QString name;
    qint64 startMs = 0;
};

class TestRunner : public QObject {
    Q_OBJECT
public:
    explicit TestRunner(QObject* parent = nullptr);
    ~TestRunner() override;

    // Launch a single test. `execPath` is the absolute path to the test
    // binary. When `useTelemetry` is true, FTD_TEST_TELEMETRY=1 is injected
    // into the environment and the runner expects NDJSON output.
    void runTest(const QString& testName,
                 const QString& execPath,
                 bool useTelemetry = true);

    // Ask every running subprocess to terminate. Waits briefly for graceful
    // exit, then kills. Safe to call from any slot.
    void stopAll();

    // Number of currently running subprocesses (used by SmartDispatcher to
    // enforce the GPU/CPU concurrency split).
    int runningCount() const { return m_running.size(); }

    // Returns true if a test by this name is currently executing.
    bool isRunning(const QString& testName) const;

signals:
    void testStarted(const QString& testName, qint64 pid);
    void checkReceived(const QString& testName, const QVariantMap& evt);
    void metricReceived(const QString& testName, const QVariantMap& evt);
    void snapshotReceived(const QString& testName, const QVariantMap& evt);
    void sectionReceived(const QString& testName, const QString& sectionName);
    void rawLineReceived(const QString& testName, const QString& line);

    // `failures` is authoritative if the test emitted a telemetry "end"
    // event. Otherwise the runner synthesizes it from regex fallback passes
    // /failures plus exit code. `durationSec` is wall-clock measured by the
    // runner.
    void testFinished(const QString& testName,
                      int failures,
                      double durationSec,
                      int exitCode);

private slots:
    void onProcessStdout();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onProcessErrorOccurred(QProcess::ProcessError error);
    void onParserEvent(const QVariantMap& evt);

private:
    // Lookup "which test does this QProcess belong to?" — QProcess is stored
    // as sender() and we need the RunningTest entry by pointer.
    RunningTest* findBySender(QObject* senderObj);
    void cleanupAndNotify(const QString& testName,
                          int failures,
                          double durationSec,
                          int exitCode);

    // Per-test accumulated failure count used when telemetry is off
    // (synthesised from legacy PASS/FAIL regex matches).
    QHash<QString, int> m_legacyFailures;

    // Running subprocesses keyed by test name.
    QHash<QString, RunningTest> m_running;
};

}  // namespace ftd::testrunner
