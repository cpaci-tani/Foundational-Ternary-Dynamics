// ============================================================================
// TestModel.cpp — CTest-driven Category→Test tree model
// ============================================================================

#include "TestModel.h"

#include <QByteArray>
#include <QColor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QProcess>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>

#include <algorithm>

namespace ftd::testrunner {

// ============================================================================
// Category rules — ported verbatim from the retired engine/run_tests_live.py
// SSE dashboard (Python). See git history for the original table if needed.
// ============================================================================
namespace {

struct CategoryRule {
    QStringList names;
    QString category;
};

static const QVector<CategoryRule>& categoryRules() {
    static const QVector<CategoryRule> rules{
        { {QStringLiteral("constants"), QStringLiteral("lorentz"),
           QStringLiteral("lattice"), QStringLiteral("ontic_chain")},
          QStringLiteral("Core") },
        { {QStringLiteral("born_infeld"), QStringLiteral("energy"),
           QStringLiteral("gauss"), QStringLiteral("stress_energy"),
           QStringLiteral("thermodynamics")},
          QStringLiteral("Core") },
        { {QStringLiteral("lagrangian"), QStringLiteral("magnetic_lagrangian"),
           QStringLiteral("dissipation"), QStringLiteral("variational_coulomb")},
          QStringLiteral("Lagrangian") },
        { {QStringLiteral("maxwell"), QStringLiteral("em_energy_conservation"),
           QStringLiteral("continuity"), QStringLiteral("poynting"),
           QStringLiteral("larmor")},
          QStringLiteral("Electromagnetism") },
        { {QStringLiteral("dipole_radiation"), QStringLiteral("dispersion_relation"),
           QStringLiteral("thomson_scattering"), QStringLiteral("em_fields")},
          QStringLiteral("Electromagnetism") },
        { {QStringLiteral("gauss_convergence"), QStringLiteral("lorentz_force"),
           QStringLiteral("selective_damping")},
          QStringLiteral("Electromagnetism") },
        { {QStringLiteral("wave_collapse"), QStringLiteral("wave_speed"),
           QStringLiteral("interference"), QStringLiteral("gauge"),
           QStringLiteral("polarization")},
          QStringLiteral("Waves & Gauge") },
        { {QStringLiteral("momentum"), QStringLiteral("magnetic"),
           QStringLiteral("flux_mediated"), QStringLiteral("entanglement")},
          QStringLiteral("Waves & Gauge") },
        { {QStringLiteral("genesis"), QStringLiteral("gravity_dynamics"),
           QStringLiteral("annihilation"), QStringLiteral("annihilation_conservation")},
          QStringLiteral("Dynamics") },
        { {QStringLiteral("portable_field"), QStringLiteral("particle_lifetime"),
           QStringLiteral("vortex")},
          QStringLiteral("Dynamics") },
        { {QStringLiteral("voxel_properties"), QStringLiteral("lattice_operators"),
           QStringLiteral("discrete_operators")},
          QStringLiteral("Operators") },
        { {QStringLiteral("bridge_dynamics"), QStringLiteral("csv_export"),
           QStringLiteral("logic_engine")},
          QStringLiteral("Infrastructure") },
        { {QStringLiteral("poisson_coulomb"), QStringLiteral("energy_tracking"),
           QStringLiteral("energy_conservation")},
          QStringLiteral("Energy & Poisson") },
        { {QStringLiteral("selffield_profile"), QStringLiteral("wavepacket")},
          QStringLiteral("Energy & Poisson") },
        { {QStringLiteral("particle_engine"), QStringLiteral("scale_bridge"),
           QStringLiteral("hydrogen_scale1"), QStringLiteral("multiscale_bridge")},
          QStringLiteral("Multi-Scale") },
        { {QStringLiteral("atom_engine"), QStringLiteral("atom_scale_bridge")},
          QStringLiteral("Atom Engine") },
        { {QStringLiteral("dual_substrate")}, QStringLiteral("Dual Substrate") },
        { {QStringLiteral("latency_field")}, QStringLiteral("Latency") },
        { {QStringLiteral("falsifiability")}, QStringLiteral("Falsifiability") },
        { {QStringLiteral("inflation"), QStringLiteral("dark_matter"),
           QStringLiteral("cosmological_constant")},
          QStringLiteral("Cosmology") },
        { {QStringLiteral("reference frame context"), QStringLiteral("sloop")},
          QStringLiteral("Reference frame context") },
        { {QStringLiteral("lorentz_invariance"), QStringLiteral("electroweak"),
           QStringLiteral("hydrogen_em_only")},
          QStringLiteral("Precision") },
        { {QStringLiteral("correlations"), QStringLiteral("ensemble"),
           QStringLiteral("spectral"), QStringLiteral("tracker"),
           QStringLiteral("light"), QStringLiteral("benchmark")},
          QStringLiteral("Analysis") },
    };
    return rules;
}

struct PrefixRule {
    QString prefix;
    QString category;
};

static const QVector<PrefixRule>& prefixRules() {
    static const QVector<PrefixRule> rules{
        { QStringLiteral("pe_"),                 QStringLiteral("PE Extensions") },
        { QStringLiteral("ae_"),                 QStringLiteral("AE Extensions") },
        { QStringLiteral("campaign_ae_"),        QStringLiteral("AE Campaigns") },
        { QStringLiteral("campaign_pe_"),        QStringLiteral("PE Campaigns") },
        { QStringLiteral("campaign_poisson_"),   QStringLiteral("Poisson Campaigns") },
        { QStringLiteral("campaign_"),           QStringLiteral("Campaigns") },
    };
    return rules;
}

}  // namespace

// ============================================================================
// Static helpers
// ============================================================================

QString TestModel::categorize(const QString& name) {
    for (const auto& rule : categoryRules()) {
        if (rule.names.contains(name)) return rule.category;
    }
    for (const auto& rule : prefixRules()) {
        if (name.startsWith(rule.prefix)) return rule.category;
    }
    return QStringLiteral("Other");
}

QString TestModel::extractDescription(const QString& sourcePath) {
    QFile f(sourcePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return {};

    QTextStream in(&f);
    QString contents = in.read(8192);  // 8 KB is more than enough for headers.
    f.close();

    // Pattern: /** Test: <single-line-or-multiline description> */
    //
    // Accept either form: if the first line after "Test:" has text, use it.
    // Otherwise use the next non-empty line (the description sometimes wraps
    // to a new paragraph leading with "*").
    static const QRegularExpression kTestHeaderRe(
        QStringLiteral(R"(/\*\*?\s*\r?\n?\s*\*?\s*Test:\s*([^\r\n]*))"));
    const QRegularExpressionMatch m = kTestHeaderRe.match(contents);
    if (!m.hasMatch()) return {};

    QString firstLine = m.captured(1).trimmed();
    // Strip trailing */ if present inline.
    if (firstLine.endsWith(QStringLiteral("*/"))) {
        firstLine.chop(2);
        firstLine = firstLine.trimmed();
    }
    if (!firstLine.isEmpty()) return firstLine;

    // Fall back: look at the next line in the comment block.
    const int endIdx = m.capturedEnd(1);
    const QString rest = contents.mid(endIdx);
    const QStringList lines = rest.split(QRegularExpression(QStringLiteral("\\r?\\n")));
    for (const QString& raw : lines) {
        QString line = raw.trimmed();
        // Strip leading '*' used in comment continuation.
        if (line.startsWith(QLatin1Char('*'))) {
            line = line.mid(1).trimmed();
        }
        if (line.isEmpty()) continue;
        if (line.startsWith(QStringLiteral("*/"))) break;
        return line;
    }
    return {};
}

// ============================================================================
// Construction / reload
// ============================================================================

TestModel::TestModel(QObject* parent) : QAbstractItemModel(parent) {}
TestModel::~TestModel() = default;

bool TestModel::reload(const QString& buildDir, const QString& config) {
    beginResetModel();
    m_categories.clear();
    m_testsByCategory.clear();
    m_tests.clear();

    QProcess ctest;
    ctest.setWorkingDirectory(buildDir);
    QStringList args{
        QStringLiteral("--test-dir"), buildDir,
        QStringLiteral("-C"), config,
        QStringLiteral("--show-only=json-v1"),
    };
    ctest.start(QStringLiteral("ctest"), args);
    if (!ctest.waitForStarted(10000)) {
        endResetModel();
        return false;
    }
    if (!ctest.waitForFinished(60000)) {
        ctest.kill();
        endResetModel();
        return false;
    }
    const QByteArray stdout_bytes = ctest.readAllStandardOutput();

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(stdout_bytes, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        endResetModel();
        return false;
    }

    // Build a lookup from test name → (LABELS, source hint).
    // ctest --show-only=json-v1 emits:
    //   { "tests": [ { "name": "...", "command": ["<exe>", ...],
    //                  "properties": [ { "name": "LABELS", "value": [...] },
    //                                  { "name": "WORKING_DIRECTORY", "value": "..." } ] } ] }
    const QJsonObject root = doc.object();
    const QJsonArray tests = root.value(QStringLiteral("tests")).toArray();

    // Engine source tree: walk from build dir up to the engine/ directory to
    // find tests/ for description extraction.
    QDir engineDir(buildDir);
    engineDir.cdUp();  // build_runner → engine
    const QString testsDir = engineDir.filePath(QStringLiteral("tests"));

    QMap<QString, QVector<int>> byCategory;

    for (const QJsonValue& v : tests) {
        if (!v.isObject()) continue;
        const QJsonObject obj = v.toObject();
        const QString name = obj.value(QStringLiteral("name")).toString();
        if (name.isEmpty()) continue;

        TestInfo info;
        info.name = name;
        info.category = categorize(name);

        // Command: first element is the executable path.
        const QJsonArray cmd = obj.value(QStringLiteral("command")).toArray();
        QStringList cmdList;
        for (const QJsonValue& cv : cmd) cmdList << cv.toString();
        info.command = cmdList;
        if (!cmdList.isEmpty()) info.execPath = cmdList.first();

        // Properties → LABELS.
        const QJsonArray props = obj.value(QStringLiteral("properties")).toArray();
        for (const QJsonValue& pv : props) {
            if (!pv.isObject()) continue;
            const QJsonObject pobj = pv.toObject();
            if (pobj.value(QStringLiteral("name")).toString() == QStringLiteral("LABELS")) {
                const QJsonArray labels = pobj.value(QStringLiteral("value")).toArray();
                for (const QJsonValue& lv : labels) {
                    if (lv.toString() == QStringLiteral("gpu")) {
                        info.isGpuHeavy = true;
                        break;
                    }
                }
            }
        }

        // Try a few likely source names to pull a description.
        // Tests are named by stripping "test_" / "ftd_", so check:
        //   tests/test_<name>.cpp
        //   tests/campaign_<name>.cpp
        //   tests/benchmark_<name>.cpp
        const QStringList candidates{
            testsDir + QStringLiteral("/test_") + name + QStringLiteral(".cpp"),
            testsDir + QStringLiteral("/") + name + QStringLiteral(".cpp"),
            testsDir + QStringLiteral("/campaign_") + name + QStringLiteral(".cpp"),
            testsDir + QStringLiteral("/benchmark_") + name + QStringLiteral(".cpp"),
        };
        for (const QString& c : candidates) {
            if (QFile::exists(c)) {
                info.description = extractDescription(c);
                if (!info.description.isEmpty()) break;
            }
        }

        const int idx = m_tests.size();
        m_tests.push_back(info);
        byCategory[info.category].push_back(idx);
    }

    // Stable ordered categories: preserve declaration order, "Other" last.
    for (auto it = byCategory.cbegin(); it != byCategory.cend(); ++it) {
        m_categories.append(it.key());
        m_testsByCategory.append(it.value());
    }

    // Sort categories so "Other" sinks to the bottom.
    const QString other = QStringLiteral("Other");
    for (int i = 0; i < m_categories.size(); ++i) {
        if (m_categories[i] == other && i != m_categories.size() - 1) {
            m_categories.append(other);
            m_testsByCategory.append(m_testsByCategory[i]);
            m_categories.removeAt(i);
            m_testsByCategory.removeAt(i);
            break;
        }
    }

    endResetModel();
    return !m_tests.isEmpty();
}

// ============================================================================
// Selection helpers
// ============================================================================

QVector<TestInfo> TestModel::selectedTests() const {
    QVector<TestInfo> out;
    for (const TestInfo& t : m_tests) {
        if (t.checked) out.push_back(t);
    }
    return out;
}

void TestModel::checkAll(bool on) {
    for (TestInfo& t : m_tests) t.checked = on;
    // Refresh both the checkbox column and possibly computed derived values.
    if (!m_tests.isEmpty()) {
        emit dataChanged(
            index(0, 0, QModelIndex()),
            index(m_categories.size() - 1, columnCount(QModelIndex()) - 1, QModelIndex()),
            {Qt::CheckStateRole});
    }
}

// ============================================================================
// Status updates
// ============================================================================

int TestModel::findTestRowByName(const QString& testName) const {
    for (int i = 0; i < m_tests.size(); ++i) {
        if (m_tests[i].name == testName) return i;
    }
    return -1;
}

QString TestModel::categoryForRow(int testRow) const {
    if (testRow < 0 || testRow >= m_tests.size()) return {};
    return m_tests[testRow].category;
}

void TestModel::notifyRowChanged(int testRow) {
    if (testRow < 0) return;
    // Find the parent category and child row index inside it.
    for (int c = 0; c < m_testsByCategory.size(); ++c) {
        const int childPos = m_testsByCategory[c].indexOf(testRow);
        if (childPos >= 0) {
            const QModelIndex parentIdx = index(c, 0, QModelIndex());
            const QModelIndex leftIdx = index(childPos, 0, parentIdx);
            const QModelIndex rightIdx = index(childPos, columnCount(parentIdx) - 1, parentIdx);
            emit dataChanged(leftIdx, rightIdx);
            return;
        }
    }
}

void TestModel::updateStatus(const QString& testName, TestStatus s) {
    const int row = findTestRowByName(testName);
    if (row < 0) return;
    m_tests[row].status = s;
    notifyRowChanged(row);
}

void TestModel::updateDuration(const QString& testName, double seconds) {
    const int row = findTestRowByName(testName);
    if (row < 0) return;
    m_tests[row].durationSec = seconds;
    notifyRowChanged(row);
}

void TestModel::updateFailures(const QString& testName, int failures) {
    const int row = findTestRowByName(testName);
    if (row < 0) return;
    m_tests[row].failures = failures;
    notifyRowChanged(row);
}

// ============================================================================
// QAbstractItemModel interface
// ============================================================================

QModelIndex TestModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) return {};
    if (!parent.isValid()) {
        // Top-level category row.
        return createIndex(row, column, kCategoryId);
    }
    // Child row: stash parent category index in internalId.
    const quintptr categoryIdx = static_cast<quintptr>(parent.row());
    return createIndex(row, column, categoryIdx);
}

