#include "ftd/eft/native_field_discrete_action.h"

#include "ftd/constants.h"
#include "ftd/voxel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numeric>
#include <vector>

namespace ftd::eft {

namespace {

using Matrix2 = std::array<std::array<double, 2>, 2>;
using Matrix3 = std::array<std::array<double, 3>, 3>;

struct PeriodicFields {
  int size = 0;
  std::vector<double> scalar;
  std::vector<Vec3> first;
  std::vector<Vec3> second;
  std::vector<Vec3> variation;
};

int wrap(int value, int size) {
  value %= size;
  return value < 0 ? value + size : value;
}

int index(int x, int y, int z, int size) {
  return (wrap(x, size) * size + wrap(y, size)) * size + wrap(z, size);
}

Vec3 add(const Vec3& lhs, const Vec3& rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

Vec3 subtract(const Vec3& lhs, const Vec3& rhs) {
  return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

Vec3 scale(const Vec3& value, double factor) {
  return {value.x * factor, value.y * factor, value.z * factor};
}

long double dot_ld(const Vec3& lhs, const Vec3& rhs) {
  return static_cast<long double>(lhs.x) * rhs.x
      + static_cast<long double>(lhs.y) * rhs.y
      + static_cast<long double>(lhs.z) * rhs.z;
}

double norm(const Vec3& value) {
  return std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
}

double max_component(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y), std::abs(value.z)});
}

Matrix2 multiply(const Matrix2& lhs, const Matrix2& rhs) {
  Matrix2 result{};
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 2; ++j) {
      for (int k = 0; k < 2; ++k) {
        result[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
            += lhs[static_cast<std::size_t>(i)][static_cast<std::size_t>(k)]
             * rhs[static_cast<std::size_t>(k)][static_cast<std::size_t>(j)];
      }
    }
  }
  return result;
}

Matrix2 transpose(const Matrix2& value) {
  return {{{value[0][0], value[1][0]}, {value[0][1], value[1][1]}}};
}

double matrix_residual(const Matrix2& lhs, const Matrix2& rhs) {
  double result = 0.0;
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 2; ++j) {
      result = std::max(result, std::abs(
          lhs[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
          - rhs[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]));
    }
  }
  return result;
}

int rank3(std::array<std::array<double, 3>, 3> value,
          double tolerance = 1e-11) {
  int rank = 0;
  for (int column = 0; column < 3 && rank < 3; ++column) {
    int pivot = rank;
    for (int row = rank + 1; row < 3; ++row) {
      if (std::abs(value[static_cast<std::size_t>(row)]
                         [static_cast<std::size_t>(column)])
          > std::abs(value[static_cast<std::size_t>(pivot)]
                           [static_cast<std::size_t>(column)])) {
        pivot = row;
      }
    }
    if (std::abs(value[static_cast<std::size_t>(pivot)]
                      [static_cast<std::size_t>(column)]) <= tolerance) {
      continue;
    }
    std::swap(value[static_cast<std::size_t>(rank)],
              value[static_cast<std::size_t>(pivot)]);
    const double divisor = value[static_cast<std::size_t>(rank)]
                                [static_cast<std::size_t>(column)];
    for (int j = column; j < 3; ++j) {
      value[static_cast<std::size_t>(rank)][static_cast<std::size_t>(j)]
          /= divisor;
    }
    for (int row = 0; row < 3; ++row) {
      if (row == rank) continue;
      const double factor = value[static_cast<std::size_t>(row)]
                                 [static_cast<std::size_t>(column)];
      for (int j = column; j < 3; ++j) {
        value[static_cast<std::size_t>(row)][static_cast<std::size_t>(j)]
            -= factor * value[static_cast<std::size_t>(rank)]
                           [static_cast<std::size_t>(j)];
      }
    }
    ++rank;
  }
  return rank;
}

double mode_symbol(int size, int mode, int direction) {
  constexpr double TWO_PI = 6.283185307179586476925286766559;
  const double k = TWO_PI * static_cast<double>(mode) / size;
  const double cx = std::cos(direction >= 0 ? k : 0.0);
  const double cy = std::cos(direction >= 1 ? k : 0.0);
  const double cz = std::cos(direction >= 2 ? k : 0.0);
  return 4.0 - (2.0 / 3.0) * (cx + cy + cz)
      - (2.0 / 3.0) * (cx * cy + cx * cz + cy * cz);
}

