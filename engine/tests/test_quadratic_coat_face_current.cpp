/** FTD-0541: smooth positive coat, exact face current, and C1 plane gate. */

#include "ftd/eft/quadratic_coat_face_current.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double gate = 1e-12;
constexpr double derivative_gate = 1e-7;
constexpr double derivative_step = 0.000244140625;
int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double max_difference(const std::vector<double>& lhs,
                      const std::vector<double>& rhs,
                      double sign = 1.0) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, std::abs(lhs[i]-sign*rhs[i]));
  return result;
}

double maximum_metric(const ftd::eft::QuadraticCoatFaceCurrent& segment) {
  return std::max({segment.partition_residual,
                   segment.first_moment_residual,
                   segment.continuity_residual,
                   segment.current_moment_residual,
                   segment.locality_residual,
                   segment.causal_excess});
}

std::size_t index(int x, int y, int z) {
  const auto wrap = [](int value) {
    const int remainder = value%L;
    return remainder < 0 ? remainder+L : remainder;
  };
  return (static_cast<std::size_t>(wrap(x))*L+wrap(y))*L+wrap(z);
}

double shifted_residual(const std::vector<double>& source,
                        const std::vector<double>& shifted,
                        ftd::Coord shift) {
  double result = 0.0;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z)
        result = std::max(result, std::abs(
            shifted[index(x+shift.x, y+shift.y, z+shift.z)]
            -source[index(x, y, z)]));
  return result;
}

double permuted_scalar_residual(const std::vector<double>& source,
                                const std::vector<double>& permuted) {
  double result = 0.0;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z)
        result = std::max(result, std::abs(
            permuted[index(y, z, x)]-source[index(x, y, z)]));
  return result;
}

double aggregate_difference(
    const ftd::eft::QuadraticCoatAggregatedCurrent& lhs,
    const ftd::eft::QuadraticCoatAggregatedCurrent& rhs) {
  if(!lhs.valid||!rhs.valid||lhs.L!=rhs.L
      ||lhs.entries.size()!=rhs.entries.size()) return INFINITY;
  double result=0.0;
  for(std::size_t i=0;i<lhs.entries.size();++i) {
    const auto& a=lhs.entries[i];
    const auto& b=rhs.entries[i];
    if(a.axis!=b.axis||a.face.x!=b.face.x||a.face.y!=b.face.y
        ||a.face.z!=b.face.z) return INFINITY;
    result=std::max(result,std::abs(a.value-b.value));
  }
  return result;
}

double one_sided_plus(double f0, double f1, double f2,
                      double f3, double f4, double h) {
  return (-25.0*f0+48.0*f1-36.0*f2+16.0*f3-3.0*f4)/(12.0*h);
}

double one_sided_minus(double f0, double fm1, double fm2,
                       double fm3, double fm4, double h) {
  return (25.0*f0-48.0*fm1+36.0*fm2-16.0*fm3+3.0*fm4)/(12.0*h);
}

}  // namespace

