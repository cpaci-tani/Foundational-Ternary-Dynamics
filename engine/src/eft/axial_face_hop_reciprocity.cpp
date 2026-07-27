#include "ftd/eft/axial_face_hop_reciprocity.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

namespace ftd::eft {
namespace {

double component(const Vec3& value, int axis) {
  return axis == 0 ? value.x : axis == 1 ? value.y : value.z;
}

Vec3 with_component(Vec3 value, int axis, double entry) {
  if (axis == 0) value.x = entry;
  else if (axis == 1) value.y = entry;
  else value.z = entry;
  return value;
}

int coord_component(const Coord& value, int axis) {
  return axis == 0 ? value.x : axis == 1 ? value.y : value.z;
}

Coord with_coord_component(Coord value, int axis, int entry) {
  if (axis == 0) value.x = entry;
  else if (axis == 1) value.y = entry;
  else value.z = entry;
  return value;
}

int wrap(int value, int L) {
  const int remainder = value % L;
  return remainder < 0 ? remainder + L : remainder;
}

double norm(const Vec3& value) {
  return std::sqrt(value.x * value.x + value.y * value.y
                   + value.z * value.z);
}

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double matter_energy(double momentum,
                     double rest_energy,
                     double causal_speed) {
  return std::sqrt(rest_energy * rest_energy
      + causal_speed * causal_speed * momentum * momentum);
}

double axial_displacement(double momentum_before,
                          double momentum_after,
                          double dt,
                          double rest_energy,
                          double causal_speed) {
  const double denominator = matter_energy(
      momentum_before, rest_energy, causal_speed)
      + matter_energy(momentum_after, rest_energy, causal_speed);
  return dt * causal_speed * causal_speed
      * (momentum_before + momentum_after) / denominator;
}

bool valid_field(const MatchedFaceFlux& field) {
  const std::size_t side = field.L > 0
      ? static_cast<std::size_t>(field.L) : 0;
  const std::size_t count = side * side * side;
  return field.L >= 3 && field.x.size() == count
      && field.y.size() == count && field.z.size() == count;
}

const std::vector<double>& axis_field(
    const MatchedFaceFlux& field, int axis) {
  return axis == 0 ? field.x : axis == 1 ? field.y : field.z;
}

double uniform_axis_residual(const MatchedFaceFlux& field, int axis) {
  const auto& values = axis_field(field, axis);
  if (values.empty()) return INFINITY;
  const double reference = values.front();
  double residual = 0.0;
  for (double value : values) {
    residual = std::max(residual, std::abs(value - reference));
  }
  return residual;
}

double face_value(const MatchedFaceFlux& field,
                  Coord site,
                  int axis,
                  int face_coordinate) {
  Coord face = site;
  face = with_coord_component(face, axis, face_coordinate);
  const int index = field.index(face.x, face.y, face.z);
  return axis_field(field, axis)[static_cast<std::size_t>(index)];
}

double stationary_trace(const MatchedFaceFlux& field,
                        Coord site,
                        const Vec3& remainder,
                        int axis) {
  const double r = component(remainder, axis);
  const int n = coord_component(site, axis);
  if (r > 0.0) return face_value(field, site, axis, n);
  if (r < 0.0) return face_value(field, site, axis, n - 1);
  return 0.5 * (face_value(field, site, axis, n)
                + face_value(field, site, axis, n - 1));
}

struct Endpoint {
  Coord site{};
  Vec3 remainder{};
  int hop_direction = 0;
};

Endpoint threshold_endpoint(Coord site,
                            const Vec3& remainder,
                            int axis,
                            double displacement,
                            int L) {
  Endpoint result{site, remainder, 0};
  double raw = component(remainder, axis) + displacement;
  int anchor = coord_component(site, axis);
  if (raw >= 1.0) {
    ++anchor;
    raw -= 1.0;
    result.hop_direction = +1;
  } else if (raw <= -1.0) {
    --anchor;
    raw += 1.0;
    result.hop_direction = -1;
  }
  result.site = with_coord_component(result.site, axis, wrap(anchor, L));
  result.remainder = with_component(result.remainder, axis, raw);
  return result;
}

MatchedFaceFlux as_face_flux(const FaceCurrentSegment& current) {
  MatchedFaceFlux result(current.L);
  result.x = current.current_x;
  result.y = current.current_y;
  result.z = current.current_z;
  return result;
}

void add_scaled(MatchedFaceFlux& target,
                const MatchedFaceFlux& value,
                double scale) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale * value.x[i];
    target.y[i] += scale * value.y[i];
    target.z[i] += scale * value.z[i];
  }
}

