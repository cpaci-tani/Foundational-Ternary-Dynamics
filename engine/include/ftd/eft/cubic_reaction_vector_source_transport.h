#pragma once
/**
 * @file cubic_reaction_vector_source_transport.h
 * @brief FTD-0889/0890 cubic reaction-vector/source-transport reference.
 *
 * A scalar Gauss reaction amplitude cannot select a spatial direction under
 * O_h.  This isolated witness uses the minimum orientation-free vector
 * carrier (three canonical pairs), maps its ready-slice quadratic energy
 * exactly to the selected production relativistic dispersion by a cotangent
 * chart, and continues the resulting source through one reversible free drift
 * with an exact face-current continuity observer.
 *
 * This is not a production matter law, a mass derivation, or a native common
 * action coupling.
 */

#include "ftd/eft/canonical_subcell_section.h"
#include "ftd/eft/face_current_segment.h"
#include "ftd/ontic/particle_masses.h"

#include <cstdint>

namespace ftd::eft {

enum class CubicReactionSourceTransportStatus : std::uint8_t {
  Valid = 0,
  NonFiniteInput,
  InvalidRestEnergy,
  InvalidSpeed,
  InvalidTimeStep,
  InvalidLatticeSize,
  InvalidCharge,
  InsufficientResidualEnergy,
  InvalidSubcellChart,
  CurrentContinuityFailure,
};

struct CubicReactionSourceTransportInput {
  /// Coordinate half of the orientation-free T1u reaction triplet.
  Vec3 reaction_coordinate{};
  /// Required matter impulse, normally -Delta P_field from the matched field.
  Vec3 required_matter_impulse{};
  Coord source_anchor{};
  Vec3 source_remainder{};
  double residual_amplitude = 1.0;
  double rest_energy = E_REST;
  double limiting_speed = C_SPEED;
  double dt = 1.0;
  double tolerance = 1e-12;
  int lattice_size = 9;
  int charge = 1;
};

struct CubicReactionSourceTransportResult {
  CubicReactionSourceTransportStatus status =
      CubicReactionSourceTransportStatus::NonFiniteInput;

  Vec3 reaction_coordinate{};
  Vec3 reaction_momentum{};
  Vec3 recovered_reaction_coordinate{};
  Vec3 recovered_reaction_momentum{};
  Vec3 physical_coordinate{};
  Vec3 physical_momentum{};
  Vec3 physical_velocity{};
  Vec3 required_matter_impulse{};

  SubcellChart source_before{};
  SubcellChart source_after{};
  FaceCurrentSegment current_segment{};

  double reaction_radius = 0.0;
  double tangential_jacobian_eigenvalue = 0.0;
  double radial_jacobian_eigenvalue = 0.0;
  double jacobian_determinant = 0.0;
  double residual_energy = 0.0;
  double required_kinetic_energy = 0.0;
  double history_energy = 0.0;
  double reaction_energy = 0.0;
  double split_angle = 0.0;
  double low_energy_inertial_mass = 0.0;
  double energy_chart_residual = 0.0;
  double reaction_inverse_residual = 0.0;
  double coordinate_inverse_residual = 0.0;
  double split_amplitude_residual = 0.0;
  double drift_inverse_residual = 0.0;
  double current_continuity_residual = 0.0;

  bool scalar_reaction_direction_forbidden_by_cubic_symmetry = true;
  bool orientation_free_vector_requires_three_canonical_pairs = true;
  bool fixed_direction_scalar_pair_is_conditional = true;
  bool orientation_defined_by_field_impulse = false;
  bool exact_relativistic_energy_chart = false;
  bool cotangent_chart_symplectic = true;
  bool exact_reversible_free_transport = false;
  bool exact_face_current_continuity = false;
  bool split_angle_fixed_by_local_conservation = false;
  bool equal_split = false;
  bool inertial_mass_scale_derived = false;
  bool native_vector_common_action_supplied = false;
  bool production_coupling_supplied = false;
  bool born_target_used = false;
  bool native_gstar_synchronization_supplied = false;
  bool new_selected_type_added = false;

  bool valid() const {
    return status == CubicReactionSourceTransportStatus::Valid;
  }
};

CubicReactionSourceTransportResult
analyze_cubic_reaction_vector_source_transport(
    const CubicReactionSourceTransportInput& input);

}  // namespace ftd::eft
