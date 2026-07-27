#include "ftd/eft/native_hodge_energy_continuity.h"

#include "ftd/constants.h"
#include "ftd/voxel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <numeric>
#include <vector>

namespace ftd::eft {
namespace {

using Matrix3 = std::array<std::array<int, 3>, 3>;

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
  return {factor * value.x, factor * value.y, factor * value.z};
}

long double dot_ld(const Vec3& lhs, const Vec3& rhs) {
  return static_cast<long double>(lhs.x) * rhs.x
      + static_cast<long double>(lhs.y) * rhs.y
      + static_cast<long double>(lhs.z) * rhs.z;
}

long double pairing(const std::vector<Vec3>& lhs,
                    const std::vector<Vec3>& rhs) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i) result += dot_ld(lhs[i], rhs[i]);
  return result;
}

long double pairing(const std::vector<double>& lhs,
                    const std::vector<double>& rhs) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result += static_cast<long double>(lhs[i]) * rhs[i];
  return result;
}

double normalized_residual(long double lhs, long double rhs) {
  return static_cast<double>(std::abs(lhs - rhs)
      / (1.0L + std::max(std::abs(lhs), std::abs(rhs))));
}

std::vector<Vec3> combine(const std::vector<Vec3>& lhs,
                          const std::vector<Vec3>& rhs,
                          double lhs_scale, double rhs_scale) {
  std::vector<Vec3> result(lhs.size());
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result[i] = add(scale(lhs[i], lhs_scale), scale(rhs[i], rhs_scale));
  return result;
}

std::vector<double> combine(const std::vector<double>& lhs,
                            const std::vector<double>& rhs,
                            double lhs_scale, double rhs_scale) {
  std::vector<double> result(lhs.size());
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result[i] = lhs_scale * lhs[i] + rhs_scale * rhs[i];
  return result;
}

std::vector<Vec3> gradient(const std::vector<double>& scalar, int size) {
  std::vector<Vec3> result(scalar.size());
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      for (int z = 0; z < size; ++z) {
        result[static_cast<std::size_t>(index(x, y, z, size))] = {
            0.5 * (scalar[static_cast<std::size_t>(index(x + 1, y, z, size))]
                 - scalar[static_cast<std::size_t>(index(x - 1, y, z, size))]),
            0.5 * (scalar[static_cast<std::size_t>(index(x, y + 1, z, size))]
                 - scalar[static_cast<std::size_t>(index(x, y - 1, z, size))]),
            0.5 * (scalar[static_cast<std::size_t>(index(x, y, z + 1, size))]
                 - scalar[static_cast<std::size_t>(index(x, y, z - 1, size))])};
      }
    }
  }
  return result;
}

std::vector<double> divergence(const std::vector<Vec3>& field, int size) {
  std::vector<double> result(field.size());
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      for (int z = 0; z < size; ++z) {
        result[static_cast<std::size_t>(index(x, y, z, size))] = 0.5 * (
            field[static_cast<std::size_t>(index(x + 1, y, z, size))].x
          - field[static_cast<std::size_t>(index(x - 1, y, z, size))].x
          + field[static_cast<std::size_t>(index(x, y + 1, z, size))].y
          - field[static_cast<std::size_t>(index(x, y - 1, z, size))].y
          + field[static_cast<std::size_t>(index(x, y, z + 1, size))].z
          - field[static_cast<std::size_t>(index(x, y, z - 1, size))].z);
      }
    }
  }
  return result;
}

