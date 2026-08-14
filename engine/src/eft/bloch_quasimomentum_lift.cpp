#include "ftd/eft/bloch_quasimomentum_lift.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

constexpr int kMaximumFiniteRangeOrder = 4096;

double pi() {
  return std::acos(-1.0);
}

double tau() {
  return 2.0 * pi();
}

bool finite(const BlochTriplet& value) {
  return std::all_of(value.begin(), value.end(), [](double component) {
    return std::isfinite(component);
  });
}

bool principal(double value) {
  return value >= -pi() && value < pi();
}

struct WrappedAngle {
  double principal = 0.0;
  std::int64_t carry = 0;
};

WrappedAngle wrap_principal_sum(double value) {
  WrappedAngle result{value, 0};
  if (result.principal >= pi()) {
    result.principal -= tau();
    result.carry = 1;
  } else if (result.principal < -pi()) {
    result.principal += tau();
    result.carry = -1;
  }
  return result;
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

double sawtooth_weight(double value, int order) {
  double sum = 0.0;
  for (int separation = 1; separation <= order; ++separation) {
    const double sign = separation % 2 == 1 ? 1.0 : -1.0;
    sum += 2.0 * sign * std::sin(separation * value)
        / static_cast<double>(separation);
  }
  return sum;
}

double max_abs(const BlochTriplet& value) {
  return std::max({std::abs(value[0]), std::abs(value[1]),
                   std::abs(value[2])});
}

}  // namespace

BlochQuasimomentumLiftResult analyze_bloch_quasimomentum_lift(
    const BlochQuasimomentumLiftInput& input) {
  BlochQuasimomentumLiftResult result;
  if (!finite(input.principal_first) || !finite(input.principal_second)
      || !std::isfinite(input.momentum_scale)
      || !std::isfinite(input.tolerance)) {
    return result;
  }
  if (!(input.tolerance > 0.0)) {
    result.status = BlochQuasimomentumLiftStatus::InvalidTolerance;
    return result;
  }
  if (!(input.momentum_scale > 0.0)) {
    result.status = BlochQuasimomentumLiftStatus::InvalidMomentumScale;
    return result;
  }
  if (input.finite_range_order <= 0
      || input.finite_range_order > kMaximumFiniteRangeOrder) {
    result.status = BlochQuasimomentumLiftStatus::InvalidFiniteRangeOrder;
    return result;
  }
  for (int axis = 0; axis < 3; ++axis) {
    if (!principal(input.principal_first[axis])
        || !principal(input.principal_second[axis])) {
      result.status = BlochQuasimomentumLiftStatus::NonPrincipalLabel;
      return result;
    }
  }

  for (int axis = 0; axis < 3; ++axis) {
    const auto index = static_cast<std::size_t>(axis);
    const WrappedAngle wrapped = wrap_principal_sum(
        input.principal_first[index] + input.principal_second[index]);
    result.principal_sum[index] = wrapped.principal;
    result.principal_carry[index] = wrapped.carry;

    std::int64_t partial_winding = 0;
    if (!safe_add(input.winding_first[index], input.winding_second[index],
                  partial_winding)
        || !safe_add(partial_winding, wrapped.carry,
                     result.combined_winding[index])) {
      result.status = BlochQuasimomentumLiftStatus::WindingOverflow;
      return result;
    }

    result.lifted_first[index] = input.principal_first[index]
        + tau() * static_cast<double>(input.winding_first[index]);
    result.lifted_second[index] = input.principal_second[index]
        + tau() * static_cast<double>(input.winding_second[index]);
    result.lifted_sum[index] = result.lifted_first[index]
        + result.lifted_second[index];
    result.reconstructed_lifted_sum[index] = result.principal_sum[index]
        + tau() * static_cast<double>(result.combined_winding[index]);
    result.reciprocal_information[index] = tau()
        * static_cast<double>(result.combined_winding[index]);
    result.physical_momentum_candidate[index] = input.momentum_scale
        * result.lifted_sum[index];
    result.doubled_scale_momentum_candidate[index] = 2.0
        * input.momentum_scale * result.lifted_sum[index];

    result.finite_range_sawtooth_weight[index] = sawtooth_weight(
        result.principal_sum[index], input.finite_range_order);
    const double periodic_copy = sawtooth_weight(
        result.principal_sum[index] + tau(), input.finite_range_order);
    result.periodicity_residual = std::max(
        result.periodicity_residual,
        std::abs(periodic_copy
                 - result.finite_range_sawtooth_weight[index]));
    result.finite_range_branch_residual = std::max(
        result.finite_range_branch_residual,
        std::abs(result.finite_range_sawtooth_weight[index]
                 - result.principal_sum[index]));
  }

  BlochTriplet addition_difference{};
  for (int axis = 0; axis < 3; ++axis) {
    const auto index = static_cast<std::size_t>(axis);
    addition_difference[index] = result.reconstructed_lifted_sum[index]
        - result.lifted_sum[index];
  }
  result.real_addition_residual = max_abs(addition_difference);
  const double scale = std::max({
      1.0, max_abs(result.lifted_sum),
      max_abs(result.reconstructed_lifted_sum),
      max_abs(result.physical_momentum_candidate)});
  result.torus_quasimomentum_addition_exact = true;
  result.winding_reconstructs_real_addition =
      result.real_addition_residual <= input.tolerance * scale;
  result.zone_crossing_observed = std::any_of(
      result.principal_carry.begin(), result.principal_carry.end(),
      [](std::int64_t carry) { return carry != 0; });
  result.principal_only_loses_reciprocal_information =
      max_abs(result.reciprocal_information) > input.tolerance;
  result.finite_range_weight_is_periodic =
      result.periodicity_residual <= input.tolerance * scale;

  if (result.winding_reconstructs_real_addition
      && result.finite_range_weight_is_periodic) {
    result.status = BlochQuasimomentumLiftStatus::Valid;
  }
  return result;
}

}  // namespace ftd::eft