QModelIndex TestModel::parent(const QModelIndex& idx) const {
    if (!idx.isValid()) return {};
    if (idx.internalId() == kCategoryId) return {};
    const int categoryIdx = static_cast<int>(idx.internalId());
    if (categoryIdx < 0 || categoryIdx >= m_categories.size()) return {};
    return createIndex(categoryIdx, 0, kCategoryId);
}

int TestModel::rowCount(const QModelIndex& parent) const {
    if (!parent.isValid()) return m_categories.size();
    if (parent.internalId() == kCategoryId) {
        const int cat = parent.row();
        if (cat < 0 || cat >= m_testsByCategory.size()) return 0;
        return m_testsByCategory[cat].size();
    }
    return 0;
}

int TestModel::columnCount(const QModelIndex& /*parent*/) const {
    return 4;
}

QVariant TestModel::data(const QModelIndex& idx, int role) const {
    if (!idx.isValid()) return {};

    // Top-level (category) row.
    if (idx.internalId() == kCategoryId) {
        const int cat = idx.row();
        if (cat < 0 || cat >= m_categories.size()) return {};
        if (role == Qt::DisplayRole && idx.column() == 0) {
            const int n = m_testsByCategory[cat].size();
            return QStringLiteral("%1 (%2)").arg(m_categories[cat]).arg(n);
        }
        if (role == Qt::FontRole && idx.column() == 0) {
            QFont f;
            f.setBold(true);
            return f;
        }
        return {};
    }

    // Test (leaf) row.
    const int cat = static_cast<int>(idx.internalId());
    if (cat < 0 || cat >= m_testsByCategory.size()) return {};
    const int childPos = idx.row();
    if (childPos < 0 || childPos >= m_testsByCategory[cat].size()) return {};
    const int testRow = m_testsByCategory[cat][childPos];
    if (testRow < 0 || testRow >= m_tests.size()) return {};
    const TestInfo& t = m_tests[testRow];

    if (role == Qt::CheckStateRole && idx.column() == 0) {
        return t.checked ? Qt::Checked : Qt::Unchecked;
    }
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (idx.column()) {
        case 0: {
            if (t.isGpuHeavy) {
                return t.name + QStringLiteral("  [GPU]");
            }
            return t.name;
        }
        case 1: {
            switch (t.status) {
            case TestStatus::Pending: return QStringLiteral("pending");
            case TestStatus::Running: return QStringLiteral("running");
            case TestStatus::Pass:    return QStringLiteral("PASS");
            case TestStatus::Fail:    return QStringLiteral("FAIL");
            case TestStatus::Error:   return QStringLiteral("ERROR");
            }
            return {};
        }
        case 2: {
            if (t.durationSec < 0) return QStringLiteral("-");
            return QString::number(t.durationSec, 'f', 2) + QStringLiteral("s");
        }
        case 3: return t.description;
        }
    }
    if (role == Qt::ForegroundRole && idx.column() == 1) {
        switch (t.status) {
        case TestStatus::Pass:    return QColor(0x2e, 0xa0, 0x43);
        case TestStatus::Fail:    return QColor(0xcc, 0x32, 0x32);
        case TestStatus::Error:   return QColor(0x99, 0x22, 0xaa);
        case TestStatus::Running: return QColor(0x2e, 0x6f, 0xd6);
        default: break;
        }
    }
    if (role == Qt::ToolTipRole) {
        QString tip = t.name;
        if (!t.description.isEmpty()) tip += QStringLiteral("\n") + t.description;
        if (t.isGpuHeavy) tip += QStringLiteral("\n[GPU-heavy — dispatched serially]");
        if (!t.execPath.isEmpty()) tip += QStringLiteral("\n") + t.execPath;
        return tip;
    }
    return {};
}

