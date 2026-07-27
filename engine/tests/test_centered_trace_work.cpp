/** FTD-0493: exact field work omitted by the centered knot trace. */

#include "ftd/eft/centered_trace_work.h"
#include "ftd/eft/discrete_legendre_worldline.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

namespace {

constexpr int L = 17;
constexpr ftd::Coord knot{8, 8, 8};
constexpr double c_speed = 0.57735026918962576451;
constexpr double rest_energy = 0.511;
constexpr double coupling = 0.73;
constexpr double gate = 1e-12;
int failures = 0;
double worst_formula_residual = 0.0;
double worst_energy_residual = 0.0;
double worst_gauss_residual = 0.0;
double worst_continuity_residual = 0.0;
double worst_reverse_residual = 0.0;
double polarity_mirror_residual = 0.0;
double largest_omitted_work = 0.0;
double source_free_omitted_work = 0.0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

ftd::eft::MatchedFaceFlux plane_trace_field(
    ftd::Coord site, const ftd::Vec3& centered,
    const ftd::Vec3& jump) {
  ftd::eft::MatchedFaceFlux field(L);
  std::fill(field.x.begin(), field.x.end(), centered.x);
  std::fill(field.y.begin(), field.y.end(), centered.y);
  std::fill(field.z.begin(), field.z.end(), centered.z);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = field.index(x, y, z);
        if (x == site.x) field.x[static_cast<std::size_t>(i)] += 0.5 * jump.x;
        if (x == site.x - 1) field.x[static_cast<std::size_t>(i)] -= 0.5 * jump.x;
        if (y == site.y) field.y[static_cast<std::size_t>(i)] += 0.5 * jump.y;
        if (y == site.y - 1) field.y[static_cast<std::size_t>(i)] -= 0.5 * jump.y;
        if (z == site.z) field.z[static_cast<std::size_t>(i)] += 0.5 * jump.z;
        if (z == site.z - 1) field.z[static_cast<std::size_t>(i)] -= 0.5 * jump.z;
      }
    }
  }
  return field;
}

void accumulate(const ftd::eft::CenteredTraceWorkResult& result) {
  worst_formula_residual = std::max(
      worst_formula_residual, result.cusp_formula_residual);
  worst_energy_residual = std::max(
      worst_energy_residual, result.field_energy_residual);
  worst_gauss_residual = std::max(
      worst_gauss_residual, result.relative_gauss_transport_residual);
  worst_continuity_residual = std::max(
      worst_continuity_residual, result.continuity_residual);
  worst_reverse_residual = std::max({worst_reverse_residual,
      result.reverse_field_work_residual,
      result.reverse_centered_work_residual,
      result.reverse_omitted_work_residual});
  largest_omitted_work = std::max(
      largest_omitted_work, std::abs(result.omitted_work));
}

}  // namespace

