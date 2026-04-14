// ============================================================================
// MainWindow.cpp — top-level window wiring
// ============================================================================

#include "MainWindow.h"
#include "LatticeViewer.h"
#include "OutputPanel.h"
#include "SmartDispatcher.h"
#include "TestModel.h"
#include "TestRunner.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QElapsedTimer>
#include <QHeaderView>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTimer>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

namespace ftd::testrunner {

MainWindow::MainWindow(const QString& buildDir, QWidget* parent)
    : QMainWindow(parent), m_buildDir(buildDir) {

    m_model = new TestModel(this);
    m_runner = new TestRunner(this);
    m_dispatcher = new SmartDispatcher(m_runner, this);

    buildUi();

    connect(m_runner, &TestRunner::testStarted,
            this, &MainWindow::onTestStarted);
    connect(m_runner, &TestRunner::checkReceived,
            this, &MainWindow::onCheckReceived);
    connect(m_runner, &TestRunner::metricReceived,
            this, &MainWindow::onMetricReceived);
    connect(m_runner, &TestRunner::snapshotReceived,
            this, &MainWindow::onSnapshotReceived);
    connect(m_runner, &TestRunner::sectionReceived,
            this, &MainWindow::onSectionReceived);
    connect(m_runner, &TestRunner::rawLineReceived,
            this, &MainWindow::onRawLineReceived);
    connect(m_runner, &TestRunner::testFinished,
            this, &MainWindow::onTestFinished);
    connect(m_dispatcher, &SmartDispatcher::allComplete,
            this, &MainWindow::onAllComplete);

    // Load the test inventory on startup.
    QTimer::singleShot(0, this, &MainWindow::onReload);
}

MainWindow::~MainWindow() = default;

// ============================================================================
// UI construction
// ============================================================================

void MainWindow::buildUi() {
    setWindowTitle(QStringLiteral("FTD Test Bench"));
    resize(1400, 900);

    buildActions();
    buildToolbar();
    buildStatusBar();
    buildCentralWidget();

    // File / Help menus.
    QMenu* fileMenu = menuBar()->addMenu(QStringLiteral("&File"));
    fileMenu->addAction(QStringLiteral("&Quit"),
                        QKeySequence::Quit,
                        this, &QWidget::close);
    QMenu* helpMenu = menuBar()->addMenu(QStringLiteral("&Help"));
    helpMenu->addAction(QStringLiteral("&About"), this, &MainWindow::onAbout);
}

void MainWindow::buildActions() {
    m_actRun = new QAction(QStringLiteral("Run Selected"), this);
    m_actRun->setShortcut(QKeySequence(QStringLiteral("F5")));
    connect(m_actRun, &QAction::triggered, this, &MainWindow::onRunSelected);

    m_actStop = new QAction(QStringLiteral("Stop"), this);
    m_actStop->setShortcut(QKeySequence(QStringLiteral("Shift+F5")));
    m_actStop->setEnabled(false);
    connect(m_actStop, &QAction::triggered, this, &MainWindow::onStop);

    m_actReload = new QAction(QStringLiteral("Reload Tests"), this);
    m_actReload->setShortcut(QKeySequence(QStringLiteral("Ctrl+R")));
    connect(m_actReload, &QAction::triggered, this, &MainWindow::onReload);

    m_actSelectAll = new QAction(QStringLiteral("Select All"), this);
    connect(m_actSelectAll, &QAction::triggered,
            this, &MainWindow::onSelectAll);

    m_actClearAll = new QAction(QStringLiteral("Clear All"), this);
    connect(m_actClearAll, &QAction::triggered,
            this, &MainWindow::onClearAll);
}

void MainWindow::buildToolbar() {
    QToolBar* tb = addToolBar(QStringLiteral("Main"));
    tb->setMovable(false);
    tb->addAction(m_actRun);
    tb->addAction(m_actStop);
    tb->addSeparator();
    tb->addAction(m_actSelectAll);
    tb->addAction(m_actClearAll);
    tb->addSeparator();

    m_gpuDefault = new QCheckBox(QStringLiteral("GPU default"), this);
    m_gpuDefault->setChecked(true);
    m_gpuDefault->setToolTip(QStringLiteral(
        "When checked, GPU-heavy tests are dispatched by default. Uncheck to\n"
        "skip GPU tests when running the selected set."));
    tb->addWidget(m_gpuDefault);
    tb->addSeparator();
    tb->addAction(m_actReload);
}

void MainWindow::buildStatusBar() {
    m_passLabel = new QLabel(QStringLiteral("Pass: 0"), this);
    m_failLabel = new QLabel(QStringLiteral("Fail: 0"), this);
    m_elapsedLabel = new QLabel(QStringLiteral("Elapsed: 0.0s"), this);
    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 100);
    m_progress->setValue(0);
    m_progress->setFixedWidth(240);

