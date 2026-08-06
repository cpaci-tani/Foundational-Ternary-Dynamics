#include "ftd/eft/matched_symmetry_ray_spectrum.h"

#include "ftd/constants.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <numeric>
#include <set>
#include <vector>

namespace ftd::eft {
namespace {

bool finite(const std::vector<double>& values) {
  return std::all_of(values.begin(), values.end(),
      [](double value) { return std::isfinite(value); });
}

bool valid(const MatchedFaceFlux& field, int L) {
  const auto count = static_cast<std::size_t>(L) * L * L;
  return field.L == L && field.x.size() == count
      && field.y.size() == count && field.z.size() == count
      && finite(field.x) && finite(field.y) && finite(field.z);
}

bool valid(const MatchedEdgeField& field, int L) {
  const auto count = static_cast<std::size_t>(L) * L * L;
  return field.L == L && field.x.size() == count
      && field.y.size() == count && field.z.size() == count
      && finite(field.x) && finite(field.y) && finite(field.z);
}

double power(const MatchedComplexVector& value) {
  return std::norm(value[0]) + std::norm(value[1]) + std::norm(value[2]);
}

double projection_residual(const MatchedComplexVector& original,
                           const MatchedComplexVector& transverse,
                           const MatchedComplexVector& longitudinal) {
  double result = 0.0;
  for (int axis = 0; axis < 3; ++axis)
    result = std::max(result, std::abs(
        original[axis] - transverse[axis] - longitudinal[axis]));
  return result;
}

void project(const MatchedComplexVector& original,
             const std::array<double, 3>& lattice_wavevector,
             MatchedComplexVector& transverse,
             MatchedComplexVector& longitudinal) {
  const double norm2 = lattice_wavevector[0] * lattice_wavevector[0]
      + lattice_wavevector[1] * lattice_wavevector[1]
      + lattice_wavevector[2] * lattice_wavevector[2];
  std::complex<double> dot{};
  for (int axis = 0; axis < 3; ++axis)
    dot += lattice_wavevector[axis] * original[axis];
  for (int axis = 0; axis < 3; ++axis) {
    longitudinal[axis] = lattice_wavevector[axis] * dot / norm2;
    transverse[axis] = original[axis] - longitudinal[axis];
  }
}

bool initialize_geometry(MatchedWavevectorSpectrum& result) {
  const int L = result.L;
  const bool nonzero = result.mode[0] != 0 || result.mode[1] != 0
      || result.mode[2] != 0;
  if (L <= 0 || !nonzero) return false;
  for (int axis = 0; axis < 3; ++axis) {
    result.wavevector[axis] = 2.0 * PI * result.mode[axis] / L;
    result.lattice_wavevector[axis] =
        2.0 * std::sin(0.5 * result.wavevector[axis]);
  }
  const double norm2 =
      result.lattice_wavevector[0] * result.lattice_wavevector[0]
      + result.lattice_wavevector[1] * result.lattice_wavevector[1]
      + result.lattice_wavevector[2] * result.lattice_wavevector[2];
  return norm2 > 0.0 && std::isfinite(norm2);
}

bool finalize(MatchedWavevectorSpectrum& result, double wave_speed) {
  if (!initialize_geometry(result)) return false;
  project(result.electric, result.lattice_wavevector,
          result.electric_transverse, result.electric_longitudinal);
  project(result.magnetic, result.lattice_wavevector,
          result.magnetic_transverse, result.magnetic_longitudinal);
  result.electric_power = power(result.electric);
  result.magnetic_power = power(result.magnetic);
  result.transverse_power = power(result.electric_transverse)
      + wave_speed * wave_speed * power(result.magnetic_transverse);
  result.longitudinal_power = power(result.electric_longitudinal)
      + wave_speed * wave_speed * power(result.magnetic_longitudinal);
  result.total_power = result.electric_power
      + wave_speed * wave_speed * result.magnetic_power;
  result.electric_projection_residual = projection_residual(
      result.electric, result.electric_transverse,
      result.electric_longitudinal);
  result.magnetic_projection_residual = projection_residual(
      result.magnetic, result.magnetic_transverse,
      result.magnetic_longitudinal);
  result.valid = std::isfinite(result.transverse_power)
      && std::isfinite(result.longitudinal_power)
      && std::isfinite(result.total_power)
      && result.transverse_power >= 0.0 && result.longitudinal_power >= 0.0
      && result.total_power >= 0.0
      && result.electric_projection_residual <= 1e-14
      && result.magnetic_projection_residual <= 1e-14;
  return result.valid;
}

int wrap(int value, int L) {
  const int result = value % L;
  return result < 0 ? result + L : result;
}

bool valid_request(const MatchedSymmetryRayRequest& request) {
  const auto& d = request.direction;
  if ((d[0] == 0 && d[1] == 0 && d[2] == 0)
      || request.harmonics.empty())
    return false;
  const int divisor = std::gcd(std::gcd(std::abs(d[0]), std::abs(d[1])),
                               std::abs(d[2]));
  if (divisor != 1) return false;
  std::set<int> unique;
  for (int harmonic : request.harmonics)
    if (harmonic == 0 || !unique.insert(harmonic).second) return false;
  return true;
}

}  // namespace

MatchedWavevectorSpectrum observe_matched_wavevector_spectrum(
    const MatchedFaceFlux& reference_electric,
    const MatchedEdgeField& reference_magnetic,
    const MatchedFaceFlux& candidate_electric,
    const MatchedEdgeField& candidate_magnetic,
    const std::array<int, 3>& mode,
    double wave_speed) {
  MatchedWavevectorSpectrum result;
  result.L = reference_electric.L;
  result.mode = mode;
  const int L = result.L;
  const bool nonzero = mode[0] != 0 || mode[1] != 0 || mode[2] != 0;
  if (L <= 0 || !nonzero || !(wave_speed > 0.0)
      || !std::isfinite(wave_speed)
      || !valid(reference_electric, L) || !valid(reference_magnetic, L)
      || !valid(candidate_electric, L) || !valid(candidate_magnetic, L))
    return result;

  if (!initialize_geometry(result)) return result;

  const std::array<std::array<double, 3>, 3> face_offset{{
      {{0.5, 0.0, 0.0}}, {{0.0, 0.5, 0.0}}, {{0.0, 0.0, 0.5}}}};
  const std::array<std::array<double, 3>, 3> edge_offset{{
      {{0.0, 0.5, 0.5}}, {{0.5, 0.0, 0.5}}, {{0.5, 0.5, 0.0}}}};
  const std::array<const std::vector<double>*, 3> reference_e{{
      &reference_electric.x, &reference_electric.y, &reference_electric.z}};
  const std::array<const std::vector<double>*, 3> candidate_e{{
      &candidate_electric.x, &candidate_electric.y, &candidate_electric.z}};
  const std::array<const std::vector<double>*, 3> reference_b{{
      &reference_magnetic.x, &reference_magnetic.y, &reference_magnetic.z}};
  const std::array<const std::vector<double>*, 3> candidate_b{{
      &candidate_magnetic.x, &candidate_magnetic.y, &candidate_magnetic.z}};

  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto index = static_cast<std::size_t>(
            reference_electric.index(x, y, z));
        const std::array<double, 3> coordinate{{
            static_cast<double>(x), static_cast<double>(y),
            static_cast<double>(z)}};
        for (int component = 0; component < 3; ++component) {
          double face_phase = 0.0;
          double edge_phase = 0.0;
          for (int axis = 0; axis < 3; ++axis) {
            face_phase += result.wavevector[axis]
                * (coordinate[axis] + face_offset[component][axis]);
            edge_phase += result.wavevector[axis]
                * (coordinate[axis] + edge_offset[component][axis]);
          }
          result.electric[component] +=
              ((*candidate_e[component])[index]
               - (*reference_e[component])[index])
              * std::polar(1.0, -face_phase);
          result.magnetic[component] +=
              ((*candidate_b[component])[index]
               - (*reference_b[component])[index])
              * std::polar(1.0, -edge_phase);
        }
      }
    }
  }
  const double inverse_volume = 1.0 /
      (static_cast<double>(L) * static_cast<double>(L)
       * static_cast<double>(L));
  for (int component = 0; component < 3; ++component) {
    result.electric[component] *= inverse_volume;
    result.magnetic[component] *= inverse_volume;
  }

  finalize(result, wave_speed);
  return result;
}

