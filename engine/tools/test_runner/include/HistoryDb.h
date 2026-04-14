// ============================================================================
// HistoryDb.h — SQLite-backed run history for the FTD Test Bench
// ============================================================================
//
// Persists every test run (with per-test results) to a local sqlite file so
// users can diff two runs, see trend history, and be alerted when a
// previously-passing test starts failing.
//
// Two tables (plus a tiny schema_version table for migrations):
//   runs          — one row per invocation of the test bench
//   test_results  — one row per (run, test_name) pair
//
// This class is a pure database wrapper. It has no UI and no Qt widget
// dependencies beyond QSqlDatabase / QString / QList. HistoryTab owns a
// pointer into one of these via MainWindow.
//
// Threading: intended for the GUI thread only. QSqlDatabase is NOT thread-
// safe; callers must funnel all access through the owning thread.
//
// Error handling: methods return -1 / empty lists and qWarning on failure —
// no exceptions. Callers check isOpen() before relying on the database.
// ----------------------------------------------------------------------------

#pragma once

#include <QList>
#include <QSqlDatabase>
#include <QString>

namespace ftd::testrunner {

// --- POD rows ---------------------------------------------------------------

struct RunSummary {
    qint64 id = -1;
    QString startedAt;      // ISO 8601 with milliseconds & offset
    QString finishedAt;     // empty while still running
    QString gitSha;
    QString buildType;      // 'CPU' or 'CUDA' (opaque to HistoryDb)
    int cpuWorkers = 0;
    bool gpuEnabled = false;
    int totalTests = 0;
    int passCount = 0;
    int failCount = 0;
    double wallTimeSec = 0.0;
};

struct TestResultRow {
    qint64 id = -1;
    qint64 runId = -1;
    QString testName;
    QString category;
    QString status;         // 'pass', 'fail', 'timeout', 'crash', 'skipped'
    double durationSec = 0.0;
    bool gpuUsed = false;
    int nChecks = 0;
    int nFails = 0;
    QString stderrTail;     // last ~2KB, only populated on crash/fail
};

struct Regression {
    QString testName;
    QString prevStatus;
    QString curStatus;
    double prevDuration = 0.0;
    double curDuration = 0.0;
    // +1 = regression (pass -> fail), -1 = fix (fail -> pass), 0 = other flip
    int sign = 0;
};

// --- Database wrapper -------------------------------------------------------

class HistoryDb {
public:
    // Opens (or creates) the sqlite file at `dbPath`. The connection is held
    // internally under a UUID-tagged connection name so multiple instances
    // can coexist without clobbering each other's Qt database registry slot.
    explicit HistoryDb(const QString& dbPath);
    ~HistoryDb();

    // Non-copyable, non-movable — single owner per sqlite file.
    HistoryDb(const HistoryDb&) = delete;
    HistoryDb& operator=(const HistoryDb&) = delete;

    bool isOpen() const { return m_open; }
    QString dbPath() const { return m_dbPath; }

    // --- Run lifecycle ------------------------------------------------------

    // Inserts a new `runs` row with `started_at = now` and returns its id.
    // Returns -1 on error (check isOpen() and see qWarning output).
    qint64 startRun(const QString& gitSha,
                    const QString& buildType,
                    int cpuWorkers,
                    bool gpuEnabled,
                    int totalTests);

    // Appends one test_results row for the current run. Callers should
    // ideally batch these between startRun() and finishRun(), which is
    // achieved by the implicit transaction wrapped around the run by
    // finishRun() (see .cpp). No-op if runId == -1.
    void recordResult(qint64 runId, const TestResultRow& row);

    // Stamps finished_at, pass/fail/wall totals, and commits the transaction
    // (if one was begun).
    void finishRun(qint64 runId,
                   int passCount,
                   int failCount,
                   double wallTimeSec);

    // --- Queries ------------------------------------------------------------

    // Most recent `limit` runs, newest first. Empty list on error.
    QList<RunSummary> listRuns(int limit = 100);

    // Single run by id. id == -1 in the returned row indicates not found.
    RunSummary getRun(qint64 runId);

    // All per-test rows for `runId`, sorted by test_name.
    QList<TestResultRow> getRunResults(qint64 runId);

    // Historical row set for a specific test (newest first). Useful for
    // trend queries or future sparkline rendering.
    QList<TestResultRow> getTestHistory(const QString& testName, int limit = 50);

    // --- Regression detection ----------------------------------------------

    // Compares `runId` to the most recent PRIOR finished run (finished_at
    // NOT NULL) and returns the tests whose status changed. Only pass->fail
    // flips have sign = +1 ("regression"); fail->pass have sign = -1 ("fix");
    // other status transitions (pass->skip, etc.) get sign = 0.
    //
    // Callers interested only in regressions should filter on sign == +1.
    QList<Regression> findRegressions(qint64 runId);

    // Two-way diff between arbitrary runs. Same semantics as findRegressions
    // but with explicit run ids.
    QList<Regression> diffRuns(qint64 runIdA, qint64 runIdB);

private:
    // Create tables / indexes on first open, and apply schema migrations.
    bool initSchema();

    // Helper: current timestamp in ISO 8601 with milliseconds & offset.
    static QString nowIso8601();

    // Compare two run ids and return the status flips. Used by both
    // findRegressions and diffRuns.
    QList<Regression> computeFlips(qint64 runIdA, qint64 runIdB);

    QString        m_dbPath;
    QString        m_connectionName;
    QSqlDatabase   m_db;
    bool           m_open = false;

    // Tracks whether we are currently inside an explicit transaction opened
    // by startRun() — finishRun() will commit it.
    bool           m_inRunTxn = false;
};

}  // namespace ftd::testrunner
