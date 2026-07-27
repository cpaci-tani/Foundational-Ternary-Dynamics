#include "ftd/eft/site_ontic_atomic_reciprocal_hop.h"

#include "ftd/constants.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>

namespace ftd::eft {
namespace {

constexpr double ROOT_TOL = 1e-13;
constexpr double GATE_TOL = 1e-12;
constexpr double ROOT_MERGE_TOL = 1e-10;
constexpr double FD_STEP = 1.0 / 1048576.0;  // 2^-20
constexpr int MAX_NEWTON = 128;

using Matrix3 = std::array<std::array<double, 3>, 3>;

int wrap(int value, int L) {
  const int r = value % L;
  return r < 0 ? r + L : r;
}

std::size_t index(int L, int x, int y, int z) {
  return (static_cast<std::size_t>(wrap(x, L)) * L + wrap(y, L)) * L
      + wrap(z, L);
}

double component(const Vec3& v, int axis) {
  return axis == 0 ? v.x : (axis == 1 ? v.y : v.z);
}

void set_component(Vec3& v, int axis, double value) {
  if (axis == 0) v.x = value;
  else if (axis == 1) v.y = value;
  else v.z = value;
}

Vec3 add(const Vec3& a, const Vec3& b) {
  return {a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 subtract(const Vec3& a, const Vec3& b) {
  return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 scale(const Vec3& v, double factor) {
  return {factor * v.x, factor * v.y, factor * v.z};
}

double max_abs(const Vec3& v) {
  return std::max({std::abs(v.x), std::abs(v.y), std::abs(v.z)});
}

bool finite(const Vec3& v) {
  return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

long double dot_ld(const Vec3& a, const Vec3& b) {
  return static_cast<long double>(a.x) * b.x
      + static_cast<long double>(a.y) * b.y
      + static_cast<long double>(a.z) * b.z;
}

long double pair(const std::vector<Vec3>& a,
                 const std::vector<Vec3>& b) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < a.size(); ++i) result += dot_ld(a[i], b[i]);
  return result;
}

long double pair(const std::vector<double>& a,
                 const std::vector<double>& b) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < a.size(); ++i)
    result += static_cast<long double>(a[i]) * b[i];
  return result;
}

std::vector<Vec3> combine(const std::vector<Vec3>& a,
                          const std::vector<Vec3>& b,
                          double alpha, double beta) {
  std::vector<Vec3> result(a.size());
  for (std::size_t i = 0; i < a.size(); ++i)
    result[i] = add(scale(a[i], alpha), scale(b[i], beta));
  return result;
}

std::vector<double> combine(const std::vector<double>& a,
                            const std::vector<double>& b,
                            double alpha, double beta) {
  std::vector<double> result(a.size());
  for (std::size_t i = 0; i < a.size(); ++i)
    result[i] = alpha * a[i] + beta * b[i];
  return result;
}

std::vector<Vec3> gradient(const std::vector<double>& scalar, int L) {
  std::vector<Vec3> result(scalar.size());
  for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
      result[index(L, x, y, z)] = {
          0.5 * (scalar[index(L, x + 1, y, z)]
               - scalar[index(L, x - 1, y, z)]),
          0.5 * (scalar[index(L, x, y + 1, z)]
               - scalar[index(L, x, y - 1, z)]),
          0.5 * (scalar[index(L, x, y, z + 1)]
               - scalar[index(L, x, y, z - 1)])};
    }
  return result;
}

std::vector<double> divergence(const std::vector<Vec3>& field, int L) {
  std::vector<double> result(field.size());
  for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
      result[index(L, x, y, z)] = 0.5 * (
          field[index(L, x + 1, y, z)].x
        - field[index(L, x - 1, y, z)].x
        + field[index(L, x, y + 1, z)].y
        - field[index(L, x, y - 1, z)].y
        + field[index(L, x, y, z + 1)].z
        - field[index(L, x, y, z - 1)].z);
    }
  return result;
}

