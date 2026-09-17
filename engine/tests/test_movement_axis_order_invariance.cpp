/**
 * Test: the symmetric_movement_order axis permutation is mathematically inert.
 *
 * WHAT THIS PINS. `extract_remainder_hops` (ftd/movement_order.h) draws a
 * per-site permutation of {x, y, z} when symmetric_movement_order is ON and
 * then calls hop_axis in that order. Each hop_axis call reads and writes a
 * DISJOINT (rem, d) pair — there is no shared accumulator, no early exit, and
 * no cross-axis carry — so all 6 orders produce bit-identical dx/dy/dz and
 * bit-identical residual remainders. The axis-ordering half of the toggle is
 * therefore redundant given how hop extraction is structured; the half that
 * genuinely delivers coordinate-independent traversal is the Fisher-Yates SITE
 * shuffle (movement_shuffle_j).
 *
 * This is a documentation-and-lock test, not a bug reproduction. It exists so
 * that a future reader who notices the inertness cannot quietly "fix" it into
 * an order-sensitive variant without a failing test and a physics review: any
 * such change would alter particle transport under a toggle whose contract
 * says nothing about order-dependent hops.
 *
 * Checks:
 *   MAO-1: over an exhaustive grid of remainder triples (including the exact
 *          +/-1 boundaries and multi-axis simultaneous hops), all 6 axis
 *          orders give bit-identical (dx, dy, dz) and bit-identical residuals.
 *   MAO-2: the symmetric path with ANY drawn permutation equals the
 *          non-symmetric (x, y, z) path bit-exactly, over the same grid and
 *          over every (seed, site, tick) that selects each of the 6 orders.
 *   MAO-3: movement_axis_perm really does produce all 6 permutations (so
 *          MAO-1/2 are not vacuously covering one branch), and every result is
 *          a valid permutation of {0, 1, 2}.
 */

#include "ftd/movement_order.h"
#include "ftd/test_telemetry.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <set>

