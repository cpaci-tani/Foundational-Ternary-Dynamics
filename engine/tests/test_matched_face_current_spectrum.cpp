#include "ftd/eft/matched_face_current_spectrum.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <iostream>
#include <limits>
#include <vector>

namespace {

constexpr int L = 17;
constexpr char protocol_sha256[] =
    "D92EF568A3933AB1BFD2AC0C6FAD72B0D8788DB5AE9198886CB8E784F96ACE8A";

using ftd::Vec3;
using ftd::eft::MatchedFaceCurrentSpectrum;
using ftd::eft::QuadraticCoatSparseCurrentEntry;

double coefficient_residual(const MatchedFaceCurrentSpectrum& lhs,
                            const MatchedFaceCurrentSpectrum& rhs) {
  double result = 0.0;
  for (int axis = 0; axis < 3; ++axis)
    result = std::max(result, std::abs(lhs.current[axis]-rhs.current[axis]));
  return result;
}

double relative(double lhs, double rhs) {
  return std::abs(lhs-rhs)/std::max({1e-300, std::abs(lhs), std::abs(rhs)});
}

ftd::eft::MatchedFaceFlux dense_current(
    const ftd::eft::QuadraticCoatFaceCurrent& segment) {
  ftd::eft::MatchedFaceFlux result(segment.L);
  result.x = segment.current_x;
  result.y = segment.current_y;
  result.z = segment.current_z;
  return result;
}

std::array<double, 3> cycle(std::array<double, 3> value) {
  return {{value[2], value[0], value[1]}};
}

Vec3 cycle(Vec3 value) { return {value.z, value.x, value.y}; }

}  // namespace