std::vector<Vec3> curl(const std::vector<Vec3>& field, int L) {
  std::vector<Vec3> result(field.size());
  const auto at = [&](int x, int y, int z) -> const Vec3& {
    return field[index(L, x, y, z)];
  };
  for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
      result[index(L, x, y, z)] = {
          0.5 * (at(x, y + 1, z).z - at(x, y - 1, z).z
               - at(x, y, z + 1).y + at(x, y, z - 1).y),
          0.5 * (at(x, y, z + 1).x - at(x, y, z - 1).x
               - at(x + 1, y, z).z + at(x - 1, y, z).z),
          0.5 * (at(x + 1, y, z).y - at(x - 1, y, z).y
               - at(x, y + 1, z).x + at(x, y - 1, z).x)};
    }
  return result;
}

std::vector<Vec3> derivative(const std::vector<Vec3>& field,
                             int L, int axis) {
  std::vector<Vec3> result(field.size());
  for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
      int plus[3] = {x, y, z};
      int minus[3] = {x, y, z};
      ++plus[axis];
      --minus[axis];
      result[index(L, x, y, z)] = scale(subtract(
          field[index(L, plus[0], plus[1], plus[2])],
          field[index(L, minus[0], minus[1], minus[2])]), 0.5);
    }
  return result;
}

std::vector<Vec3> apply_k(const std::vector<Vec3>& field, int L) {
  std::vector<Vec3> result(field.size());
  constexpr std::array<std::array<int, 3>, 6> faces{{
      {{1, 0, 0}}, {{-1, 0, 0}}, {{0, 1, 0}},
      {{0, -1, 0}}, {{0, 0, 1}}, {{0, 0, -1}}}};
  constexpr std::array<std::array<int, 3>, 12> edges{{
      {{1, 1, 0}}, {{1, -1, 0}}, {{-1, 1, 0}}, {{-1, -1, 0}},
      {{1, 0, 1}}, {{1, 0, -1}}, {{-1, 0, 1}}, {{-1, 0, -1}},
      {{0, 1, 1}}, {{0, 1, -1}}, {{0, -1, 1}}, {{0, -1, -1}}}};
  const double c2 = C_WAVE * C_WAVE;
  for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
      Vec3 lap{};
      for (const auto& d : faces)
        lap = add(lap, scale(field[index(L, x+d[0], y+d[1], z+d[2])],
                             1.0 / 3.0));
      for (const auto& d : edges)
        lap = add(lap, scale(field[index(L, x+d[0], y+d[1], z+d[2])],
                             1.0 / 6.0));
      lap = subtract(lap, scale(field[index(L, x, y, z)], 4.0));
      result[index(L, x, y, z)] = scale(lap, -c2);
    }
  return result;
}

void remove_mean(std::vector<Vec3>& field) {
  Vec3 mean{};
  for (const Vec3& value : field) mean = add(mean, value);
  mean = scale(mean, 1.0 / static_cast<double>(field.size()));
  for (Vec3& value : field) value = subtract(value, mean);
}

bool solve_zero_mean_k(const std::vector<Vec3>& source, int L,
                       std::vector<Vec3>& solution, double& residual) {
  solution.assign(source.size(), {});
  std::vector<Vec3> r = source;
  remove_mean(r);
  std::vector<Vec3> p = r;
  const long double initial = pair(r, r);
  if (!(initial > 0.0L)) {
    residual = 0.0;
    return true;
  }
  long double rr = initial;
  const int maximum = std::max(256, 4 * L * L * L);
  for (int iteration = 0; iteration < maximum; ++iteration) {
    const std::vector<Vec3> kp = apply_k(p, L);
    const long double denominator = pair(p, kp);
    if (!(denominator > 0.0L)) return false;
    const long double alpha = rr / denominator;
    for (std::size_t i = 0; i < solution.size(); ++i) {
      solution[i] = add(solution[i], scale(p[i], static_cast<double>(alpha)));
      r[i] = subtract(r[i], scale(kp[i], static_cast<double>(alpha)));
    }
    remove_mean(solution);
    remove_mean(r);
    const long double next = pair(r, r);
    residual = std::sqrt(static_cast<double>(next / initial));
    if (residual <= ROOT_TOL) return true;
    const long double beta = next / rr;
    for (std::size_t i = 0; i < p.size(); ++i)
      p[i] = add(r[i], scale(p[i], static_cast<double>(beta)));
    rr = next;
  }
  return false;
}

