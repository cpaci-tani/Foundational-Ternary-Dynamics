#include "ftd/eft/symmetric_diagonal_coupled_endpoint.h"

#include "ftd/constants.h"
#include "ftd/eft/matched_face_energy_transaction.h"
#include "ftd/eft/ternary_collision_vertex.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace ftd::eft {
namespace {

Vec3 position(const ContactCarrierRecord& carrier) {
  return {static_cast<double>(carrier.anchor.x)+carrier.remainder.x,
          static_cast<double>(carrier.anchor.y)+carrier.remainder.y,
          static_cast<double>(carrier.anchor.z)+carrier.remainder.z};
}

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double energy(double momentum) {
  return std::sqrt(E_REST*E_REST
      + C_SPEED*C_SPEED*momentum*momentum);
}

double momentum_from_speed(double speed) {
  const double h = E_REST/std::sqrt(
      1.0-speed*speed/(C_SPEED*C_SPEED));
  return h*speed/(C_SPEED*C_SPEED);
}

MatchedFaceFlux current_field(const PiecewiseCurrentSignature& history) {
  MatchedFaceFlux result(history.L);
  result.x = history.current_x;
  result.y = history.current_y;
  result.z = history.current_z;
  return result;
}

void add_scaled(MatchedFaceFlux& target,
                const MatchedFaceFlux& value, double amount) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += amount*value.x[i];
    target.y[i] += amount*value.y[i];
    target.z[i] += amount*value.z[i];
  }
}

void scale(MatchedEdgeField& target, double amount) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] *= amount;
    target.y[i] *= amount;
    target.z[i] *= amount;
  }
}

double vector_residual(const std::vector<double>& lhs,
                       const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double residual = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    residual = std::max(residual, std::abs(lhs[i]-rhs[i]));
  return residual;
}

double gauss_residual(const MatchedFaceFlux& field,
                      const std::vector<double>& source) {
  if (source.size() != field.x.size()) return INFINITY;
  double residual = 0.0;
  for (int x = 0; x < field.L; ++x)
    for (int y = 0; y < field.L; ++y)
      for (int z = 0; z < field.L; ++z) {
        const int i = field.index(x, y, z);
        residual = std::max(residual, std::abs(
            divergence_at(field, x, y, z)
            - source[static_cast<std::size_t>(i)]));
      }
  return residual;
}

std::vector<double> divergence_field(const MatchedFaceFlux& field) {
  std::vector<double> result(field.x.size(), 0.0);
  for (int x = 0; x < field.L; ++x)
    for (int y = 0; y < field.L; ++y)
      for (int z = 0; z < field.L; ++z) {
        const int i = field.index(x, y, z);
        result[static_cast<std::size_t>(i)] =
            divergence_at(field, x, y, z);
      }
  return result;
}

struct Candidate {
  bool valid = false;
  double momentum = 0.0;
  double h = 0.0;
  double residual = NAN;
  double field_change = NAN;
  double work = NAN;
  double matter_change = NAN;
  double displacement_magnitude = 0.0;
  std::array<Vec3, 2> start{};
  std::array<Vec3, 2> end{};
  PiecewiseCurrentSignature history{};
  MatchedFaceFlux current;
  MatchedFaceFlux electric_after;

  explicit Candidate(int L = 0) : current(L), electric_after(L) {}
};

struct Problem {
  int L = 0;
  int polarity = 0;
  double p0 = 0.0;
  double h0 = 0.0;
  double beta = 0.0;
  std::array<Vec3, 2> start{};
  std::array<Vec3, 2> unit{};
  MatchedFaceFlux electric_before;

  explicit Problem(int size = 0) : L(size), electric_before(size) {}
};

Candidate evaluate(const Problem& problem, double p1) {
  Candidate result(problem.L);
  if (!(p1 >= 0.0) || !std::isfinite(p1)) return result;
  result.momentum = p1;
  result.h = energy(p1);
  const double displacement = C_SPEED*C_SPEED
      *(problem.p0+p1)/(problem.h0+result.h);
  result.displacement_magnitude = displacement;
  std::vector<PiecewiseWorldline> lines;
  lines.reserve(2);
  for (int i = 0; i < 2; ++i) {
    result.start[static_cast<std::size_t>(i)] =
        problem.start[static_cast<std::size_t>(i)];
    result.end[static_cast<std::size_t>(i)] =
        result.start[static_cast<std::size_t>(i)]
        + problem.unit[static_cast<std::size_t>(i)]*displacement;
    lines.push_back({problem.polarity,
        {result.start[static_cast<std::size_t>(i)],
         result.end[static_cast<std::size_t>(i)]}});
  }
  result.history = make_piecewise_current_signature(problem.L, lines);
  if (!result.history.valid) return result;
  result.current = current_field(result.history);
  result.electric_after = problem.electric_before;
  add_scaled(result.electric_after, result.current, -1.0);
  result.field_change = problem.beta*(
      quadratic_energy(result.electric_after)
      - quadratic_energy(problem.electric_before));
  MatchedFaceFlux midpoint = problem.electric_before;
  add_scaled(midpoint, result.electric_after, 1.0);
  for (std::size_t i = 0; i < midpoint.x.size(); ++i) {
    midpoint.x[i] *= 0.5;
    midpoint.y[i] *= 0.5;
    midpoint.z[i] *= 0.5;
  }
  result.work = problem.beta*static_cast<double>(
      matched_face_dot(result.current, midpoint));
  result.matter_change = 2.0*(result.h-problem.h0);
  result.residual = result.matter_change+result.field_change;
  result.valid = std::isfinite(result.residual)
      && std::isfinite(result.work)
      && result.history.continuity_residual <= 1e-10;
  return result;
}

