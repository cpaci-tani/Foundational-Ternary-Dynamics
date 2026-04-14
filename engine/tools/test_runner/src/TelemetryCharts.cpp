// ============================================================================
// TelemetryCharts.cpp — live multi-trace scalar telemetry (Qt6 QtCharts)
// ============================================================================

#include "TelemetryCharts.h"

#include <QChart>
#include <QChartView>
#include <QComboBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QLineSeries>
#include <QPainter>
#include <QScrollArea>
#include <QStackedWidget>
#include <QValueAxis>
#include <QVariant>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace ftd::testrunner {

// ---------------------------------------------------------------------------
// Internal data structures
// ---------------------------------------------------------------------------

struct TelemetryCharts::TraceData {
    QString name;
    QLineSeries* series = nullptr;   // owned by QChart, not by us
    QList<QPointF> ring;             // shadow ring buffer, <= kRingCapacity
    double minY = std::numeric_limits<double>::infinity();
    double maxY = -std::numeric_limits<double>::infinity();
    double minX = std::numeric_limits<double>::infinity();
    double maxX = -std::numeric_limits<double>::infinity();
};

struct TelemetryCharts::TestChartSet {
    QWidget* page = nullptr;                   // scrollable page
    QVBoxLayout* pageLayout = nullptr;         // owns the charts
    int stackIndex = -1;
    QHash<QString, TraceData> traces;          // metricName -> trace
    QHash<QString, QChart*> charts;            // chartTitle -> chart
    QHash<QString, QChartView*> chartViews;    // chartTitle -> view
    QHash<QString, QValueAxis*> axesX;         // chartTitle -> axisX
    QHash<QString, QValueAxis*> axesY;         // chartTitle -> axisY
    int eventCounter = 0;
};

// ---------------------------------------------------------------------------
// Chart grouping rules
// ---------------------------------------------------------------------------

const std::vector<std::pair<QString, QString>>&
TelemetryCharts::chartGroupRules() {
    static const std::vector<std::pair<QString, QString>> kRules = {
        {QStringLiteral("energy"),           QStringLiteral("Energy")},
        {QStringLiteral("kinetic_energy"),   QStringLiteral("Energy")},
        {QStringLiteral("potential_energy"), QStringLiteral("Energy")},
        {QStringLiteral("field_energy"),     QStringLiteral("Energy")},
        {QStringLiteral("gauss_rms"),        QStringLiteral("Gauss violation")},
        {QStringLiteral("gauss_max"),        QStringLiteral("Gauss violation")},
        {QStringLiteral("gauss_violation"),  QStringLiteral("Gauss violation")},
        {QStringLiteral("force_total"),      QStringLiteral("Forces")},
        {QStringLiteral("force_coulomb"),    QStringLiteral("Forces")},
        {QStringLiteral("force_gravity"),    QStringLiteral("Forces")},
        {QStringLiteral("force_strong"),     QStringLiteral("Forces")},
        {QStringLiteral("force_weak"),       QStringLiteral("Forces")},
        {QStringLiteral("n_particles"),      QStringLiteral("Counts")},
        {QStringLiteral("manifested"),       QStringLiteral("Counts")},
        {QStringLiteral("void_count"),       QStringLiteral("Counts")},
        {QStringLiteral("tick_rate"),        QStringLiteral("Performance")},
        {QStringLiteral("dt"),               QStringLiteral("Performance")},
    };
    return kRules;
}

QString TelemetryCharts::chartTitleFor(const QString& metricName) {
    for (const auto& [key, title] : chartGroupRules()) {
        if (metricName == key) return title;
    }
    return metricName;  // Unknown metric: solo chart.
}

// ---------------------------------------------------------------------------
// Construction / teardown
// ---------------------------------------------------------------------------

TelemetryCharts::TelemetryCharts(QWidget* parent) : QWidget(parent) {
    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setContentsMargins(4, 4, 4, 4);
    m_rootLayout->setSpacing(4);

    auto* header = new QWidget(this);
    auto* headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(2);
    m_statusLabel = new QLabel(QStringLiteral("No active test"), header);
    headerLayout->addWidget(m_statusLabel);
    m_testSelector = new QComboBox(header);
    m_testSelector->setMinimumWidth(240);
    m_testSelector->setToolTip(QStringLiteral(
        "Select which test's charts to view. Populated as tests run."));
    connect(m_testSelector,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TelemetryCharts::onTestSelectorChanged);
    headerLayout->addWidget(m_testSelector);
    m_rootLayout->addWidget(header);

    m_chartStack = new QStackedWidget(this);
    m_emptyPage = new QWidget(m_chartStack);
    auto* emptyLayout = new QVBoxLayout(m_emptyPage);
    emptyLayout->setAlignment(Qt::AlignCenter);
    auto* emptyLabel = new QLabel(
        QStringLiteral("No telemetry yet. Run a test to populate charts."),
        m_emptyPage);
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setStyleSheet(
        QStringLiteral("color: #888888; font-style: italic;"));
    emptyLayout->addWidget(emptyLabel);
    m_emptyPageIndex = m_chartStack->addWidget(m_emptyPage);
    m_rootLayout->addWidget(m_chartStack, 1);
}