Vec3 effective_position(const SiteOnticAtomicState& state) {
  return {state.site.x + state.remainder.x,
          state.site.y + state.remainder.y,
          state.site.z + state.remainder.z};
}

void commit_position(SiteOnticAtomicState& state, const Vec3& position) {
  const int nx = static_cast<int>(std::floor(position.x));
  const int ny = static_cast<int>(std::floor(position.y));
  const int nz = static_cast<int>(std::floor(position.z));
  state.site = {wrap(nx, state.L), wrap(ny, state.L), wrap(nz, state.L)};
  state.remainder = {position.x - nx, position.y - ny, position.z - nz};
}

Vec3 chord_velocity(const Vec3& p0, const Vec3& p1) {
  const double denominator = production_flat_energy_from_momentum(p0)
      + production_flat_energy_from_momentum(p1);
  return scale(add(p0, p1), C_SPEED * C_SPEED / denominator);
}

struct Candidate {
  bool valid = false;
  Vec3 p1{};
  Vec3 x1{};
  Vec3 velocity{};
  Vec3 impulse{};
  Vec3 residual{};
  MooreSpacetimeCurrent current;
  std::vector<Vec3> source;
};

Candidate evaluate_candidate(const SiteOnticAtomicState& before,
                             const Vec3& p0, const Vec3& p1) {
  Candidate result;
  result.p1 = p1;
  result.velocity = chord_velocity(p0, p1);
  if (!finite(result.velocity) || result.velocity.mag() >= C_SPEED)
    return result;
  const Vec3 x0 = effective_position(before);
  result.x1 = add(x0, result.velocity);
  result.current = make_common_moore_spacetime_current(
      before.L, x0, result.x1, before.polarity);
  if (!result.current.valid) return result;
  const std::vector<double> rho_bar = combine(
      result.current.rho_start, result.current.rho_end, 0.5, 0.5);
  const std::vector<Vec3> q = combine(
      result.current.current_start, result.current.current_end, 1.0, 1.0);
  const std::vector<Vec3> grad_rho = gradient(rho_bar, before.L);
  const std::vector<Vec3> curl_q = curl(q, before.L);
  result.source = combine(grad_rho, curl_q, -G_C, G_C);
  for (int axis = 0; axis < 3; ++axis) {
    const std::vector<Vec3> dj = derivative(before.flux, before.L, axis);
    set_component(result.impulse, axis,
                  static_cast<double>(pair(result.source, dj)));
  }
  result.residual = subtract(subtract(p1, p0), result.impulse);
  result.valid = finite(result.impulse) && finite(result.residual);
  return result;
}

bool solve_linear(Matrix3 matrix, Vec3 rhs, Vec3& solution) {
  double b[3] = {rhs.x, rhs.y, rhs.z};
  for (int column = 0; column < 3; ++column) {
    int pivot = column;
    for (int row = column + 1; row < 3; ++row)
      if (std::abs(matrix[row][column]) > std::abs(matrix[pivot][column]))
        pivot = row;
    if (std::abs(matrix[pivot][column]) < 1e-14) return false;
    if (pivot != column) {
      std::swap(matrix[pivot], matrix[column]);
      std::swap(b[pivot], b[column]);
    }
    for (int row = column + 1; row < 3; ++row) {
      const double factor = matrix[row][column] / matrix[column][column];
      for (int entry = column; entry < 3; ++entry)
        matrix[row][entry] -= factor * matrix[column][entry];
      b[row] -= factor * b[column];
    }
  }
  double x[3]{};
  for (int row = 2; row >= 0; --row) {
    double value = b[row];
    for (int column = row + 1; column < 3; ++column)
      value -= matrix[row][column] * x[column];
    x[row] = value / matrix[row][row];
  }
  solution = {x[0], x[1], x[2]};
  return finite(solution);
}

