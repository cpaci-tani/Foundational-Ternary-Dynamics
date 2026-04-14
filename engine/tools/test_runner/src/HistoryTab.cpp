// ============================================================================
// HistoryTab.cpp — "History" tab widget for the FTD Test Bench
// ============================================================================

#include "HistoryTab.h"

#include "HistoryDb.h"

#include <QBrush>
#include <QColor>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QLabel>
#include <QModelIndexList>
#include <QPushButton>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStringList>
#include <QTableView>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QVariant>

namespace ftd::testrunner {

namespace {

// HTML helpers --------------------------------------------------------------

QString esc(const QString& s) {
    // Qt's QTextDocument handles these when setHtml() is used, but the text
    // might contain angle brackets from test names (unlikely) or git SHAs.
    QString out = s;
    out.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    out.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
    out.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
    return out;
}

QString statusColor(const QString& status) {
    if (status == QStringLiteral("pass"))    return QStringLiteral("#2a9d3e");
    if (status == QStringLiteral("fail"))    return QStringLiteral("#c83232");
    if (status == QStringLiteral("crash"))   return QStringLiteral("#8b00ff");
    if (status == QStringLiteral("timeout")) return QStringLiteral("#d97706");
    if (status == QStringLiteral("skipped")) return QStringLiteral("#888888");
    return QStringLiteral("#444444");
}

QString statusSpan(const QString& status) {
    return QStringLiteral("<span style=\"color:%1;font-weight:bold\">%2</span>")
        .arg(statusColor(status), esc(status));
}

}  // namespace

// ---------------------------------------------------------------------------
// ctor / dtor
// ---------------------------------------------------------------------------

HistoryTab::HistoryTab(HistoryDb* db, QWidget* parent)
    : QWidget(parent), m_db(db) {

    // Main vertical layout: splitter on top, bottom action bar.
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(4, 4, 4, 4);
    rootLayout->setSpacing(4);

    auto* splitter = new QSplitter(Qt::Horizontal, this);

    // --- runs table ---
    m_runsTable = new QTableView(splitter);
    m_runsModel = new QStandardItemModel(0, 8, this);
    m_runsModel->setHorizontalHeaderLabels({
        QStringLiteral("ID"),
        QStringLiteral("Started"),
        QStringLiteral("Git"),
        QStringLiteral("Build"),
        QStringLiteral("Total"),
        QStringLiteral("Pass"),
        QStringLiteral("Fail"),
        QStringLiteral("Duration (s)"),
    });
    m_runsTable->setModel(m_runsModel);
    m_runsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_runsTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_runsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_runsTable->setAlternatingRowColors(true);
    m_runsTable->verticalHeader()->setVisible(false);
    m_runsTable->horizontalHeader()->setStretchLastSection(true);
    m_runsTable->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Interactive);

    splitter->addWidget(m_runsTable);

    // --- details panel ---
    m_detailsPanel = new QTextEdit(splitter);
    m_detailsPanel->setReadOnly(true);
    m_detailsPanel->setPlaceholderText(
        QStringLiteral("Select a run to see details, or tick two rows and "
                       "click \"Diff Selected\" to compare runs."));
    splitter->addWidget(m_detailsPanel);

    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 4);
    rootLayout->addWidget(splitter, /*stretch=*/1);

    // --- bottom action bar ---
    auto* bottom = new QHBoxLayout();
    bottom->setContentsMargins(0, 0, 0, 0);

    m_diffButton = new QPushButton(QStringLiteral("Diff Selected"), this);
    m_diffButton->setEnabled(false);
    m_diffButton->setToolTip(
        QStringLiteral("Select exactly two runs to enable."));
    connect(m_diffButton, &QPushButton::clicked,
            this, &HistoryTab::onDiffButtonClicked);
    bottom->addWidget(m_diffButton);

    m_statusLabel = new QLabel(QStringLiteral("No history yet."), this);
    m_statusLabel->setStyleSheet(QStringLiteral("color:#666;"));
    bottom->addWidget(m_statusLabel, /*stretch=*/1);
    rootLayout->addLayout(bottom);

    connect(m_runsTable->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this, [this](const QItemSelection&, const QItemSelection&) {
                onRunSelected();
            });

    refresh();
}

HistoryTab::~HistoryTab() = default;