    statusBar()->addPermanentWidget(m_passLabel);
    statusBar()->addPermanentWidget(m_failLabel);
    statusBar()->addPermanentWidget(m_elapsedLabel);
    statusBar()->addPermanentWidget(m_progress);
    statusBar()->showMessage(QStringLiteral("Ready."));

    m_elapsedTimer = new QTimer(this);
    m_elapsedTimer->setInterval(250);
    connect(m_elapsedTimer, &QTimer::timeout, this, &MainWindow::tickElapsed);
}

void MainWindow::buildCentralWidget() {
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    m_tree = new QTreeView(this);
    m_tree->setModel(m_model);
    m_tree->setRootIsDecorated(true);
    m_tree->setAlternatingRowColors(true);
    m_tree->setUniformRowHeights(true);
    m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tree->header()->setSectionResizeMode(QHeaderView::Interactive);
    m_tree->header()->setStretchLastSection(true);
    m_tree->setColumnWidth(0, 360);
    m_tree->setColumnWidth(1, 80);
    m_tree->setColumnWidth(2, 80);
    splitter->addWidget(m_tree);

    auto* tabs = new QTabWidget(this);
    m_output = new OutputPanel(this);
    tabs->addTab(m_output, QStringLiteral("Output"));

    m_latticeViewer = new LatticeViewer(this);
    tabs->addTab(m_latticeViewer, QStringLiteral("Live Lattice"));

    auto* telemetryPlaceholder = new QLabel(
        QStringLiteral("TelemetryCharts lands in Phase 5"), this);
    telemetryPlaceholder->setAlignment(Qt::AlignCenter);
    tabs->addTab(telemetryPlaceholder, QStringLiteral("Telemetry"));

    auto* historyPlaceholder = new QLabel(
        QStringLiteral("HistoryDb lands in Phase 6"), this);
    historyPlaceholder->setAlignment(Qt::AlignCenter);
    tabs->addTab(historyPlaceholder, QStringLiteral("History"));

    splitter->addWidget(tabs);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    setCentralWidget(splitter);
}

void MainWindow::updateStatusCounters() {
    m_passLabel->setText(QStringLiteral("Pass: %1").arg(m_passCount));
    m_failLabel->setText(QStringLiteral("Fail: %1").arg(m_failCount));
    if (m_totalStarted > 0) {
        const int pct = static_cast<int>(
            (100.0 * m_totalFinished) / static_cast<double>(m_totalStarted));
        m_progress->setValue(pct);
    } else {
        m_progress->setValue(0);
    }
}

// ============================================================================
// Toolbar slots
// ============================================================================

void MainWindow::onRunSelected() {
    QVector<TestInfo> selected = m_model->selectedTests();

    if (!m_gpuDefault->isChecked()) {
        QVector<TestInfo> filtered;
        filtered.reserve(selected.size());
        for (const TestInfo& t : selected) {
            if (!t.isGpuHeavy) filtered.push_back(t);
        }
        selected = filtered;
    }

    if (selected.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("FTD Test Bench"),
            QStringLiteral("No tests selected. Tick rows in the tree first."));
        return;
    }

    // Reset counters.
    m_passCount = 0;
    m_failCount = 0;
    m_totalStarted = selected.size();
    m_totalFinished = 0;
    m_runActive = true;

    m_actRun->setEnabled(false);
    m_actStop->setEnabled(true);
    m_actReload->setEnabled(false);

    // Mark all selected tests pending.
    for (const TestInfo& t : selected) {
        m_model->updateStatus(t.name, TestStatus::Pending);
    }

    m_output->appendSystem(QStringLiteral("Running %1 tests").arg(selected.size()));
    statusBar()->showMessage(QStringLiteral("Running..."));

    m_wallClock.restart();
    m_elapsedTimer->start();

    updateStatusCounters();
    m_dispatcher->setTests(selected);
    m_dispatcher->start();
}

void MainWindow::onStop() {
    m_output->appendSystem(QStringLiteral("Stop requested — terminating subprocesses"));
    m_dispatcher->cancel();
    m_runner->stopAll();
    statusBar()->showMessage(QStringLiteral("Stopped."));
}

