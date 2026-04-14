// ============================================================================
// HistoryDb.cpp — sqlite-backed run history
// ============================================================================

#include "HistoryDb.h"

#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QHash>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>

namespace ftd::testrunner {

// ---------------------------------------------------------------------------
// Schema (embedded so a fresh file auto-migrates). Bumping kSchemaVersion
// will later trigger the migration path in initSchema().
// ---------------------------------------------------------------------------

static constexpr int kSchemaVersion = 1;

static const char* const kCreateRunsSql = R"SQL(
CREATE TABLE IF NOT EXISTS runs (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    started_at    TEXT    NOT NULL,
    finished_at   TEXT,
    git_sha       TEXT,
    build_type    TEXT,
    cpu_workers   INTEGER,
    gpu_enabled   INTEGER,
    total_tests   INTEGER,
    pass_count    INTEGER,
    fail_count    INTEGER,
    wall_time_sec REAL
);
)SQL";

static const char* const kCreateResultsSql = R"SQL(
CREATE TABLE IF NOT EXISTS test_results (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    run_id       INTEGER REFERENCES runs(id),
    test_name    TEXT NOT NULL,
    category     TEXT,
    status       TEXT,
    duration_sec REAL,
    gpu_used     INTEGER,
    n_checks     INTEGER,
    n_fails      INTEGER,
    stderr_tail  TEXT
);
)SQL";

static const char* const kCreateVersionSql = R"SQL(
CREATE TABLE IF NOT EXISTS schema_version (
    version INTEGER NOT NULL
);
)SQL";

static const char* const kCreateIndexes[] = {
    "CREATE INDEX IF NOT EXISTS idx_test_results_run  ON test_results(run_id);",
    "CREATE INDEX IF NOT EXISTS idx_test_results_name ON test_results(test_name);",
    "CREATE INDEX IF NOT EXISTS idx_runs_started      ON runs(started_at);",
};

// ---------------------------------------------------------------------------
// ctor / dtor
// ---------------------------------------------------------------------------

HistoryDb::HistoryDb(const QString& dbPath)
    : m_dbPath(dbPath),
      m_connectionName(QStringLiteral("ftd_testrunner_") +
                       QUuid::createUuid().toString(QUuid::WithoutBraces)) {

    if (!QSqlDatabase::isDriverAvailable(QStringLiteral("QSQLITE"))) {
        qWarning() << "HistoryDb: QSQLITE driver unavailable — history disabled";
        return;
    }

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qWarning() << "HistoryDb: failed to open" << dbPath
                   << "—" << m_db.lastError().text();
        return;
    }

    // SQLite performance knobs: WAL + NORMAL synchronous keep writes fast
    // while still being crash-safe for our use case.
    {
        QSqlQuery pragma(m_db);
        pragma.exec(QStringLiteral("PRAGMA journal_mode=WAL;"));
        pragma.exec(QStringLiteral("PRAGMA synchronous=NORMAL;"));
        pragma.exec(QStringLiteral("PRAGMA foreign_keys=ON;"));
    }

    if (!initSchema()) {
        qWarning() << "HistoryDb: schema init failed";
        m_db.close();
        return;
    }

    m_open = true;
}

HistoryDb::~HistoryDb() {
    if (m_inRunTxn) {
        m_db.rollback();
        m_inRunTxn = false;
    }
    if (m_db.isOpen()) {
        m_db.close();
    }
    // Qt requires the name, not the QSqlDatabase handle, to be removed.
    QSqlDatabase::removeDatabase(m_connectionName);
}

// ---------------------------------------------------------------------------
// Schema
// ---------------------------------------------------------------------------

