#pragma once
/**
 * @file paired_field_response.h
 * @brief Observer-only moving/rest field-response algebra (FTD-0768).
 */

#include "ftd/eft/connected_moore_block_action.h"

#include <array>
#include <limits>

namespace ftd::eft {

inline constexpr double kInvalidFieldResponseResidual =
    std::numeric_limits<double>::infinity();

enum class FieldResponseRegionKind {
  OrientedSlab,
  ChebyshevCube,
};

struct FieldResponseRegionSpec {
  FieldResponseRegionKind kind = FieldResponseRegionKind::ChebyshevCube;
  Vec3 center{};
  Vec3 longitudinal{0.0, 0.0, 1.0};
  Vec3 transverse_u{1.0, 0.0, 0.0};
  Vec3 transverse_v{0.0, 1.0, 0.0};
  double longitudinal_half_width = 0.5;
  double transverse_half_width = 4.0;
  double chebyshev_radius = 0.0;
};

struct QuadraticFieldDifferenceChannel {
  double moving_energy = 0.0;
  double rest_energy = 0.0;
  double energy_difference = 0.0;
  double difference_field_energy = 0.0;
  double cross_energy = 0.0;
  double energy_identity_residual = 0.0;
  double energy_difference_first_moment = 0.0;
  double difference_field_first_moment = 0.0;
  double cross_first_moment = 0.0;
};

struct PairedFieldResponseRegion {
  FieldResponseRegionSpec spec{};
  QuadraticFieldDifferenceChannel actual{};
  QuadraticFieldDifferenceChannel residual{};
};

struct PairedFieldResponseOptions {
  int support_half_width = 4;
  int near_radius = 8;
  int outer_radius = 48;
  Vec3 laboratory_center{};
  Vec3 moving_center{};
  Vec3 longitudinal{0.0, 0.0, 1.0};
  Vec3 transverse_u{1.0, 0.0, 0.0};
  Vec3 transverse_v{0.0, 1.0, 0.0};
  double wave_speed = C_SPEED;
  double dt = 0.25;
  double poisson_tolerance = 1e-13;
  int poisson_max_iterations = 4096;
  double gate_tolerance = 1e-12;
};

struct PairedFieldResponseObservation {
  int L = 0;
  bool valid = false;
  Vec3 moving_bound_center{};
  Vec3 rest_bound_center{};
  double moving_bound_gauss_residual = kInvalidFieldResponseResidual;
  double rest_bound_gauss_residual = kInvalidFieldResponseResidual;
  double maximum_energy_identity_residual = kInvalidFieldResponseResidual;
  std::array<PairedFieldResponseRegion, 4> regions{};
};

struct RegionalModifiedEnergyTransportObservation {
  bool valid = false;
  FieldResponseRegionSpec spec{};
  double energy_before = 0.0;
  double energy_pre_current = 0.0;
  double energy_after = 0.0;
  double outside_energy_before = 0.0;
  double outside_energy_pre_current = 0.0;
  double outside_energy_after = 0.0;
  double boundary_transport_into = 0.0;
  double boundary_transport_into_complement = 0.0;
  double source_exchange_into_field = 0.0;
  double energy_change = 0.0;
  double global_source_free_residual = kInvalidFieldResponseResidual;
  double boundary_quadrature_residual = kInvalidFieldResponseResidual;
  double ledger_residual = kInvalidFieldResponseResidual;
};

/** Exact discrete Reynolds term between two region charts on one before-state. */
struct RegionalControlVolumeTransportObservation {
  bool valid = false;
  double previous_energy_before = 0.0;
  double current_energy_before = 0.0;
  double current_energy_after = 0.0;
  double mask_sweep_into = 0.0;
  double mask_sweep_into_complement = 0.0;
  double mask_sweep_quadrature_residual = kInvalidFieldResponseResidual;
  double transported_energy_change = 0.0;
  double transport_identity_residual = kInvalidFieldResponseResidual;
};

std::array<FieldResponseRegionSpec, 4> make_ftd0768_response_regions(
    const PairedFieldResponseOptions& options);

PairedFieldResponseObservation observe_paired_field_response(
    const ConnectedMooreBlockState& moving,
    const ConnectedMooreBlockState& rest,
    const ConnectedMooreBlockOptions& action_options,
    const PairedFieldResponseOptions& observer_options);

RegionalModifiedEnergyTransportObservation
observe_regional_modified_energy_transport(
    const MatchedFaceFlux& electric_before,
    const MatchedEdgeField& magnetic_before,
    const MatchedFaceFlux& electric_pre_current,
    const MatchedEdgeField& magnetic_after,
    const MatchedFaceFlux& electric_after,
    double lambda,
    const FieldResponseRegionSpec& region,
    double tolerance = 1e-12);

RegionalControlVolumeTransportObservation
derive_regional_control_volume_transport(
    const RegionalModifiedEnergyTransportObservation& previous_region,
    const RegionalModifiedEnergyTransportObservation& current_region,
    double tolerance = 1e-12);

}  // namespace ftd::eft