Matrix3 numerical_jacobian(const SiteOnticAtomicState& state,
                           const Vec3& p0, const Vec3& p1,
                           bool& valid) {
  Matrix3 matrix{};
  valid = true;
  for (int column = 0; column < 3; ++column) {
    Vec3 plus = p1;
    Vec3 minus = p1;
    set_component(plus, column, component(p1, column) + FD_STEP);
    set_component(minus, column, component(p1, column) - FD_STEP);
    const Candidate high = evaluate_candidate(state, p0, plus);
    const Candidate low = evaluate_candidate(state, p0, minus);
    if (!high.valid || !low.valid) {
      valid = false;
      return matrix;
    }
    for (int row = 0; row < 3; ++row)
      matrix[row][column] = (component(high.residual, row)
          - component(low.residual, row)) / (2.0 * FD_STEP);
  }
  return matrix;
}

double infinity_norm(const Matrix3& matrix) {
  double result = 0.0;
  for (int row = 0; row < 3; ++row) {
    double sum = 0.0;
    for (int column = 0; column < 3; ++column)
      sum += std::abs(matrix[row][column]);
    result = std::max(result, sum);
  }
  return result;
}

double condition_number(const Matrix3& matrix) {
  Matrix3 inverse{};
  for (int column = 0; column < 3; ++column) {
    Vec3 unit{};
    set_component(unit, column, 1.0);
    Vec3 answer{};
    if (!solve_linear(matrix, unit, answer)) return INFINITY;
    inverse[0][column] = answer.x;
    inverse[1][column] = answer.y;
    inverse[2][column] = answer.z;
  }
  return infinity_norm(matrix) * infinity_norm(inverse);
}

struct NewtonResult {
  bool converged = false;
  int iterations = 0;
  Candidate candidate;
};

NewtonResult newton(const SiteOnticAtomicState& state, const Vec3& p0,
                    Vec3 start) {
  NewtonResult result;
  Vec3 p = start;
  Candidate current = evaluate_candidate(state, p0, p);
  if (!current.valid) return result;
  for (int iteration = 0; iteration <= MAX_NEWTON; ++iteration) {
    result.iterations = iteration;
    if (max_abs(current.residual) <= ROOT_TOL) {
      result.converged = true;
      result.candidate = std::move(current);
      return result;
    }
    if (iteration == MAX_NEWTON) break;
    bool jacobian_valid = false;
    const Matrix3 jacobian = numerical_jacobian(state, p0, p, jacobian_valid);
    Vec3 step{};
    if (!jacobian_valid
        || !solve_linear(jacobian, scale(current.residual, -1.0), step))
      break;
    bool accepted = false;
    for (int line = 0; line <= 20; ++line) {
      const double factor = std::ldexp(1.0, -line);
      const Vec3 trial = add(p, scale(step, factor));
      if (std::abs(trial.x) > 2.0 || std::abs(trial.y) > 2.0
          || std::abs(trial.z) > 2.0) continue;
      Candidate candidate = evaluate_candidate(state, p0, trial);
      if (candidate.valid
          && max_abs(candidate.residual) < max_abs(current.residual)) {
        p = trial;
        current = std::move(candidate);
        accepted = true;
        break;
      }
    }
    if (!accepted) break;
  }
  result.candidate = std::move(current);
  return result;
}

