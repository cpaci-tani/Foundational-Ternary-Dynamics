#include "ftd/eft/quadratic_coat_matter_work.h"

#include "ftd/eft/local_polarity_regularity.h"

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

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

bool finite(const std::vector<double>& values) {
  return std::all_of(values.begin(), values.end(),
      [](double value) { return std::isfinite(value); });
}

bool finite(const MatchedFaceFlux& field) {
  return finite(field.x) && finite(field.y) && finite(field.z);
}

bool valid_slab(const DualGaugePotentialSlab& slab) {
  const auto count = slab.L > 0
      ? static_cast<std::size_t>(slab.L) * slab.L * slab.L : 0;
  return slab.L >= 5 && slab.temporal_scale > 0.0
      && std::isfinite(slab.temporal_scale)
      && slab.A_start.L == slab.L && slab.A_end.L == slab.L
      && slab.A_start.x.size() == count
      && slab.A_start.y.size() == count
      && slab.A_start.z.size() == count
      && slab.A_end.x.size() == count
      && slab.A_end.y.size() == count
      && slab.A_end.z.size() == count
      && slab.Phi.size() == count
      && finite(slab.A_start) && finite(slab.A_end)
      && finite(slab.Phi);
}

double b1(double u) {
  return evaluate_local_polarity_kernel(LocalPolarityKernel::Hat, u);
}

double db1(double u, bool& smooth) {
  constexpr double knot_tolerance = 64.0
      * std::numeric_limits<double>::epsilon();
  const double absolute = std::abs(u);
  if (absolute <= knot_tolerance
      || std::abs(absolute - 1.0) <= knot_tolerance) {
    smooth = false;
  }
  if (absolute >= 1.0) return 0.0;
  return u < 0.0 ? 1.0 : -1.0;
}

double b2(double u) {
  return evaluate_local_polarity_kernel(
      LocalPolarityKernel::QuadraticBSpline, u);
}

double db2(double u) {
  const double absolute = std::abs(u);
  if (absolute <= 0.5) return -2.0 * u;
  if (absolute >= 1.5) return 0.0;
  return -(1.5 - absolute) * (u < 0.0 ? -1.0 : 1.0);
}

struct FaceSample {
  Vec3 value{};
  std::array<std::array<double, 3>, 3> jacobian{};
  bool smooth = true;
};

FaceSample sample_connection(const MatchedFaceFlux& potential,
                             const Vec3& point) {
  FaceSample result;
  const int lower[3] = {
      static_cast<int>(std::floor(point.x)) - 2,
      static_cast<int>(std::floor(point.y)) - 2,
      static_cast<int>(std::floor(point.z)) - 2};
  for (int face_axis = 0; face_axis < 3; ++face_axis) {
    const std::vector<double>& field = face_axis == 0 ? potential.x
        : (face_axis == 1 ? potential.y : potential.z);
    for (int x = lower[0]; x <= lower[0] + 4; ++x) {
      for (int y = lower[1]; y <= lower[1] + 4; ++y) {
        for (int z = lower[2]; z <= lower[2] + 4; ++z) {
          const int coordinate[3] = {x, y, z};
          double factor[3]{};
          double derivative[3]{};
          for (int axis = 0; axis < 3; ++axis) {
            const double center = coordinate[axis]
                + (axis == face_axis ? 0.5 : 0.0);
            const double u = component(point, axis) - center;
            if (axis == face_axis) {
              factor[axis] = b1(u);
              derivative[axis] = db1(u, result.smooth);
            } else {
              factor[axis] = b2(u);
              derivative[axis] = db2(u);
            }
          }
          const double basis = factor[0] * factor[1] * factor[2];
          bool derivative_support = false;
          for (int axis = 0; axis < 3; ++axis) {
            derivative_support = derivative_support
                || derivative[axis] != 0.0;
          }
          if (basis == 0.0 && !derivative_support) continue;
          const auto index = static_cast<std::size_t>(
              potential.index(x, y, z));
          const double coefficient = field[index];
          set_component(result.value, face_axis,
              component(result.value, face_axis) + coefficient * basis);
          for (int axis = 0; axis < 3; ++axis) {
            double derivative_basis = derivative[axis];
            for (int other = 0; other < 3; ++other) {
              if (other != axis) derivative_basis *= factor[other];
            }
            result.jacobian[static_cast<std::size_t>(face_axis)]
                           [static_cast<std::size_t>(axis)]
                += coefficient * derivative_basis;
          }
        }
      }
    }
  }
  return result;
}

