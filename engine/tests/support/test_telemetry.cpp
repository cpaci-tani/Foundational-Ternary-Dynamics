// ============================================================================
// tests/support/test_telemetry.cpp
// ----------------------------------------------------------------------------
// Implementation of the `ftd::test::` NDJSON telemetry API declared in
// <ftd/test_telemetry.h>. Phase 7 (2026-04-27) extraction: this body used
// to be `inline` in the header, dragging 412 LOC of <iostream>/<chrono>
// machinery into 155+ test TUs on every rebuild. Moving it here makes the
// header ~150 LOC of declarations and ships the impl in the
// `ftd_test_support` static library (linked once per test binary).
//
// Logic is verbatim from the previous header-only version — no behavior
// changes, no new branches. The runner's NDJSON parser sees byte-identical
// output.
// ============================================================================

#include "ftd/test_telemetry.h"

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
static inline bool telemetry_on() {
    return State::instance().ndjson;
}

static inline bool telemetry_requested() {
    const char* env = std::getenv("FTD_TEST_TELEMETRY");
    return env != nullptr && env[0] != '0' && env[0] != '\0';
}

// Escape a string literal for safe embedding in a JSON value. Handles the
// five characters JSON requires be escaped plus control characters.
static std::string json_escape(std::string_view s) {
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
static std::string json_number(double v) {
    if (std::isnan(v) || std::isinf(v)) return "null";
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.17g", v);
    return std::string(buf);
}

static double now_seconds() {
    return std::chrono::duration<double>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

static double elapsed_seconds() {
    return std::chrono::duration<double>(
        std::chrono::steady_clock::now() - State::instance().start).count();
}

}  // namespace detail

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void init(const char* test_name) {
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

void section(const char* name) {
    if (detail::telemetry_on()) {
        std::cout << "{\"event\":\"section\",\"name\":\"" << detail::json_escape(name) << "\"}\n";
    } else {
        std::cout << "\n--- " << name << " ---\n";
    }
    std::cout.flush();
}

void contract(const ConstructorContract& c) {
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

void check(const char* name, bool condition, const char* detail) {
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

void check_close(const char* name, double a, double b, double tol) {
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

void metric(const char* name, double value, int tick) {
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

void tick(int tick_num, double dt, const TickExtras& extras) {
    if (!detail::telemetry_on()) return;
    std::cout << "{\"event\":\"tick\",\"tick\":" << tick_num
              << ",\"dt\":" << detail::json_number(dt);
    for (const auto& [k, v] : extras.scalars) {
        std::cout << ",\"" << detail::json_escape(k) << "\":" << detail::json_number(v);
    }
    std::cout << "}\n";
    std::cout.flush();
}

std::string b64encode(const std::uint8_t* data, std::size_t n) {
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

void snapshot(int tick_num, int L, int stride,
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

int finalize() {
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

void mark_failure(const char* reason) {
    auto& s = detail::State::instance();
    ++s.failures;
    if (s.ndjson) {
        std::cout << "{\"event\":\"check\",\"name\":\"" << detail::json_escape(reason ? reason : "")
                  << "\",\"pass\":false}\n";
    } else {
        std::cout << "  FAIL  " << (reason ? reason : "") << "\n";
    }
    std::cout.flush();
}

}  // namespace test
}  // namespace ftd
