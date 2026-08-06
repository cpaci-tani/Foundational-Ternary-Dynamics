// FTD-0697: batched/direct matched symmetry-ray spectrum equivalence.

#include "ftd/eft/matched_symmetry_ray_spectrum.h"

#include "ftd/constants.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr char protocol_sha256[] =
    "9C6CBA9957E215061CA7983177ADE97496566FD9356DE1688AB3F35542084376";
constexpr int L = 31;
int failures = 0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

double relative(double left, double right) {
  if (left == 0.0 && right == 0.0) return 0.0;
  return std::abs(left - right)
      / std::max({std::abs(left), std::abs(right), 1e-300});
}

double difference(const ftd::eft::MatchedComplexVector& left,
                  const ftd::eft::MatchedComplexVector& right) {
  double result = 0.0;
  for (int axis = 0; axis < 3; ++axis)
    result = std::max(result, std::abs(left[axis] - right[axis]));
  return result;
}

void add_modes(ftd::eft::MatchedFaceFlux& electric,
               ftd::eft::MatchedEdgeField& magnetic) {
  const std::array<std::array<int, 3>, 3> rays{{
      {{1, 0, 0}}, {{1, 1, 0}}, {{1, 1, 1}}}};
  const std::array<int, 3> harmonics{{3, 5, 7}};
  const std::array<std::array<double, 3>, 3> face_offset{{
      {{0.5, 0.0, 0.0}}, {{0.0, 0.5, 0.0}}, {{0.0, 0.0, 0.5}}}};
  const std::array<std::array<double, 3>, 3> edge_offset{{
      {{0.0, 0.5, 0.5}}, {{0.5, 0.0, 0.5}}, {{0.5, 0.5, 0.0}}}};
  std::array<std::vector<double>*, 3> e{{
      &electric.x, &electric.y, &electric.z}};
  std::array<std::vector<double>*, 3> b{{
      &magnetic.x, &magnetic.y, &magnetic.z}};
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto index = static_cast<std::size_t>(electric.index(x, y, z));
        const std::array<double, 3> coordinate{{
            static_cast<double>(x), static_cast<double>(y),
            static_cast<double>(z)}};
        for (std::size_t ray = 0; ray < rays.size(); ++ray) {
          for (std::size_t h = 0; h < harmonics.size(); ++h) {
            const double scale = 1e-5 * (1.0 + ray + 0.25 * h);
            for (int component = 0; component < 3; ++component) {
              double face_phase = 0.0;
              double edge_phase = 0.0;
              for (int axis = 0; axis < 3; ++axis) {
                const double k = 2.0 * ftd::PI
                    * harmonics[h] * rays[ray][axis] / L;
                face_phase += k * (coordinate[axis]
                    + face_offset[component][axis]);
                edge_phase += k * (coordinate[axis]
                    + edge_offset[component][axis]);
              }
              (*e[component])[index] += scale
                  * ((component + 1.0) * std::cos(face_phase)
                     + (0.5 + ray) * std::sin(face_phase));
              (*b[component])[index] += scale
                  * ((0.25 + h) * std::cos(edge_phase)
                     - (component + 0.75) * std::sin(edge_phase));
            }
          }
        }
      }
    }
  }
}

template <typename Field>
Field sample_plus(const Field& field, int dx, int dy, int dz) {
  Field result(field.L);
  for (int x = 0; x < field.L; ++x) {
    for (int y = 0; y < field.L; ++y) {
      for (int z = 0; z < field.L; ++z) {
        const auto destination = static_cast<std::size_t>(result.index(x, y, z));
        const auto source = static_cast<std::size_t>(
            field.index(x + dx, y + dy, z + dz));
        result.x[destination] = field.x[source];
        result.y[destination] = field.y[source];
        result.z[destination] = field.z[source];
      }
    }
  }
  return result;
}

