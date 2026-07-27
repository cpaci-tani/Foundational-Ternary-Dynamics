#include "ftd/eft/noncompact_face_cohomology.h"

#include "ftd/eft/matched_gauss_transport.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <vector>

namespace ftd::eft {
namespace {

using Matrix3 = std::array<std::array<int, 3>, 3>;
using Complex = std::complex<double>;
constexpr double PI = 3.1415926535897932384626433832795;
constexpr double TOL = 1e-12;

int wrap(int value, int L) {
  const int remainder = value % L;
  return remainder < 0 ? remainder + L : remainder;
}

std::vector<double>& component(MatchedFaceFlux& field, int axis) {
  return axis == 0 ? field.x : (axis == 1 ? field.y : field.z);
}

const std::vector<double>& component(const MatchedFaceFlux& field, int axis) {
  return axis == 0 ? field.x : (axis == 1 ? field.y : field.z);
}

std::vector<double>& component(MatchedEdgeField& field, int axis) {
  return axis == 0 ? field.x : (axis == 1 ? field.y : field.z);
}

const std::vector<double>& component(const MatchedEdgeField& field, int axis) {
  return axis == 0 ? field.x : (axis == 1 ? field.y : field.z);
}

double vec_max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

Vec3 rotate(const Matrix3& matrix, const Vec3& value) {
  const std::array<double, 3> input{{value.x, value.y, value.z}};
  std::array<double, 3> output{};
  for (int row = 0; row < 3; ++row)
    for (int column = 0; column < 3; ++column)
      output[static_cast<std::size_t>(row)] +=
          matrix[static_cast<std::size_t>(row)]
                [static_cast<std::size_t>(column)]
          * input[static_cast<std::size_t>(column)];
  return {output[0], output[1], output[2]};
}

std::array<int, 3> rotate(const Matrix3& matrix,
                          const std::array<int, 3>& value) {
  std::array<int, 3> output{};
  for (int row = 0; row < 3; ++row)
    for (int column = 0; column < 3; ++column)
      output[static_cast<std::size_t>(row)] +=
          matrix[static_cast<std::size_t>(row)]
                [static_cast<std::size_t>(column)]
          * value[static_cast<std::size_t>(column)];
  return output;
}

std::vector<Matrix3> proper_rotations() {
  std::vector<Matrix3> result;
  std::array<int, 3> permutation{{0, 1, 2}};
  do {
    int inversions = 0;
    for (int i = 0; i < 3; ++i)
      for (int j = i + 1; j < 3; ++j)
        if (permutation[static_cast<std::size_t>(i)]
            > permutation[static_cast<std::size_t>(j)]) ++inversions;
    const int parity = inversions % 2 == 0 ? 1 : -1;
    for (int sx : {-1, 1}) for (int sy : {-1, 1}) for (int sz : {-1, 1}) {
      if (parity * sx * sy * sz != 1) continue;
      Matrix3 matrix{};
      const std::array<int, 3> signs{{sx, sy, sz}};
      for (int row = 0; row < 3; ++row)
        matrix[static_cast<std::size_t>(row)]
              [static_cast<std::size_t>(
                  permutation[static_cast<std::size_t>(row)])] =
            signs[static_cast<std::size_t>(row)];
      result.push_back(matrix);
    }
  } while (std::next_permutation(permutation.begin(), permutation.end()));
  return result;
}

int complex_rank(std::array<std::array<Complex, 3>, 3> matrix,
                 int rows, int columns) {
  int pivot_row = 0;
  for (int column = 0; column < columns && pivot_row < rows; ++column) {
    int pivot = pivot_row;
    for (int row = pivot_row + 1; row < rows; ++row) {
      if (std::abs(matrix[static_cast<std::size_t>(row)]
                         [static_cast<std::size_t>(column)])
          > std::abs(matrix[static_cast<std::size_t>(pivot)]
                           [static_cast<std::size_t>(column)])) {
        pivot = row;
      }
    }
    if (std::abs(matrix[static_cast<std::size_t>(pivot)]
                       [static_cast<std::size_t>(column)]) <= TOL) {
      continue;
    }
    std::swap(matrix[static_cast<std::size_t>(pivot_row)],
              matrix[static_cast<std::size_t>(pivot)]);
    const Complex pivot_value =
        matrix[static_cast<std::size_t>(pivot_row)]
              [static_cast<std::size_t>(column)];
    for (int entry = column; entry < columns; ++entry) {
      matrix[static_cast<std::size_t>(pivot_row)]
            [static_cast<std::size_t>(entry)] /= pivot_value;
    }
    for (int row = 0; row < rows; ++row) {
      if (row == pivot_row) continue;
      const Complex factor =
          matrix[static_cast<std::size_t>(row)]
                [static_cast<std::size_t>(column)];
      for (int entry = column; entry < columns; ++entry) {
        matrix[static_cast<std::size_t>(row)]
              [static_cast<std::size_t>(entry)] -=
            factor * matrix[static_cast<std::size_t>(pivot_row)]
                           [static_cast<std::size_t>(entry)];
      }
    }
    ++pivot_row;
  }
  return pivot_row;
}

MatchedEdgeField rotate_cochain(const MatchedEdgeField& source,
                                const Matrix3& matrix) {
  MatchedEdgeField target(source.L);
  for (int x = 0; x < source.L; ++x) {
    for (int y = 0; y < source.L; ++y) {
      for (int z = 0; z < source.L; ++z) {
        const int source_index = source.index(x, y, z);
        const auto rotated_site = rotate(matrix, std::array<int, 3>{{x, y, z}});
        for (int source_axis = 0; source_axis < 3; ++source_axis) {
          int target_axis = -1;
          int sign = 0;
          for (int row = 0; row < 3; ++row) {
            const int value = matrix[static_cast<std::size_t>(row)]
                                    [static_cast<std::size_t>(source_axis)];
            if (value != 0) {
              target_axis = row;
              sign = value;
              break;
            }
          }
          std::array<int, 3> target_site = rotated_site;
          // Edge component A_i(n) is the +i edge whose terminal vertex is n.
          // A reflected edge terminates one positive target step beyond Rn.
          if (sign < 0) ++target_site[static_cast<std::size_t>(target_axis)];
          const int target_index = target.index(
              target_site[0], target_site[1], target_site[2]);
          component(target, target_axis)[static_cast<std::size_t>(target_index)]
              += sign * component(source, source_axis)
                              [static_cast<std::size_t>(source_index)];
        }
      }
    }
  }
  return target;
}

MatchedFaceFlux rotate_cochain(const MatchedFaceFlux& source,
                               const Matrix3& matrix) {
  MatchedFaceFlux target(source.L);
  for (int x = 0; x < source.L; ++x) {
    for (int y = 0; y < source.L; ++y) {
      for (int z = 0; z < source.L; ++z) {
        const int source_index = source.index(x, y, z);
        const auto rotated_site = rotate(matrix, std::array<int, 3>{{x, y, z}});
        for (int source_axis = 0; source_axis < 3; ++source_axis) {
          int target_axis = -1;
          int sign = 0;
          for (int row = 0; row < 3; ++row) {
            const int value = matrix[static_cast<std::size_t>(row)]
                                    [static_cast<std::size_t>(source_axis)];
            if (value != 0) {
              target_axis = row;
              sign = value;
              break;
            }
          }
          std::array<int, 3> target_site = rotated_site;
          // Face component E_i(n) is the +i-oriented face whose terminal
          // vertex is n in both transverse coordinates. Reflections of a
          // transverse source axis move that terminal vertex forward.
          for (int transverse = 0; transverse < 3; ++transverse) {
            if (transverse == source_axis) continue;
            for (int row = 0; row < 3; ++row) {
              if (matrix[static_cast<std::size_t>(row)]
                        [static_cast<std::size_t>(transverse)] < 0) {
                ++target_site[static_cast<std::size_t>(row)];
              }
            }
          }
          const int target_index = target.index(
              target_site[0], target_site[1], target_site[2]);
          component(target, target_axis)[static_cast<std::size_t>(target_index)]
              += sign * component(source, source_axis)
                              [static_cast<std::size_t>(source_index)];
        }
      }
    }
  }
  return target;
}

double max_difference(const MatchedFaceFlux& lhs,
                      const MatchedFaceFlux& rhs) {
  if (lhs.L != rhs.L) return std::numeric_limits<double>::infinity();
  double residual = 0.0;
  for (int axis = 0; axis < 3; ++axis) {
    const auto& a = component(lhs, axis);
    const auto& b = component(rhs, axis);
    for (std::size_t i = 0; i < a.size(); ++i)
      residual = std::max(residual, std::abs(a[i] - b[i]));
  }
  return residual;
}

MatchedFaceFlux scaled(const MatchedFaceFlux& source, double scale) {
  MatchedFaceFlux result(source.L);
  for (int axis = 0; axis < 3; ++axis)
    for (std::size_t i = 0; i < component(source, axis).size(); ++i)
      component(result, axis)[i] = scale * component(source, axis)[i];
  return result;
}

MatchedEdgeField scaled(const MatchedEdgeField& source, double scale) {
  MatchedEdgeField result(source.L);
  for (int axis = 0; axis < 3; ++axis)
    for (std::size_t i = 0; i < component(source, axis).size(); ++i)
      component(result, axis)[i] = scale * component(source, axis)[i];
  return result;
}

void add(MatchedFaceFlux& target, const MatchedFaceFlux& value) {
  for (int axis = 0; axis < 3; ++axis)
    for (std::size_t i = 0; i < component(target, axis).size(); ++i)
      component(target, axis)[i] += component(value, axis)[i];
}

int support(const MatchedFaceFlux& field) {
  int result = 0;
  for (int axis = 0; axis < 3; ++axis)
    for (double value : component(field, axis))
      if (value != 0.0) ++result;
  return result;
}

int support_excess(const MatchedFaceFlux& reference,
                   const MatchedFaceFlux& candidate) {
  int result = 0;
  for (int axis = 0; axis < 3; ++axis)
    for (std::size_t i = 0; i < component(reference, axis).size(); ++i)
      if (component(reference, axis)[i] == 0.0
          && component(candidate, axis)[i] != 0.0) ++result;
  return result;
}

struct PlaneFlux {
  Vec3 coefficient{};
  double plane_residual = 0.0;
};

PlaneFlux plane_flux(const MatchedFaceFlux& field) {
  PlaneFlux result;
  for (int axis = 0; axis < 3; ++axis) {
    double reference = 0.0;
    for (int plane = 0; plane < field.L; ++plane) {
      long double sum = 0.0L;
      for (int a = 0; a < field.L; ++a) {
        for (int b = 0; b < field.L; ++b) {
          int x = a, y = b, z = plane;
          if (axis == 0) { x = plane; y = a; z = b; }
          if (axis == 1) { x = a; y = plane; z = b; }
          const int i = field.index(x, y, z);
          sum += component(field, axis)[static_cast<std::size_t>(i)];
        }
      }
      const double coefficient = static_cast<double>(sum)
          / static_cast<double>(field.L * field.L);
      if (plane == 0) {
        reference = coefficient;
        if (axis == 0) result.coefficient.x = coefficient;
        if (axis == 1) result.coefficient.y = coefficient;
        if (axis == 2) result.coefficient.z = coefficient;
      } else {
        result.plane_residual = std::max(
            result.plane_residual, std::abs(coefficient - reference));
      }
    }
  }
  return result;
}

MatchedFaceFlux constant_harmonic(int L, const Vec3& value) {
  MatchedFaceFlux result(L);
  std::fill(result.x.begin(), result.x.end(), value.x);
  std::fill(result.y.begin(), result.y.end(), value.y);
  std::fill(result.z.begin(), result.z.end(), value.z);
  return result;
}

MatchedEdgeField local_potential(int L, int axis, int which) {
  MatchedEdgeField result(L);
  const int c = L / 2;
  const std::array<int, 3> site = which == 0
      ? std::array<int, 3>{{c, c, c}}
      : std::array<int, 3>{{1 % L, 2 % L, (L - 1) % L}};
  const int i = result.index(site[0], site[1], site[2]);
  component(result, axis)[static_cast<std::size_t>(i)] =
      which == 0 ? 0.375 : -0.21875;
  return result;
}

double total_divergence(const MatchedFaceFlux& field) {
  long double result = 0.0L;
  for (int x = 0; x < field.L; ++x)
    for (int y = 0; y < field.L; ++y)
      for (int z = 0; z < field.L; ++z)
        result += divergence_at(field, x, y, z);
  return static_cast<double>(result);
}

}  // namespace

NoncompactFaceCohomologyResult analyze_noncompact_face_cohomology() {
  NoncompactFaceCohomologyResult result;
  result.minimum_localized_support = std::numeric_limits<int>::max();
  result.minimum_nonzero_localized_energy =
      std::numeric_limits<double>::infinity();
  const std::array<int, 4> volumes{{3, 4, 5, 8}};

  const std::array<Complex, 3> witness{{
      Complex{0.37, -0.11}, Complex{-0.23, 0.19}, Complex{0.17, 0.29}}};
  for (int L : volumes) {
    std::array<int, 4> local_betti{{0, 0, 0, 0}};
    for (int nx = 0; nx < L; ++nx) {
      for (int ny = 0; ny < L; ++ny) {
        for (int nz = 0; nz < L; ++nz) {
          const std::array<int, 3> n{{nx, ny, nz}};
          std::array<Complex, 3> q{};
          double norm2 = 0.0;
          for (int axis = 0; axis < 3; ++axis) {
            const double k = 2.0 * PI * n[static_cast<std::size_t>(axis)] / L;
            q[static_cast<std::size_t>(axis)] =
                Complex{1.0, 0.0} - std::exp(Complex{0.0, -k});
            norm2 += std::norm(q[static_cast<std::size_t>(axis)]);
          }
          const std::array<Complex, 3> curl{{
              q[1] * witness[2] - q[2] * witness[1],
              q[2] * witness[0] - q[0] * witness[2],
              q[0] * witness[1] - q[1] * witness[0]}};
          std::array<std::array<Complex, 3>, 3> d0{};
          std::array<std::array<Complex, 3>, 3> d1{};
          std::array<std::array<Complex, 3>, 3> d2{};
          for (int axis = 0; axis < 3; ++axis) {
            d0[static_cast<std::size_t>(axis)][0] =
                q[static_cast<std::size_t>(axis)];
            d2[0][static_cast<std::size_t>(axis)] =
                q[static_cast<std::size_t>(axis)];
          }
          d1[0][1] = -q[2]; d1[0][2] = q[1];
          d1[1][0] = q[2];  d1[1][2] = -q[0];
          d1[2][0] = -q[1]; d1[2][1] = q[0];
          const int rank_d0 = complex_rank(d0, 3, 1);
          const int rank_d1 = complex_rank(d1, 3, 3);
          const int rank_d2 = complex_rank(d2, 1, 3);
          local_betti[0] += 1 - rank_d0;
          local_betti[1] += (3 - rank_d1) - rank_d0;
          local_betti[2] += (3 - rank_d2) - rank_d1;
          local_betti[3] += 1 - rank_d2;
          result.maximum_symbol_complex_residual = std::max(
              result.maximum_symbol_complex_residual,
              std::abs(q[0] * curl[0] + q[1] * curl[1]
                       + q[2] * curl[2]));
          if (norm2 <= 1e-24) {
            ++result.zero_momentum_mode_arms;
            if (rank_d0 != 0 || rank_d1 != 0 || rank_d2 != 0)
              ++result.fourier_rank_mismatches;
          } else {
            ++result.nonzero_momentum_mode_arms;
            if (rank_d0 != 1 || rank_d1 != 2 || rank_d2 != 1)
              ++result.fourier_rank_mismatches;
          }
          ++result.fourier_mode_arms;
        }
      }
    }
    const std::array<int, 4> expected_betti{{1, 3, 3, 1}};
    if (local_betti != expected_betti) ++result.betti_volume_mismatches;
    result.betti_0 = local_betti[0];
    result.betti_1 = local_betti[1];
    result.betti_2 = local_betti[2];
    result.betti_3 = local_betti[3];
  }

  const std::array<double, 4> harmonic_amplitudes{{-1.0, -0.5, 0.5, 1.0}};
  for (int L : volumes) {
    for (int axis = 0; axis < 3; ++axis) {
      const MatchedFaceFlux curl = matched_curl(local_potential(L, axis, 0));
      const PlaneFlux curl_flux = plane_flux(curl);
      result.maximum_divergence_of_curl = std::max(
          result.maximum_divergence_of_curl, max_divergence(curl));
      result.maximum_curl_plane_flux = std::max(
          result.maximum_curl_plane_flux,
          std::max(curl_flux.plane_residual,
                   vec_max_abs(curl_flux.coefficient)));
      for (double amplitude : harmonic_amplitudes) {
        Vec3 value{};
        if (axis == 0) value.x = amplitude;
        if (axis == 1) value.y = amplitude;
        if (axis == 2) value.z = amplitude;
        MatchedFaceFlux harmonic = constant_harmonic(L, value);
        const PlaneFlux before = plane_flux(harmonic);
        add(harmonic, curl);
        const PlaneFlux after = plane_flux(harmonic);
        result.maximum_harmonic_plane_residual = std::max({
            result.maximum_harmonic_plane_residual,
            before.plane_residual, after.plane_residual,
            vec_max_abs(before.coefficient - value)});
        result.maximum_harmonic_flux_change_under_curl = std::max(
            result.maximum_harmonic_flux_change_under_curl,
            vec_max_abs(after.coefficient - before.coefficient));
        ++result.harmonic_arms;
      }
    }
  }

  const std::array<double, 5> contraction{{0.0, 0.25, 0.5, 0.75, 1.0}};
  for (int L : volumes) {
    for (int axis = 0; axis < 3; ++axis) {
      for (int which = 0; which < 2; ++which) {
        const MatchedEdgeField potential = local_potential(L, axis, which);
        const MatchedFaceFlux field = matched_curl(potential);
        const int initial_support = support(field);
        const double initial_energy = quadratic_energy(field);
        result.minimum_localized_support = std::min(
            result.minimum_localized_support, initial_support);
        result.maximum_localized_support = std::max(
            result.maximum_localized_support, initial_support);
        result.minimum_nonzero_localized_energy = std::min(
            result.minimum_nonzero_localized_energy, initial_energy);
        result.maximum_divergence_of_curl = std::max(
            result.maximum_divergence_of_curl, max_divergence(field));
        const PlaneFlux base_flux = plane_flux(field);
        result.maximum_curl_plane_flux = std::max({
            result.maximum_curl_plane_flux, base_flux.plane_residual,
            vec_max_abs(base_flux.coefficient)});
        for (double t : contraction) {
          const MatchedFaceFlux contracted = scaled(field, t);
          const MatchedFaceFlux reconstructed = matched_curl(
              scaled(potential, t));
          const PlaneFlux flux = plane_flux(contracted);
          result.maximum_contraction_divergence = std::max(
              result.maximum_contraction_divergence,
              max_divergence(contracted));
          result.maximum_contraction_harmonic_flux = std::max({
              result.maximum_contraction_harmonic_flux,
              flux.plane_residual, vec_max_abs(flux.coefficient)});
          result.maximum_contraction_energy_residual = std::max(
              result.maximum_contraction_energy_residual,
              std::abs(quadratic_energy(contracted)
                       - t * t * initial_energy));
          result.maximum_contraction_curl_residual = std::max(
              result.maximum_contraction_curl_residual,
              max_difference(contracted, reconstructed));
          result.maximum_contraction_support_excess = std::max(
              result.maximum_contraction_support_excess,
              support_excess(field, contracted));
          ++result.contraction_samples;
        }
        ++result.localized_curl_arms;
      }
    }
  }

  const std::array<double, 5> charge_scales{{0.0, 0.25, 0.5, 0.75, 1.0}};
  for (int L : volumes) {
    const int c = L / 2;
    for (int axis = 0; axis < 3; ++axis) {
      for (int polarity : {-1, 1}) {
        for (double t : charge_scales) {
          MatchedFaceFlux field(L);
          std::array<int, 3> source{{c, c, c}};
          std::array<int, 3> sink = source;
          sink[static_cast<std::size_t>(axis)] = wrap(
              sink[static_cast<std::size_t>(axis)] + 1, L);
          const int source_index = field.index(source[0], source[1], source[2]);
          const int sink_index = field.index(sink[0], sink[1], sink[2]);
          if (t != 0.0)
            seed_dipole_path(field, source_index, sink_index, polarity * t);
          const double expected_source = polarity * t;
          const double expected_sink = -polarity * t;
          result.maximum_periodic_charge_sum = std::max(
              result.maximum_periodic_charge_sum,
              std::abs(total_divergence(field)));
          for (int x = 0; x < L; ++x) {
            for (int y = 0; y < L; ++y) {
              for (int z = 0; z < L; ++z) {
                const int i = field.index(x, y, z);
                const double value = divergence_at(field, x, y, z);
                if (i == source_index) {
                  result.maximum_charge_scaling_residual = std::max(
                      result.maximum_charge_scaling_residual,
                      std::abs(value - expected_source));
                } else if (i == sink_index) {
                  result.maximum_charge_scaling_residual = std::max(
                      result.maximum_charge_scaling_residual,
                      std::abs(value - expected_sink));
                } else {
                  result.maximum_off_source_divergence = std::max(
                      result.maximum_off_source_divergence, std::abs(value));
                }
              }
            }
          }
          const auto surface = measure_face_cube_charge(
              field, source[0], source[1], source[2], 0);
          result.maximum_surface_telescope_residual = std::max(
              result.maximum_surface_telescope_residual,
              std::abs(surface.telescope_residual));
          ++result.charge_scaling_arms;
        }
      }
    }
  }

  const int rotation_L = 7;
  MatchedEdgeField rotation_potential(rotation_L);
  component(rotation_potential, 0)[static_cast<std::size_t>(
      rotation_potential.index(2, 3, 1))] = 0.375;
  component(rotation_potential, 1)[static_cast<std::size_t>(
      rotation_potential.index(4, 1, 5))] = -0.21875;
  component(rotation_potential, 2)[static_cast<std::size_t>(
      rotation_potential.index(1, 5, 2))] = 0.15625;
  const MatchedFaceFlux rotation_curl = matched_curl(rotation_potential);
  const Vec3 harmonic_value{0.125, -0.1875, 0.25};
  const MatchedFaceFlux harmonic = constant_harmonic(
      rotation_L, harmonic_value);
  const double reference_energy = quadratic_energy(rotation_curl);
  for (const Matrix3& matrix : proper_rotations()) {
    const MatchedEdgeField rotated_potential = rotate_cochain(
        rotation_potential, matrix);
    const MatchedFaceFlux direct = matched_curl(rotated_potential);
    const MatchedFaceFlux expected = rotate_cochain(rotation_curl, matrix);
    const MatchedFaceFlux rotated_harmonic = rotate_cochain(harmonic, matrix);
    const PlaneFlux harmonic_flux = plane_flux(rotated_harmonic);
    const double curl_covariance = max_difference(direct, expected);
    const double rotated_divergence = max_divergence(direct);
    const double energy_residual =
        std::abs(quadratic_energy(direct) - reference_energy);
    const double harmonic_plane_residual = harmonic_flux.plane_residual;
    const double harmonic_rotation_residual =
        vec_max_abs(harmonic_flux.coefficient
                    - rotate(matrix, harmonic_value));
    result.maximum_curl_covariance_residual = std::max(
        result.maximum_curl_covariance_residual, curl_covariance);
    result.maximum_rotated_divergence = std::max(
        result.maximum_rotated_divergence, rotated_divergence);
    result.maximum_rotation_energy_residual = std::max(
        result.maximum_rotation_energy_residual, energy_residual);
    result.maximum_rotated_harmonic_plane_residual = std::max(
        result.maximum_rotated_harmonic_plane_residual,
        harmonic_plane_residual);
    result.maximum_harmonic_rotation_residual = std::max(
        result.maximum_harmonic_rotation_residual,
        harmonic_rotation_residual);
    result.maximum_cubic_covariance_residual = std::max({
        result.maximum_cubic_covariance_residual,
        curl_covariance, rotated_divergence, energy_residual,
        harmonic_plane_residual, harmonic_rotation_residual});
    ++result.cubic_rotation_arms;
  }

  result.periodic_complex_exact_off_zero_mode =
      result.fourier_mode_arms == 728
      && result.zero_momentum_mode_arms == 4
      && result.nonzero_momentum_mode_arms == 724
      && result.fourier_rank_mismatches == 0
      && result.betti_volume_mismatches == 0
      && result.maximum_symbol_complex_residual <= TOL;
  result.face_cohomology_is_three_global_real_fluxes =
      result.periodic_complex_exact_off_zero_mode
      && result.betti_0 == 1 && result.betti_1 == 3
      && result.betti_2 == 3 && result.betti_3 == 1
      && result.harmonic_arms == 48
      && result.maximum_divergence_of_curl <= TOL
      && result.maximum_curl_plane_flux <= TOL
      && result.maximum_harmonic_plane_residual <= TOL
      && result.maximum_harmonic_flux_change_under_curl <= TOL;
  result.localized_zero_harmonic_fields_contractible =
      result.localized_curl_arms == 24
      && result.contraction_samples == 120
      && result.minimum_localized_support > 0
      && result.maximum_localized_support < 32
      && result.minimum_nonzero_localized_energy > 0.0
      && result.maximum_contraction_divergence <= TOL
      && result.maximum_contraction_harmonic_flux <= TOL
      && result.maximum_contraction_energy_residual <= TOL
      && result.maximum_contraction_curl_residual <= TOL
      && result.maximum_contraction_support_excess == 0;
  result.real_gauss_charge_continuously_scalable =
      result.charge_scaling_arms == 120
      && result.maximum_periodic_charge_sum <= TOL
      && result.maximum_charge_scaling_residual <= TOL
      && result.maximum_off_source_divergence <= TOL
      && result.maximum_surface_telescope_residual <= TOL;
  result.localized_protected_carrier_in_current_variables = false;
  result.compact_u1_structure_derived = false;
  result.production_changed = false;
  result.valid = result.face_cohomology_is_three_global_real_fluxes
      && result.localized_zero_harmonic_fields_contractible
      && result.real_gauss_charge_continuously_scalable
      && result.cubic_rotation_arms == 24
      && result.maximum_cubic_covariance_residual <= TOL
      && !result.localized_protected_carrier_in_current_variables
      && !result.compact_u1_structure_derived
      && !result.production_changed;
  return result;
}

}  // namespace ftd::eft
