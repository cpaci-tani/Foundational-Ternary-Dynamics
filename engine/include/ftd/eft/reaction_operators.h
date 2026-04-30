#pragma once
/**
 * @file ftd/eft/reaction_operators.h
 * @brief Reaction-sector operators (O7-O10) for the S_eff campaign (FTD-0112).
 *
 * Pre-registration: docs/theory/10_eft_program/PROTOCOL_S_EFF_NONLINEAR_CAMPAIGN.md
 *                   §2.2 (LOCKED operator basis extension)
 * Tag: preregister-s-eff-nonlinear-v1
 *
 * Extends the FTD-0098 6-operator flux/state basis with 4 reaction-sector
 * operators that depend on the per-tick state increment δs(x) := s(x, t+1) -
 * s(x, t). These cannot be evaluated on a single DualCellFields snapshot;
 * they require a SnapshotPair holding (before, after) at adjacent ticks.
 *
 * Operator definitions (PROTOCOL §2.2):
 *
 *   O7 reactionDensity := (δs)^2
 *      Total reaction-rate density per cell. Non-zero iff state changes
 *      occurred between snapshots.
 *
 *   O8 genesisFlux := (δs) * θ(s_before == 0) * sign(δs) * |J_before|
 *      From-vacuum sourcing: positive for s_before = 0 → s_after ≠ 0
 *      transitions, weighted by the pre-transition flux magnitude.
 *
 *   O9 evapFlux := (δs) * θ(s_before ≠ 0 ∧ s_after == 0) * |J_before|
 *      To-vacuum sinking: positive for s_before ≠ 0 → s_after = 0
 *      transitions, weighted by the pre-transition flux magnitude.
 *
 *   O10 JdotDeltaS := J_before · ∇(δs)
 *      Reaction-flux coupling: tests cross-correlation between flux
 *      gradient and the spatial structure of reactions.
 *
 * Naive lattice dimensions (PROTOCOL §2.2):
 *   O7: Δ = 2 (reaction-rate density, like state mass)
 *   O8: Δ = 4 (sourcing operator with one factor of |J|)
 *   O9: Δ = 4 (sinking operator with one factor of |J|)
 *   O10: Δ = 4 (gradient-coupled mixing)
 *
 * Implementation notes:
 *   - All operators are evaluated per-cell on a SnapshotPair{before, after}
 *     where both snapshots are DualCellFields with identical L.
 *   - The "_before" flux components come from snapshot A; the state
 *     increment comes from comparing A.rho_cell to B.rho_cell.
 *   - O8/O9 polarity (sign(δs)) preserves the directionality of state
 *     changes — useful for tracking matter/antimatter asymmetry signals.
 *   - O10 gradient is a central difference on δs(x); periodic boundaries.
 */

#include "ftd/eft/dual_cell_blocking.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace ftd {
namespace eft {

/// Pair of consecutive DualCellFields snapshots (before / after one tick).
struct SnapshotPair {
    const DualCellFields& before;
    const DualCellFields& after;