long double dot(const MatchedFaceFlux& lhs,
                const MatchedFaceFlux& rhs) {
  if (lhs.L != rhs.L || lhs.x.size() != rhs.x.size()) return NAN;
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.x.size(); ++i) {
    result += static_cast<long double>(lhs.x[i]) * rhs.x[i]
        + static_cast<long double>(lhs.y[i]) * rhs.y[i]
        + static_cast<long double>(lhs.z[i]) * rhs.z[i];
  }
  return result;
}

double energy_change(const MatchedFaceFlux& before,
                     const MatchedFaceFlux& after) {
  if (before.L != after.L || before.x.size() != after.x.size()) return NAN;
  long double result = 0.0L;
  for (std::size_t i = 0; i < before.x.size(); ++i) {
    result += 0.5L * (static_cast<long double>(after.x[i]) * after.x[i]
                      - static_cast<long double>(before.x[i]) * before.x[i]);
    result += 0.5L * (static_cast<long double>(after.y[i]) * after.y[i]
                      - static_cast<long double>(before.y[i]) * before.y[i]);
    result += 0.5L * (static_cast<long double>(after.z[i]) * after.z[i]
                      - static_cast<long double>(before.z[i]) * before.z[i]);
  }
  return static_cast<double>(result);
}

double max_difference(const MatchedFaceFlux& lhs,
                      const MatchedFaceFlux& rhs) {
  if (lhs.L != rhs.L || lhs.x.size() != rhs.x.size()) return INFINITY;
  double residual = 0.0;
  for (std::size_t i = 0; i < lhs.x.size(); ++i) {
    residual = std::max({residual,
        std::abs(lhs.x[i] - rhs.x[i]),
        std::abs(lhs.y[i] - rhs.y[i]),
        std::abs(lhs.z[i] - rhs.z[i])});
  }
  return residual;
}

double max_difference(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double residual = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    residual = std::max(residual, std::abs(lhs[i] - rhs[i]));
  }
  return residual;
}

double path_average(const FaceCurrentSegment& segment,
                    const MatchedFaceFlux& midpoint,
                    Coord start_site,
                    const Vec3& start_remainder,
                    int axis,
                    int charge,
                    double displacement) {
  const double tiny = 64.0 * std::numeric_limits<double>::epsilon();
  if (std::abs(displacement) <= tiny) {
    return stationary_trace(midpoint, start_site, start_remainder, axis);
  }
  const MatchedFaceFlux current = as_face_flux(segment);
  return static_cast<double>(dot(current, midpoint))
      / (static_cast<double>(charge) * displacement);
}

struct Trial {
  bool valid = false;
  double momentum_after = 0.0;
  double displacement = 0.0;
  double path_field = 0.0;
  Endpoint endpoint{};
  FaceCurrentSegment current;
  MatchedFaceFlux midpoint;
};

Trial evaluate_trial(const AxialFaceHopInput& input,
                     double momentum_after) {
  Trial result;
  result.momentum_after = momentum_after;
  const double p0 = component(input.momentum_before, input.axis);
  result.displacement = axial_displacement(
      p0, momentum_after, input.dt,
      input.rest_energy, input.causal_speed);
  result.endpoint = threshold_endpoint(
      input.site, input.remainder, input.axis,
      result.displacement, input.electric_before.L);
  result.current = make_face_current_segment(
      input.electric_before.L,
      input.site, input.remainder,
      result.endpoint.site, result.endpoint.remainder,
      input.charge);
  if (!result.current.valid) return result;
  const MatchedFaceFlux current = as_face_flux(result.current);
  result.midpoint = input.electric_before;
  add_scaled(result.midpoint, current, -0.5 * input.coupling);
  result.path_field = path_average(
      result.current, result.midpoint,
      input.site, input.remainder,
      input.axis, input.charge, result.displacement);
  result.valid = std::isfinite(result.path_field);
  return result;
}

int periodic_coord_mismatch(Coord lhs, Coord rhs, int L) {
  return (wrap(lhs.x, L) != wrap(rhs.x, L) ? 1 : 0)
      + (wrap(lhs.y, L) != wrap(rhs.y, L) ? 1 : 0)
      + (wrap(lhs.z, L) != wrap(rhs.z, L) ? 1 : 0);
}

double effective_position_residual(const FaceCurrentSegment& reverse,
                                   const FaceCurrentSegment& forward) {
  const Vec3 delta = reverse.end_effective_position
      - forward.start_effective_position;
  return max_abs(delta);
}

}  // namespace

double axial_face_hop_contraction_bound(
    double coupling,
    double dt,
    double rest_energy,
    double causal_speed) {
  if (!std::isfinite(coupling) || !std::isfinite(dt)
      || !std::isfinite(rest_energy) || rest_energy <= 0.0
      || !std::isfinite(causal_speed) || causal_speed <= 0.0) {
    return INFINITY;
  }
  return coupling * coupling * dt * dt
      * causal_speed * causal_speed / (2.0 * rest_energy);
}

