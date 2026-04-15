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

namespace {

std::tuple<int,int,int> rotate_offset_90(int dx, int dy, int dz, int axis) {
    switch (axis) {
        case 0: return { dx, -dz,  dy };
        case 1: return { dz,  dy, -dx };
        default: return { -dy, dx,  dz };
    }
}

std::set<std::tuple<int,int,int>> offsets_from_sites(
    const ftd::Lattice& lat,
    ftd::Coord center,
    const std::vector<int>& sites)
{
    std::set<std::tuple<int,int,int>> out;
    const int N = lat.size();
    for (int idx : sites) {
        ftd::Coord c = lat.coord(idx);
        int dx = c.x - center.x;
        int dy = c.y - center.y;
        int dz = c.z - center.z;
        if (dx >  N/2) dx -= N;  if (dx < -N/2) dx += N;
        if (dy >  N/2) dy -= N;  if (dy < -N/2) dy += N;
        if (dz >  N/2) dz -= N;  if (dz < -N/2) dz += N;
        out.insert({dx, dy, dz});
    }
    return out;
}

bool is_rotation_invariant(const std::set<std::tuple<int,int,int>>& s, int axis) {
    std::set<std::tuple<int,int,int>> rotated;
    for (const auto& [dx, dy, dz] : s) {
        rotated.insert(rotate_offset_90(dx, dy, dz, axis));
    }
    return rotated == s;
}

bool no_leakage(const ftd::RenderBridge& rb, ftd::Coord center,
                const std::vector<int>& stamped) {
    std::set<int> touched(stamped.begin(), stamped.end());
    const ftd::Lattice& lat = rb.lattice();
    const auto& vox = rb.voxels();
    for (int dx = -2; dx <= 2; ++dx)
    for (int dy = -2; dy <= 2; ++dy)
    for (int dz = -2; dz <= 2; ++dz) {
        int idx = lat.index(center.x + dx, center.y + dy, center.z + dz);
        if (touched.count(idx)) continue;
        const auto& v = vox[idx];
        if (v.state != 0) return false;
        if (v.flux.x != 0 || v.flux.y != 0 || v.flux.z != 0) return false;
    }
    return true;
}

}  // anonymous namespace

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

static void section_level0_particle() {
    ftd::test::section("Level 0 / particle");

    ftd::RenderBridge rb(16);
    ftd::Coord at{8, 8, 8};
    ftd::Vec3 J{0.0, 0.0, ftd::K_B};

    auto r = ftd::ctor::particle(rb, at, +1, J, /*spin=*/+1, /*color=*/2);

    ftd::test::check("P1: name is 'particle'", std::string(r.name) == "particle");
    ftd::test::check("P2: level is 0",         r.level == 0);
    ftd::test::check("P3: exactly 1 site",     r.sites.size() == 1);

    const int idx = rb.lattice().index(8, 8, 8);
    const auto& v = rb.voxels()[idx];
    ftd::test::check("P4: state = +1",         v.state == 1);
    ftd::test::check_close("P5: flux.z = K_B", v.flux.z, ftd::K_B, 1e-12);
    ftd::test::check("P6: spin = +1",          v.spin == 1);
    ftd::test::check("P7: color = 2",          v.color == 2);
    ftd::test::check("P8: site index is center", r.sites.size() == 1 && r.sites[0] == idx);
}

static void section_level0_entangled_pair() {
    ftd::test::section("Level 0 / entangled_pair");

    ftd::RenderBridge rb(16);
    ftd::Coord at{8, 8, 8};
    ftd::Vec3 J{0.0, 0.0, ftd::K_B};

    auto r = ftd::ctor::entangled_pair(rb, at, J);

    ftd::test::check("EP1: name is 'entangled_pair'", std::string(r.name) == "entangled_pair");
    ftd::test::check("EP2: level is 0",               r.level == 0);
    ftd::test::check("EP3: exactly 2 sites",          r.sites.size() == 2);

    const int idx0 = rb.lattice().index(8, 8, 8);
    ftd::test::check("EP4: center voxel state = +1", rb.voxels()[idx0].state == 1);

    const int pair_id = rb.voxels()[idx0].pair_id;
    ftd::test::check("EP5: primary has non-negative pair_id", pair_id >= 0);

    auto nbrs = rb.lattice().neighbors_6(idx0);
    int partners_found = 0;
    for (int n : nbrs) {
        const auto& v = rb.voxels()[n];
        if (v.pair_id == pair_id && v.state == -1) ++partners_found;
    }
    ftd::test::check("EP6: exactly 1 face-neighbor partner with state=-1", partners_found == 1);

    std::set<int> sites_set(r.sites.begin(), r.sites.end());
    ftd::test::check("EP7: sites contains primary", sites_set.count(idx0) == 1);

    bool partner_in_sites = false;
    for (int n : nbrs) {
        const auto& v = rb.voxels()[n];
        if (v.pair_id == pair_id && v.state == -1 && sites_set.count(n)) {
            partner_in_sites = true;
            break;
        }
    }
    ftd::test::check("EP8: sites contains partner", partner_in_sites);
}

