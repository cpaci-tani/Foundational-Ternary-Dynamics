// ============================================================================
// ftd/test_telemetry.h — NDJSON telemetry for the FTD Test Bench runner
// ============================================================================
//
// Provides a unified `ftd::test::` API that every C++ test can use to emit
// structured events to stdout. The FTD Test Bench runner
// (engine/tools/test_runner/) parses these NDJSON events live to drive the
// test tree, live output panels, 3D lattice viewer, streaming telemetry
// charts, and SQLite history DB.
//
// DESIGN: zero-overhead when FTD_TEST_TELEMETRY is not set.
//   - Unset:  the helpers print human-readable "  PASS  name" / "  FAIL  name"
//             lines to stdout — byte-identical to the old hand-written
//             `check()` / `check_close()` helpers every test used to define.
//   - Set:    each helper emits one JSON line per event (NDJSON), e.g.
//             {"event":"check","name":"C1","pass":true,"detail":"rms=3e-8"}
//             The runner's NdjsonParser reads these and drives the UI.
//
// USAGE (replaces the old per-file helpers):
//
//   #include "ftd/test_telemetry.h"
//
//   int main() {
//       ftd::test::init("test_gauss");
//       ftd::test::section("Experiment 1: Flux shell");
//
//       // scalar comparisons
//       ftd::test::check("C1: Gauss < 1e-6", rms < 1e-6);
//       ftd::test::check_close("C2: mass ratio", measured, 1836.47, 1e-3);
//
//       // tick-by-tick telemetry (optional, sampled every N ticks in the loop)
//       for (int t = 0; t < 1000; ++t) {
//           rb.tick();
//           if (t % 100 == 0) ftd::test::metric("energy", rb.energy(), t);
//       }
//
//       // lattice snapshots for the live 3D viewer (base64 int8 voxels)
//       ftd::test::snapshot(rb, /*tick=*/500, /*stride=*/4);
//
//       return ftd::test::finalize();   // prints footer, returns failure count
//   }
//
// Protocol (see docs/theory/archive/test_bench_spec.md once landed):
//   Every line is a valid JSON object with required "event" field.
//     {"event":"start",    "test": "...", "pid": N, "ts": <unix>}
//     {"event":"section",  "name": "..."}
//     {"event":"check",    "name": "...", "pass": bool, "detail": "..."}
//     {"event":"contract", "domain": "...", "epistemic_tag": "...", ...}
//     {"event":"metric",   "name": "...", "value": <num>, "tick": N}
//     {"event":"tick",     "tick": N, "dt": <num>, "energy": <num>, ...}
//     {"event":"snapshot", "tick": N, "L": N, "stride": N, "format": "b64-int8", "data": "..."}
//     {"event":"end",      "test": "...", "failures": N, "duration": <sec>}
//
// PHASE 7 (2026-04-27): impl extracted into engine/tests/support/test_telemetry.cpp
// to stop recompiling 412 LOC across 155+ test TUs on every build. The header
// keeps function declarations + the small POD `ConstructorContract` /
// `TickExtras` types + the tiny `valid_contract()` helper inline. Tests link
// the `ftd_test_support` static library to pull in the implementation; they
// continue to `#include "ftd/test_telemetry.h"` exactly as before.
// ----------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace ftd {
namespace test {

// ---------------------------------------------------------------------------
// Forward declaration so RenderBridge can be passed without dragging the
// full header in at compile time. The snapshot encoder for RenderBridge is
// defined in <ftd/test_telemetry_snapshot.h> which tests include alongside
// <ftd/render_bridge.h>. Keeping this header clean of render_bridge.h so
// header-only tests can still use init/section/check/metric without pulling
// in the engine.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Public API — bodies live in tests/support/test_telemetry.cpp.
// ---------------------------------------------------------------------------

void init(const char* test_name);
void section(const char* name);

struct ConstructorContract {
    const char* domain = "";
    const char* epistemic_tag = "";
    const char* required_inputs = "";
    const char* optional_inputs = "";
    const char* observable_map = "";
    const char* closure_domain = "";
    const char* backend_policy = "";
    const char* expected_invariant = "";
    const char* failure_meaning = "";
};

inline bool valid_contract(const ConstructorContract& c) {
    return c.domain && *c.domain &&
           c.epistemic_tag && *c.epistemic_tag &&
           c.required_inputs && *c.required_inputs &&
           c.observable_map && *c.observable_map &&
           c.closure_domain && *c.closure_domain &&
           c.backend_policy && *c.backend_policy &&
           c.expected_invariant && *c.expected_invariant &&
           c.failure_meaning && *c.failure_meaning;
}

void contract(const ConstructorContract& c);

void check(const char* name, bool condition, const char* detail = "");
void check_close(const char* name, double a, double b, double tol);

// Scalar telemetry event for the runner's streaming charts. `tick` is
// optional (-1 means "not tied to a simulation step").
void metric(const char* name, double value, int tick = -1);

// Aggregate tick telemetry. Dispatch call-site decides which scalars to
// include in the optional extras map. Extras are emitted as additional
// JSON fields.
struct TickExtras {
    std::vector<std::pair<const char*, double>> scalars;
};

void tick(int tick_num, double dt, const TickExtras& extras = TickExtras{});

// Base64 encoder (RFC 4648, standard alphabet). Exposed so that the
// RenderBridge-aware snapshot overload in <ftd/test_telemetry_snapshot.h>
// can reuse it.
std::string b64encode(const std::uint8_t* data, std::size_t n);

// Lattice snapshot convenience: takes a generic voxel state array, downsampled
// by `stride`, packed as int8 (values in {-1, 0, +1}), base64-encoded, and
// emitted as a single NDJSON line. The runner's LatticeViewer decodes these
// into its ring buffer at up to 60 Hz playback.
//
// The engine-aware overload that takes a RenderBridge is in
// <ftd/test_telemetry_snapshot.h> to keep this header independent.
void snapshot(int tick_num, int L, int stride,
              const std::int8_t* voxels, std::size_t voxel_count);

// End-of-test footer. Returns the accumulated failure count so the test
// binary's main() can `return ftd::test::finalize();`.
int finalize();

// Manually bump failure count (for edge cases where a test fails outside
// of a check() call, e.g. a try/catch branch).
void mark_failure(const char* reason = "");

}  // namespace test
}  // namespace ftd
