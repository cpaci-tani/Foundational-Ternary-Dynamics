/**
 * test_constructors — unit tests for ftd::ctor::*
 * Spec: docs/superpowers/specs/2026-04-15-ftd-constructors-design.md
 */

#include <algorithm>
#include <array>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "ftd/constructors.h"
#include "ftd/render_bridge.h"
#include "ftd/lattice.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

static void section_level0_flux() {
    ftd::test::section("Level 0 / flux");

    ftd::RenderBridge rb(16);
    ftd::Coord at{8, 8, 8};
    ftd::Vec3 J{0.1, 0.2, 0.3};

    auto r = ftd::ctor::flux(rb, at, J);

    ftd::test::check("F1: name is 'flux'",      std::string(r.name) == "flux");
    ftd::test::check("F2: level is 0",          r.level == 0);
    ftd::test::check("F3: center preserved",    r.center.x == 8 && r.center.y == 8 && r.center.z == 8);
    ftd::test::check("F4: exactly 1 site",      r.sites.size() == 1);

    const int idx = rb.lattice().index(8, 8, 8);
    ftd::test::check("F5: site index is center",       r.sites.size() == 1 && r.sites[0] == idx);

    const auto& v = rb.voxels()[idx];
    ftd::test::check_close("F6: flux.x committed", v.flux.x, 0.1, 1e-12);
    ftd::test::check_close("F7: flux.y committed", v.flux.y, 0.2, 1e-12);
    ftd::test::check_close("F8: flux.z committed", v.flux.z, 0.3, 1e-12);
}

int main() {
    ftd::test::init("test_constructors");
    section_level0_flux();
    return ftd::test::finalize();
}