struct ScalarSample {
  double value = 0.0;
  Vec3 gradient{};
};

ScalarSample sample_scalar(const DualGaugePotentialSlab& slab,
                           const Vec3& point) {
  ScalarSample result;
  const int lower[3] = {
      static_cast<int>(std::floor(point.x)) - 2,
      static_cast<int>(std::floor(point.y)) - 2,
      static_cast<int>(std::floor(point.z)) - 2};
  for (int x = lower[0]; x <= lower[0] + 4; ++x) {
    for (int y = lower[1]; y <= lower[1] + 4; ++y) {
      for (int z = lower[2]; z <= lower[2] + 4; ++z) {
        const int coordinate[3] = {x, y, z};
        double factor[3]{};
        double derivative[3]{};
        for (int axis = 0; axis < 3; ++axis) {
          const double u = component(point, axis) - coordinate[axis];
          factor[axis] = b2(u);
          derivative[axis] = db2(u);
        }
        const auto index = static_cast<std::size_t>(slab.index(x, y, z));
        const double coefficient = slab.Phi[index];
        result.value += coefficient * factor[0] * factor[1] * factor[2];
        for (int axis = 0; axis < 3; ++axis) {
          double derivative_basis = derivative[axis];
          for (int other = 0; other < 3; ++other) {
            if (other != axis) derivative_basis *= factor[other];
          }
          set_component(result.gradient, axis,
              component(result.gradient, axis)
              + coefficient * derivative_basis);
        }
      }
    }
  }
  return result;
}

std::vector<double> half_integer_breaks(const Vec3& start,
                                        const Vec3& end) {
  std::vector<double> breaks{0.0, 1.0};
  for (int axis = 0; axis < 3; ++axis) {
    const double p0 = component(start, axis);
    const double delta = component(end, axis) - p0;
    if (delta == 0.0) continue;
    const double lower = std::min(p0, p0 + delta);
    const double upper = std::max(p0, p0 + delta);
    const int first = static_cast<int>(std::floor(lower)) - 2;
    const int last = static_cast<int>(std::ceil(upper)) + 2;
    for (int k = first; k <= last; ++k) {
      const double plane = k + 0.5;
      const double t = (plane - p0) / delta;
      if (t > 0.0 && t < 1.0) breaks.push_back(t);
    }
  }
  std::sort(breaks.begin(), breaks.end());
  breaks.erase(std::unique(breaks.begin(), breaks.end(),
      [](double a, double b) {
        return std::abs(a - b)
            <= 32.0 * std::numeric_limits<double>::epsilon();
      }), breaks.end());
  return breaks;
}

struct DirectEndpointAction {
  double action = 0.0;
  Vec3 d1{};
  Vec3 d2{};
  bool smooth = true;
};

