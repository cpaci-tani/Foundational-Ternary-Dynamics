/** FTD-0739: exact finite-support neutral-pair Gauss preparation. */

#include "ftd/eft/connected_moore_block_action.h"
#include "ftd/eft/matched_gauss_transport.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

namespace {

using ftd::Vec3;
using ftd::eft::ConnectedBindingLaw;
using ftd::eft::ConnectedMooreBlockOptions;
using ftd::eft::ConnectedMooreBlockState;
using ftd::eft::MatchedFaceFlux;
using ftd::eft::MatchedMatterPoint;

constexpr int kL = 33;
constexpr int kRadius = 4;
int failures = 0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

int wrap(int value, int L) {
  const int remainder = value % L;
  return remainder < 0 ? remainder + L : remainder;
}

MatchedMatterPoint point_at(const Vec3& position,
                            const Vec3& momentum, int L) {
  MatchedMatterPoint result;
  const long long ax = std::llround(position.x);
  const long long ay = std::llround(position.y);
  const long long az = std::llround(position.z);
  result.anchor = {wrap(static_cast<int>(ax),L),
                   wrap(static_cast<int>(ay),L),
                   wrap(static_cast<int>(az),L)};
  result.remainder = {position.x-ax,position.y-ay,position.z-az};
  result.momentum = momentum;
  return result;
}

ConnectedMooreBlockState geometry(const Vec3& direction,
                                  bool conjugate = false) {
  ConnectedMooreBlockState result(kL);
  const Vec3 center{static_cast<double>(kL/2),
                    static_cast<double>(kL/2),
                    static_cast<double>(kL/2)};
  const Vec3 unit = direction*(1.0/direction.mag());
  result.constituents.push_back(point_at(
      center-unit*0.65,unit*0.012,kL));
  result.constituents.push_back(point_at(
      center+unit*0.65,unit*(-0.012),kL));
  const int first = conjugate ? -1 : +1;
  result.charges = {first,-first};
  return result;
}

template <typename Field>
Field translated_field(const Field& field, int dx, int dy, int dz) {
  Field result(field.L);
  for (int x = 0; x < field.L; ++x)
    for (int y = 0; y < field.L; ++y)
      for (int z = 0; z < field.L; ++z) {
        const auto old_i = static_cast<std::size_t>(field.index(x,y,z));
        const auto new_i = static_cast<std::size_t>(
            result.index(x+dx,y+dy,z+dz));
        result.x[new_i] = field.x[old_i];
        result.y[new_i] = field.y[old_i];
        result.z[new_i] = field.z[old_i];
      }
  return result;
}

ConnectedMooreBlockState translated_geometry(
    const ConnectedMooreBlockState& state, int dx, int dy, int dz) {
  auto result = state;
  result.electric = translated_field(state.electric,dx,dy,dz);
  result.magnetic_half = translated_field(state.magnetic_half,dx,dy,dz);
  for (auto& point : result.constituents) {
    point.anchor.x = wrap(point.anchor.x+dx,state.electric.L);
    point.anchor.y = wrap(point.anchor.y+dy,state.electric.L);
    point.anchor.z = wrap(point.anchor.z+dz,state.electric.L);
  }
  return result;
}

template <typename Field>
Field cyclic_rotated_field(const Field& field) {
  Field result(field.L);
  for (int x = 0; x < field.L; ++x)
    for (int y = 0; y < field.L; ++y)
      for (int z = 0; z < field.L; ++z) {
        const auto old_i = static_cast<std::size_t>(field.index(x,y,z));
        const auto new_i = static_cast<std::size_t>(result.index(y,z,x));
        result.x[new_i] = field.y[old_i];
        result.y[new_i] = field.z[old_i];
        result.z[new_i] = field.x[old_i];
      }
  return result;
}

ConnectedMooreBlockState cyclic_rotated_geometry(
    const ConnectedMooreBlockState& state) {
  auto result = state;
  result.electric = cyclic_rotated_field(state.electric);
  result.magnetic_half = cyclic_rotated_field(state.magnetic_half);
  for (auto& point : result.constituents) {
    point.anchor = {point.anchor.y,point.anchor.z,point.anchor.x};
    point.remainder = {point.remainder.y,point.remainder.z,point.remainder.x};
    point.momentum = {point.momentum.y,point.momentum.z,point.momentum.x};
  }
  return result;
}

double field_difference(const MatchedFaceFlux& lhs,
                        const MatchedFaceFlux& rhs,
                        double rhs_scale = 1.0) {
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.x.size(); ++i) {
    result = std::max(result,std::abs(lhs.x[i]-rhs_scale*rhs.x[i]));
    result = std::max(result,std::abs(lhs.y[i]-rhs_scale*rhs.y[i]));
    result = std::max(result,std::abs(lhs.z[i]-rhs_scale*rhs.z[i]));
  }
  return result;
}

double gauss_difference(const MatchedFaceFlux& lhs,
                        const MatchedFaceFlux& rhs) {
  double result = 0.0;
  for (int x = 0; x < lhs.L; ++x)
    for (int y = 0; y < lhs.L; ++y)
      for (int z = 0; z < lhs.L; ++z)
        result = std::max(result,std::abs(
            ftd::eft::divergence_at(lhs,x,y,z)
            -ftd::eft::divergence_at(rhs,x,y,z)));
  return result;
}

}  // namespace

