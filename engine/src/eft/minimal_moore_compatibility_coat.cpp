#include "ftd/eft/minimal_moore_compatibility_coat.h"

#include "ftd/constants.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>
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

std::size_t flat_index(int L, int x, int y, int z) {
  const std::size_t wx = static_cast<std::size_t>(wrap(x, L));
  const std::size_t wy = static_cast<std::size_t>(wrap(y, L));
  const std::size_t wz = static_cast<std::size_t>(wrap(z, L));
  return (wx * static_cast<std::size_t>(L) + wy)
      * static_cast<std::size_t>(L) + wz;
}

void set_component(Vec3& value, int axis, double component_value) {
  if (axis == 0) value.x = component_value;
  if (axis == 1) value.y = component_value;
  if (axis == 2) value.z = component_value;
}

std::vector<double> smooth_axis(
    const std::vector<double>& input, int L, int axis) {
  std::vector<double> output(input.size(), 0.0);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        std::array<int, 3> site{{x, y, z}};
        auto minus = site;
        auto plus = site;
        --minus[static_cast<std::size_t>(axis)];
        ++plus[static_cast<std::size_t>(axis)];
        output[flat_index(L, x, y, z)] =
            0.25 * input[flat_index(L, minus[0], minus[1], minus[2])]
          + 0.50 * input[flat_index(L, x, y, z)]
          + 0.25 * input[flat_index(L, plus[0], plus[1], plus[2])];
      }
    }
  }
  return output;
}

std::vector<double> smooth_all(
    const std::vector<double>& input, int L) {
  std::vector<double> result = input;
  for (int axis = 0; axis < 3; ++axis) result = smooth_axis(result, L, axis);
  return result;
}

std::vector<double> smooth_transverse(
    const std::vector<double>& input, int L, int component_axis) {
  std::vector<double> result = input;
  for (int axis = 0; axis < 3; ++axis) {
    if (axis != component_axis) result = smooth_axis(result, L, axis);
  }
  return result;
}

double sum_field(const std::vector<double>& field) {
  long double result = 0.0L;
  for (double value : field) result += static_cast<long double>(value);
  return static_cast<double>(result);
}

int scalar_support(const std::vector<double>& field) {
  return static_cast<int>(std::count_if(
      field.begin(), field.end(), [](double value) {
        return std::abs(value) > 64.0 * std::numeric_limits<double>::epsilon();
      }));
}

int vector_support(const std::vector<Vec3>& field) {
  return static_cast<int>(std::count_if(
      field.begin(), field.end(), [](const Vec3& value) {
        return std::max({std::abs(value.x), std::abs(value.y),
                         std::abs(value.z)})
            > 64.0 * std::numeric_limits<double>::epsilon();
      }));
}

double max_wrong_sign(
    const std::vector<double>& field, int charge) {
  double residual = 0.0;
  for (double value : field) {
    residual = std::max(residual,
        std::max(0.0, -static_cast<double>(charge) * value));
  }
  return residual;
}

double max_continuity(const MooreCoatedCurrent& coated) {
  double residual = 0.0;
  for (int x = 0; x < coated.L; ++x) {
    for (int y = 0; y < coated.L; ++y) {
      for (int z = 0; z < coated.L; ++z) {
        residual = std::max(
            residual, std::abs(coated_continuity_at(coated, x, y, z)));
      }
    }
  }
  return residual;
}

Vec3 signed_first_moment(const std::vector<double>& rho, int L) {
  Vec3 result{};
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const double value = rho[flat_index(L, x, y, z)];
        result.x += value * x;
        result.y += value * y;
        result.z += value * z;
      }
    }
  }
  return result;
}

double max_component_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

