// ============================================================================
// TestRunner.cpp — QProcess-based subprocess launcher
// ============================================================================

#include "TestRunner.h"
#include "NdjsonParser.h"

#include <QDateTime>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QVariantMap>

namespace ftd::testrunner {

TestRunner::TestRunner(QObject* parent) : QObject(parent) {}

TestRunner::~TestRunner() {
    stopAll();
}

bool TestRunner::isRunning(const QString& testName) const {
    return m_running.contains(testName);
}

void TestRunner::runTest(const QString& testName,
                         const QString& execPath,
                         bool useTelemetry) {
    if (m_running.contains(testName)) {
        return;  // Already running.
    }

    QProcess* proc = new QProcess(this);

    // Merge stdout+stderr so the NDJSON parser also sees crash messages and
    // the output panel gets everything.
    proc->setProcessChannelMode(QProcess::MergedChannels);

    // Set working directory to the binary's containing directory so tests
    // that look for resources relative to CWD keep working.
    const QFileInfo fi(execPath);
    proc->setWorkingDirectory(fi.absolutePath());

    // Environment.
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (useTelemetry) {
        env.insert(QStringLiteral("FTD_TEST_TELEMETRY"), QStringLiteral("1"));
    } else {
        env.remove(QStringLiteral("FTD_TEST_TELEMETRY"));
    }
    proc->setProcessEnvironment(env);

    // Parser.
    NdjsonParser* parser = new NdjsonParser(proc);
    connect(parser, &NdjsonParser::eventParsed,
            this, &TestRunner::onParserEvent);
    connect(parser, &NdjsonParser::rawLine, this,
            [this, testName](const QString& line) {
                emit rawLineReceived(testName, line);
            });

    RunningTest entry;
    entry.process = proc;
    entry.parser = parser;
    entry.name = testName;
    entry.startMs = QDateTime::currentMSecsSinceEpoch();
    m_running.insert(testName, entry);
    m_legacyFailures.insert(testName, 0);

    connect(proc, &QProcess::readyReadStandardOutput,
            this, &TestRunner::onProcessStdout);
    connect(proc,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TestRunner::onProcessFinished);
    connect(proc, &QProcess::errorOccurred,
            this, &TestRunner::onProcessErrorOccurred);

    // Use the absolute exec path as the command (argv[0]) with no extra args —
    // CTest populates the full command, but for a Phase 3 runner the test's
    // main() does not require them.
    proc->setProgram(execPath);
    proc->setArguments({});
    proc->start();
}

void TestRunner::stopAll() {
    const QList<QString> names = m_running.keys();
    for (const QString& n : names) {
        RunningTest& rt = m_running[n];
        if (rt.process && rt.process->state() != QProcess::NotRunning) {
            rt.process->terminate();
            if (!rt.process->waitForFinished(1500)) {
                rt.process->kill();
                rt.process->waitForFinished(500);
            }
        }
    }
}

// ============================================================================
// QProcess slots
// ============================================================================

RunningTest* TestRunner::findBySender(QObject* senderObj) {
    for (auto it = m_running.begin(); it != m_running.end(); ++it) {
        if (it.value().process == senderObj ||
            it.value().parser == senderObj) {
            return &it.value();
        }
    }
    return nullptr;
}

void TestRunner::onProcessStdout() {
    QProcess* p = qobject_cast<QProcess*>(sender());
    if (!p) return;
    RunningTest* rt = findBySender(p);
    if (!rt || !rt->parser) return;
    rt->parser->feed(p->readAllStandardOutput());
}

void TestRunner::onProcessFinished(int exitCode, QProcess::ExitStatus status) {
    QProcess* p = qobject_cast<QProcess*>(sender());
    if (!p) return;
    RunningTest* rt = findBySender(p);
    if (!rt) return;

    const QString testName = rt->name;

    // Drain any residual bytes + partial line.
    if (rt->parser) {
        rt->parser->feed(p->readAllStandardOutput());
        rt->parser->flush();
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const double durationSec = std::max(0.0, (now - rt->startMs) / 1000.0);

    // If we never saw an NDJSON "end" event, synthesise failures from legacy
    // counters + process exit status.
    int failures = m_legacyFailures.value(testName, 0);
    if (status != QProcess::NormalExit) {
        // Crash or terminated: bump failure count so UI marks it red.
        if (failures == 0) failures = 1;
    } else if (exitCode != 0 && failures == 0) {
        // Process exited non-zero without emitting any check event.
        failures = exitCode;
    }

    cleanupAndNotify(testName, failures, durationSec, exitCode);
}

void TestRunner::onProcessErrorOccurred(QProcess::ProcessError error) {
    QProcess* p = qobject_cast<QProcess*>(sender());
    if (!p) return;
    RunningTest* rt = findBySender(p);
    if (!rt) return;

    const QString testName = rt->name;

    // QProcess::FailedToStart is a common blocker we need to surface. Other
    // errors (Crashed, Timedout) are redundant with finished() — skip those.
    if (error != QProcess::FailedToStart) return;

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const double durationSec = std::max(0.0, (now - rt->startMs) / 1000.0);
    cleanupAndNotify(testName, /*failures=*/1, durationSec, /*exitCode=*/-1);
}

void TestRunner::onParserEvent(const QVariantMap& evt) {
    NdjsonParser* parser = qobject_cast<NdjsonParser*>(sender());
    if (!parser) return;

    // Find the owning RunningTest by parser pointer.
    QString testName;
    for (auto it = m_running.cbegin(); it != m_running.cend(); ++it) {
        if (it.value().parser == parser) {
            testName = it.key();
            break;
        }
    }
    if (testName.isEmpty()) return;

    const QString eventType = evt.value(QStringLiteral("event")).toString();

    if (eventType == QStringLiteral("start")) {
        const qint64 pid = evt.value(QStringLiteral("pid")).toLongLong();
        emit testStarted(testName, pid);
    } else if (eventType == QStringLiteral("section")) {
        emit sectionReceived(testName, evt.value(QStringLiteral("name")).toString());
    } else if (eventType == QStringLiteral("check")) {
        if (!evt.value(QStringLiteral("pass")).toBool()) {
            m_legacyFailures[testName] += 1;
        }
        emit checkReceived(testName, evt);
    } else if (eventType == QStringLiteral("metric")) {
        emit metricReceived(testName, evt);
    } else if (eventType == QStringLiteral("tick")) {
        emit metricReceived(testName, evt);
    } else if (eventType == QStringLiteral("snapshot")) {
        emit snapshotReceived(testName, evt);
    } else if (eventType == QStringLiteral("end")) {
        // Authoritative failure count from the test itself. Overwrite the
        // legacy counter so onProcessFinished picks it up.
        const int failures = evt.value(QStringLiteral("failures"), 0).toInt();
        m_legacyFailures[testName] = failures;
    }
}

void TestRunner::cleanupAndNotify(const QString& testName,
                                  int failures,
                                  double durationSec,
                                  int exitCode) {
    if (!m_running.contains(testName)) return;

    RunningTest& rt = m_running[testName];
    if (rt.process) {
        rt.process->deleteLater();
        rt.process = nullptr;
    }
    // Parser is parented to the process, so deleteLater on process handles
    // both. Null our local ptr to avoid UAF.
    rt.parser = nullptr;

    m_running.remove(testName);
    m_legacyFailures.remove(testName);

    emit testFinished(testName, failures, durationSec, exitCode);
}

}  // namespace ftd::testrunner
