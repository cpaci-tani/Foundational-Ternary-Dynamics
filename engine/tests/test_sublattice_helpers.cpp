/**
 * @file test_sublattice_helpers.cpp
 * @brief Unit tests for sublattice classification and neighbors_8_corner.
 *
 * Covers:
 *   1. classify_voxel parity correctness on a 4×4×4 block:
 *      SC_SITES count = 8 (all-even), BCC_SITES count = 8 (all-odd),
 *      FCC_SITES count = 48 (mixed). Total = 64.
 *   2. neighbors_8_corner returns 8 distinct indices, all at body-diagonal
 *      offsets (±1,±1,±1), all wrap correctly under periodic boundary.
 *   3. site_matches_filter behaves correctly under ALL_SITES and class match.
 */

#include <array>
#include <cstdio>
#include <set>

#include "ftd/lattice.h"
#include "ftd/sublattice.h"

using namespace ftd;

static int failures = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { std::printf("[FAIL] %s\n", msg); ++failures; } \
    else { std::printf("[ ok ] %s\n", msg); } \
} while (0)

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  Sublattice helpers test\n");
    std::printf("================================================================\n");

    // Test 1: classify_voxel parity counts on a 4x4x4 block.
    // Per-2³-cell: 1 SC + 1 BCC + 6 FCC = 8 voxels. 4³ = 64 = 8×8 cells.
    int n_sc = 0, n_fcc = 0, n_bcc = 0;
    for (int x = 0; x < 4; ++x)
        for (int y = 0; y < 4; ++y)
            for (int z = 0; z < 4; ++z) {
                SiteClass c = classify_voxel(x, y, z);
                if (c == SiteClass::SC_SITES) ++n_sc;
                else if (c == SiteClass::BCC_SITES) ++n_bcc;
                else ++n_fcc;
            }
    std::printf("  4^3 block: SC=%d, BCC=%d, FCC=%d (total=%d)\n",
                n_sc, n_bcc, n_fcc, n_sc + n_bcc + n_fcc);
    CHECK(n_sc  == 8,  "SC site count = 8 in 4^3");
    CHECK(n_bcc == 8,  "BCC site count = 8 in 4^3");
    CHECK(n_fcc == 48, "FCC site count = 48 in 4^3");
    CHECK(n_sc + n_bcc + n_fcc == 64, "total = 64 in 4^3");

    // Spot-check classification.
    CHECK(classify_voxel(0, 0, 0) == SiteClass::SC_SITES,  "(0,0,0) is SC");
    CHECK(classify_voxel(1, 1, 1) == SiteClass::BCC_SITES, "(1,1,1) is BCC");
    CHECK(classify_voxel(2, 2, 2) == SiteClass::SC_SITES,  "(2,2,2) is SC");
    CHECK(classify_voxel(3, 3, 3) == SiteClass::BCC_SITES, "(3,3,3) is BCC");
    CHECK(classify_voxel(0, 1, 0) == SiteClass::FCC_SITES, "(0,1,0) is FCC");
    CHECK(classify_voxel(1, 0, 1) == SiteClass::FCC_SITES, "(1,0,1) is FCC");
    CHECK(classify_voxel(2, 1, 0) == SiteClass::FCC_SITES, "(2,1,0) is FCC");

    // Test 2: neighbors_8_corner — 8 distinct indices, all at body-diagonal offsets.
    Lattice lat(8);
    int center = lat.index(4, 4, 4);
    auto corners = lat.neighbors_8_corner(center);

    // All 8 indices distinct.
    std::set<int> seen(corners.begin(), corners.end());
    CHECK(seen.size() == 8, "neighbors_8_corner: 8 distinct indices");

    // All at body-diagonal offsets (±1,±1,±1) from center.
    int n_correct_offset = 0;
    for (int idx : corners) {
        Coord c = lat.coord(idx);
        int dx = c.x - 4, dy = c.y - 4, dz = c.z - 4;
        // periodic wrap-around: nothing should wrap at L=8 with center=(4,4,4)
        if (std::abs(dx) == 1 && std::abs(dy) == 1 && std::abs(dz) == 1) {
            ++n_correct_offset;
        }
    }
    CHECK(n_correct_offset == 8, "all 8 corners at (±1,±1,±1) offset");

    // Test 2b: periodic wrap at boundary (corner=(0,0,0) → neighbors include (-1,-1,-1) which wraps to (7,7,7)).
    int corner = lat.index(0, 0, 0);
    auto wrapped = lat.neighbors_8_corner(corner);
    std::set<int> wrapped_set(wrapped.begin(), wrapped.end());
    CHECK(wrapped_set.size() == 8, "periodic wrap: 8 distinct indices at (0,0,0)");
    bool has_777 = wrapped_set.count(lat.index(7, 7, 7)) == 1;
    CHECK(has_777, "(0,0,0)'s (-1,-1,-1) corner wraps to (7,7,7)");

    // Test 3: site_matches_filter
    CHECK(site_matches_filter(SiteClass::BCC_SITES, SiteClass::ALL_SITES),
          "ALL_SITES matches anything");
    CHECK(site_matches_filter(SiteClass::BCC_SITES, SiteClass::BCC_SITES),
          "BCC matches BCC filter");
    CHECK(!site_matches_filter(SiteClass::FCC_SITES, SiteClass::BCC_SITES),
          "FCC does NOT match BCC filter");
    CHECK(site_matches_filter(lat, lat.index(1, 1, 1), SiteClass::BCC_SITES),
          "lattice-aware filter on (1,1,1) BCC");
    CHECK(!site_matches_filter(lat, lat.index(0, 1, 0), SiteClass::SC_SITES),
          "lattice-aware filter rejects (0,1,0) for SC");

    std::printf("================================================================\n");
    std::printf("  Result: %s (%d failure(s))\n",
                failures == 0 ? "PASS" : "FAIL", failures);
    std::printf("================================================================\n");
    return failures == 0 ? 0 : 1;
}
