#pragma once
/**
 * @file state_only_matter_field_observer.h
 * @brief Instantaneous bound/characteristic field observer (FTD-0754).
 *
 * This observer is a deterministic function of one complete connected-pair
 * state.  It uses no tick number, route label, preparation label, stored
 * history, future state, or periodic-return information.
 *
 * The selected bound representative is the already registered finite-support
 * minimum-energy Gauss preparation.  The residual is split in a site-centred
 * readout into outgoing radial Maxwell characteristic and complementary
 * incoming-plus-radial background.  Reconstruction is exact in that declared
 * readout; this is not a primitive-cochain or ontological uniqueness claim.
 */

#include "ftd/eft/connected_moore_block_action.h"

#include <vector>

namespace ftd::eft {

struct CenteredCharacteristicSample {
  bool valid = false;
  Vec3 residual_electric{};
  Vec3 residual_magnetic{};
  Vec3 outgoing_electric{};
  Vec3 outgoing_magnetic{};
  Vec3 background_electric{};
  Vec3 background_magnetic{};
  double residual_energy = 0.0;
  double outgoing_energy = 0.0;
  double incoming_energy = 0.0;
  double radial_energy = 0.0;
  double background_energy = 0.0;
  double signed_radial_poynting = 0.0;
  double reconstruction_residual = 0.0;
  double energy_partition_residual = 0.0;
  double characteristic_flux_residual = 0.0;
};

/// Exact pointwise characteristic split for a supplied radial unit vector.
/// A zero radial vector deterministically assigns the full sample to
/// background because the state supplies no preferred central direction.
CenteredCharacteristicSample decompose_centered_characteristic_sample(
    const Vec3& residual_electric,
    const Vec3& residual_magnetic,
    const Vec3& radial_unit,
    double tolerance = 1e-12);

struct StateOnlyCharacteristicShell {
  int radius = 0;
  int samples = 0;
  double residual_energy = 0.0;
  double outgoing_energy = 0.0;
  double incoming_energy = 0.0;
  double radial_energy = 0.0;
  double background_energy = 0.0;
  double signed_radial_poynting = 0.0;
  double outward_characteristic_power = 0.0;
  double inward_characteristic_power = 0.0;
};

struct StateOnlyMatterFieldObserverOptions {
  int support_half_width = 4;
  std::vector<int> shell_radii{8, 12, 24, 48};
  double wave_speed = C_SPEED;
  double dt = 1.0;
  double poisson_tolerance = 1e-13;
  int poisson_max_iterations = 4096;
  double gate_tolerance = 1e-12;
  // Default false preserves the historical exact-integer-center domain.
  // True selects the FTD-0763 piecewise fractional-center chart.
  bool allow_fractional_center = false;
};

struct StateOnlyMatterFieldObservation {
  bool valid = false;
  bool state_only = true;
  bool centered_readout_only = true;
  bool primitive_cochain_uniqueness_claimed = false;
  int L = 0;
  int support_half_width = 0;
  int net_charge = 0;
  bool fractional_center_enabled = false;
  Vec3 center{};
  Vec3 support_center{};
  Vec3 fractional_center_offset{};
  double constituent_kinetic_energy = 0.0;
  double pair_internal_energy = 0.0;
  double bound_energy = 0.0;
  double residual_energy = 0.0;
  double outgoing_energy = 0.0;
  double incoming_energy = 0.0;
  double radial_energy = 0.0;
  double background_energy = 0.0;
  double bound_residual_interference = 0.0;
  // FTD-0754B observer-only decomposition of the preceding cross term.
  // The primitive face term equals a support-boundary exchange when the
  // residual is Gauss-free.  Centering and integer-time magnetic readout then
  // supply the two remaining metric terms.  None changes the frozen dynamics.
  bool boundary_energy_ledger_valid = false;
  double primitive_face_interference = 0.0;
  double induced_boundary_interference = 0.0;
  double centered_electric_interference = 0.0;
  double centered_magnetic_interference = 0.0;
  double centering_metric_interference = 0.0;
  double boundary_flux_sum = 0.0;
  double primitive_boundary_identity_residual = 0.0;
  double readout_interference_reconstruction_residual = 0.0;
  double signed_radial_poynting = 0.0;
  double outward_characteristic_power = 0.0;
  double inward_characteristic_power = 0.0;
  double maximum_reconstruction_residual = 0.0;
  double actual_gauss_compatibility_residual = 0.0;
  double energy_partition_residual = 0.0;
  double characteristic_flux_residual = 0.0;
  double bound_poisson_residual = 0.0;
  double bound_gauss_residual = 0.0;
  double bound_outside_maximum = 0.0;
  double bound_boundary_crossing_maximum = 0.0;
  std::vector<StateOnlyCharacteristicShell> shells;
};

/**
 * One primitive-face accounting row for a selected compact Gauss support.
 * The support is an observer resolution scale, not a material boundary.
 */
struct StateOnlySupportScale {
  bool valid = false;
  int support_half_width = 0;
  double actual_face_energy = 0.0;
  double bound_face_energy = 0.0;
  double residual_face_energy = 0.0;
  double primitive_interference = 0.0;
  double energy_reconstruction_residual = 0.0;
  double poisson_residual = 0.0;
  double gauss_residual = 0.0;
};

/**
 * Exact projection relation between two nested minimum-energy dressings.
 * If K_inner is contained in K_outer, then the zero-extended inner solution
 * is feasible for the outer problem and
 *
 *   U_inner = U_outer + 1/2 ||E_inner-E_outer||^2.
 */
struct StateOnlySupportTransition {
  bool valid = false;
  int inner_half_width = 0;
  int outer_half_width = 0;
  double relaxation_energy = 0.0;
  double outer_difference_inner_product = 0.0;
  double pythagorean_residual = 0.0;
  double monotonicity_margin = 0.0;
};

struct StateOnlySupportLadderObservation {
  bool valid = false;
  bool state_only = true;
  bool support_is_resolution_scale = true;
  int L = 0;
  bool fractional_center_enabled = false;
  Vec3 center{};
  Vec3 support_center{};
  Vec3 fractional_center_offset{};
  double maximum_energy_reconstruction_residual = 0.0;
  double maximum_projection_residual = 0.0;
  std::vector<StateOnlySupportScale> scales;
  std::vector<StateOnlySupportTransition> transitions;
};

/**
 * Observe one complete instantaneous state.  Odd L is required because the
 * adjacent-face/edge centering map has a Nyquist null mode on even periodic
 * volumes.  No state is mutated.
 */
StateOnlyMatterFieldObservation observe_state_only_matter_field(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& action_options,
    const StateOnlyMatterFieldObserverOptions& observer_options = {});

/**
 * Observe the exact primitive energy flow over a strictly increasing ladder
 * of finite Gauss supports.  This is a deterministic read-only function of
 * the same complete state and selected action parameters as the FTD-0754
 * observer.  It neither selects an ontic object boundary nor mutates state.
 */
StateOnlySupportLadderObservation observe_state_only_support_ladder(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& action_options,
    const std::vector<int>& support_half_widths,
    double poisson_tolerance = 1e-13,
    int poisson_max_iterations = 4096,
    double gate_tolerance = 1e-12,
    bool allow_fractional_center = false);

}  // namespace ftd::eft
