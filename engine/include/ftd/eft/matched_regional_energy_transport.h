#pragma once
/**
 * @file matched_regional_energy_transport.h
 * @brief Exact regional split of matched modified field energy (FTD-0671).
 */

#include "ftd/eft/matched_face_energy_transaction.h"
#include "ftd/voxel.h"

namespace ftd::eft {

struct MatchedRegionalEnergyTransportResult {
  int L = 0;
  Vec3 center{};
  double chebyshev_radius = 0.0;
  double lambda = 0.0;

  double energy_before = 0.0;
  double energy_pre_current = 0.0;
  double energy_after = 0.0;
  double boundary_transport_into = 0.0;
  double source_exchange_into_field = 0.0;
  double energy_change = 0.0;

  double magnetic_update_residual = 0.0;
  double electric_pre_update_residual = 0.0;
  double global_source_free_residual = 0.0;
  double partition_residual = 0.0;
  double regional_ledger_residual = 0.0;
  bool valid = false;
};

struct MatchedRegionalEnergySnapshot {
  int L = 0;
  Vec3 center{};
  double chebyshev_radius = 0.0;
  double lambda = 0.0;
  double total_energy = 0.0;
  double inside_energy = 0.0;
  double outside_energy = 0.0;
  double partition_residual = 0.0;
  bool valid = false;
};

MatchedRegionalEnergySnapshot measure_matched_regional_energy(
    const MatchedFaceFlux& electric,
    const MatchedEdgeField& magnetic,
    double lambda,
    const Vec3& center,
    double chebyshev_radius,
    double tolerance = 1e-12);

/**
 * Evaluate the exact split
 *
 *   U_R(after)-U_R(before)
 *     = [U_R(pre-current)-U_R(before)]
 *     + [U_R(after)-U_R(pre-current)].
 *
 * The first bracket is signed curl transport into the region and the second
 * is local current/source exchange.  The regional modified energy uses a
 * symmetric face/edge projector, including the leapfrog cross term.
 */
MatchedRegionalEnergyTransportResult
evaluate_matched_regional_energy_transport(
    const MatchedFaceFlux& electric_before,
    const MatchedEdgeField& magnetic_before,
    const MatchedFaceFlux& electric_pre_current,
    const MatchedEdgeField& magnetic_after,
    const MatchedFaceFlux& electric_after,
    double lambda,
    const Vec3& center,
    double chebyshev_radius,
    double tolerance = 1e-12);

}  // namespace ftd::eft
