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

namespace ftd { class RenderBridge; }  // forward decl for adapter below

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

/// Translate a RenderBridge state into a DualCellFields snapshot.
///
/// - rho_cell[i] := rb.voxels()[i].state
/// - phi_x[i]    := 0.5 * (flux.x[i] + flux.x[i + e_x])  (face-averaged flux
///                  through the +x face of cell i, periodic wrap)
/// - phi_y[i], phi_z[i] analogously
///
/// Under this face-averaging convention, the finite-volume div_face(phi)(i)
/// is half the engine's central-difference divergence of flux at cell i:
///   div_face(phi)(i) = 0.5 * (flux.x[i+e_x] - flux.x[i-e_x] + ... y, z terms)
/// (the factor of 1/2 comes from the face average canceling adjacent terms).
/// This is internally consistent with the DualCellFields divergence
/// convention, but it does NOT equal the engine's energy_audit().gauss_violation;
/// bridge-comparison tests must account for the 1/2 scaling.
///
/// The adapter is a pure snapshot: it does NOT modify the bridge. Under GPU
/// backend it triggers a sync-to-host via voxels().
DualCellFields render_bridge_to_dual_cell_fields(const RenderBridge& rb);

}  // namespace eft
}  // namespace ftd
