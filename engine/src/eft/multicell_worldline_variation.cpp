#include "ftd/eft/multicell_worldline_variation.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace ftd::eft {
namespace {

double component(const Vec3& value, int axis) {
  if (axis == 0) return value.x;
  if (axis == 1) return value.y;
  return value.z;
}

Vec3 axis_vector(int axis, double amount) {
  if (axis == 0) return {amount, 0.0, 0.0};
  if (axis == 1) return {0.0, amount, 0.0};
  return {0.0, 0.0, amount};
}

double max_component(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

bool finite(const std::vector<double>& values) {
  return std::all_of(values.begin(), values.end(),
      [](double value) { return std::isfinite(value); });
}

bool finite(const DualGaugePotentialSlab& slab) {
  const std::size_t count = slab.L > 0
      ? static_cast<std::size_t>(slab.L*slab.L*slab.L) : 0;
  return slab.L > 0 && std::isfinite(slab.temporal_scale)
      && slab.temporal_scale > 0.0
      && slab.A_start.L == slab.L && slab.A_end.L == slab.L
      && slab.A_start.x.size() == count
      && slab.A_start.y.size() == count
      && slab.A_start.z.size() == count
      && slab.A_end.x.size() == count
      && slab.A_end.y.size() == count
      && slab.A_end.z.size() == count
      && slab.Phi.size() == count
      && finite(slab.A_start.x) && finite(slab.A_start.y)
      && finite(slab.A_start.z) && finite(slab.A_end.x)
      && finite(slab.A_end.y) && finite(slab.A_end.z)
      && finite(slab.Phi);
}

double max_difference(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, std::abs(lhs[i]-rhs[i]));
  return result;
}

double max_difference(const MatchedFaceFlux& lhs,
                      const MatchedFaceFlux& rhs) {
  if (lhs.L != rhs.L) return INFINITY;
  return std::max({max_difference(lhs.x, rhs.x),
      max_difference(lhs.y, rhs.y), max_difference(lhs.z, rhs.z)});
}

void decompose(const Vec3& position, Coord& anchor, Vec3& remainder) {
  anchor = {static_cast<int>(std::floor(position.x)),
            static_cast<int>(std::floor(position.y)),
            static_cast<int>(std::floor(position.z))};
  remainder = {position.x-anchor.x,
               position.y-anchor.y,
               position.z-anchor.z};
}

double segment_action(const Vec3& start, const Vec3& end,
                      int charge, const DualGaugePotentialSlab& slab,
                      double coupling) {
  Coord start_anchor{};
  Coord end_anchor{};
  Vec3 start_remainder{};
  Vec3 end_remainder{};
  decompose(start, start_anchor, start_remainder);
  decompose(end, end_anchor, end_remainder);
  const auto current = make_spacetime_worldline_current(
      slab.L, start_anchor, start_remainder,
      end_anchor, end_remainder, charge, slab.temporal_scale);
  const std::vector<double> zero(
      static_cast<std::size_t>(slab.L*slab.L*slab.L), 0.0);
  const auto evaluated = evaluate_spacetime_gauge_coupling(
      current, slab, zero, zero, coupling);
  return evaluated.valid ? evaluated.interaction_action : NAN;
}

double total_action(const Vec3& previous, const Vec3& shared,
                    const Vec3& next, int charge,
                    const DualGaugePotentialSlab& previous_slab,
                    const DualGaugePotentialSlab& next_slab,
                    double coupling) {
  const double first = segment_action(
      previous, shared, charge, previous_slab, coupling);
  const double second = segment_action(
      shared, next, charge, next_slab, coupling);
  return first+second;
}

double five_point_derivative(
    const Vec3& previous, const Vec3& shared, const Vec3& next,
    int charge, const DualGaugePotentialSlab& previous_slab,
    const DualGaugePotentialSlab& next_slab, double coupling,
    const Vec3& direction, double h) {
  const double fm2 = total_action(previous, shared-direction*(2.0*h), next,
      charge, previous_slab, next_slab, coupling);
  const double fm1 = total_action(previous, shared-direction*h, next,
      charge, previous_slab, next_slab, coupling);
  const double fp1 = total_action(previous, shared+direction*h, next,
      charge, previous_slab, next_slab, coupling);
  const double fp2 = total_action(previous, shared+direction*(2.0*h), next,
      charge, previous_slab, next_slab, coupling);
  return (fm2-8.0*fm1+8.0*fp1-fp2)/(12.0*h);
}

struct OneSidedPair {
  double forward = NAN;
  double backward = NAN;
};

OneSidedPair one_sided_derivatives(
    const Vec3& previous, const Vec3& shared, const Vec3& next,
    int charge, const DualGaugePotentialSlab& previous_slab,
    const DualGaugePotentialSlab& next_slab, double coupling,
    const Vec3& direction, double h, double f0) {
  const double fm2 = total_action(previous, shared-direction*(2.0*h), next,
      charge, previous_slab, next_slab, coupling);
  const double fm1 = total_action(previous, shared-direction*h, next,
      charge, previous_slab, next_slab, coupling);
  const double fp1 = total_action(previous, shared+direction*h, next,
      charge, previous_slab, next_slab, coupling);
  const double fp2 = total_action(previous, shared+direction*(2.0*h), next,
      charge, previous_slab, next_slab, coupling);
  return {(-3.0*f0+4.0*fp1-fp2)/(2.0*h),
          (3.0*f0-4.0*fm1+fm2)/(2.0*h)};
}

struct BreakData {
  int unique_breaks = 0;
  int maximum_multiplicity = 0;
};

BreakData internal_breaks(const Vec3& start, const Vec3& end) {
  struct Entry { double tau = 0.0; };
  std::vector<Entry> entries;
  for (int axis = 0; axis < 3; ++axis) {
    const double a = component(start, axis);
    const double b = component(end, axis);
    const double delta = b-a;
    if (delta == 0.0) continue;
    const int first = static_cast<int>(std::floor(std::min(a, b)))+1;
    const int last = static_cast<int>(std::ceil(std::max(a, b)))-1;
    for (int plane = first; plane <= last; ++plane) {
      const double tau = (static_cast<double>(plane)-a)/delta;
      if (tau > 0.0 && tau < 1.0) entries.push_back({tau});
    }
  }
  std::sort(entries.begin(), entries.end(),
      [](const Entry& lhs, const Entry& rhs) { return lhs.tau < rhs.tau; });
  BreakData result;
  // FTD-0532 registers simultaneous signed-cubic crossings at 1e-12.  Use
  // that geometric equivalence tolerance here rather than demanding bitwise
  // equality of independently reconstructed crossing parameters.
  constexpr double tolerance = 1e-12;
  for (std::size_t i = 0; i < entries.size();) {
    std::size_t j = i+1;
    while (j < entries.size()
           && std::abs(entries[j].tau-entries[i].tau) <= tolerance) ++j;
    ++result.unique_breaks;
    result.maximum_multiplicity = std::max(
        result.maximum_multiplicity, static_cast<int>(j-i));
    i = j;
  }
  return result;
}

}  // namespace

