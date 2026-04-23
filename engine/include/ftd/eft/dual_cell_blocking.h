#pragma once
/**
 * @file ftd/eft/dual_cell_blocking.h
 * @brief Native finite-volume source/flux fields and b=2 blocking.
 *
 * This module implements the theorem-level blocking map specified in
 * docs/theory/10_eft_program/SPEC_FTD_NATIVE_BLOCKING_MAP.md. It is
 * intentionally independent of RenderBridge's cell-centered ternary storage.
 *
 * Native variables:
 *   - rho_cell: integrated source in each cell.
 *   - phi_x/y/z: oriented integrated flux through the +x/+y/+z face of
 *     each cell.
 *
 * The divergence is the finite-volume boundary sum:
 *
 *   div Phi(x) = Phi_x(x) - Phi_x(x-e_x)
 *              + Phi_y(x) - Phi_y(x-e_y)
 *              + Phi_z(x) - Phi_z(x-e_z).
 *
 * Under b=2 blocking, cell sources sum over 2^3 fine cells and coarse face
 * fluxes sum over the b^2 fine faces crossing the same coarse boundary.
 */

#include <vector>

namespace ftd {
namespace eft {

struct DualCellFields {
    int L = 0;
    std::vector<int> rho_cell;
    std::vector<double> phi_x;
    std::vector<double> phi_y;
    std::vector<double> phi_z;

    explicit DualCellFields(int size = 0);

    int total_sites() const { return L * L * L; }
    int index(int x, int y, int z) const;
};

double div_face_at(const DualCellFields& fields, int x, int y, int z);

void set_source_from_divergence(DualCellFields& fields);

int total_source(const DualCellFields& fields);

double max_gauss_residual(const DualCellFields& fields);

/// Native b=2 finite-volume blocking. Returns an empty field if fine.L is not
/// even or is smaller than 2.
DualCellFields block_dual_cell_b2(const DualCellFields& fine);

}  // namespace eft
}  // namespace ftd