int main() {
  const ftd::Vec3 bias{0.4, 0.5, 0.6};
  ftd::eft::CenteredTraceWorkResult positive_reference;
  for (int polarity : {-1, +1}) {
    const ftd::Vec3 jump{
        polarity / 3.0, polarity / 3.0, polarity / 3.0};
    const auto field = plane_trace_field(knot, bias, jump);
    const ftd::Vec3 momentum = bias
        * (coupling * polarity * c_speed / 2.0);
    const auto displacement = ftd::eft::free_displacement_from_momentum(
        momentum, rest_energy, c_speed, c_speed);
    const auto result = ftd::eft::evaluate_centered_trace_work(
        field, knot, displacement, polarity, coupling);
    accumulate(result);
    check(polarity > 0 ? "+ exact cusp-work identity"
                       : "- exact cusp-work identity",
          result.valid && result.cusp_formula_residual <= gate
          && result.field_energy_residual <= gate
          && result.relative_gauss_transport_residual <= gate
          && result.continuity_residual <= gate
          && result.reverse_field_work_residual <= gate
          && result.reverse_centered_work_residual <= gate
          && result.reverse_omitted_work_residual <= gate
          && std::abs(result.omitted_work) > gate);
    if (polarity > 0) positive_reference = result;
    if (polarity < 0) {
      // Filled after the positive arm below via a direct second comparison.
    }
  }
  const ftd::Vec3 positive_jump{1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0};
  const ftd::Vec3 negative_jump = positive_jump * -1.0;
  const auto positive_displacement = ftd::eft::free_displacement_from_momentum(
      bias * (coupling * c_speed / 2.0),
      rest_energy, c_speed, c_speed);
  const auto negative_displacement = positive_displacement * -1.0;
  const auto negative_reference = ftd::eft::evaluate_centered_trace_work(
      plane_trace_field(knot, bias, negative_jump),
      knot, negative_displacement, -1, coupling);
  polarity_mirror_residual = std::abs(
      positive_reference.omitted_work - negative_reference.omitted_work);
  check("polarity mirror preserves positive cusp ledger",
        polarity_mirror_residual <= gate);

  const ftd::Coord shifted{11, 6, 12};
  const auto translated = ftd::eft::evaluate_centered_trace_work(
      plane_trace_field(shifted, bias, positive_jump),
      shifted, positive_displacement, +1, coupling);
  accumulate(translated);
  check("integer translation preserves work ledger",
        translated.valid
        && std::abs(translated.omitted_work
                    - positive_reference.omitted_work) <= gate);

  const ftd::Vec3 rotated_bias{bias.y, bias.z, bias.x};
  const ftd::Vec3 rotated_jump{
      positive_jump.y, positive_jump.z, positive_jump.x};
  const ftd::Vec3 rotated_displacement{
      positive_displacement.y,
      positive_displacement.z,
      positive_displacement.x};
  const ftd::Coord rotated_site{knot.y, knot.z, knot.x};
  const auto rotated = ftd::eft::evaluate_centered_trace_work(
      plane_trace_field(rotated_site, rotated_bias, rotated_jump),
      rotated_site, rotated_displacement, +1, coupling);
  accumulate(rotated);
  check("cyclic cubic rotation preserves work ledger",
        rotated.valid
        && std::abs(rotated.omitted_work
                    - positive_reference.omitted_work) <= gate);

  const ftd::Vec3 zero_jump{};
  const auto no_jump = ftd::eft::evaluate_centered_trace_work(
      plane_trace_field(knot, bias, zero_jump),
      knot, positive_displacement, +1, coupling);
  accumulate(no_jump);
  check("zero-jump control has no omitted work",
        no_jump.valid && std::abs(no_jump.omitted_work) <= gate);

  const ftd::Vec3 source_free_jump{0.3, -0.3, 0.0};
  const auto source_free = ftd::eft::evaluate_centered_trace_work(
      plane_trace_field(knot, bias, source_free_jump),
      knot, positive_displacement, +1, coupling);
  accumulate(source_free);
  source_free_omitted_work = source_free.omitted_work;
  const double source_free_divergence = source_free_jump.x
      + source_free_jump.y + source_free_jump.z;
  check("source-free jump retains nonzero omitted work",
        source_free.valid && std::abs(source_free_divergence) <= gate
        && std::abs(source_free.omitted_work) > gate
        && source_free.cusp_formula_residual <= gate);

  check("invalid charge fails closed",
        !ftd::eft::evaluate_centered_trace_work(
            plane_trace_field(knot, bias, zero_jump),
            knot, positive_displacement, 0, coupling).valid);

  std::cout.precision(17);
  std::cout << "largest_omitted_work=" << largest_omitted_work << '\n'
            << "source_free_omitted_work="
            << source_free_omitted_work << '\n'
            << "worst_cusp_formula_residual="
            << worst_formula_residual << '\n'
            << "worst_field_energy_residual="
            << worst_energy_residual << '\n'
            << "worst_relative_gauss_residual="
            << worst_gauss_residual << '\n'
            << "worst_continuity_residual="
            << worst_continuity_residual << '\n'
            << "worst_reverse_residual="
            << worst_reverse_residual << '\n'
            << "polarity_mirror_residual="
            << polarity_mirror_residual << '\n'
            << "centered_trace_work failures=" << failures << '\n'
            << "verdict=CENTERED_TRACE_LEAVES_EXACT_CUSP_WORK_LEDGER\n";
  return failures == 0 ? 0 : 1;
}