Vec3 subtract(const Vec3& lhs, const Vec3& rhs) {
  return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

Vec3 add(const Vec3& lhs, const Vec3& rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

Vec3 scale(const Vec3& value, double factor) {
  return {factor * value.x, factor * value.y, factor * value.z};
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
    for (int sx : {-1, 1}) {
      for (int sy : {-1, 1}) {
        for (int sz : {-1, 1}) {
          if (parity * sx * sy * sz != 1) continue;
          Matrix3 matrix{};
          const std::array<int, 3> signs{{sx, sy, sz}};
          for (int row = 0; row < 3; ++row) {
            matrix[static_cast<std::size_t>(row)]
                  [static_cast<std::size_t>(
                      permutation[static_cast<std::size_t>(row)])]
                = signs[static_cast<std::size_t>(row)];
          }
          result.push_back(matrix);
        }
      }
    }
  } while (std::next_permutation(permutation.begin(), permutation.end()));
  return result;
}

Vec3 rotate(const Matrix3& matrix, const Vec3& value) {
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

std::array<int, 3> rotate_site(
    const Matrix3& matrix, int x, int y, int z, int center) {
  const std::array<int, 3> input{{x - center, y - center, z - center}};
  std::array<int, 3> output{{center, center, center}};
  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 3; ++col) {
      output[static_cast<std::size_t>(row)] +=
          matrix[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)]
          * input[static_cast<std::size_t>(col)];
    }
  }
  return output;
}

double compare_translated(
    const MooreCoatedCurrent& reference,
    const MooreCoatedCurrent& translated,
    const Coord& shift) {
  double residual = 0.0;
  for (int x = 0; x < reference.L; ++x) {
    for (int y = 0; y < reference.L; ++y) {
      for (int z = 0; z < reference.L; ++z) {
        const std::size_t from = flat_index(reference.L, x, y, z);
        const std::size_t to = flat_index(
            reference.L, x + shift.x, y + shift.y, z + shift.z);
        residual = std::max({residual,
            std::abs(translated.rho_before[to] - reference.rho_before[from]),
            std::abs(translated.rho_after[to] - reference.rho_after[from]),
            max_component_abs(subtract(
                translated.central_current[to],
                reference.central_current[from]))});
      }
    }
  }
  return residual;
}

double compare_rotated(
    const MooreCoatedCurrent& reference,
    const MooreCoatedCurrent& rotated,
    const Matrix3& matrix,
    int center) {
  double residual = 0.0;
  for (int x = 0; x < reference.L; ++x) {
    for (int y = 0; y < reference.L; ++y) {
      for (int z = 0; z < reference.L; ++z) {
        const std::size_t from = flat_index(reference.L, x, y, z);
        const auto target = rotate_site(matrix, x, y, z, center);
        const std::size_t to = flat_index(
            reference.L, target[0], target[1], target[2]);
        const Vec3 expected_current = rotate(
            matrix, reference.central_current[from]);
        residual = std::max({residual,
            std::abs(rotated.rho_before[to] - reference.rho_before[from]),
            std::abs(rotated.rho_after[to] - reference.rho_after[from]),
            max_component_abs(subtract(
                rotated.central_current[to], expected_current))});
      }
    }
  }
  return residual;
}

std::vector<double> divergence_field(
    const std::vector<Vec3>& field, int L) {
  std::vector<double> result(field.size(), 0.0);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        result[flat_index(L, x, y, z)] = 0.5 * (
            field[flat_index(L, x + 1, y, z)].x
          - field[flat_index(L, x - 1, y, z)].x
          + field[flat_index(L, x, y + 1, z)].y
          - field[flat_index(L, x, y - 1, z)].y
          + field[flat_index(L, x, y, z + 1)].z
          - field[flat_index(L, x, y, z - 1)].z);
      }
    }
  }
  return result;
}

std::vector<Vec3> gradient_field(
    const std::vector<double>& scalar, int L) {
  std::vector<Vec3> result(scalar.size());
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        result[flat_index(L, x, y, z)] = {
            0.5 * (scalar[flat_index(L, x + 1, y, z)]
                 - scalar[flat_index(L, x - 1, y, z)]),
            0.5 * (scalar[flat_index(L, x, y + 1, z)]
                 - scalar[flat_index(L, x, y - 1, z)]),
            0.5 * (scalar[flat_index(L, x, y, z + 1)]
                 - scalar[flat_index(L, x, y, z - 1)])};
      }
    }
  }
  return result;
}