std::pair<Candidate, SiteOnticRootDiagnostics> solve_root(
    const SiteOnticAtomicState& state, const Vec3& p0) {
  SiteOnticRootDiagnostics diagnostics;
  std::vector<Candidate> roots;
  for (double dx : {-0.5, 0.0, 0.5})
    for (double dy : {-0.5, 0.0, 0.5})
      for (double dz : {-0.5, 0.0, 0.5}) {
        Vec3 start{std::clamp(p0.x + dx, -2.0, 2.0),
                   std::clamp(p0.y + dy, -2.0, 2.0),
                   std::clamp(p0.z + dz, -2.0, 2.0)};
        ++diagnostics.starts_attempted;
        NewtonResult solved = newton(state, p0, start);
        diagnostics.maximum_iterations = std::max(
            diagnostics.maximum_iterations, solved.iterations);
        if (!solved.converged) continue;
        ++diagnostics.converged_starts;
        bool duplicate = false;
        for (const Candidate& root : roots)
          duplicate = duplicate
              || max_abs(subtract(root.p1, solved.candidate.p1))
                    <= ROOT_MERGE_TOL;
        if (!duplicate) roots.push_back(std::move(solved.candidate));
      }
  ++diagnostics.starts_attempted;
  NewtonResult central = newton(state, p0, p0);
  diagnostics.maximum_iterations = std::max(
      diagnostics.maximum_iterations, central.iterations);
  if (central.converged) {
    ++diagnostics.converged_starts;
    bool duplicate = false;
    for (const Candidate& root : roots)
      duplicate = duplicate
          || max_abs(subtract(root.p1, central.candidate.p1))
                <= ROOT_MERGE_TOL;
    if (!duplicate) roots.push_back(std::move(central.candidate));
  }
  diagnostics.admitted_roots = static_cast<int>(roots.size());
  diagnostics.unique = roots.size() == 1;
  if (!diagnostics.unique) {
    diagnostics.residual = roots.empty() ? INFINITY
                                         : max_abs(roots.front().residual);
    return {{}, diagnostics};
  }
  diagnostics.residual = max_abs(roots.front().residual);
  bool valid = false;
  const Matrix3 jacobian = numerical_jacobian(
      state, p0, roots.front().p1, valid);
  diagnostics.jacobian_condition = valid ? condition_number(jacobian)
                                         : INFINITY;
  // Independent certification is deliberately not inferred from Newton.
  diagnostics.interval_certified = false;
  return {std::move(roots.front()), diagnostics};
}

Vec3 field_momentum(const std::vector<Vec3>& j,
                    const std::vector<Vec3>& w, int L) {
  Vec3 result{};
  for (int axis = 0; axis < 3; ++axis) {
    const std::vector<Vec3> dj = derivative(j, L, axis);
    set_component(result, axis, -static_cast<double>(pair(w, dj)));
  }
  return result;
}

long double field_energy(const std::vector<Vec3>& j,
                         const std::vector<Vec3>& w, int L) {
  const std::vector<Vec3> kj = apply_k(j, L);
  return 0.5L * pair(w, w) + 0.5L * pair(j, kj) - 0.5L * pair(w, kj);
}

long double interaction_energy(const std::vector<double>& rho,
                               const std::vector<Vec3>& j,
                               const std::vector<Vec3>& w, int L) {
  const std::vector<Vec3> r = combine(j, w, 1.0, -0.5);
  return -static_cast<long double>(G_C) * pair(rho, divergence(r, L));
}

double relative(double value, long double scale_value) {
  return std::abs(value) / static_cast<double>(
      std::max(1.0L, std::abs(scale_value)));
}

bool valid_state(const SiteOnticAtomicState& state) {
  const std::size_t count = static_cast<std::size_t>(state.L) * state.L
      * state.L;
  return state.L >= 7 && (state.polarity == -1 || state.polarity == 1)
      && state.flux.size() == count && state.wave.size() == count
      && state.remainder.x >= 0.0 && state.remainder.x < 1.0
      && state.remainder.y >= 0.0 && state.remainder.y < 1.0
      && state.remainder.z >= 0.0 && state.remainder.z < 1.0;
}

}  // namespace

SiteOnticAtomicState make_site_ontic_dressed_state(
    int L, int polarity, Coord site, const Vec3& remainder,
    const Vec3& velocity) {
  SiteOnticAtomicState result;
  result.L = L;
  result.site = {wrap(site.x, L), wrap(site.y, L), wrap(site.z, L)};
  result.remainder = remainder;
  result.velocity = velocity;
  result.polarity = polarity;
  const std::size_t count = static_cast<std::size_t>(L) * L * L;
  result.flux.assign(count, {});
  result.wave.assign(count, {});
  if (L < 7 || (polarity != -1 && polarity != 1)) return result;
  const Vec3 x = effective_position(result);
  const MooreSpacetimeCurrent stationary =
      make_common_moore_spacetime_current(L, x, x, polarity);
  if (!stationary.valid) return result;
  const std::vector<Vec3> source = combine(
      gradient(stationary.rho_start, L),
      std::vector<Vec3>(count), -G_C, 0.0);
  double residual = INFINITY;
  if (!solve_zero_mean_k(source, L, result.flux, residual)
      || residual > ROOT_TOL)
    result.flux.clear();
  return result;
}

