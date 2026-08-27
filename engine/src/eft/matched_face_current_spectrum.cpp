#include "ftd/eft/matched_face_current_spectrum.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {
namespace {

double power(const FaceCurrentComplexVector& value) {
  return std::norm(value[0]) + std::norm(value[1]) + std::norm(value[2]);
}

bool finite(const std::array<double, 3>& value) {
  return std::all_of(value.begin(), value.end(),
      [](double component) { return std::isfinite(component); });
}

bool finalize(MatchedFaceCurrentSpectrum& result) {
  if (result.L < 5 || !(result.normalization > 0.0)
      || !std::isfinite(result.normalization) || !finite(result.wavevector))
    return false;
  double norm2 = 0.0;
  for (int axis = 0; axis < 3; ++axis) {
    result.lattice_wavevector[axis] =
        2.0 * std::sin(0.5 * result.wavevector[axis]);
    norm2 += result.lattice_wavevector[axis]
        * result.lattice_wavevector[axis];
  }
  if (!(norm2 > 0.0) || !std::isfinite(norm2)) return false;
  std::complex<double> dot{};
  for (int axis = 0; axis < 3; ++axis)
    dot += result.lattice_wavevector[axis] * result.current[axis];
  for (int axis = 0; axis < 3; ++axis) {
    result.longitudinal[axis] = result.lattice_wavevector[axis] * dot / norm2;
    result.transverse[axis] = result.current[axis]
        - result.longitudinal[axis];
    result.projection_residual = std::max(result.projection_residual,
        std::abs(result.current[axis] - result.transverse[axis]
                 - result.longitudinal[axis]));
  }
  result.total_power = power(result.current);
  result.transverse_power = power(result.transverse);
  result.longitudinal_power = power(result.longitudinal);
  result.transverse_fraction = result.total_power > 0.0
      ? result.transverse_power / result.total_power : 0.0;
  result.power_partition_residual = std::abs(
      result.total_power - result.transverse_power - result.longitudinal_power);
  result.valid = std::isfinite(result.input_l1)
      && std::isfinite(result.total_power)
      && std::isfinite(result.transverse_power)
      && std::isfinite(result.longitudinal_power)
      && std::isfinite(result.transverse_fraction)
      && result.input_l1 >= 0.0 && result.total_power >= 0.0
      && result.transverse_power >= 0.0 && result.longitudinal_power >= 0.0
      && result.transverse_fraction >= 0.0
      && result.transverse_fraction <= 1.0 + 1e-14
      && result.projection_residual <= 1e-14;
  return result.valid;
}

}  // namespace

MatchedFaceCurrentSpectrum observe_sparse_face_current_spectrum(
    int L,
    const std::vector<QuadraticCoatSparseCurrentEntry>& entries,
    const std::array<double, 3>& wavevector,
    double normalization) {
  MatchedFaceCurrentSpectrum result;
  result.L = L;
  result.wavevector = wavevector;
  result.normalization = normalization;
  if (L < 5 || entries.empty() || !(normalization > 0.0)
      || !std::isfinite(normalization) || !finite(wavevector))
    return result;
  const std::array<std::array<double, 3>, 3> offset{{
      {{0.5, 0.0, 0.0}}, {{0.0, 0.5, 0.0}}, {{0.0, 0.0, 0.5}}}};
  for (const auto& entry : entries) {
    if (entry.axis < 0 || entry.axis > 2 || !std::isfinite(entry.value))
      return MatchedFaceCurrentSpectrum{};
    const std::array<double, 3> coordinate{{
        static_cast<double>(entry.face.x),
        static_cast<double>(entry.face.y),
        static_cast<double>(entry.face.z)}};
    double phase = 0.0;
    for (int axis = 0; axis < 3; ++axis)
      phase += wavevector[axis]
          * (coordinate[axis] + offset[entry.axis][axis]);
    result.current[entry.axis] += entry.value
        * std::polar(1.0, -phase) / normalization;
    result.input_l1 += std::abs(entry.value) / normalization;
  }
  finalize(result);
  return result;
}

MatchedFaceCurrentSpectrum observe_dense_face_current_spectrum(
    const MatchedFaceFlux& current,
    const std::array<double, 3>& wavevector,
    double normalization) {
  const int L = current.L;
  const auto expected = static_cast<std::size_t>(L) * L * L;
  if (L < 5 || current.x.size() != expected || current.y.size() != expected
      || current.z.size() != expected)
    return {};
  std::vector<QuadraticCoatSparseCurrentEntry> entries;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto index = static_cast<std::size_t>(current.index(x, y, z));
        const std::array<double, 3> value{{
            current.x[index], current.y[index], current.z[index]}};
        for (int axis = 0; axis < 3; ++axis) {
          if (!std::isfinite(value[axis])) return {};
          if (value[axis] != 0.0)
            entries.push_back({{x, y, z}, axis, value[axis]});
        }
      }
    }
  }
  return observe_sparse_face_current_spectrum(
      L, entries, wavevector, normalization);
}

}  // namespace ftd::eft
