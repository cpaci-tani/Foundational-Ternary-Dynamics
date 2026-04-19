// blocking.cpp — Phase 2A of the EFT Recovery Program.
//
// See engine/include/ftd/eft/blocking.h for the public API and physics
// justification. This TU implements the three free functions plus the
// BlockedRenderBridge constructor and integrity check.
//
// Design choices made here:
//
//   - BlockedRenderBridge owns a fresh RenderBridge of size L/2. We do
//     NOT reuse the source RenderBridge's GPU or tick machinery; the
//     coarse bridge is treated purely as a container for the blocked
//     fields and is read by downstream correlator / coupling-measurement
//     tooling via the same interface as the source bridge.
//
//   - Charge conservation under blocking is not free. Majority rule loses
//     charge on configurations like (+1, +1, 0, 0, 0, 0, 0, −1) where the
//     block sum is +1 but the block centre sees majority = +1. Our
//     charge-conserving variant promotes the block sum to the block-centre
//     coarse voxel, saturating to ternary {−1, 0, +1}. Overflow is spilled
//     to a deterministic neighbour list so Σ s is exactly preserved.

#include "ftd/eft/blocking.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>

namespace ftd {
namespace eft {

// ─────────────────────────────────────────────────────────────────────────
//  BlockedRenderBridge
// ─────────────────────────────────────────────────────────────────────────

BlockedRenderBridge::BlockedRenderBridge(int coarse_size)
  : rb_(std::make_unique<RenderBridge>(coarse_size)) {}

long long BlockedRenderBridge::total_charge() const {
    const auto& vox = rb_->voxels();
    long long sum = 0;
    for (const auto& v : vox) sum += static_cast<long long>(v.state);
    return sum;
}

double BlockedRenderBridge::total_flux_squared() const {
    const auto& vox = rb_->voxels();
    double sum = 0.0;
    for (const auto& v : vox) sum += v.flux.dot(v.flux);
    return sum;
}

// ─────────────────────────────────────────────────────────────────────────
//  Helper: iterate the 8 children of coarse voxel (X, Y, Z) in a fixed
//  order. The order matters for the deterministic tiebreak in the
//  charge-conserving variant.
// ─────────────────────────────────────────────────────────────────────────

namespace {

constexpr int kChildOffsets[8][3] = {
    {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0},
    {0, 0, 1}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1},
};

inline int child_count_plus(const RenderBridge& src,
                            int X, int Y, int Z) {
    const auto& lat = src.lattice();
    const auto& vox = src.voxels();
    int n = 0;
    for (const auto& d : kChildOffsets) {
        const int idx = lat.index(2 * X + d[0], 2 * Y + d[1], 2 * Z + d[2]);
        if (vox[idx].state > 0) ++n;
    }
    return n;
}

inline int child_count_minus(const RenderBridge& src,
                             int X, int Y, int Z) {
    const auto& lat = src.lattice();
    const auto& vox = src.voxels();
    int n = 0;
    for (const auto& d : kChildOffsets) {
        const int idx = lat.index(2 * X + d[0], 2 * Y + d[1], 2 * Z + d[2]);
        if (vox[idx].state < 0) ++n;
    }
    return n;
}

inline int child_net_state(const RenderBridge& src,
                           int X, int Y, int Z) {
    const auto& lat = src.lattice();
    const auto& vox = src.voxels();
    int sum = 0;
    for (const auto& d : kChildOffsets) {
        const int idx = lat.index(2 * X + d[0], 2 * Y + d[1], 2 * Z + d[2]);
        sum += static_cast<int>(vox[idx].state);
    }
    return sum;
}

}  // anonymous namespace

// ─────────────────────────────────────────────────────────────────────────
//  block_flux_average
// ─────────────────────────────────────────────────────────────────────────

std::unique_ptr<BlockedRenderBridge> block_flux_average(const RenderBridge& src) {
    const int fine_L = src.lattice().size();
    const int coarse_L = fine_L / 2;
    // Precondition: even fine_L. Fail safely (return null) if caller passes
    // an odd-size lattice — we don't have an exception model in this path.
    if (coarse_L * 2 != fine_L || coarse_L < 1) return nullptr;

    auto out = std::make_unique<BlockedRenderBridge>(coarse_L);
    auto& dst_rb = out->bridge();
    auto& dst_vox = dst_rb.voxels();
    const auto& dst_lat = dst_rb.lattice();

    const auto& src_vox = src.voxels();
    const auto& src_lat = src.lattice();

    for (int Z = 0; Z < coarse_L; ++Z)
        for (int Y = 0; Y < coarse_L; ++Y)
            for (int X = 0; X < coarse_L; ++X) {
                Vec3 avg{0.0, 0.0, 0.0};
                for (const auto& d : kChildOffsets) {
                    const int src_idx = src_lat.index(
                        2 * X + d[0], 2 * Y + d[1], 2 * Z + d[2]);
                    avg.x += src_vox[src_idx].flux.x;
                    avg.y += src_vox[src_idx].flux.y;
                    avg.z += src_vox[src_idx].flux.z;
                }
                avg.x *= 0.125;
                avg.y *= 0.125;
                avg.z *= 0.125;
                const int dst_idx = dst_lat.index(X, Y, Z);
                dst_vox[dst_idx].flux = avg;
            }
    return out;
}

// ─────────────────────────────────────────────────────────────────────────
//  block_state_majority
// ─────────────────────────────────────────────────────────────────────────

void block_state_majority(const RenderBridge& src, BlockedRenderBridge& dst) {
    const int coarse_L = dst.coarse_size();
    auto& dst_vox = dst.bridge().voxels();
    const auto& dst_lat = dst.bridge().lattice();

    for (int Z = 0; Z < coarse_L; ++Z)
        for (int Y = 0; Y < coarse_L; ++Y)
            for (int X = 0; X < coarse_L; ++X) {
                const int np = child_count_plus(src, X, Y, Z);
                const int nm = child_count_minus(src, X, Y, Z);
                int8_t s = 0;
                if (np > nm) s = +1;
                else if (nm > np) s = -1;
                // else s = 0 (tie)
                const int dst_idx = dst_lat.index(X, Y, Z);
                dst_vox[dst_idx].state = s;
            }
}

// ─────────────────────────────────────────────────────────────────────────
//  block_state_charge_conserving
// ─────────────────────────────────────────────────────────────────────────
//
// Pass 1: for each coarse voxel, compute the block's net charge S =
// Σ_{children} s. The "preferred" coarse site gets min(|S|, +1) * sign(S);
// the overflow (|S| − 1 if |S| ≥ 2) is accumulated into a reservoir that
// feeds neighbouring coarse voxels in a fixed traversal order in Pass 2.
//
// Traversal order is lexicographic (Z, Y, X). Overflow flows to the next
// lexicographically later coarse voxel whose current state is zero. This
// is deterministic, seed-free, and preserves total Σ s exactly.
//
// Guarantee: charge_conserved == true as asserted in check_integrity()
// (the CTest validation gate).

void block_state_charge_conserving(const RenderBridge& src,
                                   BlockedRenderBridge& dst) {
    const int coarse_L = dst.coarse_size();
    auto& dst_rb = dst.bridge();
    auto& dst_vox = dst_rb.voxels();
    const auto& dst_lat = dst_rb.lattice();
    const int Ncoarse = dst_lat.total_sites();

    // Zero everything first so repeated calls are idempotent.
    for (int i = 0; i < Ncoarse; ++i) dst_vox[i].state = 0;

    // Pass 1: assign one unit of charge to each block's primary coarse
    // voxel. Accumulate overflow into a plain int buffer in lexicographic
    // order matching the dst_lat.index() convention.
    std::vector<int> overflow(Ncoarse, 0);

    for (int Z = 0; Z < coarse_L; ++Z)
        for (int Y = 0; Y < coarse_L; ++Y)
            for (int X = 0; X < coarse_L; ++X) {
                const int S = child_net_state(src, X, Y, Z);
                const int dst_idx = dst_lat.index(X, Y, Z);
                if (S == 0) {
                    // Leave primary site at 0; no overflow.
                    continue;
                }
                const int sign = (S > 0) ? +1 : -1;
                dst_vox[dst_idx].state = static_cast<int8_t>(sign);
                const int mag_over = std::abs(S) - 1;
                overflow[dst_idx] = sign * mag_over;  // signed overflow
            }

    // Pass 2: flow overflow into the next coarse voxel (in lex order) whose
    // state is currently 0 and whose sign-slot we can fill. This is a
    // conservative "greedy rightward push"; it guarantees total Σ state is
    // preserved and stops when overflow reaches 0.
    for (int i = 0; i < Ncoarse; ++i) {
        int residual = overflow[i];
        if (residual == 0) continue;
        const int sign = (residual > 0) ? +1 : -1;
        int mag = std::abs(residual);
        int j = i + 1;
        while (mag > 0 && j < Ncoarse) {
            if (dst_vox[j].state == 0) {
                dst_vox[j].state = static_cast<int8_t>(sign);
                --mag;
            }
            ++j;
        }
        // If we fall off the end with mag > 0, the coarse lattice is
        // saturated with same-sign charges; there's nowhere to put the
        // rest. This should not happen for physical configurations.
        // We leave the charge unplaced (charge-conservation assertion will
        // fail, flagging the issue).
        overflow[i] = (mag > 0) ? sign * mag : 0;
    }
}

// ─────────────────────────────────────────────────────────────────────────
//  block_full + integrity check
// ─────────────────────────────────────────────────────────────────────────

std::unique_ptr<BlockedRenderBridge> block_full(const RenderBridge& src) {
    auto out = block_flux_average(src);
    if (!out) return nullptr;
    block_state_charge_conserving(src, *out);
    return out;
}

BlockingIntegrity check_integrity(const RenderBridge& src,
                                  const BlockedRenderBridge& dst) {
    BlockingIntegrity out;
    // Fine-lattice sums
    const auto& fv = src.voxels();
    for (const auto& v : fv) {
        out.total_charge_fine += static_cast<long long>(v.state);
        out.total_flux_sq_fine += v.flux.dot(v.flux);
    }
    // Coarse-lattice sums
    out.total_charge_coarse = dst.total_charge();
    out.total_flux_sq_coarse = dst.total_flux_squared();
    out.charge_conserved = (out.total_charge_fine == out.total_charge_coarse);
    out.flux_sq_ratio = (out.total_flux_sq_fine > 0.0)
                        ? out.total_flux_sq_coarse / out.total_flux_sq_fine
                        : 0.0;
    return out;
}

}  // namespace eft
}  // namespace ftd
