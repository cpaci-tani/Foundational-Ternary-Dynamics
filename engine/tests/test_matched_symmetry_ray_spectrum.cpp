// FTD-0696: carrier-aware matched-field symmetry-ray spectrum observer.

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
    "3A750500246EDDED017E3CBC2D9DB3F5408616E062E15478A79FDAE93CCCB05B";
constexpr int L = 31;
constexpr double speed = ftd::C_SPEED;
constexpr double amplitude = 1e-4;
int failures = 0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

double relative(double left, double right) {
  return std::abs(left - right)
      / std::max({std::abs(left), std::abs(right), 1e-300});
}

double vector_difference(const ftd::eft::MatchedComplexVector& left,
                         const ftd::eft::MatchedComplexVector& right) {
  double result = 0.0;
  for (int axis = 0; axis < 3; ++axis)
    result = std::max(result, std::abs(left[axis] - right[axis]));
  return result;
}

std::array<double, 3> lattice_wavevector(const std::array<int, 3>& mode) {
  std::array<double, 3> result{};
  for (int axis = 0; axis < 3; ++axis)
    result[axis] = 2.0 * std::sin(ftd::PI * mode[axis] / L);
  return result;
}

std::array<double, 3> normalized(std::array<double, 3> value) {
  const double norm = std::sqrt(
      value[0] * value[0] + value[1] * value[1] + value[2] * value[2]);
  for (double& component : value) component /= norm;
  return value;
}

std::array<double, 3> transverse_polarization(
    const std::array<int, 3>& mode, int polarization) {
  const auto khat = lattice_wavevector(mode);
  std::array<double, 3> seed = polarization == 0
      ? std::array<double, 3>{{0.0, 0.0, 1.0}}
      : std::array<double, 3>{{0.0, 1.0, 0.0}};
  auto cross = std::array<double, 3>{{
      khat[1] * seed[2] - khat[2] * seed[1],
      khat[2] * seed[0] - khat[0] * seed[2],
      khat[0] * seed[1] - khat[1] * seed[0]}};
  double norm2 = cross[0] * cross[0] + cross[1] * cross[1]
      + cross[2] * cross[2];
  if (norm2 < 1e-20) {
    seed = {{1.0, 0.0, 0.0}};
    cross = {{khat[1] * seed[2] - khat[2] * seed[1],
              khat[2] * seed[0] - khat[0] * seed[2],
              khat[0] * seed[1] - khat[1] * seed[0]}};
  }
  auto first = normalized(cross);
  if (polarization == 0) return first;
  return normalized({{
      khat[1] * first[2] - khat[2] * first[1],
      khat[2] * first[0] - khat[0] * first[2],
      khat[0] * first[1] - khat[1] * first[0]}});
}

ftd::eft::MatchedFaceFlux face_plane_wave(
    const std::array<int, 3>& mode,
    const std::array<double, 3>& polarization,
    double scale) {
  ftd::eft::MatchedFaceFlux result(L);
  const std::array<std::array<double, 3>, 3> offsets{{
      {{0.5, 0.0, 0.0}}, {{0.0, 0.5, 0.0}}, {{0.0, 0.0, 0.5}}}};
  std::array<std::vector<double>*, 3> values{{
      &result.x, &result.y, &result.z}};
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto index = static_cast<std::size_t>(result.index(x, y, z));
        const std::array<double, 3> coordinate{{
            static_cast<double>(x), static_cast<double>(y),
            static_cast<double>(z)}};
        for (int component = 0; component < 3; ++component) {
          double phase = 0.0;
          for (int axis = 0; axis < 3; ++axis)
            phase += 2.0 * ftd::PI * mode[axis] / L
                * (coordinate[axis] + offsets[component][axis]);
          (*values[component])[index] =
              scale * polarization[component] * std::cos(phase);
        }
      }
    }
  }
  return result;
}

