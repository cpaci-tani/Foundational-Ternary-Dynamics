#include "ftd/eft/native_hodge_reciprocity.h"

#include "ftd/constants.h"
#include "ftd/voxel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <limits>
#include <vector>

namespace ftd::eft {

namespace {

using Complex = std::complex<double>;
using CVec3 = std::array<Complex, 3>;
using Matrix3 = std::array<std::array<double, 3>, 3>;

constexpr double TWO_PI = 6.283185307179586476925286766559;
constexpr double PI = 3.1415926535897932384626433832795;

Vec3 add(const Vec3& a, const Vec3& b) {
  return {a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 subtract(const Vec3& a, const Vec3& b) {
  return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 scale(const Vec3& a, double factor) {
  return {factor * a.x, factor * a.y, factor * a.z};
}

double dot(const Vec3& a, const Vec3& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 cross(const Vec3& a, const Vec3& b) {
  return {a.y * b.z - a.z * b.y,
          a.z * b.x - a.x * b.z,
          a.x * b.y - a.y * b.x};
}

double norm(const Vec3& a) {
  return std::sqrt(dot(a, a));
}

double max_component(const Vec3& a) {
  return std::max({std::abs(a.x), std::abs(a.y), std::abs(a.z)});
}

double normalized_residual(double lhs, double rhs) {
  return std::abs(lhs - rhs) / (1.0 + std::max(std::abs(lhs), std::abs(rhs)));
}

double normalized_residual(const Complex& lhs, const Complex& rhs) {
  return std::abs(lhs - rhs) / (1.0 + std::max(std::abs(lhs), std::abs(rhs)));
}

double normalized_residual(const CVec3& lhs, const CVec3& rhs) {
  double result = 0.0;
  for (int i = 0; i < 3; ++i) {
    result = std::max(result, normalized_residual(
        lhs[static_cast<std::size_t>(i)], rhs[static_cast<std::size_t>(i)]));
  }
  return result;
}

Vec3 principal_wavevector(int size, int mode, int direction) {
  const double k = TWO_PI * static_cast<double>(mode) / static_cast<double>(size);
  if (direction == 0) return {k, 0.0, 0.0};
  if (direction == 1) return {k, k, 0.0};
  return {k, k, k};
}

Vec3 sine_symbol(const Vec3& k) {
  return {std::sin(k.x), std::sin(k.y), std::sin(k.z)};
}

double full_stencil_symbol(const Vec3& k) {
  const double cx = std::cos(k.x);
  const double cy = std::cos(k.y);
  const double cz = std::cos(k.z);
  return 4.0 - (2.0 / 3.0) * (cx + cy + cz)
      - (2.0 / 3.0) * (cx * cy + cx * cz + cy * cz);
}

double static_kernel(const Vec3& k) {
  const Vec3 s = sine_symbol(k);
  const double sigma2 = dot(s, s);
  const double stiffness = C_WAVE * C_WAVE * full_stencil_symbol(k);
  return stiffness > 0.0 ? sigma2 / stiffness : 0.0;
}

CVec3 complex_scale(const CVec3& value, Complex factor) {
  return {factor * value[0], factor * value[1], factor * value[2]};
}

CVec3 complex_cross(const Vec3& a, const CVec3& b) {
  return {
      a.y * b[2] - a.z * b[1],
      a.z * b[0] - a.x * b[2],
      a.x * b[1] - a.y * b[0]};
}

CVec3 curl_symbol(const Vec3& s, const CVec3& value) {
  return complex_scale(complex_cross(s, value), Complex{0.0, 1.0});
}

Complex divergence_symbol(const Vec3& s, const CVec3& value) {
  return Complex{0.0, 1.0}
      * (s.x * value[0] + s.y * value[1] + s.z * value[2]);
}

CVec3 real_to_complex(const Vec3& value) {
  return {Complex{value.x, 0.0}, Complex{value.y, 0.0},
          Complex{value.z, 0.0}};
}

std::array<Vec3, 2> transverse_basis(int direction) {
  if (direction == 0) return {{{0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}};
  if (direction == 1) {
    const double r = 1.0 / std::sqrt(2.0);
    return {{{r, -r, 0.0}, {0.0, 0.0, 1.0}}};
  }
  const double r2 = 1.0 / std::sqrt(2.0);
  const double r6 = 1.0 / std::sqrt(6.0);
  return {{{r2, -r2, 0.0}, {r6, r6, -2.0 * r6}}};
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

Vec3 rotate(const Matrix3& rotation, const Vec3& value) {
  const std::array<double, 3> input{{value.x, value.y, value.z}};
  std::array<double, 3> output{};
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      output[static_cast<std::size_t>(i)] +=
          rotation[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
          * input[static_cast<std::size_t>(j)];
    }
  }
  return {output[0], output[1], output[2]};
}

int wrap(int value, int size) {
  value %= size;
  return value < 0 ? value + size : value;
}

int index(int x, int y, int z, int size) {
  return (wrap(x, size) * size + wrap(y, size)) * size + wrap(z, size);
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

std::vector<double> divergence_field(const std::vector<Vec3>& field, int size) {
  std::vector<double> result(field.size());
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      for (int z = 0; z < size; ++z) {
        result[static_cast<std::size_t>(index(x, y, z, size))] =
            divergence(field, size, x, y, z);
      }
    }
  }
  return result;
}

std::vector<Vec3> gradient_field(const std::vector<double>& scalar, int size) {
  std::vector<Vec3> result(scalar.size());
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      for (int z = 0; z < size; ++z) {
        result[static_cast<std::size_t>(index(x, y, z, size))] =
            gradient(scalar, size, x, y, z);
      }
    }
  }
  return result;
}

std::vector<Vec3> curl_field(const std::vector<Vec3>& field, int size) {
  std::vector<Vec3> result(field.size());
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      for (int z = 0; z < size; ++z) {
        result[static_cast<std::size_t>(index(x, y, z, size))] =
            curl(field, size, x, y, z);
      }
    }
  }
  return result;
}

std::vector<Vec3> fixture_field(int size, int which, double time_shift) {
  std::vector<Vec3> result(static_cast<std::size_t>(size * size * size));
  const double phase = static_cast<double>(which + 1);
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      for (int z = 0; z < size; ++z) {
        const double kx = TWO_PI * x / size;
        const double ky = TWO_PI * y / size;
        const double kz = TWO_PI * z / size;
        result[static_cast<std::size_t>(index(x, y, z, size))] = {
            0.23 * std::cos(kx + ky + time_shift)
                + 0.07 * std::sin(phase * kz),
            -0.17 * std::sin(ky + kz - 0.3 * time_shift)
                + 0.05 * std::cos(phase * kx),
            0.29 * std::cos(kz + kx + 0.2 * time_shift)
                - 0.11 * std::sin(phase * ky)};
      }
    }
  }
  return result;
}

