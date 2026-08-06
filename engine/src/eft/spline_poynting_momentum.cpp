#include "ftd/eft/spline_poynting_momentum.h"

#include "ftd/eft/local_polarity_regularity.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

namespace ftd::eft {
namespace {

enum class Kernel { B1, B2 };

struct ShiftWeight {
  int shift = 0;
  double weight = 0.0;
};

double kernel_value(Kernel kernel, double value) {
  return evaluate_local_polarity_kernel(
      kernel == Kernel::B1 ? LocalPolarityKernel::Hat
                           : LocalPolarityKernel::QuadraticBSpline,
      value);
}

void append_knots(std::vector<double>& knots, Kernel kernel, double center) {
  if (kernel == Kernel::B1) {
    knots.insert(knots.end(), {center - 1.0, center, center + 1.0});
  } else {
    knots.insert(knots.end(), {center - 1.5, center - 0.5,
                               center + 0.5, center + 1.5});
  }
}

double overlap(Kernel lhs, Kernel rhs, double relative_center) {
  // Four-point Gauss-Legendre is exact on each polynomial piece (degree <=4).
  constexpr std::array<long double, 4> nodes{{
      -0.861136311594052575223946488893L,
      -0.339981043584856264802665759103L,
       0.339981043584856264802665759103L,
       0.861136311594052575223946488893L}};
  constexpr std::array<long double, 4> weights{{
      0.347854845137453857373063949222L,
      0.652145154862546142626936050778L,
      0.652145154862546142626936050778L,
      0.347854845137453857373063949222L}};
  std::vector<double> knots;
  knots.reserve(8);
  append_knots(knots, lhs, 0.0);
  append_knots(knots, rhs, relative_center);
  std::sort(knots.begin(), knots.end());
  knots.erase(std::unique(knots.begin(), knots.end()), knots.end());
  long double integral = 0.0L;
  for (std::size_t piece = 1; piece < knots.size(); ++piece) {
    const long double lo = knots[piece - 1];
    const long double hi = knots[piece];
    const long double midpoint = 0.5L * (lo + hi);
    const long double half_width = 0.5L * (hi - lo);
    for (std::size_t sample = 0; sample < nodes.size(); ++sample) {
      const double x = static_cast<double>(midpoint
          + half_width * nodes[sample]);
      integral += half_width * weights[sample]
          * static_cast<long double>(kernel_value(lhs, x))
          * static_cast<long double>(kernel_value(
              rhs, x - relative_center));
    }
  }
  return static_cast<double>(integral);
}

Kernel face_kernel(int component, int axis) {
  return component == axis ? Kernel::B1 : Kernel::B2;
}

Kernel edge_kernel(int component, int axis) {
  return component == axis ? Kernel::B2 : Kernel::B1;
}

double face_shift(int component, int axis) {
  return component == axis ? 0.5 : 0.0;
}

double edge_shift(int component, int axis) {
  return component == axis ? 0.0 : 0.5;
}

std::vector<ShiftWeight> overlap_stencil(
    int face_component, int edge_component, int axis) {
  std::vector<ShiftWeight> result;
  const Kernel lhs = face_kernel(face_component, axis);
  const Kernel rhs = edge_kernel(edge_component, axis);
  const double lhs_shift = face_shift(face_component, axis);
  const double rhs_shift = edge_shift(edge_component, axis);
  for (int shift = -3; shift <= 3; ++shift) {
    const double value = overlap(
        lhs, rhs, static_cast<double>(shift) + rhs_shift - lhs_shift);
    if (std::abs(value) > 1e-18) result.push_back({shift, value});
  }
  return result;
}

const std::vector<double>& face_values(
    const MatchedFaceFlux& field, int component) {
  return component == 0 ? field.x : (component == 1 ? field.y : field.z);
}

const std::vector<double>& edge_values(
    const MatchedEdgeField& field, int component) {
  return component == 0 ? field.x : (component == 1 ? field.y : field.z);
}

void convolve_axis(int L, const std::vector<double>& input,
                   std::vector<double>& output, int axis,
                   const std::vector<ShiftWeight>& stencil) {
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        long double value = 0.0L;
        for (const auto& item : stencil) {
          int sx = x, sy = y, sz = z;
          if (axis == 0) sx += item.shift;
          if (axis == 1) sy += item.shift;
          if (axis == 2) sz += item.shift;
          int wx = sx % L, wy = sy % L, wz = sz % L;
          if (wx < 0) wx += L;
          if (wy < 0) wy += L;
          if (wz < 0) wz += L;
          const std::size_t index = static_cast<std::size_t>(
              (wx * L + wy) * L + wz);
          value += static_cast<long double>(item.weight) * input[index];
        }
        output[static_cast<std::size_t>((x * L + y) * L + z)]
            = static_cast<double>(value);
      }
    }
  }
}