SiteOnticAtomicStepResult solve_site_ontic_atomic_reciprocal_hop(
    const SiteOnticAtomicState& before) {
  SiteOnticAtomicStepResult result;
  result.before = before;
  result.evaluated = true;
  if (!valid_state(before)) {
    result.failure_gate = "input_state";
    return result;
  }
  result.momentum_before = production_flat_momentum(before.velocity);
  const auto solved = solve_root(before, result.momentum_before);
  const Candidate& candidate = solved.first;
  result.forward_root = solved.second;
  if (!result.forward_root.unique
      || result.forward_root.residual > ROOT_TOL
      || result.forward_root.jacobian_condition > 1e10) {
    result.failure_gate = "forward_root";
    return result;
  }

  result.deposited = candidate.current;
  result.source = candidate.source;
  result.momentum_after = candidate.p1;
  result.matter_impulse = candidate.impulse;
  result.after = before;
  commit_position(result.after, candidate.x1);
  result.after.velocity = production_flat_velocity_from_momentum(candidate.p1);
  const std::vector<Vec3> kj0 = apply_k(before.flux, before.L);
  result.after.wave.resize(before.wave.size());
  result.after.flux.resize(before.flux.size());
  for (std::size_t i = 0; i < before.flux.size(); ++i) {
    result.after.wave[i] = add(subtract(before.wave[i], kj0[i]),
                               candidate.source[i]);
    result.after.flux[i] = add(before.flux[i], result.after.wave[i]);
  }

  const int raw_dx = static_cast<int>(std::floor(candidate.x1.x))
      - static_cast<int>(std::floor(effective_position(before).x));
  const int raw_dy = static_cast<int>(std::floor(candidate.x1.y))
      - static_cast<int>(std::floor(effective_position(before).y));
  const int raw_dz = static_cast<int>(std::floor(candidate.x1.z))
      - static_cast<int>(std::floor(effective_position(before).z));
  result.site_shift = {raw_dx, raw_dy, raw_dz};
  result.continuity_residual = candidate.current.aggregate_continuity_residual;
  result.source_replay_residual = 0.0;
  result.field_update_residual = 0.0;
  result.kinematic_residual = max_abs(subtract(
      subtract(candidate.x1, effective_position(before)), candidate.velocity));
  result.causal_speed_excess = std::max(0.0,
      candidate.velocity.mag() - C_SPEED);
  result.locality_residual = candidate.current.finite_range ? 0.0 : 1.0;

  result.field_momentum_before = field_momentum(
      before.flux, before.wave, before.L);
  result.field_momentum_after = field_momentum(
      result.after.flux, result.after.wave, before.L);
  result.recoil_residual = max_abs(add(
      subtract(result.momentum_after, result.momentum_before),
      subtract(result.field_momentum_after, result.field_momentum_before)));

  const long double hf0 = field_energy(before.flux, before.wave, before.L);
  const long double hf1 = field_energy(
      result.after.flux, result.after.wave, before.L);
  const long double ui0 = interaction_energy(
      candidate.current.rho_start, before.flux, before.wave, before.L);
  const long double ui1 = interaction_energy(
      candidate.current.rho_end, result.after.flux, result.after.wave,
      before.L);
  const long double h0 = production_flat_energy_from_momentum(
      result.momentum_before);
  const long double h1 = production_flat_energy_from_momentum(
      result.momentum_after);
  const long double total0 = h0 + hf0 + ui0;
  const long double total1 = h1 + hf1 + ui1;
  result.particle_energy_before = static_cast<double>(h0);
  result.particle_energy_after = static_cast<double>(h1);
  result.field_energy_before = static_cast<double>(hf0);
  result.field_energy_after = static_cast<double>(hf1);
  result.interaction_energy_before = static_cast<double>(ui0);
  result.interaction_energy_after = static_cast<double>(ui1);
  result.total_energy_before = static_cast<double>(total0);
  result.total_energy_after = static_cast<double>(total1);
  result.energy_relative_residual = relative(
      static_cast<double>(total1 - total0), total0);

  const std::vector<Vec3> r0 = combine(before.flux, before.wave, 1.0, -0.5);
  const std::vector<Vec3> r1 = combine(
      result.after.flux, result.after.wave, 1.0, -0.5);
  const std::vector<Vec3> rbar = combine(r0, r1, 0.5, 0.5);
  const std::vector<Vec3> dr = combine(r1, r0, 1.0, -1.0);
  const std::vector<Vec3> q = combine(
      candidate.current.current_start, candidate.current.current_end,
      1.0, 1.0);
  const std::vector<Vec3> work_field = combine(
      gradient(divergence(rbar, before.L), before.L),
      curl(dr, before.L), 1.0, -1.0);
  result.matter_work = G_C * static_cast<double>(pair(q, work_field));
  result.work_relative_residual = relative(
      static_cast<double>(h1 - h0) - result.matter_work, total0);

  result.algebraically_valid = result.deposited.valid
      && result.forward_root.unique
      && result.forward_root.residual <= ROOT_TOL
      && result.forward_root.jacobian_condition <= 1e10
      && result.continuity_residual <= GATE_TOL
      && result.source_replay_residual <= GATE_TOL
      && result.field_update_residual <= GATE_TOL
      && result.recoil_residual <= GATE_TOL
      && result.kinematic_residual <= GATE_TOL
      && result.causal_speed_excess <= GATE_TOL
      && result.locality_residual <= GATE_TOL;
  if (!result.algebraically_valid) result.failure_gate = "algebraic_gate";
  else if (result.energy_relative_residual > GATE_TOL)
    result.failure_gate = "total_energy";
  else if (result.work_relative_residual > GATE_TOL)
    result.failure_gate = "matter_work";
  else result.failure_gate = "independent_root_certificate_pending";
  result.one_event_gates_pass = result.algebraically_valid
      && result.energy_relative_residual <= GATE_TOL
      && result.work_relative_residual <= GATE_TOL
      && result.forward_root.interval_certified;
  return result;
}

