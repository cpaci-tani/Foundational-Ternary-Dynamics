#include "ftd/eft/connected_reservoir_decomposition.h"

#include "ftd/eft/matched_face_energy_transaction.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace ftd::eft {
namespace {

bool finite(double value) { return std::isfinite(value); }

bool finite(const Vec3& value) {
  return finite(value.x) && finite(value.y) && finite(value.z);
}

Vec3 position(const MatchedMatterPoint& point) {
  return {static_cast<double>(point.anchor.x) + point.remainder.x,
          static_cast<double>(point.anchor.y) + point.remainder.y,
          static_cast<double>(point.anchor.z) + point.remainder.z};
}

double periodic_delta(double value, int L) {
  while (value > 0.5 * L) value -= L;
  while (value < -0.5 * L) value += L;
  return value;
}

double component(const Vec3& value, int axis) {
  return axis == 0 ? value.x : axis == 1 ? value.y : value.z;
}

bool same_graph(const ConnectedMooreBlockState& left,
                const ConnectedMooreBlockState& right) {
  if (left.charges != right.charges || left.edges.size() != right.edges.size()
      || left.width != right.width
      || left.orientation_axis != right.orientation_axis) {
    return false;
  }
  for (std::size_t index = 0; index < left.edges.size(); ++index) {
    const auto& a = left.edges[index];
    const auto& b = right.edges[index];
    if (a.first != b.first || a.second != b.second
        || a.reference_delta.x != b.reference_delta.x
        || a.reference_delta.y != b.reference_delta.y
        || a.reference_delta.z != b.reference_delta.z
        || a.rest_length_squared != b.rest_length_squared) {
      return false;
    }
  }
  return true;
}

bool finite_state(const ConnectedMooreBlockState& state) {
  const std::size_t field_size = static_cast<std::size_t>(state.electric.L)
      * state.electric.L * state.electric.L;
  if (state.electric.L < 5 || state.magnetic_half.L != state.electric.L
      || state.electric.x.size() != field_size
      || state.electric.y.size() != field_size
      || state.electric.z.size() != field_size
      || state.magnetic_half.x.size() != field_size
      || state.magnetic_half.y.size() != field_size
      || state.magnetic_half.z.size() != field_size) {
    return false;
  }
  const auto finite_values = [](const std::vector<double>& values) {
    return std::all_of(values.begin(), values.end(),
        [](double value) { return finite(value); });
  };
  if (!finite_values(state.electric.x) || !finite_values(state.electric.y)
      || !finite_values(state.electric.z)
      || !finite_values(state.magnetic_half.x)
      || !finite_values(state.magnetic_half.y)
      || !finite_values(state.magnetic_half.z)) {
    return false;
  }
  return std::all_of(state.constituents.begin(), state.constituents.end(),
      [](const MatchedMatterPoint& point) {
        return finite(point.remainder) && finite(point.momentum);
      });
}

double kinetic_energy_difference(const ConnectedMooreBlockState& control,
                                 const ConnectedMooreBlockState& excited,
                                 double mass_scale) {
  const long double rest = static_cast<long double>(mass_scale) * E_REST;
  const long double speed2 = static_cast<long double>(C_SPEED) * C_SPEED;
  long double result = 0.0L;
  for (std::size_t index = 0; index < control.constituents.size(); ++index) {
    const long double control_p2 = control.constituents[index].momentum.mag2();
    const long double excited_p2 = excited.constituents[index].momentum.mag2();
    const long double control_energy = std::sqrt(
        rest * rest + speed2 * control_p2);
    const long double excited_energy = std::sqrt(
        rest * rest + speed2 * excited_p2);
    result += speed2 * (excited_p2 - control_p2)
        / (excited_energy + control_energy);
  }
  return static_cast<double>(result);
}

double binding_energy_difference(const ConnectedMooreBlockState& control,
                                 const ConnectedMooreBlockState& excited,
                                 double stiffness) {
  long double result = 0.0L;
  for (const auto& edge : control.edges) {
    const Vec3 control_delta = position(control.constituents[edge.first])
        - position(control.constituents[edge.second]);
    const Vec3 excited_delta = position(excited.constituents[edge.first])
        - position(excited.constituents[edge.second]);
    const long double control_u = control_delta.mag2()
        - edge.rest_length_squared;
    const long double excited_u = excited_delta.mag2()
        - edge.rest_length_squared;
    result += 0.25L * stiffness * (excited_u - control_u)
        * (excited_u + control_u);
  }
  return static_cast<double>(result);
}

MatchedFaceFlux subtract(const MatchedFaceFlux& left,
                         const MatchedFaceFlux& right) {
  MatchedFaceFlux result(left.L);
  for (std::size_t index = 0; index < left.x.size(); ++index) {
    result.x[index] = left.x[index] - right.x[index];
    result.y[index] = left.y[index] - right.y[index];
    result.z[index] = left.z[index] - right.z[index];
  }
  return result;
}

MatchedEdgeField subtract(const MatchedEdgeField& left,
                          const MatchedEdgeField& right) {
  MatchedEdgeField result(left.L);
  for (std::size_t index = 0; index < left.x.size(); ++index) {
    result.x[index] = left.x[index] - right.x[index];
    result.y[index] = left.y[index] - right.y[index];
    result.z[index] = left.z[index] - right.z[index];
  }
  return result;
}

}  // namespace