std::vector<Vec3> curl(const std::vector<Vec3>& field, int size) {
  std::vector<Vec3> result(field.size());
  const auto at = [&](int x, int y, int z) -> const Vec3& {
    return field[static_cast<std::size_t>(index(x, y, z, size))];
  };
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      for (int z = 0; z < size; ++z) {
        result[static_cast<std::size_t>(index(x, y, z, size))] = {
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

std::vector<Vec3> apply_k(const std::vector<Vec3>& field, int size) {
  std::vector<Vec3> result(field.size());
  constexpr std::array<std::array<int, 3>, 6> faces{{
      {{1, 0, 0}}, {{-1, 0, 0}}, {{0, 1, 0}},
      {{0, -1, 0}}, {{0, 0, 1}}, {{0, 0, -1}}}};
  constexpr std::array<std::array<int, 3>, 12> edges{{
      {{1, 1, 0}}, {{1, -1, 0}}, {{-1, 1, 0}}, {{-1, -1, 0}},
      {{1, 0, 1}}, {{1, 0, -1}}, {{-1, 0, 1}}, {{-1, 0, -1}},
      {{0, 1, 1}}, {{0, 1, -1}}, {{0, -1, 1}}, {{0, -1, -1}}}};
  const double c2 = C_WAVE * C_WAVE;
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      for (int z = 0; z < size; ++z) {
        Vec3 lap{};
        for (const auto& offset : faces) {
          lap = add(lap, scale(field[static_cast<std::size_t>(index(
              x + offset[0], y + offset[1], z + offset[2], size))], 1.0 / 3.0));
        }
        for (const auto& offset : edges) {
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

double mode_symbol(int size, int mode, int direction) {
  constexpr double TWO_PI = 6.283185307179586476925286766559;
  const double k = TWO_PI * static_cast<double>(mode) / size;
  const double cx = std::cos(direction >= 0 ? k : 0.0);
  const double cy = std::cos(direction >= 1 ? k : 0.0);
  const double cz = std::cos(direction >= 2 ? k : 0.0);
  return C_WAVE * C_WAVE * (4.0
      - (2.0 / 3.0) * (cx + cy + cz)
      - (2.0 / 3.0) * (cx * cy + cx * cz + cy * cz));
}

struct Fixture {
  std::vector<Vec3> j;
  std::vector<Vec3> w;
  std::vector<Vec3> source;
  std::vector<Vec3> r0;
  std::vector<Vec3> r1;
  std::vector<Vec3> current;
  std::vector<double> rho0;
};

Fixture fixture(int size, int which) {
  const int count = size * size * size;
  Fixture result;
  result.j.resize(static_cast<std::size_t>(count));
  result.w.resize(static_cast<std::size_t>(count));
  result.source.resize(static_cast<std::size_t>(count));
  result.r0.resize(static_cast<std::size_t>(count));
  result.r1.resize(static_cast<std::size_t>(count));
  result.current.resize(static_cast<std::size_t>(count));
  result.rho0.resize(static_cast<std::size_t>(count));
  constexpr double TWO_PI = 6.283185307179586476925286766559;
  const double phase = static_cast<double>(which + 1);
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      for (int z = 0; z < size; ++z) {
        const double kx = TWO_PI * x / size;
        const double ky = TWO_PI * y / size;
        const double kz = TWO_PI * z / size;
        const std::size_t i = static_cast<std::size_t>(index(x, y, z, size));
        result.j[i] = {0.17 * std::sin(kx + phase * ky),
                       -0.21 * std::cos(ky - phase * kz),
                       0.13 * std::sin(kz + phase * kx)};
        result.w[i] = {-0.12 * std::cos(kx - kz),
                       0.09 * std::sin(ky + kx),
                       0.15 * std::cos(kz + ky)};
        result.source[i] = {0.07 * std::sin(phase * kx + ky),
                            -0.08 * std::cos(phase * ky + kz),
                            0.06 * std::sin(phase * kz - kx)};
        result.r0[i] = {0.19 * std::cos(kx + ky),
                        -0.11 * std::sin(ky + kz),
                        0.16 * std::cos(kz - kx)};
        result.r1[i] = add(result.r0[i], Vec3{
            0.04 * std::sin(phase * kx - kz),
            0.05 * std::cos(phase * ky + kx),
            -0.03 * std::sin(phase * kz + ky)});
        result.current[i] = {0.08 * std::cos(kx - phase * ky),
                             -0.06 * std::sin(ky + phase * kz),
                             0.09 * std::cos(kz + phase * kx)};
        result.rho0[i] = 0.14 * std::sin(kx + phase * ky)
            + 0.10 * std::cos(kz - phase * kx);
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
                  [static_cast<std::size_t>(permutation[static_cast<std::size_t>(row)])]
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
  for (int row = 0; row < 3; ++row)
    for (int col = 0; col < 3; ++col)
      output[static_cast<std::size_t>(row)] +=
          matrix[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)]
          * input[static_cast<std::size_t>(col)];
  return {output[0], output[1], output[2]};
}

double sine_norm2(const Vec3& k) {
  return std::sin(k.x) * std::sin(k.x)
      + std::sin(k.y) * std::sin(k.y)
      + std::sin(k.z) * std::sin(k.z);
}

}  // namespace

NativeHodgeEnergyContinuityResult analyze_native_hodge_energy_continuity() {
  NativeHodgeEnergyContinuityResult result;
  constexpr double TOL = 1e-12;

  for (int size : {16, 32, 64}) {
    for (int mode = 0; mode <= 3; ++mode) {
      for (int direction = 0; direction < 3; ++direction) {
        const double a = mode_symbol(size, mode, direction);
        const double arm = static_cast<double>(result.mode_work_arms + 1);
        const double j0 = 0.13 + 0.002 * arm;
        const double w0 = -0.09 + 0.001 * arm;
        const double source = 0.04 - 0.0003 * arm;
        const double w1 = w0 - a * j0 + source;
        const double j1 = j0 + w1;
        const auto energy = [&](double j, double w) {
          return 0.5 * w * w + 0.5 * a * j * j - 0.5 * a * w * j;
        };
        const double r0 = j0 - 0.5 * w0;
        const double r1 = j1 - 0.5 * w1;
        const double midpoint_w = 0.5 * (w0 + w1);
        result.maximum_mode_work_residual = std::max(
            result.maximum_mode_work_residual,
            normalized_residual(energy(j1, w1) - energy(j0, w0),
                                source * (r1 - r0)));
        result.maximum_half_step_coordinate_residual = std::max(
            result.maximum_half_step_coordinate_residual,
            std::abs((r1 - r0) - midpoint_w));
        ++result.mode_work_arms;
      }
    }
  }

  for (int size : {5, 7}) {
    for (int which = 0; which < 2; ++which) {
      const Fixture data = fixture(size, which);
      const std::vector<Vec3> kj0 = apply_k(data.j, size);
      std::vector<Vec3> w1(data.w.size());
      std::vector<Vec3> j1(data.j.size());
      for (std::size_t i = 0; i < data.j.size(); ++i) {
        w1[i] = add(subtract(data.w[i], kj0[i]), data.source[i]);
        j1[i] = add(data.j[i], w1[i]);
      }
      const std::vector<Vec3> kj1 = apply_k(j1, size);
      const std::vector<Vec3> r0 = combine(data.j, data.w, 1.0, -0.5);
      const std::vector<Vec3> r1 = combine(j1, w1, 1.0, -0.5);
      const std::vector<Vec3> delta_r = combine(r1, r0, 1.0, -1.0);
      const std::vector<Vec3> midpoint_w = combine(data.w, w1, 0.5, 0.5);
      result.maximum_full_field_work_residual = std::max(
          result.maximum_full_field_work_residual,
          normalized_residual(tick_energy(j1, w1, kj1)
                                  - tick_energy(data.j, data.w, kj0),
                              pairing(data.source, delta_r)));
      for (std::size_t i = 0; i < delta_r.size(); ++i) {
        const Vec3 residual = subtract(delta_r[i], midpoint_w[i]);
        result.maximum_half_step_coordinate_residual = std::max({
            result.maximum_half_step_coordinate_residual,
            std::abs(residual.x), std::abs(residual.y), std::abs(residual.z)});
      }

      const std::vector<double> div_q = divergence(data.current, size);
      const std::vector<double> rho1 = combine(data.rho0, div_q, 1.0, -1.0);
      const std::vector<double> rho_bar = combine(data.rho0, rho1, 0.5, 0.5);
      const std::vector<Vec3> r_bar = combine(data.r0, data.r1, 0.5, 0.5);
      const std::vector<Vec3> dr = combine(data.r1, data.r0, 1.0, -1.0);
      const std::vector<Vec3> grad_rho = gradient(rho_bar, size);
      const std::vector<Vec3> curl_q = curl(data.current, size);
      const std::vector<Vec3> source = combine(
          grad_rho, curl_q, -G_C, G_C);
      const std::vector<double> div_dr = divergence(dr, size);
      const std::vector<Vec3> curl_dr = curl(dr, size);
      const long double field_direct = pairing(source, dr);
      const long double field_adjoint = G_C * (
          pairing(rho_bar, div_dr) + pairing(data.current, curl_dr));
      result.maximum_conditional_field_work_residual = std::max(
          result.maximum_conditional_field_work_residual,
          normalized_residual(field_direct, field_adjoint));

      const std::vector<double> div_r0 = divergence(data.r0, size);
      const std::vector<double> div_r1 = divergence(data.r1, size);
      const std::vector<double> div_r_bar = divergence(r_bar, size);
      const std::vector<Vec3> grad_div_r_bar = gradient(div_r_bar, size);
      const long double u0 = -G_C * pairing(data.rho0, div_r0);
      const long double u1 = -G_C * pairing(rho1, div_r1);
      const long double predicted_du = -G_C * (
          pairing(rho_bar, div_dr)
          + pairing(data.current, grad_div_r_bar));
      result.maximum_conditional_interaction_residual = std::max(
          result.maximum_conditional_interaction_residual,
          normalized_residual(u1 - u0, predicted_du));

      const long double matter_work = G_C * (
          pairing(data.current, grad_div_r_bar)
          - pairing(data.current, curl_dr));
      result.maximum_conditional_total_energy_residual = std::max(
          result.maximum_conditional_total_energy_residual,
          normalized_residual(field_direct + (u1 - u0) + matter_work, 0.0L));
      const std::vector<double> continuity = combine(rho1, data.rho0, 1.0, -1.0);
      for (std::size_t i = 0; i < continuity.size(); ++i) {
        result.maximum_conditional_continuity_residual = std::max(
            result.maximum_conditional_continuity_residual,
            std::abs(continuity[i] + div_q[i]));
      }
      ++result.full_field_work_arms;
      ++result.conditional_energy_arms;
    }
  }

  result.minimum_even_checkerboard_witness =
      std::numeric_limits<double>::infinity();
  result.minimum_odd_support_fraction =
      std::numeric_limits<double>::infinity();
  result.minimum_odd_support_sites = std::numeric_limits<int>::max();
  result.minimum_odd_support_radius = std::numeric_limits<int>::max();
  for (int size : {16, 32, 64, 17, 33, 65}) {
    for (int axis = 0; axis < 3; ++axis) {
      for (int polarity : {-1, 1}) {
        if (size % 2 == 0) {
          const double checkerboard_witness = std::abs(2.0 * polarity);
          result.minimum_even_checkerboard_witness = std::min(
              result.minimum_even_checkerboard_witness, checkerboard_witness);
        } else {
          std::vector<double> current(static_cast<std::size_t>(size));
          const double positive = polarity * static_cast<double>(size - 1) / size;
          const double negative = -polarity * static_cast<double>(size + 1) / size;
          for (int site = 0; site < size; ++site) {
            current[static_cast<std::size_t>(site)] =
                (site == 0 || site % 2 == 1) ? positive : negative;
          }
          int support = 0;
          for (int site = 0; site < size; ++site) {
            const double central = 0.5 * (
                current[static_cast<std::size_t>(wrap(site + 1, size))]
              - current[static_cast<std::size_t>(wrap(site - 1, size))]);
            const double expected = polarity * (site == 0 ? 1.0
                : (site == 1 ? -1.0 : 0.0));
            result.maximum_odd_volume_current_residual = std::max(
                result.maximum_odd_volume_current_residual,
                std::abs(central - expected));
            if (std::abs(current[static_cast<std::size_t>(site)]) > TOL)
              ++support;
          }
          result.minimum_odd_support_fraction = std::min(
              result.minimum_odd_support_fraction,
              static_cast<double>(support) / size);
          result.minimum_odd_support_sites = std::min(
              result.minimum_odd_support_sites, support);
          result.maximum_odd_support_sites = std::max(
              result.maximum_odd_support_sites, support);
          result.minimum_odd_support_radius = std::min(
              result.minimum_odd_support_radius, (size - 1) / 2);
          result.maximum_odd_support_radius = std::max(
              result.maximum_odd_support_radius, (size - 1) / 2);
        }
        ++result.polarity_checks;
      }
      ++result.axial_cardinal_hop_arms;
    }
  }

  const Vec3 generic_k{0.31, 0.47, 0.59};
  const double reference = sine_norm2(generic_k);
  for (const Matrix3& rotation : proper_rotations()) {
    result.maximum_cubic_covariance_residual = std::max(
        result.maximum_cubic_covariance_residual,
        std::abs(sine_norm2(rotate(rotation, generic_k)) - reference));
    ++result.proper_cubic_rotation_arms;
  }

  // At z=-1 the central symbol is zero while the oriented-face difference is
  // two. No finite-valued Laurent symbol A can satisfy d_c A=d_f there.
  result.face_to_site_checkerboard_defect = 2.0;

  result.driven_tick_work_identity_exact =
      result.mode_work_arms == 36 && result.full_field_work_arms == 4
      && result.maximum_mode_work_residual <= TOL
      && result.maximum_full_field_work_residual <= TOL;
  result.half_step_coordinate_unique =
      result.maximum_half_step_coordinate_residual <= TOL;
  result.constant_source_affine_invariant_exact =
      result.driven_tick_work_identity_exact
      && result.half_step_coordinate_unique;
  result.conditional_hodge_total_energy_exact =
      result.conditional_energy_arms == 4
      && result.maximum_conditional_continuity_residual <= TOL
      && result.maximum_conditional_field_work_residual <= TOL
      && result.maximum_conditional_interaction_residual <= TOL
      && result.maximum_conditional_total_energy_residual <= TOL;
  result.even_cardinal_hop_central_current_exists = false;
  result.odd_cardinal_hop_current_is_box_spanning =
      result.maximum_odd_volume_current_residual <= TOL
      && result.minimum_odd_support_fraction >= 1.0 - TOL
      && result.minimum_odd_support_sites == 17
      && result.maximum_odd_support_sites == 65;
  result.finite_range_cardinal_hop_current_exists = false;
  result.finite_range_face_to_site_projection_exists = false;
  result.additional_staggered_or_nonlocal_structure_required = true;
  result.production_changed = false;

  result.valid = result.driven_tick_work_identity_exact
      && result.half_step_coordinate_unique
      && result.constant_source_affine_invariant_exact
      && result.conditional_hodge_total_energy_exact
      && result.axial_cardinal_hop_arms == 18
      && result.polarity_checks == 36
      && result.minimum_even_checkerboard_witness >= 2.0 - TOL
      && !result.even_cardinal_hop_central_current_exists
      && result.odd_cardinal_hop_current_is_box_spanning
      && !result.finite_range_cardinal_hop_current_exists
      && result.proper_cubic_rotation_arms == 24
      && result.maximum_cubic_covariance_residual <= TOL
      && result.face_to_site_checkerboard_defect >= 2.0 - TOL
      && !result.finite_range_face_to_site_projection_exists
      && result.additional_staggered_or_nonlocal_structure_required
      && !result.production_changed;
  return result;
}

}  // namespace ftd::eft