struct PolynomialField {
  double a = 0.0;
  double b = 0.0;
  double c = 0.0;
  double d = 0.0;
  double e = 0.0;
  double h = 0.0;

  Vec3 j(const Vec3& x, double time) const {
    const double psi = a * x.x * x.x + b * x.y * x.y + c * x.x * x.y
        + d * time * x.x + e * time * x.y;
    return {h * x.x * x.x, 0.0, psi};
  }

  double div_j(const Vec3& x) const {
    return 2.0 * h * x.x;
  }

  Vec3 grad_div_j() const {
    return {2.0 * h, 0.0, 0.0};
  }

  Vec3 curl_j(const Vec3& x, double time) const {
    return {2.0 * b * x.y + c * x.x + e * time,
            -2.0 * a * x.x - c * x.y - d * time,
            0.0};
  }

  Vec3 dt_curl_j() const {
    return {e, -d, 0.0};
  }

  Vec3 curl_curl_j() const {
    return {0.0, 0.0, -2.0 * (a + b)};
  }

  Vec3 direct_el_force(const Vec3& velocity, int charge) const {
    const double qg = static_cast<double>(charge) * G_C;
    return {qg * (2.0 * h - e - 2.0 * (a + b) * velocity.y),
            qg * (d + 2.0 * (a + b) * velocity.x),
            0.0};
  }
};

}  // namespace

