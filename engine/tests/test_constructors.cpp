/**
 * test_constructors — unit tests for ftd::ctor::*
 * Spec: docs/superpowers/specs/2026-04-15-ftd-constructors-design.md
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
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

// ============================================================================
// Level 2 — field configurations
// ============================================================================

static void section_level2_plane_wave() {
    ftd::test::section("Level 2 / plane_wave");

    ftd::RenderBridge rb(16);
    ftd::Vec3 dir{1, 0, 0};
    ftd::Vec3 pol{0, 1, 0};
    double wavelength = 8.0;
    double amplitude = 0.5;

    auto r = ftd::ctor::plane_wave(rb, dir, pol, wavelength, amplitude);

    ftd::test::check("PW1: name is 'plane_wave'", std::string(r.name) == "plane_wave");
    ftd::test::check("PW2: level is 2", r.level == 2);
    ftd::test::check("PW3: sites count > 0", r.site_count() > 0);

    // Check that flux is along polarization direction (y-axis)
    bool found_nonzero_flux = false;
    for (int idx : r.sites) {
        const auto& v = rb.voxels()[idx];
        if (std::abs(v.flux.y) > 1e-10) {
            found_nonzero_flux = true;
            // Flux should have no x or z component (polarized along y)
            ftd::test::check("PW4: flux is along polarization (y)",
                std::abs(v.flux.x) < 1e-12 && std::abs(v.flux.z) < 1e-12);
            break;
        }
    }
    ftd::test::check("PW4b: found nonzero flux site", found_nonzero_flux);

    // Check that wave_vel is also along polarization
    bool found_wvel = false;
    for (int idx : r.sites) {
        const auto& v = rb.voxels()[idx];
        if (std::abs(v.wave_vel.y) > 1e-10) {
            found_wvel = true;
            break;
        }
    }
    ftd::test::check("PW5: wave_vel is nonzero (propagating)", found_wvel);

    // Symmetry: count positive and negative flux.y values
    int pos_flux = 0, neg_flux = 0;
    for (int idx : r.sites) {
        double fy = rb.voxels()[idx].flux.y;
        if (fy > 1e-10) ++pos_flux;
        if (fy < -1e-10) ++neg_flux;
    }
    ftd::test::check("PW6: has both positive and negative phases",
        pos_flux > 0 && neg_flux > 0);
}

static void section_level2_standing_wave() {
    ftd::test::section("Level 2 / standing_wave");

    ftd::RenderBridge rb(16);
    ftd::Vec3 dir{0, 0, 1};
    ftd::Vec3 pol{1, 0, 0};
    double wavelength = 8.0;
    double amplitude = 0.3;

    auto r = ftd::ctor::standing_wave(rb, dir, pol, wavelength, amplitude);

    ftd::test::check("SW1: name is 'standing_wave'", std::string(r.name) == "standing_wave");
    ftd::test::check("SW2: level is 2", r.level == 2);
    ftd::test::check("SW3: sites count > 0", r.site_count() > 0);

    // Standing wave: wave_vel should be zero everywhere
    bool all_wvel_zero = true;
    for (int idx : r.sites) {
        const auto& v = rb.voxels()[idx];
        if (v.wave_vel.mag2() > 1e-20) { all_wvel_zero = false; break; }
    }
    ftd::test::check("SW4: wave_vel is zero everywhere (standing wave)", all_wvel_zero);

    // Flux should be along polarization (x-axis)
    bool found_flux = false;
    for (int idx : r.sites) {
        const auto& v = rb.voxels()[idx];
        if (std::abs(v.flux.x) > 1e-10) {
            found_flux = true;
            ftd::test::check("SW5: flux is along polarization (x)",
                std::abs(v.flux.y) < 1e-12 && std::abs(v.flux.z) < 1e-12);
            break;
        }
    }
    ftd::test::check("SW5b: found nonzero flux", found_flux);
}

static void section_level2_uniform_e() {
    ftd::test::section("Level 2 / uniform_e");

    ftd::RenderBridge rb(16);
    ftd::Vec3 E{0.0, 0.0, 0.1};

    auto r = ftd::ctor::uniform_e(rb, E);

    ftd::test::check("UE1: name is 'uniform_e'", std::string(r.name) == "uniform_e");
    ftd::test::check("UE2: level is 2", r.level == 2);
    ftd::test::check("UE3: sites = N^3", r.site_count() == 16*16*16);

    // wave_vel = -E at every site
    int idx_check = rb.lattice().index(5, 7, 3);
    const auto& v = rb.voxels()[idx_check];
    ftd::test::check_close("UE4: wave_vel.z = -0.1", v.wave_vel.z, -0.1, 1e-12);
    ftd::test::check_close("UE5: wave_vel.x = 0", v.wave_vel.x, 0.0, 1e-12);

    // Flux should remain zero
    ftd::test::check_close("UE6: flux stays zero", v.flux.mag(), 0.0, 1e-15);
}

static void section_level2_uniform_b() {
    ftd::test::section("Level 2 / uniform_b");

    ftd::RenderBridge rb(16);
    ftd::Vec3 B{0.0, 0.0, 0.5};

    auto r = ftd::ctor::uniform_b(rb, B);

    ftd::test::check("UB1: name is 'uniform_b'", std::string(r.name) == "uniform_b");
    ftd::test::check("UB2: level is 2", r.level == 2);
    ftd::test::check("UB3: sites count > 0", r.site_count() > 0);

    // At lattice center, flux should be ~zero (r=0)
    double mid = (16 - 1) / 2.0;
    int cx = static_cast<int>(mid);
    int idx_center = rb.lattice().index(cx, cx, cx);
    const auto& vc = rb.voxels()[idx_center];
    ftd::test::check("UB4: flux near center is small", vc.flux.mag() < 0.5);

    // Off-center site should have nonzero flux in xy plane
    int idx_off = rb.lattice().index(cx + 3, cx, cx);
    const auto& vo = rb.voxels()[idx_off];
    ftd::test::check("UB5: off-center flux is nonzero", vo.flux.mag() > 1e-6);

    // Flux should be perpendicular to B (in xy plane) for Bz field
    ftd::test::check("UB6: flux.z is small (perpendicular to B)",
        std::abs(vo.flux.z) < 1e-10);
}

static void section_level2_photon_pulse() {
    ftd::test::section("Level 2 / photon_pulse");

    ftd::RenderBridge rb(32);
    ftd::Coord center{16, 16, 16};
    ftd::Vec3 dir{1, 0, 0};
    ftd::Vec3 pol{0, 0, 1};
    double sigma = 4.0;
    double amplitude = 1.0;

    auto r = ftd::ctor::photon_pulse(rb, center, dir, pol, sigma, amplitude);

    ftd::test::check("PP1: name is 'photon_pulse'", std::string(r.name) == "photon_pulse");
    ftd::test::check("PP2: level is 2", r.level == 2);
    ftd::test::check("PP3: sites count > 0", r.site_count() > 0);
    ftd::test::check("PP4: fewer sites than full lattice (localized)",
        r.site_count() < 32*32*32);

    // Check Gaussian falloff: sites far from center should not be stamped
    bool found_near = false;
    for (int idx : r.sites) {
        ftd::Coord c = rb.lattice().coord(idx);
        int dx = c.x - 16, dy = c.y - 16, dz = c.z - 16;
        double dist = std::sqrt(static_cast<double>(dx*dx + dy*dy + dz*dz));
        if (dist < 2.0) { found_near = true; break; }
    }
    ftd::test::check("PP5: has sites near center", found_near);

    // Check flux is along polarization (z)
    bool found_flux_z = false;
    for (int idx : r.sites) {
        const auto& v = rb.voxels()[idx];
        if (std::abs(v.flux.z) > 0.01) { found_flux_z = true; break; }
    }
    ftd::test::check("PP6: flux along polarization direction", found_flux_z);
}

static void section_level2_electric_dipole() {
    ftd::test::section("Level 2 / electric_dipole");

    ftd::RenderBridge rb(16);
    ftd::Coord center{8, 8, 8};
    ftd::Vec3 axis{0, 0, 1};
    int separation = 4;

    auto r = ftd::ctor::electric_dipole(rb, center, axis, separation);

    ftd::test::check("ED1: name is 'electric_dipole'", std::string(r.name) == "electric_dipole");
    ftd::test::check("ED2: level is 2", r.level == 2);
    ftd::test::check("ED3: sites count > 2", r.site_count() > 2);

    // Check that +1 and -1 charges exist
    int idx_pos = rb.lattice().index(8, 8, 10); // center + axis*2
    int idx_neg = rb.lattice().index(8, 8, 6);  // center - axis*2
    ftd::test::check("ED4: positive charge state = +1",
        rb.voxels()[idx_pos].state == +1);
    ftd::test::check("ED5: negative charge state = -1",
        rb.voxels()[idx_neg].state == -1);

    // Coulomb dressing: some intermediate site should have nonzero flux
    int idx_mid = rb.lattice().index(8, 8, 8);
    ftd::test::check("ED6: midpoint has nonzero flux from dressing",
        rb.voxels()[idx_mid].flux.mag() > 1e-10);
}

static void section_level2_magnetic_dipole() {
    ftd::test::section("Level 2 / magnetic_dipole");

    ftd::RenderBridge rb(16);
    ftd::Coord center{8, 8, 8};
    ftd::Vec3 moment{0, 0, 1};
    int radius = 3;
    double amplitude = 0.5;

    auto r = ftd::ctor::magnetic_dipole(rb, center, moment, radius, amplitude);

    ftd::test::check("MD1: name is 'magnetic_dipole'", std::string(r.name) == "magnetic_dipole");
    ftd::test::check("MD2: level is 2", r.level == 2);
    ftd::test::check("MD3: sites count > 0", r.site_count() > 0);

    // All stamped sites should have flux perpendicular to moment (in xy plane)
    bool all_perp = true;
    for (int idx : r.sites) {
        const auto& v = rb.voxels()[idx];
        if (std::abs(v.flux.z) > 1e-10) { all_perp = false; break; }
    }
    ftd::test::check("MD4: flux is in plane perpendicular to moment", all_perp);

    // Check that flux magnitude is approximately `amplitude`
    bool found_correct_amp = false;
    for (int idx : r.sites) {
        double m = rb.voxels()[idx].flux.mag();
        if (std::abs(m - amplitude) < 0.01) { found_correct_amp = true; break; }
    }
    ftd::test::check("MD5: at least one site has correct amplitude", found_correct_amp);
}

static void section_level2_vortex_line() {
    ftd::test::section("Level 2 / vortex_line");

    ftd::RenderBridge rb(16);
    ftd::Coord center{8, 8, 8};
    ftd::Vec3 axis{0, 0, 1};
    double circulation = 2.0;

    auto r = ftd::ctor::vortex_line(rb, center, axis, circulation);

    ftd::test::check("VL1: name is 'vortex_line'", std::string(r.name) == "vortex_line");
    ftd::test::check("VL2: level is 2", r.level == 2);
    ftd::test::check("VL3: sites count > 0", r.site_count() > 0);

    // Vortex extends through all z-slices: check sites in multiple z-planes
    std::set<int> z_planes;
    for (int idx : r.sites) {
        ftd::Coord c = rb.lattice().coord(idx);
        z_planes.insert(c.z);
    }
    ftd::test::check("VL4: vortex spans multiple z-planes", z_planes.size() > 1);

    // Flux should be in xy-plane (perpendicular to z-axis)
    bool all_xy = true;
    for (int idx : r.sites) {
        const auto& v = rb.voxels()[idx];
        if (std::abs(v.flux.z) > 1e-10) { all_xy = false; break; }
    }
    ftd::test::check("VL5: flux is azimuthal (in xy-plane)", all_xy);

    // 1/r falloff: site at r=1 should have stronger flux than r=5
    int idx_near = rb.lattice().index(9, 8, 8); // r=1 from center in x
    int idx_far  = rb.lattice().index(13, 8, 8); // r=5 from center in x
    double mag_near = rb.voxels()[idx_near].flux.mag();
    double mag_far  = rb.voxels()[idx_far].flux.mag();
    ftd::test::check("VL6: 1/r falloff (near > far)", mag_near > mag_far);
}

// ============================================================================
// Level 3 — elementary particles
// ============================================================================

static void section_level3_electron() {
    ftd::test::section("Level 3 / electron");

    ftd::RenderBridge rb(16);
    ftd::Coord center{8, 8, 8};

    auto r = ftd::ctor::electron(rb, center, /*spin=*/-1);

    ftd::test::check("EL1: name is 'electron'", std::string(r.name) == "electron");
    ftd::test::check("EL2: level is 3", r.level == 3);
    ftd::test::check("EL3: sites count > 1 (flux envelope)", r.site_count() > 1);

    const int idx = rb.lattice().index(8, 8, 8);
    const auto& v = rb.voxels()[idx];
    ftd::test::check("EL4: center state = -1", v.state == -1);
    ftd::test::check("EL5: center spin = -1", v.spin == -1);
    ftd::test::check("EL6: center color = 0 (colorless)", v.color == 0);

    // Flux should point inward: at (9,8,8) flux.x should be negative
    int idx_right = rb.lattice().index(9, 8, 8);
    ftd::test::check("EL7: flux at +x points inward (negative x)",
        rb.voxels()[idx_right].flux.x < 0);
}