TelemetryCharts::~TelemetryCharts() = default;

// ---------------------------------------------------------------------------
// Public slots
// ---------------------------------------------------------------------------

void TelemetryCharts::clearAll() {
    for (auto& kv : m_byTest) {
        TestChartSet* set = kv.second.get();
        if (set && set->page) {
            m_chartStack->removeWidget(set->page);
            set->page->deleteLater();
            set->page = nullptr;
        }
    }
    m_byTest.clear();
    m_activeTest.clear();
    m_statusLabel->setText(QStringLiteral("No active test"));
    refreshSelector();
    m_chartStack->setCurrentIndex(m_emptyPageIndex);
}

void TelemetryCharts::setActiveTest(const QString& testName) {
    if (testName.isEmpty()) {
        m_activeTest.clear();
        m_chartStack->setCurrentIndex(m_emptyPageIndex);
        m_statusLabel->setText(QStringLiteral("No active test"));
        return;
    }
    m_activeTest = testName;
    ensureChartSetExists(testName);
    TestChartSet* set = setFor(testName);
    if (set && set->stackIndex >= 0) {
        m_chartStack->setCurrentIndex(set->stackIndex);
    }
    m_statusLabel->setText(QStringLiteral("Active: %1 (%2 traces)")
                               .arg(testName)
                               .arg(set ? set->traces.size() : 0));
    const int idx = m_testSelector->findText(testName);
    if (idx >= 0 && m_testSelector->currentIndex() != idx) {
        const QSignalBlocker block(m_testSelector);
        m_testSelector->setCurrentIndex(idx);
    }
}

void TelemetryCharts::onMetricEvent(const QString& testName,
                                    const QVariantMap& evt) {
    if (testName.isEmpty()) return;
    const QString eventType = evt.value(QStringLiteral("event")).toString();
    ensureChartSetExists(testName);
    TestChartSet* set = setFor(testName);
    if (!set) return;
    ++set->eventCounter;

    if (eventType == QStringLiteral("metric")) {
        const QString metricName =
            evt.value(QStringLiteral("name")).toString();
        if (metricName.isEmpty()) return;
        bool ok = false;
        const double value = evt.value(QStringLiteral("value")).toDouble(&ok);
        if (!ok || std::isnan(value) || std::isinf(value)) return;
        const int tickIdx = evt.contains(QStringLiteral("tick"))
            ? evt.value(QStringLiteral("tick")).toInt()
            : set->eventCounter;
        appendSample(testName, metricName, value, tickIdx);
    } else if (eventType == QStringLiteral("tick")) {
        const int tickIdx = evt.value(QStringLiteral("tick")).toInt();
        // Auto-discover every numeric extra as its own trace.
        for (auto it = evt.cbegin(); it != evt.cend(); ++it) {
            const QString& key = it.key();
            if (key == QStringLiteral("event") ||
                key == QStringLiteral("tick")) continue;
            bool ok = false;
            const double value = it.value().toDouble(&ok);
            if (!ok || std::isnan(value) || std::isinf(value)) continue;
            appendSample(testName, key, value, tickIdx);
        }
    }
    // Other event types are ignored silently.

    if (testName == m_activeTest && m_statusLabel) {
        m_statusLabel->setText(QStringLiteral("Active: %1 (%2 traces)")
                                   .arg(testName)
                                   .arg(set->traces.size()));
    }
}

void TelemetryCharts::onTestStarted(const QString& testName) {
    ensureChartSetExists(testName);
    loadExpectedValues(testName);
    if (m_activeTest.isEmpty()) setActiveTest(testName);
    refreshSelector();
}

void TelemetryCharts::onTestFinished(const QString& testName,
                                     int /*failures*/,
                                     double /*duration*/) {
    if (testName == m_activeTest && m_statusLabel) {
        TestChartSet* set = setFor(testName);
        m_statusLabel->setText(QStringLiteral("Finished: %1 (%2 traces)")
                                   .arg(testName)
                                   .arg(set ? set->traces.size() : 0));
    }
}

