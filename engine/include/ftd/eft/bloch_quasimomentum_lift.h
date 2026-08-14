#pragma once
/**
 * @file bloch_quasimomentum_lift.h
 * @brief FTD-0894/0896 isolated Bloch wrap/lift/carry reference witness.
 *
 * Integer translation supplies quasimomentum modulo reciprocal-lattice
 * vectors. This analyzer shows exactly what an additional winding triplet
 * would retain. It does not derive winding dynamics, a physical momentum
 * unit, a total field-matter charge, or production mechanics.
 */

#include <array>
#include <cstdint>

namespace ftd::eft {

using BlochTriplet = std::array<double, 3>;
using BlochWinding = std::array<std::int64_t, 3>;

enum class BlochQuasimomentumLiftStatus : std::uint8_t {
  Valid = 0,
  NonFiniteInput,
  InvalidTolerance,
  InvalidMomentumScale,
  InvalidFiniteRangeOrder,
  NonPrincipalLabel,
  WindingOverflow,
};

struct BlochQuasimomentumLiftInput {
  BlochTriplet principal_first{};
  BlochWinding winding_first{};
  BlochTriplet principal_second{};
  BlochWinding winding_second{};

  /// Imposed conversion in P_candidate = momentum_scale * k_tilde.
  double momentum_scale = 1.0;
  /// Finite sawtooth truncation range used only as a local observer witness.
  int finite_range_order = 8;
  double tolerance = 1e-12;
};

struct BlochQuasimomentumLiftResult {
  BlochQuasimomentumLiftStatus status =
      BlochQuasimomentumLiftStatus::NonFiniteInput;

  BlochTriplet lifted_first{};
  BlochTriplet lifted_second{};
  BlochTriplet principal_sum{};
  BlochWinding principal_carry{};
  BlochWinding combined_winding{};
  BlochTriplet lifted_sum{};
  BlochTriplet reconstructed_lifted_sum{};
  BlochTriplet reciprocal_information{};
  BlochTriplet physical_momentum_candidate{};
  BlochTriplet doubled_scale_momentum_candidate{};
  BlochTriplet finite_range_sawtooth_weight{};

  double real_addition_residual = 0.0;
  double periodicity_residual = 0.0;
  double finite_range_branch_residual = 0.0;

  bool torus_quasimomentum_addition_exact = false;
  bool winding_reconstructs_real_addition = false;
  bool zone_crossing_observed = false;
  bool principal_only_loses_reciprocal_information = false;
  bool finite_range_weight_is_periodic = false;
  bool global_continuous_homomorphic_section_exists = false;
  bool finite_range_global_unwrapped_generator_exists = false;
  bool exact_principal_generator_is_finite_range = false;

  bool winding_dynamics_derived = false;
  bool physical_momentum_scale_derived = false;
  bool total_field_matter_momentum_map_derived = false;
  bool absolute_mass_derived = false;
  bool local_stress_route_ruled_out = false;
  bool production_coupling_supplied = false;
  bool born_target_used = false;
  bool native_gstar_synchronization_supplied = false;
  bool new_selected_vector_type_added = false;

  bool valid() const {
    return status == BlochQuasimomentumLiftStatus::Valid;
  }
};

BlochQuasimomentumLiftResult analyze_bloch_quasimomentum_lift(
    const BlochQuasimomentumLiftInput& input);

}  // namespace ftd::eft
