#pragma once
/**
 * @file dressed_boost_momentum_map.h
 * @brief FTD-0893 conditional dressed-inertia reference witness.
 *
 * For one matter-like and one field-like time-odd amplitude per cubic axis,
 * this isolated analyzer evaluates M = B A^{-1} B^T and the unique
 * minimum-energy allocation at fixed imposed physical momentum.  It does not
 * derive B, an absolute mass, or a production field-matter Noether charge.
 */

#include "ftd/eft/face_current_segment.h"

#include <cstdint>

namespace ftd::eft {

enum class DressedBoostMomentumMapStatus : std::uint8_t {
  Valid = 0,
  NonFiniteInput,
  InvalidTolerance,
  NonPositiveEnergyHessian,
  ZeroMomentumMap,
  InvalidMomentumMapScale,
};

struct DressedBoostMomentumMapInput {
  /// Per-axis odd-sector energy Hessian [[matter_cost, coupling],
  ///                                      [coupling, field_cost]].
  double matter_cost = 1.0;
  double field_cost = 1.0;
  double kinetic_coupling = 0.0;

  /// Imposed physical-momentum row B = [matter_weight, field_weight].
  double matter_momentum_weight = 1.0;
  double field_momentum_weight = 0.0;
  Vec3 total_momentum{};

  /// Rest/static energy.  It is recorded but cannot change inertia.
  double static_energy_offset = 0.0;
  /// Nonzero control s for the exact ambiguity B -> s B, M -> s^2 M.
  double momentum_map_scale = 2.0;
  double tolerance = 1e-12;
};

struct DressedBoostMomentumMapResult {
  DressedBoostMomentumMapStatus status =
      DressedBoostMomentumMapStatus::NonFiniteInput;

  Vec3 matter_odd_amplitude{};
  Vec3 field_odd_amplitude{};
  Vec3 reconstructed_momentum{};

  double energy_hessian_determinant = 0.0;
  double dressed_inertial_mass = 0.0;
  double inverse_mass_curvature = 0.0;
  double minimum_kinetic_energy = 0.0;
  double minimum_total_energy = 0.0;
  double scaled_momentum_map_mass = 0.0;
  double momentum_residual = 0.0;
  double energy_residual = 0.0;

  bool energy_hessian_positive_definite = false;
  bool momentum_map_rank_one_per_axis = false;
  bool unique_constrained_minimum = false;
  bool exact_conditional_dressed_mass = false;
  bool cubic_covariant_by_construction = true;
  bool field_odd_sector_participates = false;
  bool momentum_scale_ambiguity_exposed = false;
  bool static_offset_contributes_to_inertia = false;

  bool total_momentum_map_derived = false;
  bool absolute_mass_derived = false;
  bool common_action_noether_closure = false;
  bool stable_matter_pole_derived = false;
  bool production_coupling_supplied = false;
  bool born_target_used = false;
  bool native_gstar_synchronization_supplied = false;
  bool new_selected_vector_type_added = false;

  bool valid() const {
    return status == DressedBoostMomentumMapStatus::Valid;
  }
};

DressedBoostMomentumMapResult analyze_dressed_boost_momentum_map(
    const DressedBoostMomentumMapInput& input);

}  // namespace ftd::eft