DirectEndpointAction direct_endpoint_action(
    const Vec3& start,
    const Vec3& end,
    int charge,
    double coupling,
    const DualGaugePotentialSlab& slab) {
  DirectEndpointAction result;
  constexpr std::array<long double, 4> nodes{{
      -0.86113631159405257522394648889281L,
      -0.33998104358485626480266575910324L,
       0.33998104358485626480266575910324L,
       0.86113631159405257522394648889281L}};
  constexpr std::array<long double, 4> weights{{
      0.34785484513745385737306394922199L,
      0.65214515486254614262693605077801L,
      0.65214515486254614262693605077801L,
      0.34785484513745385737306394922199L}};
  const Vec3 displacement = end - start;
  const auto breaks = half_integer_breaks(start, end);
  long double action = 0.0L;
  long double d1[3]{};
  long double d2[3]{};
  for (std::size_t piece = 1; piece < breaks.size(); ++piece) {
    const long double ta = breaks[piece - 1];
    const long double tb = breaks[piece];
    const long double midpoint = 0.5L * (ta + tb);
    const long double half_width = 0.5L * (tb - ta);
    for (std::size_t sample = 0; sample < nodes.size(); ++sample) {
      const double t = static_cast<double>(
          midpoint + half_width * nodes[sample]);
      const long double weight = half_width * weights[sample];
      const Vec3 point = start + displacement * t;
      const FaceSample a0 = sample_connection(slab.A_start, point);
      const FaceSample a1 = sample_connection(slab.A_end, point);
      const ScalarSample phi = sample_scalar(slab, point);
      result.smooth = result.smooth && a0.smooth && a1.smooth;
      Vec3 a{};
      for (int axis = 0; axis < 3; ++axis) {
        set_component(a, axis,
            (1.0 - t) * component(a0.value, axis)
            + t * component(a1.value, axis));
      }
      action += weight * (static_cast<long double>(a.dot(displacement))
          - slab.temporal_scale * phi.value);
      for (int endpoint_axis = 0; endpoint_axis < 3; ++endpoint_axis) {
        long double jacobian_dot = 0.0L;
        for (int field_axis = 0; field_axis < 3; ++field_axis) {
          const double jacobian = (1.0 - t)
              * a0.jacobian[static_cast<std::size_t>(field_axis)]
                           [static_cast<std::size_t>(endpoint_axis)]
              + t * a1.jacobian[static_cast<std::size_t>(field_axis)]
                              [static_cast<std::size_t>(endpoint_axis)];
          jacobian_dot += static_cast<long double>(jacobian)
              * component(displacement, field_axis);
        }
        const long double common = jacobian_dot
            - slab.temporal_scale
                * component(phi.gradient, endpoint_axis);
        d1[endpoint_axis] += weight * (
            -component(a, endpoint_axis) + (1.0 - t) * common);
        d2[endpoint_axis] += weight * (
             component(a, endpoint_axis) + t * common);
      }
    }
  }
  const long double scale = static_cast<long double>(charge) * coupling;
  result.action = static_cast<double>(scale * action);
  result.d1 = {static_cast<double>(scale * d1[0]),
               static_cast<double>(scale * d1[1]),
               static_cast<double>(scale * d1[2])};
  result.d2 = {static_cast<double>(scale * d2[0]),
               static_cast<double>(scale * d2[1]),
               static_cast<double>(scale * d2[2])};
  return result;
}

long double dot_current(const MatchedFaceFlux& electric,
                        const QuadraticCoatFaceCurrent& current) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < current.current_x.size(); ++i) {
    result += static_cast<long double>(electric.x[i]) * current.current_x[i]
        + static_cast<long double>(electric.y[i]) * current.current_y[i]
        + static_cast<long double>(electric.z[i]) * current.current_z[i];
  }
  return result;
}

double energy(const Vec3& momentum, double rest_energy, double c_speed) {
  return std::sqrt(rest_energy * rest_energy
      + c_speed * c_speed * momentum.mag2());
}

}  // namespace

