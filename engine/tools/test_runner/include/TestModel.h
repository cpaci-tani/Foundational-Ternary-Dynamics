// ============================================================================
// TestModel.h — two-level (Category → Test) tree model for FTD Test Bench
// ============================================================================
//
// Reads the CTest inventory via `ctest --show-only=json-v1` at construction,
// classifies each test using the CATEGORY_RULES / PREFIX_RULES ported from
// engine/run_tests_live.py, extracts a one-line description from the
// `/** Test: ... */` header comment of the source file when it can find it,
// and marks GPU-heavy tests (those bearing the CTest "gpu" label) with a
// [GPU] decoration and an internal flag the SmartDispatcher honors.
//
// Columns:
//   0  Name
//   1  Status   (Pending | Running | Pass | Fail | Error)
//   2  Duration (seconds, or "-" while pending/running)
//   3  Description (extracted from source header; empty if unavailable)
//
// Selection is check-based: the user ticks rows in column 0 (Qt::CheckStateRole)
// and selectedTests() returns the list the runner should dispatch.
// ----------------------------------------------------------------------------

#pragma once

#include <QAbstractItemModel>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>

namespace ftd::testrunner {

enum class TestStatus {
    Pending,
    Running,
    Pass,
    Fail,
    Error,
};

struct TestInfo {
    QString name;
    QString category;
    QString description;
    QString execPath;           // Absolute path to the test executable
    QStringList command;        // Full ctest command (argv)
    bool isGpuHeavy = false;
    bool checked = false;

    TestStatus status = TestStatus::Pending;
    double durationSec = -1.0;  // negative means "not yet run"
    int failures = 0;
};

class TestModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit TestModel(QObject* parent = nullptr);
    ~TestModel() override;

    // Reload the tree from `ctest --show-only=json-v1` in the given build dir.
    // Returns true if at least one test was discovered.
    bool reload(const QString& buildDir, const QString& config = QStringLiteral("Release"));

    // Currently selected tests (checked in column 0).
    QVector<TestInfo> selectedTests() const;

    // Total test count (excluding categories).
    int testCount() const { return m_tests.size(); }

    // Called by the runner as test status changes.
    void updateStatus(const QString& testName, TestStatus s);
    void updateDuration(const QString& testName, double seconds);
    void updateFailures(const QString& testName, int failures);

    // Mark all / none selected (toolbar helpers).
    void checkAll(bool on);

    // --- QAbstractItemModel interface ---
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    // Classify a bare test name into a category string using CATEGORY_RULES
    // / PREFIX_RULES. Exposed as a static helper so unit tests can hit it.
    static QString categorize(const QString& name);

    // Extract the first non-empty line following "/** Test:" from a source
    // file. Returns an empty string if the file cannot be opened or no such
    // comment is present.
    static QString extractDescription(const QString& sourcePath);

private:
    // Internal lookup helpers.
    int findTestRowByName(const QString& testName) const;
    QString categoryForRow(int testRow) const;

    // Emit dataChanged() for the row owning the given test.
    void notifyRowChanged(int testRow);

    // Two-level representation:
    //   m_categories[c] = category name (in display order)
    //   m_testsByCategory[c] = indices into m_tests for that category
    QStringList m_categories;
    QVector<QVector<int>> m_testsByCategory;
    QVector<TestInfo> m_tests;

    // Internal id layout for QModelIndex.internalId():
    //   Top-level (category) rows:   id = 0xFFFFFFFFull
    //   Test rows:                   id = category index
    static constexpr quintptr kCategoryId = static_cast<quintptr>(-1);
};

}  // namespace ftd::testrunner