namespace {

// The 6 axis orders, spelled out independently of movement_axis_perm so the
// test does not inherit a bug from the code under test.
constexpr int kOrders[6][3] = {
    {0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};

struct HopResult {
    double rx, ry, rz;
    int dx, dy, dz;
    bool operator==(const HopResult& o) const {
        return rx == o.rx && ry == o.ry && rz == o.rz
            && dx == o.dx && dy == o.dy && dz == o.dz;
    }
};

// Local re-implementation of extract_remainder_hops' body with an EXPLICIT
// axis order, so we can drive all 6 without needing an RNG that happens to
// select them.
HopResult hop_in_order(double rx, double ry, double rz, const int axes[3]) {
    int dx = 0, dy = 0, dz = 0;
    auto hop_axis = [](double& rem, int& d) {
        if (rem >= 1.0) { d = 1; rem -= 1.0; }
        else if (rem <= -1.0) { d = -1; rem += 1.0; }
    };
    for (int k = 0; k < 3; ++k) {
        if (axes[k] == 0) hop_axis(rx, dx);
        else if (axes[k] == 1) hop_axis(ry, dy);
        else hop_axis(rz, dz);
    }
    return HopResult{rx, ry, rz, dx, dy, dz};
}

// Remainder samples that exercise every branch of hop_axis, including the
// exact comparison boundaries and values that trip all three axes at once.
const double kSamples[] = {
    -2.5, -1.75, -1.0, -0.999999999, -0.5, 0.0,
     0.5,  0.999999999, 1.0, 1.25, 1.75, 2.5};
constexpr int kNumSamples = static_cast<int>(sizeof(kSamples) / sizeof(kSamples[0]));

}  // namespace

int main() {
    ftd::test::init("test_movement_axis_order_invariance");

    // ------------------------------------------------------------------
    ftd::test::section("MAO-1: all 6 axis orders are bit-identical");
    {
        long long compared = 0;
        bool identical = true;
        for (int i = 0; i < kNumSamples && identical; ++i)
        for (int j = 0; j < kNumSamples && identical; ++j)
        for (int k = 0; k < kNumSamples && identical; ++k) {
            const HopResult ref =
                hop_in_order(kSamples[i], kSamples[j], kSamples[k], kOrders[0]);
            for (int p = 1; p < 6; ++p) {
                const HopResult got =
                    hop_in_order(kSamples[i], kSamples[j], kSamples[k], kOrders[p]);
                ++compared;
                if (!(got == ref)) {
                    std::printf("    MISMATCH rem=(%g,%g,%g) order %d%d%d: "
                                "d=(%d,%d,%d) vs ref (%d,%d,%d)\n",
                                kSamples[i], kSamples[j], kSamples[k],
                                kOrders[p][0], kOrders[p][1], kOrders[p][2],
                                got.dx, got.dy, got.dz, ref.dx, ref.dy, ref.dz);
                    identical = false;
                    break;
                }
            }
        }
        std::printf("    compared %lld order-pairs over %d^3 remainder triples\n",
                    compared, kNumSamples);
        ftd::test::metric("mao1.comparisons", static_cast<double>(compared), 0);
        ftd::test::check("MAO-1: axis order does not affect (dx,dy,dz) or the "
                         "residual remainders (disjoint per-axis state)",
                         identical);
    }

    // ------------------------------------------------------------------
    ftd::test::section("MAO-2: symmetric path == non-symmetric path");
    {
        // Find one (seed, site, tick) per drawn permutation so every branch of
        // movement_axis_perm is exercised through the REAL entry point.
        std::array<int, 6> site_for_perm{};
        std::array<bool, 6> found{};
        const std::uint64_t seed = 0x9E3779B97F4A7C15ull;
        const int tick = 7;
        for (int site = 0; site < 20000; ++site) {
            int axes[3];
            ftd::movement_axis_perm(seed, site, tick, axes);
            int which = -1;
            for (int p = 0; p < 6; ++p)
                if (axes[0] == kOrders[p][0] && axes[1] == kOrders[p][1]
                    && axes[2] == kOrders[p][2]) which = p;
            if (which >= 0 && !found[which]) {
                found[which] = true;
                site_for_perm[which] = site;
            }
        }
        int covered = 0;
        for (int p = 0; p < 6; ++p) covered += found[p] ? 1 : 0;
        std::printf("    permutations reached by movement_axis_perm: %d/6\n", covered);
        ftd::test::check("MAO-2a: movement_axis_perm reaches all 6 permutations",
                         covered == 6);

        bool equal = true;
        long long compared = 0;
        for (int p = 0; p < 6 && equal; ++p) {
            if (!found[p]) continue;
            for (int i = 0; i < kNumSamples && equal; ++i)
            for (int j = 0; j < kNumSamples && equal; ++j)
            for (int k = 0; k < kNumSamples && equal; ++k) {
                double ax = kSamples[i], ay = kSamples[j], az = kSamples[k];
                int adx, ady, adz;
                ftd::extract_remainder_hops(ax, ay, az, adx, ady, adz,
                                            /*symmetric=*/false, seed,
                                            site_for_perm[p], tick);
                double bx = kSamples[i], by = kSamples[j], bz = kSamples[k];
                int bdx, bdy, bdz;
                ftd::extract_remainder_hops(bx, by, bz, bdx, bdy, bdz,
                                            /*symmetric=*/true, seed,
                                            site_for_perm[p], tick);
                ++compared;
                if (ax != bx || ay != by || az != bz
                    || adx != bdx || ady != bdy || adz != bdz) {
                    std::printf("    MISMATCH perm %d site %d rem=(%g,%g,%g)\n",
                                p, site_for_perm[p], kSamples[i], kSamples[j],
                                kSamples[k]);
                    equal = false;
                }
            }
        }
        std::printf("    compared %lld symmetric/non-symmetric pairs\n", compared);
        ftd::test::check("MAO-2b: extract_remainder_hops(symmetric=true) is "
                         "bit-identical to symmetric=false for every drawn "
                         "permutation",
                         equal && compared > 0);
    }

    // ------------------------------------------------------------------
    ftd::test::section("MAO-3: movement_axis_perm output is always a permutation");
    {
        bool valid = true;
        for (int site = 0; site < 5000 && valid; ++site) {
            int axes[3];
            ftd::movement_axis_perm(0xDEADBEEFull, site, site * 3 + 1, axes);
            std::set<int> s{axes[0], axes[1], axes[2]};
            if (s.size() != 3 || *s.begin() != 0 || *s.rbegin() != 2) {
                std::printf("    invalid permutation at site %d: (%d,%d,%d)\n",
                            site, axes[0], axes[1], axes[2]);
                valid = false;
            }
        }
        ftd::test::check("MAO-3: every drawn axis order is a permutation of "
                         "{0,1,2}",
                         valid);
    }

    return ftd::test::finalize();
}