int main() {
  bool pass = true;
  double worst_one_face = 0.0;
  double worst_dense_sparse = 0.0;
  double worst_mirror = 0.0;
  double worst_translation = 0.0;
  double worst_cubic = 0.0;
  double worst_partition = 0.0;

  const std::array<double, 3> k{{0.7, -0.45, 0.3}};
  const std::vector<QuadraticCoatSparseCurrentEntry> one{{{2, 3, 4}, 0, 2.0}};
  const auto one_result = ftd::eft::observe_sparse_face_current_spectrum(
      L, one, k, 2.0);
  const double phase = k[0]*2.5+k[1]*3.0+k[2]*4.0;
  worst_one_face = std::abs(
      one_result.current[0]-std::polar(1.0, -phase));
  pass = pass && one_result.valid && worst_one_face <= 1e-14;

  const Vec3 start{7.17, 7.38, 8.11};
  const Vec3 delta{0.08, -0.03, 0.02};
  const auto positive = ftd::eft::make_quadratic_coat_face_current(
      L, start, start+delta, 1, true);
  const auto negative = ftd::eft::make_quadratic_coat_face_current(
      L, start, start+delta, -1, true);
  const auto doubled = ftd::eft::make_quadratic_coat_face_current(
      L, start, start+delta*2.0, 1, true);
  pass = pass && positive.valid && negative.valid && doubled.valid;
  pass = pass && positive.continuity_residual <= 1e-12
      && negative.continuity_residual <= 1e-12
      && doubled.continuity_residual <= 1e-12
      && positive.current_moment_residual <= 1e-12
      && negative.current_moment_residual <= 1e-12
      && doubled.current_moment_residual <= 1e-12;

  const auto sparse = ftd::eft::observe_sparse_face_current_spectrum(
      L, positive.sparse_current, k, delta.mag());
  const auto dense = ftd::eft::observe_dense_face_current_spectrum(
      dense_current(positive), k, delta.mag());
  const auto mirror = ftd::eft::observe_sparse_face_current_spectrum(
      L, negative.sparse_current, k, delta.mag());
  worst_dense_sparse = coefficient_residual(sparse, dense);
  for (int axis = 0; axis < 3; ++axis)
    worst_mirror = std::max(worst_mirror,
        std::abs(sparse.current[axis]+mirror.current[axis]));
  worst_mirror = std::max(worst_mirror,
      std::abs(sparse.total_power-mirror.total_power));
  pass = pass && sparse.valid && dense.valid && mirror.valid
      && worst_dense_sparse <= 1e-14 && worst_mirror <= 1e-14;

  worst_partition = relative(sparse.total_power,
      sparse.transverse_power+sparse.longitudinal_power);
  pass = pass && worst_partition <= 2e-13
      && sparse.projection_residual <= 1e-14;

  for (const Vec3 shift : {Vec3{1,0,0}, Vec3{0,-2,0}, Vec3{0,0,3}}) {
    const auto translated = ftd::eft::make_quadratic_coat_face_current(
        L, start+shift, start+shift+delta, 1, false);
    const auto observed = ftd::eft::observe_sparse_face_current_spectrum(
        L, translated.sparse_current, k, delta.mag());
    const double shift_phase = k[0]*shift.x+k[1]*shift.y+k[2]*shift.z;
    for (int axis = 0; axis < 3; ++axis)
      worst_translation = std::max(worst_translation, std::abs(
          observed.current[axis]
          - sparse.current[axis]*std::polar(1.0, -shift_phase)));
    pass = pass && translated.valid && observed.valid;
  }
  pass = pass && worst_translation <= 2e-13;

  Vec3 rotated_start = start;
  Vec3 rotated_delta = delta;
  auto rotated_k = k;
  auto prior = sparse;
  for (int turn = 0; turn < 2; ++turn) {
    rotated_start = cycle(rotated_start);
    rotated_delta = cycle(rotated_delta);
    rotated_k = cycle(rotated_k);
    const auto segment = ftd::eft::make_quadratic_coat_face_current(
        L, rotated_start, rotated_start+rotated_delta, 1, false);
    const auto observed = ftd::eft::observe_sparse_face_current_spectrum(
        L, segment.sparse_current, rotated_k, delta.mag());
    for (int axis = 0; axis < 3; ++axis)
      worst_cubic = std::max(worst_cubic, std::abs(
          observed.current[axis]-prior.current[(axis+2)%3]));
    worst_cubic = std::max({worst_cubic,
        std::abs(observed.total_power-prior.total_power),
        std::abs(observed.transverse_power-prior.transverse_power),
        std::abs(observed.longitudinal_power-prior.longitudinal_power)});
    pass = pass && segment.valid && observed.valid;
    prior = observed;
  }
  pass = pass && worst_cubic <= 2e-13;

  auto duplicates = positive.sparse_current;
  duplicates.insert(duplicates.end(), positive.sparse_current.begin(),
                    positive.sparse_current.end());
  const auto superposed = ftd::eft::observe_sparse_face_current_spectrum(
      L, duplicates, k, 2.0*delta.mag());
  pass = pass && superposed.valid
      && coefficient_residual(superposed, sparse) <= 1e-14;

  const std::vector<QuadraticCoatSparseCurrentEntry> invalid_axis{{{1,1,1},3,1.0}};
  const std::vector<QuadraticCoatSparseCurrentEntry> invalid_value{{
      {1,1,1},0,std::numeric_limits<double>::quiet_NaN()}};
  pass = pass
      && !ftd::eft::observe_sparse_face_current_spectrum(
          L, one, {{0,0,0}}, 1.0).valid
      && !ftd::eft::observe_sparse_face_current_spectrum(
          L, one, k, 0.0).valid
      && !ftd::eft::observe_sparse_face_current_spectrum(
          L, invalid_axis, k, 1.0).valid
      && !ftd::eft::observe_sparse_face_current_spectrum(
          L, invalid_value, k, 1.0).valid;

  std::cout << "protocol_sha256=" << protocol_sha256 << '\n'
            << "one_face=" << worst_one_face << '\n'
            << "dense_sparse=" << worst_dense_sparse << '\n'
            << "mirror=" << worst_mirror << '\n'
            << "translation=" << worst_translation << '\n'
            << "cubic=" << worst_cubic << '\n'
            << "power_partition=" << worst_partition << '\n'
            << "doubled_diagnostic_power=" << doubled.current_support << '\n'
            << "verdict=" << (pass ? "MATCHED_FACE_CURRENT_SPECTRUM_QUALIFIED"
                                    : "MATCHED_FACE_CURRENT_SPECTRUM_CLOSED")
            << '\n';
  return pass ? 0 : 1;
}