static void section_level3_positron() {
    ftd::test::section("Level 3 / positron");

    ftd::RenderBridge rb(16);
    ftd::Coord center{8, 8, 8};

    auto r = ftd::ctor::positron(rb, center, /*spin=*/+1);

    ftd::test::check("POS1: name is 'positron'", std::string(r.name) == "positron");
    ftd::test::check("POS2: level is 3", r.level == 3);
    ftd::test::check("POS3: sites count > 1", r.site_count() > 1);

    const int idx = rb.lattice().index(8, 8, 8);
    const auto& v = rb.voxels()[idx];
    ftd::test::check("POS4: center state = +1", v.state == +1);
    ftd::test::check("POS5: center spin = +1", v.spin == +1);
    ftd::test::check("POS6: center color = 0", v.color == 0);

    // Flux should point outward: at (9,8,8) flux.x should be positive
    int idx_right = rb.lattice().index(9, 8, 8);
    ftd::test::check("POS7: flux at +x points outward (positive x)",
        rb.voxels()[idx_right].flux.x > 0);
}

static void section_level3_neutrino() {
    ftd::test::section("Level 3 / neutrino");

    ftd::RenderBridge rb(16);
    ftd::Coord center{8, 8, 8};

    auto r = ftd::ctor::neutrino(rb, center, /*chirality=*/-1);

    ftd::test::check("NU1: name is 'neutrino'", std::string(r.name) == "neutrino");
    ftd::test::check("NU2: level is 3", r.level == 3);
    ftd::test::check("NU3: sites count > 0", r.site_count() > 0);

    const int idx = rb.lattice().index(8, 8, 8);
    const auto& v = rb.voxels()[idx];
    ftd::test::check("NU4: center state = 0 (neutral)", v.state == 0);
    ftd::test::check("NU5: spin = chirality = -1", v.spin == -1);

    // Left-handed: flux_L should be >= flux_R at center
    ftd::test::check("NU6: flux_L >= flux_R (left-handed chirality)",
        v.flux_L.x >= v.flux_R.x);
}

