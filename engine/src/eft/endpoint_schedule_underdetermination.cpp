#include "ftd/eft/endpoint_schedule_underdetermination.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace ftd::eft {
namespace {

double norm(const Vec3& value) {
  return std::sqrt(value.mag2());
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

double schedule(double tau, double epsilon) {
  const double f = tau*tau*(1.0-tau)*(1.0-tau);
  return tau+epsilon*f;
}

double schedule_derivative(double tau, double epsilon) {
  return 1.0+2.0*epsilon*tau*(1.0-tau)*(1.0-2.0*tau);
}

struct IntegratedMoments {
  long double temporal = 0.0L;
  long double start = 0.0L;
  long double end = 0.0L;
  long double total = 0.0L;
};

IntegratedMoments integrate(double epsilon) {
  constexpr std::array<long double, 8> nodes{{
      -0.960289856497536231683560868569L,
      -0.796666477413626739591553936476L,
      -0.525532409916328985817739049189L,
      -0.183434642495649804939476142360L,
       0.183434642495649804939476142360L,
       0.525532409916328985817739049189L,
       0.796666477413626739591553936476L,
       0.960289856497536231683560868569L}};
  constexpr std::array<long double, 8> weights{{
      0.101228536290376259152531354310L,
      0.222381034453374470544355994426L,
      0.313706645877887287337962201987L,
      0.362683783378361982965150449277L,
      0.362683783378361982965150449277L,
      0.313706645877887287337962201987L,
      0.222381034453374470544355994426L,
      0.101228536290376259152531354310L}};
  IntegratedMoments result;
  for (std::size_t i = 0; i < nodes.size(); ++i) {
    const long double tau = 0.5L*(1.0L+nodes[i]);
    const long double w = 0.5L*weights[i];
    const long double s = schedule(
        static_cast<double>(tau),epsilon);
    const long double derivative = schedule_derivative(
        static_cast<double>(tau),epsilon);
    result.temporal += w*s;
    result.start += w*(1.0L-tau)*derivative;
    result.end += w*tau*derivative;
    result.total += w*derivative;
  }
  return result;
}

}  // namespace

EndpointScheduleUnderdeterminationResult
evaluate_endpoint_schedule_underdetermination(
    double displacement,
    double epsilon,
    const Vec3& raw_direction,
    int charge) {
  EndpointScheduleUnderdeterminationResult result;
  result.charge = charge;
  result.displacement = displacement;
  result.epsilon = epsilon;
  if (!(displacement > 0.0) || !std::isfinite(displacement)
      || !std::isfinite(epsilon) || !(std::abs(epsilon) <= 0.5)
      || epsilon == 0.0 || (charge != -1 && charge != 1)
      || !finite(raw_direction) || !(raw_direction.mag2() > 0.0))
    return result;
  result.direction = raw_direction*(1.0/std::sqrt(raw_direction.mag2()));
  result.monotonicity_margin =
      1.0-std::abs(epsilon)/(3.0*std::sqrt(3.0));

  result.endpoint_position_residual = std::max(
      std::abs(schedule(0.0,epsilon)),
      std::abs(schedule(1.0,epsilon)-1.0));
  result.endpoint_derivative_residual = std::max(
      std::abs(schedule_derivative(0.0,epsilon)-1.0),
      std::abs(schedule_derivative(1.0,epsilon)-1.0));
  result.midpoint_derivative_residual = std::abs(
      schedule_derivative(0.5,epsilon)-1.0);

  const IntegratedMoments base = integrate(0.0);
  const IntegratedMoments deformed = integrate(epsilon);
  const long double scale = static_cast<long double>(charge)*displacement;
  const long double temporal_difference =
      scale*(deformed.temporal-base.temporal);
  const long double start_difference =
      scale*(deformed.start-base.start);
  const long double end_difference =
      scale*(deformed.end-base.end);
  const long double total_difference =
      scale*(deformed.total-base.total);
  result.temporal_first_moment_difference = result.direction
      *static_cast<double>(temporal_difference);
  result.start_current_difference = result.direction
      *static_cast<double>(start_difference);
  result.end_current_difference = result.direction
      *static_cast<double>(end_difference);
  result.total_current_difference = result.direction
      *static_cast<double>(total_difference);

  const Vec3 expected = result.direction
      *(charge*displacement*epsilon/30.0);
  result.analytic_moment_residual = std::max({
      norm(result.temporal_first_moment_difference-expected),
      norm(result.start_current_difference-expected),
      norm(result.end_current_difference+expected)});
  result.split_recombination_residual = norm(
      result.start_current_difference+result.end_current_difference
      -result.total_current_difference);
  result.schedule_split_norm = norm(expected);

  const Vec3 reverse_direction = result.direction*(-1.0);
  const Vec3 reverse_temporal = reverse_direction
      *(charge*displacement*(-epsilon)/30.0);
  const Vec3 reverse_start = reverse_temporal;
  const Vec3 reverse_end = reverse_temporal*(-1.0);
  result.reversal_residual = std::max({
      norm(reverse_temporal-result.temporal_first_moment_difference),
      norm(reverse_start+result.end_current_difference),
      norm(reverse_end+result.start_current_difference)});

  result.valid = result.monotonicity_margin > 0.0
      && result.endpoint_position_residual <= 1e-14
      && result.endpoint_derivative_residual <= 1e-14
      && result.midpoint_derivative_residual <= 1e-14
      && norm(result.total_current_difference) <= 1e-14
      && result.analytic_moment_residual <= 1e-14
      && result.split_recombination_residual <= 1e-14
      && result.reversal_residual <= 1e-14;
  return result;
}

}  // namespace ftd::eft
