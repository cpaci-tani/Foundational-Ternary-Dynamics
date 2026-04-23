/**
 * @file test_manifestation_background.cpp
 * @brief Unit tests for prepare_manifestation_background.
 *
 * Checks:
 *   T1: n=0 produces zero manifestations, zero flux
 *   T2: n=0.01 produces exactly floor(0.01 * L^3) manifestations (L=16)
 *   T3: net charge sum_Q = 0 when N is even
 *   T4: same seed -> identical manifestation pattern
 *   T5: different seed -> different manifestation pattern
 *   T6: high-density stress - function returns, no infinite loop
 */
#include <cstdio>
#include <cstdlib>
#include <unordered_set>
#include "ftd/eft/manifestation_background.h"

static int g_failures = 0;
#define CHECK(cond, name) do { \
    if (cond) std::printf("  PASS  %s\n", name); \
    else { std::printf("  FAIL  %s\n", name); ++g_failures; } \
} while(0)

int main() {
    using ftd::eft::prepare_manifestation_background;
    using ftd::eft::count_manifested_sites;

    // T1: n=0 => zero manifestations
    {
        auto rb = prepare_manifestation_background(16, 0.0, /*seed=*/1, /*settle=*/0);
        CHECK(count_manifested_sites(*rb) == 0, "T1 n=0 zero manifestations");
    }

    // T2: n=0.01 on L=16 => floor(0.01 * 4096) = 40 manifestations
    {
        auto rb = prepare_manifestation_background(16, 0.01, /*seed=*/1, /*settle=*/0);
        const int n = count_manifested_sites(*rb);
        char msg[128];
        std::snprintf(msg, sizeof(msg), "T2 placed=%d expected=40", n);
        CHECK(n == 40, msg);
    }

    // T3: net charge = 0 for even N
    {
        auto rb = prepare_manifestation_background(16, 0.01, /*seed=*/2, /*settle=*/0);
        int q = 0;
        for (const auto& v : rb->voxels()) q += v.state;
        char msg[128];
        std::snprintf(msg, sizeof(msg), "T3 net charge=%d (expected 0)", q);
        CHECK(q == 0, msg);
    }

    // T4: same seed -> same pattern
    {
        auto rb1 = prepare_manifestation_background(16, 0.01, /*seed=*/7, /*settle=*/0);
        auto rb2 = prepare_manifestation_background(16, 0.01, /*seed=*/7, /*settle=*/0);
        bool same = true;
        const auto& v1 = rb1->voxels();
        const auto& v2 = rb2->voxels();
        for (size_t i = 0; i < v1.size(); ++i) {
            if (v1[i].state != v2[i].state) { same = false; break; }
        }
        CHECK(same, "T4 same seed same pattern");
    }

    // T5: different seed -> different pattern
    {
        auto rb1 = prepare_manifestation_background(16, 0.01, /*seed=*/7, /*settle=*/0);
        auto rb2 = prepare_manifestation_background(16, 0.01, /*seed=*/8, /*settle=*/0);
        bool diff = false;
        const auto& v1 = rb1->voxels();
        const auto& v2 = rb2->voxels();
        for (size_t i = 0; i < v1.size(); ++i) {
            if (v1[i].state != v2[i].state) { diff = true; break; }
        }
        CHECK(diff, "T5 different seed different pattern");
    }

    // T6: high density stress test - function returns, no infinite loop.
    {
        auto rb = prepare_manifestation_background(8, 0.5, /*seed=*/99, /*settle=*/0);
        const int placed = count_manifested_sites(*rb);
        CHECK(placed > 0, "T6 high-density returns with placed > 0");
        CHECK(placed <= 256, "T6 high-density placed <= L^3");
    }

    std::printf("\n%s: %d failures\n", (g_failures == 0 ? "OK" : "FAIL"), g_failures);
    return g_failures;
}