void add(ftd::eft::MatchedFaceFlux& target,
         const ftd::eft::MatchedFaceFlux& source) {
  for (std::size_t index = 0; index < target.x.size(); ++index) {
    target.x[index] += source.x[index];
    target.y[index] += source.y[index];
    target.z[index] += source.z[index];
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

ftd::eft::MatchedComplexVector phased(
    const ftd::eft::MatchedComplexVector& value,
    std::complex<double> phase) {
  auto result = value;
  for (auto& component : result) component *= phase;
  return result;
}

}  // namespace

int main() {
  const ftd::eft::MatchedFaceFlux zero_e(L);
  const ftd::eft::MatchedEdgeField zero_b(L);

  const auto zero = ftd::eft::observe_matched_wavevector_spectrum(
      zero_e, zero_b, zero_e, zero_b, {3, 0, 0}, speed);
  check("zero valid", zero.valid);
  check("zero exact", zero.total_power == 0.0
      && zero.transverse_power == 0.0 && zero.longitudinal_power == 0.0);
  check("zero mode rejected",
      !ftd::eft::observe_matched_wavevector_spectrum(
          zero_e, zero_b, zero_e, zero_b, {0, 0, 0}, speed).valid);
  auto nonfinite = zero_e;
  nonfinite.x[0] = std::numeric_limits<double>::quiet_NaN();
  check("nonfinite rejected",
      !ftd::eft::observe_matched_wavevector_spectrum(
          zero_e, zero_b, nonfinite, zero_b, {3, 0, 0}, speed).valid);

  const std::vector<std::array<int, 3>> symmetry_modes{{
      {{3, 0, 0}}, {{0, 3, 0}}, {{0, 0, 3}},
      {{5, 5, 0}}, {{0, 5, 5}}, {{5, 0, 5}}, {{7, 7, 7}}}};
  double worst_longitudinal_fraction = 0.0;
  double worst_empty_fraction = 0.0;
  double worst_cubic_power = 0.0;
  for (int polarization = 0; polarization < 2; ++polarization) {
    double family_100 = -1.0;
    double family_110 = -1.0;
    for (std::size_t arm = 0; arm < symmetry_modes.size(); ++arm) {
      const auto mode = symmetry_modes[arm];
      const auto direction = transverse_polarization(mode, polarization);
      const auto field = face_plane_wave(mode, direction, amplitude);
      const auto occupied = ftd::eft::observe_matched_wavevector_spectrum(
          zero_e, zero_b, field, zero_b, mode, speed);
      check("transverse arm valid", occupied.valid && occupied.total_power > 0.0);
      const double longitudinal_fraction = occupied.longitudinal_power
          / occupied.total_power;
      worst_longitudinal_fraction = std::max(
          worst_longitudinal_fraction, longitudinal_fraction);
      check("transverse leakage", longitudinal_fraction <= 1e-24);
      const std::array<int, 3> empty_mode{{mode[0] + 1, mode[1], mode[2]}};
      const auto empty = ftd::eft::observe_matched_wavevector_spectrum(
          zero_e, zero_b, field, zero_b, empty_mode, speed);
      const double empty_fraction = empty.total_power / occupied.total_power;
      worst_empty_fraction = std::max(worst_empty_fraction, empty_fraction);
      check("unoccupied leakage", empty.valid && empty_fraction <= 1e-24);
      check("projection reconstruction",
          occupied.electric_projection_residual <= 1e-14
          && occupied.magnetic_projection_residual <= 1e-14);

      const int nonzero_count = (mode[0] != 0) + (mode[1] != 0)
          + (mode[2] != 0);
      double* family = nonzero_count == 1 ? &family_100
          : (nonzero_count == 2 ? &family_110 : nullptr);
      if (family != nullptr && *family < 0.0) *family = occupied.total_power;
      else if (family != nullptr) {
        worst_cubic_power = std::max(
            worst_cubic_power, relative(*family, occupied.total_power));
        check("cubic family power", relative(*family, occupied.total_power) <= 1e-12);
      }
    }
  }

  const std::array<int, 3> mode{{5, 5, 0}};
  const auto polarization = transverse_polarization(mode, 0);
  const auto field = face_plane_wave(mode, polarization, amplitude);
  const auto doubled_field = face_plane_wave(mode, polarization, 2.0 * amplitude);
  const auto negative_field = face_plane_wave(mode, polarization, -amplitude);
  const auto base = ftd::eft::observe_matched_wavevector_spectrum(
      zero_e, zero_b, field, zero_b, mode, speed);
  const auto doubled = ftd::eft::observe_matched_wavevector_spectrum(
      zero_e, zero_b, doubled_field, zero_b, mode, speed);
  const auto negative = ftd::eft::observe_matched_wavevector_spectrum(
      zero_e, zero_b, negative_field, zero_b, mode, speed);
  check("quadratic amplitude", relative(doubled.total_power,
      4.0 * base.total_power) <= 1e-12);
  check("sign power", relative(negative.total_power, base.total_power) <= 1e-12);
  ftd::eft::MatchedComplexVector negated = base.electric;
  for (auto& component : negated) component = -component;
  check("sign coefficient", vector_difference(negative.electric, negated)
      <= amplitude * 1e-12);

  const std::array<int, 3> shift{{1, -1, 1}};
  const auto shifted_field = sample_plus(field, shift[0], shift[1], shift[2]);
  const auto shifted = ftd::eft::observe_matched_wavevector_spectrum(
      zero_e, zero_b, shifted_field, zero_b, mode, speed);
  const double translation_angle = 2.0 * ftd::PI / L
      * (mode[0] * shift[0] + mode[1] * shift[1] + mode[2] * shift[2]);
  const auto expected_shift = phased(
      base.electric, std::polar(1.0, translation_angle));
  check("translation phase", vector_difference(
      shifted.electric, expected_shift) <= amplitude * 1e-12);
  check("translation power", relative(
      shifted.total_power, base.total_power) <= 1e-12);

  const std::array<int, 3> other_mode{{0, 0, 7}};
  const auto other_field = face_plane_wave(
      other_mode, transverse_polarization(other_mode, 1), 0.7 * amplitude);
  auto superposed = field;
  add(superposed, other_field);
  const auto base_superposed = ftd::eft::observe_matched_wavevector_spectrum(
      zero_e, zero_b, superposed, zero_b, mode, speed);
  const auto other = ftd::eft::observe_matched_wavevector_spectrum(
      zero_e, zero_b, other_field, zero_b, other_mode, speed);
  const auto other_superposed = ftd::eft::observe_matched_wavevector_spectrum(
      zero_e, zero_b, superposed, zero_b, other_mode, speed);
  check("superposition first coefficient", vector_difference(
      base_superposed.electric, base.electric) <= amplitude * 1e-12);
  check("superposition second coefficient", vector_difference(
      other_superposed.electric, other.electric) <= amplitude * 1e-12);

  // A face field generated by the exact matched curl is transverse under the
  // carrier-aware lattice projector.
  double worst_curl_fraction = 0.0;
  for (const auto curl_mode : std::vector<std::array<int, 3>>{
           {{3, 0, 0}}, {{5, 5, 0}}, {{7, 7, 7}}}) {
    for (int edge_axis = 0; edge_axis < 3; ++edge_axis) {
      ftd::eft::MatchedEdgeField potential(L);
      auto* values = edge_axis == 0 ? &potential.x
          : (edge_axis == 1 ? &potential.y : &potential.z);
      for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
          for (int z = 0; z < L; ++z) {
            const double phase = 2.0 * ftd::PI / L
                * (curl_mode[0] * x + curl_mode[1] * y + curl_mode[2] * z);
            (*values)[static_cast<std::size_t>(potential.index(x, y, z))]
                = amplitude * std::cos(phase);
          }
        }
      }
      const auto curl_field = ftd::eft::matched_curl(potential);
      const auto spectrum = ftd::eft::observe_matched_wavevector_spectrum(
          zero_e, zero_b, curl_field, zero_b, curl_mode, speed);
      if (!(spectrum.total_power > 1e-30)) continue;
      const double fraction = spectrum.longitudinal_power / spectrum.total_power;
      worst_curl_fraction = std::max(worst_curl_fraction, fraction);
      check("curl longitudinal leakage", spectrum.valid && fraction <= 1e-24);
    }
  }

  const auto khat = normalized(lattice_wavevector(mode));
  const auto longitudinal_field = face_plane_wave(mode, khat, amplitude);
  const auto longitudinal = ftd::eft::observe_matched_wavevector_spectrum(
      zero_e, zero_b, longitudinal_field, zero_b, mode, speed);
  const double transverse_fraction = longitudinal.transverse_power
      / longitudinal.total_power;
  check("longitudinal arm valid", longitudinal.valid
      && longitudinal.total_power > 0.0);
  check("longitudinal transverse leakage", transverse_fraction <= 1e-24);

  std::cout.precision(17);
  std::cout << "protocol_sha256=" << protocol_sha256
            << " worst_transverse_longitudinal_fraction="
            << worst_longitudinal_fraction
            << " worst_longitudinal_transverse_fraction="
            << transverse_fraction
            << " worst_curl_fraction=" << worst_curl_fraction
            << " worst_empty_fraction=" << worst_empty_fraction
            << " worst_cubic_power_residual=" << worst_cubic_power
            << " failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}