double integrated_pair(const MatchedFaceFlux& electric,
                       const MatchedEdgeField& magnetic,
                       int face_component, int edge_component) {
  const int L = electric.L;
  const std::size_t count = electric.x.size();
  std::vector<double> first(count, 0.0), second(count, 0.0), third(count, 0.0);
  const auto sx = overlap_stencil(face_component, edge_component, 0);
  const auto sy = overlap_stencil(face_component, edge_component, 1);
  const auto sz = overlap_stencil(face_component, edge_component, 2);
  convolve_axis(L, edge_values(magnetic, edge_component), first, 0, sx);
  convolve_axis(L, first, second, 1, sy);
  convolve_axis(L, second, third, 2, sz);
  long double result = 0.0L;
  const auto& face = face_values(electric, face_component);
  for (std::size_t i = 0; i < count; ++i)
    result += static_cast<long double>(face[i]) * third[i];
  return static_cast<double>(result);
}

bool valid_field(const MatchedFaceFlux& field) {
  const std::size_t count = field.L > 0
      ? static_cast<std::size_t>(field.L) * field.L * field.L : 0;
  if (field.x.size() != count || field.y.size() != count
      || field.z.size() != count) return false;
  const auto finite = [](const std::vector<double>& values) {
    return std::all_of(values.begin(), values.end(),
        [](double value) { return std::isfinite(value); });
  };
  return finite(field.x) && finite(field.y) && finite(field.z);
}

bool valid_field(const MatchedEdgeField& field) {
  const std::size_t count = field.L > 0
      ? static_cast<std::size_t>(field.L) * field.L * field.L : 0;
  if (field.x.size() != count || field.y.size() != count
      || field.z.size() != count) return false;
  const auto finite = [](const std::vector<double>& values) {
    return std::all_of(values.begin(), values.end(),
        [](double value) { return std::isfinite(value); });
  };
  return finite(field.x) && finite(field.y) && finite(field.z);
}

}  // namespace

MatchedEdgeField matched_integer_time_magnetic(
    const MatchedFaceFlux& electric,
    const MatchedEdgeField& magnetic_half,
    double wave_speed,
    double dt) {
  MatchedEdgeField result(magnetic_half.L);
  if (electric.L <= 0 || electric.L != magnetic_half.L
      || !valid_field(electric) || !valid_field(magnetic_half)
      || !(wave_speed > 0.0) || !std::isfinite(dt)) return result;
  result = magnetic_half;
  const auto curl = matched_curl_adjoint(electric);
  const double scale = -0.5 * wave_speed * dt;
  for (std::size_t i = 0; i < result.x.size(); ++i) {
    result.x[i] += scale * curl.x[i];
    result.y[i] += scale * curl.y[i];
    result.z[i] += scale * curl.z[i];
  }
  return result;
}

Vec3 integrate_quadratic_spline_cross(
    const MatchedFaceFlux& electric,
    const MatchedEdgeField& magnetic_integer) {
  if (electric.L <= 0 || electric.L != magnetic_integer.L
      || !valid_field(electric) || !valid_field(magnetic_integer))
    return {NAN, NAN, NAN};
  return {
      integrated_pair(electric, magnetic_integer, 1, 2)
          - integrated_pair(electric, magnetic_integer, 2, 1),
      integrated_pair(electric, magnetic_integer, 2, 0)
          - integrated_pair(electric, magnetic_integer, 0, 2),
      integrated_pair(electric, magnetic_integer, 0, 1)
          - integrated_pair(electric, magnetic_integer, 1, 0)};
}

SplinePoyntingMomentumResult measure_spline_poynting_momentum(
    const MatchedFaceFlux& electric,
    const MatchedEdgeField& magnetic_half,
    double wave_speed,
    double dt,
    double beta) {
  SplinePoyntingMomentumResult result;
  result.beta = beta;
  result.wave_speed = wave_speed;
  result.dt = dt;
  if (!(wave_speed > 0.0) || !(beta > 0.0) || !std::isfinite(dt)
      || electric.L <= 0 || electric.L != magnetic_half.L) return result;
  const auto magnetic_integer = matched_integer_time_magnetic(
      electric, magnetic_half, wave_speed, dt);
  result.integrated_cross = integrate_quadratic_spline_cross(
      electric, magnetic_integer);
  result.momentum = result.integrated_cross * (beta / wave_speed);
  result.valid = std::isfinite(result.momentum.x)
      && std::isfinite(result.momentum.y)
      && std::isfinite(result.momentum.z);
  return result;
}

}  // namespace ftd::eft

