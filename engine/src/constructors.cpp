#include "ftd/constructors.h"
#include "ftd/render_bridge.h"
#include "ftd/lattice.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

namespace ftd {
namespace ctor {

namespace {

using SnapshotEntry = std::pair<int, Voxel>;
using Snapshot      = std::vector<SnapshotEntry>;

inline bool voxel_changed(const Voxel& a, const Voxel& b) {
    return a.state != b.state
        || a.flux.x  != b.flux.x  || a.flux.y  != b.flux.y  || a.flux.z  != b.flux.z
        || a.flux_L.x != b.flux_L.x || a.flux_L.y != b.flux_L.y || a.flux_L.z != b.flux_L.z
        || a.flux_R.x != b.flux_R.x || a.flux_R.y != b.flux_R.y || a.flux_R.z != b.flux_R.z
        || a.spin  != b.spin
        || a.color != b.color;
}

Snapshot snapshot_box(const RenderBridge& rb, Coord center, int radius) {
    Snapshot out;
    const Lattice& lat = rb.lattice();
    const auto& vox = rb.voxels();
    const int side = 2 * radius + 1;
    out.reserve(static_cast<size_t>(side) * side * side);
    for (int dx = -radius; dx <= radius; ++dx)
        for (int dy = -radius; dy <= radius; ++dy)
            for (int dz = -radius; dz <= radius; ++dz) {
                int idx = lat.index(center.x + dx, center.y + dy, center.z + dz);
                out.push_back({idx, vox[idx]});
            }
    return out;
}

std::vector<int> diff_sites(const RenderBridge& rb, const Snapshot& before) {
    const auto& vox = rb.voxels();
    std::vector<int> changed;
    changed.reserve(before.size());
    for (const auto& entry : before) {
        if (voxel_changed(vox[entry.first], entry.second)) changed.push_back(entry.first);
    }
    std::sort(changed.begin(), changed.end());
    changed.erase(std::unique(changed.begin(), changed.end()), changed.end());
    return changed;
}

}  // anonymous namespace

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
    (void)rb; (void)state;
    return StampResult{"cuboctahedron", 1, center, {}};
}

StampResult stella_octangula(RenderBridge& rb, Coord center, int8_t state) {
    (void)rb; (void)state;
    return StampResult{"stella_octangula", 1, center, {}};
}

StampResult moore_cell(RenderBridge& rb, Coord center, int8_t state) {
    (void)rb; (void)state;
    return StampResult{"moore_cell", 1, center, {}};
}

}  // namespace ctor
}  // namespace ftd