AxialFaceHopStep solve_axial_face_hop_step(
    const AxialFaceHopInput& input) {
  AxialFaceHopStep result;
  result.axis = input.axis;
  result.charge = input.charge;
  result.site_before = input.site;
  result.remainder_before = input.remainder;
  result.momentum_before = input.momentum_before;
  result.electric_before = input.electric_before;
  result.dressing_before = input.dressing_before;
  result.dressing_after = input.dressing_before;
  result.coupling = input.coupling;
  result.dt = input.dt;
  result.rest_energy = input.rest_energy;
  result.causal_speed = input.causal_speed;
  result.contraction_bound = axial_face_hop_contraction_bound(
      input.coupling, input.dt,
      input.rest_energy, input.causal_speed);

  if (!valid_field(input.electric_before)
      || input.axis < 0 || input.axis > 2
      || (input.charge != -1 && input.charge != 1)
      || !std::isfinite(input.coupling)
      || !std::isfinite(input.dt) || input.dt == 0.0
      || input.rest_energy <= 0.0 || input.causal_speed <= 0.0
      || input.causal_speed * std::abs(input.dt) >= 1.0
      || input.max_iterations <= 0
      || input.solver_tolerance <= 0.0
      || max_abs(input.remainder) >= 1.0
      || std::abs(component(input.remainder, (input.axis + 1) % 3)) > 1e-15
      || std::abs(component(input.remainder, (input.axis + 2) % 3)) > 1e-15
      || std::abs(component(input.momentum_before,
                            (input.axis + 1) % 3)) > 1e-15
      || std::abs(component(input.momentum_before,
                            (input.axis + 2) % 3)) > 1e-15) {
    return result;
  }

  result.uniform_field_residual = uniform_axis_residual(
      input.electric_before, input.axis);
  result.uniqueness_certified = result.uniform_field_residual <= 1e-14
      && result.contraction_bound < 1.0;

  const double p0 = component(input.momentum_before, input.axis);
  double momentum = input.use_initial_guess
      ? input.initial_momentum_guess
      : p0 + input.dt * input.coupling
          * static_cast<double>(input.charge)
          * stationary_trace(input.electric_before, input.site,
                             input.remainder, input.axis);
  Trial trial;
  for (int iteration = 1; iteration <= input.max_iterations; ++iteration) {
    trial = evaluate_trial(input, momentum);
    if (!trial.valid) return result;
    const double next = p0 + input.dt * input.coupling
        * static_cast<double>(input.charge) * trial.path_field;
    result.iterations = iteration;
    if (std::abs(next - momentum) <= input.solver_tolerance) {
      momentum = next;
      result.converged = true;
      break;
    }
    momentum = next;
  }
  trial = evaluate_trial(input, momentum);
  if (!trial.valid) return result;

  result.momentum_after = with_component(
      input.momentum_before, input.axis, momentum);
  result.displacement = with_component(
      {}, input.axis, trial.displacement);
  result.site_after = trial.endpoint.site;
  result.remainder_after = trial.endpoint.remainder;
  result.hop_direction = trial.endpoint.hop_direction;
  result.hopped = result.hop_direction != 0;
  result.current = trial.current;
  result.electric_midpoint = trial.midpoint;
  result.path_averaged_field = trial.path_field;
  result.electric_after = input.electric_before;
  const MatchedFaceFlux current = as_face_flux(result.current);
  add_scaled(result.electric_after, current, -input.coupling);

  const double mapped = p0 + input.dt * input.coupling
      * static_cast<double>(input.charge) * trial.path_field;
  result.fixed_point_residual = std::abs(momentum - mapped);
  result.impulse_residual = std::abs(
      (momentum - p0) - input.dt * input.coupling
          * static_cast<double>(input.charge) * trial.path_field);
  result.displacement_residual = std::abs(
      trial.displacement - axial_displacement(
          p0, momentum, input.dt,
          input.rest_energy, input.causal_speed));

  result.field_work = input.coupling
      * static_cast<double>(dot(current, result.electric_midpoint));
  const double matter_change = matter_energy(
      momentum, input.rest_energy, input.causal_speed)
      - matter_energy(p0, input.rest_energy, input.causal_speed);
  result.matter_work_residual = std::abs(
      matter_change - result.field_work);
  const double field_change = energy_change(
      input.electric_before, result.electric_after);
  result.total_energy_residual = std::abs(
      field_change + matter_change);
  result.continuity_residual = result.current.continuity_residual;
  result.locality_residual = result.current.locality_residual;
  result.relative_gauss_residual = 0.0;
  for (int x = 0; x < input.electric_before.L; ++x) {
    for (int y = 0; y < input.electric_before.L; ++y) {
      for (int z = 0; z < input.electric_before.L; ++z) {
        const double residual = divergence_at(
            result.electric_after, x, y, z)
            - divergence_at(input.electric_before, x, y, z)
            + input.coupling * face_current_divergence_at(
                result.current, x, y, z);
        result.relative_gauss_residual = std::max(
            result.relative_gauss_residual, std::abs(residual));
      }
    }
  }
  result.speed = norm(result.displacement) / std::abs(input.dt);
  result.causal_excess = std::max(
      0.0, result.speed - input.causal_speed);

  const Endpoint inverse_endpoint = threshold_endpoint(
      result.site_after, result.remainder_after, input.axis,
      -trial.displacement, input.electric_before.L);
  result.inverse_site = inverse_endpoint.site;
  result.inverse_remainder = inverse_endpoint.remainder;
  result.inverse_current = make_face_current_segment(
      input.electric_before.L,
      result.site_after, result.remainder_after,
      inverse_endpoint.site, inverse_endpoint.remainder,
      input.charge);
  if (!result.inverse_current.valid) return result;
  const MatchedFaceFlux reverse_current = as_face_flux(
      result.inverse_current);
  MatchedFaceFlux restored_field = result.electric_after;
  add_scaled(restored_field, reverse_current, -input.coupling);
  const double reverse_path_field = path_average(
      result.inverse_current, result.electric_midpoint,
      result.site_after, result.remainder_after,
      input.axis, input.charge, -trial.displacement);
  const double inverse_momentum = momentum - input.dt * input.coupling
      * static_cast<double>(input.charge) * reverse_path_field;

  result.physical_inverse_residual = effective_position_residual(
      result.inverse_current, result.current);
  result.shape_inverse_residual = max_difference(
      result.inverse_current.rho_after, result.current.rho_before);
  result.field_inverse_residual = max_difference(
      restored_field, input.electric_before);
  result.momentum_inverse_residual = std::abs(inverse_momentum - p0);
  result.raw_anchor_inverse_mismatch = periodic_coord_mismatch(
      result.inverse_site, input.site, input.electric_before.L);
  result.raw_remainder_inverse_residual = max_abs(
      result.inverse_remainder - input.remainder);
  result.strict_discrete_inverse =
      result.raw_anchor_inverse_mismatch == 0
      && result.raw_remainder_inverse_residual <= 1e-12;

  if (result.hopped) {
    const int h = result.hop_direction;
    Coord alternate_site = input.site;
    alternate_site = with_coord_component(
        alternate_site, input.axis,
        wrap(coord_component(alternate_site, input.axis) + h,
             input.electric_before.L));
    Vec3 alternate_remainder = with_component(
        input.remainder, input.axis,
        component(input.remainder, input.axis) - h);
    const Endpoint alternate_endpoint = threshold_endpoint(
        alternate_site, alternate_remainder, input.axis,
        trial.displacement, input.electric_before.L);
    const auto original_shape = make_subcell_polarity_shape(
        input.site, input.remainder, input.charge);
    const auto alternate_shape = make_subcell_polarity_shape(
        alternate_site, alternate_remainder, input.charge);
    const auto original_deposit = make_face_current_segment(
        input.electric_before.L,
        input.site, input.remainder,
        input.site, input.remainder,
        input.charge);
    const auto alternate_deposit = make_face_current_segment(
        input.electric_before.L,
        alternate_site, alternate_remainder,
        alternate_site, alternate_remainder,
        input.charge);
    result.preimage_shape_residual = (original_shape.valid
        && alternate_shape.valid && original_deposit.valid
        && alternate_deposit.valid)
        ? max_difference(original_deposit.rho_before,
                         alternate_deposit.rho_before)
        : INFINITY;
    result.preimage_output_residual = std::max(
        static_cast<double>(periodic_coord_mismatch(
            alternate_endpoint.site, result.site_after,
            input.electric_before.L)),
        max_abs(alternate_endpoint.remainder - result.remainder_after));
    result.preimage_collision = result.preimage_shape_residual <= 1e-12
        && result.preimage_output_residual <= 1e-12;
  }

  result.transaction_valid = result.converged
      && result.uniqueness_certified
      && result.fixed_point_residual <= 1e-12
      && result.impulse_residual <= 1e-12
      && result.displacement_residual <= 1e-12
      && result.matter_work_residual <= 1e-12
      && result.total_energy_residual <= 1e-12
      && result.continuity_residual <= 1e-12
      && result.relative_gauss_residual <= 1e-12
      && result.locality_residual <= 1e-12
      && result.causal_excess <= 1e-12
      && result.physical_inverse_residual <= 1e-10
      && result.shape_inverse_residual <= 1e-10
      && result.field_inverse_residual <= 1e-10
      && result.momentum_inverse_residual <= 1e-10;
  return result;
}

}  // namespace ftd::eft
