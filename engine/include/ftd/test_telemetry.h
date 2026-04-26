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
// Header-only for simplicity — no extra object file to link. Heavy lifting
// (snapshot encoding, etc.) is inlined; callers must tolerate that.
// ----------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifdef _WIN32
#  define NOMINMAX
#  include <process.h>   // _getpid
#  define FTD_GETPID()   _getpid()
#else
#  include <unistd.h>
#  define FTD_GETPID()   getpid()
#endif

namespace ftd {
namespace test {

// ---------------------------------------------------------------------------
// Forward declaration so RenderBridge can be passed without dragging the
// full header in at compile time. The snapshot encoder is defined below
// and only needs the forward decl at the point of declaration.
// ---------------------------------------------------------------------------
// (intentionally left blank — the snapshot overload that takes a
//  RenderBridge is defined inline in <ftd/test_telemetry_snapshot.h> which
//  tests include alongside <ftd/render_bridge.h>. Keeping the main header
//  clean of render_bridge.h so that header-only tests can still use
//  init/section/check/metric without pulling in the engine.)

// ---------------------------------------------------------------------------
// Internal state — single TU per test binary (tests are individual .exe).
// ---------------------------------------------------------------------------
namespace detail {

struct State {
    bool ndjson = false;             // FTD_TEST_TELEMETRY env var at init
    int failures = 0;
    std::string test_name;
    std::chrono::steady_clock::time_point start;

    static State& instance() {
        static State s;
        return s;
    }
};

// Cheap runtime check; memoized after init().
inline bool telemetry_on() {
    return State::instance().ndjson;
}

inline bool telemetry_requested() {
    const char* env = std::getenv("FTD_TEST_TELEMETRY");
    return env != nullptr && env[0] != '0' && env[0] != '\0';
}

// Escape a string literal for safe embedding in a JSON value. Handles the
// five characters JSON requires be escaped plus control characters.
inline std::string json_escape(std::string_view s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        case '\b': out += "\\b";  break;
        case '\f': out += "\\f";  break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<int>(c));
                out += buf;
            } else {
                out += c;
            }
        }
    }
    return out;
}

// Format a double with enough precision to round-trip, but trim trailing
// zeros to keep lines small. Produces NaN / Infinity handling identical
// to what Qt's QJsonDocument expects (we emit JSON null for non-finite).
inline std::string json_number(double v) {
    if (std::isnan(v) || std::isinf(v)) return "null";
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.17g", v);
    return std::string(buf);
}

inline double now_seconds() {
    return std::chrono::duration<double>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

inline double elapsed_seconds() {
    return std::chrono::duration<double>(
        std::chrono::steady_clock::now() - State::instance().start).count();
}

}  // namespace detail

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

inline void init(const char* test_name) {
    auto& s = detail::State::instance();
    s.ndjson = detail::telemetry_requested();
    s.failures = 0;
    s.test_name = test_name ? test_name : "";
    s.start = std::chrono::steady_clock::now();

    if (s.ndjson) {
        std::cout << "{\"event\":\"start\",\"test\":\"" << detail::json_escape(s.test_name)
                  << "\",\"pid\":" << FTD_GETPID()
                  << ",\"ts\":" << detail::json_number(detail::now_seconds()) << "}\n";
    } else {
        std::cout << "================================================================\n";
        std::cout << "  TEST: " << s.test_name << "\n";
        std::cout << "================================================================\n\n";
    }
    std::cout.flush();
}

inline void section(const char* name) {
    if (detail::telemetry_on()) {
        std::cout << "{\"event\":\"section\",\"name\":\"" << detail::json_escape(name) << "\"}\n";
    } else {
        std::cout << "\n--- " << name << " ---\n";
    }
    std::cout.flush();
}

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

