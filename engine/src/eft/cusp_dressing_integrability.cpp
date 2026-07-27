#include "ftd/eft/cusp_dressing_integrability.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {
namespace {

bool valid_closed_remainder(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z)
      && std::abs(value.x) <= 1.0
      && std::abs(value.y) <= 1.0
      && std::abs(value.z) <= 1.0;
}

bool valid_interior_remainder(const Vec3& value) {
  return valid_closed_remainder(value)
      && std::abs(value.x) < 1.0
      && std::abs(value.y) < 1.0
      && std::abs(value.z) < 1.0;
}

Vec3 jump_at(const MatchedFaceFlux& electric, Coord site, bool& valid) {
  const CenteredKnotTrace trace = evaluate_centered_knot_trace(electric, site);
  valid = trace.valid;
  return trace.outgoing - trace.incoming;
}

double l2_norm(const Vec3& value) {
  return std::sqrt(value.x * value.x + value.y * value.y
                   + value.z * value.z);
}

double sign_nonzero(double value) {
  return value < 0.0 ? -1.0 : value > 0.0 ? 1.0 : 0.0;
}

double max_difference(const Vec3& lhs, const Vec3& rhs) {
  return std::max({std::abs(lhs.x - rhs.x),
                   std::abs(lhs.y - rhs.y),
                   std::abs(lhs.z - rhs.z)});
}

}  // namespace

double local_cusp_dressing_energy(
    const Vec3& jump,
    const Vec3& remainder,
    int charge,
    double coupling) {
  if ((charge != -1 && charge != 1) || !valid_closed_remainder(remainder)
      || !std::isfinite(jump.x) || !std::isfinite(jump.y)
      || !std::isfinite(jump.z) || !std::isfinite(coupling)) {
    return NAN;
  }
  return 0.5 * coupling * static_cast<double>(charge)
      * (jump.x * std::abs(remainder.x)
         + jump.y * std::abs(remainder.y)
         + jump.z * std::abs(remainder.z));
}

CuspDressingIntegrabilityResult evaluate_cusp_dressing_integrability(
    const MatchedFaceFlux& electric,
    Coord site,
    const Vec3& remainder,
    int charge,
    double coupling) {
  CuspDressingIntegrabilityResult result;
  result.L = electric.L;
  result.charge = charge;
  result.site = site;
  result.remainder = remainder;
  result.coupling = coupling;
  if (electric.L <= 0 || (charge != -1 && charge != 1)
      || !valid_interior_remainder(remainder) || !std::isfinite(coupling)) {
    return result;
  }

  bool valid_0 = false;
  bool valid_x = false;
  bool valid_y = false;
  result.jump = jump_at(electric, site, valid_0);
  const Vec3 jump_x = jump_at(
      electric, {site.x + 1, site.y, site.z}, valid_x);
  const Vec3 jump_y = jump_at(
      electric, {site.x, site.y + 1, site.z}, valid_y);
  if (!valid_0 || !valid_x || !valid_y) return result;

  result.local_energy = local_cusp_dressing_energy(
      result.jump, remainder, charge, coupling);
  const CenteredTraceWorkResult work = evaluate_centered_trace_work(
      electric, site, remainder, charge, coupling);
  if (!work.valid || !std::isfinite(result.local_energy)) return result;
  result.exact_cusp_work = work.omitted_work;
  result.local_primitive_residual = std::abs(
      result.local_energy - result.exact_cusp_work);
  result.reverse_residual = work.reverse_omitted_work_residual;

  const double half_gq = 0.5 * coupling * static_cast<double>(charge);
  result.cusp_position_gradient = {
      half_gq * result.jump.x * sign_nonzero(remainder.x),
      half_gq * result.jump.y * sign_nonzero(remainder.y),
      half_gq * result.jump.z * sign_nonzero(remainder.z)};
  const CenteredKnotTrace trace = evaluate_centered_knot_trace(electric, site);
  const Vec3 branch_trace{
      trace.centered.x
          + 0.5 * result.jump.x * sign_nonzero(remainder.x),
      trace.centered.y
          + 0.5 * result.jump.y * sign_nonzero(remainder.y),
      trace.centered.z
          + 0.5 * result.jump.z * sign_nonzero(remainder.z)};
  const Vec3 restored_gradient = trace.centered
      * (coupling * static_cast<double>(charge))
      + result.cusp_position_gradient;
  result.branch_trace_gradient_residual = max_difference(
      restored_gradient,
      branch_trace * (coupling * static_cast<double>(charge)));

  result.threshold_site_offset_increment = result.jump * half_gq;
  result.threshold_representation_mismatch = std::max({
      std::abs(result.threshold_site_offset_increment.x),
      std::abs(result.threshold_site_offset_increment.y),
      std::abs(result.threshold_site_offset_increment.z)});

  result.path_xy = half_gq * (result.jump.x + jump_x.y);
  result.path_yx = half_gq * (result.jump.y + jump_y.x);
  result.plaquette_holonomy_xy = result.path_xy - result.path_yx;
  const double curl_xy = (jump_x.y - result.jump.y)
      - (jump_y.x - result.jump.x);
  result.predicted_holonomy_xy = half_gq * curl_xy;
  result.holonomy_residual = std::abs(
      result.plaquette_holonomy_xy - result.predicted_holonomy_xy);
  result.local_divergence = result.jump.x + result.jump.y + result.jump.z;

  // U contains two faces per active axis with derivatives +/-gq|r_a|/2.
  const Vec3 one_face_derivative{
      half_gq * std::abs(remainder.x),
      half_gq * std::abs(remainder.y),
      half_gq * std::abs(remainder.z)};
  result.field_euler_derivative_l2 = std::sqrt(2.0)
      * l2_norm(one_face_derivative);
  result.predicted_field_euler_derivative_l2 = std::abs(coupling)
      * std::sqrt((remainder.x * remainder.x
                   + remainder.y * remainder.y
                   + remainder.z * remainder.z) / 2.0);
  result.field_euler_derivative_residual = std::abs(
      result.field_euler_derivative_l2
      - result.predicted_field_euler_derivative_l2);

  result.valid = std::isfinite(result.local_energy)
      && std::isfinite(result.plaquette_holonomy_xy)
      && std::isfinite(result.field_euler_derivative_l2);
  return result;
}

}  // namespace ftd::eft