bool compare_batch(const ftd::eft::MatchedFaceFlux& reference_e,
                   const ftd::eft::MatchedEdgeField& reference_b,
                   const ftd::eft::MatchedFaceFlux& candidate_e,
                   const ftd::eft::MatchedEdgeField& candidate_b,
                   double& worst_coefficient,
                   double& worst_power) {
  const std::vector<ftd::eft::MatchedSymmetryRayRequest> requests{
      {{{1, 0, 0}}, {3, 5, 7}},
      {{{1, 1, 0}}, {3, 5, 7}},
      {{{1, 1, 1}}, {3, 5, 7}}};
  const auto batch = ftd::eft::observe_batched_matched_symmetry_ray_spectra(
      reference_e, reference_b, candidate_e, candidate_b,
      requests, ftd::C_SPEED);
  if (!batch.valid || batch.spectra.size() != 9) return false;
  std::size_t output = 0;
  for (const auto& request : requests) {
    for (int harmonic : request.harmonics) {
      std::array<int, 3> mode{};
      for (int axis = 0; axis < 3; ++axis)
        mode[axis] = harmonic * request.direction[axis];
      const auto direct = ftd::eft::observe_matched_wavevector_spectrum(
          reference_e, reference_b, candidate_e, candidate_b,
          mode, ftd::C_SPEED);
      const auto& batched = batch.spectra[output++];
      if (!direct.valid || batched.mode != mode) return false;
      worst_coefficient = std::max({worst_coefficient,
          difference(direct.electric, batched.electric),
          difference(direct.magnetic, batched.magnetic),
          difference(direct.electric_transverse,
                     batched.electric_transverse),
          difference(direct.electric_longitudinal,
                     batched.electric_longitudinal),
          difference(direct.magnetic_transverse,
                     batched.magnetic_transverse),
          difference(direct.magnetic_longitudinal,
                     batched.magnetic_longitudinal)});
      worst_power = std::max({worst_power,
          relative(direct.total_power, batched.total_power),
          relative(direct.transverse_power, batched.transverse_power),
          relative(direct.longitudinal_power, batched.longitudinal_power)});
      if (worst_coefficient > 1e-14 || worst_power > 1e-12
          || batched.electric_projection_residual > 1e-14
          || batched.magnetic_projection_residual > 1e-14)
        return false;
    }
  }
  return true;
}

}  // namespace

int main() {
  const ftd::eft::MatchedFaceFlux reference_e(L);
  const ftd::eft::MatchedEdgeField reference_b(L);
  auto candidate_e = reference_e;
  auto candidate_b = reference_b;
  add_modes(candidate_e, candidate_b);

  double worst_coefficient = 0.0;
  double worst_power = 0.0;
  check("multi-ray direct equivalence", compare_batch(
      reference_e, reference_b, candidate_e, candidate_b,
      worst_coefficient, worst_power));

  const auto shifted_e = sample_plus(candidate_e, 1, -1, 1);
  const auto shifted_b = sample_plus(candidate_b, 1, -1, 1);
  check("translated direct equivalence", compare_batch(
      reference_e, reference_b, shifted_e, shifted_b,
      worst_coefficient, worst_power));

  check("zero direct equivalence", compare_batch(
      reference_e, reference_b, reference_e, reference_b,
      worst_coefficient, worst_power));

  const std::vector<ftd::eft::MatchedSymmetryRayRequest> valid{
      {{{1, 0, 0}}, {3, 5, 7}}};
  auto nonfinite = candidate_e;
  nonfinite.x[0] = std::numeric_limits<double>::quiet_NaN();
  check("nonfinite fails closed",
      !ftd::eft::observe_batched_matched_symmetry_ray_spectra(
          reference_e, reference_b, nonfinite, candidate_b,
          valid, ftd::C_SPEED).valid);
  check("zero direction fails closed",
      !ftd::eft::observe_batched_matched_symmetry_ray_spectra(
          reference_e, reference_b, candidate_e, candidate_b,
          {{{{0, 0, 0}}, {3}}}, ftd::C_SPEED).valid);
  check("nonprimitive direction fails closed",
      !ftd::eft::observe_batched_matched_symmetry_ray_spectra(
          reference_e, reference_b, candidate_e, candidate_b,
          {{{{2, 0, 0}}, {3}}}, ftd::C_SPEED).valid);
  check("zero harmonic fails closed",
      !ftd::eft::observe_batched_matched_symmetry_ray_spectra(
          reference_e, reference_b, candidate_e, candidate_b,
          {{{{1, 0, 0}}, {0}}}, ftd::C_SPEED).valid);
  check("duplicate harmonic fails closed",
      !ftd::eft::observe_batched_matched_symmetry_ray_spectra(
          reference_e, reference_b, candidate_e, candidate_b,
          {{{{1, 0, 0}}, {3, 3}}}, ftd::C_SPEED).valid);
  check("duplicate ray fails closed",
      !ftd::eft::observe_batched_matched_symmetry_ray_spectra(
          reference_e, reference_b, candidate_e, candidate_b,
          {{{{1, 0, 0}}, {3}}, {{{1, 0, 0}}, {5}}},
          ftd::C_SPEED).valid);

  std::cout.precision(17);
  std::cout << "protocol_sha256=" << protocol_sha256
            << " worst_coefficient_difference=" << worst_coefficient
            << " worst_power_relative_difference=" << worst_power
            << " failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}