inline void contract(const ConstructorContract& c) {
    const bool ndjson = detail::telemetry_on() || detail::telemetry_requested();
    if (ndjson) {
        std::cout << "{\"event\":\"contract\""
                  << ",\"domain\":\"" << detail::json_escape(c.domain ? c.domain : "") << "\""
                  << ",\"epistemic_tag\":\"" << detail::json_escape(c.epistemic_tag ? c.epistemic_tag : "") << "\""
                  << ",\"required_inputs\":\"" << detail::json_escape(c.required_inputs ? c.required_inputs : "") << "\""
                  << ",\"optional_inputs\":\"" << detail::json_escape(c.optional_inputs ? c.optional_inputs : "") << "\""
                  << ",\"observable_map\":\"" << detail::json_escape(c.observable_map ? c.observable_map : "") << "\""
                  << ",\"closure_domain\":\"" << detail::json_escape(c.closure_domain ? c.closure_domain : "") << "\""
                  << ",\"backend_policy\":\"" << detail::json_escape(c.backend_policy ? c.backend_policy : "") << "\""
                  << ",\"expected_invariant\":\"" << detail::json_escape(c.expected_invariant ? c.expected_invariant : "") << "\""
                  << ",\"failure_meaning\":\"" << detail::json_escape(c.failure_meaning ? c.failure_meaning : "") << "\""
                  << "}\n";
    } else {
        std::cout << "\n--- CONTRACT: " << (c.domain ? c.domain : "") << " ---\n"
                  << "  epistemic_tag: " << (c.epistemic_tag ? c.epistemic_tag : "") << "\n"
                  << "  required_inputs: " << (c.required_inputs ? c.required_inputs : "") << "\n";
        if (c.optional_inputs && *c.optional_inputs) {
            std::cout << "  optional_inputs: " << c.optional_inputs << "\n";
        }
        std::cout << "  observable_map: " << (c.observable_map ? c.observable_map : "") << "\n"
                  << "  closure_domain: " << (c.closure_domain ? c.closure_domain : "") << "\n"
                  << "  backend_policy: " << (c.backend_policy ? c.backend_policy : "") << "\n"
                  << "  expected_invariant: " << (c.expected_invariant ? c.expected_invariant : "") << "\n"
                  << "  failure_meaning: " << (c.failure_meaning ? c.failure_meaning : "") << "\n";
    }
    std::cout.flush();
}

inline void check(const char* name, bool condition, const char* detail = "") {
    auto& s = detail::State::instance();
    if (!condition) ++s.failures;
    if (s.ndjson) {
        std::cout << "{\"event\":\"check\",\"name\":\""
                  << detail::json_escape(name)
                  << "\",\"pass\":" << (condition ? "true" : "false");
        if (detail && *detail) {
            std::cout << ",\"detail\":\"" << detail::json_escape(detail) << "\"";
        }
        std::cout << "}\n";
    } else {
        std::cout << "  " << (condition ? "PASS" : "FAIL") << "  " << name << "\n";
        if (!condition && detail && *detail) {
            std::cout << "        " << detail << "\n";
        }
    }
    std::cout.flush();
}

inline void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::fabs(a - b) < tol;
    auto& s = detail::State::instance();
    if (!ok) ++s.failures;
    if (s.ndjson) {
        std::cout << "{\"event\":\"check\",\"name\":\""
                  << detail::json_escape(name)
                  << "\",\"pass\":" << (ok ? "true" : "false")
                  << ",\"got\":" << detail::json_number(a)
                  << ",\"expected\":" << detail::json_number(b)
                  << ",\"tol\":" << detail::json_number(tol)
                  << ",\"diff\":" << detail::json_number(std::fabs(a - b))
                  << "}\n";
    } else {
        std::cout << "  " << (ok ? "PASS" : "FAIL") << "  " << name;
        if (!ok) {
            std::cout << " (got " << std::setprecision(15) << a
                      << ", expected " << b
                      << ", diff " << std::fabs(a - b) << ")";
        }
        std::cout << "\n";
    }
    std::cout.flush();
}

// Scalar telemetry event for the runner's streaming charts. `tick` is
// optional (-1 means "not tied to a simulation step").
inline void metric(const char* name, double value, int tick = -1) {
    if (detail::telemetry_on()) {
        std::cout << "{\"event\":\"metric\",\"name\":\""
                  << detail::json_escape(name)
                  << "\",\"value\":" << detail::json_number(value);
        if (tick >= 0) std::cout << ",\"tick\":" << tick;
        std::cout << "}\n";
        std::cout.flush();
    }
    // Silent when telemetry is off — scalar series would bloat human-readable output.
}