// ---------------------------------------------------------------------------
// Private: chart set / trace management
// ---------------------------------------------------------------------------

void TelemetryCharts::ensureChartSetExists(const QString& testName) {
    if (m_byTest.find(testName) != m_byTest.end()) return;
    auto set = std::make_unique<TestChartSet>();
    auto* scroll = new QScrollArea(m_chartStack);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto* inner = new QWidget(scroll);
    auto* innerLayout = new QVBoxLayout(inner);
    innerLayout->setContentsMargins(4, 4, 4, 4);
    innerLayout->setSpacing(6);
    innerLayout->addStretch(1);  // keeps charts top-aligned as they arrive
    scroll->setWidget(inner);
    set->page = scroll;
    set->pageLayout = innerLayout;
    set->stackIndex = m_chartStack->addWidget(scroll);
    m_byTest.emplace(testName, std::move(set));
    refreshSelector();
}

TelemetryCharts::TestChartSet*
TelemetryCharts::setFor(const QString& testName) {
    auto it = m_byTest.find(testName);
    if (it == m_byTest.end()) return nullptr;
    return it->second.get();
}

void TelemetryCharts::appendSample(const QString& testName,
                                   const QString& metricName,
                                   double value,
                                   int tickIdx) {
    TestChartSet* set = setFor(testName);
    if (!set) return;
    const QString chartTitle = chartTitleFor(metricName);

    // Lazily create the QChart / QChartView for this title.
    if (!set->charts.contains(chartTitle)) {
        auto* chart = new QChart();
        chart->setTitle(chartTitle);
        chart->legend()->setAlignment(Qt::AlignBottom);
        chart->setAnimationOptions(QChart::NoAnimation);
        chart->setMargins(QMargins(4, 4, 4, 4));
        auto* axisX = new QValueAxis();
        axisX->setTitleText(QStringLiteral("tick"));
        axisX->setLabelFormat(QStringLiteral("%d"));
        chart->addAxis(axisX, Qt::AlignBottom);
        auto* axisY = new QValueAxis();
        axisY->setTitleText(QStringLiteral("value"));
        axisY->setLabelFormat(QStringLiteral("%.4g"));
        chart->addAxis(axisY, Qt::AlignLeft);
        auto* view = new QChartView(chart);
        view->setRenderHint(QPainter::Antialiasing);
        view->setMinimumHeight(180);
        // Insert above the stretch spacer so charts stack top-down.
        const int insertAt = set->pageLayout->count() - 1;
        set->pageLayout->insertWidget(insertAt >= 0 ? insertAt : 0, view);
        set->charts.insert(chartTitle, chart);
        set->chartViews.insert(chartTitle, view);
        set->axesX.insert(chartTitle, axisX);
        set->axesY.insert(chartTitle, axisY);
    }

    QChart* chart = set->charts.value(chartTitle);
    QValueAxis* axisX = set->axesX.value(chartTitle);
    QValueAxis* axisY = set->axesY.value(chartTitle);

    // Lazily create the series for this metric name within the chart.
    if (!set->traces.contains(metricName)) {
        TraceData trace;
        trace.name = metricName;
        trace.series = new QLineSeries();
        trace.series->setName(metricName);
        chart->addSeries(trace.series);
        trace.series->attachAxis(axisX);
        trace.series->attachAxis(axisY);
        set->traces.insert(metricName, trace);
    }

    TraceData& trace = set->traces[metricName];

    // Append + enforce ring capacity.
    const QPointF p(static_cast<qreal>(tickIdx), static_cast<qreal>(value));
    trace.ring.append(p);
    trace.series->append(p);
    if (trace.ring.size() > kRingCapacity) {
        trace.ring.removeFirst();
        trace.series->remove(0);
    }

    // Running bounds. X is monotonic-ish so first/last work; Y needs a
    // periodic rescan to drop stale extrema that rolled off the ring.
    if (!trace.ring.isEmpty()) {
        trace.minX = trace.ring.first().x();
        trace.maxX = trace.ring.last().x();
    }
    if (value < trace.minY) trace.minY = value;
    if (value > trace.maxY) trace.maxY = value;
    if ((set->eventCounter % 256) == 0) {
        double mn = std::numeric_limits<double>::infinity();
        double mx = -std::numeric_limits<double>::infinity();
        for (const QPointF& pt : trace.ring) {
            const double y = pt.y();
            if (y < mn) mn = y;
            if (y > mx) mx = y;
        }
        if (std::isfinite(mn)) trace.minY = mn;
        if (std::isfinite(mx)) trace.maxY = mx;
    }

    // Recompute chart-wide bounds across every trace sharing this chart.
    double chartMinX = std::numeric_limits<double>::infinity();
    double chartMaxX = -std::numeric_limits<double>::infinity();
    double chartMinY = std::numeric_limits<double>::infinity();
    double chartMaxY = -std::numeric_limits<double>::infinity();
    for (auto it = set->traces.cbegin(); it != set->traces.cend(); ++it) {
        if (chartTitleFor(it.key()) != chartTitle) continue;
        const TraceData& t = it.value();
        if (std::isfinite(t.minX)) chartMinX = std::min(chartMinX, t.minX);
        if (std::isfinite(t.maxX)) chartMaxX = std::max(chartMaxX, t.maxX);
        if (std::isfinite(t.minY)) chartMinY = std::min(chartMinY, t.minY);
        if (std::isfinite(t.maxY)) chartMaxY = std::max(chartMaxY, t.maxY);
    }
    if (std::isfinite(chartMinX) && std::isfinite(chartMaxX)) {
        if (chartMaxX - chartMinX < 1.0) chartMaxX = chartMinX + 1.0;
        axisX->setRange(chartMinX, chartMaxX);
    }
    if (std::isfinite(chartMinY) && std::isfinite(chartMaxY)) {
        double lo = chartMinY;
        double hi = chartMaxY;
        if (hi - lo < 1e-12) {
            const double pad = std::max(1e-9, std::fabs(lo) * 0.05 + 1e-9);
            lo -= pad;
            hi += pad;
        } else {
            const double pad = (hi - lo) * 0.1;
            lo -= pad;
            hi += pad;
        }
        axisY->setRange(lo, hi);
    }
}