MatchedSymmetryRayBatch observe_batched_matched_symmetry_ray_spectra(
    const MatchedFaceFlux& reference_electric,
    const MatchedEdgeField& reference_magnetic,
    const MatchedFaceFlux& candidate_electric,
    const MatchedEdgeField& candidate_magnetic,
    const std::vector<MatchedSymmetryRayRequest>& requests,
    double wave_speed) {
  MatchedSymmetryRayBatch result;
  result.L = reference_electric.L;
  const int L = result.L;
  if (L <= 0 || requests.empty() || !(wave_speed > 0.0)
      || !std::isfinite(wave_speed)
      || !valid(reference_electric, L) || !valid(reference_magnetic, L)
      || !valid(candidate_electric, L) || !valid(candidate_magnetic, L))
    return result;
  std::set<std::array<int, 3>> directions;
  std::size_t output_count = 0;
  for (const auto& request : requests) {
    if (!valid_request(request)
        || !directions.insert(request.direction).second)
      return result;
    output_count += request.harmonics.size();
  }

  const std::array<std::array<double, 3>, 3> face_offset{{
      {{0.5, 0.0, 0.0}}, {{0.0, 0.5, 0.0}}, {{0.0, 0.0, 0.5}}}};
  const std::array<std::array<double, 3>, 3> edge_offset{{
      {{0.0, 0.5, 0.5}}, {{0.5, 0.0, 0.5}}, {{0.5, 0.5, 0.0}}}};
  const std::array<const std::vector<double>*, 3> reference_e{{
      &reference_electric.x, &reference_electric.y, &reference_electric.z}};
  const std::array<const std::vector<double>*, 3> candidate_e{{
      &candidate_electric.x, &candidate_electric.y, &candidate_electric.z}};
  const std::array<const std::vector<double>*, 3> reference_b{{
      &reference_magnetic.x, &reference_magnetic.y, &reference_magnetic.z}};
  const std::array<const std::vector<double>*, 3> candidate_b{{
      &candidate_magnetic.x, &candidate_magnetic.y, &candidate_magnetic.z}};

  struct RayBins {
    std::array<std::vector<double>, 3> electric;
    std::array<std::vector<double>, 3> magnetic;
  };
  std::vector<RayBins> bins(requests.size());
  for (auto& ray : bins) {
    for (int component = 0; component < 3; ++component) {
      ray.electric[component].assign(static_cast<std::size_t>(L), 0.0);
      ray.magnetic[component].assign(static_cast<std::size_t>(L), 0.0);
    }
  }
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto index = static_cast<std::size_t>(
            reference_electric.index(x, y, z));
        std::array<double, 3> delta_e{};
        std::array<double, 3> delta_b{};
        for (int component = 0; component < 3; ++component) {
          delta_e[component] = (*candidate_e[component])[index]
              - (*reference_e[component])[index];
          delta_b[component] = (*candidate_b[component])[index]
              - (*reference_b[component])[index];
        }
        for (std::size_t ray = 0; ray < requests.size(); ++ray) {
          const auto& d = requests[ray].direction;
          const auto bin = static_cast<std::size_t>(
              wrap(d[0] * x + d[1] * y + d[2] * z, L));
          for (int component = 0; component < 3; ++component) {
            bins[ray].electric[component][bin] += delta_e[component];
            bins[ray].magnetic[component][bin] += delta_b[component];
          }
        }
      }
    }
  }

  const double inverse_volume = 1.0 /
      (static_cast<double>(L) * static_cast<double>(L)
       * static_cast<double>(L));
  result.spectra.reserve(output_count);
  for (std::size_t ray = 0; ray < requests.size(); ++ray) {
    const auto& request = requests[ray];
    for (int harmonic : request.harmonics) {
      MatchedWavevectorSpectrum spectrum;
      spectrum.L = L;
      for (int axis = 0; axis < 3; ++axis)
        spectrum.mode[axis] = harmonic * request.direction[axis];
      for (int component = 0; component < 3; ++component) {
        std::complex<double> electric_sum{};
        std::complex<double> magnetic_sum{};
        for (int bin = 0; bin < L; ++bin) {
          const auto phase = std::polar(
              1.0, -2.0 * PI * harmonic * bin / L);
          electric_sum += bins[ray].electric[component][bin] * phase;
          magnetic_sum += bins[ray].magnetic[component][bin] * phase;
        }
        double face_shift = 0.0;
        double edge_shift = 0.0;
        for (int axis = 0; axis < 3; ++axis) {
          face_shift += request.direction[axis]
              * face_offset[component][axis];
          edge_shift += request.direction[axis]
              * edge_offset[component][axis];
        }
        spectrum.electric[component] = electric_sum * inverse_volume
            * std::polar(1.0, -2.0 * PI * harmonic * face_shift / L);
        spectrum.magnetic[component] = magnetic_sum * inverse_volume
            * std::polar(1.0, -2.0 * PI * harmonic * edge_shift / L);
      }
      if (!finalize(spectrum, wave_speed)) return MatchedSymmetryRayBatch{};
      result.spectra.push_back(std::move(spectrum));
    }
  }
  result.valid = result.spectra.size() == output_count;
  return result;
}

}  // namespace ftd::eft
