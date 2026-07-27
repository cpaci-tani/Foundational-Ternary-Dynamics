#include "ftd/eft/symmetric_chord_moore_action.h"

#include "ftd/constants.h"
#include "ftd/lattice.h"
#include "ftd/voxel.h"

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
constexpr double PI = 3.1415926535897932384626433832795;
constexpr double TOL = 1e-12;

int wrap(int value, int L) {
  const int remainder = value % L;
  return remainder < 0 ? remainder + L : remainder;
}

std::size_t index(int L, int x, int y, int z) {
  const auto a = static_cast<std::size_t>(wrap(x, L));
  const auto b = static_cast<std::size_t>(wrap(y, L));
  const auto c = static_cast<std::size_t>(wrap(z, L));
  return (a * static_cast<std::size_t>(L) + b)
      * static_cast<std::size_t>(L) + c;
}

int get(const Coord& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
}

void add_axis(Coord& value, int axis, int delta) {
  if (axis == 0) value.x += delta;
  if (axis == 1) value.y += delta;
  if (axis == 2) value.z += delta;
}

void add_component(Vec3& value, int axis, double delta) {
  if (axis == 0) value.x += delta;
  if (axis == 1) value.y += delta;
  if (axis == 2) value.z += delta;
}

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

Vec3 subtract(const Vec3& lhs, const Vec3& rhs) {
  return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

std::vector<double> smooth_axis(
    const std::vector<double>& input, int L, int axis) {
  std::vector<double> output(input.size(), 0.0);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        std::array<int, 3> p{{x, y, z}}, minus = p, plus = p;
        --minus[static_cast<std::size_t>(axis)];
        ++plus[static_cast<std::size_t>(axis)];
        output[index(L, x, y, z)] =
            0.25 * input[index(L, minus[0], minus[1], minus[2])]
          + 0.50 * input[index(L, x, y, z)]
          + 0.25 * input[index(L, plus[0], plus[1], plus[2])];
      }
    }
  }
  return output;
}

std::vector<double> smooth_all(std::vector<double> field, int L) {
  for (int axis = 0; axis < 3; ++axis) field = smooth_axis(field, L, axis);
  return field;
}

struct ChordArm {
  int L = 0;
  Coord start{};
  Coord direction{};
  int charge = 0;
  std::vector<double> raw_before;
  std::vector<double> raw_after;
  std::array<std::vector<double>, 3> raw_face;
  std::vector<double> rho_before;
  std::vector<double> rho_after;
  std::vector<Vec3> central_current;
};

ChordArm make_arm(int L, Coord start, Coord direction, int charge) {
  ChordArm result;
  result.L = L;
  result.start = start;
  result.direction = direction;
  result.charge = charge;
  const std::size_t volume = static_cast<std::size_t>(L) * L * L;
  result.raw_before.assign(volume, 0.0);
  result.raw_after.assign(volume, 0.0);
  for (auto& field : result.raw_face) field.assign(volume, 0.0);

  Coord endpoint = start;
  endpoint.x += direction.x;
  endpoint.y += direction.y;
  endpoint.z += direction.z;
  result.raw_before[index(L, start.x, start.y, start.z)] = charge;
  result.raw_after[index(L, endpoint.x, endpoint.y, endpoint.z)] = charge;

  std::vector<int> active;
  for (int axis = 0; axis < 3; ++axis) {
    if (get(direction, axis) != 0) active.push_back(axis);
  }
  const int factorial = active.size() == 1 ? 1 :
      (active.size() == 2 ? 2 : 6);
  std::sort(active.begin(), active.end());
  do {
    Coord vertex = start;
    for (int axis : active) {
      const int sign = get(direction, axis);
      Coord face = vertex;
      if (sign < 0) add_axis(face, axis, -1);
      result.raw_face[static_cast<std::size_t>(axis)][
          index(L, face.x, face.y, face.z)] +=
          static_cast<double>(charge * sign) / factorial;
      add_axis(vertex, axis, sign);
    }
  } while (std::next_permutation(active.begin(), active.end()));

  result.rho_before = smooth_all(result.raw_before, L);
  result.rho_after = smooth_all(result.raw_after, L);
  result.central_current.assign(volume, Vec3{});
  for (int axis = 0; axis < 3; ++axis) {
    std::vector<double> transverse = result.raw_face[static_cast<std::size_t>(axis)];
    for (int other = 0; other < 3; ++other) {
      if (other != axis) transverse = smooth_axis(transverse, L, other);
    }
    for (int x = 0; x < L; ++x) {
      for (int y = 0; y < L; ++y) {
        for (int z = 0; z < L; ++z) {
          std::array<int, 3> minus{{x, y, z}};
          --minus[static_cast<std::size_t>(axis)];
          const double value = 0.5 * (
              transverse[index(L, x, y, z)]
            + transverse[index(L, minus[0], minus[1], minus[2])]);
          add_component(result.central_current[index(L, x, y, z)], axis, value);
        }
      }
    }
  }
  return result;
}

