#pragma once

/**
 * @file native_event_characteristics.h
 * @brief FTD-0858 isolated event-acceptance and characteristic-chart witness.
 *
 * This API reproduces the source-level acceptance predicates and the exact
 * relative two-port chart proved by FTD-0858. It deliberately keeps them as
 * separate interfaces: common-field event acceptance does not determine an
 * on-shell relative receiver. Nothing here is called by Voxel or a production
 * tick phase, and no physical controller or clock gate is supplied.
 */

#include <cstdint>

namespace ftd::eft {

enum class NativeEventCharacteristicStatus : std::uint8_t {
  Valid = 0,
  InvalidRecord,
  InvalidMagnitude,
  InvalidThreshold,
  InvalidScale,
  InvalidDraw,
  InvalidEnergy,
  InvalidRate,
  InvalidProperTimeRate,
  InvalidCoordinate,
  InvalidWaveSpeed,
};

struct NativeGenesisAcceptanceInput {
  bool enabled = false;
  std::int8_t record = 0;
  double common_magnitude = 0.0;
  double genesis_threshold = 0.0;
  double manifestation_scale = 0.0;
  double uniform_draw = 0.0;
};

struct NativeEvaporationAcceptanceInput {
  bool event_processing_enabled = false;
  std::int8_t record = 0;
  bool locked = false;
  double common_energy_site_plus_faces = 0.0;
  double manifestation_scale = 0.0;
  double evaporation_rate = 0.0;
  double proper_time_rate = 0.0;
  double uniform_draw = 0.0;
};

struct NativeEventAcceptanceResult {
  NativeEventCharacteristicStatus status =
      NativeEventCharacteristicStatus::InvalidRecord;
  double acceptance_threshold = 0.0;
  bool accepted = false;

  bool valid() const {
    return status == NativeEventCharacteristicStatus::Valid;
  }
};

/** Exact dual-path genesis predicate, conditional on the supplied keyed draw. */
NativeEventAcceptanceResult evaluate_native_genesis_acceptance(
    const NativeGenesisAcceptanceInput& input);

/** Exact shared evaporation predicate, conditional on the supplied keyed draw. */
NativeEventAcceptanceResult evaluate_native_evaporation_acceptance(
    const NativeEvaporationAcceptanceInput& input);

enum class OrderedNativeEvent : std::uint8_t {
  None = 0,
  Genesis,
  Evaporation,
  GenesisThenEvaporation,
};

OrderedNativeEvent classify_ordered_native_event(
    bool genesis_accepted, bool evaporation_accepted);

struct CommonRelativeScalar {
  double common = 0.0;
  double relative = 0.0;
};

struct BilateralScalar {
  double left = 0.0;
  double right = 0.0;
};

CommonRelativeScalar common_relative_scalar(double left, double right);
BilateralScalar bilateral_scalar(double common, double relative);

struct RelativeCharacteristicChart {
  NativeEventCharacteristicStatus status =
      NativeEventCharacteristicStatus::InvalidCoordinate;
  double relative_momentum = 0.0;
  double oriented_strain = 0.0;
  double incoming = 0.0;
  double outgoing = 0.0;
  double pair_energy = 0.0;
  double characteristic_energy = 0.0;
  double energy_residual = 0.0;
  double signed_current = 0.0;
  double characteristic_current = 0.0;
  double current_residual = 0.0;

  bool valid() const {
    return status == NativeEventCharacteristicStatus::Valid;
  }
};

RelativeCharacteristicChart relative_characteristic_chart(
    double relative_momentum, double oriented_strain);

struct AxialC18Dispersion {
  NativeEventCharacteristicStatus status =
      NativeEventCharacteristicStatus::InvalidWaveSpeed;
  double wave_number = 0.0;
  double wave_speed_squared = 0.0;
  double source_eigenvalue = 0.0;
  double production_trace = 0.0;
  double production_determinant = 0.0;
  double sin_half_theta_squared = 0.0;
  double one_cell_shift_trace = 0.0;
  double one_cell_shift_trace_defect = 0.0;

  bool valid() const {
    return status == NativeEventCharacteristicStatus::Valid;
  }
};

AxialC18Dispersion axial_c18_dispersion(
    double wave_number, double wave_speed_squared);

/** Production-selected C_WAVE^2 specialization. */
AxialC18Dispersion production_axial_c18_dispersion(double wave_number);

}  // namespace ftd::eft

