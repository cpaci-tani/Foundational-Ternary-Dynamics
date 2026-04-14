// ============================================================================
// HistoryTab.h — "History" tab widget for the FTD Test Bench
// ============================================================================
//
// Left: QTableView of past runs (id, started, SHA, build, total, pass, fail,
// duration). Right: QTextEdit showing details for the selected run —
// per-test statuses plus regressions against the prior run. Bottom: a
// "Diff Selected" button that enables when exactly two rows are selected.
//
// Owns no data: HistoryDb lives in MainWindow and is passed in by pointer.
// MainWindow calls refresh() after every finishRun().
// ----------------------------------------------------------------------------

#pragma once

#include <QList>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QStandardItemModel;
class QTableView;
class QTextEdit;
QT_END_NAMESPACE

namespace ftd::testrunner {

class HistoryDb;
struct Regression;
struct RunSummary;
struct TestResultRow;

class HistoryTab : public QWidget {
    Q_OBJECT
public:
    explicit HistoryTab(HistoryDb* db, QWidget* parent = nullptr);
    ~HistoryTab() override;

public slots:
    // Re-query the database and rebuild the runs table. Called by
    // MainWindow after every finishRun().
    void refresh();

private slots:
    void onRunSelected();
    void onDiffButtonClicked();

private:
    void populateRunsTable();
    void showRunDetails(qint64 runId);
    void showDiff(qint64 runIdA, qint64 runIdB);

    // Build formatted HTML describing one run (summary + per-test list +
    // regressions since the prior run).
    QString formatRunDetails(const RunSummary& run,
                             const QList<TestResultRow>& results,
                             const QList<Regression>& regressions) const;

    // Build formatted HTML describing a diff between two runs.
    QString formatDiffDetails(const RunSummary& a,
                              const RunSummary& b,
                              const QList<Regression>& flips) const;

    // Extract the run id stored on the model's row-0 item for the row
    // at QModelIndex. Returns -1 on bad index.
    qint64 runIdFromRow(int row) const;

    HistoryDb*         m_db = nullptr;

    QTableView*        m_runsTable   = nullptr;
    QStandardItemModel* m_runsModel  = nullptr;
    QTextEdit*         m_detailsPanel = nullptr;
    QPushButton*       m_diffButton   = nullptr;
    QLabel*            m_statusLabel  = nullptr;
};

}  // namespace ftd::testrunner