bool HistoryDb::initSchema() {
    QSqlQuery q(m_db);

    if (!q.exec(QString::fromLatin1(kCreateRunsSql))) {
        qWarning() << "HistoryDb: create runs failed —" << q.lastError().text();
        return false;
    }
    if (!q.exec(QString::fromLatin1(kCreateResultsSql))) {
        qWarning() << "HistoryDb: create test_results failed —"
                   << q.lastError().text();
        return false;
    }
    if (!q.exec(QString::fromLatin1(kCreateVersionSql))) {
        qWarning() << "HistoryDb: create schema_version failed —"
                   << q.lastError().text();
        return false;
    }
    for (const char* idx : kCreateIndexes) {
        if (!q.exec(QString::fromLatin1(idx))) {
            qWarning() << "HistoryDb: create index failed —"
                       << q.lastError().text();
            return false;
        }
    }

    // Ensure a version row exists.
    int existingVersion = -1;
    if (q.exec(QStringLiteral("SELECT version FROM schema_version LIMIT 1;"))) {
        if (q.next()) existingVersion = q.value(0).toInt();
    }
    if (existingVersion < 0) {
        q.prepare(QStringLiteral(
            "INSERT INTO schema_version (version) VALUES (:v);"));
        q.bindValue(QStringLiteral(":v"), kSchemaVersion);
        if (!q.exec()) {
            qWarning() << "HistoryDb: insert schema_version failed —"
                       << q.lastError().text();
            return false;
        }
    } else if (existingVersion < kSchemaVersion) {
        // Placeholder for future migrations.
        qWarning() << "HistoryDb: schema version" << existingVersion
                   << "< expected" << kSchemaVersion
                   << "— no migration registered yet";
    }

    return true;
}

// ---------------------------------------------------------------------------
// Timestamp helper
// ---------------------------------------------------------------------------

QString HistoryDb::nowIso8601() {
    return QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
}

// ---------------------------------------------------------------------------
// Run lifecycle
// ---------------------------------------------------------------------------

qint64 HistoryDb::startRun(const QString& gitSha,
                           const QString& buildType,
                           int cpuWorkers,
                           bool gpuEnabled,
                           int totalTests) {
    if (!m_open) return -1;

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "INSERT INTO runs "
        "(started_at, git_sha, build_type, cpu_workers, gpu_enabled, total_tests, "
        " pass_count, fail_count, wall_time_sec) "
        "VALUES (:started, :sha, :build, :cpu, :gpu, :total, 0, 0, 0.0);"));
    q.bindValue(QStringLiteral(":started"), nowIso8601());
    q.bindValue(QStringLiteral(":sha"), gitSha);
    q.bindValue(QStringLiteral(":build"), buildType);
    q.bindValue(QStringLiteral(":cpu"), cpuWorkers);
    q.bindValue(QStringLiteral(":gpu"), gpuEnabled ? 1 : 0);
    q.bindValue(QStringLiteral(":total"), totalTests);

    if (!q.exec()) {
        qWarning() << "HistoryDb::startRun: insert failed —"
                   << q.lastError().text();
        return -1;
    }

    const qint64 runId = q.lastInsertId().toLongLong();

    // Open a write transaction that finishRun() will commit. This groups
    // per-test inserts for much higher throughput on large suites.
    if (m_db.transaction()) {
        m_inRunTxn = true;
    } else {
        qWarning() << "HistoryDb::startRun: transaction() failed —"
                   << m_db.lastError().text();
    }

    return runId;
}

void HistoryDb::recordResult(qint64 runId, const TestResultRow& row) {
    if (!m_open || runId < 0) return;

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "INSERT INTO test_results "
        "(run_id, test_name, category, status, duration_sec, gpu_used, "
        " n_checks, n_fails, stderr_tail) "
        "VALUES (:run, :name, :cat, :status, :dur, :gpu, :checks, :fails, :tail);"));
    q.bindValue(QStringLiteral(":run"),    runId);
    q.bindValue(QStringLiteral(":name"),   row.testName);
    q.bindValue(QStringLiteral(":cat"),    row.category);
    q.bindValue(QStringLiteral(":status"), row.status);
    q.bindValue(QStringLiteral(":dur"),    row.durationSec);
    q.bindValue(QStringLiteral(":gpu"),    row.gpuUsed ? 1 : 0);
    q.bindValue(QStringLiteral(":checks"), row.nChecks);
    q.bindValue(QStringLiteral(":fails"),  row.nFails);
    q.bindValue(QStringLiteral(":tail"),   row.stderrTail);

    if (!q.exec()) {
        qWarning() << "HistoryDb::recordResult: insert failed for"
                   << row.testName << "—" << q.lastError().text();
    }
}

