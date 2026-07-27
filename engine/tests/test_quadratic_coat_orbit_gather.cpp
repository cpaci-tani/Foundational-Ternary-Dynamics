/** FTD-0550: quadratic-coat adjoint orbit gather and commuting curl. */

#include "ftd/eft/quadratic_coat_orbit_gather.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double gate = 5e-13;
int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double vec_residual(const ftd::Vec3& lhs, const ftd::Vec3& rhs,
                    double sign = 1.0) {
  return std::max({std::abs(lhs.x-sign*rhs.x),
                   std::abs(lhs.y-sign*rhs.y),
                   std::abs(lhs.z-sign*rhs.z)});
}

ftd::Vec3 cyclic(const ftd::Vec3& value) {
  return {value.y, value.z, value.x};
}

ftd::eft::MatchedFaceFlux make_face_field(double phase) {
  ftd::eft::MatchedFaceFlux field(L);
  constexpr double two_pi = 6.283185307179586476925286766559;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = field.index(x, y, z);
        field.x[static_cast<std::size_t>(i)] = 0.031
            +0.009*std::sin(two_pi*(x+2*y+3*z)/L+phase)
            +0.004*std::cos(two_pi*(2*x-z)/L-0.3*phase);
        field.y[static_cast<std::size_t>(i)] = -0.043
            +0.008*std::cos(two_pi*(3*x+y+z)/L-0.7*phase)
            +0.003*std::sin(two_pi*(y-2*z)/L+phase);
        field.z[static_cast<std::size_t>(i)] = 0.027
            +0.007*std::sin(two_pi*(x-3*y+2*z)/L+0.5*phase)
            -0.005*std::cos(two_pi*(x+z)/L-phase);
      }
    }
  }
  return field;
}

ftd::eft::MatchedFaceFlux translate_face(
    const ftd::eft::MatchedFaceFlux& source, const ftd::Coord& shift) {
  ftd::eft::MatchedFaceFlux target(source.L);
  for (int x = 0; x < source.L; ++x)
    for (int y = 0; y < source.L; ++y)
      for (int z = 0; z < source.L; ++z) {
        const int from = source.index(x, y, z);
        const int to = target.index(x+shift.x, y+shift.y, z+shift.z);
        target.x[static_cast<std::size_t>(to)] =
            source.x[static_cast<std::size_t>(from)];
        target.y[static_cast<std::size_t>(to)] =
            source.y[static_cast<std::size_t>(from)];
        target.z[static_cast<std::size_t>(to)] =
            source.z[static_cast<std::size_t>(from)];
      }
  return target;
}

ftd::eft::MatchedEdgeField translate_edge(
    const ftd::eft::MatchedEdgeField& source, const ftd::Coord& shift) {
  ftd::eft::MatchedEdgeField target(source.L);
  for (int x = 0; x < source.L; ++x)
    for (int y = 0; y < source.L; ++y)
      for (int z = 0; z < source.L; ++z) {
        const int from = source.index(x, y, z);
        const int to = target.index(x+shift.x, y+shift.y, z+shift.z);
        target.x[static_cast<std::size_t>(to)] =
            source.x[static_cast<std::size_t>(from)];
        target.y[static_cast<std::size_t>(to)] =
            source.y[static_cast<std::size_t>(from)];
        target.z[static_cast<std::size_t>(to)] =
            source.z[static_cast<std::size_t>(from)];
      }
  return target;
}

ftd::eft::MatchedFaceFlux cyclic_face(
    const ftd::eft::MatchedFaceFlux& source) {
  ftd::eft::MatchedFaceFlux target(source.L);
  for (int x = 0; x < source.L; ++x)
    for (int y = 0; y < source.L; ++y)
      for (int z = 0; z < source.L; ++z) {
        const int from = source.index(x, y, z);
        const int to = target.index(y, z, x);
        target.x[static_cast<std::size_t>(to)] =
            source.y[static_cast<std::size_t>(from)];
        target.y[static_cast<std::size_t>(to)] =
            source.z[static_cast<std::size_t>(from)];
        target.z[static_cast<std::size_t>(to)] =
            source.x[static_cast<std::size_t>(from)];
      }
  return target;
}

ftd::eft::MatchedEdgeField cyclic_edge(
    const ftd::eft::MatchedEdgeField& source) {
  ftd::eft::MatchedEdgeField target(source.L);
  for (int x = 0; x < source.L; ++x)
    for (int y = 0; y < source.L; ++y)
      for (int z = 0; z < source.L; ++z) {
        const int from = source.index(x, y, z);
        const int to = target.index(y, z, x);
        target.x[static_cast<std::size_t>(to)] =
            source.y[static_cast<std::size_t>(from)];
        target.y[static_cast<std::size_t>(to)] =
            source.z[static_cast<std::size_t>(from)];
        target.z[static_cast<std::size_t>(to)] =
            source.x[static_cast<std::size_t>(from)];
      }
  return target;
}

}  // namespace