std::vector<Vec3> curl_field(
    const std::vector<Vec3>& field, int L) {
  std::vector<Vec3> result(field.size());
  const auto at = [&](int x, int y, int z) -> const Vec3& {
    return field[flat_index(L, x, y, z)];
  };
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        result[flat_index(L, x, y, z)] = {
            0.5 * (at(x, y + 1, z).z - at(x, y - 1, z).z
                 - at(x, y, z + 1).y + at(x, y, z - 1).y),
            0.5 * (at(x, y, z + 1).x - at(x, y, z - 1).x
                 - at(x + 1, y, z).z + at(x - 1, y, z).z),
            0.5 * (at(x + 1, y, z).y - at(x - 1, y, z).y
                 - at(x, y + 1, z).x + at(x, y - 1, z).x)};
      }
    }
  }
  return result;
}

long double pair_scalar(
    const std::vector<double>& lhs, const std::vector<double>& rhs) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    result += static_cast<long double>(lhs[i]) * rhs[i];
  }
  return result;
}

long double pair_vector(
    const std::vector<Vec3>& lhs, const std::vector<Vec3>& rhs) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    result += static_cast<long double>(lhs[i].x) * rhs[i].x
        + static_cast<long double>(lhs[i].y) * rhs[i].y
        + static_cast<long double>(lhs[i].z) * rhs[i].z;
  }
  return result;
}

double normalized_residual(long double lhs, long double rhs) {
  return static_cast<double>(std::abs(lhs - rhs)
      / (1.0L + std::max(std::abs(lhs), std::abs(rhs))));
}

std::vector<Vec3> deterministic_field(int L, int fixture, double phase) {
  std::vector<Vec3> result(static_cast<std::size_t>(L * L * L));
  const double factor = static_cast<double>(fixture + 1);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const double kx = 2.0 * PI * x / L;
        const double ky = 2.0 * PI * y / L;
        const double kz = 2.0 * PI * z / L;
        result[flat_index(L, x, y, z)] = {
            0.17 * std::sin(kx + factor * ky + phase)
              + 0.03 * std::cos(kz),
            -0.13 * std::cos(ky + factor * kz - 0.2 * phase)
              + 0.05 * std::sin(kx),
            0.19 * std::sin(kz + factor * kx + 0.3 * phase)
              - 0.07 * std::cos(ky)};
      }
    }
  }
  return result;
}

void audit_conditional_energy(
    const MooreCoatedCurrent& coated,
    int fixture,
    MinimalMooreCompatibilityCoatResult& result) {
  const int L = coated.L;
  const std::vector<Vec3> r0 = deterministic_field(L, fixture, 0.0);
  const std::vector<Vec3> r1 = deterministic_field(L, fixture, 0.37);
  std::vector<Vec3> dr(r0.size());
  std::vector<Vec3> rbar(r0.size());
  std::vector<double> rhobar(r0.size());
  for (std::size_t i = 0; i < r0.size(); ++i) {
    dr[i] = subtract(r1[i], r0[i]);
    rbar[i] = scale(add(r0[i], r1[i]), 0.5);
    rhobar[i] = 0.5 * (coated.rho_before[i] + coated.rho_after[i]);
  }

  const std::vector<Vec3> grad_rho = gradient_field(rhobar, L);
  const std::vector<Vec3> curl_q = curl_field(coated.central_current, L);
  const std::vector<Vec3> curl_dr = curl_field(dr, L);
  const std::vector<double> div_dr = divergence_field(dr, L);
  const std::vector<double> div_r0 = divergence_field(r0, L);
  const std::vector<double> div_r1 = divergence_field(r1, L);
  const std::vector<double> div_rbar = divergence_field(rbar, L);
  const std::vector<Vec3> grad_div_rbar = gradient_field(div_rbar, L);

  std::vector<Vec3> source(r0.size());
  std::vector<Vec3> matter_integrand(r0.size());
  for (std::size_t i = 0; i < r0.size(); ++i) {
    source[i] = add(scale(grad_rho[i], -G_C), scale(curl_q[i], G_C));
    matter_integrand[i] = subtract(grad_div_rbar[i], curl_dr[i]);
  }

  const long double field_direct = pair_vector(source, dr);
  const long double field_adjoint = static_cast<long double>(G_C) * (
      pair_scalar(rhobar, div_dr)
      + pair_vector(coated.central_current, curl_dr));
  result.maximum_conditional_field_work_residual = std::max(
      result.maximum_conditional_field_work_residual,
      normalized_residual(field_direct, field_adjoint));

  const long double u0 = -static_cast<long double>(G_C)
      * pair_scalar(coated.rho_before, div_r0);
  const long double u1 = -static_cast<long double>(G_C)
      * pair_scalar(coated.rho_after, div_r1);
  const long double predicted_du = -static_cast<long double>(G_C) * (
      pair_scalar(rhobar, div_dr)
      + pair_vector(coated.central_current, grad_div_rbar));
  result.maximum_conditional_interaction_residual = std::max(
      result.maximum_conditional_interaction_residual,
      normalized_residual(u1 - u0, predicted_du));

  const long double matter_work = static_cast<long double>(G_C)
      * pair_vector(coated.central_current, matter_integrand);
  result.maximum_conditional_total_energy_residual = std::max(
      result.maximum_conditional_total_energy_residual,
      normalized_residual(field_direct + (u1 - u0) + matter_work, 0.0L));
  ++result.conditional_energy_arms;
}