void HistoryDb::finishRun(qint64 runId,
                          int passCount,
                          int failCount,
                          double wallTimeSec) {
    if (!m_open || runId < 0) return;

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "UPDATE runs "
        "   SET finished_at   = :finished, "
        "       pass_count    = :pass, "
        "       fail_count    = :fail, "
        "       wall_time_sec = :wall "
        " WHERE id = :id;"));
    q.bindValue(QStringLiteral(":finished"), nowIso8601());
    q.bindValue(QStringLiteral(":pass"),     passCount);
    q.bindValue(QStringLiteral(":fail"),     failCount);
    q.bindValue(QStringLiteral(":wall"),     wallTimeSec);
    q.bindValue(QStringLiteral(":id"),       runId);

    if (!q.exec()) {
        qWarning() << "HistoryDb::finishRun: update failed —"
                   << q.lastError().text();
    }

    if (m_inRunTxn) {
        if (!m_db.commit()) {
            qWarning() << "HistoryDb::finishRun: commit failed —"
                       << m_db.lastError().text();
            m_db.rollback();
        }
        m_inRunTxn = false;
    }
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

static RunSummary unpackRun(QSqlQuery& q) {
    RunSummary r;
    r.id           = q.value(0).toLongLong();
    r.startedAt    = q.value(1).toString();
    r.finishedAt   = q.value(2).toString();
    r.gitSha       = q.value(3).toString();
    r.buildType    = q.value(4).toString();
    r.cpuWorkers   = q.value(5).toInt();
    r.gpuEnabled   = q.value(6).toInt() != 0;
    r.totalTests   = q.value(7).toInt();
    r.passCount    = q.value(8).toInt();
    r.failCount    = q.value(9).toInt();
    r.wallTimeSec  = q.value(10).toDouble();
    return r;
}

static const char* const kRunSelectCols =
    "id, started_at, finished_at, git_sha, build_type, cpu_workers, "
    "gpu_enabled, total_tests, pass_count, fail_count, wall_time_sec";

QList<RunSummary> HistoryDb::listRuns(int limit) {
    QList<RunSummary> out;
    if (!m_open) return out;

    QSqlQuery q(m_db);
    q.prepare(QString::fromLatin1(
        "SELECT %1 FROM runs ORDER BY id DESC LIMIT :lim;"
        ).arg(QString::fromLatin1(kRunSelectCols)));
    q.bindValue(QStringLiteral(":lim"), limit);

    if (!q.exec()) {
        qWarning() << "HistoryDb::listRuns: failed —"
                   << q.lastError().text();
        return out;
    }
    while (q.next()) out.append(unpackRun(q));
    return out;
}

RunSummary HistoryDb::getRun(qint64 runId) {
    RunSummary r;
    if (!m_open) return r;

    QSqlQuery q(m_db);
    q.prepare(QString::fromLatin1(
        "SELECT %1 FROM runs WHERE id = :id;"
        ).arg(QString::fromLatin1(kRunSelectCols)));
    q.bindValue(QStringLiteral(":id"), runId);

    if (!q.exec()) {
        qWarning() << "HistoryDb::getRun: failed —" << q.lastError().text();
        return r;
    }
    if (q.next()) r = unpackRun(q);
    return r;
}

static TestResultRow unpackResult(QSqlQuery& q) {
    TestResultRow r;
    r.id          = q.value(0).toLongLong();
    r.runId       = q.value(1).toLongLong();
    r.testName    = q.value(2).toString();
    r.category    = q.value(3).toString();
    r.status      = q.value(4).toString();
    r.durationSec = q.value(5).toDouble();
    r.gpuUsed     = q.value(6).toInt() != 0;
    r.nChecks     = q.value(7).toInt();
    r.nFails      = q.value(8).toInt();
    r.stderrTail  = q.value(9).toString();
    return r;
}

static const char* const kResultSelectCols =
    "id, run_id, test_name, category, status, duration_sec, gpu_used, "
    "n_checks, n_fails, stderr_tail";

