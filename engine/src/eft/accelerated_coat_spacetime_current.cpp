#include "ftd/eft/accelerated_coat_spacetime_current.h"

#include "ftd/eft/local_polarity_regularity.h"
#include "ftd/eft/matched_face_energy_transaction.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>

namespace ftd::eft {
namespace {

double component(const Vec3& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
}

void set_component(Vec3& value, int axis, double component_value) {
  if (axis == 0) value.x = component_value;
  if (axis == 1) value.y = component_value;
  if (axis == 2) value.z = component_value;
}

double b1(double u) {
  return evaluate_local_polarity_kernel(LocalPolarityKernel::Hat, u);
}

double b2(double u) {
  return evaluate_local_polarity_kernel(
      LocalPolarityKernel::QuadraticBSpline, u);
}

long double energy(long double momentum,
                   long double rest_energy,
                   long double c_speed) {
  return std::sqrt(rest_energy * rest_energy
      + c_speed * c_speed * momentum * momentum);
}

long double velocity(long double momentum,
                     long double rest_energy,
                     long double c_speed) {
  return c_speed * c_speed * momentum
      / energy(momentum, rest_energy, c_speed);
}

struct PathSample {
  Vec3 position{};
  Vec3 derivative{};
};

PathSample sample_path(double tau,
                       const Vec3& start,
                       const Vec3& direction,
                       double rest_energy,
                       double c_speed,
                       double temporal_scale,
                       double midpoint_momentum,
                       double half_impulse) {
  const long double p = midpoint_momentum;
  const long double a = half_impulse;
  const long double M = rest_energy;
  const long double c = c_speed;
  const long double h = temporal_scale;
  long double distance = 0.0L;
  long double derivative = 0.0L;
  const long double scale = std::max(1.0L, std::abs(p));
  if (std::abs(a)
      <= 64.0L * std::numeric_limits<long double>::epsilon() * scale) {
    derivative = h * velocity(p, M, c);
    distance = tau * derivative;
  } else {
    const long double pt = p - a + 2.0L * a * tau;
    distance = h * (energy(pt, M, c)-energy(p-a, M, c))
        / (2.0L * a);
    derivative = h * velocity(pt, M, c);
  }
  return {start + direction * static_cast<double>(distance),
          direction * static_cast<double>(derivative)};
}

std::vector<double> temporal_breaks(
    const Vec3& start,
    const Vec3& end,
    const Vec3& direction,
    double rest_energy,
    double c_speed,
    double temporal_scale,
    double midpoint_momentum,
    double half_impulse) {
  std::vector<double> breaks{0.0, 1.0};
  for (int axis = 0; axis < 3; ++axis) {
    const double begin = component(start, axis);
    const double finish = component(end, axis);
    if (finish == begin) continue;
    const double lower = std::min(begin, finish);
    const double upper = std::max(begin, finish);
    const int first = static_cast<int>(std::floor(lower)) - 2;
    const int last = static_cast<int>(std::ceil(upper)) + 2;
    for (int k = first; k <= last; ++k) {
      const double plane = k + 0.5;
      if (!(plane > lower && plane < upper)) continue;
      double lo = 0.0;
      double hi = 1.0;
      const bool increasing = finish > begin;
      for (int iteration = 0; iteration < 80; ++iteration) {
        const double mid = 0.5 * (lo + hi);
        const double value = component(sample_path(
            mid, start, direction, rest_energy, c_speed,
            temporal_scale, midpoint_momentum, half_impulse).position, axis);
        if ((increasing && value < plane)
            || (!increasing && value > plane)) lo = mid;
        else hi = mid;
      }
      breaks.push_back(0.5 * (lo + hi));
    }
  }
  std::sort(breaks.begin(), breaks.end());
  breaks.erase(std::unique(breaks.begin(), breaks.end(),
      [](double lhs, double rhs) {
        return std::abs(lhs-rhs) <= 64.0
            * std::numeric_limits<double>::epsilon();
      }), breaks.end());
  return breaks;
}

double max_difference(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, std::abs(lhs[i]-rhs[i]));
  return result;
}

long double dot(const std::vector<double>& lhs,
                const std::vector<double>& rhs) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result += static_cast<long double>(lhs[i])*rhs[i];
  return result;
}

long double dot(const MatchedFaceFlux& lhs,
                const MatchedFaceFlux& rhs) {
  return dot(lhs.x, rhs.x)+dot(lhs.y, rhs.y)+dot(lhs.z, rhs.z);
}

bool finite(const std::vector<double>& values) {
  return std::all_of(values.begin(), values.end(),
      [](double value) { return std::isfinite(value); });
}

}  // namespace

AcceleratedCoatSpacetimeCurrent::AcceleratedCoatSpacetimeCurrent(int size)
    : L(size), spatial_quadrature(size), spatial_start(size),
      spatial_end(size) {
  const std::size_t count = size > 0
      ? static_cast<std::size_t>(size)*size*size : 0;
  temporal_charge.assign(count, 0.0);
}

int AcceleratedCoatSpacetimeCurrent::index(int x, int y, int z) const {
  return spatial_start.index(x, y, z);
}