static void section_level0_wavepacket() {
    ftd::test::section("Level 0 / wavepacket");

    ftd::RenderBridge rb(32);
    ftd::Coord at{16, 16, 16};
    const double sigma = 3.0;
    const double amp   = ftd::K_B;

    auto r = ftd::ctor::wavepacket(rb, at, +1, sigma, amp);

    ftd::test::check("W1: name is 'wavepacket'", std::string(r.name) == "wavepacket");
    ftd::test::check("W2: level is 0",           r.level == 0);
    ftd::test::check("W3: more than 1 site",     r.sites.size() > 1);

    const int idx0 = rb.lattice().index(16, 16, 16);
    std::set<int> sites_set(r.sites.begin(), r.sites.end());
    ftd::test::check("W4: center is in sites", sites_set.count(idx0) == 1);
    ftd::test::check("W5: center voxel state = +1", rb.voxels()[idx0].state == 1);

    const double cutoff  = ftd::GAUSSIAN_CUTOFF_SIGMA * sigma;
    const double cutoff2 = cutoff * cutoff;
    bool all_within = true;
    for (int idx : r.sites) {
        ftd::Coord c = rb.lattice().coord(idx);
        int dx = c.x - 16, dy = c.y - 16, dz = c.z - 16;
        double r2 = static_cast<double>(dx*dx + dy*dy + dz*dz);
        if (r2 > cutoff2 + 1e-9) { all_within = false; break; }
    }
    ftd::test::check("W6: every site within Gaussian L2 cutoff", all_within);

    const int far_radius = static_cast<int>(cutoff) + 2;
    int far_idx = rb.lattice().index(16 + far_radius, 16, 16);
    const auto& far = rb.voxels()[far_idx];
    bool far_unchanged = (far.state == 0)
                      && (far.flux.x == 0 && far.flux.y == 0 && far.flux.z == 0);
    ftd::test::check("W7: voxel outside cutoff is unchanged", far_unchanged);
}

static void section_level1a_octahedron() {
    ftd::test::section("Level 1A / octahedron");

    ftd::RenderBridge rb(16);
    ftd::Coord center{8, 8, 8};

    auto r = ftd::ctor::octahedron(rb, center, +1);

    ftd::test::check("O1: name is 'octahedron'", std::string(r.name) == "octahedron");
    ftd::test::check("O2: level is 1",           r.level == 1);
    ftd::test::check("O3: exactly 6 sites",      r.sites.size() == 6);

    bool all_set = true;
    for (int idx : r.sites) {
        if (rb.voxels()[idx].state != 1) { all_set = false; break; }
    }
    ftd::test::check("O4: every site has state=+1", all_set);

    auto offs = offsets_from_sites(rb.lattice(), center, r.sites);
    bool all_distance_1 = true;
    for (const auto& [dx, dy, dz] : offs) {
        if (dx*dx + dy*dy + dz*dz != 1) { all_distance_1 = false; break; }
    }
    ftd::test::check("O5: every offset has L2 distance = 1", all_distance_1);

    std::set<std::tuple<int,int,int>> expected = {
        { 1, 0, 0}, {-1, 0, 0},
        { 0, 1, 0}, { 0,-1, 0},
        { 0, 0, 1}, { 0, 0,-1},
    };
    ftd::test::check("O6: offsets match face-neighbor pattern", offs == expected);

    ftd::test::check("O7: invariant under 90deg rotation around x", is_rotation_invariant(offs, 0));
    ftd::test::check("O8: invariant under 90deg rotation around y", is_rotation_invariant(offs, 1));
    ftd::test::check("O9: invariant under 90deg rotation around z", is_rotation_invariant(offs, 2));

    const auto& cvox = rb.voxels()[rb.lattice().index(8, 8, 8)];
    ftd::test::check("O10: center voxel unmodified", cvox.state == 0);

    ftd::test::check("O11: no leakage in 5x5x5 box", no_leakage(rb, center, r.sites));
}

