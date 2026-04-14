// ============================================================================
// main.cpp — entry point for the FTD Test Bench runner (ftd_test_runner)
// ============================================================================
//
// Parses --build-dir (default: the directory configured alongside the source
// tree) and spawns a MainWindow. Everything else happens in slots.
// ----------------------------------------------------------------------------

#include "MainWindow.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>
#include <QString>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("FTD"));
    QApplication::setApplicationName(QStringLiteral("FTD Test Bench"));
    QApplication::setApplicationVersion(QStringLiteral("0.3.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("FTD Test Bench — Qt6 native test runner"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption buildDirOpt(
        {QStringLiteral("b"), QStringLiteral("build-dir")},
        QStringLiteral("Path to the CMake build directory (configured with CTest)"),
        QStringLiteral("dir"));
    parser.addOption(buildDirOpt);
    parser.process(app);

    // Default: <cwd>/engine/build_runner if present, else engine/build, else
    // build_runner, else build. This covers the common shapes the engine
    // repo ships with.
    QString buildDir = parser.value(buildDirOpt);
    if (buildDir.isEmpty()) {
        const QStringList candidates{
            QStringLiteral("engine/build_runner"),
            QStringLiteral("engine/build_cuda"),
            QStringLiteral("engine/build"),
            QStringLiteral("build_runner"),
            QStringLiteral("build_cuda"),
            QStringLiteral("build"),
        };
        for (const QString& c : candidates) {
            if (QFileInfo::exists(c + QStringLiteral("/CMakeCache.txt"))) {
                buildDir = QDir(c).absolutePath();
                break;
            }
        }
        if (buildDir.isEmpty()) {
            buildDir = QDir::currentPath();
        }
    } else {
        buildDir = QDir(buildDir).absolutePath();
    }

    ftd::testrunner::MainWindow win(buildDir);
    win.show();
    return app.exec();
}