static void section_level3_quark() {
    ftd::test::section("Level 3 / quark");

    ftd::RenderBridge rb(16);
    ftd::Coord center{8, 8, 8};

    auto r = ftd::ctor::quark(rb, center, /*charge=*/+1, /*color=*/1, /*spin=*/+1);

    ftd::test::check("QK1: name is 'quark'", std::string(r.name) == "quark");
    ftd::test::check("QK2: level is 3", r.level == 3);
    ftd::test::check("QK3: sites count > 1", r.site_count() > 1);

    const int idx = rb.lattice().index(8, 8, 8);
    const auto& v = rb.voxels()[idx];
    ftd::test::check("QK4: center state = +1 (up-type)", v.state == +1);
    ftd::test::check("QK5: center spin = +1", v.spin == +1);
    ftd::test::check("QK6: center color = 1 (red)", v.color == 1);
}

static void section_level3_antiquark() {
    ftd::test::section("Level 3 / antiquark");

    ftd::RenderBridge rb(16);
    ftd::Coord center{8, 8, 8};

    auto r = ftd::ctor::antiquark(rb, center, /*charge=*/+1, /*color=*/1, /*spin=*/-1);

    ftd::test::check("AQ1: name is 'antiquark'", std::string(r.name) == "antiquark");
    ftd::test::check("AQ2: level is 3", r.level == 3);
    ftd::test::check("AQ3: sites count > 1", r.site_count() > 1);

    const int idx = rb.lattice().index(8, 8, 8);
    const auto& v = rb.voxels()[idx];
    ftd::test::check("AQ4: center state = -1 (anti up-type)", v.state == -1);
    ftd::test::check("AQ5: center spin = -1", v.spin == -1);
    ftd::test::check("AQ6: center color = 1", v.color == 1);
}

