// ============================================================================
// SmartDispatcher.h — parallel CPU + serial GPU test scheduler
// ============================================================================
//
// The FTD test suite mixes lightweight CPU unit tests with a handful of
// CUDA-heavy tests that each expect exclusive access to the device. The
// dispatcher enforces that split:
//
//   - At most ONE GPU-heavy test runs at a time
//   - Up to (idealThreadCount - 1, clamped to [1, 16]) CPU tests run
//     in parallel
//
// Usage:
//
//   SmartDispatcher d;
//   d.setTests(selected);     // list of TestInfo snapshots
//   d.start();                // launches respecting concurrency limits
//   // ... wait for allComplete()
// ----------------------------------------------------------------------------

#pragma once

#include "TestModel.h"

#include <QHash>
#include <QObject>
#include <QQueue>
#include <QString>
#include <QVector>

namespace ftd::testrunner {

class TestRunner;

class SmartDispatcher : public QObject {
    Q_OBJECT
public:
    explicit SmartDispatcher(TestRunner* runner, QObject* parent = nullptr);
    ~SmartDispatcher() override;

    // Enqueue the given tests. Replaces any pending work. Existing running
    // tests are not interrupted — call TestRunner::stopAll() for that.
    void setTests(const QVector<TestInfo>& tests);

    // Start pumping the queues.
    void start();

    // Flush queues and stop issuing new work. Does not kill currently
    // running subprocesses.
    void cancel();

    int pendingCpuCount() const { return m_cpuQueue.size(); }
    int pendingGpuCount() const { return m_gpuQueue.size(); }
    int cpuWorkerLimit() const { return m_cpuWorkers; }

signals:
    void queueUpdated();
    void allComplete();

private slots:
    void onTestFinished(const QString& testName, int failures,
                        double durationSec, int exitCode);

private:
    // Issue as many new runs as concurrency limits allow.
    void pump();

    TestRunner* m_runner = nullptr;

    // Queues hold (testName, execPath) tuples; we ignore unused extras.
    struct PendingRun {
        QString name;
        QString execPath;
    };

    QQueue<PendingRun> m_cpuQueue;
    QQueue<PendingRun> m_gpuQueue;

    // Tracks which queue a running test came from so the finish handler
    // can decrement the correct counter without ambiguity.
    enum class QueueKind { Cpu, Gpu };
    QHash<QString, QueueKind> m_inFlight;

    int m_cpuRunning = 0;
    int m_gpuRunning = 0;
    int m_cpuWorkers = 1;
    bool m_cancelled = false;
};

}  // namespace ftd::testrunner
