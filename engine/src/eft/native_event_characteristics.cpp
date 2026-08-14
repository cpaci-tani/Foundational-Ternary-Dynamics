#include "ftd/eft/native_event_characteristics.h"

#include "ftd/ontic/gauge_couplings.h"

#include <cmath>

namespace ftd::eft {
namespace {

bool valid_record(std::int8_t record) {
  return record >= -1 && record <= 1;
}

bool valid_uniform_draw(double draw) {
  return std::isfinite(draw) && draw >= 0.0 && draw < 1.0;
}

}  // namespace

NativeEventAcceptanceResult evaluate_native_genesis_acceptance(
    const NativeGenesisAcceptanceInput& input) {
  NativeEventAcceptanceResult result;
  if (!valid_record(input.record)) {
    result.status = NativeEventCharacteristicStatus::InvalidRecord;
    return result;
  }
  if (!std::isfinite(input.common_magnitude)
      || input.common_magnitude < 0.0) {
    result.status = NativeEventCharacteristicStatus::InvalidMagnitude;
    return result;
  }
  if (!std::isfinite(input.genesis_threshold)
      || !(input.genesis_threshold > 0.0)) {
    result.status = NativeEventCharacteristicStatus::InvalidThreshold;
    return result;
  }
  if (!std::isfinite(input.manifestation_scale)
      || !(input.manifestation_scale > 0.0)) {
    result.status = NativeEventCharacteristicStatus::InvalidScale;
    return result;
  }
  if (!valid_uniform_draw(input.uniform_draw)) {
    result.status = NativeEventCharacteristicStatus::InvalidDraw;
    return result;
  }

  result.status = NativeEventCharacteristicStatus::Valid;
  if (!input.enabled || input.record != 0
      || !(input.common_magnitude > input.genesis_threshold)) {
    return result;
  }

  const double excess =
      input.common_magnitude - input.genesis_threshold;
  result.acceptance_threshold =
      1.0 - std::exp(-excess / input.manifestation_scale);
  result.accepted = input.uniform_draw < result.acceptance_threshold;
  return result;
}

NativeEventAcceptanceResult evaluate_native_evaporation_acceptance(
    const NativeEvaporationAcceptanceInput& input) {
  NativeEventAcceptanceResult result;
  if (!valid_record(input.record)) {
    result.status = NativeEventCharacteristicStatus::InvalidRecord;
    return result;
  }
  if (!std::isfinite(input.common_energy_site_plus_faces)
      || input.common_energy_site_plus_faces < 0.0) {
    result.status = NativeEventCharacteristicStatus::InvalidEnergy;
    return result;
  }
  if (!std::isfinite(input.manifestation_scale)
      || !(input.manifestation_scale > 0.0)) {
    result.status = NativeEventCharacteristicStatus::InvalidScale;
    return result;
  }
  if (!std::isfinite(input.evaporation_rate)
      || input.evaporation_rate < 0.0) {
    result.status = NativeEventCharacteristicStatus::InvalidRate;
    return result;
  }
  if (!std::isfinite(input.proper_time_rate)
      || input.proper_time_rate < 0.0
      || input.proper_time_rate > 1.0) {
    result.status = NativeEventCharacteristicStatus::InvalidProperTimeRate;
    return result;
  }
  if (!valid_uniform_draw(input.uniform_draw)) {
    result.status = NativeEventCharacteristicStatus::InvalidDraw;
    return result;
  }

  result.status = NativeEventCharacteristicStatus::Valid;
  if (!input.event_processing_enabled || input.record == 0 || input.locked) {
    return result;
  }

  const double scale_squared =
      input.manifestation_scale * input.manifestation_scale;
  result.acceptance_threshold =
      std::exp(-input.common_energy_site_plus_faces / scale_squared)
      * input.evaporation_rate * input.proper_time_rate;
  result.accepted = input.uniform_draw < result.acceptance_threshold;
  return result;
}

OrderedNativeEvent classify_ordered_native_event(
    bool genesis_accepted, bool evaporation_accepted) {
  if (genesis_accepted && evaporation_accepted) {
    return OrderedNativeEvent::GenesisThenEvaporation;
  }
  if (genesis_accepted) return OrderedNativeEvent::Genesis;
  if (evaporation_accepted) return OrderedNativeEvent::Evaporation;
  return OrderedNativeEvent::None;
}

CommonRelativeScalar common_relative_scalar(double left, double right) {
  return {left + right, left - right};
}

BilateralScalar bilateral_scalar(double common, double relative) {
  return {0.5 * (common + relative), 0.5 * (common - relative)};
}

RelativeCharacteristicChart relative_characteristic_chart(
    double relative_momentum, double oriented_strain) {
  RelativeCharacteristicChart result;
  result.relative_momentum = relative_momentum;
  result.oriented_strain = oriented_strain;
  if (!std::isfinite(relative_momentum) || !std::isfinite(oriented_strain)) {
    result.status = NativeEventCharacteristicStatus::InvalidCoordinate;
    return result;
  }

  const double inverse_sqrt_two = 1.0 / std::sqrt(2.0);
  result.incoming =
      (relative_momentum + oriented_strain) * inverse_sqrt_two;
  result.outgoing =
      (relative_momentum - oriented_strain) * inverse_sqrt_two;
  result.pair_energy = 0.5 * (
      relative_momentum * relative_momentum
      + oriented_strain * oriented_strain);
  result.characteristic_energy = 0.5 * (
      result.incoming * result.incoming
      + result.outgoing * result.outgoing);
  result.energy_residual =
      result.characteristic_energy - result.pair_energy;
  result.signed_current = relative_momentum * oriented_strain;
  result.characteristic_current = 0.5 * (
      result.incoming * result.incoming
      - result.outgoing * result.outgoing);
  result.current_residual =
      result.characteristic_current - result.signed_current;
  result.status = NativeEventCharacteristicStatus::Valid;
  return result;
}

AxialC18Dispersion axial_c18_dispersion(
    double wave_number, double wave_speed_squared) {
  AxialC18Dispersion result;
  result.wave_number = wave_number;
  result.wave_speed_squared = wave_speed_squared;
  if (!std::isfinite(wave_number)) {
    result.status = NativeEventCharacteristicStatus::InvalidCoordinate;
    return result;
  }
  if (!std::isfinite(wave_speed_squared)
      || !(wave_speed_squared > 0.0)) {
    result.status = NativeEventCharacteristicStatus::InvalidWaveSpeed;
    return result;
  }

  const double sine_half = std::sin(0.5 * wave_number);
  const double sine_half_squared = sine_half * sine_half;
  result.source_eigenvalue =
      4.0 * wave_speed_squared * sine_half_squared;
  result.production_trace = 2.0 - result.source_eigenvalue;
  result.production_determinant = 1.0;
  result.sin_half_theta_squared =
      wave_speed_squared * sine_half_squared;
  result.one_cell_shift_trace = 2.0 - 4.0 * sine_half_squared;
  result.one_cell_shift_trace_defect =
      result.production_trace - result.one_cell_shift_trace;
  result.status = NativeEventCharacteristicStatus::Valid;
  return result;
}

AxialC18Dispersion production_axial_c18_dispersion(double wave_number) {
  const double c2 = ftd::ontic::C_WAVE * ftd::ontic::C_WAVE;
  return axial_c18_dispersion(wave_number, c2);
}

}  // namespace ftd::eft

