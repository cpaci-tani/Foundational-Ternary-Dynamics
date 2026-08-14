#pragma once
/**
 * @file collective_reaction_triplet_inertia.h
 * @brief FTD-0891/0892 collective symplectic and inertia witness.
 *
 * Once selected constituent canonical pairs exist, their Helmert reduction
 * contains the three-pair collective sector (X,P) exactly. This isolated
 * witness also evaluates the exact minimum-energy composite dispersion that
 * follows conditionally from selected relativistic constituent dispersions.
 *
 * It does not derive constituent phase space, an absolute mass scale, a
 * stable pole, or an exact continuous-translation field+matter Noether charge.
 */

#include "ftd/eft/face_current_segment.h"

#include <cstdint>
#include <vector>

namespace ftd::eft {

enum class CollectiveReactionTripletStatus : std::uint8_t {
  Valid = 0,
  EmptyConstituentSet,
  SizeMismatch,
  NonFiniteInput,
  InvalidRestEnergy,
  InvalidSpeed,
  InvalidTolerance,
};

struct CollectiveReactionTripletInput {
  std::vector<Vec3> positions;
  std::vector<Vec3> momenta;
  /// Nontrivial tangent probe used to evaluate the canonical one-form split.
  std::vector<Vec3> position_tangents;
  std::vector<double> constituent_rest_energies;
  /// Any simultaneous constituent impulses. Their sum is the collective kick.
  std::vector<Vec3> constituent_impulses;
  double limiting_speed = C_SPEED;
  /// Static binding/field offset that does not participate in the boost.
  double static_binding_offset = 0.0;
  double tolerance = 1e-12;
};

struct CollectiveReactionTripletResult {
  CollectiveReactionTripletStatus status =
      CollectiveReactionTripletStatus::NonFiniteInput;

  Vec3 center{};
  Vec3 total_momentum{};
  Vec3 summed_constituent_impulse{};
  Vec3 momentum_after_impulse{};

  std::vector<Vec3> modal_positions;
  std::vector<Vec3> modal_momenta;
  std::vector<Vec3> modal_position_tangents;
  std::vector<Vec3> reconstructed_positions;
  std::vector<Vec3> reconstructed_momenta;
  std::vector<Vec3> minimum_energy_momenta;

  double summed_rest_energy = 0.0;
  double input_constituent_energy = 0.0;
  double minimum_constituent_energy = 0.0;
  double collective_dispersion_energy = 0.0;
  double collective_inertial_mass = 0.0;
  double zero_momentum_energy_curvature = 0.0;
  double rest_energy_with_static_offset = 0.0;
  double static_offset_mass_mismatch = 0.0;

  double constituent_one_form = 0.0;
  double modal_one_form = 0.0;
  double collective_internal_one_form = 0.0;
  double one_form_residual = 0.0;
  double orthogonality_residual = 0.0;
  double position_reconstruction_residual = 0.0;
  double momentum_reconstruction_residual = 0.0;
  double impulse_sum_residual = 0.0;
  double composite_energy_residual = 0.0;
  double common_velocity_residual = 0.0;

  bool exact_collective_symplectic_sector = false;
  bool three_collective_canonical_pairs = false;
  bool cubic_covariant_by_construction = true;
  bool internal_zero_sum_impulses_cancel = false;
  bool external_impulses_sum_to_collective_kick = false;
  bool constituent_dispersion_strictly_convex = false;
  bool exact_conditional_composite_dispersion = false;
  bool conditional_inertial_additivity = false;
  bool static_binding_offset_participates_in_boost = false;
  bool static_hessian_determines_inertia = false;
  bool rest_energy_alone_determines_dispersion_curvature = false;
  bool absolute_mass_scale_derived = false;
  bool exact_total_field_matter_noether_momentum_supplied = false;
  bool constituent_phase_space_derived = false;
  bool stable_matter_pole_derived = false;
  bool production_coupling_supplied = false;
  bool born_target_used = false;
  bool native_gstar_synchronization_supplied = false;
  bool new_selected_vector_type_added = false;

  bool valid() const {
    return status == CollectiveReactionTripletStatus::Valid;
  }
};

CollectiveReactionTripletResult analyze_collective_reaction_triplet_inertia(
    const CollectiveReactionTripletInput& input);

}  // namespace ftd::eft