MulticellWorldlineVariationResult
evaluate_multicell_worldline_variation(
    const Vec3& previous_position, const Vec3& shared_position,
    const Vec3& next_position, int charge,
    const DualGaugePotentialSlab& previous_slab,
    const DualGaugePotentialSlab& next_slab,
    double coupling, double largest_step) {
  MulticellWorldlineVariationResult result;
  result.L = previous_slab.L;
  result.charge = charge;
  result.coupling = coupling;
  result.temporal_scale = previous_slab.temporal_scale;
  result.largest_step = largest_step;
  result.previous_position = previous_position;
  result.shared_position = shared_position;
  result.next_position = next_position;
  if ((charge != -1 && charge != 1) || !std::isfinite(coupling)
      || !std::isfinite(largest_step) || !(largest_step > 0.0)
      || !finite(previous_position) || !finite(shared_position)
      || !finite(next_position) || !finite(previous_slab)
      || !finite(next_slab) || previous_slab.L != next_slab.L
      || previous_slab.temporal_scale != next_slab.temporal_scale
      || (shared_position-previous_position).mag()
          >= previous_slab.temporal_scale
      || (next_position-shared_position).mag()
          >= next_slab.temporal_scale) return result;

  result.connection_join_residual = max_difference(
      previous_slab.A_end, next_slab.A_start);
  if (result.connection_join_residual > 1e-12) return result;
  const BreakData previous_breaks = internal_breaks(
      previous_position, shared_position);
  const BreakData next_breaks = internal_breaks(
      shared_position, next_position);
  result.previous_internal_breaks = previous_breaks.unique_breaks;
  result.next_internal_breaks = next_breaks.unique_breaks;
  result.maximum_simultaneous_crossing_multiplicity = std::max(
      previous_breaks.maximum_multiplicity,
      next_breaks.maximum_multiplicity);
  result.interaction_action = total_action(
      previous_position, shared_position, next_position, charge,
      previous_slab, next_slab, coupling);
  if (!std::isfinite(result.interaction_action)) return result;

  result.minimum_resolved_one_sided_gap_ratio = INFINITY;
  for (int level = 0; level < 4; ++level) {
    const double h = std::ldexp(largest_step, -level);
    Vec3 gradient{};
    double maximum_gap = 0.0;
    for (int axis = 0; axis < 3; ++axis) {
      const Vec3 direction = axis_vector(axis, 1.0);
      const double derivative = five_point_derivative(
          previous_position, shared_position, next_position, charge,
          previous_slab, next_slab, coupling, direction, h);
      if (axis == 0) gradient.x = derivative;
      if (axis == 1) gradient.y = derivative;
      if (axis == 2) gradient.z = derivative;
      const OneSidedPair sides = one_sided_derivatives(
          previous_position, shared_position, next_position, charge,
          previous_slab, next_slab, coupling, direction, h,
          result.interaction_action);
      maximum_gap = std::max(maximum_gap,
          std::abs(sides.forward-sides.backward));
    }
    result.centered_gradient[static_cast<std::size_t>(level)] = gradient;
    result.maximum_one_sided_gap[static_cast<std::size_t>(level)] =
        maximum_gap;
    if (!finite(gradient) || !std::isfinite(maximum_gap)) return result;
    if (level > 0 && maximum_gap > 1e-14) {
      result.minimum_resolved_one_sided_gap_ratio = std::min(
          result.minimum_resolved_one_sided_gap_ratio,
          result.maximum_one_sided_gap[static_cast<std::size_t>(level-1)]
              / maximum_gap);
    }
  }
  result.interaction_impulse = result.centered_gradient[3];
  result.final_centered_convergence_residual = max_component(
      result.centered_gradient[3]-result.centered_gradient[2]);

  const double fine_h = std::ldexp(largest_step, -3);
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        const int shell = dx*dx+dy*dy+dz*dz;
        if (shell == 0) continue;
        const double inverse_norm = 1.0/std::sqrt(static_cast<double>(shell));
        const Vec3 direction{dx*inverse_norm, dy*inverse_norm,
                             dz*inverse_norm};
        const double directional = five_point_derivative(
            previous_position, shared_position, next_position, charge,
            previous_slab, next_slab, coupling, direction, fine_h);
        result.maximum_directional_linearity_residual = std::max(
            result.maximum_directional_linearity_residual,
            std::abs(directional-result.interaction_impulse.dot(direction)));
      }
    }
  }
  if (!std::isfinite(result.minimum_resolved_one_sided_gap_ratio))
    result.minimum_resolved_one_sided_gap_ratio = INFINITY;
  result.valid = finite(result.interaction_impulse)
      && std::isfinite(result.final_centered_convergence_residual)
      && std::isfinite(result.maximum_directional_linearity_residual);
  return result;
}

}  // namespace ftd::eft
