/**
 * Test: Extended lattice topology and wrapping
 *
 * Complements test_lattice.cpp with additional checks:
 * neighbor symmetry, self-reference exclusion, boundary
 * wrapping edge cases, and multi-size sanity.
 *
 * Theory references:
 *   - SPEC_FTD_LAGRANGIAN.md             (lattice postulate)
 *   - EXPLR_CUBOCTAHEDRAL_GEOMETRY.md    (geometric structure)
 */

#include <iostream>
#include <set>
#include "ftd/lattice.h"

int failures = 0;

void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Extended Lattice Operators\n";
    std::cout << "================================================================\n\n";

    // ---- Coord/index roundtrip for small lattice ----
    {
        ftd::Lattice lat(4);
        bool ok = true;
        for (int i = 0; i < lat.total_sites(); ++i) {
            auto c = lat.coord(i);
            if (lat.index(c) != i) { ok = false; break; }
        }
        check("Coord/index roundtrip (N=4, all 64 sites)", ok);
    }

    // ---- wrap() edge cases ----
    {
        ftd::Lattice lat(8);
        check("wrap(-8) = 0", lat.wrap(-8) == 0);
        check("wrap(-9) = 7", lat.wrap(-9) == 7);
        check("wrap(100) = 100%8 = 4", lat.wrap(100) == 4);
        check("wrap(-100) wraps correctly", lat.wrap(-100) >= 0 && lat.wrap(-100) < 8);
    }

    // ---- 6-neighbor symmetry ----
    // If j in neighbors_6(i), then i in neighbors_6(j)
    {
        ftd::Lattice lat(4);
        bool symmetric = true;
        for (int i = 0; i < lat.total_sites(); ++i) {
            auto n6 = lat.neighbors_6(i);
            for (int k = 0; k < 6; ++k) {
                int j = n6[k];
                auto nj = lat.neighbors_6(j);
                bool found = false;
                for (int m = 0; m < 6; ++m) {
                    if (nj[m] == i) { found = true; break; }
                }
                if (!found) { symmetric = false; break; }
            }
            if (!symmetric) break;
        }
        check("6-neighbor symmetry (N=4, all sites)", symmetric);
    }

    // ---- 26-neighbor symmetry ----
    {
        ftd::Lattice lat(4);
        bool symmetric = true;
        for (int i = 0; i < lat.total_sites(); ++i) {
            auto n26 = lat.neighbors_26(i);
            for (int k = 0; k < 26; ++k) {
                int j = n26[k];
                auto nj = lat.neighbors_26(j);
                bool found = false;
                for (int m = 0; m < 26; ++m) {
                    if (nj[m] == i) { found = true; break; }
                }
                if (!found) { symmetric = false; break; }
            }
            if (!symmetric) break;
        }
        check("26-neighbor symmetry (N=4, all sites)", symmetric);
    }

    // ---- No self-reference in neighbors ----
    {
        ftd::Lattice lat(4);
        bool no_self_6 = true;
        bool no_self_26 = true;
        for (int i = 0; i < lat.total_sites(); ++i) {
            auto n6 = lat.neighbors_6(i);
            for (int k = 0; k < 6; ++k) {
                if (n6[k] == i) { no_self_6 = false; break; }
            }
            auto n26 = lat.neighbors_26(i);
            for (int k = 0; k < 26; ++k) {
                if (n26[k] == i) { no_self_26 = false; break; }
            }
            if (!no_self_6 || !no_self_26) break;
        }
        check("No self-reference in neighbors_6", no_self_6);
        check("No self-reference in neighbors_26", no_self_26);
    }

    // ---- All 6-neighbors distinct ----
    {
        ftd::Lattice lat(4);
        bool all_distinct = true;
        for (int i = 0; i < lat.total_sites(); ++i) {
            auto n6 = lat.neighbors_6(i);
            std::set<int> s(n6.begin(), n6.end());
            if ((int)s.size() != 6) { all_distinct = false; break; }
        }
        check("All 6-neighbors distinct (N=4)", all_distinct);
    }

    // ---- All 26-neighbors distinct ----
    {
        ftd::Lattice lat(4);
        bool all_distinct = true;
        for (int i = 0; i < lat.total_sites(); ++i) {
            auto n26 = lat.neighbors_26(i);
            std::set<int> s(n26.begin(), n26.end());
            if ((int)s.size() != 26) { all_distinct = false; break; }
        }
        check("All 26-neighbors distinct (N=4)", all_distinct);
    }

    // ---- Corner wrapping for 26-neighbors ----
    {
        ftd::Lattice lat(4);
        auto n26 = lat.neighbors_26(lat.index(0, 0, 0));
        // The corner (0,0,0) should have neighbor (-1,-1,-1) = (3,3,3)
        int diag = lat.index(3, 3, 3);
        bool has_diag = false;
        for (int k = 0; k < 26; ++k) {
            if (n26[k] == diag) { has_diag = true; break; }
        }
        check("Corner (0,0,0) has diagonal neighbor (3,3,3)", has_diag);
    }

    // ---- Multi-size sanity ----
    {
        int sizes[] = {4, 8, 16, 32};
        bool ok = true;
        for (int s : sizes) {
            ftd::Lattice lat(s);
            if (lat.total_sites() != s * s * s) { ok = false; break; }
            // Spot-check center roundtrip
            int mid = s / 2;
            auto c = lat.coord(lat.index(mid, mid, mid));
            if (c.x != mid || c.y != mid || c.z != mid) { ok = false; break; }
        }
        check("Multi-size sanity (4, 8, 16, 32)", ok);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All extended lattice tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
