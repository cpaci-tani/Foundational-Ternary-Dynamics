/** FTD-0860 isolated relative action/orientation transducer verifier. */

#include "ftd/eft/relative_action_transducer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>

namespace {

using namespace ftd::eft;

int failures = 0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

bool close(double first, double second, double tolerance = 1e-12) {
  return std::abs(first - second)
      <= tolerance * std::max({1.0, std::abs(first), std::abs(second)});
}

RelativeActionPumpInput make_input(
    std::int8_t sign, double energy, double q, double p) {
  RelativeActionPumpInput input;
  input.event_sign = sign;
  input.event_energy = energy;
  input.canonical_q = q;
  input.canonical_p = p;
  return input;
}

}  // namespace

int main() {
  const auto positive = pump_relative_action(make_input(1, 1.75, 3.0, 4.0));
  check("positive branch is valid", positive.valid());
  check("input action is twelve and one half", close(positive.action_before, 12.5));
  check("event energy is added exactly",
      close(positive.action_after, positive.action_before + 1.75));
  check("action residual closes", close(positive.action_residual, 0.0));
  check("positive sign gives positive oriented area", positive.oriented_area > 0.0);
  check("reference Jacobian is one", close(positive.jacobian_determinant, 1.0));
  check("quarter turn makes input and output orthogonal",
      close(
          3.0 * positive.canonical_q_after
          + 4.0 * positive.canonical_p_after,
          0.0));

  const auto negative = pump_relative_action(make_input(-1, 1.75, 3.0, 4.0));
  check("negative branch is valid", negative.valid());
  check("negative sign gives negative oriented area", negative.oriented_area < 0.0);
  check("opposite signs give opposite quarter turns",
      close(negative.canonical_q_after, -positive.canonical_q_after)
      && close(negative.canonical_p_after, -positive.canonical_p_after));

  RelativeActionInverseInput inverse_input;
  inverse_input.event_sign = positive.event_sign;
  inverse_input.event_energy = positive.event_energy;
  inverse_input.canonical_q_after = positive.canonical_q_after;
  inverse_input.canonical_p_after = positive.canonical_p_after;
  const auto inverse = invert_relative_action_pump(inverse_input);
  check("known-event inverse is valid", inverse.valid());
  check("known-event inverse recovers q", close(inverse.canonical_q_before, 3.0));
  check("known-event inverse recovers p", close(inverse.canonical_p_before, 4.0));
  check("known-event inverse closes its residual", close(inverse.inverse_residual, 0.0));

  const auto collision_plus =
      pump_relative_action(make_input(1, 0.75, 2.0, -1.0));
  const auto collision_minus =
      pump_relative_action(make_input(-1, 0.75, -2.0, 1.0));
  check("opposite sign and phase inputs are both valid",
      collision_plus.valid() && collision_minus.valid());
  check("unlabelled output has the exact sign collision",
      close(collision_plus.canonical_q_after, collision_minus.canonical_q_after)
      && close(collision_plus.canonical_p_after, collision_minus.canonical_p_after));

  const double action_a = 2.0;
  const double energy_a = 3.0;
  const double action_b = action_a + energy_a / 2.0;
  const double energy_b = energy_a / 2.0;
  const auto energy_collision_a = pump_relative_action(
      make_input(1, energy_a, std::sqrt(2.0 * action_a), 0.0));
  const auto energy_collision_b = pump_relative_action(
      make_input(1, energy_b, std::sqrt(2.0 * action_b), 0.0));
  check("two action-energy decompositions are valid",
      energy_collision_a.valid() && energy_collision_b.valid());
  check("unlabelled output conflates prior action and event energy",
      close(
          energy_collision_a.canonical_q_after,
          energy_collision_b.canonical_q_after)
      && close(
          energy_collision_a.canonical_p_after,
          energy_collision_b.canonical_p_after));

  const auto forward_time =
      pump_relative_action(make_input(1, 0.5, 1.25, -0.75));
  const auto reversed_input =
      pump_relative_action(make_input(-1, 0.5, 1.25, 0.75));
  check("time-reversal pair is valid",
      forward_time.valid() && reversed_input.valid());
  check("time reversal flips momentum and event orientation",
      close(reversed_input.canonical_q_after, forward_time.canonical_q_after)
      && close(reversed_input.canonical_p_after, -forward_time.canonical_p_after));

  check("empty carrier fails closed",
      pump_relative_action(make_input(1, 1.0, 0.0, 0.0)).status
          == RelativeActionTransducerStatus::EmptyCarrier);
  check("invalid event sign fails closed",
      pump_relative_action(make_input(0, 1.0, 1.0, 0.0)).status
          == RelativeActionTransducerStatus::InvalidEventSign);
  check("nonpositive energy fails closed",
      pump_relative_action(make_input(1, 0.0, 1.0, 0.0)).status
          == RelativeActionTransducerStatus::InvalidEventEnergy);
  check("nonfinite coordinate fails closed",
      pump_relative_action(make_input(
          1, 1.0, std::numeric_limits<double>::infinity(), 0.0)).status
          == RelativeActionTransducerStatus::InvalidCoordinate);

  RelativeActionInverseInput outside;
  outside.event_sign = 1;
  outside.event_energy = 1.0;
  outside.canonical_q_after = 1.0;
  outside.canonical_p_after = 0.0;
  check("action below B is outside the nonzero inverse image",
      invert_relative_action_pump(outside).status
          == RelativeActionTransducerStatus::OutsideInverseImage);

  std::cout << "FTD-0860 relative action transducer EFT: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  std::cout << "scope=NONZERO_CARRIER_LOSSY_ACTION_PUMP\n";
  std::cout << "empty_carrier=REJECTED\n";
  std::cout << "faithful_signed_history=SEPARATE_RAIL_REQUIRED\n";
  std::cout << "production_integration=NONE\n";
  return failures == 0 ? 0 : 1;
}
