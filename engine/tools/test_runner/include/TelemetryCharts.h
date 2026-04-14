// ============================================================================
// TelemetryCharts.h — live multi-trace scalar telemetry (Qt6 QtCharts)
// ============================================================================
//
// Phase 5 of the FTD Test Bench. Consumes `metric` and `tick` NDJSON events
// routed from TestRunner::metricReceived and renders them as streaming
// QLineSeries plots.
//
// Ingestion model:
//   - `metric` events have shape {"event":"metric","name":"energy",
//     "value":0.234,"tick":100}. Each unique `name` spawns (or joins) a trace.
//   - `tick` events have shape {"event":"tick","tick":N,"dt":X, ...extras}.
//     Every numeric extra field (other than tick/dt/event) is treated as a
//     per-tick scalar metric with the field name as the trace name. `dt`
//     itself is routed into a "dt" trace so tick rate is visible.
//
// Per-test isolation:
//   Each test name gets its own TestChartSet (a page in a QStackedWidget).
//   Switching active tests switches the visible chart set. The top-of-widget
//   QComboBox lets users flip between tests without losing focus on the main
//   tree.
//
// Chart grouping:
//   A small static vector<pair<QString, QString>> maps well-known metric
//   names to shared chart titles (e.g. "energy","kinetic_energy" → "Energy").
//   Unknown metrics get their own solo chart titled with the metric name.
//
// Ring buffer:
//   Each trace holds the most recent `kRingCapacity` samples (= 1000). When
//   the buffer overflows, the oldest sample is dropped from both the QList
//   shadow and the underlying QLineSeries. Axes auto-rescale around the
//   live window.
//
// Expected-value sidecars (optional):
//   loadExpectedValues(testName) looks for
//     engine/tests/expected/<testName>.json
//   and draws a horizontal reference line for each scalar key. Absence is
//   silent — the file is a nicety, not a requirement.
// ----------------------------------------------------------------------------

#pragma once

#include <QHash>
#include <QList>
#include <QPointF>
#include <QString>
#include <QVariantMap>
#include <QWidget>

#include <limits>
#include <map>
#include <memory>
#include <utility>
#include <vector>

QT_BEGIN_NAMESPACE
class QChart;
class QChartView;
class QComboBox;
class QLabel;
class QLineSeries;
class QStackedWidget;
class QValueAxis;
class QVBoxLayout;
QT_END_NAMESPACE

namespace ftd::testrunner {

class TelemetryCharts : public QWidget {
    Q_OBJECT
public:
    explicit TelemetryCharts(QWidget* parent = nullptr);
    ~TelemetryCharts() override;

    // Clear all state for a fresh test run.
    void clearAll();

public slots:
    // Called when the user selects a test (from the tree or output panel).
    void setActiveTest(const QString& testName);

    // Routed from TestRunner::metricReceived. The QVariantMap is the parsed
    // NDJSON event — may be {"event":"metric","name":...,"value":...} or
    // {"event":"tick","tick":N,"dt":X, ...extras}. This slot dispatches on
    // evt["event"] internally.
    void onMetricEvent(const QString& testName, const QVariantMap& evt);

    // Called on test start — create a new chart set for this test, load
    // expected values if the sidecar JSON exists.
    void onTestStarted(const QString& testName);

    // Called on test end — finalize chart state (no-op for now; charts stay
    // visible after the test ends so the user can review them).
    void onTestFinished(const QString& testName, int failures, double duration);

private slots:
    void onTestSelectorChanged(int index);

private:
    struct TraceData;      // forward decl, defined in .cpp
    struct TestChartSet;   // forward decl, defined in .cpp

    // testName -> TestChartSet (owning unique_ptr so the QStackedWidget child
    // widgets stay tied to our lifetime). std::map because QHash is
    // copy-on-write and requires a copyable value type.
    std::map<QString, std::unique_ptr<TestChartSet>> m_byTest;

    QString m_activeTest;

    // Widgets.
    QVBoxLayout* m_rootLayout = nullptr;
    QComboBox* m_testSelector = nullptr;
    QStackedWidget* m_chartStack = nullptr;
    QLabel* m_statusLabel = nullptr;

    // Placeholder page shown when no test is active.
    QWidget* m_emptyPage = nullptr;
    int m_emptyPageIndex = -1;

    // Ring buffer capacity per trace.
    static constexpr int kRingCapacity = 1000;

    // Chart grouping rules — metric name → chart title. Unknown metrics get
    // a solo chart. Kept in a function-local static so initialization order
    // is well-defined.
    static const std::vector<std::pair<QString, QString>>& chartGroupRules();
    static QString chartTitleFor(const QString& metricName);

    void ensureChartSetExists(const QString& testName);
    TestChartSet* setFor(const QString& testName);
    void appendSample(const QString& testName,
                      const QString& metricName,
                      double value,
                      int tickIdx);
    void addReferenceLine(const QString& testName,
                          const QString& metricName,
                          double expected);
    void loadExpectedValues(const QString& testName);

    // Rebuild the QComboBox entries from m_byTest while preserving selection.
    void refreshSelector();
};

}  // namespace ftd::testrunner
