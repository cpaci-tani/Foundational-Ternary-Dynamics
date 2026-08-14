#include "ftd/eft/relative_action_transducer.h"

#include <cmath>

namespace ftd::eft {
namespace {

bool valid_event_sign(std::int8_t sign) {
  return sign == -1 || sign == 1;
}

bool finite_pair(double first, double second) {
  return std::isfinite(first) && std::isfinite(second);
}

double pair_action(double q, double p) {
  const double radius = std::hypot(q, p);
  return 0.5 * radius * radius;
}

}  // namespace

RelativeActionPumpResult pump_relative_action(
    const RelativeActionPumpInput& input) {
  RelativeActionPumpResult result;
  result.event_sign = input.event_sign;
  result.event_energy = input.event_energy;
  result.canonical_q_before = input.canonical_q;
  result.canonical_p_before = input.canonical_p;

  if (!valid_event_sign(input.event_sign)) {
    result.status = RelativeActionTransducerStatus::InvalidEventSign;
    return result;
  }
  if (!std::isfinite(input.event_energy) || !(input.event_energy > 0.0)) {
    result.status = RelativeActionTransducerStatus::InvalidEventEnergy;
    return result;
  }
  if (!finite_pair(input.canonical_q, input.canonical_p)) {
    result.status = RelativeActionTransducerStatus::InvalidCoordinate;
    return result;
  }

  result.action_before = pair_action(input.canonical_q, input.canonical_p);
  if (!std::isfinite(result.action_before)) {
    result.status = RelativeActionTransducerStatus::InvalidCoordinate;
    return result;
  }
  if (!(result.action_before > 0.0)) {
    result.status = RelativeActionTransducerStatus::EmptyCarrier;
    return result;
  }

  const double target_action = result.action_before + input.event_energy;
  if (!std::isfinite(target_action)) {
    result.status = RelativeActionTransducerStatus::NonFiniteOutput;
    return result;
  }
  result.radial_gain = std::sqrt(target_action / result.action_before);
  const double signed_gain =
      static_cast<double>(input.event_sign) * result.radial_gain;
  result.canonical_q_after = -signed_gain * input.canonical_p;
  result.canonical_p_after = signed_gain * input.canonical_q;
  if (!finite_pair(result.canonical_q_after, result.canonical_p_after)) {
    result.status = RelativeActionTransducerStatus::NonFiniteOutput;
    return result;
  }

  result.action_after =
      pair_action(result.canonical_q_after, result.canonical_p_after);
  result.action_residual =
      result.action_after - result.action_before - input.event_energy;
  result.oriented_area =
      input.canonical_q * result.canonical_p_after
      - input.canonical_p * result.canonical_q_after;
  result.jacobian_determinant = 1.0;
  result.status = RelativeActionTransducerStatus::Valid;
  return result;
}

RelativeActionInverseResult invert_relative_action_pump(
    const RelativeActionInverseInput& input) {
  RelativeActionInverseResult result;
  if (!valid_event_sign(input.event_sign)) {
    result.status = RelativeActionTransducerStatus::InvalidEventSign;
    return result;
  }
  if (!std::isfinite(input.event_energy) || !(input.event_energy > 0.0)) {
    result.status = RelativeActionTransducerStatus::InvalidEventEnergy;
    return result;
  }
  if (!finite_pair(input.canonical_q_after, input.canonical_p_after)) {
    result.status = RelativeActionTransducerStatus::InvalidCoordinate;
    return result;
  }

  result.action_after =
      pair_action(input.canonical_q_after, input.canonical_p_after);
  if (!std::isfinite(result.action_after)) {
    result.status = RelativeActionTransducerStatus::InvalidCoordinate;
    return result;
  }
  if (!(result.action_after > input.event_energy)) {
    result.status = RelativeActionTransducerStatus::OutsideInverseImage;
    return result;
  }

  result.action_before = result.action_after - input.event_energy;
  result.inverse_radial_gain =
      std::sqrt(result.action_before / result.action_after);
  const double signed_inverse_gain =
      static_cast<double>(input.event_sign) * result.inverse_radial_gain;
  result.canonical_q_before =
      signed_inverse_gain * input.canonical_p_after;
  result.canonical_p_before =
      -signed_inverse_gain * input.canonical_q_after;
  if (!finite_pair(result.canonical_q_before, result.canonical_p_before)) {
    result.status = RelativeActionTransducerStatus::NonFiniteOutput;
    return result;
  }

  const double recovered_action =
      pair_action(result.canonical_q_before, result.canonical_p_before);
  result.inverse_residual = recovered_action - result.action_before;
  result.status = RelativeActionTransducerStatus::Valid;
  return result;
}

}  // namespace ftd::eft