bool TestModel::setData(const QModelIndex& idx, const QVariant& value, int role) {
    if (!idx.isValid()) return false;
    if (role != Qt::CheckStateRole || idx.column() != 0) return false;

    // Category toggle cascades to children.
    if (idx.internalId() == kCategoryId) {
        const int cat = idx.row();
        if (cat < 0 || cat >= m_testsByCategory.size()) return false;
        const bool on = (value.toInt() == Qt::Checked);
        for (int testRow : m_testsByCategory[cat]) {
            if (testRow >= 0 && testRow < m_tests.size()) {
                m_tests[testRow].checked = on;
            }
        }
        const QModelIndex leftChild = index(0, 0, idx);
        const QModelIndex rightChild = index(m_testsByCategory[cat].size() - 1,
                                             columnCount(idx) - 1, idx);
        emit dataChanged(leftChild, rightChild, {Qt::CheckStateRole});
        emit dataChanged(idx, idx, {Qt::CheckStateRole});
        return true;
    }

    // Leaf row toggle.
    const int cat = static_cast<int>(idx.internalId());
    if (cat < 0 || cat >= m_testsByCategory.size()) return false;
    const int childPos = idx.row();
    if (childPos < 0 || childPos >= m_testsByCategory[cat].size()) return false;
    const int testRow = m_testsByCategory[cat][childPos];
    m_tests[testRow].checked = (value.toInt() == Qt::Checked);
    emit dataChanged(idx, idx, {Qt::CheckStateRole});
    return true;
}

Qt::ItemFlags TestModel::flags(const QModelIndex& idx) const {
    if (!idx.isValid()) return Qt::NoItemFlags;
    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (idx.column() == 0) {
        f |= Qt::ItemIsUserCheckable;
    }
    return f;
}

QVariant TestModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
    switch (section) {
    case 0: return QStringLiteral("Test");
    case 1: return QStringLiteral("Status");
    case 2: return QStringLiteral("Duration");
    case 3: return QStringLiteral("Description");
    }
    return {};
}

}  // namespace ftd::testrunner