NativeHodgeReciprocityResult analyze_native_hodge_reciprocity() {
  NativeHodgeReciprocityResult result;
  constexpr double TOL = 1e-12;
  const double g2 = G_C * G_C;
  bool infrared_monotonic = true;
  bool soft_monotonic = true;
  result.minimum_static_kernel = std::numeric_limits<double>::infinity();
  result.minimum_soft_residue = std::numeric_limits<double>::infinity();

  for (int mode = 1; mode <= 3; ++mode) {
    for (int direction = 0; direction < 3; ++direction) {
      double previous_ir_error = std::numeric_limits<double>::infinity();
      double previous_soft_residue = std::numeric_limits<double>::infinity();
      for (int size : {16, 32, 64}) {
        const Vec3 k = principal_wavevector(size, mode, direction);
        const Vec3 s = sine_symbol(k);
        const double sigma2 = dot(s, s);
        const double symbol = full_stencil_symbol(k);
        const double kernel = static_kernel(k);
        const double u1 = 1.0 - std::cos(k.x);
        const double u2 = 1.0 - std::cos(k.y);
        const double u3 = 1.0 - std::cos(k.z);
        const double u_sum = u1 + u2 + u3;
        const double u_pairs = u1 * u2 + u1 * u3 + u2 * u3;
        const double u_squares = u1 * u1 + u2 * u2 + u3 * u3;
        const double symbol_from_u = 2.0 * u_sum - (2.0 / 3.0) * u_pairs;
        const double sigma_from_u = 2.0 * u_sum - u_squares;
        result.maximum_kernel_identity_residual = std::max({
            result.maximum_kernel_identity_residual,
            normalized_residual(symbol, symbol_from_u),
            normalized_residual(sigma2, sigma_from_u),
            normalized_residual(symbol - sigma2,
                                u_squares - (2.0 / 3.0) * u_pairs)});
        result.minimum_static_kernel = std::min(result.minimum_static_kernel, kernel);
        result.maximum_static_kernel = std::max(result.maximum_static_kernel, kernel);
        result.maximum_kernel_bound_excess = std::max(
            result.maximum_kernel_bound_excess, std::max(0.0, kernel - 3.0));
        const double ir_error = std::abs(kernel - 3.0);
        result.maximum_infrared_error = std::max(
            result.maximum_infrared_error, ir_error);
        infrared_monotonic = infrared_monotonic && ir_error < previous_ir_error;
        previous_ir_error = ir_error;

        const double soft_residue = g2 * sigma2;
        result.minimum_soft_residue = std::min(
            result.minimum_soft_residue, soft_residue);
        result.maximum_soft_residue = std::max(
            result.maximum_soft_residue, soft_residue);
        soft_monotonic = soft_monotonic && soft_residue < previous_soft_residue;
        previous_soft_residue = soft_residue;
        ++result.infrared_symbol_arms;
      }
    }
  }

  const Vec3 generic_k{TWO_PI / 17.0, 4.0 * PI / 17.0, 6.0 * PI / 17.0};
  const double generic_kernel = static_kernel(generic_k);
  for (const Matrix3& rotation : proper_rotations()) {
    const Vec3 rotated = rotate(rotation, generic_k);
    result.maximum_cubic_covariance_residual = std::max(
        result.maximum_cubic_covariance_residual,
        normalized_residual(static_kernel(rotated), generic_kernel));
    ++result.proper_cubic_rotation_arms;
  }

  result.largest_same_polarity_cross_energy =
      -std::numeric_limits<double>::infinity();
  result.smallest_opposite_polarity_cross_energy =
      std::numeric_limits<double>::infinity();
  for (int size : {16, 32}) {
    for (int charge : {-1, 1}) {
      for (int direction = 0; direction < 3; ++direction) {
        const Vec3 k = principal_wavevector(size, 1, direction);
        const Vec3 s = sine_symbol(k);
        const double stiffness = C_WAVE * C_WAVE * full_stencil_symbol(k);
        const double kernel = static_kernel(k);
        const CVec3 gradient_rho{
            Complex{0.0, s.x * charge}, Complex{0.0, s.y * charge},
            Complex{0.0, s.z * charge}};
        const CVec3 source = complex_scale(gradient_rho, -G_C);
        const CVec3 j_field = complex_scale(source, 1.0 / stiffness);
        const Complex phi = -G_C * divergence_symbol(s, j_field);
        const Complex predicted{-g2 * kernel * charge, 0.0};
        result.maximum_charge_response_residual = std::max(
            result.maximum_charge_response_residual,
            normalized_residual(phi, predicted));
        const double same_cross = -g2 * kernel;
        const double opposite_cross = g2 * kernel;
        result.largest_same_polarity_cross_energy = std::max(
            result.largest_same_polarity_cross_energy, same_cross);
        result.smallest_opposite_polarity_cross_energy = std::min(
            result.smallest_opposite_polarity_cross_energy, opposite_cross);
        ++result.static_charge_arms;
      }
    }
  }

  for (int size : {16, 32}) {
    for (int direction = 0; direction < 3; ++direction) {
      const Vec3 k = principal_wavevector(size, 1, direction);
      const Vec3 s = sine_symbol(k);
      const double stiffness = C_WAVE * C_WAVE * full_stencil_symbol(k);
      const double kernel = static_kernel(k);
      for (const Vec3& current : transverse_basis(direction)) {
        const CVec3 current_complex = real_to_complex(current);
        const CVec3 source = complex_scale(
            curl_symbol(s, current_complex), G_C);
        const CVec3 j_field = complex_scale(source, 1.0 / stiffness);
        const CVec3 potential = complex_scale(
            curl_symbol(s, j_field), G_C);
        const CVec3 predicted = complex_scale(current_complex, g2 * kernel);
        result.maximum_current_response_residual = std::max(
            result.maximum_current_response_residual,
            normalized_residual(potential, predicted));
        ++result.static_transverse_current_arms;
      }
    }
  }

  for (const Vec3& corner : std::array<Vec3, 4>{
           Vec3{PI, 0.0, 0.0}, Vec3{PI, PI, 0.0},
           Vec3{PI, 0.0, PI}, Vec3{PI, PI, PI}}) {
    result.maximum_corner_response = std::max(
        result.maximum_corner_response, std::abs(static_kernel(corner)));
    ++result.brillouin_corner_controls;
  }

  for (int size : {5, 7}) {
    for (int which = 0; which < 2; ++which) {
      const std::vector<Vec3> j0 = fixture_field(size, which, 0.0);
      const std::vector<Vec3> j1 = fixture_field(size, which, 0.37);
      const std::vector<Vec3> curl0 = curl_field(j0, size);
      const std::vector<Vec3> curl1 = curl_field(j1, size);
      const std::vector<Vec3> b0_raw = curl_field(curl0, size);
      const std::vector<Vec3> b1_raw = curl_field(curl1, size);
      std::vector<Vec3> midpoint(j0.size());
      std::vector<Vec3> delta(j0.size());
      for (std::size_t i = 0; i < j0.size(); ++i) {
        midpoint[i] = scale(add(j0[i], j1[i]), 0.5);
        delta[i] = subtract(j1[i], j0[i]);
      }
      const std::vector<double> div_mid = divergence_field(midpoint, size);
      const std::vector<Vec3> grad_div_mid = gradient_field(div_mid, size);
      const std::vector<Vec3> curl_delta = curl_field(delta, size);
      std::vector<Vec3> electric(j0.size());
      for (std::size_t i = 0; i < j0.size(); ++i) {
        electric[i] = scale(subtract(grad_div_mid[i], curl_delta[i]), G_C);
      }
      const std::vector<Vec3> curl_electric = curl_field(electric, size);
      const std::vector<double> div_b = divergence_field(b0_raw, size);
      for (std::size_t i = 0; i < j0.size(); ++i) {
        result.maximum_divergence_of_b_residual = std::max(
            result.maximum_divergence_of_b_residual,
            std::abs(G_C * div_b[i]));
        const Vec3 faraday = add(
            scale(subtract(b1_raw[i], b0_raw[i]), G_C),
            curl_electric[i]);
        result.maximum_faraday_residual = std::max(
            result.maximum_faraday_residual, max_component(faraday));
      }
      ++result.periodic_operator_identity_arms;
    }
  }

  const std::array<PolynomialField, 4> fields{{
      {0.21, -0.13, 0.17, 0.09, -0.11, 0.07},
      {-0.16, 0.24, -0.08, -0.12, 0.15, -0.05},
      {0.31, 0.18, 0.12, 0.07, 0.19, 0.09},
      {-0.22, -0.27, 0.14, 0.16, -0.06, 0.11}}};
  const std::array<Vec3, 4> positions{{
      {0.13, -0.21, 0.17}, {-0.19, 0.08, 0.23},
      {0.07, 0.16, -0.11}, {-0.14, -0.09, 0.18}}};
  const std::array<Vec3, 4> velocities{{
      {0.11, -0.17, 0.05}, {-0.09, 0.13, -0.04},
      {0.16, 0.07, -0.08}, {-0.12, -0.15, 0.06}}};
  const std::array<double, 4> times{{0.19, -0.23, 0.31, -0.17}};
  for (int charge : {-1, 1}) {
    for (std::size_t arm = 0; arm < fields.size(); ++arm) {
      const PolynomialField& field = fields[arm];
      const Vec3 position = positions[arm];
      const Vec3 velocity = velocities[arm];
      const double time = times[arm];
      const Vec3 curl_j = field.curl_j(position, time);
      const double interaction = static_cast<double>(charge) * G_C
          * (field.div_j(position) + dot(velocity, curl_j));
      const double phi = -G_C * field.div_j(position);
      const Vec3 vector_potential = scale(curl_j, G_C);
      const double rewritten = static_cast<double>(charge)
          * (dot(velocity, vector_potential) - phi);
      result.maximum_interaction_rewrite_residual = std::max(
          result.maximum_interaction_rewrite_residual,
          normalized_residual(interaction, rewritten));

      const Vec3 electric = scale(
          subtract(field.grad_div_j(), field.dt_curl_j()), G_C);
      const Vec3 magnetic = scale(field.curl_curl_j(), G_C);
      const Vec3 lorentz = scale(
          add(electric, cross(velocity, magnetic)),
          static_cast<double>(charge));
      const Vec3 direct = field.direct_el_force(velocity, charge);
      result.maximum_path_variation_residual = std::max(
          result.maximum_path_variation_residual,
          max_component(subtract(lorentz, direct)));
      result.maximum_magnetic_scalar_work = std::max(
          result.maximum_magnetic_scalar_work,
          std::abs(dot(velocity, cross(velocity, magnetic))));
      ++result.smooth_path_variation_arms;
    }
  }

  result.hodge_potentials_rewrite_interaction =
      result.smooth_path_variation_arms == 8
      && result.maximum_interaction_rewrite_residual <= TOL;
  result.lorentz_form_path_variation =
      result.maximum_path_variation_residual <= 1e-10
      && result.maximum_magnetic_scalar_work <= TOL;
  result.homogeneous_identities_exact =
      result.periodic_operator_identity_arms == 4
      && result.maximum_divergence_of_b_residual <= TOL
      && result.maximum_faraday_residual <= TOL;
  result.static_charge_pole_canceled =
      result.static_charge_arms == 12
      && result.maximum_charge_response_residual <= TOL
      && result.minimum_static_kernel >= -TOL
      && result.maximum_static_kernel <= 3.0 + TOL
      && result.maximum_kernel_bound_excess <= TOL
      && infrared_monotonic;
  result.static_current_pole_canceled =
      result.static_transverse_current_arms == 12
      && result.maximum_current_response_residual <= TOL;
  result.same_polarity_static_interaction_attractive =
      result.largest_same_polarity_cross_energy < 0.0
      && result.smallest_opposite_polarity_cross_energy > 0.0;
  result.soft_radiative_residue_quadratic = soft_monotonic
      && result.minimum_soft_residue > 0.0
      && result.maximum_soft_residue > result.minimum_soft_residue;
  result.reciprocal_force_is_coulomb_electromagnetism = false;
  result.exact_finite_step_total_energy_derived = false;
  result.mobile_manifested_solution_derived = false;
  result.production_changed = false;

  result.valid = result.infrared_symbol_arms == 27
      && result.proper_cubic_rotation_arms == 24
      && result.brillouin_corner_controls == 4
      && result.maximum_kernel_identity_residual <= TOL
      && result.maximum_cubic_covariance_residual <= TOL
      && result.maximum_corner_response <= TOL
      && result.hodge_potentials_rewrite_interaction
      && result.lorentz_form_path_variation
      && result.homogeneous_identities_exact
      && result.static_charge_pole_canceled
      && result.static_current_pole_canceled
      && result.same_polarity_static_interaction_attractive
      && result.soft_radiative_residue_quadratic
      && !result.reciprocal_force_is_coulomb_electromagnetism
      && !result.exact_finite_step_total_energy_derived
      && !result.mobile_manifested_solution_derived
      && !result.production_changed;
  return result;
}

}  // namespace ftd::eft