int main() {
  const auto electric = make_face_field(0.37);
  const auto potential = make_face_field(-0.21);
  const auto magnetic = ftd::eft::matched_curl_adjoint(potential);

  std::vector<ftd::Vec3> curl_samples;
  for (int i = 0; i < 18; ++i)
    curl_samples.push_back({2.137+0.417*i, 3.219+0.283*i,
                            4.311+0.193*i});
  const double curl_residual =
      ftd::eft::quadratic_spline_curl_commutation_residual(
          potential, curl_samples);
  check("quadratic reconstruction commutes with matched C^T curl",
        curl_residual <= gate);

  const std::array<ftd::Vec3, 6> raw_directions{{
      {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0},
      {1.0, 1.0, 0.0}, {1.0, -1.0, 1.0}, {1.0, 1.0, 1.0}}};
  const std::array<double, 3> speeds{{0.08, 0.16, 0.24}};
  const std::array<ftd::Vec3, 2> starts{{
      {7.173, 7.281, 7.397}, {9.173, 6.281, 10.397}}};
  int arms = 0;
  double worst_adjoint = 0.0;
  double worst_zero_work = 0.0;
  double worst_kinematic = 0.0;
  double worst_polarity = 0.0;
  double worst_reversal = 0.0;
  double axial_transverse = 0.0;
  bool campaign_valid = true;
  for (const auto& raw : raw_directions) {
    const double inverse_norm = 1.0/raw.mag();
    const ftd::Vec3 direction = raw*inverse_norm;
    for (double speed : speeds) {
      const ftd::Vec3 velocity = direction*speed;
      for (const auto& start : starts) {
        const ftd::Vec3 end = start+velocity;
        for (int charge : {-1, +1}) {
          const auto segment = ftd::eft::make_quadratic_coat_face_current(
              L, start, end, charge);
          const auto gather =
              ftd::eft::evaluate_quadratic_coat_orbit_gather(
                  segment, electric, magnetic, velocity, 1.0, 0.73);
          const auto mirror_segment =
              ftd::eft::make_quadratic_coat_face_current(
                  L, start, end, -charge);
          const auto mirror =
              ftd::eft::evaluate_quadratic_coat_orbit_gather(
                  mirror_segment, electric, magnetic, velocity, 1.0, 0.73);
          const auto reverse_segment =
              ftd::eft::make_quadratic_coat_face_current(
                  L, end, start, charge);
          const auto reverse =
              ftd::eft::evaluate_quadratic_coat_orbit_gather(
                  reverse_segment, electric, magnetic, velocity*(-1.0),
                  1.0, 0.73);
          ++arms;
          campaign_valid = campaign_valid && gather.valid && mirror.valid
              && reverse.valid;
          worst_adjoint = std::max(worst_adjoint,
              gather.electric_adjoint_residual);
          worst_zero_work = std::max(worst_zero_work,
              gather.magnetic_work_residual);
          worst_kinematic = std::max(worst_kinematic,
              gather.kinematic_residual);
          worst_polarity = std::max({worst_polarity,
              vec_residual(gather.electric_force,
                           mirror.electric_force, -1.0),
              vec_residual(gather.magnetic_impulse,
                           mirror.magnetic_impulse, -1.0),
              std::abs(gather.current_work+mirror.current_work)});
          worst_reversal = std::max({worst_reversal,
              vec_residual(gather.electric_force, reverse.electric_force),
              vec_residual(gather.magnetic_average,
                           reverse.magnetic_average),
              vec_residual(gather.magnetic_impulse,
                           reverse.magnetic_impulse, -1.0),
              std::abs(gather.current_work+reverse.current_work)});
          if (raw.y == 0.0 && raw.z == 0.0)
            axial_transverse = std::max(axial_transverse,
                std::max(std::abs(gather.electric_force.y),
                         std::abs(gather.electric_force.z)));
        }
      }
    }
  }
  check("all 72 registered signed axial/diagonal orbit arms close",
        campaign_valid && arms == 72 && worst_adjoint <= gate
        && worst_zero_work <= gate && worst_kinematic <= gate);
  check("axial paths retain determined transverse electric force",
        axial_transverse > 1e-8);
  check("polarity mirror and path reversal transform the gathers exactly",
        worst_polarity <= gate && worst_reversal <= gate);

  const ftd::Vec3 base_start{7.173, 7.281, 7.397};
  const ftd::Vec3 base_velocity{0.11, -0.07, 0.09};
  const auto base_segment = ftd::eft::make_quadratic_coat_face_current(
      L, base_start, base_start+base_velocity, +1);
  const auto base = ftd::eft::evaluate_quadratic_coat_orbit_gather(
      base_segment, electric, magnetic, base_velocity, 1.0, 0.73);
  const ftd::Coord shift{2, -1, 3};
  const ftd::Vec3 shift_vector{2.0, -1.0, 3.0};
  const auto shifted_electric = translate_face(electric, shift);
  const auto shifted_magnetic = translate_edge(magnetic, shift);
  const auto shifted_segment = ftd::eft::make_quadratic_coat_face_current(
      L, base_start+shift_vector,
      base_start+shift_vector+base_velocity, +1);
  const auto shifted = ftd::eft::evaluate_quadratic_coat_orbit_gather(
      shifted_segment, shifted_electric, shifted_magnetic,
      base_velocity, 1.0, 0.73);
  const double translation_residual = std::max({
      vec_residual(base.electric_force, shifted.electric_force),
      vec_residual(base.magnetic_average, shifted.magnetic_average),
      vec_residual(base.magnetic_impulse, shifted.magnetic_impulse),
      std::abs(base.current_work-shifted.current_work)});
  check("integer translation transports path and staggered fields together",
        base.valid && shifted.valid && translation_residual <= gate);

  const auto rotated_electric = cyclic_face(electric);
  const auto rotated_magnetic = cyclic_edge(magnetic);
  const auto rotated_segment = ftd::eft::make_quadratic_coat_face_current(
      L, cyclic(base_start), cyclic(base_start+base_velocity), +1);
  const auto rotated = ftd::eft::evaluate_quadratic_coat_orbit_gather(
      rotated_segment, rotated_electric, rotated_magnetic,
      cyclic(base_velocity), 1.0, 0.73);
  const double rotation_residual = std::max({
      vec_residual(cyclic(base.electric_force), rotated.electric_force),
      vec_residual(cyclic(base.magnetic_average), rotated.magnetic_average),
      vec_residual(cyclic(base.magnetic_impulse), rotated.magnetic_impulse),
      std::abs(base.current_work-rotated.current_work)});
  check("cyclic cubic rotation transports the staggered orbit complex",
        rotated.valid && rotation_residual <= gate);

  ftd::eft::MatchedFaceFlux nan_field = electric;
  nan_field.x[0] = NAN;
  const auto invalid_charge = ftd::eft::make_quadratic_coat_face_current(
      L, base_start, base_start+base_velocity, 0);
  const ftd::Vec3 fast_velocity{ftd::C_SPEED+0.01, 0.0, 0.0};
  const auto fast_segment = ftd::eft::make_quadratic_coat_face_current(
      L, base_start, base_start+fast_velocity, +1);
  const auto wrong_size = make_face_field(0.0);
  ftd::eft::MatchedFaceFlux actually_wrong(L-1);
  check("invalid fields, charges, sizes, kinematics, and speeds fail closed",
      !ftd::eft::evaluate_quadratic_coat_orbit_gather(
          base_segment, nan_field, magnetic, base_velocity, 1.0).valid
      && !ftd::eft::evaluate_quadratic_coat_orbit_gather(
          invalid_charge, electric, magnetic, base_velocity, 1.0).valid
      && !ftd::eft::evaluate_quadratic_coat_orbit_gather(
          base_segment, actually_wrong, magnetic, base_velocity, 1.0).valid
      && !ftd::eft::evaluate_quadratic_coat_orbit_gather(
          base_segment, wrong_size, magnetic, base_velocity*0.5, 1.0).valid
      && !ftd::eft::evaluate_quadratic_coat_orbit_gather(
          fast_segment, electric, magnetic, fast_velocity, 1.0).valid);

  const bool constructive = failures == 0 && arms >= 72
      && worst_adjoint <= gate && curl_residual <= gate
      && worst_zero_work <= gate && worst_polarity <= gate
      && worst_reversal <= gate && translation_residual <= gate
      && rotation_residual <= gate && axial_transverse > 1e-8;
  const char* verdict = constructive
      ? "QUADRATIC_COAT_ORBIT_GATHER_CONSTRUCTIVE"
      : (worst_adjoint <= gate
          ? "ELECTRIC_GATHER_CONSTRUCTIVE_MAGNETIC_ORIGIN_UNRESOLVED"
          : "QUADRATIC_COAT_ORBIT_GATHER_CLOSED_NEGATIVE");
  std::cout << "arms," << arms << '\n'
            << "worst_electric_adjoint_residual," << worst_adjoint << '\n'
            << "curl_commutation_residual," << curl_residual << '\n'
            << "worst_magnetic_work_residual," << worst_zero_work << '\n'
            << "worst_kinematic_residual," << worst_kinematic << '\n'
            << "maximum_axial_transverse_force," << axial_transverse << '\n'
            << "worst_polarity_residual," << worst_polarity << '\n'
            << "worst_reversal_residual," << worst_reversal << '\n'
            << "translation_residual," << translation_residual << '\n'
            << "rotation_residual," << rotation_residual << '\n'
            << "verdict," << verdict << '\n';
  return failures == 0 ? 0 : 1;
}
