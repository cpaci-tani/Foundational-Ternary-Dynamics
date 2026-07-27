/** FTD-0499: finite-fiber reversible-lift obstruction. */

#include "ftd/eft/axial_face_hop_reciprocity.h"
#include "ftd/eft/dressing_fiber_ledger.h"
#include "ftd/eft/finite_memory_reversible_lift.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double c_speed = 0.57735026918962576451;
constexpr double rest_energy = 0.511;
constexpr double coupling = 0.73;
constexpr double gate = 1e-12;
int failures = 0;
std::uint64_t largest_hidden_capacity = 0;
std::uint64_t binary_history_bits = 0;
std::uint64_t octant_history_bits = 0;
double shape_branch_difference = 0.0;
double current_branch_difference = 0.0;
double field_branch_difference = 0.0;
double momentum_branch_difference = 0.0;
double work_branch_difference = 0.0;
double dressing_branch_difference = 0.0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double max_abs(const ftd::Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double max_difference(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double residual = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    residual = std::max(residual, std::abs(lhs[i] - rhs[i]));
  }
  return residual;
}

double max_current_difference(
    const ftd::eft::FaceCurrentSegment& lhs,
    const ftd::eft::FaceCurrentSegment& rhs) {
  return std::max({
      max_difference(lhs.rho_before, rhs.rho_before),
      max_difference(lhs.rho_after, rhs.rho_after),
      max_difference(lhs.current_x, rhs.current_x),
      max_difference(lhs.current_y, rhs.current_y),
      max_difference(lhs.current_z, rhs.current_z)});
}

double max_field_difference(const ftd::eft::MatchedFaceFlux& lhs,
                            const ftd::eft::MatchedFaceFlux& rhs) {
  if (lhs.L != rhs.L || lhs.x.size() != rhs.x.size()) return INFINITY;
  return std::max({max_difference(lhs.x, rhs.x),
                   max_difference(lhs.y, rhs.y),
                   max_difference(lhs.z, rhs.z)});
}

std::vector<double> shape_density(ftd::Coord anchor,
                                  const ftd::Vec3& remainder) {
  std::vector<double> result(static_cast<std::size_t>(L * L * L), 0.0);
  const auto shape = ftd::eft::make_subcell_polarity_shape(
      anchor, remainder, +1);
  if (!shape.valid) return {};
  for (std::size_t i = 0; i < shape.weight_count; ++i) {
    const auto& entry = shape.weights[i];
    const int index = ((entry.site.x % L + L) % L) * L * L
        + ((entry.site.y % L + L) % L) * L
        + ((entry.site.z % L + L) % L);
    result[static_cast<std::size_t>(index)] += entry.weight;
  }
  return result;
}

ftd::eft::MatchedFaceFlux uniform_field(double amplitude) {
  ftd::eft::MatchedFaceFlux field(L);
  std::fill(field.x.begin(), field.x.end(), amplitude);
  return field;
}

ftd::eft::AxialFaceHopStep solve_chart(
    ftd::Coord site, double remainder) {
  ftd::eft::AxialFaceHopInput input;
  input.electric_before = uniform_field(1.35);
  input.site = site;
  input.remainder = {remainder, 0.0, 0.0};
  input.momentum_before = {};
  input.dressing_before = 0.37;
  input.axis = 0;
  input.charge = +1;
  input.coupling = coupling;
  input.dt = 1.0;
  input.rest_energy = rest_energy;
  input.causal_speed = c_speed;
  return ftd::eft::solve_axial_face_hop_step(input);
}

}  // namespace