// ---------------------------------------------------------------------------
// Refresh + population
// ---------------------------------------------------------------------------

void HistoryTab::refresh() {
    if (!m_db) {
        m_statusLabel->setText(QStringLiteral("History database unavailable."));
        return;
    }
    if (!m_db->isOpen()) {
        m_statusLabel->setText(
            QStringLiteral("History database not open (%1).").arg(m_db->dbPath()));
        return;
    }
    populateRunsTable();
}

void HistoryTab::populateRunsTable() {
    const qint64 previouslySelectedId = [this]() -> qint64 {
        const auto sel = m_runsTable->selectionModel()
                             ? m_runsTable->selectionModel()->selectedRows()
                             : QModelIndexList{};
        if (sel.size() == 1) {
            return runIdFromRow(sel.first().row());
        }
        return -1;
    }();

    m_runsModel->removeRows(0, m_runsModel->rowCount());

    const QList<RunSummary> runs = m_db->listRuns(200);
    m_statusLabel->setText(
        QStringLiteral("%1 run%2 recorded.")
            .arg(runs.size())
            .arg(runs.size() == 1 ? QString() : QStringLiteral("s")));

    int targetRow = -1;

    for (int i = 0; i < runs.size(); ++i) {
        const RunSummary& r = runs.at(i);
        QList<QStandardItem*> row;

        auto* idItem = new QStandardItem(QString::number(r.id));
        idItem->setData(QVariant::fromValue(r.id), Qt::UserRole + 1);
        row.append(idItem);
        row.append(new QStandardItem(r.startedAt));
        row.append(new QStandardItem(r.gitSha));
        row.append(new QStandardItem(r.buildType));
        row.append(new QStandardItem(QString::number(r.totalTests)));
        row.append(new QStandardItem(QString::number(r.passCount)));
        row.append(new QStandardItem(QString::number(r.failCount)));
        row.append(new QStandardItem(
            QString::number(r.wallTimeSec, 'f', 2)));

        // Color the fail column red when failCount > 0.
        if (r.failCount > 0) {
            row.at(6)->setForeground(QBrush(QColor(QStringLiteral("#c83232"))));
        }

        m_runsModel->appendRow(row);

        if (r.id == previouslySelectedId) targetRow = i;
    }

    m_runsTable->resizeColumnsToContents();

    if (targetRow >= 0) {
        const QModelIndex idx = m_runsModel->index(targetRow, 0);
        m_runsTable->selectionModel()->select(
            idx,
            QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    } else if (m_runsModel->rowCount() > 0) {
        // Default to the latest run.
        const QModelIndex idx = m_runsModel->index(0, 0);
        m_runsTable->selectionModel()->select(
            idx,
            QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    } else {
        m_detailsPanel->clear();
    }
}

// ---------------------------------------------------------------------------
// Selection + diff button
// ---------------------------------------------------------------------------

qint64 HistoryTab::runIdFromRow(int row) const {
    if (row < 0 || row >= m_runsModel->rowCount()) return -1;
    const QStandardItem* item = m_runsModel->item(row, 0);
    if (!item) return -1;
    bool ok = false;
    const qint64 id = item->data(Qt::UserRole + 1).toLongLong(&ok);
    return ok ? id : -1;
}

void HistoryTab::onRunSelected() {
    const auto sel = m_runsTable->selectionModel()
                         ? m_runsTable->selectionModel()->selectedRows()
                         : QModelIndexList{};
    m_diffButton->setEnabled(sel.size() == 2);

    if (sel.size() == 1) {
        const qint64 runId = runIdFromRow(sel.first().row());
        if (runId > 0) showRunDetails(runId);
    } else if (sel.size() == 2) {
        m_detailsPanel->setHtml(QStringLiteral(
            "<p><i>Two runs selected. Click <b>Diff Selected</b> to compare.</i></p>"));
    } else if (sel.isEmpty()) {
        m_detailsPanel->clear();
    }
}

void HistoryTab::onDiffButtonClicked() {
    const auto sel = m_runsTable->selectionModel()->selectedRows();
    if (sel.size() != 2) return;
    const qint64 a = runIdFromRow(sel.at(0).row());
    const qint64 b = runIdFromRow(sel.at(1).row());
    if (a < 0 || b < 0) return;
    showDiff(a, b);
}

// ---------------------------------------------------------------------------
// Details rendering
// ---------------------------------------------------------------------------

void HistoryTab::showRunDetails(qint64 runId) {
    if (!m_db || !m_db->isOpen()) return;

    const RunSummary run = m_db->getRun(runId);
    if (run.id < 0) {
        m_detailsPanel->setHtml(
            QStringLiteral("<p><i>Run not found.</i></p>"));
        return;
    }
    const QList<TestResultRow> results     = m_db->getRunResults(runId);
    const QList<Regression>    regressions = m_db->findRegressions(runId);

    m_detailsPanel->setHtml(formatRunDetails(run, results, regressions));
}

void HistoryTab::showDiff(qint64 runIdA, qint64 runIdB) {
    if (!m_db || !m_db->isOpen()) return;

    RunSummary a = m_db->getRun(runIdA);
    RunSummary b = m_db->getRun(runIdB);
    // Keep the older run on the left.
    if (a.id > b.id) std::swap(a, b);

    const QList<Regression> flips = m_db->diffRuns(a.id, b.id);
    m_detailsPanel->setHtml(formatDiffDetails(a, b, flips));
}

QString HistoryTab::formatRunDetails(const RunSummary& run,
                                     const QList<TestResultRow>& results,
                                     const QList<Regression>& regressions) const {
    QString html;
    html.reserve(4096 + results.size() * 120);

    html += QStringLiteral(
        "<h3 style=\"margin-bottom:4px\">Run #%1 &mdash; %2</h3>")
        .arg(run.id)
        .arg(esc(run.buildType));

    html += QStringLiteral("<table cellpadding=\"2\">");
    html += QStringLiteral("<tr><td><b>Started</b></td><td>%1</td></tr>")
                .arg(esc(run.startedAt));
    html += QStringLiteral("<tr><td><b>Finished</b></td><td>%1</td></tr>")
                .arg(esc(run.finishedAt.isEmpty()
                              ? QStringLiteral("(in progress)")
                              : run.finishedAt));
    html += QStringLiteral("<tr><td><b>Git SHA</b></td><td>%1</td></tr>")
                .arg(esc(run.gitSha));
    html += QStringLiteral("<tr><td><b>CPU workers</b></td><td>%1</td></tr>")
                .arg(run.cpuWorkers);
    html += QStringLiteral("<tr><td><b>GPU enabled</b></td><td>%1</td></tr>")
                .arg(run.gpuEnabled ? QStringLiteral("yes")
                                    : QStringLiteral("no"));
    html += QStringLiteral("<tr><td><b>Total</b></td><td>%1</td></tr>")
                .arg(run.totalTests);
    html += QStringLiteral(
        "<tr><td><b>Pass / Fail</b></td><td>"
        "<span style=\"color:#2a9d3e\">%1</span> / "
        "<span style=\"color:#c83232\">%2</span></td></tr>")
        .arg(run.passCount).arg(run.failCount);
    html += QStringLiteral("<tr><td><b>Wall time</b></td><td>%1 s</td></tr>")
                .arg(QString::number(run.wallTimeSec, 'f', 2));
    html += QStringLiteral("</table>");

    // Regressions.
    int regCount = 0;
    int fixCount = 0;
    for (const Regression& r : regressions) {
        if (r.sign > 0) ++regCount;
        else if (r.sign < 0) ++fixCount;
    }

    html += QStringLiteral(
        "<h4 style=\"margin-bottom:4px\">Regressions vs. prior run "
        "(%1 regression%2, %3 fix%4)</h4>")
        .arg(regCount).arg(regCount == 1 ? QString() : QStringLiteral("s"))
        .arg(fixCount).arg(fixCount == 1 ? QString() : QStringLiteral("es"));

    if (regressions.isEmpty()) {
        html += QStringLiteral(
            "<p><i>No status changes since the last finished run.</i></p>");
    } else {
        html += QStringLiteral(
            "<table cellpadding=\"3\" cellspacing=\"0\" border=\"1\" "
            "style=\"border-collapse:collapse\">");
        html += QStringLiteral(
            "<tr><th>Test</th><th>Prev</th><th>&rarr;</th><th>Now</th><th>Duration</th></tr>");
        for (const Regression& r : regressions) {
            const QString arrow = (r.sign > 0) ? QStringLiteral("&#8595;")
                                : (r.sign < 0) ? QStringLiteral("&#8593;")
                                               : QStringLiteral("&#8594;");
            html += QStringLiteral("<tr><td>%1</td><td>%2</td><td>%3</td>"
                                   "<td>%4</td><td>%5&rarr;%6 s</td></tr>")
                .arg(esc(r.testName))
                .arg(statusSpan(r.prevStatus))
                .arg(arrow)
                .arg(statusSpan(r.curStatus))
                .arg(QString::number(r.prevDuration, 'f', 2))
                .arg(QString::number(r.curDuration,  'f', 2));
        }
        html += QStringLiteral("</table>");
    }

    // Per-test list.
    html += QStringLiteral(
        "<h4 style=\"margin-top:12px;margin-bottom:4px\">Per-test results "
        "(%1)</h4>").arg(results.size());

    if (results.isEmpty()) {
        html += QStringLiteral("<p><i>No per-test rows recorded.</i></p>");
    } else {
        html += QStringLiteral(
            "<table cellpadding=\"3\" cellspacing=\"0\" border=\"1\" "
            "style=\"border-collapse:collapse\">");
        html += QStringLiteral(
            "<tr><th>Test</th><th>Status</th><th>Duration (s)</th>"
            "<th>Failures</th></tr>");
        for (const TestResultRow& r : results) {
            html += QStringLiteral(
                "<tr><td>%1</td><td>%2</td><td align=\"right\">%3</td>"
                "<td align=\"right\">%4</td></tr>")
                .arg(esc(r.testName))
                .arg(statusSpan(r.status))
                .arg(QString::number(r.durationSec, 'f', 2))
                .arg(r.nFails);
        }
        html += QStringLiteral("</table>");
    }

    return html;
}

QString HistoryTab::formatDiffDetails(const RunSummary& a,
                                      const RunSummary& b,
                                      const QList<Regression>& flips) const {
    QString html;
    html.reserve(4096 + flips.size() * 120);

    html += QStringLiteral("<h3>Diff: run #%1 &rarr; run #%2</h3>")
                .arg(a.id).arg(b.id);
    html += QStringLiteral(
        "<p><b>Before:</b> %1 (%2 pass / %3 fail)<br>"
        "<b>After:</b>&nbsp; %4 (%5 pass / %6 fail)</p>")
        .arg(esc(a.startedAt)).arg(a.passCount).arg(a.failCount)
        .arg(esc(b.startedAt)).arg(b.passCount).arg(b.failCount);

    int regCount = 0, fixCount = 0;
    for (const Regression& r : flips) {
        if (r.sign > 0) ++regCount;
        else if (r.sign < 0) ++fixCount;
    }

    html += QStringLiteral(
        "<p>%1 regression%2, %3 fix%4, %5 total flip%6.</p>")
        .arg(regCount).arg(regCount == 1 ? QString() : QStringLiteral("s"))
        .arg(fixCount).arg(fixCount == 1 ? QString() : QStringLiteral("es"))
        .arg(flips.size()).arg(flips.size() == 1 ? QString() : QStringLiteral("s"));

    if (flips.isEmpty()) {
        html += QStringLiteral(
            "<p><i>No status changes between these runs.</i></p>");
        return html;
    }

    html += QStringLiteral(
        "<table cellpadding=\"3\" cellspacing=\"0\" border=\"1\" "
        "style=\"border-collapse:collapse\">");
    html += QStringLiteral(
        "<tr><th>Test</th><th>Before</th><th>&rarr;</th><th>After</th>"
        "<th>Duration</th></tr>");
    for (const Regression& r : flips) {
        const QString arrow = (r.sign > 0) ? QStringLiteral("&#8595;")
                            : (r.sign < 0) ? QStringLiteral("&#8593;")
                                           : QStringLiteral("&#8594;");
        html += QStringLiteral("<tr><td>%1</td><td>%2</td><td>%3</td>"
                               "<td>%4</td><td>%5&rarr;%6 s</td></tr>")
            .arg(esc(r.testName))
            .arg(statusSpan(r.prevStatus))
            .arg(arrow)
            .arg(statusSpan(r.curStatus))
            .arg(QString::number(r.prevDuration, 'f', 2))
            .arg(QString::number(r.curDuration,  'f', 2));
    }
    html += QStringLiteral("</table>");

    return html;
}

}  // namespace ftd::testrunner