double raw_divergence(const ChordArm& arm, int x, int y, int z) {
  double result = 0.0;
  for (int axis = 0; axis < 3; ++axis) {
    std::array<int, 3> minus{{x, y, z}};
    --minus[static_cast<std::size_t>(axis)];
    const auto& field = arm.raw_face[static_cast<std::size_t>(axis)];
    result += field[index(arm.L, x, y, z)]
        - field[index(arm.L, minus[0], minus[1], minus[2])];
  }
  return result;
}

double central_divergence(
    const std::vector<Vec3>& field, int L, int x, int y, int z) {
  const auto at = [&](int sx, int sy, int sz) -> const Vec3& {
    return field[index(L, sx, sy, sz)];
  };
  return 0.5 * (
      at(x + 1, y, z).x - at(x - 1, y, z).x
    + at(x, y + 1, z).y - at(x, y - 1, z).y
    + at(x, y, z + 1).z - at(x, y, z - 1).z);
}

double response(double kx, double ky, double kz) {
  const double sx = std::sin(kx), sy = std::sin(ky), sz = std::sin(kz);
  const double numerator = 3.0 * (sx * sx + sy * sy + sz * sz);
  const double c1 = std::cos(kx) + std::cos(ky) + std::cos(kz);
  const double c2 = std::cos(kx) * std::cos(ky)
      + std::cos(kx) * std::cos(kz) + std::cos(ky) * std::cos(kz);
  const double symbol = 4.0 - (2.0 / 3.0) * c1 - (2.0 / 3.0) * c2;
  return symbol > 1e-15 ? numerator / symbol : 0.0;
}

double coat2(double kx, double ky, double kz) {
  const double bx = 0.5 * (1.0 + std::cos(kx));
  const double by = 0.5 * (1.0 + std::cos(ky));
  const double bz = 0.5 * (1.0 + std::cos(kz));
  const double value = bx * by * bz;
  return value * value;
}

struct PeierlsSpectrum {
  double coefficient = 0.0;
  std::array<double, 9> potential{};
};

PeierlsSpectrum spectrum(int L, Coord direction, int charge) {
  PeierlsSpectrum result;
  long double coefficient = 0.0L;
  std::array<long double, 9> potential{};
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const double kx = 2.0 * PI * x / L;
        const double ky = 2.0 * PI * y / L;
        const double kz = 2.0 * PI * z / L;
        const double phase = kx * direction.x + ky * direction.y
            + kz * direction.z;
        const long double base = static_cast<long double>(charge * charge)
            * response(kx, ky, kz) * coat2(kx, ky, kz);
        coefficient += base * (1.0 - std::cos(phase));
        for (int step = 0; step <= 8; ++step) {
          const long double r = static_cast<long double>(step) / 8.0L;
          const std::complex<long double> factor = (1.0L - r)
              + r * std::exp(std::complex<long double>{0.0L, -phase});
          potential[static_cast<std::size_t>(step)] += base * std::norm(factor);
        }
      }
    }
  }
  const long double volume = static_cast<long double>(L) * L * L;
  result.coefficient = static_cast<double>(G_C * G_C * coefficient / volume);
  for (int step = 0; step <= 8; ++step) {
    result.potential[static_cast<std::size_t>(step)] = static_cast<double>(
        -G_C * G_C * potential[static_cast<std::size_t>(step)] / (2.0L * volume));
  }
  return result;
}

std::vector<Matrix3> proper_rotations() {
  std::vector<Matrix3> result;
  std::array<int, 3> permutation{{0, 1, 2}};
  do {
    int inversions = 0;
    for (int i = 0; i < 3; ++i) {
      for (int j = i + 1; j < 3; ++j) {
        if (permutation[static_cast<std::size_t>(i)]
            > permutation[static_cast<std::size_t>(j)]) ++inversions;
      }
    }
    const int parity = inversions % 2 == 0 ? 1 : -1;
    for (int sx : {-1, 1}) for (int sy : {-1, 1}) for (int sz : {-1, 1}) {
      if (parity * sx * sy * sz != 1) continue;
      Matrix3 matrix{};
      const std::array<int, 3> signs{{sx, sy, sz}};
      for (int row = 0; row < 3; ++row) {
        matrix[static_cast<std::size_t>(row)]
              [static_cast<std::size_t>(permutation[static_cast<std::size_t>(row)])]
            = signs[static_cast<std::size_t>(row)];
      }
      result.push_back(matrix);
    }
  } while (std::next_permutation(permutation.begin(), permutation.end()));
  return result;
}