double inverse_residual(const Candidate& forward,
                        const Problem& problem) {
  std::vector<PiecewiseWorldline> reverse_lines;
  for (int i = 0; i < 2; ++i) {
    reverse_lines.push_back({problem.polarity,
        {forward.end[static_cast<std::size_t>(i)],
         forward.start[static_cast<std::size_t>(i)]}});
  }
  const PiecewiseCurrentSignature reverse =
      make_piecewise_current_signature(problem.L, reverse_lines);
  if (!reverse.valid) return INFINITY;
  const MatchedFaceFlux reverse_current = current_field(reverse);
  MatchedFaceFlux current_sum = forward.current;
  add_scaled(current_sum, reverse_current, 1.0);
  MatchedFaceFlux restored = forward.electric_after;
  add_scaled(restored, reverse_current, -1.0);
  return std::max({
      matched_face_max_difference(
          current_sum, MatchedFaceFlux(problem.L)),
      matched_face_max_difference(restored, problem.electric_before),
      vector_residual(reverse.rho_before, forward.history.rho_after),
      vector_residual(reverse.rho_after, forward.history.rho_before),
      std::abs(2.0*(problem.h0-forward.h)
               - (-forward.matter_change))});
}

}  // namespace

SymmetricDiagonalCoupledEndpointResult
solve_symmetric_diagonal_coupled_endpoint(
    int L, const Vec3& contact_position, Coord diagonal_direction,
    int polarity, double speed, double tolerance) {
  SymmetricDiagonalCoupledEndpointResult result;
  result.shell = diagonal_direction.x*diagonal_direction.x
      + diagonal_direction.y*diagonal_direction.y
      + diagonal_direction.z*diagonal_direction.z;
  result.rebase = analyze_overshoot_preserving_contact_rebase(
      L, contact_position, diagonal_direction, polarity, speed, tolerance);
  result.normalization = measure_face_flux_normalization();
  result.interaction_scale =
      result.normalization.mapped_field_work_coefficient;
  if ((result.shell != 2 && result.shell != 3)
      || !result.rebase.valid || !result.normalization.valid
      || !(speed > 0.0) || !(speed < C_SPEED)
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;

  Problem problem(L);
  problem.polarity = polarity;
  problem.p0 = momentum_from_speed(speed);
  problem.h0 = energy(problem.p0);
  problem.beta = result.interaction_scale;
  for (int i = 0; i < 2; ++i) {
    const auto& carrier = result.rebase.bounce_preimage.carrier[
        static_cast<std::size_t>(i)];
    problem.start[static_cast<std::size_t>(i)] = position(carrier);
    problem.unit[static_cast<std::size_t>(i)] =
        carrier.velocity*(1.0/speed);
  }
  const Candidate reference = evaluate(problem, problem.p0);
  if (!reference.valid) return result;
  const MatchedEdgeField reference_curl =
      matched_curl_adjoint(reference.current);
  result.reference_transverse_norm_squared = static_cast<double>(
      matched_edge_dot(reference_curl, reference_curl));
  const MatchedFaceFlux challenge = matched_curl(reference_curl);
  problem.electric_before = reference.current;
  for (std::size_t i = 0; i < problem.electric_before.x.size(); ++i) {
    problem.electric_before.x[i] *= 0.5;
    problem.electric_before.y[i] *= 0.5;
    problem.electric_before.z[i] *= 0.5;
  }
  add_scaled(problem.electric_before, challenge, 0.125);

  const Candidate low = evaluate(problem, problem.p0);
  const double vmax = 0.95*C_SPEED;
  const double pmax = momentum_from_speed(vmax);
  const Candidate high = evaluate(problem, pmax);
  if (!low.valid || !high.valid) return result;
  result.root_bracketed = low.residual < 0.0 && high.residual > 0.0;
  if (!result.root_bracketed) return result;

  result.monotonic_on_locked_grid = true;
  result.minimum_monotonic_increment = INFINITY;
  double previous = low.residual;
  for (int i = 1; i <= 64; ++i) {
    const double p = problem.p0
        +(pmax-problem.p0)*static_cast<double>(i)/64.0;
    const Candidate sample = evaluate(problem, p);
    if (!sample.valid) return result;
    const double increment = sample.residual-previous;
    result.minimum_monotonic_increment = std::min(
        result.minimum_monotonic_increment, increment);
    result.monotonic_on_locked_grid =
        result.monotonic_on_locked_grid && increment > 0.0;
    previous = sample.residual;
  }

  double lo = problem.p0;
  double hi = pmax;
  Candidate root(L);
  for (int iteration = 1; iteration <= 128; ++iteration) {
    const double mid = 0.5*(lo+hi);
    root = evaluate(problem, mid);
    result.iterations = iteration;
    if (!root.valid) return result;
    if (std::abs(root.residual) <= tolerance) {
      result.converged = true;
      break;
    }
    if (root.residual > 0.0) hi = mid;
    else lo = mid;
  }
  if (!result.converged) return result;

  result.momentum_before = problem.p0;
  result.momentum_after = root.momentum;
  result.momentum_change = root.momentum-problem.p0;
  result.energy_before_per_carrier = problem.h0;
  result.energy_after_per_carrier = root.h;
  result.displacement_magnitude = root.displacement_magnitude;
  result.reference_displacement_magnitude = reference.displacement_magnitude;
  result.endpoint_change = std::abs(
      root.displacement_magnitude-reference.displacement_magnitude);
  result.speed = C_SPEED*C_SPEED*(problem.p0+root.momentum)
      /(problem.h0+root.h);
  result.root_residual = std::abs(root.residual);
  result.continuity_residual = root.history.continuity_residual;

  const std::vector<double> div_before = divergence_field(
      problem.electric_before);
  std::vector<double> stationary = div_before;
  for (std::size_t i = 0; i < stationary.size(); ++i)
    stationary[i] -= root.history.rho_before[i];
  std::vector<double> source_after = stationary;
  for (std::size_t i = 0; i < source_after.size(); ++i)
    source_after[i] += root.history.rho_after[i];
  result.gauss_before_residual = gauss_residual(
      problem.electric_before, div_before);
  result.gauss_after_residual = gauss_residual(
      root.electric_after, source_after);

  MatchedEdgeField magnetic_before =
      matched_curl_adjoint(problem.electric_before);
  scale(magnetic_before, C_SPEED);
  MatchedEdgeField magnetic_half = magnetic_before;
  const MatchedEdgeField electric_curl =
      matched_curl_adjoint(problem.electric_before);
  for (std::size_t i = 0; i < magnetic_half.x.size(); ++i) {
    magnetic_half.x[i] -= C_SPEED*electric_curl.x[i];
    magnetic_half.y[i] -= C_SPEED*electric_curl.y[i];
    magnetic_half.z[i] -= C_SPEED*electric_curl.z[i];
  }
  MatchedFaceFlux pre_current = problem.electric_before;
  add_scaled(pre_current, matched_curl(magnetic_half), C_SPEED);
  result.staggered_embedding_residual = std::max(
      matched_edge_max_difference(magnetic_half, MatchedEdgeField(L)),
      matched_face_max_difference(pre_current, problem.electric_before));
  result.field_work_residual = std::abs(root.field_change+root.work);
  result.matter_work_residual = std::abs(root.matter_change-root.work);
  result.total_energy_residual = std::abs(root.residual);

  double displacement_residual = 0.0;
  for (int i = 0; i < 2; ++i) {
    const Vec3 expected = problem.unit[static_cast<std::size_t>(i)]
        * root.displacement_magnitude;
    displacement_residual = std::max(displacement_residual,
        max_abs(root.end[static_cast<std::size_t>(i)]
                - root.start[static_cast<std::size_t>(i)]-expected));
  }
  result.displacement_residual = displacement_residual;
  result.causal_excess = std::max(0.0, result.speed-C_SPEED);
  result.inverse_residual = inverse_residual(root, problem);
  result.valid = result.root_bracketed && result.converged
      && result.monotonic_on_locked_grid
      && result.minimum_monotonic_increment > 0.0
      && result.momentum_change > 1e-8
      && result.endpoint_change > 0.0
      && result.root_residual <= tolerance
      && result.continuity_residual <= tolerance
      && result.gauss_before_residual <= tolerance
      && result.gauss_after_residual <= tolerance
      && result.staggered_embedding_residual <= tolerance
      && result.field_work_residual <= tolerance
      && result.matter_work_residual <= tolerance
      && result.total_energy_residual <= tolerance
      && result.displacement_residual <= tolerance
      && result.causal_excess <= tolerance
      && result.inverse_residual <= 1e-10;
  return result;
}

}  // namespace ftd::eft