Vec3 gradient(const std::vector<double>& scalar, int size,
              int x, int y, int z) {
  return {
      0.5 * (scalar[static_cast<std::size_t>(index(x + 1, y, z, size))]
           - scalar[static_cast<std::size_t>(index(x - 1, y, z, size))]),
      0.5 * (scalar[static_cast<std::size_t>(index(x, y + 1, z, size))]
           - scalar[static_cast<std::size_t>(index(x, y - 1, z, size))]),
      0.5 * (scalar[static_cast<std::size_t>(index(x, y, z + 1, size))]
           - scalar[static_cast<std::size_t>(index(x, y, z - 1, size))])};
}

double divergence(const std::vector<Vec3>& field, int size,
                  int x, int y, int z) {
  return 0.5 * (
      field[static_cast<std::size_t>(index(x + 1, y, z, size))].x
      - field[static_cast<std::size_t>(index(x - 1, y, z, size))].x
      + field[static_cast<std::size_t>(index(x, y + 1, z, size))].y
      - field[static_cast<std::size_t>(index(x, y - 1, z, size))].y
      + field[static_cast<std::size_t>(index(x, y, z + 1, size))].z
      - field[static_cast<std::size_t>(index(x, y, z - 1, size))].z);
}

Vec3 curl(const std::vector<Vec3>& field, int size,
          int x, int y, int z) {
  const auto at = [&](int dx, int dy, int dz) -> const Vec3& {
    return field[static_cast<std::size_t>(index(x + dx, y + dy, z + dz, size))];
  };
  return {
      0.5 * (at(0, 1, 0).z - at(0, -1, 0).z
             - at(0, 0, 1).y + at(0, 0, -1).y),
      0.5 * (at(0, 0, 1).x - at(0, 0, -1).x
             - at(1, 0, 0).z + at(-1, 0, 0).z),
      0.5 * (at(1, 0, 0).y - at(-1, 0, 0).y
             - at(0, 1, 0).x + at(0, -1, 0).x)};
}

std::vector<Vec3> apply_k(const std::vector<Vec3>& field, int size) {
  std::vector<Vec3> result(field.size());
  const double c2 = C_WAVE * C_WAVE;
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      for (int z = 0; z < size; ++z) {
        Vec3 lap{};
        for (const auto& offset : std::array<std::array<int, 3>, 6>{
                 {{{1, 0, 0}}, {{-1, 0, 0}}, {{0, 1, 0}},
                  {{0, -1, 0}}, {{0, 0, 1}}, {{0, 0, -1}}}}) {
          lap = add(lap, scale(field[static_cast<std::size_t>(index(
              x + offset[0], y + offset[1], z + offset[2], size))], 1.0 / 3.0));
        }
        for (const auto& offset : std::array<std::array<int, 3>, 12>{
                 {{{1, 1, 0}}, {{1, -1, 0}}, {{-1, 1, 0}}, {{-1, -1, 0}},
                  {{1, 0, 1}}, {{1, 0, -1}}, {{-1, 0, 1}}, {{-1, 0, -1}},
                  {{0, 1, 1}}, {{0, 1, -1}}, {{0, -1, 1}}, {{0, -1, -1}}}}) {
          lap = add(lap, scale(field[static_cast<std::size_t>(index(
              x + offset[0], y + offset[1], z + offset[2], size))], 1.0 / 6.0));
        }
        const std::size_t here = static_cast<std::size_t>(index(x, y, z, size));
        lap = subtract(lap, scale(field[here], 4.0));
        result[here] = scale(lap, -c2);
      }
    }
  }
  return result;
}

long double tick_energy(const std::vector<Vec3>& j,
                        const std::vector<Vec3>& w,
                        const std::vector<Vec3>& kj) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < j.size(); ++i) {
    result += 0.5L * dot_ld(w[i], w[i])
        + 0.5L * dot_ld(j[i], kj[i])
        - 0.5L * dot_ld(w[i], kj[i]);
  }
  return result;
}