int main() {
  const auto integer_coat = ftd::eft::make_quadratic_polarity_coat(
      {8.0, 8.0, 8.0}, +1);
  bool coat_ok = integer_coat.valid && integer_coat.weight_count == 27
      && integer_coat.minimum_unsigned_weight > 0.0;
  double center_weight = 0.0;
  double axis_neighbor_weight = 0.0;
  for (std::size_t i = 0; i < integer_coat.weight_count; ++i) {
    const auto& entry = integer_coat.weights[i];
    if (entry.site.x == 8 && entry.site.y == 8 && entry.site.z == 8)
      center_weight = entry.weight;
    if (entry.site.x == 9 && entry.site.y == 8 && entry.site.z == 8)
      axis_neighbor_weight = entry.weight;
  }
  coat_ok = coat_ok
      && std::abs(center_weight-27.0/64.0) <= gate
      && std::abs(axis_neighbor_weight-9.0/128.0) <= gate;
  check("integer manifestation has the exact positive 27-site smooth coat",
        coat_ok);

  int arms = 0;
  double worst_identity = 0.0;
  double worst_locality = 0.0;
  double worst_reversal = 0.0;
  double worst_polarity = 0.0;
  bool orbit_ok = true;
  const std::array<ftd::Coord, 3> translations{{
      {0, 0, 0}, {2, -1, 1}, {-2, 1, -1}}};
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        for (const auto translation : translations) {
          const ftd::Vec3 start{8.125+translation.x,
                                8.25+translation.y,
                                8.375+translation.z};
          const ftd::Vec3 delta{0.375*dx, 0.375*dy, 0.375*dz};
          const ftd::Vec3 end = start+delta;
          const auto positive = ftd::eft::make_quadratic_coat_face_current(
              L, start, end, +1);
          const auto negative = ftd::eft::make_quadratic_coat_face_current(
              L, start, end, -1);
          const auto reverse = ftd::eft::make_quadratic_coat_face_current(
              L, end, start, +1);
          ++arms;
          orbit_ok = orbit_ok && positive.valid && negative.valid
              && reverse.valid;
          worst_identity = std::max({worst_identity,
              maximum_metric(positive), maximum_metric(negative),
              maximum_metric(reverse)});
          worst_locality = std::max({worst_locality,
              positive.locality_residual, negative.locality_residual,
              reverse.locality_residual});
          worst_polarity = std::max({worst_polarity,
              max_difference(positive.rho_before, negative.rho_before, -1.0),
              max_difference(positive.rho_after, negative.rho_after, -1.0),
              max_difference(positive.current_x, negative.current_x, -1.0),
              max_difference(positive.current_y, negative.current_y, -1.0),
              max_difference(positive.current_z, negative.current_z, -1.0)});
          worst_reversal = std::max({worst_reversal,
              max_difference(positive.rho_before, reverse.rho_after),
              max_difference(positive.rho_after, reverse.rho_before),
              max_difference(positive.current_x, reverse.current_x, -1.0),
              max_difference(positive.current_y, reverse.current_y, -1.0),
              max_difference(positive.current_z, reverse.current_z, -1.0)});
        }
      }
    }
  }
  check("all 78 signed-cubic and translated straight paths close exactly",
        orbit_ok && arms == 78 && worst_identity <= gate);
  check("polarity mirror and path reversal are exact",
        worst_polarity <= gate && worst_reversal <= gate);

  const ftd::Vec3 base_start{8.125, 8.25, 8.375};
  const ftd::Vec3 base_end{8.5, 8.625, 8.75};
  const auto base = ftd::eft::make_quadratic_coat_face_current(
      L, base_start, base_end, +1);
  const ftd::Coord shift{2, -1, 1};
  const auto translated = ftd::eft::make_quadratic_coat_face_current(
      L, base_start+ftd::Vec3{2.0, -1.0, 1.0},
      base_end+ftd::Vec3{2.0, -1.0, 1.0}, +1);
  double translation_residual = std::max({
      shifted_residual(base.rho_before, translated.rho_before, shift),
      shifted_residual(base.rho_after, translated.rho_after, shift),
      shifted_residual(base.current_x, translated.current_x, shift),
      shifted_residual(base.current_y, translated.current_y, shift),
      shifted_residual(base.current_z, translated.current_z, shift)});
  check("integer translation transports every coat and face coefficient",
        base.valid && translated.valid && translation_residual <= gate);

  const auto permuted = ftd::eft::make_quadratic_coat_face_current(
      L, {base_start.y, base_start.z, base_start.x},
      {base_end.y, base_end.z, base_end.x}, +1);
  const double cubic_residual = std::max({
      permuted_scalar_residual(base.rho_before, permuted.rho_before),
      permuted_scalar_residual(base.rho_after, permuted.rho_after),
      permuted_scalar_residual(base.current_x, permuted.current_z),
      permuted_scalar_residual(base.current_y, permuted.current_x),
      permuted_scalar_residual(base.current_z, permuted.current_y)});
  check("cyclic cubic permutation transports charge and oriented current",
        permuted.valid && cubic_residual <= gate);

  const auto periodic = ftd::eft::make_quadratic_coat_face_current(
      L, {16.75, 8.125, 8.25}, {0.25, 8.5, 8.0}, +1);
  const auto stationary = ftd::eft::make_quadratic_coat_face_current(
      L, {8.5, 8.5, 8.5}, {8.5, 8.5, 8.5}, -1);
  check("periodic crossing and stationary control retain exact continuity",
        periodic.valid && stationary.valid
        && maximum_metric(periodic) <= gate
        && maximum_metric(stationary) <= gate
        && stationary.current_support == 0);

  const auto sparse_base=ftd::eft::make_quadratic_coat_face_current(
      L,base_start,base_end,+1,false);
  auto split=sparse_base;
  split.sparse_current.clear();
  for(const auto& entry:sparse_base.sparse_current) {
    auto half=entry;
    half.value*=0.5;
    split.sparse_current.push_back(half);
    split.sparse_current.push_back(half);
  }
  auto periodic_image=sparse_base;
  for(auto& entry:periodic_image.sparse_current) {
    entry.face.x+=L;
    entry.face.y-=L;
  }
  const auto aggregate_base=ftd::eft::aggregate_quadratic_coat_face_current(
      {sparse_base});
  const auto aggregate_split=ftd::eft::aggregate_quadratic_coat_face_current(
      {split});
  const auto aggregate_periodic=ftd::eft::aggregate_quadratic_coat_face_current(
      {periodic_image});
  check("aggregated support is invariant under entry splitting and periodic images",
      aggregate_base.valid&&aggregate_split.valid&&aggregate_periodic.valid
      &&aggregate_split.raw_contributions==2*aggregate_base.raw_contributions
      &&aggregate_difference(aggregate_base,aggregate_split)<=gate
      &&aggregate_difference(aggregate_base,aggregate_periodic)<=gate);

  const auto sparse_opposite=ftd::eft::make_quadratic_coat_face_current(
      L,base_start,base_end,-1,false);
  const auto cancelled=ftd::eft::aggregate_quadratic_coat_face_current(
      {sparse_base,sparse_opposite},1.0,gate);
  check("opposite duplicate contributions cancel before support is counted",
      cancelled.valid&&cancelled.entries.empty()&&cancelled.net_l1<=gate
      &&cancelled.cancelled_l1>0.0
      &&cancelled.aggregation_moment_residual<=gate);

  auto with_noise=sparse_base;
  with_noise.sparse_current.push_back({{0,0,0},0,0.5*gate});
  const auto gated=ftd::eft::aggregate_quadratic_coat_face_current(
      {with_noise},1.0,gate);
  check("the explicit support tolerance quarantines only its reported L1 mass",
      gated.valid&&gated.entries.size()==aggregate_base.entries.size()
      &&std::abs(gated.discarded_l1-0.5*gate)<=gate*1e-6
      &&gated.aggregation_moment_residual<=gate);

  const std::size_t volume = static_cast<std::size_t>(L)*L*L;
  std::vector<double> potential_x(volume, 0.0);
  std::vector<double> potential_y(volume, 0.0);
  std::vector<double> potential_z(volume, 0.0);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto i = index(x, y, z);
        potential_x[i] = 0.013*x+0.002*y*y-0.007*z;
        potential_y[i] = -0.005*x+0.011*y+0.001*z*z;
        potential_z[i] = 0.003*x*x-0.004*y+0.009*z;
      }
    }
  }
  const auto coupling = [&](double normal) {
    const auto segment = ftd::eft::make_quadratic_coat_face_current(
        L, {8.125, 8.25, normal}, {8.375, 8.625, normal}, +1);
    return ftd::eft::quadratic_coat_connection_coupling(
        segment, potential_x, potential_y, potential_z);
  };
  const auto derivatives = [&](double h) {
    const double f0 = coupling(8.0);
    const double left = one_sided_minus(f0, coupling(8.0-h),
        coupling(8.0-2*h), coupling(8.0-3*h), coupling(8.0-4*h), h);
    const double right = one_sided_plus(f0, coupling(8.0+h),
        coupling(8.0+2*h), coupling(8.0+3*h), coupling(8.0+4*h), h);
    return std::array<double, 2>{{left, right}};
  };
  const auto coarse = derivatives(derivative_step);
  const auto fine = derivatives(derivative_step/2.0);
  const double derivative_jump = std::abs(fine[1]-fine[0]);
  const double derivative_convergence = std::max(
      std::abs(fine[0]-coarse[0]), std::abs(fine[1]-coarse[1]));
  check("quadratic coat removes the inactive integer-plane action cusp",
        std::isfinite(fine[0]) && std::isfinite(fine[1])
        && derivative_jump <= derivative_gate
        && derivative_convergence <= derivative_gate);

  const auto invalid_polarity = ftd::eft::make_quadratic_polarity_coat(
      {8.0, 8.0, 8.0}, 0);
  const auto invalid_nan = ftd::eft::make_quadratic_coat_face_current(
      L, {NAN, 0.0, 0.0}, {}, +1);
  const auto invalid_causal = ftd::eft::make_quadratic_coat_face_current(
      L, {8.0, 8.0, 8.0}, {9.125, 8.0, 8.0}, +1);
  check("invalid and over-causal inputs fail closed",
        !invalid_polarity.valid && !invalid_nan.valid
        && !invalid_causal.valid);

  const bool constructive = failures == 0 && worst_identity <= gate
      && derivative_jump <= derivative_gate
      && derivative_convergence <= derivative_gate;
  const char* verdict = constructive
      ? "QUADRATIC_COAT_EXACT_CURRENT_C1_CONSTRUCTIVE"
      : (worst_identity > gate
          ? "QUADRATIC_COAT_CURRENT_CLOSED_NEGATIVE"
          : (derivative_jump > derivative_gate
              ? "QUADRATIC_COAT_C1_ESCAPE_CLOSED_NEGATIVE"
              : "QUADRATIC_COAT_FACE_CURRENT_UNRESOLVED"));

  std::cout.precision(17);
  std::cout << "arms=" << arms << '\n'
            << "integer_coat_support=" << integer_coat.weight_count << '\n'
            << "integer_center_weight=" << center_weight << '\n'
            << "integer_axis_neighbor_weight=" << axis_neighbor_weight << '\n'
            << "worst_identity_residual=" << worst_identity << '\n'
            << "worst_locality_residual=" << worst_locality << '\n'
            << "worst_polarity_residual=" << worst_polarity << '\n'
            << "worst_reversal_residual=" << worst_reversal << '\n'
            << "translation_residual=" << translation_residual << '\n'
            << "cubic_residual=" << cubic_residual << '\n'
            << "periodic_continuity_residual="
            << periodic.continuity_residual << '\n'
            << "reflection_left_derivative=" << fine[0] << '\n'
            << "reflection_right_derivative=" << fine[1] << '\n'
            << "reflection_derivative_jump=" << derivative_jump << '\n'
            << "reflection_derivative_convergence="
            << derivative_convergence << '\n'
            << "quadratic_coat_face_current failures=" << failures << '\n'
            << "verdict=" << verdict << '\n';
  return failures == 0 ? 0 : 1;
}
