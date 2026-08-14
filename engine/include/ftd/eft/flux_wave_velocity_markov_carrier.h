#pragma once

/**
 * @file flux_wave_velocity_markov_carrier.h
 * @brief FTD-0876 read-only canonical chart for native flux/wave velocity.
 *
 * The free undamped production wave state already stores three canonical
 * pairs per voxel: configuration `flux` and staggered momentum `wave_vel`.
 * This isolated witness exposes the exact two-slice history chart and the
 * symmetric-stiffness kick/drift map. It does not modify Voxel or claim that
 * the complete production tick is symplectic.
 */

#include "ftd/voxel.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace ftd::eft {

enum class FluxWaveVelocityCarrierStatus : std::uint8_t {
  Valid = 0,
  EmptyState,
  InvalidStep,
  InvalidTolerance,
  InvalidStiffnessShape,
  NonsymmetricStiffness,
  NonFiniteInput,
  NonFiniteOutput,
};

struct FluxWaveVelocityCarrierSite {
  Vec3 flux;
  Vec3 wave_velocity;
};

struct FluxHistoryChartResult {
  FluxWaveVelocityCarrierStatus status =
      FluxWaveVelocityCarrierStatus::InvalidStep;
  Vec3 previous_flux;
  Vec3 current_flux;
  FluxWaveVelocityCarrierSite carrier;
  Vec3 recovered_previous_flux;
  bool exact_roundtrip = false;

  bool valid() const { return status == FluxWaveVelocityCarrierStatus::Valid; }
};

struct FreeWaveKickDriftInput {
  std::vector<FluxWaveVelocityCarrierSite> sites;
  // Row-major scalar stiffness. The same K acts on all three flux components.
  std::vector<double> stiffness;
  double step = 1.0;
  double tolerance = 1e-12;
};

struct FreeWaveKickDriftResult {
  FluxWaveVelocityCarrierStatus status =
      FluxWaveVelocityCarrierStatus::EmptyState;
  std::vector<FluxWaveVelocityCarrierSite> before;
  std::vector<FluxWaveVelocityCarrierSite> after;
  std::vector<FluxWaveVelocityCarrierSite> recovered;
  double maximum_inverse_residual = 0.0;
  bool stiffness_symmetric = false;
  bool exact_inverse_verified = false;
  bool free_wave_symplectic = false;
  bool history_markov_equivalent = true;
  std::size_t native_canonical_pairs_per_site = 3;
  bool production_voxel_pair_used = true;
  bool complete_production_tick_symplectic = false;
  bool production_parity_actuator_supplied = false;
  bool native_record_preparation_supplied = false;
  bool native_gstar_synchronization_supplied = false;

  bool valid() const { return status == FluxWaveVelocityCarrierStatus::Valid; }
};

/** Copy the existing production flux/wave-velocity pair without mutation. */
FluxWaveVelocityCarrierSite carrier_from_voxel(const Voxel& voxel);

/** Exact local chart (J_(n-1),J_n) -> (J_n,(J_n-J_(n-1))/h). */
FluxHistoryChartResult flux_history_to_markov_carrier(
    const Vec3& previous_flux,
    const Vec3& current_flux,
    double step,
    double tolerance = 1e-12);

/** Exact symmetric-stiffness kick/drift and inverse reconstruction. */
FreeWaveKickDriftResult evolve_free_wave_kick_drift(
    const FreeWaveKickDriftInput& input);

/** Component-summed FTD-0875 bond generator. */
double vector_canonical_bond_generator(
    const FluxWaveVelocityCarrierSite& left,
    const FluxWaveVelocityCarrierSite& right);

/** Pullback scale of the canonical form under uniform (J,P)->rho(J,P). */
double uniform_damping_symplectic_scale(double rho);

/** Phase-volume determinant for N native vector sites under uniform damping. */
double uniform_damping_phase_determinant(double rho, std::size_t sites);

}  // namespace ftd::eft
