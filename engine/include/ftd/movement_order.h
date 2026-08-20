#pragma once
// Coordinate-independent movement helpers shared by CPU phase_movement and
// the CUDA serial commit kernel. Used only when
// TermToggles::symmetric_movement_order is on (default OFF).
//
// Site-order shuffle used to be std::mt19937 + std::shuffle, which is not
// bit-stable across standard libraries and has no device twin. Both
// backends now Fisher-Yates with VoxelRng::MovementShuffle so CPU/GPU
// contended-site order matches.

#include "ftd/voxel_rng.h"

namespace ftd {

FTD_RNG_HD void movement_axis_perm(std::uint64_t seed, int site, int tick,
                                   int axes[3]) {
    double r = voxel_uniform(seed, site, tick,
                             static_cast<std::uint64_t>(VoxelRng::MovementOrder));
    int perm = static_cast<int>(r * 6.0);
    if (perm < 0) perm = 0;
    else if (perm > 5) perm = 5;
    switch (perm) {
        case 0: axes[0] = 0; axes[1] = 1; axes[2] = 2; break;
        case 1: axes[0] = 0; axes[1] = 2; axes[2] = 1; break;
        case 2: axes[0] = 1; axes[1] = 0; axes[2] = 2; break;
        case 3: axes[0] = 1; axes[1] = 2; axes[2] = 0; break;
        case 4: axes[0] = 2; axes[1] = 0; axes[2] = 1; break;
        default: axes[0] = 2; axes[1] = 1; axes[2] = 0; break;
    }
}

FTD_RNG_HD void extract_remainder_hops(
    double& rx, double& ry, double& rz,
    int& dx, int& dy, int& dz,
    bool symmetric, std::uint64_t seed, int site, int tick) {
    dx = 0;
    dy = 0;
    dz = 0;
    auto hop_axis = [](double& rem, int& d) {
        if (rem >= 1.0) { d = 1; rem -= 1.0; }
        else if (rem <= -1.0) { d = -1; rem += 1.0; }
    };
    if (!symmetric) {
        hop_axis(rx, dx);
        hop_axis(ry, dy);
        hop_axis(rz, dz);
        return;
    }
    int axes[3];
    movement_axis_perm(seed, site, tick, axes);
    for (int k = 0; k < 3; ++k) {
        if (axes[k] == 0) hop_axis(rx, dx);
        else if (axes[k] == 1) hop_axis(ry, dy);
        else hop_axis(rz, dz);
    }
}

// Uniform integer in [0, i] for Fisher-Yates at position i.
FTD_RNG_HD int movement_shuffle_j(std::uint64_t seed, int i, int tick) {
    double r = voxel_uniform(seed, i, tick,
                             static_cast<std::uint64_t>(VoxelRng::MovementShuffle));
    int j = static_cast<int>(r * static_cast<double>(i + 1));
    if (j < 0) j = 0;
    if (j > i) j = i;
    return j;
}

}  // namespace ftd
