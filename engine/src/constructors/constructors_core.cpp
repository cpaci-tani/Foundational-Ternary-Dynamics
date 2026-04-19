// constructors_core.cpp
// Level 0 (flux/particle/wavepacket/entangled_pair) and Level 1A
// (octahedron/cuboctahedron/stella_octangula/moore_cell) constructors.
//
// Also hosts the snapshot/diff machinery used by Level 0 to compute
// the list of voxels that actually changed after a bridge-side mutation.
// Those helpers live in "./_common.h" so other constructors_*.cpp files
// can reuse them.

#include "ftd/constructors.h"
#include "./_common.h"

#include <algorithm>
#include <array>

namespace ftd {
namespace ctor {

using detail::snapshot_box;
using detail::diff_sites;

// Level 0 implementations
StampResult flux(RenderBridge& rb, Coord at, Vec3 J) {
    auto before = snapshot_box(rb, at, 0);
    rb.inject_flux(at.x, at.y, at.z, J);
    return StampResult{"flux", 0, at, diff_sites(rb, before)};
}

StampResult particle(RenderBridge& rb, Coord at, int8_t state, Vec3 J,
                     int8_t spin, int8_t color) {
    auto before = snapshot_box(rb, at, 0);
    rb.inject_particle(at.x, at.y, at.z, state, J, spin, color);
    return StampResult{"particle", 0, at, diff_sites(rb, before)};
}

StampResult wavepacket(RenderBridge& rb, Coord at, int8_t state, double sigma, double amp) {
    const int radius = static_cast<int>(GAUSSIAN_CUTOFF_SIGMA * sigma) + 1;
    auto before = snapshot_box(rb, at, radius);
    rb.inject_wavepacket(at.x, at.y, at.z, state, sigma, amp);
    return StampResult{"wavepacket", 0, at, diff_sites(rb, before)};
}

StampResult entangled_pair(RenderBridge& rb, Coord at, Vec3 J) {
    auto before = snapshot_box(rb, at, 1);
    rb.create_entangled_pair(at.x, at.y, at.z, J);
    return StampResult{"entangled_pair", 0, at, diff_sites(rb, before)};
}

// Level 1A implementations
StampResult octahedron(RenderBridge& rb, Coord center, int8_t state) {
    static constexpr std::array<std::array<int,3>, 6> OFFSETS = {{
        { 1, 0, 0}, {-1, 0, 0},
        { 0, 1, 0}, { 0,-1, 0},
        { 0, 0, 1}, { 0, 0,-1},
    }};
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();
    StampResult r{"octahedron", 1, center, {}};
    r.sites.reserve(6);
    for (const auto& o : OFFSETS) {
        int idx = lat.index(center.x + o[0], center.y + o[1], center.z + o[2]);
        vox[idx].state = state;
        r.sites.push_back(idx);
    }
    return r;
}

StampResult cuboctahedron(RenderBridge& rb, Coord center, int8_t state) {
    static constexpr std::array<std::array<int,3>, 12> OFFSETS = {{
        { 1, 1, 0}, { 1,-1, 0}, {-1, 1, 0}, {-1,-1, 0},
        { 1, 0, 1}, { 1, 0,-1}, {-1, 0, 1}, {-1, 0,-1},
        { 0, 1, 1}, { 0, 1,-1}, { 0,-1, 1}, { 0,-1,-1},
    }};
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();
    StampResult r{"cuboctahedron", 1, center, {}};
    r.sites.reserve(12);
    for (const auto& o : OFFSETS) {
        int idx = lat.index(center.x + o[0], center.y + o[1], center.z + o[2]);
        vox[idx].state = state;
        r.sites.push_back(idx);
    }
    return r;
}

StampResult stella_octangula(RenderBridge& rb, Coord center, int8_t state) {
    static constexpr std::array<std::array<int,3>, 8> OFFSETS = {{
        { 1, 1, 1}, { 1, 1,-1}, { 1,-1, 1}, { 1,-1,-1},
        {-1, 1, 1}, {-1, 1,-1}, {-1,-1, 1}, {-1,-1,-1},
    }};
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();
    StampResult r{"stella_octangula", 1, center, {}};
    r.sites.reserve(8);
    for (const auto& o : OFFSETS) {
        int idx = lat.index(center.x + o[0], center.y + o[1], center.z + o[2]);
        vox[idx].state = state;
        r.sites.push_back(idx);
    }
    return r;
}

StampResult moore_cell(RenderBridge& rb, Coord center, int8_t state) {
    auto r_oct  = octahedron(rb, center, state);
    auto r_cub  = cuboctahedron(rb, center, state);
    auto r_stel = stella_octangula(rb, center, state);

    StampResult r{"moore_cell", 1, center, {}};
    r.sites.reserve(r_oct.sites.size() + r_cub.sites.size() + r_stel.sites.size());
    r.sites.insert(r.sites.end(), r_oct.sites.begin(),  r_oct.sites.end());
    r.sites.insert(r.sites.end(), r_cub.sites.begin(),  r_cub.sites.end());
    r.sites.insert(r.sites.end(), r_stel.sites.begin(), r_stel.sites.end());
    std::sort(r.sites.begin(), r.sites.end());
    r.sites.erase(std::unique(r.sites.begin(), r.sites.end()), r.sites.end());
    return r;
}

}  // namespace ctor
}  // namespace ftd
