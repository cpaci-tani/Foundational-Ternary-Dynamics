#include "ftd/eft/coupled_matched_face_transaction.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>

namespace ftd::eft {
namespace {

double component(const Vec3& value, int axis) {
  if (axis == 0) return value.x;
  if (axis == 1) return value.y;
  return value.z;
}

void set_component(Vec3& value, int axis, double component_value) {
  if (axis == 0) value.x = component_value;
  if (axis == 1) value.y = component_value;
  if (axis == 2) value.z = component_value;
}

int wrap(int coordinate, int L) {
  const int remainder = coordinate % L;
  return remainder < 0 ? remainder + L : remainder;
}

double maximum_component(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

bool finite(const MatchedFaceFlux& field) {
  const auto finite_values = [](const std::vector<double>& values) {
    return std::all_of(values.begin(), values.end(), [](double value) {
      return std::isfinite(value);
    });
  };
  return finite_values(field.x) && finite_values(field.y)
      && finite_values(field.z);
}

bool finite(const MatchedEdgeField& field) {
  const auto finite_values = [](const std::vector<double>& values) {
    return std::all_of(values.begin(), values.end(), [](double value) {
      return std::isfinite(value);
    });
  };
  return finite_values(field.x) && finite_values(field.y)
      && finite_values(field.z);
}

Vec3 effective_position(const MatchedMatterPoint& point) {
  return {static_cast<double>(point.anchor.x) + point.remainder.x,
          static_cast<double>(point.anchor.y) + point.remainder.y,
          static_cast<double>(point.anchor.z) + point.remainder.z};
}

MatchedMatterPoint point_at_effective_position(
    const Vec3& position, int L, const Vec3& momentum) {
  MatchedMatterPoint result;
  result.momentum = momentum;
  const long long ax = std::llround(position.x);
  const long long ay = std::llround(position.y);
  const long long az = std::llround(position.z);
  result.anchor = {wrap(static_cast<int>(ax), L),
                   wrap(static_cast<int>(ay), L),
                   wrap(static_cast<int>(az), L)};
  result.remainder = {
      position.x - static_cast<double>(ax),
      position.y - static_cast<double>(ay),
      position.z - static_cast<double>(az)};
  return result;
}

double periodic_difference(double a, double b, int L) {
  double difference = a - b;
  difference -= std::round(difference / static_cast<double>(L)) * L;
  return difference;
}

double matter_position_residual(const MatchedMatterPoint& a,
                                const MatchedMatterPoint& b,
                                int L) {
  const Vec3 pa = effective_position(a);
  const Vec3 pb = effective_position(b);
  return std::max({std::abs(periodic_difference(pa.x, pb.x, L)),
                   std::abs(periodic_difference(pa.y, pb.y, L)),
                   std::abs(periodic_difference(pa.z, pb.z, L))});
}

const std::vector<double>& component_field(
    const MatchedFaceFlux& field, int axis) {
  if (axis == 0) return field.x;
  if (axis == 1) return field.y;
  return field.z;
}

const std::vector<double>& component_field(
    const MatchedEdgeField& field, int axis) {
  if (axis == 0) return field.x;
  if (axis == 1) return field.y;
  return field.z;
}

const std::vector<double>& component_current(
    const FaceCurrentSegment& segment, int axis) {
  if (axis == 0) return segment.current_x;
  if (axis == 1) return segment.current_y;
  return segment.current_z;
}

template <typename Field>
double trilinear_component(const Field& field,
                           const Vec3& position,
                           int axis) {
  const int x0 = static_cast<int>(std::floor(position.x));
  const int y0 = static_cast<int>(std::floor(position.y));
  const int z0 = static_cast<int>(std::floor(position.z));
  const double fx = position.x - x0;
  const double fy = position.y - y0;
  const double fz = position.z - z0;
  const auto& values = component_field(field, axis);
  long double result = 0.0L;
  for (int dx = 0; dx <= 1; ++dx) {
    const double wx = dx == 0 ? 1.0 - fx : fx;
    for (int dy = 0; dy <= 1; ++dy) {
      const double wy = dy == 0 ? 1.0 - fy : fy;
      for (int dz = 0; dz <= 1; ++dz) {
        const double wz = dz == 0 ? 1.0 - fz : fz;
        const int index = field.index(x0 + dx, y0 + dy, z0 + dz);
        result += static_cast<long double>(wx * wy * wz)
            * values[static_cast<std::size_t>(index)];
      }
    }
  }
  return static_cast<double>(result);
}

template <typename Field>
Vec3 trilinear_vector(const Field& field, const Vec3& position) {
  return {trilinear_component(field, position, 0),
          trilinear_component(field, position, 1),
          trilinear_component(field, position, 2)};
}

MatchedFaceFlux midpoint_field(const MatchedFaceFlux& before,
                               const MatchedFaceFlux& after) {
  MatchedFaceFlux result(before.L);
  for (std::size_t index = 0; index < before.x.size(); ++index) {
    result.x[index] = 0.5 * (before.x[index] + after.x[index]);
    result.y[index] = 0.5 * (before.y[index] + after.y[index]);
    result.z[index] = 0.5 * (before.z[index] + after.z[index]);
  }
  return result;
}

long double current_field_pairing(const FaceCurrentSegment& segment,
                                  const MatchedFaceFlux& field) {
  long double result = 0.0L;
  for (std::size_t index = 0; index < field.x.size(); ++index) {
    result += static_cast<long double>(segment.current_x[index])
        * field.x[index];
    result += static_cast<long double>(segment.current_y[index])
        * field.y[index];
    result += static_cast<long double>(segment.current_z[index])
        * field.z[index];
  }
  return result;
}

Vec3 compatible_electric_path_average(
    const FaceCurrentSegment& segment,
    const MatchedFaceFlux& electric_midpoint) {
  Vec3 result{};
  const Vec3 displacement = segment.end_effective_position
      - segment.start_effective_position;
  const Vec3 midpoint = segment.start_effective_position
      + displacement * 0.5;
  for (int axis = 0; axis < 3; ++axis) {
    const double distance = component(displacement, axis);
    const auto& current = component_current(segment, axis);
    const auto& electric = component_field(electric_midpoint, axis);
    long double component_work = 0.0L;
    for (std::size_t index = 0; index < current.size(); ++index)
      component_work += static_cast<long double>(current[index])
          * electric[index];
    const double scale = std::max(1.0,
        std::max(std::abs(component(segment.start_effective_position, axis)),
                 std::abs(component(segment.end_effective_position, axis))));
    const double threshold = 64.0
        * std::numeric_limits<double>::epsilon() * scale;
    const double gathered = std::abs(distance) > threshold
        ? static_cast<double>(component_work)
            / (static_cast<double>(segment.charge) * distance)
        : trilinear_component(electric_midpoint, midpoint, axis);
    set_component(result, axis, gathered);
  }
  return result;
}

void add_current(MatchedFaceFlux& electric,
                 const FaceCurrentSegment& segment,
                 double scale) {
  for (std::size_t index = 0; index < electric.x.size(); ++index) {
    electric.x[index] += scale * segment.current_x[index];
    electric.y[index] += scale * segment.current_y[index];
    electric.z[index] += scale * segment.current_z[index];
  }
}

struct Candidate {
  bool valid = false;
  Vec3 unknown_momentum{};
  Vec3 residual{};
  Vec3 velocity{};
  Vec3 electric_path{};
  Vec3 magnetic_path{};
  Vec3 electric_impulse{};
  Vec3 magnetic_impulse{};
  Vec3 total_impulse{};
  MatchedMatterPoint other_point{};
  FaceCurrentSegment segment;
  MatchedFaceFlux electric_other;
  MatchedEdgeField magnetic_other;
};

double solve_linear_3x3(std::array<std::array<double, 3>, 3> matrix,
                        std::array<double, 3> rhs,
                        Vec3& solution) {
  double determinant = 1.0;
  int sign = 1;
  for (int column = 0; column < 3; ++column) {
    int pivot = column;
    for (int row = column + 1; row < 3; ++row) {
      if (std::abs(matrix[static_cast<std::size_t>(row)]
                          [static_cast<std::size_t>(column)])
          > std::abs(matrix[static_cast<std::size_t>(pivot)]
                            [static_cast<std::size_t>(column)])) {
        pivot = row;
      }
    }
    const double pivot_value = matrix[static_cast<std::size_t>(pivot)]
        [static_cast<std::size_t>(column)];
    if (!std::isfinite(pivot_value) || std::abs(pivot_value) < 1e-14)
      return 0.0;
    if (pivot != column) {
      std::swap(matrix[static_cast<std::size_t>(pivot)],
                matrix[static_cast<std::size_t>(column)]);
      std::swap(rhs[static_cast<std::size_t>(pivot)],
                rhs[static_cast<std::size_t>(column)]);
      sign = -sign;
    }
    determinant *= matrix[static_cast<std::size_t>(column)]
        [static_cast<std::size_t>(column)];
    for (int row = column + 1; row < 3; ++row) {
      const double factor = matrix[static_cast<std::size_t>(row)]
          [static_cast<std::size_t>(column)]
          / matrix[static_cast<std::size_t>(column)]
                  [static_cast<std::size_t>(column)];
      for (int entry = column; entry < 3; ++entry)
        matrix[static_cast<std::size_t>(row)]
              [static_cast<std::size_t>(entry)] -= factor
            * matrix[static_cast<std::size_t>(column)]
                    [static_cast<std::size_t>(entry)];
      rhs[static_cast<std::size_t>(row)] -=
          factor * rhs[static_cast<std::size_t>(column)];
    }
  }
  std::array<double, 3> values{};
  for (int row = 2; row >= 0; --row) {
    double value = rhs[static_cast<std::size_t>(row)];
    for (int column = row + 1; column < 3; ++column)
      value -= matrix[static_cast<std::size_t>(row)]
                     [static_cast<std::size_t>(column)]
          * values[static_cast<std::size_t>(column)];
    values[static_cast<std::size_t>(row)] = value
        / matrix[static_cast<std::size_t>(row)]
                [static_cast<std::size_t>(row)];
  }
  solution = {values[0], values[1], values[2]};
  return sign * determinant;
}

struct RootResult {
  Vec3 momentum{};
  Candidate candidate;
  LocalImplicitSolveDiagnostics diagnostics;
};

RootResult solve_local_root(
    const Vec3& initial,
    const CoupledMatchedFaceOptions& options,
    const std::function<Candidate(const Vec3&)>& evaluate) {
  RootResult result;
  result.momentum = initial;
  result.diagnostics.attempted = true;
  result.diagnostics.minimum_abs_jacobian_determinant =
      std::numeric_limits<double>::infinity();
  Candidate current = evaluate(result.momentum);
  if (!current.valid) {
    result.diagnostics.residual = INFINITY;
    result.diagnostics.minimum_abs_jacobian_determinant = 0.0;
    return result;
  }

  for (int iteration = 0; iteration <= options.max_iterations; ++iteration) {
    const double residual = maximum_component(current.residual);
    result.diagnostics.residual = residual;
    result.diagnostics.iterations = iteration;
    if (residual <= options.solve_tolerance) {
      result.diagnostics.converged = true;
      break;
    }
    if (iteration == options.max_iterations) break;

    std::array<std::array<double, 3>, 3> jacobian{};
    bool jacobian_valid = true;
    for (int axis = 0; axis < 3; ++axis) {
      const double base = component(result.momentum, axis);
      const double step = options.finite_difference_scale
          * std::max(1.0, std::abs(base));
      Vec3 plus = result.momentum;
      Vec3 minus = result.momentum;
      set_component(plus, axis, base + step);
      set_component(minus, axis, base - step);
      const Candidate high = evaluate(plus);
      const Candidate low = evaluate(minus);
      if (!high.valid || !low.valid) {
        jacobian_valid = false;
        break;
      }
      for (int row = 0; row < 3; ++row)
        jacobian[static_cast<std::size_t>(row)]
                [static_cast<std::size_t>(axis)] =
            (component(high.residual, row) - component(low.residual, row))
            / (2.0 * step);
    }

    Vec3 newton_step = current.residual * -1.0;
    double determinant = 0.0;
    if (jacobian_valid) {
      determinant = solve_linear_3x3(
          jacobian,
          {{-current.residual.x, -current.residual.y,
            -current.residual.z}},
          newton_step);
    }
    if (determinant != 0.0 && std::isfinite(determinant)) {
      result.diagnostics.minimum_abs_jacobian_determinant = std::min(
          result.diagnostics.minimum_abs_jacobian_determinant,
          std::abs(determinant));
    }

    bool accepted = false;
    double scale = 1.0;
    Candidate next;
    Vec3 next_momentum{};
    for (int line_search = 0; line_search < 14; ++line_search) {
      next_momentum = result.momentum + newton_step * scale;
      next = evaluate(next_momentum);
      if (next.valid
          && maximum_component(next.residual) < residual) {
        accepted = true;
        break;
      }
      scale *= 0.5;
      ++result.diagnostics.rejected_steps;
    }
    if (!accepted) break;
    result.diagnostics.step_residual =
        maximum_component(next_momentum - result.momentum);
    result.momentum = next_momentum;
    current = std::move(next);
  }
  result.candidate = current;
  if (!std::isfinite(
          result.diagnostics.minimum_abs_jacobian_determinant))
    result.diagnostics.minimum_abs_jacobian_determinant = 0.0;
  return result;
}

struct PreparedForward {
  MatchedEdgeField magnetic_after;
  MatchedFaceFlux electric_pre_current;
};

PreparedForward prepare_forward_fields(
    const CoupledMatchedFaceState& before,
    double lambda) {
  PreparedForward prepared{before.magnetic_half, before.electric};
  const auto electric_curl = matched_curl_adjoint(before.electric);
  for (std::size_t index = 0; index < prepared.magnetic_after.x.size();
       ++index) {
    prepared.magnetic_after.x[index] -= lambda * electric_curl.x[index];
    prepared.magnetic_after.y[index] -= lambda * electric_curl.y[index];
    prepared.magnetic_after.z[index] -= lambda * electric_curl.z[index];
  }
  const auto magnetic_curl = matched_curl(prepared.magnetic_after);
  for (std::size_t index = 0; index < prepared.electric_pre_current.x.size();
       ++index) {
    prepared.electric_pre_current.x[index] += lambda * magnetic_curl.x[index];
    prepared.electric_pre_current.y[index] += lambda * magnetic_curl.y[index];
    prepared.electric_pre_current.z[index] += lambda * magnetic_curl.z[index];
  }
  return prepared;
}

Candidate evaluate_forward_candidate(
    const CoupledMatchedFaceState& before,
    int charge,
    const CoupledMatchedFaceOptions& options,
    const PreparedForward& prepared,
    double interaction_scale,
    const Vec3& momentum_after) {
  Candidate candidate;
  candidate.unknown_momentum = momentum_after;
  const double energy_before = production_flat_energy_from_momentum(
      before.matter.momentum);
  const double energy_after = production_flat_energy_from_momentum(
      momentum_after);
  const double denominator = energy_before + energy_after;
  if (!(denominator > 0.0) || !std::isfinite(denominator)) return candidate;
  candidate.velocity = (before.matter.momentum + momentum_after)
      * (C_SPEED * C_SPEED / denominator);
  const Vec3 start_position = effective_position(before.matter);
  const Vec3 end_position = start_position
      + candidate.velocity * options.dt;
  candidate.other_point = point_at_effective_position(
      end_position, before.electric.L, momentum_after);
  candidate.segment = make_face_current_segment(
      before.electric.L,
      before.matter.anchor, before.matter.remainder,
      candidate.other_point.anchor, candidate.other_point.remainder,
      charge);
  if (!candidate.segment.valid) return candidate;

  candidate.electric_other = prepared.electric_pre_current;
  add_current(candidate.electric_other, candidate.segment, -1.0);
  candidate.magnetic_other = prepared.magnetic_after;
  const MatchedFaceFlux electric_midpoint = midpoint_field(
      before.electric, candidate.electric_other);
  candidate.electric_path = compatible_electric_path_average(
      candidate.segment, electric_midpoint);
  const Vec3 path_midpoint = candidate.segment.start_effective_position
      + (candidate.segment.end_effective_position
         - candidate.segment.start_effective_position) * 0.5;
  // Selected local edge-to-path collocation.  This is deliberately surfaced
  // as underderived by the public transaction result.
  candidate.magnetic_path = trilinear_vector(
      prepared.magnetic_after, path_midpoint);
  candidate.electric_impulse = candidate.electric_path
      * (static_cast<double>(charge) * options.dt * interaction_scale);
  candidate.magnetic_impulse = Vec3::cross(
      candidate.velocity, candidate.magnetic_path)
      * (static_cast<double>(charge) * options.dt * interaction_scale);
  candidate.total_impulse = candidate.electric_impulse
      + candidate.magnetic_impulse;
  candidate.residual = momentum_after - before.matter.momentum
      - candidate.total_impulse;
  candidate.valid = finite(candidate.velocity)
      && finite(candidate.electric_path) && finite(candidate.magnetic_path)
      && finite(candidate.residual) && finite(candidate.electric_other)
      && finite(candidate.magnetic_other);
  return candidate;
}

Candidate evaluate_inverse_candidate(
    const CoupledMatchedFaceState& after,
    int charge,
    const CoupledMatchedFaceOptions& options,
    double interaction_scale,
    const Vec3& momentum_before) {
  Candidate candidate;
  candidate.unknown_momentum = momentum_before;
  const double energy_before = production_flat_energy_from_momentum(
      momentum_before);
  const double energy_after = production_flat_energy_from_momentum(
      after.matter.momentum);
  const double denominator = energy_before + energy_after;
  if (!(denominator > 0.0) || !std::isfinite(denominator)) return candidate;
  candidate.velocity = (momentum_before + after.matter.momentum)
      * (C_SPEED * C_SPEED / denominator);
  const Vec3 end_position = effective_position(after.matter);
  const Vec3 start_position = end_position
      - candidate.velocity * options.dt;
  candidate.other_point = point_at_effective_position(
      start_position, after.electric.L, momentum_before);
  candidate.segment = make_face_current_segment(
      after.electric.L,
      candidate.other_point.anchor, candidate.other_point.remainder,
      after.matter.anchor, after.matter.remainder,
      charge);
  if (!candidate.segment.valid) return candidate;

  MatchedFaceFlux electric_pre_current = after.electric;
  add_current(electric_pre_current, candidate.segment, +1.0);
  candidate.electric_other = electric_pre_current;
  const double lambda = options.wave_speed * options.dt;
  const auto magnetic_curl = matched_curl(after.magnetic_half);
  for (std::size_t index = 0; index < candidate.electric_other.x.size();
       ++index) {
    candidate.electric_other.x[index] -= lambda * magnetic_curl.x[index];
    candidate.electric_other.y[index] -= lambda * magnetic_curl.y[index];
    candidate.electric_other.z[index] -= lambda * magnetic_curl.z[index];
  }
  candidate.magnetic_other = after.magnetic_half;
  const auto electric_curl = matched_curl_adjoint(candidate.electric_other);
  for (std::size_t index = 0; index < candidate.magnetic_other.x.size();
       ++index) {
    candidate.magnetic_other.x[index] += lambda * electric_curl.x[index];
    candidate.magnetic_other.y[index] += lambda * electric_curl.y[index];
    candidate.magnetic_other.z[index] += lambda * electric_curl.z[index];
  }

  const MatchedFaceFlux electric_midpoint = midpoint_field(
      candidate.electric_other, after.electric);
  candidate.electric_path = compatible_electric_path_average(
      candidate.segment, electric_midpoint);
  const Vec3 path_midpoint = candidate.segment.start_effective_position
      + (candidate.segment.end_effective_position
         - candidate.segment.start_effective_position) * 0.5;
  candidate.magnetic_path = trilinear_vector(
      after.magnetic_half, path_midpoint);
  candidate.electric_impulse = candidate.electric_path
      * (static_cast<double>(charge) * options.dt * interaction_scale);
  candidate.magnetic_impulse = Vec3::cross(
      candidate.velocity, candidate.magnetic_path)
      * (static_cast<double>(charge) * options.dt * interaction_scale);
  candidate.total_impulse = candidate.electric_impulse
      + candidate.magnetic_impulse;
  candidate.residual = after.matter.momentum - momentum_before
      - candidate.total_impulse;
  candidate.valid = finite(candidate.velocity)
      && finite(candidate.electric_path) && finite(candidate.magnetic_path)
      && finite(candidate.residual) && finite(candidate.electric_other)
      && finite(candidate.magnetic_other);
  return candidate;
}

std::vector<double> total_density(
    const std::vector<double>& moving,
    const std::vector<double>& stationary) {
  std::vector<double> result = moving;
  for (std::size_t index = 0; index < result.size(); ++index)
    result[index] += stationary[index];
  return result;
}

double maximum_state_difference(const CoupledMatchedFaceState& expected,
                                const Candidate& recovered) {
  return std::max({
      matched_face_max_difference(expected.electric,
                                  recovered.electric_other),
      matched_edge_max_difference(expected.magnetic_half,
                                  recovered.magnetic_other),
      matter_position_residual(expected.matter,
                               recovered.other_point,
                               expected.electric.L),
      maximum_component(expected.matter.momentum
                        - recovered.unknown_momentum)});
}

}  // namespace

double max_fractional_gauss_residual(
    const MatchedFaceFlux& electric,
    const std::vector<double>& density) {
  if (electric.L <= 0 || density.size() != electric.x.size()) return INFINITY;
  double residual = 0.0;
  for (int x = 0; x < electric.L; ++x) {
    for (int y = 0; y < electric.L; ++y) {
      for (int z = 0; z < electric.L; ++z) {
        const int index = electric.index(x, y, z);
        residual = std::max(residual,
            std::abs(divergence_at(electric, x, y, z)
                     - density[static_cast<std::size_t>(index)]));
      }
    }
  }
  return residual;
}

CoupledMatchedFaceTransaction solve_coupled_matched_face_transaction(
    const CoupledMatchedFaceState& before,
    int charge,
    const std::vector<double>& stationary_density,
    const CoupledMatchedFaceOptions& options) {
  CoupledMatchedFaceTransaction result;
  result.before = before;
  result.after = CoupledMatchedFaceState(before.electric.L);
  result.charge = charge;
  result.normalization = measure_face_flux_normalization();
  result.interaction_scale =
      result.normalization.mapped_field_work_coefficient;
  const std::size_t expected = before.electric.x.size();
  const bool input_valid = before.electric.L >= 2
      && before.electric.L == before.magnetic_half.L
      && expected == before.electric.y.size()
      && expected == before.electric.z.size()
      && expected == before.magnetic_half.x.size()
      && expected == before.magnetic_half.y.size()
      && expected == before.magnetic_half.z.size()
      && stationary_density.size() == expected
      && (charge == -1 || charge == 1)
      && options.dt > 0.0 && std::isfinite(options.dt)
      && options.wave_speed >= 0.0 && std::isfinite(options.wave_speed)
      && options.gate_tolerance > 0.0
      && options.solve_tolerance > 0.0
      && options.finite_difference_scale > 0.0
      && options.max_iterations > 0
      && result.normalization.valid && result.interaction_scale > 0.0
      && finite(before.electric) && finite(before.magnetic_half)
      && finite(before.matter.remainder) && finite(before.matter.momentum);
  const bool stationary_finite = std::all_of(
      stationary_density.begin(), stationary_density.end(),
      [](double value) { return std::isfinite(value); });
  if (!input_valid || !stationary_finite) return result;
  const auto start_shape = make_subcell_polarity_shape(
      before.matter.anchor, before.matter.remainder, charge);
  if (!start_shape.valid) return result;

  const double lambda = options.wave_speed * options.dt;
  const PreparedForward prepared = prepare_forward_fields(before, lambda);
  const auto evaluator = [&](const Vec3& momentum_after) {
    return evaluate_forward_candidate(
        before, charge, options, prepared, result.interaction_scale,
        momentum_after);
  };
  const RootResult forward = solve_local_root(
      before.matter.momentum, options, evaluator);
  result.solve = forward.diagnostics;
  if (!forward.candidate.valid) return result;

  const Candidate& candidate = forward.candidate;
  result.segment = candidate.segment;
  result.after.electric = candidate.electric_other;
  result.after.magnetic_half = candidate.magnetic_other;
  result.after.matter = candidate.other_point;
  result.displacement = candidate.segment.end_effective_position
      - candidate.segment.start_effective_position;
  const double displacement_scale = std::max({
      1.0, std::abs(result.displacement.x),
      std::abs(result.displacement.y), std::abs(result.displacement.z)});
  const double transverse_threshold = 64.0
      * std::numeric_limits<double>::epsilon() * displacement_scale;
  result.electric_transverse_rule_underderived =
      std::abs(result.displacement.x) <= transverse_threshold
      || std::abs(result.displacement.y) <= transverse_threshold
      || std::abs(result.displacement.z) <= transverse_threshold;
  result.discrete_gradient_velocity = candidate.velocity;
  result.electric_path_average = candidate.electric_path;
  result.magnetic_path_average = candidate.magnetic_path;
  result.electric_impulse = candidate.electric_impulse;
  result.magnetic_impulse = candidate.magnetic_impulse;
  result.total_impulse = candidate.total_impulse;

  result.particle_energy_before = production_flat_energy_from_momentum(
      before.matter.momentum);
  result.particle_energy_after = production_flat_energy_from_momentum(
      result.after.matter.momentum);
  result.field_energy_before = result.interaction_scale
      * matched_modified_energy(before.electric, before.magnetic_half,
                                lambda);
  result.field_energy_after = result.interaction_scale
      * matched_modified_energy(result.after.electric,
                                result.after.magnetic_half, lambda);
  const MatchedFaceFlux electric_midpoint = midpoint_field(
      before.electric, result.after.electric);
  result.current_midpoint_work = result.interaction_scale
      * static_cast<double>(
          current_field_pairing(result.segment, electric_midpoint));
  result.magnetic_work = result.discrete_gradient_velocity.dot(
      result.magnetic_impulse);

  const std::vector<double> density_before = total_density(
      result.segment.rho_before, stationary_density);
  const std::vector<double> density_after = total_density(
      result.segment.rho_after, stationary_density);
  result.continuity_residual = result.segment.continuity_residual;
  result.gauss_before_residual = max_fractional_gauss_residual(
      before.electric, density_before);
  result.gauss_after_residual = max_fractional_gauss_residual(
      result.after.electric, density_after);
  result.force_residual = maximum_component(
      result.after.matter.momentum - before.matter.momentum
      - result.total_impulse);
  const double particle_change = result.particle_energy_after
      - result.particle_energy_before;
  const double field_change = result.field_energy_after
      - result.field_energy_before;
  result.discrete_gradient_residual = particle_change
      - result.discrete_gradient_velocity.dot(
          result.after.matter.momentum - before.matter.momentum);
  result.work_residual = particle_change - result.current_midpoint_work;
  result.field_work_residual = field_change
      + result.current_midpoint_work;
  result.total_energy_residual = particle_change + field_change;
  const double kinematic_covariance = maximum_component(
      result.displacement
      - result.discrete_gradient_velocity * options.dt);
  const double work_covariance = result.current_midpoint_work
      - result.discrete_gradient_velocity.dot(result.electric_impulse);
  result.covariance_residual = std::max(
      kinematic_covariance, std::abs(work_covariance));
  result.causal_speed_excess = std::max(
      0.0, result.discrete_gradient_velocity.mag() - C_SPEED);

  // Explicit inverse: retain the accepted segment/current and undo the three
  // affine field substeps in reverse order.  Matter uses the recorded common
  // impulse.  This tests the actual stored transaction, not an inferred path.
  Candidate explicit_inverse;
  explicit_inverse.valid = true;
  explicit_inverse.electric_other = result.after.electric;
  add_current(explicit_inverse.electric_other, result.segment, +1.0);
  const auto reverse_magnetic_curl = matched_curl(
      result.after.magnetic_half);
  for (std::size_t index = 0; index < expected; ++index) {
    explicit_inverse.electric_other.x[index] -=
        lambda * reverse_magnetic_curl.x[index];
    explicit_inverse.electric_other.y[index] -=
        lambda * reverse_magnetic_curl.y[index];
    explicit_inverse.electric_other.z[index] -=
        lambda * reverse_magnetic_curl.z[index];
  }
  explicit_inverse.magnetic_other = result.after.magnetic_half;
  const auto reverse_electric_curl = matched_curl_adjoint(
      explicit_inverse.electric_other);
  for (std::size_t index = 0; index < expected; ++index) {
    explicit_inverse.magnetic_other.x[index] +=
        lambda * reverse_electric_curl.x[index];
    explicit_inverse.magnetic_other.y[index] +=
        lambda * reverse_electric_curl.y[index];
    explicit_inverse.magnetic_other.z[index] +=
        lambda * reverse_electric_curl.z[index];
  }
  explicit_inverse.unknown_momentum = result.after.matter.momentum
      - result.total_impulse;
  explicit_inverse.other_point = point_at_effective_position(
      result.segment.start_effective_position, before.electric.L,
      explicit_inverse.unknown_momentum);
  result.inverse.explicit_available = true;
  result.inverse.explicit_residual = maximum_state_difference(
      before, explicit_inverse);

  if (options.infer_inverse) {
    result.inverse.inferred_attempted = true;
    const auto inverse_evaluator = [&](const Vec3& momentum_before) {
      return evaluate_inverse_candidate(
          result.after, charge, options, result.interaction_scale,
          momentum_before);
    };
    const RootResult inferred = solve_local_root(
        result.after.matter.momentum, options, inverse_evaluator);
    result.inverse.inferred_converged = inferred.diagnostics.converged;
    result.inverse.inferred_iterations = inferred.diagnostics.iterations;
    result.inverse.inferred_solve_residual =
        inferred.diagnostics.residual;
    result.inverse.inferred_state_residual = inferred.candidate.valid
        ? maximum_state_difference(before, inferred.candidate)
        : INFINITY;
  }

  const bool finite_diagnostics = std::isfinite(result.field_energy_before)
      && std::isfinite(result.field_energy_after)
      && std::isfinite(result.current_midpoint_work)
      && std::isfinite(result.magnetic_work)
      && std::isfinite(result.continuity_residual)
      && std::isfinite(result.gauss_before_residual)
      && std::isfinite(result.gauss_after_residual)
      && std::isfinite(result.force_residual)
      && std::isfinite(result.discrete_gradient_residual)
      && std::isfinite(result.work_residual)
      && std::isfinite(result.field_work_residual)
      && std::isfinite(result.total_energy_residual)
      && std::isfinite(result.covariance_residual)
      && std::isfinite(result.causal_speed_excess)
      && std::isfinite(result.inverse.explicit_residual);
  result.valid = result.solve.converged && result.segment.valid
      && finite_diagnostics;
  const double gate = options.gate_tolerance;
  const bool inverse_gate = result.inverse.explicit_residual <= gate
      && (!options.infer_inverse
          || (result.inverse.inferred_converged
              && result.inverse.inferred_solve_residual <= gate
              && result.inverse.inferred_state_residual <= gate));
  result.gates_pass = result.valid
      && result.continuity_residual <= gate
      && result.gauss_before_residual <= gate
      && result.gauss_after_residual <= gate
      && result.force_residual <= gate
      && std::abs(result.discrete_gradient_residual) <= gate
      && std::abs(result.work_residual) <= gate
      && std::abs(result.field_work_residual) <= gate
      && std::abs(result.total_energy_residual) <= gate
      && std::abs(result.magnetic_work) <= gate
      && result.covariance_residual <= gate
      && result.causal_speed_excess <= gate
      && inverse_gate;
  return result;
}

}  // namespace ftd::eft