// ============================================================================
// Level 4 — composite particles
// ============================================================================

static void section_level4_pion() {
    ftd::test::section("Level 4 / pion");

    ftd::RenderBridge rb(16);
    ftd::Coord center{8, 8, 8};

    auto r = ftd::ctor::pion(rb, center, /*separation=*/3);

    ftd::test::check("PI1: name is 'pion'", std::string(r.name) == "pion");
    ftd::test::check("PI2: level is 4", r.level == 4);
    ftd::test::check("PI3: sites count > 2 (two quarks + envelopes)", r.site_count() > 2);

    // Quark at center + (1, 0, 0) (half of sep=3 truncated)
    int idx_q = rb.lattice().index(9, 8, 8);
    ftd::test::check("PI4: quark position has state = +1",
        rb.voxels()[idx_q].state == +1);

    // Antiquark at center - (1, 0, 0): state = -charge = -1
    int idx_aq = rb.lattice().index(7, 8, 8);
    ftd::test::check("PI5: antiquark position has state = -1",
        rb.voxels()[idx_aq].state == -1);
}

static void section_level4_proton() {
    ftd::test::section("Level 4 / proton");

    ftd::RenderBridge rb(16);
    ftd::Coord center{8, 8, 8};

    auto r = ftd::ctor::proton(rb, center, /*radius=*/2);

    ftd::test::check("PR1: name is 'proton'", std::string(r.name) == "proton");
    ftd::test::check("PR2: level is 4", r.level == 4);
    ftd::test::check("PR3: sites count > 3 (three quarks + envelopes)", r.site_count() > 3);

    // First quark at angle=0: center + (2, 0, 0) = (10, 8, 8)
    int idx_u1 = rb.lattice().index(10, 8, 8);
    ftd::test::check("PR4: u-quark at 0deg has state = +1",
        rb.voxels()[idx_u1].state == +1);
    ftd::test::check("PR5: u-quark at 0deg has color = 1 (red)",
        rb.voxels()[idx_u1].color == 1);

    // Check that all three color charges are present among the quarks
    std::set<int8_t> colors;
    for (int idx : r.sites) {
        int8_t c = rb.voxels()[idx].color;
        if (c > 0) colors.insert(c);
    }
    ftd::test::check("PR6: all three colors present (color singlet)",
        colors.count(1) && colors.count(2) && colors.count(3));
}

