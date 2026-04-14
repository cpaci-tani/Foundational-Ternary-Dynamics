// ============================================================================
// OutputPanel.cpp — interleaved output view
// ============================================================================

#include "OutputPanel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextEdit>
#include <QVBoxLayout>

namespace ftd::testrunner {

OutputPanel::OutputPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    auto* header = new QHBoxLayout;
    m_status = new QLabel(QStringLiteral("Idle."), this);
    header->addWidget(m_status, 1);

    m_clearBtn = new QPushButton(QStringLiteral("Clear"), this);
    connect(m_clearBtn, &QPushButton::clicked, this, &OutputPanel::clear);
    header->addWidget(m_clearBtn, 0);

    layout->addLayout(header);

    m_log = new QTextEdit(this);
    m_log->setReadOnly(true);
    m_log->setLineWrapMode(QTextEdit::NoWrap);

    QFont mono(QStringLiteral("Consolas"));
    if (!mono.exactMatch()) {
        mono = QFont(QStringLiteral("Courier New"));
    }
    mono.setStyleHint(QFont::Monospace);
    mono.setPointSize(9);
    m_log->setFont(mono);

    layout->addWidget(m_log, 1);
}

OutputPanel::~OutputPanel() = default;

void OutputPanel::clear() {
    m_log->clear();
    m_lineCount = 0;
    m_seenMetricKeys.clear();
    m_status->setText(QStringLiteral("Idle."));
}

void OutputPanel::appendLine(const QString& line, const QString& colorCss) {
    if (m_lineCount > kSoftLimit) {
        // Trim the first 20% of the block to stop unbounded growth.
        QTextCursor c(m_log->document());
        c.movePosition(QTextCursor::Start);
        c.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, kSoftLimit / 5);
        c.removeSelectedText();
        c.deleteChar();
        m_lineCount -= kSoftLimit / 5;
    }

    if (colorCss.isEmpty()) {
        m_log->append(line.toHtmlEscaped());
    } else {
        const QString html = QStringLiteral("<span style=\"color:%1;\">%2</span>")
                                 .arg(colorCss, line.toHtmlEscaped());
        m_log->append(html);
    }
    ++m_lineCount;
}

void OutputPanel::appendSystem(const QString& message) {
    appendLine(QStringLiteral("[runner] ") + message, QStringLiteral("#888888"));
}

void OutputPanel::appendSection(const QString& testName, const QString& sectionName) {
    appendLine(QStringLiteral("[%1] === %2 ===").arg(testName, sectionName),
               QStringLiteral("#555555"));
}

void OutputPanel::appendCheck(const QString& testName, const QVariantMap& evt) {
    const bool pass = evt.value(QStringLiteral("pass")).toBool();
    const QString name = evt.value(QStringLiteral("name")).toString();
    QString line = QStringLiteral("[%1] %2 %3")
                       .arg(testName,
                            pass ? QStringLiteral("PASS") : QStringLiteral("FAIL"),
                            name);

    // Include got/expected/tol/diff if present (check_close).
    if (evt.contains(QStringLiteral("got"))) {
        line += QStringLiteral("  got=%1 expected=%2 tol=%3 diff=%4")
                    .arg(evt.value(QStringLiteral("got")).toDouble())
                    .arg(evt.value(QStringLiteral("expected")).toDouble())
                    .arg(evt.value(QStringLiteral("tol")).toDouble())
                    .arg(evt.value(QStringLiteral("diff")).toDouble());
    } else if (evt.contains(QStringLiteral("detail"))) {
        const QString detail = evt.value(QStringLiteral("detail")).toString();
        if (!detail.isEmpty()) {
            line += QStringLiteral("  (") + detail + QStringLiteral(")");
        }
    }
    appendLine(line, pass ? QStringLiteral("#2ea043") : QStringLiteral("#cc3232"));
}

void OutputPanel::appendMetric(const QString& testName, const QVariantMap& evt) {
    // Phase 5: TelemetryCharts receives every metric event un-throttled.
    // OutputPanel only logs the FIRST occurrence of each (testName, metricName)
    // pair so the log stays readable. Tick events are logged once per test
    // (the first tick seen) for the same reason.
    const QString eventType = evt.value(QStringLiteral("event")).toString();

    if (eventType == QStringLiteral("tick")) {
        const QString key = QStringLiteral("%1::__tick__").arg(testName);
        if (m_seenMetricKeys.contains(key)) return;
        m_seenMetricKeys.insert(key);
        const int tick = evt.value(QStringLiteral("tick")).toInt();
        const double dt = evt.value(QStringLiteral("dt")).toDouble();
        appendLine(QStringLiteral("[%1] first tick=%2 dt=%3 (subsequent "
                                  "ticks routed to Telemetry tab)")
                       .arg(testName).arg(tick).arg(dt),
                   QStringLiteral("#444444"));
    } else {
        const QString name = evt.value(QStringLiteral("name")).toString();
        const QString key = QStringLiteral("%1::%2").arg(testName, name);
        if (m_seenMetricKeys.contains(key)) return;
        m_seenMetricKeys.insert(key);
        const double value = evt.value(QStringLiteral("value")).toDouble();
        appendLine(QStringLiteral("[%1] first metric %2 = %3 (subsequent "
                                  "samples routed to Telemetry tab)")
                       .arg(testName, name).arg(value),
                   QStringLiteral("#444444"));
    }
}

void OutputPanel::appendRaw(const QString& testName, const QString& line) {
    if (line.isEmpty()) return;
    appendLine(QStringLiteral("[%1] %2").arg(testName, line));
}

void OutputPanel::appendFinished(const QString& testName, int failures,
                                 double durationSec, int exitCode) {
    const bool ok = (failures == 0 && exitCode == 0);
    const QString line = QStringLiteral("[%1] %2 in %3s (%4 failures, exit %5)")
                             .arg(testName,
                                  ok ? QStringLiteral("DONE") : QStringLiteral("FAILED"))
                             .arg(durationSec, 0, 'f', 2)
                             .arg(failures)
                             .arg(exitCode);
    appendLine(line, ok ? QStringLiteral("#2ea043") : QStringLiteral("#cc3232"));
    m_status->setText(line);
}

}  // namespace ftd::testrunner
