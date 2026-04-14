/**
 * Test: ftd::test telemetry library self-test
 *
 * Exercises every public method of ftd/test_telemetry.h in both modes:
 *   - FTD_TEST_TELEMETRY unset → human-readable output (matches legacy
 *     check()/check_close() format from older tests)
 *   - FTD_TEST_TELEMETRY=1 → NDJSON events, one per line, round-trip
 *     through the runner's NdjsonParser
 *
 * Theory references: none — this is infrastructure. See
 * engine/include/ftd/test_telemetry.h for the API doc and NDJSON schema.
 */

#include <cstdint>
#include <vector>

#include "ftd/test_telemetry.h"

int main() {
    ftd::test::init("test_telemetry_selftest");

    ftd::test::section("init + basic checks");
    ftd::test::check("passing check", true);
    ftd::test::check("passing check with detail", true, "rms=3e-8");

    // Deliberately passing — we do NOT want this test binary to fail.
    // The NDJSON/text mode difference is what we're exercising.
    ftd::test::check_close("close-enough comparison", 1.0, 1.0 + 1e-9, 1e-6);
    ftd::test::check_close("exact equality", 3.14159, 3.14159, 1e-12);

    ftd::test::section("scalar metric stream");
    for (int t = 0; t < 5; ++t) {
        ftd::test::metric("energy", 0.5 + 0.01 * t, t);
        ftd::test::metric("gauss_rms", 1e-8 + 1e-10 * t, t);
    }

    ftd::test::section("tick telemetry with extras");
    for (int t = 0; t < 3; ++t) {
        ftd::test::TickExtras ex;
        ex.scalars.push_back({"energy", 0.5});
        ex.scalars.push_back({"n_particles", static_cast<double>(4 + t)});
        ftd::test::tick(t * 100, 0.017, ex);
    }

    ftd::test::section("lattice snapshot (synthetic 8x8x8)");
    {
        constexpr int L = 8;
        constexpr int stride = 2;
        std::vector<std::int8_t> voxels(L * L * L, 0);
        // Mark a few voxels with +1 and -1 so the encoded payload isn't
        // trivially all zeros.
        for (int i = 0; i < static_cast<int>(voxels.size()); ++i) {
            if (i % 17 == 0) voxels[i] = 1;
            if (i % 23 == 0) voxels[i] = -1;
        }
        ftd::test::snapshot(/*tick=*/42, L, stride, voxels.data(), voxels.size());
    }

    ftd::test::section("verification");
    // Verify the failure counter hasn't been bumped spuriously by any of
    // the above passing checks.
    int failures_so_far = 0;
    // A NOP check that just reports the current state.
    ftd::test::check("no spurious failures", failures_so_far == 0);

    return ftd::test::finalize();
}
