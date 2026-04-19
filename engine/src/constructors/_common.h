// Internal shared helpers for the split constructors.cpp translation units.
// NOT part of the public API — do not install this header.
//
// Each constructors_*.cpp file reopens `namespace ftd::ctor` and uses these
// helpers. They are defined `inline` so multiple TUs can include this header
// without ODR violations.

#pragma once

#include "ftd/constructors.h"
#include "ftd/render_bridge.h"
#include "ftd/lattice.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

namespace ftd {
namespace ctor {
namespace detail {

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

inline Snapshot snapshot_box(const RenderBridge& rb, Coord center, int radius) {
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

inline std::vector<int> diff_sites(const RenderBridge& rb, const Snapshot& before) {
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

// Normalize a vector; returns zero vector if input is near-zero.
inline Vec3 safe_normalize(Vec3 v) {
    double m = v.mag();
    if (m < 1e-30) return {0.0, 0.0, 0.0};
    return v * (1.0 / m);
}

// Build an orthonormal basis where u1 = normalized(a).
// Returns two vectors perpendicular to a and to each other.
inline void ortho_basis(Vec3 a_norm, Vec3& e1, Vec3& e2) {
    Vec3 tmp = (std::abs(a_norm.x) < 0.9) ? Vec3{1,0,0} : Vec3{0,1,0};
    e1 = safe_normalize(Vec3::cross(a_norm, tmp));
    e2 = Vec3::cross(a_norm, e1);
}

/// Merge sites from a sub-result into a parent result (sort + dedup).
inline void merge_sites(std::vector<int>& dst, const std::vector<int>& src) {
    dst.insert(dst.end(), src.begin(), src.end());
    std::sort(dst.begin(), dst.end());
    dst.erase(std::unique(dst.begin(), dst.end()), dst.end());
}

}  // namespace detail
}  // namespace ctor
}  // namespace ftd