void TelemetryCharts::addReferenceLine(const QString& testName,
                                       const QString& metricName,
                                       double expected) {
    TestChartSet* set = setFor(testName);
    if (!set) return;
    const QString chartTitle = chartTitleFor(metricName);
    if (!set->charts.contains(chartTitle)) return;
    QChart* chart = set->charts.value(chartTitle);
    QValueAxis* axisX = set->axesX.value(chartTitle);
    QValueAxis* axisY = set->axesY.value(chartTitle);
    if (!chart || !axisX || !axisY) return;

    auto* ref = new QLineSeries();
    ref->setName(QStringLiteral("%1 (expected)").arg(metricName));
    ref->append(axisX->min(), expected);
    ref->append(std::max(axisX->max(), axisX->min() + 1.0), expected);
    QPen pen(Qt::DashLine);
    pen.setColor(Qt::gray);
    pen.setWidthF(1.2);
    ref->setPen(pen);
    chart->addSeries(ref);
    ref->attachAxis(axisX);
    ref->attachAxis(axisY);
}

void TelemetryCharts::loadExpectedValues(const QString& testName) {
    // Look for engine/tests/expected/<testName>.json relative to CWD.
    // Absence is the common case; stay silent.
    const QString path =
        QStringLiteral("engine/tests/expected/%1.json").arg(testName);
    QFile f(path);
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) return;
    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    f.close();
    if (err.error != QJsonParseError::NoError || !doc.isObject()) return;
    const QJsonObject obj = doc.object();
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        const QJsonValue& v = it.value();
        if (!v.isDouble()) continue;
        addReferenceLine(testName, it.key(), v.toDouble());
    }
}

// ---------------------------------------------------------------------------
// Selector
// ---------------------------------------------------------------------------

void TelemetryCharts::refreshSelector() {
    if (!m_testSelector) return;
    const QString prev = m_testSelector->currentText();
    const QSignalBlocker block(m_testSelector);
    m_testSelector->clear();
    // std::map is already ordered by key, so iteration yields sorted names.
    for (const auto& kv : m_byTest) {
        m_testSelector->addItem(kv.first);
    }
    int idx = -1;
    if (!m_activeTest.isEmpty()) idx = m_testSelector->findText(m_activeTest);
    if (idx < 0 && !prev.isEmpty()) idx = m_testSelector->findText(prev);
    if (idx < 0 && m_testSelector->count() > 0) idx = 0;
    if (idx >= 0) m_testSelector->setCurrentIndex(idx);
}

void TelemetryCharts::onTestSelectorChanged(int index) {
    if (index < 0 || !m_testSelector) return;
    const QString name = m_testSelector->itemText(index);
    if (name.isEmpty() || name == m_activeTest) return;
    setActiveTest(name);
}

}  // namespace ftd::testrunner
