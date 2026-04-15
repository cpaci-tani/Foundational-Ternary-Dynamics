/**
 * Test: Lattice operations
 *
 * Verifies periodic boundary conditions, neighbor access,
 * and coordinate mapping.
 *
 * Theory references:
 *   - SPEC_FTD_LAGRANGIAN.md             (lattice postulate, Moore neighborhood)
 *   - SPEC_SIX_ALGORITHMS.md             (6 algorithms on 3D grid)
 *   - EXPLR_CUBOCTAHEDRAL_GEOMETRY.md    (geometric structure)
 */

#include <iostream>
#include "ftd/lattice.h"
#include "ftd/test_telemetry.h"

using ftd::test::check;

int main() {
    ftd::test::init("test_lattice");

    ftd::Lattice lat(8);

    check("Size = 8", lat.size() == 8);
    check("Total = 512", lat.total_sites() == 512);

    // Index roundtrip
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            for (int z = 0; z < 8; ++z) {
                int idx = lat.index(x, y, z);
                auto c = lat.coord(idx);
                if (c.x != x || c.y != y || c.z != z) {
                    std::cout << "  FAIL  Index roundtrip at (" << x << "," << y << "," << z << ")\n";
                    ftd::test::mark_failure("Index roundtrip");
                    goto done_roundtrip;
                }
            }
        }
    }
    check("Index roundtrip all 512 sites", true);
    done_roundtrip:

    // Periodic wrapping
    check("wrap(8) = 0", lat.wrap(8) == 0);
    check("wrap(-1) = 7", lat.wrap(-1) == 7);
    check("wrap(0) = 0", lat.wrap(0) == 0);
    check("wrap(7) = 7", lat.wrap(7) == 7);
    check("wrap(16) = 0", lat.wrap(16) == 0);

    // Periodic index: index at boundary wraps
    check("index(8,0,0) = index(0,0,0)", lat.index(8, 0, 0) == lat.index(0, 0, 0));
    check("index(-1,0,0) = index(7,0,0)", lat.index(-1, 0, 0) == lat.index(7, 0, 0));

    // 6-neighbors of center voxel
    auto n6 = lat.neighbors_6(lat.index(4, 4, 4));
    check("6-neighbors: +x", n6[0] == lat.index(5, 4, 4));
    check("6-neighbors: -x", n6[1] == lat.index(3, 4, 4));
    check("6-neighbors: +y", n6[2] == lat.index(4, 5, 4));
    check("6-neighbors: -y", n6[3] == lat.index(4, 3, 4));
    check("6-neighbors: +z", n6[4] == lat.index(4, 4, 5));
    check("6-neighbors: -z", n6[5] == lat.index(4, 4, 3));

    // 26-neighbors count
    auto n26 = lat.neighbors_26(lat.index(4, 4, 4));
    // Verify all 26 are distinct and none is center
    int center = lat.index(4, 4, 4);
    bool all_distinct = true;
    bool none_is_center = true;
    for (int i = 0; i < 26; ++i) {
        if (n26[i] == center) none_is_center = false;
        for (int j = i+1; j < 26; ++j) {
            if (n26[i] == n26[j]) all_distinct = false;
        }
    }
    check("26-neighbors all distinct", all_distinct);
    check("26-neighbors none is center", none_is_center);

    // Boundary wrapping for neighbors
    auto n6_corner = lat.neighbors_6(lat.index(0, 0, 0));
    check("Corner -x wraps to 7", n6_corner[1] == lat.index(7, 0, 0));
    check("Corner -y wraps to 7", n6_corner[3] == lat.index(0, 7, 0));
    check("Corner -z wraps to 7", n6_corner[5] == lat.index(0, 0, 7));

    return ftd::test::finalize();
}