// Aggregate tick telemetry. Dispatch call-site decides which scalars to
// include in the optional extras map. Extras are emitted as additional
// JSON fields.
struct TickExtras {
    std::vector<std::pair<const char*, double>> scalars;
};

inline void tick(int tick_num, double dt, const TickExtras& extras = TickExtras{}) {
    if (!detail::telemetry_on()) return;
    std::cout << "{\"event\":\"tick\",\"tick\":" << tick_num
              << ",\"dt\":" << detail::json_number(dt);
    for (const auto& [k, v] : extras.scalars) {
        std::cout << ",\"" << detail::json_escape(k) << "\":" << detail::json_number(v);
    }
    std::cout << "}\n";
    std::cout.flush();
}

// Base64 encoder (RFC 4648, standard alphabet, no padding skip). Tiny and
// inlined so we don't need a separate implementation file.
inline std::string b64encode(const std::uint8_t* data, std::size_t n) {
    static constexpr char kAlphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((n + 2) / 3) * 4);
    for (std::size_t i = 0; i < n; i += 3) {
        std::uint32_t v = static_cast<std::uint32_t>(data[i]) << 16;
        if (i + 1 < n) v |= static_cast<std::uint32_t>(data[i + 1]) << 8;
        if (i + 2 < n) v |= static_cast<std::uint32_t>(data[i + 2]);
        out.push_back(kAlphabet[(v >> 18) & 0x3f]);
        out.push_back(kAlphabet[(v >> 12) & 0x3f]);
        out.push_back(i + 1 < n ? kAlphabet[(v >> 6) & 0x3f] : '=');
        out.push_back(i + 2 < n ? kAlphabet[v & 0x3f] : '=');
    }
    return out;
}

// Lattice snapshot convenience: takes a generic voxel state array, downsampled
// by `stride`, packed as int8 (values in {-1, 0, +1}), base64-encoded, and
// emitted as a single NDJSON line. The runner's LatticeViewer decodes these
// into its ring buffer at up to 60 Hz playback.
//
// The engine-aware overload that takes a RenderBridge is in
// <ftd/test_telemetry_snapshot.h> to keep this header independent.
inline void snapshot(int tick_num, int L, int stride,
                     const std::int8_t* voxels, std::size_t voxel_count) {
    if (!detail::telemetry_on()) return;
    std::string encoded = b64encode(
        reinterpret_cast<const std::uint8_t*>(voxels), voxel_count);
    std::cout << "{\"event\":\"snapshot\",\"tick\":" << tick_num
              << ",\"L\":" << L
              << ",\"stride\":" << stride
              << ",\"format\":\"b64-int8\""
              << ",\"data\":\"" << encoded << "\"}\n";
    std::cout.flush();
}

// End-of-test footer. Returns the accumulated failure count so the test
// binary's main() can `return ftd::test::finalize();`.
inline int finalize() {
    auto& s = detail::State::instance();
    double dur = detail::elapsed_seconds();
    if (s.ndjson) {
        std::cout << "{\"event\":\"end\",\"test\":\"" << detail::json_escape(s.test_name)
                  << "\",\"failures\":" << s.failures
                  << ",\"duration\":" << detail::json_number(dur) << "}\n";
    } else {
        std::cout << "\n================================================================\n";
        if (s.failures == 0) {
            std::cout << "  RESULTS: ALL PASS\n";
        } else {
            std::cout << "  RESULTS: FAILURES DETECTED\n";
            std::cout << "  Failures: " << s.failures << "\n";
        }
        std::cout << "================================================================\n";
    }
    std::cout.flush();
    return s.failures;
}

// Manually bump failure count (for edge cases where a test fails outside
// of a check() call, e.g. a try/catch branch).
inline void mark_failure(const char* reason = "") {
    auto& s = detail::State::instance();
    ++s.failures;
    if (s.ndjson) {
        std::cout << "{\"event\":\"check\",\"name\":\"" << detail::json_escape(reason)
                  << "\",\"pass\":false}\n";
    } else {
        std::cout << "  FAIL  " << reason << "\n";
    }
    std::cout.flush();
}

}  // namespace test
}  // namespace ftd