ConnectedReservoirDecomposition evaluate_connected_reservoir_decomposition(
    const ConnectedMooreBlockState& control,
    const ConnectedMooreBlockState& excited,
    const std::vector<ConnectedTangentMode>& modes,
    const std::vector<std::size_t>& target_modes,
    double interaction_scale,
    const ConnectedMooreBlockOptions& options,
    double tolerance) {
  ConnectedReservoirDecomposition result;
  result.L = control.electric.L;
  result.constituent_count = control.constituents.size();
  result.mode_count = modes.size();
  result.target_mode_count = target_modes.size();
  result.interaction_scale = interaction_scale;
  const std::size_t dimension = 3 * result.constituent_count;
  if (!finite_state(control) || !finite_state(excited)
      || control.electric.L != excited.electric.L
      || control.constituents.size() != excited.constituents.size()
      || !same_graph(control, excited) || modes.size() != dimension
      || target_modes.empty() || !finite(interaction_scale)
      || !(interaction_scale > 0.0) || !finite(tolerance)
      || !(tolerance > 0.0) || !finite(options.constituent_mass_scale)
      || !(options.constituent_mass_scale > 0.0)
      || !finite(options.binding_stiffness)
      || !(options.binding_stiffness >= 0.0)) {
    return result;
  }

  std::vector<bool> target(dimension, false);
  for (std::size_t index : target_modes) {
    if (index >= dimension || target[index]) return result;
    target[index] = true;
  }
  for (const auto& mode : modes) {
    if (!finite(mode.omega) || !(mode.omega > 0.0)
        || mode.vector.size() != dimension
        || !std::all_of(mode.vector.begin(), mode.vector.end(),
            [](double value) { return finite(value); })) {
      return result;
    }
  }

  const double mass = options.constituent_mass_scale * M_INERTIAL;
  for (std::size_t row = 0; row < dimension; ++row) {
    for (std::size_t column = 0; column < dimension; ++column) {
      long double inner = 0.0L;
      for (std::size_t coordinate = 0; coordinate < dimension; ++coordinate) {
        inner += static_cast<long double>(modes[row].vector[coordinate])
            * mass * modes[column].vector[coordinate];
      }
      result.mode_orthonormality_residual = std::max(
          result.mode_orthonormality_residual,
          std::abs(static_cast<double>(inner)
                   - (row == column ? 1.0 : 0.0)));
    }
  }
  if (result.mode_orthonormality_residual > tolerance) return result;

  result.modal_positions.assign(dimension, 0.0);
  result.modal_momenta.assign(dimension, 0.0);
  result.modal_energies.assign(dimension, 0.0);
  for (std::size_t mode = 0; mode < dimension; ++mode) {
    long double q = 0.0L;
    long double p = 0.0L;
    for (std::size_t particle = 0;
         particle < result.constituent_count; ++particle) {
      const Vec3 delta_x = position(excited.constituents[particle])
          - position(control.constituents[particle]);
      const Vec3 delta_p = excited.constituents[particle].momentum
          - control.constituents[particle].momentum;
      for (int axis = 0; axis < 3; ++axis) {
        const std::size_t coordinate = 3 * particle + axis;
        q += static_cast<long double>(modes[mode].vector[coordinate])
            * mass * periodic_delta(component(delta_x, axis), result.L);
        p += static_cast<long double>(modes[mode].vector[coordinate])
            * component(delta_p, axis);
      }
    }
    result.modal_positions[mode] = static_cast<double>(q);
    result.modal_momenta[mode] = static_cast<double>(p);
    result.modal_energies[mode] = 0.5 * (
        result.modal_momenta[mode] * result.modal_momenta[mode]
        + modes[mode].omega * modes[mode].omega
            * result.modal_positions[mode] * result.modal_positions[mode]);
    result.total_mode_energy += result.modal_energies[mode];
    if (target[mode]) result.target_mode_energy += result.modal_energies[mode];
    else result.other_mode_energy += result.modal_energies[mode];
  }

  result.kinetic_difference = kinetic_energy_difference(
      control, excited, options.constituent_mass_scale);
  result.binding_difference = binding_energy_difference(
      control, excited, options.binding_stiffness);
  result.exact_matter_difference = result.kinetic_difference
      + result.binding_difference;
  result.matter_nonlinear_remainder = result.exact_matter_difference
      - result.total_mode_energy;
  result.matter_decomposition_residual = std::abs(
      result.exact_matter_difference - result.target_mode_energy
      - result.other_mode_energy - result.matter_nonlinear_remainder);

  const double lambda = options.wave_speed * options.dt;
  const auto dynamic_electric = subtract(excited.electric, control.electric);
  const auto dynamic_magnetic = subtract(
      excited.magnetic_half, control.magnetic_half);
  result.dynamic_field_energy = interaction_scale * matched_modified_energy(
      dynamic_electric, dynamic_magnetic, lambda);
  const auto curl_dynamic = matched_curl_adjoint(dynamic_electric);
  const auto curl_control = matched_curl_adjoint(control.electric);
  result.field_interference = interaction_scale * static_cast<double>(
      matched_face_dot(control.electric, dynamic_electric)
      + matched_edge_dot(control.magnetic_half, dynamic_magnetic)
      - 0.5L * lambda * (
          matched_edge_dot(control.magnetic_half, curl_dynamic)
          + matched_edge_dot(dynamic_magnetic, curl_control)));
  long double square_difference = 0.0L;
  for (std::size_t index = 0; index < control.electric.x.size(); ++index) {
    square_difference += 0.5L * (
        static_cast<long double>(dynamic_electric.x[index])
            * (excited.electric.x[index] + control.electric.x[index])
        + static_cast<long double>(dynamic_electric.y[index])
            * (excited.electric.y[index] + control.electric.y[index])
        + static_cast<long double>(dynamic_electric.z[index])
            * (excited.electric.z[index] + control.electric.z[index])
        + static_cast<long double>(dynamic_magnetic.x[index])
            * (excited.magnetic_half.x[index]
               + control.magnetic_half.x[index])
        + static_cast<long double>(dynamic_magnetic.y[index])
            * (excited.magnetic_half.y[index]
               + control.magnetic_half.y[index])
        + static_cast<long double>(dynamic_magnetic.z[index])
            * (excited.magnetic_half.z[index]
               + control.magnetic_half.z[index]));
  }
  const long double cross_difference =
      matched_edge_dot(control.magnetic_half, curl_dynamic)
      + matched_edge_dot(dynamic_magnetic, curl_control)
      + matched_edge_dot(dynamic_magnetic, curl_dynamic);
  result.field_difference = interaction_scale * static_cast<double>(
      square_difference - 0.5L * lambda * cross_difference);
  result.field_decomposition_residual = std::abs(
      result.field_difference - result.dynamic_field_energy
      - result.field_interference);

  result.total_difference = result.exact_matter_difference
      + result.field_difference;
  result.complete_decomposition_residual = std::abs(
      result.total_difference - result.target_mode_energy
      - result.other_mode_energy - result.matter_nonlinear_remainder
      - result.dynamic_field_energy - result.field_interference);
  result.valid = result.field_decomposition_residual <= tolerance
      && result.matter_decomposition_residual <= tolerance
      && result.complete_decomposition_residual <= tolerance
      && finite(result.total_difference)
      && finite(result.matter_nonlinear_remainder)
      && finite(result.field_interference);
  return result;
}

}  // namespace ftd::eft