void MainWindow::onReload() {
    statusBar()->showMessage(QStringLiteral("Loading test inventory..."));
    m_output->appendSystem(QStringLiteral("Reloading test inventory from %1")
                               .arg(m_buildDir));

    const bool ok = m_model->reload(m_buildDir);
    if (!ok) {
        QMessageBox::warning(this, QStringLiteral("FTD Test Bench"),
            QStringLiteral("Failed to load test inventory from:\n\n%1\n\n"
                           "Make sure the build directory exists and contains a "
                           "configured CTest project (run `cmake -B %1 ...` first).")
                .arg(m_buildDir));
        statusBar()->showMessage(QStringLiteral("Reload failed."));
        return;
    }

    m_tree->expandAll();
    statusBar()->showMessage(
        QStringLiteral("Loaded %1 tests.").arg(m_model->testCount()));
    m_output->appendSystem(
        QStringLiteral("Loaded %1 tests.").arg(m_model->testCount()));
    setWindowTitle(QStringLiteral("FTD Test Bench — %1 tests")
                       .arg(m_model->testCount()));
}

void MainWindow::onSelectAll() {
    m_model->checkAll(true);
}

void MainWindow::onClearAll() {
    m_model->checkAll(false);
}

void MainWindow::onAbout() {
    QMessageBox::about(this, QStringLiteral("About FTD Test Bench"),
        QStringLiteral(
            "<h3>FTD Test Bench</h3>"
            "<p>Qt6 native test runner for the Foundational Ternary Dynamics "
            "engine.</p>"
            "<p>Phase 3 scaffold — includes TestModel, TestRunner, SmartDispatcher, "
            "NdjsonParser, and OutputPanel. LatticeViewer (Phase 4), "
            "TelemetryCharts (Phase 5), and HistoryDb (Phase 6) land in later "
            "phases.</p>"));
}

// ============================================================================
// Runner signals → model / output routing
// ============================================================================

void MainWindow::onTestStarted(const QString& testName, qint64 /*pid*/) {
    m_model->updateStatus(testName, TestStatus::Running);
    m_output->appendSystem(QStringLiteral("Started %1").arg(testName));
}

void MainWindow::onCheckReceived(const QString& testName, const QVariantMap& evt) {
    m_output->appendCheck(testName, evt);
}

void MainWindow::onMetricReceived(const QString& testName, const QVariantMap& evt) {
    m_output->appendMetric(testName, evt);
}

void MainWindow::onSnapshotReceived(const QString& testName,
                                    const QVariantMap& evt) {
    if (m_latticeViewer) {
        m_latticeViewer->onSnapshotEvent(testName, evt);
    }
}

void MainWindow::onSectionReceived(const QString& testName, const QString& sectionName) {
    m_output->appendSection(testName, sectionName);
}

void MainWindow::onRawLineReceived(const QString& testName, const QString& line) {
    m_output->appendRaw(testName, line);
}

void MainWindow::onTestFinished(const QString& testName, int failures,
                                 double durationSec, int exitCode) {
    m_model->updateDuration(testName, durationSec);
    m_model->updateFailures(testName, failures);
    if (failures == 0 && exitCode == 0) {
        m_model->updateStatus(testName, TestStatus::Pass);
        ++m_passCount;
    } else if (exitCode < 0) {
        m_model->updateStatus(testName, TestStatus::Error);
        ++m_failCount;
    } else {
        m_model->updateStatus(testName, TestStatus::Fail);
        ++m_failCount;
    }
    ++m_totalFinished;

    m_output->appendFinished(testName, failures, durationSec, exitCode);
    updateStatusCounters();
}

void MainWindow::onAllComplete() {
    m_runActive = false;
    m_elapsedTimer->stop();
    m_actRun->setEnabled(true);
    m_actStop->setEnabled(false);
    m_actReload->setEnabled(true);

    const double elapsed = m_wallClock.elapsed() / 1000.0;
    const QString msg = QStringLiteral("All complete: %1 pass, %2 fail, %3s")
                            .arg(m_passCount).arg(m_failCount)
                            .arg(elapsed, 0, 'f', 2);
    statusBar()->showMessage(msg);
    m_output->appendSystem(msg);
    m_progress->setValue(100);
}

void MainWindow::tickElapsed() {
    if (!m_runActive) return;
    const double elapsed = m_wallClock.elapsed() / 1000.0;
    m_elapsedLabel->setText(QStringLiteral("Elapsed: %1s")
                                .arg(elapsed, 0, 'f', 1));
}

}  // namespace ftd::testrunner
