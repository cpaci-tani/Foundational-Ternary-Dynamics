/** FTD-0494: cellwise cusp primitive and global gluing obstruction. */

#include "ftd/eft/cusp_dressing_integrability.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>

namespace {

constexpr int L = 8;
constexpr ftd::Coord site{2, 2, 3};
constexpr double jump_amplitude = 0.3;
constexpr double coupling = 0.73;
constexpr double gate = 1e-12;
int failures = 0;
double worst_local_residual = 0.0;
double worst_reverse_residual = 0.0;
double worst_holonomy_residual = 0.0;
double worst_divergence = 0.0;
double worst_symmetry_residual = 0.0;
double threshold_mismatch = 0.0;
double source_free_holonomy = 0.0;
double field_euler_norm = 0.0;
double finite_difference_euler_residual = 0.0;
double branch_trace_gradient_residual = 0.0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double checker(int x, int y, int shift_x = 0, int shift_y = 0) {
  const int parity = (x - shift_x + y - shift_y) & 1;
  return parity == 0 ? jump_amplitude : -jump_amplitude;
}

ftd::eft::MatchedFaceFlux checker_field(
    int shift_x = 0, int shift_y = 0, double sign = 1.0) {
  ftd::eft::MatchedFaceFlux field(L);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = field.index(x, y, z);
        const double h = sign * checker(x, y, shift_x, shift_y);
        field.x[static_cast<std::size_t>(i)] = 0.5 * h;
        field.y[static_cast<std::size_t>(i)] = -0.5 * h;
        field.z[static_cast<std::size_t>(i)] = 0.0;
      }
    }
  }
  return field;
}

double fixture_max_divergence(const ftd::eft::MatchedFaceFlux& field) {
  double result = 0.0;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        result = std::max(result, std::abs(
            ftd::eft::divergence_at(field, x, y, z)));
      }
    }
  }
  return result;
}

ftd::Vec3 permute(const ftd::Vec3& value,
                  const std::array<int, 3>& p) {
  const double v[3] = {value.x, value.y, value.z};
  return {v[p[0]], v[p[1]], v[p[2]]};
}

double finite_difference_field_gradient_norm(
    const ftd::eft::MatchedFaceFlux& field,
    ftd::Coord knot,
    const ftd::Vec3& remainder,
    int charge,
    double g) {
  constexpr double epsilon = 0.125;
  long double norm_squared = 0.0L;
  for (int axis = 0; axis < 3; ++axis) {
    for (int side : {-1, +1}) {
      auto plus = field;
      auto minus = field;
      ftd::Coord face = knot;
      if (side < 0) {
        if (axis == 0) --face.x;
        if (axis == 1) --face.y;
        if (axis == 2) --face.z;
      }
      const int index = field.index(face.x, face.y, face.z);
      std::vector<double>* plus_component = axis == 0 ? &plus.x
          : axis == 1 ? &plus.y : &plus.z;
      std::vector<double>* minus_component = axis == 0 ? &minus.x
          : axis == 1 ? &minus.y : &minus.z;
      (*plus_component)[static_cast<std::size_t>(index)] += epsilon;
      (*minus_component)[static_cast<std::size_t>(index)] -= epsilon;
      const auto plus_trace = ftd::eft::evaluate_centered_knot_trace(
          plus, knot);
      const auto minus_trace = ftd::eft::evaluate_centered_knot_trace(
          minus, knot);
      const double u_plus = ftd::eft::local_cusp_dressing_energy(
          plus_trace.outgoing - plus_trace.incoming,
          remainder, charge, g);
      const double u_minus = ftd::eft::local_cusp_dressing_energy(
          minus_trace.outgoing - minus_trace.incoming,
          remainder, charge, g);
      const double derivative = (u_plus - u_minus) / (2.0 * epsilon);
      norm_squared += static_cast<long double>(derivative) * derivative;
    }
  }
  return std::sqrt(static_cast<double>(norm_squared));
}

}  // namespace

