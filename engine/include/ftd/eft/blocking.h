#pragma once
/**
 * @file ftd/eft/blocking.h
 * @brief Real-space block-spin transformation (EFT Recovery Program, Phase 2A).
 *
 * Physics motivation
 * ------------------
 * A Wilsonian renormalization group flow is built from a *blocking
 * transformation* R that coarse-grains a lattice field theory by combining
 * neighbouring degrees of freedom into a new, coarser-grained theory on a
 * lattice with larger spacing. Iterating R produces the RG trajectory in
 * coupling space; its derivative is the β-function.
 *
 * This module implements a factor-of-2 block-spin transformation on the
 * FTD lattice: each 2×2×2 block of the fine lattice maps to one site of
 * the coarse lattice, halving the linear size from L to L/2.
 *
 * Two field-specific blocking schemes:
 *
 *   1. **Average-flux blocking** for the continuous flux field J:
 *
 *        J'(X) = (1/8) · Σ_{x ∈ 2X+[0,1]³}  J(x)
 *
 *      (arithmetic mean over the 8 child voxels). This preserves total
 *      flux in each block and is the natural choice for a vector field;
 *      it keeps charge divergence meaningful because
 *      ∫ ∇·J dV over the block depends only on boundary flux, which
 *      averages linearly.
 *
 *   2. **Majority-rule blocking** for the ternary state s ∈ {−1, 0, +1}:
 *
 *        s'(X) = sign(Σ_{x ∈ 2X+[0,1]³}  s(x))
 *
 *      with s'(X) = 0 if the sum is zero. This preserves the total
 *      charge of each block and is the standard majority rule used in
 *      real-space RG for Ising-like systems (Kadanoff 1966).
 *
 * Charge conservation under blocking (SPEC §5.1 validation gate)
 * --------------------------------------------------------------
 * Majority-rule blocking does NOT preserve total charge in general — a
 * block of (+1, 0, 0, 0, 0, 0, 0, 0) correctly maps to s'=+1, but a block
 * of (+1, +1, 0, 0, 0, 0, 0, −1) maps to sign(+1) = +1, "losing" the
 * negative charge. To preserve total charge the project commits to a
 * *charge-counting* variant: s'(X) is chosen so that the new lattice
 * contains the same total Σ s(x) as the old one, spread among 2X+k sites
 * in a stable manner.
 *
 * The pre-Phase-2B gate (SPEC §5.1) tests: a single +1 voxel, post-block,
 * must yield total charge exactly 1 on the coarse lattice. Our variant
 * `block_state_charge_conserving` achieves this by promoting the net
 * block-sum to a single voxel of that sign at the block centre.
 *
 * Epistemic status
 * ----------------
 * The *blocking schemes* are standard lattice-RG mechanics, imported from
 * Kadanoff-Wilson RG theory. No new physics. Tag: [IMPOSED framework,
 * standard RG]. The *measured* β(g) derived from iterated blocking IS the
 * physics observation; that lives in Phase 2C, not here.
 *
 * Complexity & ownership
 * ----------------------
 * All three blocking operations are O(L³): one pass over fine voxels.
 * Output is a fresh `BlockedRenderBridge` whose lattice is L/2³; the
 * caller owns the new bridge (heap-allocated). The input RenderBridge is
 * read-only (`const`); blocking never mutates the source.
 */

#include <memory>
#include <vector>

#include "ftd/lattice.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

namespace ftd {
namespace eft {

/// A coarse-grained RenderBridge produced by one application of the
/// block-spin transformation. Has the same interface as RenderBridge
/// for correlator / divergence / sampler consumption — we do NOT inherit
/// from RenderBridge because RB carries GPU state and phase-tick machinery
/// we do not need. Instead BlockedRenderBridge holds an owning RB whose
/// size is L/2 and whose voxels are seeded from blocking; every subsequent
/// operation (correlator, Gauss check, coupling measurement) uses the owned
/// RB directly via the `bridge()` accessor.
class BlockedRenderBridge {
public:
    /// Construct empty; populated by block_*() free functions.
    explicit BlockedRenderBridge(int coarse_size);

    RenderBridge& bridge() { return *rb_; }
    const RenderBridge& bridge() const { return *rb_; }

    int coarse_size() const { return rb_->lattice().size(); }

    /// Compute total net charge Σ s(x) over the coarse lattice.
    long long total_charge() const;
    /// Compute total squared flux |J|² summed over the coarse lattice.
    double total_flux_squared() const;

private:
    std::unique_ptr<RenderBridge> rb_;
};

/// Report of an integrity check on a blocking transformation.
struct BlockingIntegrity {
    long long total_charge_fine   = 0;  ///< Σ s(x) on the source (fine) lattice
    long long total_charge_coarse = 0;  ///< Σ s(x) on the coarse lattice
    double    total_flux_sq_fine   = 0.0;
    double    total_flux_sq_coarse = 0.0;
    bool      charge_conserved    = false;  ///< fine == coarse (exact, integers)
    /// Expected flux-squared ratio under (1/8)-averaging is 1/8
    /// for spatially-uncorrelated flux, 1 for spatially-uniform flux.
    /// We report the raw ratio; callers interpret against their signal.
    double    flux_sq_ratio       = 0.0;
};

/// Average-flux blocking J'(X) = mean(J over 2³ children). Seeds coarse
/// lattice voxel flux; leaves coarse state untouched (caller composes with
/// block_state_majority or block_state_charge_conserving).
///
/// Precondition: src.lattice().size() must be even.
std::unique_ptr<BlockedRenderBridge> block_flux_average(const RenderBridge& src);

/// Majority-rule state blocking s'(X) = sign(Σ s over 2³ children); 0 on
/// ties. Does NOT conserve total charge (see header discussion). Seeds
/// coarse state only; does not touch coarse flux.
void block_state_majority(const RenderBridge& src, BlockedRenderBridge& dst);

/// Charge-conserving state blocking. The net block charge Σ_{x∈block} s(x)
/// is placed as a single voxel at the block centre (with magnitude saturated
/// to {−1, 0, +1} to keep coarse state ternary). For blocks whose net
/// charge exceeds ±1, the overflow is spread to adjacent coarse voxels in a
/// deterministic order — no information loss, no charge creation.
///
/// Trade-off vs majority rule: preserves Σ s exactly, at the cost of
/// introducing long-range correlations in the coarse state field. For the
/// β-function extraction (where we measure α_eff from V(r), not from state
/// correlators), this is the correct choice because the charge is what
/// couples into the Coulomb potential.
void block_state_charge_conserving(const RenderBridge& src,
                                   BlockedRenderBridge& dst);

/// Full blocking: flux-average + charge-conserving state. Convenience
/// wrapper. Returns a new BlockedRenderBridge.
std::unique_ptr<BlockedRenderBridge> block_full(const RenderBridge& src);

/// Measure integrity invariants between fine and coarse lattices.
BlockingIntegrity check_integrity(const RenderBridge& src,
                                  const BlockedRenderBridge& dst);

}  // namespace eft
}  // namespace ftd
