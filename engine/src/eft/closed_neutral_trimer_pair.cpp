#include "ftd/eft/closed_neutral_trimer_pair.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <utility>
#include <vector>

namespace ftd::eft {
namespace {

constexpr std::size_t N = CLOSED_NEUTRAL_PAIR_SIZE;
constexpr std::size_t NG = CLOSED_NEUTRAL_TRIMER_SIZE;
constexpr std::size_t ND = 3 * N;
using VectorN = std::array<double, ND>;
using MatrixN = std::array<std::array<double, ND>, ND>;

double component(const Vec3& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
}

double maximum_component(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

std::size_t volume(int L) {
  return L > 0 ? static_cast<std::size_t>(L) * L * L : 0;
}

template <typename Field>
bool finite_field(const Field& field) {
  const std::size_t expected = volume(field.L);
  if (field.L <= 0 || field.x.size() != expected
      || field.y.size() != expected || field.z.size() != expected)
    return false;
  const auto finite_entries = [](const std::vector<double>& entries) {
    return std::all_of(entries.begin(), entries.end(),
        [](double value) { return std::isfinite(value); });
  };
  return finite_entries(field.x) && finite_entries(field.y)
      && finite_entries(field.z);
}

int wrap(int value, int L) {
  const int remainder = value % L;
  return remainder < 0 ? remainder + L : remainder;
}

Vec3 effective_position(const MatchedMatterPoint& point) {
  return {point.anchor.x + point.remainder.x,
          point.anchor.y + point.remainder.y,
          point.anchor.z + point.remainder.z};
}

MatchedMatterPoint point_at(const Vec3& position, int L,
                            const Vec3& momentum) {
  MatchedMatterPoint point;
  const long long ax = std::llround(position.x);
  const long long ay = std::llround(position.y);
  const long long az = std::llround(position.z);
  point.anchor = {wrap(static_cast<int>(ax), L),
                  wrap(static_cast<int>(ay), L),
                  wrap(static_cast<int>(az), L)};
  point.remainder = {position.x - ax, position.y - ay, position.z - az};
  point.momentum = momentum;
  return point;
}

double periodic_difference(double lhs, double rhs, int L) {
  const double raw = lhs - rhs;
  return raw - std::round(raw / L) * L;
}

MatchedFaceFlux midpoint(const MatchedFaceFlux& lhs,
                         const MatchedFaceFlux& rhs) {
  MatchedFaceFlux result(lhs.L);
  for (std::size_t i = 0; i < lhs.x.size(); ++i) {
    result.x[i] = 0.5 * (lhs.x[i] + rhs.x[i]);
    result.y[i] = 0.5 * (lhs.y[i] + rhs.y[i]);
    result.z[i] = 0.5 * (lhs.z[i] + rhs.z[i]);
  }
  return result;
}

void add_current(MatchedFaceFlux& field,
                 const QuadraticCoatFaceCurrent& segment,
                 double scale) {
  for (std::size_t i = 0; i < field.x.size(); ++i) {
    field.x[i] += scale * segment.current_x[i];
    field.y[i] += scale * segment.current_y[i];
    field.z[i] += scale * segment.current_z[i];
  }
}

void add_density(std::vector<double>& density,
                 const std::vector<double>& addition) {
  for (std::size_t i = 0; i < density.size(); ++i)
    density[i] += addition[i];
}

bool same_anchor(const Coord& lhs, const Coord& rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

bool site_projection_valid(const ClosedNeutralTrimerPairState& state) {
  for (std::size_t a = 0; a < N; ++a) {
    if (state.charges[a] != -1 && state.charges[a] != +1) return false;
    for (std::size_t b = a + 1; b < N; ++b)
      if (same_anchor(state.constituents[a].anchor,
                      state.constituents[b].anchor)) return false;
  }
  return true;
}

bool charges_valid(const ClosedNeutralTrimerPairState& state) {
  std::array<int, 2> group_sum{{0, 0}};
  std::array<int, 2> positive{{0, 0}};
  std::array<int, 2> negative{{0, 0}};
  for (std::size_t a = 0; a < N; ++a) {
    const std::size_t group = a / NG;
    if (state.charges[a] == +1) ++positive[group];
    else if (state.charges[a] == -1) ++negative[group];
    else return false;
    group_sum[group] += state.charges[a];
  }
  const bool ternary_groups = positive[0] + negative[0] == 3
      && positive[1] + negative[1] == 3
      && std::abs(group_sum[0]) == 1 && std::abs(group_sum[1]) == 1;
  return ternary_groups && group_sum[0] + group_sum[1] == 0;
}

bool state_valid(const ClosedNeutralTrimerPairState& state,
                 const ClosedNeutralPairOptions& options) {
  if (state.electric.L < 5 || state.magnetic_half.L != state.electric.L
      || !finite_field(state.electric) || !finite_field(state.magnetic_half)
      || !charges_valid(state) || !site_projection_valid(state)
      || !(options.dt > 0.0) || !std::isfinite(options.dt)
      || !(options.wave_speed >= 0.0) || !std::isfinite(options.wave_speed)
      || !(options.binding_stiffness >= 0.0)
      || !std::isfinite(options.binding_stiffness)
      || !(options.rest_length_squared > 0.0)
      || !std::isfinite(options.rest_length_squared)
      || !(options.gate_tolerance > 0.0)
      || !(options.solve_tolerance > 0.0)
      || !(options.finite_difference_scale > 0.0)
      || options.max_iterations <= 0) return false;
  for (std::size_t a = 0; a < N; ++a) {
    if (!finite(state.constituents[a].remainder)
        || !finite(state.constituents[a].momentum)) return false;
    if (!make_quadratic_polarity_coat(
        effective_position(state.constituents[a]), state.charges[a]).valid)
      return false;
  }
  return true;
}

std::array<Vec3, N> positions(const ClosedNeutralTrimerPairState& state) {
  std::array<Vec3, N> result{};
  for (std::size_t a = 0; a < N; ++a)
    result[a] = effective_position(state.constituents[a]);
  return result;
}

double binding_energy(const std::array<Vec3, N>& x,
                      const ClosedNeutralPairOptions& options) {
  long double result = 0.0L;
  for (std::size_t group = 0; group < 2; ++group) {
    const std::size_t begin = group * NG;
    for (std::size_t a = begin; a < begin + NG; ++a)
      for (std::size_t b = a + 1; b < begin + NG; ++b) {
        const Vec3 d = x[a] - x[b];
        const long double u = static_cast<long double>(d.dot(d))
            - options.rest_length_squared;
        result += 0.25L * options.binding_stiffness * u * u;
      }
  }
  return static_cast<double>(result);
}

std::array<Vec3, N> binding_impulses(
    const std::array<Vec3, N>& x0, const std::array<Vec3, N>& x1,
    const ClosedNeutralPairOptions& options) {
  std::array<Vec3, N> result{};
  for (std::size_t group = 0; group < 2; ++group) {
    const std::size_t begin = group * NG;
    for (std::size_t a = begin; a < begin + NG; ++a)
      for (std::size_t b = a + 1; b < begin + NG; ++b) {
        const Vec3 d0 = x0[a] - x0[b];
        const Vec3 d1 = x1[a] - x1[b];
        const double u0 = d0.dot(d0) - options.rest_length_squared;
        const double u1 = d1.dot(d1) - options.rest_length_squared;
        const Vec3 gradient = (d0 + d1)
            * (0.25 * options.binding_stiffness * (u0 + u1));
        const Vec3 impulse = gradient * options.dt;
        result[a] -= impulse;
        result[b] += impulse;
      }
  }
  return result;
}

VectorN flatten_momenta(
    const std::array<MatchedMatterPoint, N>& constituents) {
  VectorN result{};
  for (std::size_t a = 0; a < N; ++a)
    for (int axis = 0; axis < 3; ++axis)
      result[3 * a + axis] = component(constituents[a].momentum, axis);
  return result;
}

std::array<Vec3, N> unflatten_momenta(const VectorN& values) {
  std::array<Vec3, N> result{};
  for (std::size_t a = 0; a < N; ++a)
    result[a] = {values[3 * a], values[3 * a + 1], values[3 * a + 2]};
  return result;
}

double infinity_norm(const VectorN& values) {
  double result = 0.0;
  for (double value : values) result = std::max(result, std::abs(value));
  return result;
}

bool finite(const VectorN& values) {
  return std::all_of(values.begin(), values.end(),
      [](double value) { return std::isfinite(value); });
}

struct PreparedForwardFields {
  MatchedEdgeField magnetic_later;
  MatchedFaceFlux electric_pre_current;
  explicit PreparedForwardFields(int L)
      : magnetic_later(L), electric_pre_current(L) {}
};

PreparedForwardFields prepare_forward_fields(
    const ClosedNeutralTrimerPairState& earlier, double lambda) {
  PreparedForwardFields result(earlier.electric.L);
  result.magnetic_later = earlier.magnetic_half;
  const MatchedEdgeField curl_adjoint = matched_curl_adjoint(earlier.electric);
  for (std::size_t i = 0; i < result.magnetic_later.x.size(); ++i) {
    result.magnetic_later.x[i] -= lambda * curl_adjoint.x[i];
    result.magnetic_later.y[i] -= lambda * curl_adjoint.y[i];
    result.magnetic_later.z[i] -= lambda * curl_adjoint.z[i];
  }
  result.electric_pre_current = earlier.electric;
  const MatchedFaceFlux curl = matched_curl(result.magnetic_later);
  for (std::size_t i = 0; i < result.electric_pre_current.x.size(); ++i) {
    result.electric_pre_current.x[i] += lambda * curl.x[i];
    result.electric_pre_current.y[i] += lambda * curl.y[i];
    result.electric_pre_current.z[i] += lambda * curl.z[i];
  }
  return result;
}

struct Candidate {
  bool valid = false;
  ClosedNeutralTrimerPairState earlier;
  ClosedNeutralTrimerPairState later;
  std::array<QuadraticCoatFaceCurrent, N> segments{};
  std::array<QuadraticCoatOrbitGatherResult, N> gathers{};
  std::array<Vec3, N> velocities{};
  std::array<Vec3, N> electric_impulses{};
  std::array<Vec3, N> magnetic_impulses{};
  std::array<Vec3, N> binding_impulses{};
  std::array<Vec3, N> total_impulses{};
  VectorN residual{};
  explicit Candidate(int L = 0) : earlier(L), later(L) {}
};

bool make_segments(Candidate& candidate) {
  const int L = candidate.earlier.electric.L;
  for (std::size_t a = 0; a < N; ++a) {
    candidate.segments[a] = make_quadratic_coat_face_current(
        L, effective_position(candidate.earlier.constituents[a]),
        effective_position(candidate.later.constituents[a]),
        candidate.earlier.charges[a]);
    if (!candidate.segments[a].valid) return false;
  }
  return true;
}

void gather_impulses(Candidate& candidate,
                     const ClosedNeutralPairOptions& options,
                     double interaction_scale) {
  const MatchedFaceFlux electric_midpoint = midpoint(
      candidate.earlier.electric, candidate.later.electric);
  candidate.binding_impulses = binding_impulses(
      positions(candidate.earlier), positions(candidate.later), options);
  for (std::size_t a = 0; a < N; ++a) {
    candidate.gathers[a] = evaluate_quadratic_coat_orbit_gather(
        candidate.segments[a], electric_midpoint,
        candidate.later.magnetic_half, candidate.velocities[a],
        options.dt, interaction_scale);
    if (!candidate.gathers[a].valid) return;
    candidate.electric_impulses[a] = candidate.gathers[a].electric_force
        * (options.dt * interaction_scale);
    candidate.magnetic_impulses[a] = candidate.gathers[a].magnetic_impulse;
    candidate.total_impulses[a] = candidate.electric_impulses[a]
        + candidate.magnetic_impulses[a] + candidate.binding_impulses[a];
    const Vec3 delta = candidate.later.constituents[a].momentum
        - candidate.earlier.constituents[a].momentum
        - candidate.total_impulses[a];
    candidate.residual[3 * a] = delta.x;
    candidate.residual[3 * a + 1] = delta.y;
    candidate.residual[3 * a + 2] = delta.z;
  }
  candidate.valid = finite(candidate.residual)
      && finite_field(candidate.earlier.electric)
      && finite_field(candidate.earlier.magnetic_half)
      && finite_field(candidate.later.electric)
      && finite_field(candidate.later.magnetic_half)
      && site_projection_valid(candidate.earlier)
      && site_projection_valid(candidate.later);
}

Candidate evaluate_forward(const ClosedNeutralTrimerPairState& earlier,
                           const ClosedNeutralPairOptions& options,
                           const PreparedForwardFields& prepared,
                           double interaction_scale,
                           const VectorN& unknown) {
  const int L = earlier.electric.L;
  Candidate candidate(L);
  candidate.earlier = earlier;
  candidate.later.charges = earlier.charges;
  const auto p1 = unflatten_momenta(unknown);
  for (std::size_t a = 0; a < N; ++a) {
    const Vec3 p0 = earlier.constituents[a].momentum;
    const double h0 = production_flat_energy_from_momentum(p0);
    const double h1 = production_flat_energy_from_momentum(p1[a]);
    const double denominator = h0 + h1;
    if (!(denominator > 0.0) || !std::isfinite(denominator)) return candidate;
    candidate.velocities[a] = (p0 + p1[a])
        * (C_SPEED * C_SPEED / denominator);
    candidate.later.constituents[a] = point_at(
        effective_position(earlier.constituents[a])
            + candidate.velocities[a] * options.dt,
        L, p1[a]);
  }
  if (!make_segments(candidate)) return candidate;
  candidate.later.magnetic_half = prepared.magnetic_later;
  candidate.later.electric = prepared.electric_pre_current;
  for (const auto& segment : candidate.segments)
    add_current(candidate.later.electric, segment, -1.0);
  gather_impulses(candidate, options, interaction_scale);
  return candidate;
}

Candidate evaluate_reverse(const ClosedNeutralTrimerPairState& later,
                           const ClosedNeutralPairOptions& options,
                           double interaction_scale,
                           const VectorN& unknown) {
  const int L = later.electric.L;
  const double lambda = options.wave_speed * options.dt;
  Candidate candidate(L);
  candidate.later = later;
  candidate.earlier.charges = later.charges;
  const auto p0 = unflatten_momenta(unknown);
  for (std::size_t a = 0; a < N; ++a) {
    const Vec3 p1 = later.constituents[a].momentum;
    const double h0 = production_flat_energy_from_momentum(p0[a]);
    const double h1 = production_flat_energy_from_momentum(p1);
    const double denominator = h0 + h1;
    if (!(denominator > 0.0) || !std::isfinite(denominator)) return candidate;
    candidate.velocities[a] = (p0[a] + p1)
        * (C_SPEED * C_SPEED / denominator);
    candidate.earlier.constituents[a] = point_at(
        effective_position(later.constituents[a])
            - candidate.velocities[a] * options.dt,
        L, p0[a]);
  }
  if (!make_segments(candidate)) return candidate;
  candidate.earlier.electric = later.electric;
  for (const auto& segment : candidate.segments)
    add_current(candidate.earlier.electric, segment, +1.0);
  const MatchedFaceFlux magnetic_curl = matched_curl(later.magnetic_half);
  for (std::size_t i = 0; i < candidate.earlier.electric.x.size(); ++i) {
    candidate.earlier.electric.x[i] -= lambda * magnetic_curl.x[i];
    candidate.earlier.electric.y[i] -= lambda * magnetic_curl.y[i];
    candidate.earlier.electric.z[i] -= lambda * magnetic_curl.z[i];
  }
  candidate.earlier.magnetic_half = later.magnetic_half;
  const MatchedEdgeField electric_curl = matched_curl_adjoint(
      candidate.earlier.electric);
  for (std::size_t i = 0; i < candidate.earlier.magnetic_half.x.size(); ++i) {
    candidate.earlier.magnetic_half.x[i] += lambda * electric_curl.x[i];
    candidate.earlier.magnetic_half.y[i] += lambda * electric_curl.y[i];
    candidate.earlier.magnetic_half.z[i] += lambda * electric_curl.z[i];
  }
  gather_impulses(candidate, options, interaction_scale);
  return candidate;
}

double solve_linear(MatrixN matrix, VectorN rhs, VectorN& solution) {
  long double determinant = 1.0L;
  int sign = 1;
  for (std::size_t column = 0; column < ND; ++column) {
    std::size_t pivot = column;
    for (std::size_t row = column + 1; row < ND; ++row)
      if (std::abs(matrix[row][column]) > std::abs(matrix[pivot][column]))
        pivot = row;
    if (!std::isfinite(matrix[pivot][column])
        || std::abs(matrix[pivot][column]) < 1e-14) return 0.0;
    if (pivot != column) {
      std::swap(matrix[pivot], matrix[column]);
      std::swap(rhs[pivot], rhs[column]);
      sign = -sign;
    }
    determinant *= matrix[column][column];
    for (std::size_t row = column + 1; row < ND; ++row) {
      const double factor = matrix[row][column] / matrix[column][column];
      for (std::size_t entry = column; entry < ND; ++entry)
        matrix[row][entry] -= factor * matrix[column][entry];
      rhs[row] -= factor * rhs[column];
    }
  }
  for (int row = static_cast<int>(ND) - 1; row >= 0; --row) {
    double value = rhs[static_cast<std::size_t>(row)];
    for (std::size_t column = static_cast<std::size_t>(row) + 1;
         column < ND; ++column)
      value -= matrix[static_cast<std::size_t>(row)][column]
          * solution[column];
    solution[static_cast<std::size_t>(row)] = value
        / matrix[static_cast<std::size_t>(row)][static_cast<std::size_t>(row)];
  }
  return static_cast<double>(sign * determinant);
}

struct RootResult {
  Candidate candidate;
  ClosedNeutralPairSolveDiagnostics diagnostics{};
  explicit RootResult(int L) : candidate(L) {}
};

RootResult solve_root(
    int L, const VectorN& initial, const ClosedNeutralPairOptions& options,
    const std::function<Candidate(const VectorN&)>& evaluate) {
  RootResult result(L);
  result.diagnostics.attempted = true;
  result.diagnostics.minimum_abs_jacobian_determinant = INFINITY;
  VectorN unknown = initial;
  Candidate current = evaluate(unknown);
  if (!current.valid) {
    result.diagnostics.residual = INFINITY;
    result.diagnostics.minimum_abs_jacobian_determinant = 0.0;
    return result;
  }
  for (int iteration = 0; iteration <= options.max_iterations; ++iteration) {
    const double residual = infinity_norm(current.residual);
    result.diagnostics.iterations = iteration;
    result.diagnostics.residual = residual;
    if (residual <= options.solve_tolerance) {
      result.diagnostics.converged = true;
      break;
    }
    if (iteration == options.max_iterations) break;
    MatrixN jacobian{};
    bool usable = true;
    for (std::size_t column = 0; column < ND; ++column) {
      const double delta = options.finite_difference_scale
          * std::max(1.0, std::abs(unknown[column]));
      VectorN high = unknown;
      VectorN low = unknown;
      high[column] += delta;
      low[column] -= delta;
      const Candidate high_candidate = evaluate(high);
      const Candidate low_candidate = evaluate(low);
      if (!high_candidate.valid || !low_candidate.valid) {
        usable = false;
        break;
      }
      for (std::size_t row = 0; row < ND; ++row)
        jacobian[row][column] = (high_candidate.residual[row]
            - low_candidate.residual[row]) / (2.0 * delta);
    }
    VectorN rhs{};
    for (std::size_t row = 0; row < ND; ++row)
      rhs[row] = -current.residual[row];
    VectorN step{};
    const double determinant = usable
        ? solve_linear(jacobian, rhs, step) : 0.0;
    if (determinant == 0.0 || !std::isfinite(determinant)) break;
    result.diagnostics.minimum_abs_jacobian_determinant = std::min(
        result.diagnostics.minimum_abs_jacobian_determinant,
        std::abs(determinant));
    bool accepted = false;
    double scale = 1.0;
    for (int line = 0; line < 18; ++line) {
      VectorN trial = unknown;
      for (std::size_t i = 0; i < ND; ++i) trial[i] += scale * step[i];
      Candidate trial_candidate = evaluate(trial);
      if (trial_candidate.valid
          && infinity_norm(trial_candidate.residual) < residual) {
        VectorN change{};
        for (std::size_t i = 0; i < ND; ++i)
          change[i] = trial[i] - unknown[i];
        result.diagnostics.step_residual = infinity_norm(change);
        unknown = trial;
        current = std::move(trial_candidate);
        accepted = true;
        break;
      }
      scale *= 0.5;
      ++result.diagnostics.rejected_steps;
    }
    if (!accepted) break;
  }
  if (!std::isfinite(result.diagnostics.minimum_abs_jacobian_determinant))
    result.diagnostics.minimum_abs_jacobian_determinant = 0.0;
  result.candidate = std::move(current);
  return result;
}

double aggregate_continuity_residual(
    const std::array<QuadraticCoatFaceCurrent, N>& segments, int L) {
  std::vector<double> rho0(volume(L), 0.0);
  std::vector<double> rho1(volume(L), 0.0);
  MatchedFaceFlux current(L);
  for (const auto& segment : segments) {
    add_density(rho0, segment.rho_before);
    add_density(rho1, segment.rho_after);
    add_current(current, segment, +1.0);
  }
  double result = 0.0;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int i = current.index(x, y, z);
        const double divergence = current.x[static_cast<std::size_t>(i)]
            - current.x[static_cast<std::size_t>(current.index(x - 1, y, z))]
            + current.y[static_cast<std::size_t>(i)]
            - current.y[static_cast<std::size_t>(current.index(x, y - 1, z))]
            + current.z[static_cast<std::size_t>(i)]
            - current.z[static_cast<std::size_t>(current.index(x, y, z - 1))];
        result = std::max(result, std::abs(
            rho1[static_cast<std::size_t>(i)]
            - rho0[static_cast<std::size_t>(i)] + divergence));
      }
  return result;
}

std::vector<double> aggregate_density(
    const std::array<QuadraticCoatFaceCurrent, N>& segments, bool later) {
  std::vector<double> result(segments[0].rho_before.size(), 0.0);
  for (const auto& segment : segments)
    add_density(result, later ? segment.rho_after : segment.rho_before);
  return result;
}

Vec3 composite_momentum(const ClosedNeutralTrimerPairState& state,
                        std::size_t group) {
  Vec3 result{};
  const std::size_t begin = group * NG;
  for (std::size_t a = begin; a < begin + NG; ++a)
    result += state.constituents[a].momentum;
  return result;
}

Vec3 composite_center(const ClosedNeutralTrimerPairState& state,
                      std::size_t group) {
  Vec3 result{};
  const std::size_t begin = group * NG;
  for (std::size_t a = begin; a < begin + NG; ++a)
    result += effective_position(state.constituents[a]);
  return result * (1.0 / static_cast<double>(NG));
}

Vec3 shortest_center_delta(const ClosedNeutralTrimerPairState& state) {
  const Vec3 a = composite_center(state, 0);
  const Vec3 b = composite_center(state, 1);
  const int L = state.electric.L;
  return {periodic_difference(b.x, a.x, L),
          periodic_difference(b.y, a.y, L),
          periodic_difference(b.z, a.z, L)};
}

void measure_internal_distances(ClosedNeutralTrimerPairStepResult& result) {
  result.minimum_internal_pair_distance = INFINITY;
  result.maximum_internal_pair_distance = 0.0;
  for (const auto* state : {&result.earlier, &result.later}) {
    const auto x = positions(*state);
    for (std::size_t group = 0; group < 2; ++group) {
      const std::size_t begin = group * NG;
      for (std::size_t a = begin; a < begin + NG; ++a)
        for (std::size_t b = a + 1; b < begin + NG; ++b) {
          const double distance = (x[a] - x[b]).mag();
          result.minimum_internal_pair_distance = std::min(
              result.minimum_internal_pair_distance, distance);
          result.maximum_internal_pair_distance = std::max(
              result.maximum_internal_pair_distance, distance);
        }
    }
  }
}

ClosedNeutralTrimerPairStepResult finalize(
    const RootResult& root, bool forward,
    const ClosedNeutralPairOptions& options,
    const FaceFluxNormalization& normalization) {
  ClosedNeutralTrimerPairStepResult result;
  result.forward = forward;
  result.solve = root.diagnostics;
  result.normalization = normalization;
  result.interaction_scale = normalization.mapped_field_work_coefficient;
  if (!root.candidate.valid) return result;
  const Candidate& candidate = root.candidate;
  result.earlier = candidate.earlier;
  result.later = candidate.later;
  result.segments = candidate.segments;
  result.gathers = candidate.gathers;
  result.velocities = candidate.velocities;
  result.electric_impulses = candidate.electric_impulses;
  result.magnetic_impulses = candidate.magnetic_impulses;
  result.binding_impulses = candidate.binding_impulses;
  result.total_impulses = candidate.total_impulses;
  for (int charge : result.earlier.charges) result.net_charge += charge;
  result.site_projection_valid = site_projection_valid(result.earlier)
      && site_projection_valid(result.later);
  result.root_residual = root.diagnostics.residual;
  result.force_residual = infinity_norm(candidate.residual);
  result.continuity_residual = aggregate_continuity_residual(
      result.segments, result.earlier.electric.L);
  const auto density0 = aggregate_density(result.segments, false);
  const auto density1 = aggregate_density(result.segments, true);
  result.gauss_before_residual = max_fractional_gauss_residual(
      result.earlier.electric, density0);
  result.gauss_after_residual = max_fractional_gauss_residual(
      result.later.electric, density1);

  const double lambda = options.wave_speed * options.dt;
  result.field_energy_before = result.interaction_scale
      * matched_modified_energy(result.earlier.electric,
          result.earlier.magnetic_half, lambda);
  result.field_energy_after = result.interaction_scale
      * matched_modified_energy(result.later.electric,
          result.later.magnetic_half, lambda);
  for (std::size_t a = 0; a < N; ++a) {
    const Vec3 p0 = result.earlier.constituents[a].momentum;
    const Vec3 p1 = result.later.constituents[a].momentum;
    const double h0 = production_flat_energy_from_momentum(p0);
    const double h1 = production_flat_energy_from_momentum(p1);
    result.kinetic_energy_before += h0;
    result.kinetic_energy_after += h1;
    result.kinetic_discrete_gradient_residual = std::max(
        result.kinetic_discrete_gradient_residual,
        std::abs((h1 - h0) - result.velocities[a].dot(p1 - p0)));
    result.kinematic_residual = std::max(result.kinematic_residual,
        maximum_component(result.segments[a].end_effective_position
          - result.segments[a].start_effective_position
          - result.velocities[a] * options.dt));
    result.electric_adjoint_residual = std::max(
        result.electric_adjoint_residual,
        std::abs(result.gathers[a].electric_adjoint_residual));
    result.magnetic_work_residual += result.velocities[a].dot(
        result.magnetic_impulses[a]);
    result.current_work += result.interaction_scale
        * result.gathers[a].current_work;
    result.causal_speed_excess = std::max(result.causal_speed_excess,
        std::max(0.0, result.velocities[a].mag() - C_SPEED));
  }
  result.magnetic_work_residual = std::abs(result.magnetic_work_residual);
  result.binding_energy_before = binding_energy(
      positions(result.earlier), options);
  result.binding_energy_after = binding_energy(
      positions(result.later), options);
  double binding_work = 0.0;
  for (std::size_t group = 0; group < 2; ++group) {
    Vec3 binding_sum{};
    const std::size_t begin = group * NG;
    for (std::size_t a = begin; a < begin + NG; ++a) {
      binding_sum += result.binding_impulses[a];
      binding_work += result.velocities[a].dot(result.binding_impulses[a]);
    }
    result.binding_impulse_sum_residual = std::max(
        result.binding_impulse_sum_residual, binding_sum.mag());
  }
  result.binding_work_residual = std::abs(
      result.binding_energy_after - result.binding_energy_before
      + binding_work);
  const double matter0 = result.kinetic_energy_before
      + result.binding_energy_before;
  const double matter1 = result.kinetic_energy_after
      + result.binding_energy_after;
  result.matter_work_residual = std::abs(
      matter1 - matter0 - result.current_work);
  result.field_work_residual = std::abs(
      result.field_energy_after - result.field_energy_before
      + result.current_work);
  result.total_energy_residual = std::abs(
      matter1 + result.field_energy_after
      - matter0 - result.field_energy_before);

  for (std::size_t group = 0; group < 2; ++group) {
    result.composite_momentum_before[group] = composite_momentum(
        result.earlier, group);
    result.composite_momentum_after[group] = composite_momentum(
        result.later, group);
  }
  result.matter_momentum_before = result.composite_momentum_before[0]
      + result.composite_momentum_before[1];
  result.matter_momentum_after = result.composite_momentum_after[0]
      + result.composite_momentum_after[1];
  result.field_pseudomomentum_before = matched_local_translation_momentum(
      result.earlier.electric, result.earlier.magnetic_half)
      * result.interaction_scale;
  result.field_pseudomomentum_after = matched_local_translation_momentum(
      result.later.electric, result.later.magnetic_half)
      * result.interaction_scale;
  result.total_pseudomomentum_before = result.matter_momentum_before
      + result.field_pseudomomentum_before;
  result.total_pseudomomentum_after = result.matter_momentum_after
      + result.field_pseudomomentum_after;
  result.pseudomomentum_defect = result.total_pseudomomentum_after
      - result.total_pseudomomentum_before;
  result.pseudomomentum_defect_norm = result.pseudomomentum_defect.mag();

  const Vec3 center_delta0 = shortest_center_delta(result.earlier);
  const Vec3 center_delta1 = shortest_center_delta(result.later);
  result.center_separation_before = center_delta0.mag();
  result.center_separation_after = center_delta1.mag();
  if (result.center_separation_before > 0.0) {
    const Vec3 direction = center_delta0
        * (1.0 / result.center_separation_before);
    const Vec3 delta_pa = result.composite_momentum_after[0]
        - result.composite_momentum_before[0];
    const Vec3 delta_pb = result.composite_momentum_after[1]
        - result.composite_momentum_before[1];
    result.inward_impulse = 0.5
        * (delta_pa.dot(direction) - delta_pb.dot(direction));
  }
  measure_internal_distances(result);

  const bool finite_values = std::isfinite(result.root_residual)
      && std::isfinite(result.continuity_residual)
      && std::isfinite(result.gauss_before_residual)
      && std::isfinite(result.gauss_after_residual)
      && std::isfinite(result.total_energy_residual)
      && std::isfinite(result.pseudomomentum_defect_norm)
      && std::isfinite(result.inward_impulse)
      && std::isfinite(result.minimum_internal_pair_distance)
      && std::isfinite(result.maximum_internal_pair_distance);
  result.valid = result.solve.converged && finite_values
      && result.site_projection_valid && result.net_charge == 0;
  const double gate = options.gate_tolerance;
  result.common_action_gates_pass = result.valid
      && result.root_residual <= gate
      && result.continuity_residual <= gate
      && result.gauss_before_residual <= gate
      && result.gauss_after_residual <= gate
      && result.force_residual <= gate
      && result.kinematic_residual <= gate
      && result.kinetic_discrete_gradient_residual <= gate
      && result.electric_adjoint_residual <= gate
      && result.magnetic_work_residual <= gate
      && result.binding_work_residual <= gate
      && result.binding_impulse_sum_residual <= gate
      && result.matter_work_residual <= gate
      && result.field_work_residual <= gate
      && result.total_energy_residual <= gate
      && result.causal_speed_excess <= gate;
  result.isolated_momentum_gate_pass = result.valid
      && result.pseudomomentum_defect_norm <= gate;
  return result;
}

}  // namespace

double closed_neutral_pair_binding_energy(
    const ClosedNeutralTrimerPairState& state,
    const ClosedNeutralPairOptions& options) {
  return binding_energy(positions(state), options);
}

ClosedNeutralTrimerPairStepResult solve_closed_neutral_pair_forward(
    const ClosedNeutralTrimerPairState& earlier,
    const ClosedNeutralPairOptions& options) {
  const FaceFluxNormalization normalization = measure_face_flux_normalization();
  if (!normalization.valid
      || !(normalization.mapped_field_work_coefficient > 0.0)
      || !state_valid(earlier, options))
    return ClosedNeutralTrimerPairStepResult(0);
  const double lambda = options.wave_speed * options.dt;
  const PreparedForwardFields prepared = prepare_forward_fields(earlier, lambda);
  const auto evaluate = [&](const VectorN& unknown) {
    return evaluate_forward(earlier, options, prepared,
        normalization.mapped_field_work_coefficient, unknown);
  };
  const RootResult root = solve_root(earlier.electric.L,
      flatten_momenta(earlier.constituents), options, evaluate);
  return finalize(root, true, options, normalization);
}

ClosedNeutralTrimerPairStepResult solve_closed_neutral_pair_reverse(
    const ClosedNeutralTrimerPairState& later,
    const ClosedNeutralPairOptions& options) {
  const FaceFluxNormalization normalization = measure_face_flux_normalization();
  if (!normalization.valid
      || !(normalization.mapped_field_work_coefficient > 0.0)
      || !state_valid(later, options))
    return ClosedNeutralTrimerPairStepResult(0);
  const auto evaluate = [&](const VectorN& unknown) {
    return evaluate_reverse(later, options,
        normalization.mapped_field_work_coefficient, unknown);
  };
  const RootResult root = solve_root(later.electric.L,
      flatten_momenta(later.constituents), options, evaluate);
  return finalize(root, false, options, normalization);
}

double closed_neutral_pair_state_max_difference(
    const ClosedNeutralTrimerPairState& lhs,
    const ClosedNeutralTrimerPairState& rhs) {
  if (lhs.electric.L <= 0 || lhs.electric.L != rhs.electric.L
      || lhs.magnetic_half.L != rhs.magnetic_half.L) return INFINITY;
  double result = std::max(
      matched_face_max_difference(lhs.electric, rhs.electric),
      matched_edge_max_difference(lhs.magnetic_half, rhs.magnetic_half));
  const int L = lhs.electric.L;
  for (std::size_t a = 0; a < N; ++a) {
    if (lhs.charges[a] != rhs.charges[a]) return INFINITY;
    const Vec3 xl = effective_position(lhs.constituents[a]);
    const Vec3 xr = effective_position(rhs.constituents[a]);
    result = std::max({result,
        std::abs(periodic_difference(xl.x, xr.x, L)),
        std::abs(periodic_difference(xl.y, xr.y, L)),
        std::abs(periodic_difference(xl.z, xr.z, L)),
        maximum_component(lhs.constituents[a].momentum
            - rhs.constituents[a].momentum)});
  }
  return result;
}

}  // namespace ftd::eft