int main() {
  ConnectedMooreBlockOptions options;
  options.binding_law = ConnectedBindingLaw::DerivedCompactPair;
  options.compact_pair_well_depth = 0.01;
  options.compact_pair_cutoff_distance_squared = 1.5;
  options.allow_shared_anchor_chart = true;

  const auto base_geometry = geometry({0,0,1});
  const auto base = ftd::eft::prepare_finite_support_derived_compact_pair(
      base_geometry,options,kRadius,1e-13,4096);
  check("base preparation valid",base.valid);
  check("neutral and contained",base.neutral && base.density_contained);
  check("compact zero-crossing support",
        base.compact_support && base.zero_boundary_crossing
        && base.outside_maximum == 0.0
        && base.boundary_crossing_maximum == 0.0);
  check("poisson residual",base.poisson_residual <= 1e-13);
  check("gauss residual",base.gauss_residual <= 1e-12);
  check("internal KKT circulation",
        base.internal_circulation_residual <= 1e-12);
  check("nonzero finite field energy",
        base.electric_energy > 0.0 && std::isfinite(base.electric_energy));
  check("compact preparation carries boundary transverse content",
        base.curl_adjoint_residual > 1e-10);

  const auto translation = translated_geometry(base_geometry,3,-4,2);
  const auto translated =
      ftd::eft::prepare_finite_support_derived_compact_pair(
          translation,options,kRadius,1e-13,4096);
  const auto translated_expected = translated_field(base.state.electric,3,-4,2);
  const double translation_residual = translated.valid
      ? field_difference(translated.state.electric,translated_expected)
      : INFINITY;
  check("integer translation covariance",translation_residual <= 1e-11);

  const auto rotation = cyclic_rotated_geometry(base_geometry);
  const auto rotated = ftd::eft::prepare_finite_support_derived_compact_pair(
      rotation,options,kRadius,1e-13,4096);
  const auto rotated_expected = cyclic_rotated_field(base.state.electric);
  const double rotation_residual = rotated.valid
      ? field_difference(rotated.state.electric,rotated_expected) : INFINITY;
  check("proper cubic covariance",rotation_residual <= 1e-11);

  const auto conjugate = ftd::eft::prepare_finite_support_derived_compact_pair(
      geometry({0,0,1},true),options,kRadius,1e-13,4096);
  const double conjugation_residual = conjugate.valid
      ? field_difference(conjugate.state.electric,base.state.electric,-1.0)
      : INFINITY;
  check("polarity conjugation",conjugation_residual <= 1e-11);
  check("conjugate energy equality",
        conjugate.valid
        && std::abs(conjugate.electric_energy-base.electric_energy) <= 1e-12);

  for (const auto& direction : {Vec3{0,1,-1},Vec3{1,1,1}}) {
    const auto prepared =
        ftd::eft::prepare_finite_support_derived_compact_pair(
            geometry(direction),options,kRadius,1e-13,4096);
    check("symmetry-ray preparation valid",prepared.valid);
    check("symmetry-ray gauss",prepared.gauss_residual <= 1e-12);
  }

  auto plus_cycle = base.state.electric;
  auto minus_cycle = base.state.electric;
  constexpr double epsilon = 1e-3;
  const int c = kL/2;
  const auto add_cycle = [&](MatchedFaceFlux& field, double sign) {
    field.x[field.index(c,c,c)] += sign*epsilon;
    field.y[field.index(c+1,c,c)] += sign*epsilon;
    field.x[field.index(c,c+1,c)] -= sign*epsilon;
    field.y[field.index(c,c,c)] -= sign*epsilon;
  };
  add_cycle(plus_cycle,+1.0);
  add_cycle(minus_cycle,-1.0);
  check("cycle perturbation preserves divergence",
        gauss_difference(plus_cycle,base.state.electric) <= 1e-14
        && gauss_difference(minus_cycle,base.state.electric) <= 1e-14);
  check("strict energy minimum against plus cycle",
        ftd::eft::quadratic_energy(plus_cycle) > base.electric_energy);
  check("strict energy minimum against minus cycle",
        ftd::eft::quadratic_energy(minus_cycle) > base.electric_energy);

  auto nonneutral = base_geometry;
  nonneutral.charges = {+1,+1};
  check("nonneutral preparation fails closed",
        !ftd::eft::prepare_finite_support_derived_compact_pair(
            nonneutral,options,kRadius,1e-13,4096).valid);
  check("invalid support fails closed",
        !ftd::eft::prepare_finite_support_derived_compact_pair(
            base_geometry,options,1,1e-13,4096).valid);

  std::cout.precision(17);
  std::cout << "poisson_residual=" << base.poisson_residual << '\n'
            << "gauss_residual=" << base.gauss_residual << '\n'
            << "curl_adjoint_residual=" << base.curl_adjoint_residual << '\n'
            << "translation_residual=" << translation_residual << '\n'
            << "rotation_residual=" << rotation_residual << '\n'
            << "conjugation_residual=" << conjugation_residual << '\n'
            << "finite_support_pair_preparation failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}