AcceleratedCoatSpacetimeCurrent make_accelerated_coat_spacetime_current(
    int L,
    const Vec3& start_position,
    double rest_energy,
    double c_speed,
    double temporal_scale,
    double midpoint_momentum,
    double half_impulse,
    const Vec3& raw_direction,
    int charge) {
  AcceleratedCoatSpacetimeCurrent result(L);
  result.charge = charge;
  result.temporal_scale = temporal_scale;
  result.start_position = start_position;
  result.trajectory = evaluate_accelerated_worldline_energy(
      rest_energy, c_speed, temporal_scale, midpoint_momentum,
      half_impulse, raw_direction);
  if (L < 5 || (charge != -1 && charge != 1)
      || !result.trajectory.valid) return result;
  const long double momentum_start =
      static_cast<long double>(midpoint_momentum)-half_impulse;
  const long double momentum_end =
      static_cast<long double>(midpoint_momentum)+half_impulse;
  if (momentum_start*momentum_end < 0.0L) return result;
  const Vec3 direction = result.trajectory.direction;
  result.end_position = start_position
      + direction*result.trajectory.exact_displacement;
  result.spatial = make_quadratic_coat_face_current(
      L, start_position, result.end_position, charge);
  if (!result.spatial.valid) return result;

  constexpr std::array<long double, 16> nodes{{
      -0.989400934991649932596154173450L,
      -0.944575023073232576077988415535L,
      -0.865631202387831743880467897712L,
      -0.755404408355003033895101194847L,
      -0.617876244402643748446671764049L,
      -0.458016777657227386342419442984L,
      -0.281603550779258913230460501460L,
      -0.095012509837637440185319335425L,
       0.095012509837637440185319335425L,
       0.281603550779258913230460501460L,
       0.458016777657227386342419442984L,
       0.617876244402643748446671764049L,
       0.755404408355003033895101194847L,
       0.865631202387831743880467897712L,
       0.944575023073232576077988415535L,
       0.989400934991649932596154173450L}};
  constexpr std::array<long double, 16> weights{{
      0.027152459411754094851780572456L,
      0.062253523938647892862843836994L,
      0.095158511682492784809925107602L,
      0.124628971255533872052476282192L,
      0.149595988816576732081501730547L,
      0.169156519395002538189312079030L,
      0.182603415044923588866763667969L,
      0.189450610455068496285396723208L,
      0.189450610455068496285396723208L,
      0.182603415044923588866763667969L,
      0.169156519395002538189312079030L,
      0.149595988816576732081501730547L,
      0.124628971255533872052476282192L,
      0.095158511682492784809925107602L,
      0.062253523938647892862843836994L,
      0.027152459411754094851780572456L}};
  const auto breaks = temporal_breaks(
      start_position, result.end_position, direction,
      rest_energy, c_speed, temporal_scale,
      midpoint_momentum, half_impulse);
  result.quadrature_pieces = static_cast<int>(breaks.size())-1;
  const std::size_t count = result.temporal_charge.size();
  std::vector<long double> total_x(count, 0.0L);
  std::vector<long double> total_y(count, 0.0L);
  std::vector<long double> total_z(count, 0.0L);
  std::vector<long double> start_x(count, 0.0L);
  std::vector<long double> start_y(count, 0.0L);
  std::vector<long double> start_z(count, 0.0L);
  std::vector<long double> end_x(count, 0.0L);
  std::vector<long double> end_y(count, 0.0L);
  std::vector<long double> end_z(count, 0.0L);
  std::vector<long double> temporal(count, 0.0L);

  for (std::size_t piece = 1; piece < breaks.size(); ++piece) {
    const long double ta = breaks[piece-1];
    const long double tb = breaks[piece];
    const long double midpoint = 0.5L*(ta+tb);
    const long double half_width = 0.5L*(tb-ta);
    for (std::size_t sample = 0; sample < nodes.size(); ++sample) {
      const double tau = static_cast<double>(
          midpoint+half_width*nodes[sample]);
      const long double quadrature_weight = half_width*weights[sample];
      const PathSample path = sample_path(
          tau, start_position, direction, rest_energy, c_speed,
          temporal_scale, midpoint_momentum, half_impulse);
      const int lower[3]{
          static_cast<int>(std::floor(path.position.x))-2,
          static_cast<int>(std::floor(path.position.y))-2,
          static_cast<int>(std::floor(path.position.z))-2};
      for (int x = lower[0]; x <= lower[0]+4; ++x) {
        for (int y = lower[1]; y <= lower[1]+4; ++y) {
          for (int z = lower[2]; z <= lower[2]+4; ++z) {
            const int coordinate[3]{x,y,z};
            const auto site_index = static_cast<std::size_t>(
                result.index(x,y,z));
            double site_basis = 1.0;
            for (int axis = 0; axis < 3; ++axis) {
              site_basis *= b2(component(path.position,axis)
                  -coordinate[axis]);
            }
            temporal[site_index] += quadrature_weight*charge*site_basis;
            for (int face_axis = 0; face_axis < 3; ++face_axis) {
              double face_basis = 1.0;
              for (int axis = 0; axis < 3; ++axis) {
                const double center = coordinate[axis]
                    +(axis == face_axis ? 0.5 : 0.0);
                const double u = component(path.position,axis)-center;
                face_basis *= axis == face_axis ? b1(u) : b2(u);
              }
              const long double value = quadrature_weight*charge
                  *face_basis*component(path.derivative,face_axis);
              std::vector<long double>* total = face_axis == 0 ? &total_x
                  : (face_axis == 1 ? &total_y : &total_z);
              std::vector<long double>* start = face_axis == 0 ? &start_x
                  : (face_axis == 1 ? &start_y : &start_z);
              std::vector<long double>* end = face_axis == 0 ? &end_x
                  : (face_axis == 1 ? &end_y : &end_z);
              (*total)[site_index] += value;
              (*start)[site_index] += (1.0-tau)*value;
              (*end)[site_index] += tau*value;
            }
          }
        }
      }
    }
  }
  for (std::size_t i = 0; i < count; ++i) {
    result.spatial_quadrature.x[i] = static_cast<double>(total_x[i]);
    result.spatial_quadrature.y[i] = static_cast<double>(total_y[i]);
    result.spatial_quadrature.z[i] = static_cast<double>(total_z[i]);
    result.spatial_start.x[i] = static_cast<double>(start_x[i]);
    result.spatial_start.y[i] = static_cast<double>(start_y[i]);
    result.spatial_start.z[i] = static_cast<double>(start_z[i]);
    result.spatial_end.x[i] = static_cast<double>(end_x[i]);
    result.spatial_end.y[i] = static_cast<double>(end_y[i]);
    result.spatial_end.z[i] = static_cast<double>(end_z[i]);
    result.temporal_charge[i] = static_cast<double>(temporal[i]);
  }
  MatchedFaceFlux exact_spatial(L);
  exact_spatial.x = result.spatial.current_x;
  exact_spatial.y = result.spatial.current_y;
  exact_spatial.z = result.spatial.current_z;
  result.total_current_residual = matched_face_max_difference(
      result.spatial_quadrature, exact_spatial);
  MatchedFaceFlux recombined = result.spatial_start;
  for (std::size_t i = 0; i < count; ++i) {
    recombined.x[i] += result.spatial_end.x[i];
    recombined.y[i] += result.spatial_end.y[i];
    recombined.z[i] += result.spatial_end.z[i];
  }
  result.split_recombination_residual = matched_face_max_difference(
      recombined, result.spatial_quadrature);
  result.temporal_partition_residual = std::abs(
      static_cast<double>(dot(result.temporal_charge,
                              std::vector<double>(count,1.0)))
      -charge);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto i = static_cast<std::size_t>(result.index(x,y,z));
        result.split_continuity_start_residual = std::max(
            result.split_continuity_start_residual,
            std::abs(divergence_at(result.spatial_start,x,y,z)
                +result.temporal_charge[i]-result.spatial.rho_before[i]));
        result.split_continuity_end_residual = std::max(
            result.split_continuity_end_residual,
            std::abs(divergence_at(result.spatial_end,x,y,z)
                -result.temporal_charge[i]+result.spatial.rho_after[i]));
      }
    }
  }
  const auto linear = make_quadratic_coat_spacetime_current(
      L,start_position,result.end_position,charge,temporal_scale);
  if (!linear.valid) return result;
  result.linear_start_difference = matched_face_max_difference(
      result.spatial_start,linear.spatial_start);
  result.linear_end_difference = matched_face_max_difference(
      result.spatial_end,linear.spatial_end);
  result.linear_temporal_difference = max_difference(
      result.temporal_charge,linear.temporal_charge);
  result.valid = finite(result.temporal_charge)
      && result.total_current_residual <= 5e-12
      && result.split_recombination_residual <= 5e-12
      && result.temporal_partition_residual <= 5e-12
      && result.split_continuity_start_residual <= 5e-12
      && result.split_continuity_end_residual <= 5e-12;
  return result;
}

double accelerated_coat_gauge_endpoint_residual(
    const AcceleratedCoatSpacetimeCurrent& current,
    const std::vector<double>& chi_start,
    const std::vector<double>& chi_end) {
  const std::size_t count = current.temporal_charge.size();
  if (!current.valid || chi_start.size() != count
      || chi_end.size() != count || !finite(chi_start)
      || !finite(chi_end)) return INFINITY;
  const auto gradient_start = matched_forward_gradient(
      current.L,chi_start);
  const auto gradient_end = matched_forward_gradient(
      current.L,chi_end);
  const long double action_shift = dot(
      gradient_start,current.spatial_start)
      +dot(gradient_end,current.spatial_end)
      +dot(chi_end,current.temporal_charge)
      -dot(chi_start,current.temporal_charge);
  const long double endpoint_shift = dot(
      current.spatial.rho_after,chi_end)
      -dot(current.spatial.rho_before,chi_start);
  return std::abs(static_cast<double>(action_shift-endpoint_shift));
}

}  // namespace ftd::eft
