// ============================================================================
// OutputPanel.h — interleaved per-test output view for Phase 3 scaffold
// ============================================================================
//
// Phase 3 deliberately ships the minimum useful panel: a single QTextEdit
// that streams output from every running test, prefixing each line with
// the test name ("[foo] bar"). Phase 5 will replace this with a collapsible
// per-test row layout with status badges, duration counters and live metric
// sparklines.
//
// The panel is fed by MainWindow, which routes TestRunner signals through
// `appendCheck`, `appendMetric`, `appendSection`, `appendRaw` and
// `appendSystem`. It exposes a simple `clear()` slot for the toolbar.
// ----------------------------------------------------------------------------

#pragma once

#include <QSet>
#include <QString>
#include <QVariantMap>
#include <QWidget>

class QTextEdit;
class QPushButton;
class QLabel;

namespace ftd::testrunner {

class OutputPanel : public QWidget {
    Q_OBJECT
public:
    explicit OutputPanel(QWidget* parent = nullptr);
    ~OutputPanel() override;

public slots:
    void appendSystem(const QString& message);
    void appendSection(const QString& testName, const QString& sectionName);
    void appendCheck(const QString& testName, const QVariantMap& evt);
    void appendMetric(const QString& testName, const QVariantMap& evt);
    void appendRaw(const QString& testName, const QString& line);
    void appendFinished(const QString& testName, int failures,
                        double durationSec, int exitCode);
    void clear();

private:
    void appendLine(const QString& line, const QString& colorCss = {});

    QTextEdit* m_log = nullptr;
    QLabel* m_status = nullptr;
    QPushButton* m_clearBtn = nullptr;

    int m_lineCount = 0;
    static constexpr int kSoftLimit = 20000;  // trim when exceeded

    // Track "first occurrence" of each (testName, metricName) pair so we
    // log only the first time a metric appears (the rest goes to
    // TelemetryCharts, un-throttled). Clear on clear().
    QSet<QString> m_seenMetricKeys;
};

}  // namespace ftd::testrunner