long double discrete_action(const std::vector<Vec3>& j,
                            const std::vector<Vec3>& next,
                            const std::vector<Vec3>& kj) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < j.size(); ++i) {
    const Vec3 delta = subtract(next[i], j[i]);
    result += 0.5L * dot_ld(delta, delta) - 0.5L * dot_ld(j[i], kj[i]);
  }
  return result;
}

PeriodicFields fixture(int size, int which) {
  const int count = size * size * size;
  PeriodicFields result;
  result.size = size;
  result.scalar.resize(static_cast<std::size_t>(count));
  result.first.resize(static_cast<std::size_t>(count));
  result.second.resize(static_cast<std::size_t>(count));
  result.variation.resize(static_cast<std::size_t>(count));
  constexpr double TWO_PI = 6.283185307179586476925286766559;
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      for (int z = 0; z < size; ++z) {
        const double kx = TWO_PI * x / size;
        const double ky = TWO_PI * y / size;
        const double kz = TWO_PI * z / size;
        const std::size_t i = static_cast<std::size_t>(index(x, y, z, size));
        const double phase = static_cast<double>(which + 1);
        result.scalar[i] = 0.37 * std::sin(kx + phase * ky)
            + 0.19 * std::cos(kz - phase * kx);
        result.first[i] = {
            0.23 * std::cos(kx + ky) + 0.07 * std::sin(phase * kz),
            -0.17 * std::sin(ky + kz) + 0.05 * std::cos(phase * kx),
            0.29 * std::cos(kz + kx) - 0.11 * std::sin(phase * ky)};
        const Vec3 velocity{
            0.31 * std::sin(ky + phase * kz),
            -0.27 * std::cos(kz + phase * kx),
            0.21 * std::sin(kx - phase * ky)};
        result.second[i] = scale(velocity, result.scalar[i]);
        result.variation[i] = {
            0.13 * std::sin(kx - kz) + 0.09 * std::cos(phase * ky),
            0.16 * std::cos(ky - kx) - 0.08 * std::sin(phase * kz),
            -0.12 * std::sin(kz - ky) + 0.06 * std::cos(phase * kx)};
      }
    }
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
    for (int sx : {-1, 1}) {
      for (int sy : {-1, 1}) {
        for (int sz : {-1, 1}) {
          if (parity * sx * sy * sz != 1) continue;
          Matrix3 rotation{};
          const std::array<int, 3> signs{{sx, sy, sz}};
          for (int row = 0; row < 3; ++row) {
            rotation[static_cast<std::size_t>(row)]
                    [static_cast<std::size_t>(permutation[static_cast<std::size_t>(row)])]
                = static_cast<double>(signs[static_cast<std::size_t>(row)]);
          }
          result.push_back(rotation);
        }
      }
    }
  } while (std::next_permutation(permutation.begin(), permutation.end()));
  return result;
}

Vec3 rotate_vector(const Matrix3& rotation, const Vec3& value) {
  const std::array<double, 3> input{{value.x, value.y, value.z}};
  std::array<double, 3> output{};
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      output[static_cast<std::size_t>(i)]
          += rotation[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
           * input[static_cast<std::size_t>(j)];
    }
  }
  return {output[0], output[1], output[2]};
}

PeriodicFields rotate_fields(const PeriodicFields& input,
                             const Matrix3& rotation) {
  PeriodicFields result;
  result.size = input.size;
  result.scalar.resize(input.scalar.size());
  result.first.resize(input.first.size());
  result.second.resize(input.second.size());
  result.variation.resize(input.variation.size());
  for (int x = 0; x < input.size; ++x) {
    for (int y = 0; y < input.size; ++y) {
      for (int z = 0; z < input.size; ++z) {
        const std::array<int, 3> coordinate{{x, y, z}};
        std::array<int, 3> target{};
        for (int i = 0; i < 3; ++i) {
          for (int j = 0; j < 3; ++j) {
            target[static_cast<std::size_t>(i)] += static_cast<int>(
                rotation[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)])
                * coordinate[static_cast<std::size_t>(j)];
          }
        }
        const std::size_t source = static_cast<std::size_t>(index(x, y, z, input.size));
        const std::size_t destination = static_cast<std::size_t>(index(
            target[0], target[1], target[2], input.size));
        result.scalar[destination] = input.scalar[source];
        result.first[destination] = rotate_vector(rotation, input.first[source]);
        result.second[destination] = rotate_vector(rotation, input.second[source]);
        result.variation[destination] = rotate_vector(rotation, input.variation[source]);
      }
    }
  }
  return result;
}

