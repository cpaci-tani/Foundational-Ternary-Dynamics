#include "ftd/eft/centered_trace_work.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace ftd::eft {
namespace {

long double dot(const std::vector<double>& lhs,
                const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return NAN;
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    result += static_cast<long double>(lhs[i]) * rhs[i];
  }
  return result;
}

long double dot(const MatchedFaceFlux& lhs,
                const MatchedFaceFlux& rhs) {
  return dot(lhs.x, rhs.x) + dot(lhs.y, rhs.y) + dot(lhs.z, rhs.z);
}

double energy(const MatchedFaceFlux& field) {
  return 0.5 * static_cast<double>(dot(field, field));
}

MatchedFaceFlux segment_flux(const FaceCurrentSegment& segment) {
  MatchedFaceFlux result(segment.L);
  result.x = segment.current_x;
  result.y = segment.current_y;
  result.z = segment.current_z;
  return result;
}

void add_scaled(MatchedFaceFlux& target,
                const MatchedFaceFlux& value,
                double scale) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale * value.x[i];
    target.y[i] += scale * value.y[i];
    target.z[i] += scale * value.z[i];
  }
}

double max_difference(const MatchedFaceFlux& lhs,
                      const MatchedFaceFlux& rhs) {
  double residual = 0.0;
  for (std::size_t i = 0; i < lhs.x.size(); ++i) {
    residual = std::max({residual,
        std::abs(lhs.x[i] - rhs.x[i]),
        std::abs(lhs.y[i] - rhs.y[i]),
        std::abs(lhs.z[i] - rhs.z[i])});
  }
  return residual;
}

}  // namespace

CenteredTraceWorkResult evaluate_centered_trace_work(
    const MatchedFaceFlux& midpoint_electric,
    Coord site,
    const Vec3& displacement,
    int charge,
    double coupling) {
  CenteredTraceWorkResult result;
  result.L = midpoint_electric.L;
  result.charge = charge;
  result.site = site;
  result.displacement = displacement;
  result.coupling = coupling;
  if ((charge != -1 && charge != 1) || midpoint_electric.L <= 0
      || !std::isfinite(displacement.x)
      || !std::isfinite(displacement.y)
      || !std::isfinite(displacement.z)
      || std::abs(displacement.x) >= 1.0
      || std::abs(displacement.y) >= 1.0
      || std::abs(displacement.z) >= 1.0
      || !std::isfinite(coupling)) {
    return result;
  }
  const auto trace = evaluate_centered_knot_trace(
      midpoint_electric, site);
  if (!trace.valid) return result;
  result.jump = trace.outgoing - trace.incoming;

  const FaceCurrentSegment forward = make_face_current_segment(
      midpoint_electric.L, site, {}, site, displacement, charge);
  const FaceCurrentSegment reverse = make_face_current_segment(
      midpoint_electric.L, site, displacement, site, {}, charge);
  if (!forward.valid || !reverse.valid) return result;
  const MatchedFaceFlux current = segment_flux(forward);
  const MatchedFaceFlux reverse_current = segment_flux(reverse);
  result.field_work = coupling * static_cast<double>(
      dot(current, midpoint_electric));
  result.centered_work = coupling * charge
      * (trace.centered.x * displacement.x
         + trace.centered.y * displacement.y
         + trace.centered.z * displacement.z);
  result.omitted_work = result.field_work - result.centered_work;
  result.predicted_cusp_work = 0.5 * coupling * charge
      * (result.jump.x * std::abs(displacement.x)
         + result.jump.y * std::abs(displacement.y)
         + result.jump.z * std::abs(displacement.z));
  result.cusp_formula_residual = std::abs(
      result.omitted_work - result.predicted_cusp_work);

  MatchedFaceFlux field_before = midpoint_electric;
  MatchedFaceFlux field_after = midpoint_electric;
  add_scaled(field_before, current, 0.5 * coupling);
  add_scaled(field_after, current, -0.5 * coupling);
  result.field_energy_change = energy(field_after) - energy(field_before);
  result.field_energy_residual = std::abs(
      result.field_energy_change + result.field_work);
  result.relative_gauss_transport_residual = 0.0;
  for (int x = 0; x < result.L; ++x) {
    for (int y = 0; y < result.L; ++y) {
      for (int z = 0; z < result.L; ++z) {
        const double residual = divergence_at(field_after, x, y, z)
            - divergence_at(field_before, x, y, z)
            + coupling * divergence_at(current, x, y, z);
        result.relative_gauss_transport_residual = std::max(
            result.relative_gauss_transport_residual,
            std::abs(residual));
      }
    }
  }
  result.continuity_residual = forward.continuity_residual;

  const double reverse_field_work = coupling * static_cast<double>(
      dot(reverse_current, midpoint_electric));
  const double reverse_centered_work = -result.centered_work;
  const double reverse_omitted = reverse_field_work
      - reverse_centered_work;
  result.reverse_field_work_residual = std::abs(
      result.field_work + reverse_field_work);
  result.reverse_centered_work_residual = std::abs(
      result.centered_work + reverse_centered_work);
  result.reverse_omitted_work_residual = std::abs(
      result.omitted_work + reverse_omitted);
  result.valid = std::isfinite(result.field_work)
      && std::isfinite(result.centered_work)
      && std::isfinite(result.omitted_work)
      && std::isfinite(result.predicted_cusp_work)
      && max_difference(current, reverse_current) >= 0.0;
  return result;
}

}  // namespace ftd::eft
