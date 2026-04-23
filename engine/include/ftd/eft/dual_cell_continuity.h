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

double div_current_at(const DualCellContinuity& fields, int x, int y, int z);

double continuity_residual_at(const DualCellContinuity& fields,
                              int x, int y, int z);

double max_continuity_residual(const DualCellContinuity& fields);

int total_before(const DualCellContinuity& fields);
int total_after(const DualCellContinuity& fields);
int total_reaction(const DualCellContinuity& fields);

/// Native b=2 finite-volume blocking for the reaction/transport continuity
/// equation. Returns an empty field if fine.L is not even or is smaller than 2.
DualCellContinuity block_dual_cell_continuity_b2(
    const DualCellContinuity& fine);

}  // namespace eft
}  // namespace ftd
