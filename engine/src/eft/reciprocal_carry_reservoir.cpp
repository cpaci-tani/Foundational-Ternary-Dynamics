#include "ftd/eft/reciprocal_carry_reservoir.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

long double pi() {
  return std::acos(-1.0L);
}

long double tau() {
  return 2.0L * pi();
}

bool finite(const ReciprocalTriplet& value) {
  return std::all_of(value.begin(), value.end(), [](double component) {
    return std::isfinite(component);
  });
}

bool principal(double value) {
  const long double extended = static_cast<long double>(value);
  return extended >= -pi() && extended < pi();
}

bool safe_add(std::int64_t left, std::int64_t right,
              std::int64_t& result) {
  if (right > 0
      && left > std::numeric_limits<std::int64_t>::max() - right) {
    return false;
  }
  if (right < 0
      && left < std::numeric_limits<std::int64_t>::min() - right) {
    return false;
  }
  result = left + right;
  return true;
}

struct WrappedValue {
  double principal = 0.0;
  std::int64_t carry = 0;
  bool valid = false;
};

WrappedValue wrap_with_carry(long double value) {
  WrappedValue result;
  if (!std::isfinite(value)) return result;

  long double raw_carry = std::floor((value + pi()) / tau());
  const long double lower = static_cast<long double>(
      std::numeric_limits<std::int64_t>::min());
  const long double upper = static_cast<long double>(
      std::numeric_limits<std::int64_t>::max());
  // Reject the two extreme converted endpoints conservatively: on MSVC,
  // long double has double precision and cannot distinguish them safely.
  if (!std::isfinite(raw_carry) || raw_carry <= lower
      || raw_carry >= upper) {
    return result;
  }

  result.carry = static_cast<std::int64_t>(raw_carry);
  long double wrapped = value
      - tau() * static_cast<long double>(result.carry);
  if (wrapped >= pi()) {
    if (result.carry == std::numeric_limits<std::int64_t>::max()) {
      return WrappedValue{};
    }
    wrapped -= tau();
    ++result.carry;
  } else if (wrapped < -pi()) {
    if (result.carry == std::numeric_limits<std::int64_t>::min()) {
      return WrappedValue{};
    }
    wrapped += tau();
    --result.carry;
  }

  result.principal = static_cast<double>(wrapped);
  result.valid = principal(result.principal);
  return result;
}

double max_abs(const ReciprocalTriplet& value) {
  return std::max({std::abs(value[0]), std::abs(value[1]),
                   std::abs(value[2])});
}

double max_abs_difference(const ReciprocalTriplet& left,
                          const ReciprocalTriplet& right) {
  return std::max({std::abs(left[0] - right[0]),
                   std::abs(left[1] - right[1]),
                   std::abs(left[2] - right[2])});
}

double band_energy(const ReciprocalTriplet& first,
                   const ReciprocalTriplet& second) {
  double energy = 0.0;
  for (std::size_t axis = 0; axis < 3; ++axis) {
    energy += 2.0 - std::cos(first[axis]) - std::cos(second[axis]);
  }
  return energy;
}

}  // namespace