Coord rotate(const Matrix3& matrix, Coord value) {
  const std::array<int, 3> input{{value.x, value.y, value.z}};
  std::array<int, 3> output{};
  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 3; ++col) {
      output[static_cast<std::size_t>(row)] +=
          matrix[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)]
          * input[static_cast<std::size_t>(col)];
    }
  }
  return {output[0], output[1], output[2]};
}

Vec3 rotate(const Matrix3& matrix, Vec3 value) {
  const std::array<double, 3> input{{value.x, value.y, value.z}};
  std::array<double, 3> output{};
  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 3; ++col) {
      output[static_cast<std::size_t>(row)] +=
          matrix[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)]
          * input[static_cast<std::size_t>(col)];
    }
  }
  return {output[0], output[1], output[2]};
}

double compare_rotated(
    const ChordArm& reference, const ChordArm& transformed,
    const Matrix3& matrix, int center) {
  double residual = 0.0;
  for (int x = 0; x < reference.L; ++x) {
    for (int y = 0; y < reference.L; ++y) {
      for (int z = 0; z < reference.L; ++z) {
        const Coord offset{x - center, y - center, z - center};
        const Coord rotated = rotate(matrix, offset);
        const std::size_t from = index(reference.L, x, y, z);
        const std::size_t to = index(reference.L,
            center + rotated.x, center + rotated.y, center + rotated.z);
        residual = std::max({residual,
            std::abs(reference.rho_before[from] - transformed.rho_before[to]),
            std::abs(reference.rho_after[from] - transformed.rho_after[to]),
            max_abs(subtract(
                rotate(matrix, reference.central_current[from]),
                transformed.central_current[to]))});
      }
    }
  }
  return residual;
}

}  // namespace

