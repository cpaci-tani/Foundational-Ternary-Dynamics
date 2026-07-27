#include "ftd/eft/centered_knot_trace.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {
namespace {

double max_difference(const Vec3& lhs, const Vec3& rhs) {
  return std::max({std::abs(lhs.x - rhs.x),
                   std::abs(lhs.y - rhs.y),
                   std::abs(lhs.z - rhs.z)});
}

}  // namespace

CenteredKnotTrace evaluate_centered_knot_trace(
    const MatchedFaceFlux& electric,
    Coord site) {
  CenteredKnotTrace result;
  result.L = electric.L;
  result.site = site;
  const std::size_t count = electric.L > 0
      ? static_cast<std::size_t>(electric.L * electric.L * electric.L) : 0;
  if (electric.L <= 0 || electric.x.size() != count
      || electric.y.size() != count || electric.z.size() != count) {
    return result;
  }
  const int i = electric.index(site.x, site.y, site.z);
  const int ix = electric.index(site.x - 1, site.y, site.z);
  const int iy = electric.index(site.x, site.y - 1, site.z);
  const int iz = electric.index(site.x, site.y, site.z - 1);
  result.outgoing = {
      electric.x[static_cast<std::size_t>(i)],
      electric.y[static_cast<std::size_t>(i)],
      electric.z[static_cast<std::size_t>(i)]};
  result.incoming = {
      electric.x[static_cast<std::size_t>(ix)],
      electric.y[static_cast<std::size_t>(iy)],
      electric.z[static_cast<std::size_t>(iz)]};
  result.centered = (result.outgoing + result.incoming) * 0.5;
  result.divergence = (result.outgoing.x - result.incoming.x)
      + (result.outgoing.y - result.incoming.y)
      + (result.outgoing.z - result.incoming.z);

  for (double& weight : result.invariant_weights) weight = 1.0 / 8.0;
  double weight_sum = 0.0;
  Vec3 average{};
  int index = 0;
  for (int sx : {-1, +1}) {
    for (int sy : {-1, +1}) {
      for (int sz : {-1, +1}) {
        const Vec3 trace{
            sx > 0 ? result.outgoing.x : result.incoming.x,
            sy > 0 ? result.outgoing.y : result.incoming.y,
            sz > 0 ? result.outgoing.z : result.incoming.z};
        const double weight = result.invariant_weights[
            static_cast<std::size_t>(index++)];
        average += trace * weight;
        weight_sum += weight;
      }
    }
  }
  result.incident_cell_average = average;
  result.incident_average_residual = max_difference(
      result.centered, result.incident_cell_average);
  result.weight_sum_residual = std::abs(weight_sum - 1.0);
  result.valid = std::isfinite(result.divergence)
      && result.incident_average_residual <= 1e-12
      && result.weight_sum_residual <= 1e-12;
  return result;
}

}  // namespace ftd::eft