QList<TestResultRow> HistoryDb::getRunResults(qint64 runId) {
    QList<TestResultRow> out;
    if (!m_open) return out;

    QSqlQuery q(m_db);
    q.prepare(QString::fromLatin1(
        "SELECT %1 FROM test_results WHERE run_id = :id ORDER BY test_name;"
        ).arg(QString::fromLatin1(kResultSelectCols)));
    q.bindValue(QStringLiteral(":id"), runId);

    if (!q.exec()) {
        qWarning() << "HistoryDb::getRunResults: failed —"
                   << q.lastError().text();
        return out;
    }
    while (q.next()) out.append(unpackResult(q));
    return out;
}

QList<TestResultRow> HistoryDb::getTestHistory(const QString& testName, int limit) {
    QList<TestResultRow> out;
    if (!m_open) return out;

    QSqlQuery q(m_db);
    q.prepare(QString::fromLatin1(
        "SELECT %1 FROM test_results "
        "WHERE test_name = :name "
        "ORDER BY id DESC LIMIT :lim;"
        ).arg(QString::fromLatin1(kResultSelectCols)));
    q.bindValue(QStringLiteral(":name"), testName);
    q.bindValue(QStringLiteral(":lim"),  limit);

    if (!q.exec()) {
        qWarning() << "HistoryDb::getTestHistory: failed —"
                   << q.lastError().text();
        return out;
    }
    while (q.next()) out.append(unpackResult(q));
    return out;
}

// ---------------------------------------------------------------------------
// Regression detection
// ---------------------------------------------------------------------------

QList<Regression> HistoryDb::findRegressions(qint64 runId) {
    QList<Regression> out;
    if (!m_open || runId < 0) return out;

    QSqlQuery q(m_db);
    // Pick the most recent COMPLETED run strictly before runId.
    q.prepare(QStringLiteral(
        "SELECT id FROM runs "
        "WHERE id < :id AND finished_at IS NOT NULL "
        "ORDER BY id DESC LIMIT 1;"));
    q.bindValue(QStringLiteral(":id"), runId);
    if (!q.exec() || !q.next()) {
        // No prior finished run — nothing to compare against.
        return out;
    }
    const qint64 priorId = q.value(0).toLongLong();
    return computeFlips(priorId, runId);
}

QList<Regression> HistoryDb::diffRuns(qint64 runIdA, qint64 runIdB) {
    if (!m_open) return {};
    // A = older, B = newer — so sign semantics match findRegressions. If
    // the caller passed them swapped, reorder here.
    if (runIdA > runIdB) std::swap(runIdA, runIdB);
    return computeFlips(runIdA, runIdB);
}

QList<Regression> HistoryDb::computeFlips(qint64 runIdA, qint64 runIdB) {
    QList<Regression> out;
    if (!m_open) return out;

    // Pull both runs' results into hash tables keyed by test name.
    QHash<QString, TestResultRow> aRows, bRows;
    {
        const QList<TestResultRow> a = getRunResults(runIdA);
        const QList<TestResultRow> b = getRunResults(runIdB);
        aRows.reserve(a.size());
        bRows.reserve(b.size());
        for (const TestResultRow& r : a) aRows.insert(r.testName, r);
        for (const TestResultRow& r : b) bRows.insert(r.testName, r);
    }

    // Walk the B side (current run) and compare.
    for (auto it = bRows.constBegin(); it != bRows.constEnd(); ++it) {
        const QString& name = it.key();
        const TestResultRow& cur = it.value();
        const auto ait = aRows.constFind(name);
        if (ait == aRows.constEnd()) continue;          // new test, skip

        const TestResultRow& prev = ait.value();
        if (prev.status == cur.status) continue;        // unchanged

        Regression r;
        r.testName    = name;
        r.prevStatus  = prev.status;
        r.curStatus   = cur.status;
        r.prevDuration = prev.durationSec;
        r.curDuration  = cur.durationSec;
        if (prev.status == QStringLiteral("pass") &&
            cur.status  == QStringLiteral("fail")) {
            r.sign = +1;
        } else if (prev.status == QStringLiteral("fail") &&
                   cur.status  == QStringLiteral("pass")) {
            r.sign = -1;
        } else {
            r.sign = 0;
        }
        out.append(r);
    }
    return out;
}

}  // namespace ftd::testrunner