int main() {
  const ftd::Vec3 remainder{0.23, -0.31, 0.17};
  const auto field = checker_field();
  worst_divergence = fixture_max_divergence(field);
  const auto reference = ftd::eft::evaluate_cusp_dressing_integrability(
      field, site, remainder, +1, coupling);
  worst_local_residual = reference.local_primitive_residual;
  worst_reverse_residual = reference.reverse_residual;
  worst_holonomy_residual = reference.holonomy_residual;
  threshold_mismatch = reference.threshold_representation_mismatch;
  source_free_holonomy = reference.plaquette_holonomy_xy;
  field_euler_norm = reference.field_euler_derivative_l2;
  branch_trace_gradient_residual =
      reference.branch_trace_gradient_residual;

  check("cellwise cusp work has an exact local primitive",
        reference.valid && reference.local_primitive_residual <= gate
        && reference.reverse_residual <= gate);
  check("varying the primitive restores the one-sided branch trace",
        branch_trace_gradient_residual <= gate);
  check("checker face field is exactly Gauss-free",
        worst_divergence <= gate
        && std::abs(reference.local_divergence) <= gate);
  check("hop-equivalent representations have a finite raw mismatch",
        threshold_mismatch > 1e-6
        && std::abs(ftd::eft::local_cusp_dressing_energy(
                        reference.jump, {1.0, 0.0, 0.0}, +1, coupling)
                    - reference.threshold_site_offset_increment.x) <= gate
        && std::abs(ftd::eft::local_cusp_dressing_energy(
                        reference.jump, {}, +1, coupling)) <= gate);
  check("Gauss-free checker has nonzero plaquette holonomy",
        std::abs(source_free_holonomy) > 1e-6
        && reference.holonomy_residual <= gate
        && std::abs(reference.path_xy - coupling * jump_amplitude) <= gate
        && std::abs(reference.path_yx + coupling * jump_amplitude) <= gate);
  check("cellwise energy adds a nonzero field Euler derivative",
        field_euler_norm > 1e-6
        && reference.field_euler_derivative_residual <= gate);
  const double finite_difference_euler_norm =
      finite_difference_field_gradient_norm(
          field, site, remainder, +1, coupling);
  finite_difference_euler_residual = std::abs(
      finite_difference_euler_norm - field_euler_norm);
  check("six-face finite difference confirms the extra field equation",
        finite_difference_euler_residual <= gate);

  const auto mirrored = ftd::eft::evaluate_cusp_dressing_integrability(
      checker_field(0, 0, -1.0), site, remainder * -1.0,
      -1, coupling);
  const double mirror_residual = std::max({
      std::abs(mirrored.local_energy - reference.local_energy),
      std::abs(mirrored.plaquette_holonomy_xy
               - reference.plaquette_holonomy_xy),
      std::abs(mirrored.field_euler_derivative_l2
               - reference.field_euler_derivative_l2)});
  worst_symmetry_residual = std::max(worst_symmetry_residual,
                                    mirror_residual);
  check("polarity/field mirror preserves the dressing obstruction",
        mirrored.valid && mirror_residual <= gate);

  const ftd::Coord shifted_site{site.x + 1, site.y + 1, site.z};
  const auto translated = ftd::eft::evaluate_cusp_dressing_integrability(
      checker_field(1, 1), shifted_site, remainder, +1, coupling);
  const double translation_residual = std::max({
      std::abs(translated.local_energy - reference.local_energy),
      std::abs(translated.plaquette_holonomy_xy
               - reference.plaquette_holonomy_xy),
      std::abs(translated.field_euler_derivative_l2
               - reference.field_euler_derivative_l2)});
  worst_symmetry_residual = std::max(worst_symmetry_residual,
                                    translation_residual);
  check("integer translation of field and particle is exact",
        translated.valid && translation_residual <= gate);

  const std::array<std::array<int, 3>, 6> permutations{{
      {{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}},
      {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};
  const ftd::Vec3 arbitrary_jump{0.4, -0.2, 0.7};
  const double arbitrary_energy = ftd::eft::local_cusp_dressing_energy(
      arbitrary_jump, remainder, +1, coupling);
  bool cubic_ok = true;
  for (const auto& p : permutations) {
    for (int sx : {-1, +1}) {
      for (int sy : {-1, +1}) {
        for (int sz : {-1, +1}) {
          ftd::Vec3 transformed_remainder = permute(remainder, p);
          transformed_remainder.x *= sx;
          transformed_remainder.y *= sy;
          transformed_remainder.z *= sz;
          // Face jumps are parity-even divergence contributions and only
          // permute under signed cubic transformations.
          const ftd::Vec3 transformed_jump = permute(arbitrary_jump, p);
          const double transformed_energy =
              ftd::eft::local_cusp_dressing_energy(
                  transformed_jump, transformed_remainder, +1, coupling);
          const double residual = std::abs(
              transformed_energy - arbitrary_energy);
          worst_symmetry_residual = std::max(worst_symmetry_residual,
                                              residual);
          cubic_ok = cubic_ok && residual <= gate;
        }
      }
    }
  }
  check("cellwise primitive is covariant under all 48 cubic maps", cubic_ok);

  ftd::eft::MatchedFaceFlux zero_field(L);
  const auto zero_at_knot = ftd::eft::evaluate_cusp_dressing_integrability(
      zero_field, site, {}, +1, coupling);
  const auto zero_jump_moving =
      ftd::eft::evaluate_cusp_dressing_integrability(
          zero_field, site, remainder, +1, coupling);
  check("zero-jump controls distinguish value from field variation",
        zero_at_knot.valid && zero_jump_moving.valid
        && std::abs(zero_at_knot.local_energy) <= gate
        && std::abs(zero_at_knot.plaquette_holonomy_xy) <= gate
        && zero_at_knot.field_euler_derivative_l2 <= gate
        && std::abs(zero_jump_moving.local_energy) <= gate
        && zero_jump_moving.field_euler_derivative_l2 > 1e-6);

  check("invalid polarity fails closed",
        !ftd::eft::evaluate_cusp_dressing_integrability(
            field, site, remainder, 0, coupling).valid);

  std::cout.precision(17);
  std::cout << "local_energy=" << reference.local_energy << '\n'
            << "threshold_representation_mismatch="
            << threshold_mismatch << '\n'
            << "path_xy=" << reference.path_xy << '\n'
            << "path_yx=" << reference.path_yx << '\n'
            << "source_free_holonomy=" << source_free_holonomy << '\n'
            << "field_euler_derivative_l2=" << field_euler_norm << '\n'
            << "finite_difference_euler_residual="
            << finite_difference_euler_residual << '\n'
            << "branch_trace_gradient_residual="
            << branch_trace_gradient_residual << '\n'
            << "worst_local_primitive_residual="
            << worst_local_residual << '\n'
            << "worst_reverse_residual=" << worst_reverse_residual << '\n'
            << "worst_holonomy_residual="
            << worst_holonomy_residual << '\n'
            << "worst_divergence=" << worst_divergence << '\n'
            << "worst_symmetry_residual="
            << worst_symmetry_residual << '\n'
            << "cusp_dressing_integrability failures=" << failures << '\n'
            << "verdict=CELLWISE_PRIMITIVE_GLOBAL_MEMORY_OBSTRUCTION\n";
  return failures == 0 ? 0 : 1;
}