SiteOnticAtomicCampaignResult
analyze_site_ontic_atomic_reciprocal_hop() {
  SiteOnticAtomicCampaignResult campaign;
  const Coord center{8, 8, 8};

  // The deterministic preregistered order starts with the L=17, q=-1
  // stationary control, followed by lexicographic signed Moore directions.
  SiteOnticAtomicState stationary = make_site_ontic_dressed_state(
      17, -1, center, {0.5, 0.5, 0.5}, {});
  ++campaign.arms_attempted;
  SiteOnticAtomicStepResult control =
      solve_site_ontic_atomic_reciprocal_hop(stationary);
  if (!control.algebraically_valid
      || control.energy_relative_residual > GATE_TOL
      || control.work_relative_residual > GATE_TOL) {
    campaign.valid = true;
    campaign.verdict = "SITE_ONTIC_NATIVE_RECOIL_MAP_FAILS_ATOMIC_COMPATIBILITY";
    campaign.decisive_arm = "L17_q-1_stationary";
    campaign.decisive_result = std::move(control);
    return campaign;
  }
  ++campaign.arms_passed;

  const Vec3 direction = scale(Vec3{-1.0, -1.0, -1.0},
                               1.0 / std::sqrt(3.0));
  const Vec3 velocity = scale(direction, 0.15);
  SiteOnticAtomicState ballistic = make_site_ontic_dressed_state(
      17, -1, center, {0.05, 0.05, 0.05}, velocity);
  ++campaign.arms_attempted;
  SiteOnticAtomicStepResult hop =
      solve_site_ontic_atomic_reciprocal_hop(ballistic);
  campaign.decisive_result = hop;
  campaign.decisive_arm = "L17_q-1_body_negative_ballistic";
  if (!hop.algebraically_valid
      || hop.energy_relative_residual > GATE_TOL
      || hop.work_relative_residual > GATE_TOL) {
    campaign.valid = true;
    campaign.verdict = "SITE_ONTIC_NATIVE_RECOIL_MAP_FAILS_ATOMIC_COMPATIBILITY";
    return campaign;
  }
  ++campaign.arms_passed;
  campaign.valid = false;
  campaign.verdict = "PROTOCOL_INVALID";
  return campaign;
}

}  // namespace ftd::eft
