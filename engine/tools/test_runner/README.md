# FTD Test Bench (Phase 3 scaffold)

Qt6 Widgets native test runner for the FTD C++ engine. Reads the CTest
inventory, runs tests as subprocesses, parses streaming NDJSON telemetry
(see `engine/include/ftd/test_telemetry.h`), and displays results in a
main window with a category-grouped test tree, live output panel, and
placeholders for the lattice viewer, telemetry charts, and history DB
that land in Phases 4-6.

## What's in this phase

Phase 3 ships the scaffold + core runner pipeline:

- **`TestModel`** — `QAbstractItemModel` that reads `ctest --show-only=json-v1`,
  groups tests into categories (rules ported from the retired SSE dashboard
  at `engine/run_tests_live.py`, now superseded by this runner), detects
  GPU-heavy tests from the CTest `gpu` label, and extracts the first line
  of each test's `/** Test: ... */` header comment as a description.
- **`TestRunner`** — spawns one `QProcess` per test, feeds its stdout through
  an `NdjsonParser`, and re-emits structured events (`testStarted`,
  `checkReceived`, `metricReceived`, `snapshotReceived`, `testFinished`).
  Handles crashes and non-zero exit codes without leaking subprocesses.
- **`NdjsonParser`** — line-buffered consumer of the Phase 2a NDJSON protocol.
  Falls back to regex parsing of `  PASS  name` / `  FAIL  name` for
  non-instrumented tests.
- **`SmartDispatcher`** — serial GPU queue + parallel CPU queue (N-1 workers,
  clamped to [1, 16]). Tracks in-flight jobs by name so it always decrements
  the correct counter when a test finishes.
- **`OutputPanel`** — interleaved `[test_name] line` output view with check
  pass/fail coloring.
- **`MainWindow`** — splitter with the test tree on the left and a
  `QTabWidget` (Output + placeholders) on the right, plus toolbar, menus,
  and a status bar with pass/fail counts, progress bar, and elapsed time.

**Deferred:** the `Live Lattice` tab renders a static placeholder label —
the real `LatticeViewer` is Phase 4. Same for `Telemetry` (Phase 5) and
`History` (Phase 6). These subagents extend the existing MainWindow tabs
without rewriting the surrounding plumbing.

## Build

### Prerequisites

- Qt 6.10.2 at `C:\Qt\6.10.2\msvc2022_64` (adjust `CMAKE_PREFIX_PATH` for
  other installs)
- CUDA 13.0 (optional, only if `FTD_ENABLE_CUDA=ON`)
- Visual Studio 2026 (v18) — detected via `vswhere.exe`

### One-shot build

From the engine directory (or use the helper):

```bash
# Helper script — sets up vcvars64 and invokes cmake with the right paths
cmd.exe //c "engine\_build_runner.bat"
```

Or manually:

```bash
cmake -S engine -B engine/build_runner \
    -DFTD_ENABLE_CUDA=ON \
    -DCMAKE_PREFIX_PATH=C:/Qt/6.10.2/msvc2022_64
cmake --build engine/build_runner --config Release --target ftd_test_runner
```

The configure step prints `FTD Test Bench: enabled (Qt6 6.10.2)` when
everything is in place. If Qt6 is missing, the build gracefully disables
the runner and prints `FTD Test Bench: disabled (Qt6 not found)` — all
other engine targets still build.

### Output

After a successful build:

```
engine/build_runner/tools/test_runner/Release/ftd_test_runner.exe
```

## Run

```bash
engine\build_runner\tools\test_runner\Release\ftd_test_runner.exe \
    --build-dir engine/build_runner
```

The `--build-dir` argument points at any configured CMake build directory.
The runner auto-detects a few common locations (`engine/build_runner`,
`engine/build_cuda`, `engine/build`) if the argument is omitted.

## Notes for future phases

- `TestRunner::snapshotReceived` is already wired through `MainWindow` but
  currently discards the event. Phase 4 connects it to `LatticeViewer`.
- `TestRunner::metricReceived` currently feeds a sampled view into
  `OutputPanel`. Phase 5 adds a full `TelemetryCharts` tab that consumes
  every metric.
- `MainWindow` does not yet persist runs to SQLite. Phase 6 adds
  `HistoryDb` and a history tab that queries it.
- GPU-heavy tests are detected via the CTest `gpu` label, not by pattern
  matching. Adding new GPU tests via `ftd_add_test(... GPU_HEAVY)` is
  enough — no runner changes required.

## Protocol reference

See `engine/include/ftd/test_telemetry.h` for the NDJSON event schema. The
parser expects one JSON object per line with a required `event` field, one
of: `start`, `section`, `check`, `metric`, `tick`, `snapshot`, `end`.