static void section_level1a_cuboctahedron() {
    ftd::test::section("Level 1A / cuboctahedron");

    ftd::RenderBridge rb(16);
    ftd::Coord center{8, 8, 8};

    auto r = ftd::ctor::cuboctahedron(rb, center, +1);

    ftd::test::check("C1: name is 'cuboctahedron'", std::string(r.name) == "cuboctahedron");
    ftd::test::check("C2: level is 1",              r.level == 1);
    ftd::test::check("C3: exactly 12 sites",        r.sites.size() == 12);

    bool all_set = true;
    for (int idx : r.sites) {
        if (rb.voxels()[idx].state != 1) { all_set = false; break; }
    }
    ftd::test::check("C4: every site has state=+1", all_set);

    auto offs = offsets_from_sites(rb.lattice(), center, r.sites);
    bool all_distance_sqrt2 = true;
    for (const auto& [dx, dy, dz] : offs) {
        if (dx*dx + dy*dy + dz*dz != 2) { all_distance_sqrt2 = false; break; }
    }
    ftd::test::check("C5: every offset has L2 distance = sqrt(2)", all_distance_sqrt2);

    std::set<std::tuple<int,int,int>> expected = {
        { 1, 1, 0}, { 1,-1, 0}, {-1, 1, 0}, {-1,-1, 0},
        { 1, 0, 1}, { 1, 0,-1}, {-1, 0, 1}, {-1, 0,-1},
        { 0, 1, 1}, { 0, 1,-1}, { 0,-1, 1}, { 0,-1,-1},
    };
    ftd::test::check("C6: offsets match edge-neighbor pattern", offs == expected);

    ftd::test::check("C7: invariant under 90deg rotation around x", is_rotation_invariant(offs, 0));
    ftd::test::check("C8: invariant under 90deg rotation around y", is_rotation_invariant(offs, 1));
    ftd::test::check("C9: invariant under 90deg rotation around z", is_rotation_invariant(offs, 2));

    const auto& cvox = rb.voxels()[rb.lattice().index(8, 8, 8)];
    ftd::test::check("C10: center voxel unmodified", cvox.state == 0);

    ftd::test::check("C11: no leakage in 5x5x5 box", no_leakage(rb, center, r.sites));
}

static void section_level1a_stella_octangula() {
    ftd::test::section("Level 1A / stella_octangula");

    ftd::RenderBridge rb(16);
    ftd::Coord center{8, 8, 8};

    auto r = ftd::ctor::stella_octangula(rb, center, -1);

    ftd::test::check("S1: name is 'stella_octangula'", std::string(r.name) == "stella_octangula");
    ftd::test::check("S2: level is 1",                 r.level == 1);
    ftd::test::check("S3: exactly 8 sites",            r.sites.size() == 8);

    bool all_set = true;
    for (int idx : r.sites) {
        if (rb.voxels()[idx].state != -1) { all_set = false; break; }
    }
    ftd::test::check("S4: every site has state=-1", all_set);

    auto offs = offsets_from_sites(rb.lattice(), center, r.sites);
    bool all_distance_sqrt3 = true;
    for (const auto& [dx, dy, dz] : offs) {
        if (dx*dx + dy*dy + dz*dz != 3) { all_distance_sqrt3 = false; break; }
    }
    ftd::test::check("S5: every offset has L2 distance = sqrt(3)", all_distance_sqrt3);

    std::set<std::tuple<int,int,int>> expected = {
        { 1, 1, 1}, { 1, 1,-1}, { 1,-1, 1}, { 1,-1,-1},
        {-1, 1, 1}, {-1, 1,-1}, {-1,-1, 1}, {-1,-1,-1},
    };
    ftd::test::check("S6: offsets match corner-neighbor pattern", offs == expected);

    ftd::test::check("S7: invariant under 90deg rotation around x", is_rotation_invariant(offs, 0));
    ftd::test::check("S8: invariant under 90deg rotation around y", is_rotation_invariant(offs, 1));
    ftd::test::check("S9: invariant under 90deg rotation around z", is_rotation_invariant(offs, 2));

    const auto& cvox = rb.voxels()[rb.lattice().index(8, 8, 8)];
    ftd::test::check("S10: center voxel unmodified", cvox.state == 0);

    ftd::test::check("S11: no leakage in 5x5x5 box", no_leakage(rb, center, r.sites));
}