QuadraticCoatMatterWorkResult evaluate_quadratic_coat_matter_work(
    const Vec3& start_position,
    const Vec3& end_position,
    int charge,
    double rest_energy,
    double c_speed,
    double beta,
    const DualGaugePotentialSlab& slab) {
  QuadraticCoatMatterWorkResult result;
  result.charge = charge;
  result.rest_energy = rest_energy;
  result.c_speed = c_speed;
  result.temporal_scale = slab.temporal_scale;
  result.beta = beta;
  result.start_position = start_position;
  result.end_position = end_position;
  result.displacement = end_position - start_position;
  if ((charge != -1 && charge != 1)
      || !finite(start_position) || !finite(end_position)
      || !(rest_energy > 0.0) || !std::isfinite(rest_energy)
      || !(c_speed > 0.0) || !std::isfinite(c_speed)
      || !(beta > 0.0) || !std::isfinite(beta)
      || !valid_slab(slab)) return result;
  result.coupling = beta / slab.temporal_scale;
  const double displacement_squared = result.displacement.mag2();
  const double h_squared = slab.temporal_scale * slab.temporal_scale;
  if (!(displacement_squared < h_squared)) return result;
  result.current = make_quadratic_coat_spacetime_current(
      slab.L, start_position, end_position, charge, slab.temporal_scale);
  if (!result.current.valid) return result;

  const DirectEndpointAction direct = direct_endpoint_action(
      result.current.spatial.start_effective_position,
      result.current.spatial.end_effective_position,
      charge, result.coupling, slab);
  result.derivative_smooth = direct.smooth;
  result.direct_interaction_action = direct.action;
  result.d1_interaction = direct.d1;
  result.d2_interaction = direct.d2;
  result.deposited_interaction_action = quadratic_coat_interaction_action(
      result.current, slab, result.coupling);
  result.deposited_action_residual = std::abs(
      result.direct_interaction_action
      - result.deposited_interaction_action);

  const double gamma_denominator = std::sqrt(
      1.0 - displacement_squared / h_squared);
  result.free_momentum = result.displacement * (
      rest_energy / (c_speed * slab.temporal_scale * gamma_denominator));
  result.matter_action = -rest_energy * slab.temporal_scale / c_speed
      * gamma_denominator;
  result.canonical_start = result.free_momentum - result.d1_interaction;
  result.canonical_end = result.free_momentum + result.d2_interaction;
  const FaceSample start_connection = sample_connection(
      slab.A_start, result.current.spatial.start_effective_position);
  const FaceSample end_connection = sample_connection(
      slab.A_end, result.current.spatial.end_effective_position);
  result.derivative_smooth = result.derivative_smooth
      && start_connection.smooth && end_connection.smooth;
  result.connection_start = start_connection.value;
  result.connection_end = end_connection.value;
  const double endpoint_coupling = charge * result.coupling;
  result.kinetic_start = result.canonical_start
      - result.connection_start * endpoint_coupling;
  result.kinetic_end = result.canonical_end
      - result.connection_end * endpoint_coupling;
  result.electric = slab_electric_field(slab);
  result.matter_energy_before = energy(
      result.kinetic_start, rest_energy, c_speed);
  result.matter_energy_after = energy(
      result.kinetic_end, rest_energy, c_speed);
  result.matter_energy_change = result.matter_energy_after
      - result.matter_energy_before;
  result.field_work = beta * static_cast<double>(
      dot_current(result.electric, result.current.spatial));
  result.matter_work_defect = result.matter_energy_change
      - result.field_work;
  result.valid = result.derivative_smooth
      && finite(result.d1_interaction) && finite(result.d2_interaction)
      && finite(result.free_momentum)
      && finite(result.canonical_start) && finite(result.canonical_end)
      && finite(result.kinetic_start) && finite(result.kinetic_end)
      && finite(result.electric)
      && std::isfinite(result.matter_action)
      && std::isfinite(result.direct_interaction_action)
      && std::isfinite(result.deposited_interaction_action)
      && std::isfinite(result.deposited_action_residual)
      && std::isfinite(result.matter_energy_change)
      && std::isfinite(result.field_work)
      && std::isfinite(result.matter_work_defect);
  return result;
}

}  // namespace ftd::eft