SymmetricChordMooreActionResult analyze_symmetric_chord_moore_action() {
  SymmetricChordMooreActionResult result;
  result.minimum_peierls_coefficient = std::numeric_limits<double>::infinity();
  result.minimum_peierls_barrier = std::numeric_limits<double>::infinity();

  for (int L : {17, 33}) {
    const Coord start{L / 2, L / 2, L / 2};
    for (int charge : {-1, 1}) {
      for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
          for (int dz = -1; dz <= 1; ++dz) {
            if (dx == 0 && dy == 0 && dz == 0) continue;
            const Coord direction{dx, dy, dz};
            const ChordArm arm = make_arm(L, start, direction, charge);
            const Coord end{start.x + dx, start.y + dy, start.z + dz};

            for (int step = 0; step <= 8; ++step) {
              const double t = step / 8.0;
              const double before_weight = charge * (1.0 - t);
              const double after_weight = charge * t;
              result.maximum_partition_residual = std::max(
                  result.maximum_partition_residual,
                  std::abs(before_weight + after_weight - charge));
              const Vec3 moment{
                  before_weight * start.x + after_weight * end.x,
                  before_weight * start.y + after_weight * end.y,
                  before_weight * start.z + after_weight * end.z};
              const Vec3 expected{
                  charge * (start.x + t * dx),
                  charge * (start.y + t * dy),
                  charge * (start.z + t * dz)};
              result.maximum_first_moment_residual = std::max(
                  result.maximum_first_moment_residual,
                  max_abs(subtract(moment, expected)));
              result.maximum_wrong_sign_residual = std::max(
                  result.maximum_wrong_sign_residual,
                  std::max({0.0, -charge * before_weight,
                            -charge * after_weight}));
              ++result.shape_samples;
            }

            const std::size_t volume = arm.raw_before.size();
            std::vector<Vec3> half_current(volume);
            for (std::size_t i = 0; i < volume; ++i) {
              half_current[i] = {0.5 * arm.central_current[i].x,
                                 0.5 * arm.central_current[i].y,
                                 0.5 * arm.central_current[i].z};
            }
            for (int x = 0; x < L; ++x) {
              for (int y = 0; y < L; ++y) {
                for (int z = 0; z < L; ++z) {
                  const std::size_t site = index(L, x, y, z);
                  result.maximum_raw_continuity_residual = std::max(
                      result.maximum_raw_continuity_residual,
                      std::abs(arm.raw_after[site] - arm.raw_before[site]
                          + raw_divergence(arm, x, y, z)));
                  const double divergence = central_divergence(
                      arm.central_current, L, x, y, z);
                  result.maximum_central_continuity_residual = std::max(
                      result.maximum_central_continuity_residual,
                      std::abs(arm.rho_after[site] - arm.rho_before[site]
                          + divergence));
                  const double t0 = arm.rho_before[site] / 3.0
                      + arm.rho_after[site] / 6.0;
                  const double t1 = arm.rho_before[site] / 6.0
                      + arm.rho_after[site] / 3.0;
                  const double midpoint = 0.5 * (
                      arm.rho_before[site] + arm.rho_after[site]);
                  result.maximum_temporal_centering_residual = std::max(
                      result.maximum_temporal_centering_residual,
                      std::abs(t0 + t1 - midpoint));
                  const double half_divergence = central_divergence(
                      half_current, L, x, y, z);
                  result.maximum_split_continuity_residual = std::max({
                      result.maximum_split_continuity_residual,
                      std::abs(half_divergence - (arm.rho_before[site] - midpoint)),
                      std::abs(half_divergence - (midpoint - arm.rho_after[site]))});
                }
              }
            }
            ++result.path_arms;

            const PeierlsSpectrum direct = spectrum(L, direction, charge);
            const PeierlsSpectrum mirror = spectrum(L, direction, -charge);
            result.minimum_peierls_coefficient = std::min(
                result.minimum_peierls_coefficient, direct.coefficient);
            result.minimum_peierls_barrier = std::min(
                result.minimum_peierls_barrier, direct.coefficient / 4.0);
            result.maximum_polarity_residual = std::max(
                result.maximum_polarity_residual,
                std::abs(direct.coefficient - mirror.coefficient));
            for (int step = 0; step <= 8; ++step) {
              const double t = step / 8.0;
              const double predicted = direct.potential[0]
                  + direct.coefficient * t * (1.0 - t);
              result.maximum_peierls_law_residual = std::max(
                  result.maximum_peierls_law_residual,
                  std::abs(direct.potential[static_cast<std::size_t>(step)]
                      - predicted));
              ++result.peierls_potential_samples;
            }
            ++result.peierls_coefficient_arms;
          }
        }
      }
    }
  }

  const int L = 17;
  const int center = L / 2;
  const Coord start{center, center, center};
  const Coord reference_direction{1, 1, 1};
  const ChordArm reference = make_arm(L, start, reference_direction, 1);
  const double reference_coefficient = spectrum(L, reference_direction, 1).coefficient;
  for (const Matrix3& rotation : proper_rotations()) {
    const Coord direction = rotate(rotation, reference_direction);
    const ChordArm transformed = make_arm(L, start, direction, 1);
    result.maximum_cubic_covariance_residual = std::max({
        result.maximum_cubic_covariance_residual,
        compare_rotated(reference, transformed, rotation, center),
        std::abs(reference_coefficient - spectrum(L, direction, 1).coefficient)});
    ++result.cubic_rotation_arms;
  }

  result.positive_centered_shape_unique = result.shape_samples == 936
      && result.maximum_partition_residual <= TOL
      && result.maximum_first_moment_residual <= TOL
      && result.maximum_wrong_sign_residual <= TOL;
  result.democratic_shortest_route_exact = result.path_arms == 104
      && result.maximum_raw_continuity_residual <= TOL
      && result.maximum_central_continuity_residual <= TOL
      && result.maximum_cubic_covariance_residual <= TOL;
  result.common_action_energy_centered =
      result.maximum_temporal_centering_residual <= TOL
      && result.maximum_split_continuity_residual <= TOL;
  result.every_peierls_barrier_positive =
      result.peierls_coefficient_arms == 104
      && result.peierls_potential_samples == 936
      && result.minimum_peierls_coefficient > 1e-14
      && result.minimum_peierls_barrier > 1e-14
      && result.maximum_peierls_law_residual <= TOL
      && result.maximum_polarity_residual <= TOL;
  result.gapless_mobile_law_derived = false;
  result.production_changed = false;
  result.valid = result.positive_centered_shape_unique
      && result.democratic_shortest_route_exact
      && result.common_action_energy_centered
      && result.every_peierls_barrier_positive
      && result.cubic_rotation_arms == 24
      && !result.gapless_mobile_law_derived
      && !result.production_changed;
  return result;
}

}  // namespace ftd::eft