static void section_level1a_moore_cell() {
    ftd::test::section("Level 1A / moore_cell");

    ftd::RenderBridge rb(16);
    ftd::Coord center{8, 8, 8};

    auto r = ftd::ctor::moore_cell(rb, center, +1);

    ftd::test::check("M1: name is 'moore_cell'", std::string(r.name) == "moore_cell");
    ftd::test::check("M2: level is 1",           r.level == 1);
    ftd::test::check("M3: exactly 26 sites",     r.sites.size() == 26);

    bool all_set = true;
    for (int idx : r.sites) {
        if (rb.voxels()[idx].state != 1) { all_set = false; break; }
    }
    ftd::test::check("M4: every site has state=+1", all_set);

    std::set<std::tuple<int,int,int>> expected;
    for (int dx = -1; dx <= 1; ++dx)
    for (int dy = -1; dy <= 1; ++dy)
    for (int dz = -1; dz <= 1; ++dz)
        if (dx || dy || dz) expected.insert({dx, dy, dz});
    auto offs = offsets_from_sites(rb.lattice(), center, r.sites);
    ftd::test::check("M5: offsets match full Moore neighborhood", offs == expected);

    ftd::test::check("M6: invariant under 90deg rotation around x", is_rotation_invariant(offs, 0));
    ftd::test::check("M7: invariant under 90deg rotation around y", is_rotation_invariant(offs, 1));
    ftd::test::check("M8: invariant under 90deg rotation around z", is_rotation_invariant(offs, 2));

    const auto& cvox = rb.voxels()[rb.lattice().index(8, 8, 8)];
    ftd::test::check("M9: center voxel unmodified", cvox.state == 0);

    ftd::test::check("M10: no leakage in 5x5x5 box", no_leakage(rb, center, r.sites));

    // Union identity: moore_cell == octahedron union cuboctahedron union stella_octangula
    ftd::RenderBridge rb2(16);
    auto ro = ftd::ctor::octahedron(rb2, center, +1);
    auto rc = ftd::ctor::cuboctahedron(rb2, center, +1);
    auto rs = ftd::ctor::stella_octangula(rb2, center, +1);
    std::set<int> union_set;
    for (int i : ro.sites) union_set.insert(i);
    for (int i : rc.sites) union_set.insert(i);
    for (int i : rs.sites) union_set.insert(i);
    std::set<int> moore_set(r.sites.begin(), r.sites.end());
    ftd::test::check("M11: union identity (moore_cell = oct + cub + stella)",
                     moore_set == union_set);
}

static void section_integration_periodic() {
    ftd::test::section("Integration / periodic boundary wrap");

    const int N = 16;
    ftd::Coord origin{0, 0, 0};

    // Octahedron at origin
    {
        ftd::RenderBridge rb(N);
        auto r = ftd::ctor::octahedron(rb, origin, +1);
        ftd::test::check("IP1: octahedron at origin -> 6 sites", r.sites.size() == 6);

        std::set<int> expected = {
            rb.lattice().index( 1,  0,  0), rb.lattice().index(N-1,  0,  0),
            rb.lattice().index( 0,  1,  0), rb.lattice().index( 0, N-1,  0),
            rb.lattice().index( 0,  0,  1), rb.lattice().index( 0,  0, N-1),
        };
        std::set<int> got(r.sites.begin(), r.sites.end());
        ftd::test::check("IP2: octahedron sites wrap correctly", got == expected);
    }

    // Cuboctahedron at origin
    {
        ftd::RenderBridge rb(N);
        auto r = ftd::ctor::cuboctahedron(rb, origin, +1);
        ftd::test::check("IP3: cuboctahedron at origin -> 12 sites", r.sites.size() == 12);
        bool all_set = true;
        for (int idx : r.sites) if (rb.voxels()[idx].state != 1) { all_set = false; break; }
        ftd::test::check("IP4: cuboctahedron wrap: every site state=+1", all_set);
    }

    // Stella octangula at origin
    {
        ftd::RenderBridge rb(N);
        auto r = ftd::ctor::stella_octangula(rb, origin, +1);
        ftd::test::check("IP5: stella_octangula at origin -> 8 sites", r.sites.size() == 8);
        bool all_set = true;
        for (int idx : r.sites) if (rb.voxels()[idx].state != 1) { all_set = false; break; }
        ftd::test::check("IP6: stella wrap: every site state=+1", all_set);
    }

    // Moore cell at origin
    {
        ftd::RenderBridge rb(N);
        auto r = ftd::ctor::moore_cell(rb, origin, +1);
        ftd::test::check("IP7: moore_cell at origin -> 26 sites", r.sites.size() == 26);
        bool all_set = true;
        for (int idx : r.sites) if (rb.voxels()[idx].state != 1) { all_set = false; break; }
        ftd::test::check("IP8: moore_cell wrap: every site state=+1", all_set);
    }
}

int main() {
    ftd::test::init("test_constructors");
    section_level0_flux();
    section_level0_particle();
    section_level0_entangled_pair();
    section_level0_wavepacket();
    section_level1a_octahedron();
    section_level1a_cuboctahedron();
    section_level1a_stella_octangula();
    section_level1a_moore_cell();
    section_integration_periodic();
    return ftd::test::finalize();
}