double coat_symbol(double kx, double ky, double kz) {
  const auto one_axis = [](double k) {
    const double c = std::cos(0.5 * k);
    return c * c;
  };
  return one_axis(kx) * one_axis(ky) * one_axis(kz);
}

}  // namespace

int MooreCoatedCurrent::index(int x, int y, int z) const {
  if (L <= 0) return -1;
  return static_cast<int>(flat_index(L, x, y, z));
}

double central_current_divergence_at(
    const MooreCoatedCurrent& coated, int x, int y, int z) {
  if (coated.L <= 0 || coated.central_current.empty()) return NAN;
  const auto at = [&coated](int sx, int sy, int sz) -> const Vec3& {
    return coated.central_current[flat_index(coated.L, sx, sy, sz)];
  };
  return 0.5 * (
      at(x + 1, y, z).x - at(x - 1, y, z).x
    + at(x, y + 1, z).y - at(x, y - 1, z).y
    + at(x, y, z + 1).z - at(x, y, z - 1).z);
}

double coated_continuity_at(
    const MooreCoatedCurrent& coated, int x, int y, int z) {
  if (coated.L <= 0 || coated.rho_before.empty()) return NAN;
  const std::size_t site = flat_index(coated.L, x, y, z);
  return coated.rho_after[site] - coated.rho_before[site]
      + central_current_divergence_at(coated, x, y, z);
}

MooreCoatedCurrent make_minimal_moore_compatibility_coat(
    const FaceCurrentSegment& segment) {
  MooreCoatedCurrent result;
  result.L = segment.L;
  result.charge = segment.charge;
  if (!segment.valid || segment.L < 3) return result;

  result.rho_before = smooth_all(segment.rho_before, segment.L);
  result.rho_after = smooth_all(segment.rho_after, segment.L);
  result.central_current.assign(segment.rho_before.size(), Vec3{});

  const std::array<const std::vector<double>*, 3> face_current{{
      &segment.current_x, &segment.current_y, &segment.current_z}};
  for (int axis = 0; axis < 3; ++axis) {
    const std::vector<double> transverse = smooth_transverse(
        *face_current[static_cast<std::size_t>(axis)], segment.L, axis);
    for (int x = 0; x < segment.L; ++x) {
      for (int y = 0; y < segment.L; ++y) {
        for (int z = 0; z < segment.L; ++z) {
          std::array<int, 3> minus{{x, y, z}};
          --minus[static_cast<std::size_t>(axis)];
          const double value = 0.5 * (
              transverse[flat_index(segment.L, x, y, z)]
            + transverse[flat_index(
                segment.L, minus[0], minus[1], minus[2])]);
          set_component(
              result.central_current[flat_index(segment.L, x, y, z)],
              axis, value);
        }
      }
    }
  }

  result.rho_before_support = scalar_support(result.rho_before);
  result.rho_after_support = scalar_support(result.rho_after);
  result.current_support = vector_support(result.central_current);
  result.partition_residual = std::max(
      std::abs(sum_field(result.rho_before) - segment.charge),
      std::abs(sum_field(result.rho_after) - segment.charge));
  result.first_moment_residual = segment.first_moment_residual;
  result.wrong_sign_weight_residual = std::max(
      max_wrong_sign(result.rho_before, segment.charge),
      max_wrong_sign(result.rho_after, segment.charge));
  result.central_continuity_residual = max_continuity(result);
  result.finite_range = result.rho_before_support < result.total_sites()
      && result.rho_after_support < result.total_sites()
      && result.current_support < result.total_sites();
  result.valid = result.partition_residual <= TOL
      && result.first_moment_residual <= TOL
      && result.wrong_sign_weight_residual <= TOL
      && result.central_continuity_residual <= TOL
      && result.finite_range;
  return result;
}

