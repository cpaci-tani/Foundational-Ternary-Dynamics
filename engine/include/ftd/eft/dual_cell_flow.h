#pragma once
/**
 * @file ftd/eft/dual_cell_flow.h
 * @brief Bare native-flow measurements for finite-volume dual-cell fields.
 *
 * These helpers measure the scaling of the canonical Gaussian source/flux
 * energy under the native b=2 blocking map. They do not compare against QED
 * or any external target.
 */

#include "ftd/eft/dual_cell_blocking.h"
#include "ftd/eft/dual_cell_continuity.h"

namespace ftd {
namespace eft {

struct NativeB2FlowReport {
    int fine_L = 0;
    int coarse_L = 0;
    int total_source_fine = 0;
    int total_source_coarse = 0;
    double gauss_residual_fine = 0.0;
    double gauss_residual_coarse = 0.0;
    double flux_energy_fine = 0.0;
    double flux_energy_coarse = 0.0;
    double flux_energy_ratio = 0.0;
    bool source_conserved = false;
    bool gauss_preserved = false;
};

/// Canonical physical flux energy for integrated face fluxes at a given scale:
///
///   E = 1/2 sum_cells cell_volume * sum_i (Phi_i / face_area)^2.
///
/// For the microscopic lattice, cell_volume=1 and face_area=1. After one b=2
/// block, cell_volume=8 and face_area=4.
double canonical_flux_energy(const DualCellFields& fields,
                             double cell_volume = 1.0,
                             double face_area = 1.0);

/// Native static response coefficient in the Gaussian source kernel:
///
///   W[rho] = 1/2 sum rho^2 / (C_L * G_symbol)
///
/// equivalently inverse_kernel = G_symbol / C_L. In the bare native
/// generator inverse_kernel == G_symbol, hence C_L=1.
double native_static_response_coefficient(double operator_symbol,
                                          double inverse_kernel_symbol);

/// Canonical finite-volume current/flux vertex:
///
///   V = sum_cells cell_volume * (I_i / face_area) * (Phi_i / face_area)
///
/// The native interaction is -g_sJ * V, with g_sJ=1 in the Gaussian bridge.
double canonical_current_flux_vertex(const DualCellContinuity& current,
                                     const DualCellFields& flux,
                                     double cell_volume = 1.0,
                                     double face_area = 1.0);

/// Block a field once by b=2 and compare canonical flux energy under the
/// physical area/volume rescaling. This is the bare Gaussian K_T/C_L=1 audit;
/// nonlinear/reaction flow must be measured separately.
NativeB2FlowReport measure_native_b2_flow(const DualCellFields& fine,
                                          double tolerance = 1e-12);

}  // namespace eft
}  // namespace ftd
