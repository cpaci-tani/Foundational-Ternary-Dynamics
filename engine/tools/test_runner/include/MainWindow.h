// ============================================================================
// MainWindow.h — top-level window for the FTD Test Bench
// ============================================================================
//
// Layout:
//   QMainWindow
//   ├── Toolbar:     Run Selected | Stop | Select All | Clear All
//   │                GPU-default checkbox | Reload Tests
//   ├── Central:     QSplitter
//   │                ├── Left:  QTreeView backed by TestModel (checkboxes)
//   │                └── Right: QTabWidget
//   │                          ├── Output    → OutputPanel
//   │                          ├── Lattice   → placeholder (Phase 4)
//   │                          ├── Telemetry → placeholder (Phase 5)
//   │                          └── History   → placeholder (Phase 6)
//   └── Status bar:  pass / fail counts, progress bar, elapsed time
//
// The window wires TestRunner signals into the TestModel and OutputPanel.
// SmartDispatcher decides which tests run concurrently.
// ----------------------------------------------------------------------------

#pragma once

#include <QElapsedTimer>
#include <QHash>
#include <QMainWindow>
#include <QString>
#include <QVariantMap>

class QLabel;
class QProgressBar;
class QPushButton;
class QTimer;
class QTreeView;
class QAction;
class QCheckBox;

namespace ftd::testrunner {

class HistoryDb;
class HistoryTab;
class LatticeViewer;
class OutputPanel;
class SmartDispatcher;
class TelemetryCharts;
class TestModel;
class TestRunner;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(const QString& buildDir, QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onRunSelected();
    void onStop();
    void onReload();
    void onSelectAll();
    void onClearAll();
    void onAbout();

    void onTestStarted(const QString& testName, qint64 pid);
    void onCheckReceived(const QString& testName, const QVariantMap& evt);
    void onMetricReceived(const QString& testName, const QVariantMap& evt);
    void onSnapshotReceived(const QString& testName, const QVariantMap& evt);
    void onSectionReceived(const QString& testName, const QString& sectionName);
    void onRawLineReceived(const QString& testName, const QString& line);
    void onTestFinished(const QString& testName, int failures,
                         double durationSec, int exitCode);
    void onAllComplete();

    void tickElapsed();

private:
    void buildUi();
    void buildActions();
    void buildToolbar();
    void buildStatusBar();
    void buildCentralWidget();
    void updateStatusCounters();

    QString m_buildDir;

    // Core subsystems.
    TestModel* m_model = nullptr;
    TestRunner* m_runner = nullptr;
    SmartDispatcher* m_dispatcher = nullptr;

    // Widgets.
    QTreeView* m_tree = nullptr;
    OutputPanel* m_output = nullptr;
    LatticeViewer* m_latticeViewer = nullptr;
    TelemetryCharts* m_telemetryCharts = nullptr;
    HistoryTab* m_historyTab = nullptr;

    // Persistent run history (SQLite). Outlives any single run.
    HistoryDb* m_historyDb = nullptr;
    qint64 m_currentRunId = -1;

    // Per-test metadata captured at the start of each run, so that when
    // a test finishes we can persist its category / GPU flag without
    // reaching back into the tree model.
    struct RunTestMeta {
        QString category;
        bool isGpuHeavy = false;
    };
    QHash<QString, RunTestMeta> m_runMeta;

    // Toolbar actions.
    QAction* m_actRun = nullptr;
    QAction* m_actStop = nullptr;
    QAction* m_actReload = nullptr;
    QAction* m_actSelectAll = nullptr;
    QAction* m_actClearAll = nullptr;
    QCheckBox* m_gpuDefault = nullptr;

    // Status bar.
    QLabel* m_passLabel = nullptr;
    QLabel* m_failLabel = nullptr;
    QLabel* m_elapsedLabel = nullptr;
    QProgressBar* m_progress = nullptr;

    // Counters updated as tests complete.
    int m_passCount = 0;
    int m_failCount = 0;
    int m_totalStarted = 0;
    int m_totalFinished = 0;

    QTimer* m_elapsedTimer = nullptr;
    QElapsedTimer m_wallClock;
    bool m_runActive = false;
};

}  // namespace ftd::testrunner