static void section_level4_neutron() {
    ftd::test::section("Level 4 / neutron");

    ftd::RenderBridge rb(16);
    ftd::Coord center{8, 8, 8};

    auto r = ftd::ctor::neutron(rb, center, /*radius=*/2);

    ftd::test::check("NE1: name is 'neutron'", std::string(r.name) == "neutron");
    ftd::test::check("NE2: level is 4", r.level == 4);
    ftd::test::check("NE3: sites count > 3", r.site_count() > 3);

    // First quark (u) at angle=0: center + (2, 0, 0) = (10, 8, 8)
    int idx_u = rb.lattice().index(10, 8, 8);
    ftd::test::check("NE4: u-quark at 0deg has state = +1",
        rb.voxels()[idx_u].state == +1);

    // Count manifested states: neutron has 1 up (+1) and 2 down (-1)
    // Net charge should be +1 + (-1) + (-1) = -1
    // But at quark positions specifically, check the content
    // Second quark (d) at 120deg
    double angle2 = 2.0 * ftd::PI / 3.0;
    int dx2 = 8 + static_cast<int>(std::round(2.0 * std::cos(angle2)));
    int dy2 = 8 + static_cast<int>(std::round(2.0 * std::sin(angle2)));
    int idx_d = rb.lattice().index(dx2, dy2, 8);
    ftd::test::check("NE5: d-quark at 120deg has state = -1",
        rb.voxels()[idx_d].state == -1);

    // All three colors should be present
    std::set<int8_t> colors;
    for (int idx : r.sites) {
        int8_t c = rb.voxels()[idx].color;
        if (c > 0) colors.insert(c);
    }
    ftd::test::check("NE6: all three colors present",
        colors.count(1) && colors.count(2) && colors.count(3));
}

// ============================================================================
// Level 5 — atoms & molecules
// ============================================================================

