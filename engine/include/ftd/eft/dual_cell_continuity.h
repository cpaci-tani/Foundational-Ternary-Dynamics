#pragma once
/**
 * @file ftd/eft/dual_cell_continuity.h
 * @brief Finite-volume reaction/transport continuity and b=2 blocking.
 *
 * Native convention:
 *
 *   Delta rho + div I = S_reaction
 *
 * where I is integrated signed current through oriented cell faces during one
 * tick. This is the current-side analogue of dual_cell_blocking.h.
 */

#include <vector>

namespace ftd {
namespace eft {

struct DualCellContinuity {
    int L = 0;
    std::vector<int> rho_before;
    std::vector<int> rho_after;
    std::vector<int> reaction;
    std::vector<double> current_x;
    std::vector<double> current_y;
    std::vector<double> current_z;

    explicit DualCellContinuity(int size = 0);

    int total_sites() const { return L * L * L; }
    int index(int x, int y, int z) const;
};

struct DualCellHistoryExtraction {
    bool valid = false;
    int transported_events = 0;
    int annihilation_pairs = 0;
    int reaction_sites = 0;
};

struct DualCellOperatorMoments {
    double delta_rho_l1 = 0.0;
    double current_l1 = 0.0;
    double div_current_l1 = 0.0;
    int reaction_l1 = 0;
    double residual_linf = 0.0;
};

double div_current_at(const DualCellContinuity& fields, int x, int y, int z);

double continuity_residual_at(const DualCellContinuity& fields,
                              int x, int y, int z);

double max_continuity_residual(const DualCellContinuity& fields);

int total_before(const DualCellContinuity& fields);
int total_after(const DualCellContinuity& fields);
int total_reaction(const DualCellContinuity& fields);
double total_current_l1(const DualCellContinuity& fields);
int total_reaction_l1(const DualCellContinuity& fields);
DualCellOperatorMoments measure_operator_moments(
    const DualCellContinuity& fields);

/// Native b=2 finite-volume blocking for the reaction/transport continuity
/// equation. Returns an empty field if fine.L is not even or is smaller than 2.
DualCellContinuity block_dual_cell_continuity_b2(
    const DualCellContinuity& fine);

/// Add one closed one-tick history into an interval history. The interval keeps
/// the initial rho_before, updates rho_after to the step's final state, and
/// sums all integrated currents/reactions. Returns false on invalid sizes.
bool accumulate_continuity_step(DualCellContinuity& interval,
                                const DualCellContinuity& step);

/// Extract a one-tick finite-volume history from signed state snapshots.
///
/// The extractor pairs void-target Moore-neighborhood moves into oriented face
/// currents, routes diagonal moves as deterministic x/y/z face paths, treats
/// adjacent opposite-charge disappearances as annihilation reaction pairs, and
/// records any remaining local delta as S_reaction.
DualCellHistoryExtraction extract_moore_history_from_snapshots(
    int L,
    const std::vector<int>& rho_before,
    const std::vector<int>& rho_after,
    DualCellContinuity& out);

}  // namespace eft
}  // namespace ftd