long double correct_interaction(const PeriodicFields& fields,
                                const std::vector<Vec3>& j) {
  long double result = 0.0L;
  for (int x = 0; x < fields.size; ++x) {
    for (int y = 0; y < fields.size; ++y) {
      for (int z = 0; z < fields.size; ++z) {
        const std::size_t i = static_cast<std::size_t>(index(x, y, z, fields.size));
        result += static_cast<long double>(G_C) * fields.scalar[i]
            * divergence(j, fields.size, x, y, z);
        result += static_cast<long double>(G_C)
            * dot_ld(curl(j, fields.size, x, y, z), fields.second[i]);
      }
    }
  }
  return result;
}

long double documented_interaction(const PeriodicFields& fields,
                                   const std::vector<Vec3>& j) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < j.size(); ++i) {
    result -= static_cast<long double>(G_C) * dot_ld(fields.second[i], j[i]);
  }
  return result;
}

double normalized_residual(long double lhs, long double rhs) {
  return static_cast<double>(std::abs(lhs - rhs)
      / (1.0L + std::max(std::abs(lhs), std::abs(rhs))));
}

}  // namespace

NativeFieldDiscreteActionResult analyze_native_field_discrete_action() {
  NativeFieldDiscreteActionResult result;
  constexpr double TOL = 1e-12;
  const Matrix2 omega{{{{0.0, 1.0}}, {{-1.0, 0.0}}}};

  bool mode_gates = true;
  for (int size : {16, 32, 64}) {
    for (int mode = 0; mode <= 3; ++mode) {
      for (int direction = 0; direction < 3; ++direction) {
        const double a = (C_WAVE * C_WAVE)
            * mode_symbol(size, mode, direction);
        const Matrix2 u{{{{1.0 - a, 1.0}}, {{-a, 1.0}}}};
        const Matrix2 g{{{{a, -0.5 * a}}, {{-0.5 * a, 1.0}}}};
        const Matrix2 symplectic = multiply(transpose(u), multiply(omega, u));
        const Matrix2 invariant = multiply(transpose(u), multiply(g, u));
        result.maximum_symplectic_residual = std::max(
            result.maximum_symplectic_residual,
            matrix_residual(symplectic, omega));
        result.maximum_invariant_matrix_residual = std::max(
            result.maximum_invariant_matrix_residual,
            matrix_residual(invariant, g));

        const double j = 0.17 + 0.013 * result.mode_arms;
        const double w = -0.11 + 0.007 * result.mode_arms;
        const double previous_j = j - w;
        const double next_w = w - a * j;
        const double next_j = j + next_w;
        const double after_next_w = next_w - a * next_j;
        const double after_next_j = next_j + after_next_w;
        const double del = 2.0 * j - previous_j - next_j - a * j;
        result.maximum_discrete_el_residual = std::max(
            result.maximum_discrete_el_residual, std::abs(del));
        result.maximum_legendre_momentum_residual = std::max({
            result.maximum_legendre_momentum_residual,
            std::abs((j - previous_j) - w),
            std::abs((next_j - j + a * j) - w),
            std::abs((next_j - j) - next_w),
            std::abs((after_next_j - next_j + a * next_j) - next_w)});

        const double before_energy = w * w + a * j * j - a * j * w;
        const double after_energy = next_w * next_w + a * next_j * next_j
            - a * next_j * next_w;
        result.maximum_tick_invariant_residual = std::max(
            result.maximum_tick_invariant_residual,
            std::abs(before_energy - after_energy)
                / (1.0 + std::max(std::abs(before_energy), std::abs(after_energy))));

        double mu = 1.0;
        double cosine = 1.0 - 0.5 * a;
        double sine = 0.0;
        double theta = 0.0;
        if (a > 1e-15) {
          theta = std::acos(cosine);
          sine = std::sqrt(a * (1.0 - 0.25 * a));
          mu = theta / sine;
        }
        const Matrix2 glog{{{{mu * a, -0.5 * mu * a}},
                             {{-0.5 * mu * a, mu}}}};
        const Matrix2 generator = multiply(omega, glog);
        Matrix2 flow{};
        if (a <= 1e-15) {
          flow = {{{{1.0 + generator[0][0], generator[0][1]}},
                   {{generator[1][0], 1.0 + generator[1][1]}}}};
        } else {
          const double factor = sine / theta;
          flow = {{{{cosine + factor * generator[0][0],
                     factor * generator[0][1]}},
                   {{factor * generator[1][0],
                     cosine + factor * generator[1][1]}}}};
        }
        result.maximum_shadow_flow_residual = std::max(
            result.maximum_shadow_flow_residual, matrix_residual(flow, u));
        mode_gates = mode_gates && a >= -TOL && a < 4.0
            && result.maximum_symplectic_residual <= TOL
            && result.maximum_invariant_matrix_residual <= TOL;
        ++result.mode_arms;
      }
    }
  }

  // Coefficient matrix of U^T G U-G for G=[[x,y],[y,z]], at a=1/2.
  const double a_rank = 0.5;
  std::array<std::array<double, 3>, 3> constraint{{
      {{a_rank * (a_rank - 2.0), a_rank * (2.0 * a_rank - 2.0), a_rank * a_rank}},
      {{1.0 - a_rank, -2.0 * a_rank, -a_rank}},
      {{1.0, 2.0, 0.0}}}};
  result.invariant_constraint_rank = rank3(constraint);
  result.invariant_constraint_nullity = 3 - result.invariant_constraint_rank;

  bool lattice_gates = true;
  for (int size : {5, 7}) {
    for (int which = 0; which < 2; ++which) {
      PeriodicFields fields = fixture(size, which);
      std::vector<Vec3> j = fields.first;
      std::vector<Vec3> w = fields.variation;
      const std::vector<Vec3> kj = apply_k(j, size);
      std::vector<Vec3> next_j(j.size());
      std::vector<Vec3> next_w(w.size());
      std::vector<Vec3> previous_j(j.size());
      double maximum_del = 0.0;
      double maximum_legendre = 0.0;
      for (std::size_t i = 0; i < j.size(); ++i) {
        previous_j[i] = subtract(j[i], w[i]);
        next_w[i] = subtract(w[i], kj[i]);
        next_j[i] = add(j[i], next_w[i]);
        const Vec3 del = add(subtract(add(next_j[i], previous_j[i]), scale(j[i], 2.0)), kj[i]);
        maximum_del = std::max(maximum_del, max_component(del));
        const Vec3 p_left = subtract(j[i], previous_j[i]);
        const Vec3 p_right = add(subtract(next_j[i], j[i]), kj[i]);
        maximum_legendre = std::max({maximum_legendre,
                                     max_component(subtract(p_left, w[i])),
                                     max_component(subtract(p_right, w[i]))});
      }
      const std::vector<Vec3> next_kj = apply_k(next_j, size);
      const long double before = tick_energy(j, w, kj);
      const long double after = tick_energy(next_j, next_w, next_kj);
      const double energy_residual = normalized_residual(before, after);
      result.maximum_discrete_el_residual = std::max(
          result.maximum_discrete_el_residual, maximum_del);
      result.maximum_legendre_momentum_residual = std::max(
          result.maximum_legendre_momentum_residual, maximum_legendre);
      result.maximum_tick_invariant_residual = std::max(
          result.maximum_tick_invariant_residual, energy_residual);
      lattice_gates = lattice_gates && maximum_del <= TOL
          && maximum_legendre <= TOL && energy_residual <= TOL;

      const long double action = discrete_action(j, next_j, kj);
      for (const Matrix3& rotation : proper_rotations()) {
        PeriodicFields rotated = rotate_fields(fields, rotation);
        std::vector<Vec3> rotated_next(next_j.size());
        PeriodicFields next_container = fields;
        next_container.first = next_j;
        const PeriodicFields rotated_next_container = rotate_fields(next_container, rotation);
        rotated_next = rotated_next_container.first;
        const std::vector<Vec3> rotated_kj = apply_k(rotated.first, size);
        const long double rotated_action = discrete_action(
            rotated.first, rotated_next, rotated_kj);
        const long double interaction = correct_interaction(fields, fields.first);
        const long double rotated_interaction = correct_interaction(rotated, rotated.first);
        result.maximum_proper_cubic_covariance_residual = std::max({
            result.maximum_proper_cubic_covariance_residual,
            normalized_residual(action, rotated_action),
            normalized_residual(interaction, rotated_interaction)});
        ++result.proper_cubic_covariance_arms;
      }
      ++result.lattice_action_arms;
    }
  }

  bool source_gates = true;
  for (int size : {5, 7}) {
    for (int which = 0; which < 2; ++which) {
      const PeriodicFields fields = fixture(size, which);
      std::vector<Vec3> coded(fields.first.size());
      std::vector<Vec3> documented_gradient(fields.first.size());
      long double electric_left = 0.0L;
      long double electric_right = 0.0L;
      long double curl_left = 0.0L;
      long double curl_right = 0.0L;
      long double predicted_correct = 0.0L;
      long double predicted_documented = 0.0L;
      for (int x = 0; x < size; ++x) {
        for (int y = 0; y < size; ++y) {
          for (int z = 0; z < size; ++z) {
            const std::size_t i = static_cast<std::size_t>(index(x, y, z, size));
            const Vec3 grad_s = gradient(fields.scalar, size, x, y, z);
            const Vec3 curl_u = curl(fields.second, size, x, y, z);
            coded[i] = add(scale(grad_s, -G_C), scale(curl_u, G_C));
            documented_gradient[i] = scale(fields.second[i], -G_C);
            electric_left += static_cast<long double>(fields.scalar[i])
                * divergence(fields.variation, size, x, y, z);
            electric_right += dot_ld(fields.variation[i], scale(grad_s, -1.0));
            curl_left += dot_ld(curl(fields.variation, size, x, y, z), fields.second[i]);
            curl_right += dot_ld(fields.variation[i], curl_u);
            predicted_correct += dot_ld(fields.variation[i], coded[i]);
            predicted_documented += dot_ld(fields.variation[i], documented_gradient[i]);
          }
        }
      }
      result.maximum_electric_adjoint_residual = std::max(
          result.maximum_electric_adjoint_residual,
          normalized_residual(electric_left, electric_right));
      result.maximum_curl_adjoint_residual = std::max(
          result.maximum_curl_adjoint_residual,
          normalized_residual(curl_left, curl_right));

      constexpr double epsilon = 0.5;
      std::vector<Vec3> plus(fields.first.size());
      std::vector<Vec3> minus(fields.first.size());
      for (std::size_t i = 0; i < plus.size(); ++i) {
        plus[i] = add(fields.first[i], scale(fields.variation[i], epsilon));
        minus[i] = subtract(fields.first[i], scale(fields.variation[i], epsilon));
      }
      const long double measured_correct =
          (correct_interaction(fields, plus) - correct_interaction(fields, minus))
          / (2.0L * epsilon);
      const long double measured_documented =
          (documented_interaction(fields, plus) - documented_interaction(fields, minus))
          / (2.0L * epsilon);
      result.maximum_correct_source_action_residual = std::max(
          result.maximum_correct_source_action_residual,
          normalized_residual(measured_correct, predicted_correct));
      result.maximum_documented_action_derivative_residual = std::max(
          result.maximum_documented_action_derivative_residual,
          normalized_residual(measured_documented, predicted_documented));
      source_gates = source_gates
          && result.maximum_electric_adjoint_residual <= TOL
          && result.maximum_curl_adjoint_residual <= TOL
          && result.maximum_correct_source_action_residual <= TOL
          && result.maximum_documented_action_derivative_residual <= TOL;
      ++result.source_operator_arms;
    }
  }

  result.minimum_uniform_documented_source_mismatch =
      std::numeric_limits<double>::infinity();
  const double inv_sqrt14 = 1.0 / std::sqrt(14.0);
  const std::array<Vec3, 4> velocities{{
      {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0},
      {inv_sqrt14, -2.0 * inv_sqrt14, 3.0 * inv_sqrt14}}};
  for (int size : {5, 7}) {
    const int count = size * size * size;
    for (const Vec3& velocity : velocities) {
      std::vector<double> scalar(static_cast<std::size_t>(count), 1.0);
      std::vector<Vec3> current(static_cast<std::size_t>(count), velocity);
      double coded_maximum = 0.0;
      double mismatch = 0.0;
      for (int x = 0; x < size; ++x) {
        for (int y = 0; y < size; ++y) {
          for (int z = 0; z < size; ++z) {
            const Vec3 coded = add(
                scale(gradient(scalar, size, x, y, z), -G_C),
                scale(curl(current, size, x, y, z), G_C));
            const Vec3 documented = scale(velocity, -G_C);
            coded_maximum = std::max(coded_maximum, norm(coded));
            mismatch = std::max(mismatch, norm(subtract(coded, documented)));
          }
        }
      }
      result.maximum_uniform_coded_source = std::max(
          result.maximum_uniform_coded_source, coded_maximum);
      result.minimum_uniform_documented_source_mismatch = std::min(
          result.minimum_uniform_documented_source_mismatch, mismatch);
      ++result.uniform_counterexample_arms;
    }
  }

  result.maximum_affine_source_symplectic_residual =
      result.maximum_symplectic_residual;
  result.local_discrete_action_reproduces_tick = mode_gates && lattice_gates
      && result.mode_arms == 36 && result.lattice_action_arms == 4
      && result.maximum_discrete_el_residual <= TOL;
  result.wave_velocity_is_legendre_momentum =
      result.maximum_legendre_momentum_residual <= TOL;
  result.standard_pairing_is_native =
      result.local_discrete_action_reproduces_tick
      && result.wave_velocity_is_legendre_momentum
      && result.maximum_symplectic_residual <= TOL;
  result.normalized_tick_invariant_is_unique =
      result.invariant_constraint_rank == 2
      && result.invariant_constraint_nullity == 1
      && result.maximum_invariant_matrix_residual <= TOL
      && result.maximum_tick_invariant_residual <= TOL;

  // A finite-range translation-invariant continuous generator has a Laurent-
  // polynomial symbol.  Along <100>, a=(2/3)(1-cos k); mu(a) has a branch at
  // a=4, i.e. cos k=-5 and z=-5+-2sqrt(6), both finite and nonzero.  The
  // independent symbolic proof checks the analytic obstruction.  This flag
  // records that theorem, not a numerical range fit.
  const double branch_plus = -5.0 + 2.0 * std::sqrt(6.0);
  const double branch_minus = -5.0 - 2.0 * std::sqrt(6.0);
  result.exact_continuous_shadow_generator_is_nonlocal =
      std::isfinite(branch_plus) && std::isfinite(branch_minus)
      && branch_plus != 0.0 && branch_minus != 0.0
      && result.maximum_shadow_flow_residual <= TOL;
  result.prescribed_source_action_reproduces_phase_read = source_gates
      && result.source_operator_arms == 4
      && result.maximum_uniform_coded_source <= TOL;
  result.prescribed_source_map_is_affine_symplectic =
      result.maximum_affine_source_symplectic_residual <= TOL;
  result.documented_velocity_interaction_generates_coded_source =
      result.minimum_uniform_documented_source_mismatch <= TOL;
  result.full_dynamic_matter_field_action_derived = false;
  result.production_changed = false;

  result.valid = result.local_discrete_action_reproduces_tick
      && result.wave_velocity_is_legendre_momentum
      && result.standard_pairing_is_native
      && result.normalized_tick_invariant_is_unique
      && result.exact_continuous_shadow_generator_is_nonlocal
      && result.prescribed_source_action_reproduces_phase_read
      && result.prescribed_source_map_is_affine_symplectic
      && result.proper_cubic_covariance_arms == 96
      && result.maximum_proper_cubic_covariance_residual <= TOL
      && result.uniform_counterexample_arms == 8
      && result.minimum_uniform_documented_source_mismatch > 1e-6
      && !result.documented_velocity_interaction_generates_coded_source
      && !result.full_dynamic_matter_field_action_derived
      && !result.production_changed;
  return result;
}

}  // namespace ftd::eft