MinimalMooreCompatibilityCoatResult
analyze_minimal_moore_compatibility_coat() {
  MinimalMooreCompatibilityCoatResult result;

  // Solve [2 1; -2 1] [a b]^T=[1 0]^T exactly in binary doubles.
  result.radius_one_a = 0.25;
  result.radius_one_b = 0.50;
  result.maximum_filter_equation_residual = std::max(
      std::abs(2.0 * result.radius_one_a + result.radius_one_b - 1.0),
      std::abs(-2.0 * result.radius_one_a + result.radius_one_b));
  result.scoped_radius_one_filter_unique =
      std::abs(4.0) > 0.0 && result.maximum_filter_equation_residual <= TOL;

  result.center_weight = 1.0 / 8.0;
  result.face_weight = 1.0 / 16.0;
  result.edge_weight = 1.0 / 32.0;
  result.corner_weight = 1.0 / 64.0;
  long double coat_sum = 0.0L;
  long double coat_x = 0.0L;
  long double coat_y = 0.0L;
  long double coat_z = 0.0L;
  double minimum_weight = std::numeric_limits<double>::infinity();
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        const auto axis_weight = [](int offset) {
          return offset == 0 ? 0.5 : 0.25;
        };
        const double weight = axis_weight(dx) * axis_weight(dy)
            * axis_weight(dz);
        coat_sum += weight;
        coat_x += weight * dx;
        coat_y += weight * dy;
        coat_z += weight * dz;
        minimum_weight = std::min(minimum_weight, weight);
        ++result.integer_coat_sites;
      }
    }
  }
  result.maximum_filter_equation_residual = std::max({
      result.maximum_filter_equation_residual,
      static_cast<double>(std::abs(coat_sum - 1.0L)),
      static_cast<double>(std::abs(coat_x)),
      static_cast<double>(std::abs(coat_y)),
      static_cast<double>(std::abs(coat_z)),
      std::abs(6.0 * result.face_weight - 3.0 / 8.0),
      std::abs(12.0 * result.edge_weight - 3.0 / 8.0),
      std::abs(8.0 * result.corner_weight - 1.0 / 8.0)});
  result.integer_coat_positive_and_normalized =
      result.integer_coat_sites == 27 && minimum_weight > 0.0
      && result.maximum_filter_equation_residual <= TOL;

  result.maximum_zero_mode_residual = std::abs(coat_symbol(0.0, 0.0, 0.0) - 1.0);
  for (const Vec3& k : std::array<Vec3, 7>{{
           {PI, 0.0, 0.0}, {0.0, PI, 0.0}, {0.0, 0.0, PI},
           {PI, PI, 0.0}, {PI, 0.0, PI}, {0.0, PI, PI},
           {PI, PI, PI}}}) {
    result.maximum_checkerboard_response = std::max(
        result.maximum_checkerboard_response, coat_symbol(k.x, k.y, k.z));
  }
  result.checkerboard_nulls_removed_from_source =
      result.maximum_zero_mode_residual <= TOL
      && result.maximum_checkerboard_response <= TOL;

  struct Path {
    Coord anchor_delta{};
    Vec3 remainder{};
  };
  const std::array<Path, 9> paths{{
      {{1, 0, 0}, {0.0, 0.0, 0.0}},
      {{-1, 0, 0}, {0.0, 0.0, 0.0}},
      {{0, 1, 0}, {0.0, 0.0, 0.0}},
      {{0, -1, 0}, {0.0, 0.0, 0.0}},
      {{0, 0, 1}, {0.0, 0.0, 0.0}},
      {{0, 0, -1}, {0.0, 0.0, 0.0}},
      {{0, 0, 0}, {0.70, 0.45, 0.30}},
      {{0, 0, 0}, {-0.60, 0.55, -0.35}},
      {{0, 0, 0}, {0.80, -0.40, 0.65}}}};

  result.minimum_local_rho_support = std::numeric_limits<int>::max();
  result.minimum_local_current_support = std::numeric_limits<int>::max();
  std::vector<std::pair<int, int>> support_reference;
  bool all_paths_valid = true;
  bool support_volume_independent = true;
  for (int L : {17, 33}) {
    const int center = L / 2;
    std::size_t support_index = 0;
    for (int charge : {-1, 1}) {
      for (const Path& path : paths) {
        const Coord start{center, center, center};
        const Coord end{center + path.anchor_delta.x,
                        center + path.anchor_delta.y,
                        center + path.anchor_delta.z};
        const FaceCurrentSegment face = make_face_current_segment(
            L, start, Vec3{}, end, path.remainder, charge);
        const MooreCoatedCurrent coated =
            make_minimal_moore_compatibility_coat(face);
        all_paths_valid = all_paths_valid && face.valid && coated.valid;
        result.maximum_partition_residual = std::max(
            result.maximum_partition_residual, coated.partition_residual);
        result.maximum_wrong_sign_weight_residual = std::max(
            result.maximum_wrong_sign_weight_residual,
            coated.wrong_sign_weight_residual);
        result.maximum_central_continuity_residual = std::max(
            result.maximum_central_continuity_residual,
            coated.central_continuity_residual);

        const Vec3 before_moment = signed_first_moment(coated.rho_before, L);
        const Vec3 after_moment = signed_first_moment(coated.rho_after, L);
        const Vec3 expected_before = scale(face.start_effective_position, charge);
        const Vec3 expected_after = scale(face.end_effective_position, charge);
        result.maximum_first_moment_residual = std::max({
            result.maximum_first_moment_residual,
            max_component_abs(subtract(before_moment, expected_before)),
            max_component_abs(subtract(after_moment, expected_after))});

        const int rho_support = std::max(
            coated.rho_before_support, coated.rho_after_support);
        result.minimum_local_rho_support = std::min(
            result.minimum_local_rho_support, rho_support);
        result.maximum_local_rho_support = std::max(
            result.maximum_local_rho_support, rho_support);
        result.minimum_local_current_support = std::min(
            result.minimum_local_current_support, coated.current_support);
        result.maximum_local_current_support = std::max(
            result.maximum_local_current_support, coated.current_support);
        if (L == 17) {
          support_reference.emplace_back(rho_support, coated.current_support);
        } else {
          support_volume_independent = support_volume_independent
              && support_index < support_reference.size()
              && support_reference[support_index]
                  == std::make_pair(rho_support, coated.current_support);
          ++support_index;
        }
        ++result.path_arms;
      }
      ++result.polarity_arms;
    }
    ++result.volume_arms;
  }
  result.trilinear_moments_preserved = all_paths_valid
      && result.maximum_partition_residual <= TOL
      && result.maximum_first_moment_residual <= TOL
      && result.maximum_wrong_sign_weight_residual <= TOL;
  result.local_central_continuity_exact = all_paths_valid
      && result.maximum_central_continuity_residual <= TOL;
  result.local_support_volume_independent = support_volume_independent
      && result.maximum_local_rho_support < 33 * 33 * 33
      && result.maximum_local_current_support < 33 * 33 * 33;

  const int L = 17;
  const int center = L / 2;
  const Coord reference_anchor{center, center, center};
  const Vec3 reference_start{0.10, -0.20, 0.15};
  const Vec3 reference_end{0.70, 0.45, 0.30};
  const FaceCurrentSegment reference_face = make_face_current_segment(
      L, reference_anchor, reference_start,
      reference_anchor, reference_end, 1);
  const MooreCoatedCurrent reference =
      make_minimal_moore_compatibility_coat(reference_face);

  for (const Coord& shift : std::array<Coord, 3>{{
           {2, -3, 1}, {-3, 2, -2}, {1, 2, -3}}}) {
    const Coord shifted_anchor{center + shift.x, center + shift.y,
                               center + shift.z};
    const FaceCurrentSegment shifted_face = make_face_current_segment(
        L, shifted_anchor, reference_start,
        shifted_anchor, reference_end, 1);
    const MooreCoatedCurrent shifted =
        make_minimal_moore_compatibility_coat(shifted_face);
    result.maximum_translation_covariance_residual = std::max(
        result.maximum_translation_covariance_residual,
        compare_translated(reference, shifted, shift));
    ++result.translation_arms;
  }
  result.integer_translation_covariant = reference.valid
      && result.translation_arms == 3
      && result.maximum_translation_covariance_residual <= TOL;

  for (const Matrix3& rotation : proper_rotations()) {
    const Vec3 rotated_start = rotate(rotation, reference_start);
    const Vec3 rotated_end = rotate(rotation, reference_end);
    const FaceCurrentSegment rotated_face = make_face_current_segment(
        L, reference_anchor, rotated_start,
        reference_anchor, rotated_end, 1);
    const MooreCoatedCurrent rotated =
        make_minimal_moore_compatibility_coat(rotated_face);
    result.maximum_cubic_covariance_residual = std::max(
        result.maximum_cubic_covariance_residual,
        compare_rotated(reference, rotated, rotation, center));
    ++result.proper_cubic_rotation_arms;
  }
  result.proper_cubic_covariant = result.proper_cubic_rotation_arms == 24
      && result.maximum_cubic_covariance_residual <= TOL;

  // Four deterministic conditional FTD-0576 energy ledgers.
  for (int fixture = 0; fixture < 4; ++fixture) {
    const int charge = fixture % 2 == 0 ? 1 : -1;
    const Path& path = paths[static_cast<std::size_t>(6 + fixture % 3)];
    const FaceCurrentSegment face = make_face_current_segment(
        L, reference_anchor, Vec3{},
        reference_anchor, path.remainder, charge);
    const MooreCoatedCurrent coated =
        make_minimal_moore_compatibility_coat(face);
    audit_conditional_energy(coated, fixture, result);
  }
  result.conditional_hodge_energy_compatible =
      result.conditional_energy_arms == 4
      && result.maximum_conditional_field_work_residual <= TOL
      && result.maximum_conditional_interaction_residual <= TOL
      && result.maximum_conditional_total_energy_residual <= TOL;

  // The integer-centered coat has central weight 1/8, not cardinal weight 1.
  result.integer_center_cardinality_defect =
      std::abs(1.0 - result.center_weight);
  result.coupling_representation_is_cardinal = false;
  result.reciprocal_force_derived = false;
  result.static_coulomb_pole_recovered = false;
  result.mobile_manifested_solution_derived = false;
  result.production_changed = false;

  result.valid = result.scoped_radius_one_filter_unique
      && result.integer_coat_positive_and_normalized
      && result.trilinear_moments_preserved
      && result.local_central_continuity_exact
      && result.local_support_volume_independent
      && result.integer_translation_covariant
      && result.proper_cubic_covariant
      && result.checkerboard_nulls_removed_from_source
      && result.conditional_hodge_energy_compatible
      && result.path_arms == 36
      && result.polarity_arms == 4
      && result.volume_arms == 2
      && result.integer_center_cardinality_defect >= 7.0 / 8.0 - TOL
      && !result.coupling_representation_is_cardinal
      && !result.reciprocal_force_derived
      && !result.static_coulomb_pole_recovered
      && !result.mobile_manifested_solution_derived
      && !result.production_changed;
  return result;
}

}  // namespace ftd::eft
