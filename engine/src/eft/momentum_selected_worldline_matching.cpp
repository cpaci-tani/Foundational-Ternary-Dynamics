#include "ftd/eft/momentum_selected_worldline_matching.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace ftd::eft {
namespace {

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

double max_difference(const Vec3& lhs, const Vec3& rhs) {
  return std::max({std::abs(lhs.x - rhs.x),
                   std::abs(lhs.y - rhs.y),
                   std::abs(lhs.z - rhs.z)});
}

bool duplicate_endpoints(const std::vector<Vec3>& endpoints,
                         double tolerance) {
  for (std::size_t i = 0; i < endpoints.size(); ++i) {
    for (std::size_t j = i + 1; j < endpoints.size(); ++j) {
      if (max_difference(endpoints[i], endpoints[j]) <= tolerance) {
        return true;
      }
    }
  }
  return false;
}

}  // namespace

MomentumSelectedMatching match_free_worldline_endpoints(
    int L,
    const std::vector<PhaseSpaceCarrier>& carriers,
    const std::vector<Vec3>& unordered_endpoints,
    double rest_energy,
    double c_speed,
    double dt,
    double tolerance) {
  MomentumSelectedMatching result;
  result.carrier_count = static_cast<int>(carriers.size());
  const std::size_t count = carriers.size();
  if (L < 3 || count == 0 || count > 8
      || unordered_endpoints.size() != count
      || !std::isfinite(rest_energy) || rest_energy <= 0.0
      || !std::isfinite(c_speed) || c_speed <= 0.0
      || !std::isfinite(dt) || dt <= 0.0
      || !std::isfinite(tolerance) || tolerance < 0.0) {
    return result;
  }
  for (const auto& carrier : carriers) {
    if ((carrier.charge != -1 && carrier.charge != +1)
        || !finite(carrier.start_position)
        || !finite(carrier.kinetic_momentum)) {
      return result;
    }
  }
  for (const auto& endpoint : unordered_endpoints) {
    if (!finite(endpoint)) return result;
  }
  if (duplicate_endpoints(unordered_endpoints, tolerance)) {
    result.collision_rule_required = true;
    return result;
  }

  const double temporal_scale = c_speed * dt;
  const DualGaugePotentialSlab zero(L, temporal_scale);
  std::vector<int> permutation(count, 0);
  std::iota(permutation.begin(), permutation.end(), 0);
  double best = INFINITY;
  double second = INFINITY;
  std::vector<int> best_assignment;
  do {
    ++result.permutations_evaluated;
    bool candidate_valid = true;
    double residual = 0.0;
    for (std::size_t i = 0; i < count; ++i) {
      const auto segment = evaluate_discrete_legendre_worldline(
          carriers[i].start_position,
          unordered_endpoints[static_cast<std::size_t>(permutation[i])],
          carriers[i].charge, rest_energy, c_speed, zero, 0.0);
      if (!segment.valid) {
        candidate_valid = false;
        break;
      }
      residual = std::max(
          residual,
          max_difference(segment.kinetic_start,
                         carriers[i].kinetic_momentum));
    }
    if (!candidate_valid) continue;
    ++result.valid_permutations;
    if (residual <= tolerance) ++result.exact_match_count;
    if (residual < best) {
      second = best;
      best = residual;
      best_assignment = permutation;
    } else if (residual < second) {
      second = residual;
    }
  } while (std::next_permutation(permutation.begin(), permutation.end()));

  result.best_residual = best;
  result.second_best_residual = second;
  result.residual_gap = second - best;
  result.assignment = best_assignment;
  result.valid = result.exact_match_count == 1
      && best <= tolerance && std::isfinite(second)
      && result.residual_gap > tolerance
      && result.assignment.size() == count;
  return result;
}

std::vector<ShapeWorldline> worldlines_from_matching(
    const std::vector<PhaseSpaceCarrier>& carriers,
    const std::vector<Vec3>& unordered_endpoints,
    const MomentumSelectedMatching& matching) {
  std::vector<ShapeWorldline> result;
  if (!matching.valid || carriers.size() != unordered_endpoints.size()
      || matching.assignment.size() != carriers.size()) {
    return result;
  }
  result.reserve(carriers.size());
  for (std::size_t i = 0; i < carriers.size(); ++i) {
    const int endpoint = matching.assignment[i];
    if (endpoint < 0
        || endpoint >= static_cast<int>(unordered_endpoints.size())) {
      return {};
    }
    result.push_back({carriers[i].start_position,
                      unordered_endpoints[static_cast<std::size_t>(endpoint)],
                      carriers[i].charge});
  }
  return result;
}

}  // namespace ftd::eft