ReciprocalCarryResult apply_reciprocal_carry_transaction(
    const ReciprocalCarryInput& input) {
  ReciprocalCarryResult result;
  if (!finite(input.principal_first) || !finite(input.principal_second)
      || !finite(input.opposite_increment)
      || !std::isfinite(input.momentum_scale)
      || !std::isfinite(input.tolerance)) {
    return result;
  }
  if (!(input.tolerance > 0.0)) {
    result.status = ReciprocalCarryStatus::InvalidTolerance;
    return result;
  }
  if (!(input.momentum_scale > 0.0)) {
    result.status = ReciprocalCarryStatus::InvalidMomentumScale;
    return result;
  }
  for (std::size_t axis = 0; axis < 3; ++axis) {
    if (!principal(input.principal_first[axis])
        || !principal(input.principal_second[axis])) {
      result.status = ReciprocalCarryStatus::NonPrincipalLabel;
      return result;
    }
  }

  ReciprocalTriplet reversed_first{};
  ReciprocalTriplet reversed_second{};
  ReciprocalCarryTriplet reversed_reservoir{};
  bool inverse_carries_match = true;
  for (std::size_t axis = 0; axis < 3; ++axis) {
    const long double q = static_cast<long double>(
        input.opposite_increment[axis]);
    const WrappedValue first = wrap_with_carry(
        static_cast<long double>(input.principal_first[axis]) + q);
    const WrappedValue second = wrap_with_carry(
        static_cast<long double>(input.principal_second[axis]) - q);
    if (!first.valid || !second.valid) {
      result.status = ReciprocalCarryStatus::CarryOutOfRange;
      return result;
    }

    std::int64_t carry_sum = 0;
    if (!safe_add(first.carry, second.carry, carry_sum)
        || !safe_add(input.reciprocal_reservoir[axis], carry_sum,
                     result.reciprocal_reservoir_after[axis])) {
      result.status = ReciprocalCarryStatus::ReservoirOverflow;
      return result;
    }
    result.principal_first_after[axis] = first.principal;
    result.principal_second_after[axis] = second.principal;
    result.carry_first[axis] = first.carry;
    result.carry_second[axis] = second.carry;
    result.multi_zone_increment_supported =
        result.multi_zone_increment_supported
        || std::abs(first.carry) > 1 || std::abs(second.carry) > 1;

    const long double before =
        static_cast<long double>(input.principal_first[axis])
        + static_cast<long double>(input.principal_second[axis])
        + tau() * static_cast<long double>(
            input.reciprocal_reservoir[axis]);
    const long double after =
        static_cast<long double>(first.principal)
        + static_cast<long double>(second.principal)
        + tau() * static_cast<long double>(
            result.reciprocal_reservoir_after[axis]);
    result.dimensionless_total_before[axis] = static_cast<double>(before);
    result.dimensionless_total_after[axis] = static_cast<double>(after);
    result.physical_momentum_before[axis] = input.momentum_scale
        * result.dimensionless_total_before[axis];
    result.physical_momentum_after[axis] = input.momentum_scale
        * result.dimensionless_total_after[axis];

    const WrappedValue reverse_first = wrap_with_carry(
        static_cast<long double>(first.principal) - q);
    const WrappedValue reverse_second = wrap_with_carry(
        static_cast<long double>(second.principal) + q);
    if (!reverse_first.valid || !reverse_second.valid) {
      result.status = ReciprocalCarryStatus::ReversalFailure;
      return result;
    }
    std::int64_t inverse_carry_sum = 0;
    if (!safe_add(reverse_first.carry, reverse_second.carry,
                  inverse_carry_sum)
        || !safe_add(result.reciprocal_reservoir_after[axis],
                     inverse_carry_sum, reversed_reservoir[axis])) {
      result.status = ReciprocalCarryStatus::ReservoirOverflow;
      return result;
    }
    reversed_first[axis] = reverse_first.principal;
    reversed_second[axis] = reverse_second.principal;
    inverse_carries_match = inverse_carries_match
        && reverse_first.carry == -first.carry
        && reverse_second.carry == -second.carry;
  }

  result.conservation_residual = max_abs_difference(
      result.dimensionless_total_after, result.dimensionless_total_before);
  result.reversal_residual = std::max(
      max_abs_difference(reversed_first, input.principal_first),
      max_abs_difference(reversed_second, input.principal_second));
  const bool reservoir_recovered =
      reversed_reservoir == input.reciprocal_reservoir;
  const double conservation_scale = std::max({
      1.0, max_abs(result.dimensionless_total_before),
      max_abs(result.dimensionless_total_after)});
  if (result.conservation_residual
      > input.tolerance * conservation_scale) {
    result.status = ReciprocalCarryStatus::ConservationFailure;
    return result;
  }
  if (result.reversal_residual > input.tolerance
      || !reservoir_recovered || !inverse_carries_match) {
    result.status = ReciprocalCarryStatus::ReversalFailure;
    return result;
  }

  result.band_energy_before = band_energy(
      input.principal_first, input.principal_second);
  result.band_energy_after = band_energy(
      result.principal_first_after, result.principal_second_after);
  result.band_energy_change = result.band_energy_after
      - result.band_energy_before;
  result.reciprocal_carry_update_exact = true;
  result.reservoir_increment_unique_given_branch_and_conservation = true;
  result.full_state_reversal_exact = true;
  result.periodic_band_energy_blind_to_reservoir = true;
  result.status = ReciprocalCarryStatus::Valid;
  return result;
}

}  // namespace ftd::eft
