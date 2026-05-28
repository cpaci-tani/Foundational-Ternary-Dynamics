# ============================================================================
# engine/tools/test_runner/CMakeLists.txt — FTD Test Bench (Qt6 native)
# ============================================================================
#
# Builds the ftd_test_runner executable. Enabled by the top-level
# engine/CMakeLists.txt when FTD_BUILD_TEST_RUNNER is not OFF and Qt6 is
# found.
#
# The user passes CMAKE_PREFIX_PATH=C:/Qt/6.10.2/msvc2022_64 (or equivalent)
# to the configure command; see engine/archive/scripts_superseded/test_bench_qt6/_build_runner.bat
# (archived 2026-05-27) for the scripted invocation.

find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets Sql Charts)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(TEST_RUNNER_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/TestModel.cpp
    src/TestRunner.cpp
    src/SmartDispatcher.cpp
    src/NdjsonParser.cpp
    src/OutputPanel.cpp
    src/LatticeViewer.cpp
    src/FieldLines.cpp
    src/TelemetryCharts.cpp
    src/HistoryDb.cpp
    src/HistoryTab.cpp
)

set(TEST_RUNNER_HEADERS
    include/MainWindow.h
    include/TestModel.h
    include/TestRunner.h
    include/SmartDispatcher.h
    include/NdjsonParser.h
    include/OutputPanel.h
    include/LatticeViewer.h
    include/FieldLines.h
    include/TelemetryCharts.h
    include/HistoryDb.h
    include/HistoryTab.h
    src/LatticeViewer_shaders.h
)

add_executable(ftd_test_runner WIN32
    ${TEST_RUNNER_SOURCES}
    ${TEST_RUNNER_HEADERS}
    resources/test_runner.qrc
)

target_include_directories(ftd_test_runner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ftd_test_runner PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Sql
    Qt6::Charts
)

target_compile_features(ftd_test_runner PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ftd_test_runner PRIVATE /W3 /permissive-)
else()
    target_compile_options(ftd_test_runner PRIVATE -Wall -Wextra)
endif()