int main() {
  bool binary_counts_ok = true;
  bool octant_counts_ok = true;
  for (int bits = 0; bits <= 20; ++bits) {
    const std::uint64_t hidden = std::uint64_t{1} << bits;
    largest_hidden_capacity = hidden;
    const auto binary = ftd::eft::analyze_finite_reversible_lift(2, hidden);
    const auto octant = ftd::eft::analyze_finite_reversible_lift(8, hidden);
    binary_counts_ok = binary_counts_ok && binary.valid
        && !binary.injective_lift_possible
        && binary.restricted_domain == 2 * hidden
        && binary.restricted_codomain == hidden
        && binary.cardinality_deficit == hidden;
    octant_counts_ok = octant_counts_ok && octant.valid
        && !octant.injective_lift_possible
        && octant.restricted_domain == 8 * hidden
        && octant.restricted_codomain == hidden
        && octant.cardinality_deficit == 7 * hidden;
  }
  check("every tested finite binary hidden fiber fails injectivity",
        binary_counts_ok);
  check("every tested finite eight-way hidden fiber fails injectivity",
        octant_counts_ok);
  check("invalid and overflowing finite counts fail closed",
        !ftd::eft::analyze_finite_reversible_lift(0, 4).valid
        && !ftd::eft::analyze_finite_reversible_lift(2, 0).valid
        && !ftd::eft::analyze_finite_reversible_lift(
            UINT64_MAX, 2).valid);

  std::array<std::uint64_t, 63> binary_word{};
  std::uint64_t binary_history = 0;
  bool binary_push_ok = true;
  for (std::size_t i = 0; i < binary_word.size(); ++i) {
    binary_word[i] = (i * 5 + 1) & 1U;
    const auto push = ftd::eft::push_history_branch(
        binary_history, binary_word[i], 2);
    binary_push_ok = binary_push_ok && push.valid;
    binary_history = push.after;
  }
  binary_history_bits = ftd::eft::minimum_history_bits(
      2, binary_word.size());
  bool binary_pop_ok = true;
  for (std::size_t i = binary_word.size(); i-- > 0;) {
    const auto pop = ftd::eft::pop_history_branch(binary_history, 2);
    binary_pop_ok = binary_pop_ok && pop.valid
        && pop.branch == binary_word[i];
    binary_history = pop.after;
  }
  check("unbounded-stack control pushes and pops 63 binary merges",
        binary_push_ok && binary_pop_ok && binary_history == 0
        && binary_history_bits == 63);

  std::array<std::uint64_t, 21> octant_word{};
  std::uint64_t octant_history = 0;
  bool octant_push_ok = true;
  for (std::size_t i = 0; i < octant_word.size(); ++i) {
    octant_word[i] = (3 * i + 7) & 7U;
    const auto push = ftd::eft::push_history_branch(
        octant_history, octant_word[i], 8);
    octant_push_ok = octant_push_ok && push.valid;
    octant_history = push.after;
  }
  octant_history_bits = ftd::eft::minimum_history_bits(
      8, octant_word.size());
  bool octant_pop_ok = true;
  for (std::size_t i = octant_word.size(); i-- > 0;) {
    const auto pop = ftd::eft::pop_history_branch(octant_history, 8);
    octant_pop_ok = octant_pop_ok && pop.valid
        && pop.branch == octant_word[i];
    octant_history = pop.after;
  }
  check("history control costs three bits per eight-way merge",
        octant_push_ok && octant_pop_ok && octant_history == 0
        && octant_history_bits == 63);
  check("history push rejects invalid branches and finite overflow",
        !ftd::eft::push_history_branch(0, 2, 2).valid
        && !ftd::eft::push_history_branch(UINT64_MAX, 0, 2).valid
        && !ftd::eft::pop_history_branch(0, 1).valid);

  const ftd::Coord lower_site{8, 8, 8};
  const ftd::Coord upper_site{9, 8, 8};
  const auto lower = solve_chart(lower_site, +0.875);
  const auto upper = solve_chart(upper_site, -0.125);
  check("explicit raw preimages are distinct but physically equivalent",
        lower.transaction_valid && upper.transaction_valid
        && lower.hopped && !upper.hopped
        && max_abs(lower.current.start_effective_position
                   - upper.current.start_effective_position) <= gate);

  shape_branch_difference = max_difference(
      shape_density(lower_site, {+0.875, 0.0, 0.0}),
      shape_density(upper_site, {-0.125, 0.0, 0.0}));
  current_branch_difference = max_current_difference(
      lower.current, upper.current);
  check("shape and exact current erase the raw chart branch",
        shape_branch_difference <= gate
        && current_branch_difference <= gate);

  field_branch_difference = std::max(
      max_field_difference(lower.electric_midpoint,
                           upper.electric_midpoint),
      max_field_difference(lower.electric_after,
                           upper.electric_after));
  momentum_branch_difference = max_abs(
      lower.momentum_after - upper.momentum_after);
  work_branch_difference = std::abs(
      lower.field_work - upper.field_work);
  const double output_state_difference = std::max(
      static_cast<double>(std::abs(lower.site_after.x - upper.site_after.x)
          + std::abs(lower.site_after.y - upper.site_after.y)
          + std::abs(lower.site_after.z - upper.site_after.z)),
      max_abs(lower.remainder_after - upper.remainder_after));
  check("field, work, momentum, and projected raw output erase the branch",
        field_branch_difference <= gate
        && momentum_branch_difference <= gate
        && work_branch_difference <= gate
        && output_state_difference <= gate);

  ftd::eft::MatchedFaceFlux dressing_field(L);
  const ftd::Coord knot{8, 8, 8};
  dressing_field.x[static_cast<std::size_t>(
      dressing_field.index(8, 8, 8))] = 0.3;
  dressing_field.x[static_cast<std::size_t>(
      dressing_field.index(7, 8, 8))] = -0.1;
  const auto work = ftd::eft::evaluate_centered_trace_work(
      dressing_field, knot, {0.2, 0.0, 0.0}, +1, coupling);
  const auto dressing_a = ftd::eft::advance_dressing_fiber(
      0.37, work, 1.0);
  const auto dressing_b = ftd::eft::advance_dressing_fiber(
      0.37, work, 1.0);
  dressing_branch_difference = std::abs(
      dressing_a.dressing_after - dressing_b.dressing_after);
  check("the registered dressing fiber update stores work, not chart branch",
        work.valid && dressing_a.valid && dressing_b.valid
        && std::abs(dressing_a.dressing_change) > 1e-6
        && dressing_branch_difference <= gate);

  check("all current quotient-side variables retain zero branch information",
        shape_branch_difference <= gate
        && current_branch_difference <= gate
        && field_branch_difference <= gate
        && momentum_branch_difference <= gate
        && work_branch_difference <= gate
        && dressing_branch_difference <= gate);

  std::cout.precision(17);
  std::cout << "largest_hidden_capacity="
            << largest_hidden_capacity << '\n'
            << "binary_history_bits=" << binary_history_bits << '\n'
            << "octant_history_bits=" << octant_history_bits << '\n'
            << "shape_branch_difference="
            << shape_branch_difference << '\n'
            << "current_branch_difference="
            << current_branch_difference << '\n'
            << "field_branch_difference="
            << field_branch_difference << '\n'
            << "momentum_branch_difference="
            << momentum_branch_difference << '\n'
            << "work_branch_difference="
            << work_branch_difference << '\n'
            << "dressing_branch_difference="
            << dressing_branch_difference << '\n'
            << "finite_memory_reversible_lift failures="
            << failures << '\n'
            << "verdict=UNBOUNDED_HISTORY_REQUIRED_FOR_FROZEN_PROJECTION\n";
  return failures == 0 ? 0 : 1;
}