static void section_level5_hydrogen() {
    ftd::test::section("Level 5 / hydrogen");

    ftd::RenderBridge rb(32);
    ftd::Coord center{16, 16, 16};

    auto r = ftd::ctor::hydrogen(rb, center, /*orbital_radius=*/5);

    ftd::test::check("HY1: name is 'hydrogen'", std::string(r.name) == "hydrogen");
    ftd::test::check("HY2: level is 5", r.level == 5);
    ftd::test::check("HY3: sites count > 5 (proton + electron + envelopes)", r.site_count() > 5);

    // Proton: u-quark at center + (2, 0, 0) = (18, 16, 16) has state=+1
    int idx_u = rb.lattice().index(18, 16, 16);
    ftd::test::check("HY4: proton quark present (state=+1 at expected pos)",
        rb.voxels()[idx_u].state == +1);

    // Electron at (16, 16, 21)
    int idx_e = rb.lattice().index(16, 16, 21);
    ftd::test::check("HY5: electron present (state=-1)",
        rb.voxels()[idx_e].state == -1);
    ftd::test::check("HY6: electron spin = -1",
        rb.voxels()[idx_e].spin == -1);
}

static void section_level5_helium() {
    ftd::test::section("Level 5 / helium");

    ftd::RenderBridge rb(32);
    ftd::Coord center{16, 16, 16};

    auto r = ftd::ctor::helium(rb, center, /*orbital_radius=*/4);

    ftd::test::check("HE1: name is 'helium'", std::string(r.name) == "helium");
    ftd::test::check("HE2: level is 5", r.level == 5);
    ftd::test::check("HE3: sites count > 3 (nucleus + 2 electrons + envelopes)", r.site_count() > 3);

    // Nucleus at center: state=+1
    int idx_nuc = rb.lattice().index(16, 16, 16);
    ftd::test::check("HE4: nucleus has state = +1",
        rb.voxels()[idx_nuc].state == +1);

    // Electron 1 at (16, 16, 20)
    int idx_e1 = rb.lattice().index(16, 16, 20);
    ftd::test::check("HE5: electron 1 present (state = -1)",
        rb.voxels()[idx_e1].state == -1);

    // Electron 2 at (16, 16, 12)
    int idx_e2 = rb.lattice().index(16, 16, 12);
    ftd::test::check("HE6: electron 2 present (state = -1)",
        rb.voxels()[idx_e2].state == -1);

    // Electrons should have opposite spins
    ftd::test::check("HE7: electrons have opposite spins",
        rb.voxels()[idx_e1].spin != rb.voxels()[idx_e2].spin);
}

static void section_level5_h2_molecule() {
    ftd::test::section("Level 5 / h2_molecule");

    ftd::RenderBridge rb(32);
    ftd::Coord center{16, 16, 16};

    auto r = ftd::ctor::h2_molecule(rb, center, /*bond_length=*/4, /*orbital_radius=*/5);

    ftd::test::check("H21: name is 'h2_molecule'", std::string(r.name) == "h2_molecule");
    ftd::test::check("H22: level is 5", r.level == 5);
    ftd::test::check("H23: sites count > 10 (two hydrogens)", r.site_count() > 10);

    // Hydrogen 1 centered at (14, 16, 16): electron at (14, 16, 21)
    int idx_e1 = rb.lattice().index(14, 16, 21);
    ftd::test::check("H24: H1 electron present (state = -1)",
        rb.voxels()[idx_e1].state == -1);

    // Hydrogen 2 centered at (18, 16, 16): proton u-quark at (20, 16, 16)
    int idx_u2 = rb.lattice().index(20, 16, 16);
    ftd::test::check("H25: H2 proton quark present (state = +1)",
        rb.voxels()[idx_u2].state == +1);

    // Both hydrogen atoms contribute electrons with state = -1
    int count_electrons = 0;
    for (int idx : r.sites) {
        if (rb.voxels()[idx].state == -1 && rb.voxels()[idx].color == 0) {
            ++count_electrons;
        }
    }
    ftd::test::check("H26: multiple electron sites present", count_electrons >= 2);
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
    section_level2_plane_wave();
    section_level2_standing_wave();
    section_level2_uniform_e();
    section_level2_uniform_b();
    section_level2_photon_pulse();
    section_level2_electric_dipole();
    section_level2_magnetic_dipole();
    section_level2_vortex_line();
    section_level3_electron();
    section_level3_positron();
    section_level3_neutrino();
    section_level3_quark();
    section_level3_antiquark();
    section_level4_pion();
    section_level4_proton();
    section_level4_neutron();
    section_level5_hydrogen();
    section_level5_helium();
    section_level5_h2_molecule();
    return ftd::test::finalize();
}
