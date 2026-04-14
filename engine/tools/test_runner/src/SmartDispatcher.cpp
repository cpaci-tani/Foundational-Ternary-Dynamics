// ============================================================================
// SmartDispatcher.cpp — parallel CPU + serial GPU scheduler
// ============================================================================

#include "SmartDispatcher.h"
#include "TestRunner.h"

#include <QThread>

#include <algorithm>

namespace ftd::testrunner {

SmartDispatcher::SmartDispatcher(TestRunner* runner, QObject* parent)
    : QObject(parent), m_runner(runner) {
    const int ideal = QThread::idealThreadCount();
    // Leave one core for the UI thread + main process. Clamp to [1, 16] to
    // avoid thrashing the I/O subsystem on high-core-count boxes.
    m_cpuWorkers = std::clamp(ideal - 1, 1, 16);
    if (m_runner) {
        connect(m_runner, &TestRunner::testFinished,
                this, &SmartDispatcher::onTestFinished);
    }
}

SmartDispatcher::~SmartDispatcher() = default;

void SmartDispatcher::setTests(const QVector<TestInfo>& tests) {
    m_cpuQueue.clear();
    m_gpuQueue.clear();
    m_cancelled = false;

    for (const TestInfo& t : tests) {
        if (t.execPath.isEmpty()) continue;
        PendingRun r{t.name, t.execPath};
        if (t.isGpuHeavy) {
            m_gpuQueue.enqueue(r);
        } else {
            m_cpuQueue.enqueue(r);
        }
    }
    // Reset running counters — the caller is expected to have stopped
    // whatever was in flight before calling this.
    m_cpuRunning = 0;
    m_gpuRunning = 0;
    m_inFlight.clear();
    emit queueUpdated();
}

void SmartDispatcher::start() {
    m_cancelled = false;
    pump();
}

void SmartDispatcher::cancel() {
    m_cancelled = true;
    m_cpuQueue.clear();
    m_gpuQueue.clear();
    emit queueUpdated();
    // If nothing is in flight we need to fire allComplete here so callers
    // reset their UI state.
    if (m_cpuRunning == 0 && m_gpuRunning == 0) {
        emit allComplete();
    }
}

void SmartDispatcher::pump() {
    if (!m_runner) return;
    if (m_cancelled) return;

    // Start GPU jobs: at most 1 concurrent.
    while (m_gpuRunning < 1 && !m_gpuQueue.isEmpty()) {
        PendingRun r = m_gpuQueue.dequeue();
        ++m_gpuRunning;
        m_inFlight.insert(r.name, QueueKind::Gpu);
        m_runner->runTest(r.name, r.execPath, /*useTelemetry=*/true);
    }

    // Start CPU jobs: up to m_cpuWorkers concurrent.
    while (m_cpuRunning < m_cpuWorkers && !m_cpuQueue.isEmpty()) {
        PendingRun r = m_cpuQueue.dequeue();
        ++m_cpuRunning;
        m_inFlight.insert(r.name, QueueKind::Cpu);
        m_runner->runTest(r.name, r.execPath, /*useTelemetry=*/true);
    }

    emit queueUpdated();

    if (m_cpuQueue.isEmpty() && m_gpuQueue.isEmpty() &&
        m_cpuRunning == 0 && m_gpuRunning == 0) {
        emit allComplete();
    }
}

void SmartDispatcher::onTestFinished(const QString& testName,
                                     int /*failures*/,
                                     double /*durationSec*/,
                                     int /*exitCode*/) {
    const auto it = m_inFlight.find(testName);
    if (it != m_inFlight.end()) {
        if (it.value() == QueueKind::Gpu) {
            if (m_gpuRunning > 0) --m_gpuRunning;
        } else {
            if (m_cpuRunning > 0) --m_cpuRunning;
        }
        m_inFlight.erase(it);
    }
    pump();
}

}  // namespace ftd::testrunner
