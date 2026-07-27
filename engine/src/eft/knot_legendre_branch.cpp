#include "ftd/eft/knot_legendre_branch.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace ftd::eft {
namespace {

double max_component(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double max_difference(const Vec3& lhs, const Vec3& rhs) {
  return std::max({std::abs(lhs.x - rhs.x),
                   std::abs(lhs.y - rhs.y),
                   std::abs(lhs.z - rhs.z)});
}

double norm(const Vec3& value) {
  return std::sqrt(value.x * value.x
      + value.y * value.y + value.z * value.z);
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

void make_gauge(const DualGaugePotentialSlab& indexing,
                std::vector<double>& chi_start,
                std::vector<double>& chi_end) {
  const int L = indexing.L;
  chi_start.assign(static_cast<std::size_t>(L * L * L), 0.0);
  chi_end = chi_start;
  constexpr double pi = 3.141592653589793238462643383279502884;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = indexing.index(x, y, z);
        const double px = 2.0 * pi * x / L;
        const double py = 2.0 * pi * y / L;
        const double pz = 2.0 * pi * z / L;
        chi_start[static_cast<std::size_t>(i)] =
            0.031 * std::sin(px + py) + 0.011 * std::cos(pz);
        chi_end[static_cast<std::size_t>(i)] =
            -0.019 * std::cos(py + pz) + 0.023 * std::sin(px);
      }
    }
  }
}

}  // namespace

KnotLegendreBranchResult analyze_knot_legendre_branches(
    int L,
    Coord knot,
    int polarity,
    double epsilon,
    double rest_energy,
    double c_speed,
    double temporal_scale,
    double coupling,
    const Vec3& external_bias) {
  KnotLegendreBranchResult result;
  result.L = L;
  result.polarity = polarity;
  result.knot = knot;
  result.external_bias = external_bias;
  result.epsilon = epsilon;
  if (L < 5 || (polarity != -1 && polarity != 1)
      || knot.x < 2 || knot.x >= L - 2
      || knot.y < 2 || knot.y >= L - 2
      || knot.z < 2 || knot.z >= L - 2
      || !std::isfinite(epsilon) || epsilon <= 0.0
      || epsilon >= 0.1 || !std::isfinite(rest_energy)
      || rest_energy <= 0.0 || !std::isfinite(c_speed)
      || c_speed <= 0.0 || !std::isfinite(temporal_scale)
      || temporal_scale <= 0.0 || !std::isfinite(coupling)
      || !finite(external_bias)) {
    return result;
  }

  DualGaugePotentialSlab slab(L, temporal_scale);
  const double self_scale = static_cast<double>(polarity) / 6.0;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = slab.index(x, y, z);
        const double ex = external_bias.x
            + (x == knot.x ? self_scale
               : (x == knot.x - 1 ? -self_scale : 0.0));
        const double ey = external_bias.y
            + (y == knot.y ? self_scale
               : (y == knot.y - 1 ? -self_scale : 0.0));
        const double ez = external_bias.z
            + (z == knot.z ? self_scale
               : (z == knot.z - 1 ? -self_scale : 0.0));
        slab.A_end.x[static_cast<std::size_t>(i)] = -temporal_scale * ex;
        slab.A_end.y[static_cast<std::size_t>(i)] = -temporal_scale * ey;
        slab.A_end.z[static_cast<std::size_t>(i)] = -temporal_scale * ez;
      }
    }
  }
  const auto electric = slab_electric_field(slab);
  result.gauss_residual = std::abs(
      divergence_at(electric, knot.x, knot.y, knot.z) - polarity);
  MatchedFaceFlux bias_field(L);
  std::fill(bias_field.x.begin(), bias_field.x.end(), external_bias.x);
  std::fill(bias_field.y.begin(), bias_field.y.end(), external_bias.y);
  std::fill(bias_field.z.begin(), bias_field.z.end(), external_bias.z);
  result.bias_divergence_residual = std::abs(
      divergence_at(bias_field, knot.x, knot.y, knot.z));

  std::vector<double> chi_start;
  std::vector<double> chi_end;
  make_gauge(slab, chi_start, chi_end);
  const auto gauged = gauge_transform_slab(slab, chi_start, chi_end);

  int branch_index = 0;
  double minimum_magnitude = INFINITY;
  double maximum_magnitude = 0.0;
  for (int sx : {-1, +1}) {
    for (int sy : {-1, +1}) {
      for (int sz : {-1, +1}) {
        auto& branch = result.branches[static_cast<std::size_t>(branch_index++)];
        branch.incident_sign = {sx, sy, sz};
        branch.local_electric = {
            external_bias.x + self_scale * sx,
            external_bias.y + self_scale * sy,
            external_bias.z + self_scale * sz};
        const double impulse_scale = coupling * polarity
            * temporal_scale / 2.0;
        branch.analytic_momentum = branch.local_electric * impulse_scale;
        branch.displacement = free_displacement_from_momentum(
            branch.analytic_momentum, rest_energy,
            c_speed, temporal_scale);
        branch.sign_consistent =
            branch.displacement.x * sx > 0.0
            && branch.displacement.y * sy > 0.0
            && branch.displacement.z * sz > 0.0;
        if (!branch.sign_consistent) continue;
        ++result.sign_consistent_count;

        const Vec3 start{
            knot.x + epsilon * sx,
            knot.y + epsilon * sy,
            knot.z + epsilon * sz};
        branch.endpoint = start + branch.displacement;
        const auto base = evaluate_discrete_legendre_worldline(
            start, branch.endpoint, polarity, rest_energy,
            c_speed, slab, coupling);
        const auto transformed = evaluate_discrete_legendre_worldline(
            start, branch.endpoint, polarity, rest_energy,
            c_speed, gauged, coupling);
        if (!base.valid || !transformed.valid) continue;
        branch.initial_kinetic_residual = max_component(base.kinetic_start);
        branch.analytic_momentum_residual = max_difference(
            base.d2_matter, branch.analytic_momentum);
        branch.gauge_kinetic_residual = std::max(
            max_difference(base.kinetic_start, transformed.kinetic_start),
            max_difference(base.kinetic_end, transformed.kinetic_end));
        branch.valid = std::isfinite(branch.initial_kinetic_residual)
            && std::isfinite(branch.analytic_momentum_residual)
            && std::isfinite(branch.gauge_kinetic_residual);
        if (!branch.valid) continue;
        ++result.solved_branch_count;
        result.worst_initial_kinetic_residual = std::max(
            result.worst_initial_kinetic_residual,
            branch.initial_kinetic_residual);
        result.worst_analytic_momentum_residual = std::max(
            result.worst_analytic_momentum_residual,
            branch.analytic_momentum_residual);
        result.worst_gauge_kinetic_residual = std::max(
            result.worst_gauge_kinetic_residual,
            branch.gauge_kinetic_residual);
        const double magnitude = norm(branch.displacement);
        minimum_magnitude = std::min(minimum_magnitude, magnitude);
        maximum_magnitude = std::max(maximum_magnitude, magnitude);
      }
    }
  }
  result.displacement_orbit_residual = result.solved_branch_count > 0
      ? maximum_magnitude - minimum_magnitude : INFINITY;
  result.valid = result.gauss_residual <= 1e-12
      && result.bias_divergence_residual <= 1e-12
      && result.solved_branch_count == result.sign_consistent_count;
  return result;
}

}  // namespace ftd::eft