    int L() const { return before.L; }
    int total_sites() const { return before.total_sites(); }
};

/// δs(x) = s(x, t+1) - s(x, t). Always integer.
inline int delta_s_at(const SnapshotPair& p, int x, int y, int z) {
    const int idx = p.before.index(x, y, z);
    return p.after.rho_cell[idx] - p.before.rho_cell[idx];
}

/// Periodic-wrap helper for gradient.
inline int wrap(int i, int L) {
    return ((i % L) + L) % L;
}

/// |J_before|² at cell (x, y, z) using face-averaged components.
/// Mirrors the cell_J convention from campaign_operator_mixing_2026-04-26.
inline double J_before_magsq(const SnapshotPair& p, int x, int y, int z) {
    const auto& f = p.before;
    const int L = f.L;
    // Face-average to cell center (matches cell_J in campaign_operator_mixing).
    auto J_axis = [&](int xx, int yy, int zz, int axis) {
        if (axis == 0) {
            return 0.5 * (f.phi_x[f.index(xx, yy, zz)] +
                          f.phi_x[f.index(wrap(xx - 1, L), yy, zz)]);
        }
        if (axis == 1) {
            return 0.5 * (f.phi_y[f.index(xx, yy, zz)] +
                          f.phi_y[f.index(xx, wrap(yy - 1, L), zz)]);
        }
        return 0.5 * (f.phi_z[f.index(xx, yy, zz)] +
                      f.phi_z[f.index(xx, yy, wrap(zz - 1, L))]);
    };
    const double Jx = J_axis(x, y, z, 0);
    const double Jy = J_axis(x, y, z, 1);
    const double Jz = J_axis(x, y, z, 2);
    return Jx * Jx + Jy * Jy + Jz * Jz;
}

/// |J_before| at cell.
inline double J_before_mag(const SnapshotPair& p, int x, int y, int z) {
    return std::sqrt(J_before_magsq(p, x, y, z));
}

// ── Operator evaluators (per cell) ───────────────────────────────────────

/// O7 = (δs(x))²
inline double op_reactionDensity(const SnapshotPair& p, int x, int y, int z) {
    const double ds = static_cast<double>(delta_s_at(p, x, y, z));
    return ds * ds;
}

/// O8 = δs · θ(s_before = 0) · sign(δs) · |J_before|
///    = θ(s_before = 0) · |δs| · |J_before|  (since sign(δs) · δs = |δs|)
/// We separate magnitude and sign to make the "from-vacuum" channel explicit.
inline double op_genesisFlux(const SnapshotPair& p, int x, int y, int z) {
    const int idx = p.before.index(x, y, z);
    const int s_before = p.before.rho_cell[idx];
    if (s_before != 0) return 0.0;
    const int ds = p.after.rho_cell[idx] - s_before;  // = s_after
    if (ds == 0) return 0.0;
    return static_cast<double>(std::abs(ds)) * J_before_mag(p, x, y, z);
}

/// O9 = δs · θ(s_before ≠ 0 ∧ s_after = 0) · |J_before|
inline double op_evapFlux(const SnapshotPair& p, int x, int y, int z) {
    const int idx = p.before.index(x, y, z);
    const int s_before = p.before.rho_cell[idx];
    const int s_after = p.after.rho_cell[idx];
    if (s_before == 0) return 0.0;
    if (s_after != 0) return 0.0;
    // |δs| · |J_before|; δs = -s_before in the qualifying branch
    return static_cast<double>(std::abs(s_before)) * J_before_mag(p, x, y, z);
}

/// O10 = J_before(x) · ∇(δs(x))
/// Central difference on δs with periodic wrap.
inline double op_JdotDeltaS(const SnapshotPair& p, int x, int y, int z) {
    const auto& f = p.before;
    const int L = f.L;
    // Face-average J components at cell center.
    auto J_axis = [&](int xx, int yy, int zz, int axis) {
        if (axis == 0) {
            return 0.5 * (f.phi_x[f.index(xx, yy, zz)] +
                          f.phi_x[f.index(wrap(xx - 1, L), yy, zz)]);
        }
        if (axis == 1) {
            return 0.5 * (f.phi_y[f.index(xx, yy, zz)] +
                          f.phi_y[f.index(xx, wrap(yy - 1, L), zz)]);
        }
        return 0.5 * (f.phi_z[f.index(xx, yy, zz)] +
                      f.phi_z[f.index(xx, yy, wrap(zz - 1, L))]);
    };
    const double Jx = J_axis(x, y, z, 0);
    const double Jy = J_axis(x, y, z, 1);
    const double Jz = J_axis(x, y, z, 2);
    // ∇(δs) central difference
    const double dsdx = 0.5 * (delta_s_at(p, wrap(x + 1, L), y, z) -
                               delta_s_at(p, wrap(x - 1, L), y, z));
    const double dsdy = 0.5 * (delta_s_at(p, x, wrap(y + 1, L), z) -
                               delta_s_at(p, x, wrap(y - 1, L), z));
    const double dsdz = 0.5 * (delta_s_at(p, x, y, wrap(z + 1, L)) -
                               delta_s_at(p, x, y, wrap(z - 1, L)));
    return Jx * dsdx + Jy * dsdy + Jz * dsdz;
}

/// Number of reaction operators in the extension.
constexpr int kNumReactionOps = 4;

constexpr const char* kReactionOpNames[kNumReactionOps] = {
    "reactionDensity", "genesisFlux", "evapFlux", "JdotDeltaS"
};

/// Naive lattice dimensions (PROTOCOL §2.2).
constexpr double kReactionNaiveDim[kNumReactionOps] = {
    2.0, 4.0, 4.0, 4.0
};

/// Per-cell evaluator dispatched by reaction-operator id (0..3).
inline double evaluate_reaction_op(const SnapshotPair& p, int x, int y, int z, int op_id) {
    switch (op_id) {
        case 0: return op_reactionDensity(p, x, y, z);
        case 1: return op_genesisFlux(p, x, y, z);
        case 2: return op_evapFlux(p, x, y, z);
        case 3: return op_JdotDeltaS(p, x, y, z);
    }
    return 0.0;
}

/// Mean-over-cells reaction-operator vector for one snapshot pair.
template <typename Vec>
void accumulate_reaction_means(const SnapshotPair& p, Vec& out) {
    static_assert(std::is_same<typename Vec::value_type, double>::value,
                  "accumulate_reaction_means requires Vec<double>");
    const int L = p.L();
    double acc[kNumReactionOps] = {0.0, 0.0, 0.0, 0.0};
    for (int z = 0; z < L; ++z) {
        for (int y = 0; y < L; ++y) {
            for (int x = 0; x < L; ++x) {
                for (int a = 0; a < kNumReactionOps; ++a) {
                    acc[a] += evaluate_reaction_op(p, x, y, z, a);
                }
            }
        }
    }
    const double inv_N = 1.0 / static_cast<double>(L * L * L);
    for (int a = 0; a < kNumReactionOps; ++a) {
        out[a] = acc[a] * inv_N;
    }
}

}  // namespace eft
}  // namespace ftd
