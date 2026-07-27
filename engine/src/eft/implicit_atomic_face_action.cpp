#include "ftd/eft/implicit_atomic_face_action.h"

#include "ftd/constants.h"
#include "ftd/eft/matched_face_energy_transaction.h"
#include "ftd/eft/spacetime_worldline_coupling.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

namespace ftd::eft {
namespace {

Vec3 position(const ContactCarrierRecord& carrier) {
  return {carrier.anchor.x+carrier.remainder.x,
          carrier.anchor.y+carrier.remainder.y,
          carrier.anchor.z+carrier.remainder.z};
}

void decompose(const Vec3& value, Coord& anchor, Vec3& remainder) {
  anchor = {static_cast<int>(std::floor(value.x)),
            static_cast<int>(std::floor(value.y)),
            static_cast<int>(std::floor(value.z))};
  remainder = {value.x-anchor.x, value.y-anchor.y, value.z-anchor.z};
}

double component(const Vec3& value, int axis) {
  if (axis == 0) return value.x;
  if (axis == 1) return value.y;
  return value.z;
}

Vec3 axis_vector(int axis, double amount) {
  if (axis == 0) return {amount, 0.0, 0.0};
  if (axis == 1) return {0.0, amount, 0.0};
  return {0.0, 0.0, amount};
}

double max_component(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

void add(MatchedFaceFlux& target, const MatchedFaceFlux& value) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += value.x[i];
    target.y[i] += value.y[i];
    target.z[i] += value.z[i];
  }
}

void add_scaled(MatchedFaceFlux& target,
                const MatchedFaceFlux& value, double amount) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += amount*value.x[i];
    target.y[i] += amount*value.y[i];
    target.z[i] += amount*value.z[i];
  }
}

void scale(MatchedFaceFlux& target, double amount) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] *= amount;
    target.y[i] *= amount;
    target.z[i] *= amount;
  }
}

double vector_residual(const std::vector<double>& lhs,
                       const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, std::abs(lhs[i]-rhs[i]));
  return result;
}

double energy(double momentum) {
  return std::sqrt(E_REST*E_REST
      +C_SPEED*C_SPEED*momentum*momentum);
}

double momentum_from_speed(double speed) {
  const double h = E_REST/std::sqrt(
      1.0-speed*speed/(C_SPEED*C_SPEED));
  return h*speed/(C_SPEED*C_SPEED);
}

double inherited_residual(
    const SymmetricDiagonalCoupledEndpointResult& value) {
  return std::max({value.root_residual, value.continuity_residual,
      value.gauss_before_residual, value.gauss_after_residual,
      value.staggered_embedding_residual, value.field_work_residual,
      value.matter_work_residual, value.total_energy_residual,
      value.displacement_residual, value.causal_excess,
      value.inverse_residual});
}

struct AggregateCurrent {
  bool valid = false;
  MatchedFaceFlux total;
  MatchedFaceFlux start;
  MatchedFaceFlux end;
  std::vector<double> rho_before;
  std::vector<double> rho_after;
  double split_residual = 0.0;
  double continuity_residual = 0.0;

  explicit AggregateCurrent(int L)
      : total(L), start(L), end(L),
        rho_before(static_cast<std::size_t>(L*L*L), 0.0),
        rho_after(static_cast<std::size_t>(L*L*L), 0.0) {}
};

AggregateCurrent make_current(
    int L, const OvershootPreservingContactRebaseResult& rebase,
    double speed, double displacement, double temporal_scale) {
  AggregateCurrent result(L);
  result.valid = true;
  for (const auto& carrier : rebase.bounce_preimage.carrier) {
    const Vec3 begin = position(carrier);
    const Vec3 unit = carrier.velocity*(1.0/speed);
    const Vec3 finish = begin+unit*displacement;
    Coord end_anchor{};
    Vec3 end_remainder{};
    decompose(finish, end_anchor, end_remainder);
    const auto current = make_spacetime_worldline_current(
        L, carrier.anchor, carrier.remainder,
        end_anchor, end_remainder, carrier.polarity, temporal_scale);
    if (!current.valid) {
      result.valid = false;
      return result;
    }
    MatchedFaceFlux total(L);
    total.x = current.spatial.current_x;
    total.y = current.spatial.current_y;
    total.z = current.spatial.current_z;
    add(result.total, total);
    add(result.start, current.spatial_start);
    add(result.end, current.spatial_end);
    for (std::size_t i = 0; i < result.rho_before.size(); ++i) {
      result.rho_before[i] += current.spatial.rho_before[i];
      result.rho_after[i] += current.spatial.rho_after[i];
    }
    result.split_residual = std::max({result.split_residual,
        current.spatial_split_residual,
        current.split_continuity_start_residual,
        current.split_continuity_end_residual});
    result.continuity_residual = std::max(
        result.continuity_residual, current.spatial.continuity_residual);
  }
  MatchedFaceFlux recombined = result.start;
  add(recombined, result.end);
  result.split_residual = std::max(result.split_residual,
      matched_face_max_difference(recombined, result.total));
  return result;
}

AggregateCurrent make_current(
    const std::array<Vec3, 2>& begin,
    const std::array<Vec3, 2>& finish,
    const std::array<int, 2>& charge,
    int L, double temporal_scale) {
  AggregateCurrent result(L);
  result.valid = true;
  for (int carrier = 0; carrier < 2; ++carrier) {
    Coord start_anchor{};
    Coord end_anchor{};
    Vec3 start_remainder{};
    Vec3 end_remainder{};
    decompose(begin[static_cast<std::size_t>(carrier)],
              start_anchor, start_remainder);
    decompose(finish[static_cast<std::size_t>(carrier)],
              end_anchor, end_remainder);
    const auto current = make_spacetime_worldline_current(
        L, start_anchor, start_remainder, end_anchor, end_remainder,
        charge[static_cast<std::size_t>(carrier)], temporal_scale);
    if (!current.valid) {
      result.valid = false;
      return result;
    }
    MatchedFaceFlux total(L);
    total.x = current.spatial.current_x;
    total.y = current.spatial.current_y;
    total.z = current.spatial.current_z;
    add(result.total, total);
    add(result.start, current.spatial_start);
    add(result.end, current.spatial_end);
    for (std::size_t i = 0; i < result.rho_before.size(); ++i) {
      result.rho_before[i] += current.spatial.rho_before[i];
      result.rho_after[i] += current.spatial.rho_after[i];
    }
    result.split_residual = std::max({result.split_residual,
        current.spatial_split_residual,
        current.split_continuity_start_residual,
        current.split_continuity_end_residual});
    result.continuity_residual = std::max(
        result.continuity_residual, current.spatial.continuity_residual);
  }
  MatchedFaceFlux recombined = result.start;
  add(recombined, result.end);
  result.split_residual = std::max(result.split_residual,
      matched_face_max_difference(recombined, result.total));
  return result;
}

double segment_interaction_action(
    const Vec3& begin, const Vec3& finish, int charge,
    const DualGaugePotentialSlab& slab, double coupling) {
  Coord start_anchor{};
  Coord end_anchor{};
  Vec3 start_remainder{};
  Vec3 end_remainder{};
  decompose(begin, start_anchor, start_remainder);
  decompose(finish, end_anchor, end_remainder);
  const auto current = make_spacetime_worldline_current(
      slab.L, start_anchor, start_remainder,
      end_anchor, end_remainder, charge, slab.temporal_scale);
  const std::vector<double> zero(
      static_cast<std::size_t>(slab.L*slab.L*slab.L), 0.0);
  const auto evaluated = evaluate_spacetime_gauge_coupling(
      current, slab, zero, zero, coupling);
  return evaluated.valid ? evaluated.interaction_action : NAN;
}

double five_point_endpoint_derivative(
    const Vec3& begin, const Vec3& finish, int charge,
    const DualGaugePotentialSlab& slab, double coupling,
    int endpoint, int axis, double h) {
  const Vec3 delta = axis_vector(axis, h);
  const auto action = [&](double multiple) {
    return endpoint == 0
        ? segment_interaction_action(
            begin+delta*multiple, finish, charge, slab, coupling)
        : segment_interaction_action(
            begin, finish+delta*multiple, charge, slab, coupling);
  };
  return (action(-2.0)-8.0*action(-1.0)
          +8.0*action(1.0)-action(2.0))/(12.0*h);
}

double one_sided_endpoint_derivative(
    const Vec3& begin, const Vec3& finish, int charge,
    const DualGaugePotentialSlab& slab, double coupling,
    int endpoint, int axis, double h, int side) {
  const Vec3 delta = axis_vector(axis, h*side);
  const auto action = [&](int multiple) {
    return endpoint == 0
        ? segment_interaction_action(
            begin+delta*static_cast<double>(multiple),
            finish, charge, slab, coupling)
        : segment_interaction_action(
            begin, finish+delta*static_cast<double>(multiple),
            charge, slab, coupling);
  };
  const double outward = (-25.0*action(0)+48.0*action(1)
      -36.0*action(2)+16.0*action(3)-3.0*action(4))/(12.0*h);
  return side*outward;
}

struct EndpointDerivatives {
  bool valid = false;
  Vec3 start{};
  Vec3 end{};
  double convergence = 0.0;
  double minimum_chart_clearance = INFINITY;
  double minimum_step = INFINITY;
};

double endpoint_chart_clearance(double value) {
  return std::abs(value-std::round(value));
}

EndpointDerivatives endpoint_derivatives(
    const Vec3& begin, const Vec3& finish, int charge,
    const DualGaugePotentialSlab& slab, double coupling, double h,
    bool chart_contained = false) {
  EndpointDerivatives result;
  const double minimum_clearance = std::ldexp(1.0, -30);
  for (int level = 0; level < 2; ++level) {
    Vec3 d1{};
    Vec3 d2{};
    for (int axis = 0; axis < 3; ++axis) {
      const double start_clearance = endpoint_chart_clearance(
          component(begin, axis));
      const double end_clearance = endpoint_chart_clearance(
          component(finish, axis));
      result.minimum_chart_clearance = std::min({
          result.minimum_chart_clearance,
          start_clearance, end_clearance});
      if (chart_contained
          && (!(start_clearance > minimum_clearance)
              || !(end_clearance > minimum_clearance))) return result;
      const double start_coarse = chart_contained
          ? std::min(h, start_clearance/4.0) : h;
      const double end_coarse = chart_contained
          ? std::min(h, end_clearance/4.0) : h;
      result.minimum_step = std::min({result.minimum_step,
          start_coarse, end_coarse});
      const double start_step = std::ldexp(start_coarse, -level);
      const double end_step = std::ldexp(end_coarse, -level);
      const double start = five_point_endpoint_derivative(
          begin, finish, charge, slab, coupling, 0, axis, start_step);
      const double end = five_point_endpoint_derivative(
          begin, finish, charge, slab, coupling, 1, axis, end_step);
      if (axis == 0) { d1.x = start; d2.x = end; }
      if (axis == 1) { d1.y = start; d2.y = end; }
      if (axis == 2) { d1.z = start; d2.z = end; }
    }
    if (!finite(d1) || !finite(d2)) return result;
    if (level == 0) {
      result.start = d1;
      result.end = d2;
    } else {
      result.convergence = std::max(
          max_component(d1-result.start), max_component(d2-result.end));
      result.start = d1;
      result.end = d2;
    }
  }
  result.valid = true;
  return result;
}

Vec3 interpolate_connection(const MatchedFaceFlux& potential,
                            const Vec3& point) {
  const int lower[3] = {
      static_cast<int>(std::floor(point.x)),
      static_cast<int>(std::floor(point.y)),
      static_cast<int>(std::floor(point.z))};
  const double fraction[3] = {
      point.x-lower[0], point.y-lower[1], point.z-lower[2]};
  Vec3 result{};
  for (int axis = 0; axis < 3; ++axis) {
    const int ta = (axis+1)%3;
    const int tb = (axis+2)%3;
    const std::vector<double>* field = axis == 0 ? &potential.x
        : (axis == 1 ? &potential.y : &potential.z);
    double value = 0.0;
    for (int ba = 0; ba <= 1; ++ba) {
      const double wa = ba == 0 ? 1.0-fraction[ta] : fraction[ta];
      for (int bb = 0; bb <= 1; ++bb) {
        const double wb = bb == 0 ? 1.0-fraction[tb] : fraction[tb];
        int c[3] = {lower[0], lower[1], lower[2]};
        c[ta] += ba;
        c[tb] += bb;
        value += wa*wb*(*field)[static_cast<std::size_t>(
            potential.index(c[0], c[1], c[2]))];
      }
    }
    if (axis == 0) result.x = value;
    if (axis == 1) result.y = value;
    if (axis == 2) result.z = value;
  }
  return result;
}

double transverse_residual(const Vec3& residual, const Vec3& unit) {
  return (residual-unit*residual.dot(unit)).mag();
}

double gauss_evolution_residual(
    const MatchedFaceFlux& before, const MatchedFaceFlux& after,
    const AggregateCurrent& current) {
  double result = 0.0;
  for (int x = 0; x < before.L; ++x) {
    for (int y = 0; y < before.L; ++y) {
      for (int z = 0; z < before.L; ++z) {
        const int i = before.index(x, y, z);
        result = std::max(result, std::abs(
            divergence_at(after, x, y, z)
            -divergence_at(before, x, y, z)
            -current.rho_after[static_cast<std::size_t>(i)]
            +current.rho_before[static_cast<std::size_t>(i)]));
      }
    }
  }
  return result;
}

}  // namespace

ImplicitAtomicFaceActionResult analyze_implicit_atomic_face_action(
    int L, const Vec3& contact_position, Coord diagonal_direction,
    int polarity, double speed, double derivative_step, double tolerance) {
  ImplicitAtomicFaceActionResult result;
  result.shell = diagonal_direction.x*diagonal_direction.x
      +diagonal_direction.y*diagonal_direction.y
      +diagonal_direction.z*diagonal_direction.z;
  if (L < 3 || (result.shell != 2 && result.shell != 3)
      || (polarity != -1 && polarity != 1)
      || !(speed > 0.0) || !(speed < C_SPEED)
      || !std::isfinite(derivative_step) || !(derivative_step > 0.0)
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;

  result.coupled = solve_symmetric_diagonal_coupled_endpoint(
      L, contact_position, diagonal_direction, polarity, speed, tolerance);
  const FaceFluxNormalization normalization =
      measure_face_flux_normalization();
  if (!result.coupled.valid || !normalization.valid) return result;
  result.carrier_count = 2;
  result.beta = normalization.mapped_field_work_coefficient;
  result.temporal_scale = C_SPEED;
  result.coupling = result.beta/result.temporal_scale;
  result.momentum_before = result.coupled.momentum_before;
  result.momentum_after = result.coupled.momentum_after;
  result.inherited_endpoint_residual = inherited_residual(result.coupled);

  const AggregateCurrent reference = make_current(
      L, result.coupled.rebase, speed,
      result.coupled.reference_displacement_magnitude,
      result.temporal_scale);
  const AggregateCurrent current = make_current(
      L, result.coupled.rebase, speed,
      result.coupled.displacement_magnitude,
      result.temporal_scale);
  if (!reference.valid || !current.valid) return result;
  result.current_split_residual = current.split_residual;
  result.continuity_residual = current.continuity_residual;

  MatchedFaceFlux electric_before = reference.total;
  scale(electric_before, 0.5);
  const MatchedFaceFlux challenge = matched_curl(
      matched_curl_adjoint(reference.total));
  add_scaled(electric_before, challenge, 0.125);
  MatchedFaceFlux potential_before = electric_before;
  scale(potential_before, result.temporal_scale);
  MatchedFaceFlux potential_after = potential_before;
  add_scaled(potential_after, electric_before, -result.temporal_scale);
  add_scaled(potential_after, current.start, result.temporal_scale);

  DualGaugePotentialSlab slab(L, result.temporal_scale);
  slab.A_start = potential_before;
  slab.A_end = potential_after;
  const MatchedFaceFlux slab_electric = slab_electric_field(slab);
  const MatchedEdgeField magnetic_before =
      matched_curl_adjoint(potential_before);
  const MatchedEdgeField magnetic_after =
      matched_curl_adjoint(potential_after);
  const MatchedFaceFlux magnetic_curl = matched_curl(magnetic_after);
  MatchedFaceFlux electric_after = slab_electric;
  add_scaled(electric_after, magnetic_curl, result.temporal_scale);
  add_scaled(electric_after, current.end, -1.0);

  MatchedFaceFlux predicted_start = slab_electric;
  add(predicted_start, current.start);
  result.field_start_equation_residual = matched_face_max_difference(
      electric_before, predicted_start);
  MatchedFaceFlux predicted_end = slab_electric;
  add_scaled(predicted_end, magnetic_curl, result.temporal_scale);
  add_scaled(predicted_end, current.end, -1.0);
  result.field_end_equation_residual = matched_face_max_difference(
      electric_after, predicted_end);
  MatchedFaceFlux update = electric_before;
  add_scaled(update, magnetic_curl, result.temporal_scale);
  add_scaled(update, current.total, -1.0);
  result.field_update_residual = matched_face_max_difference(
      electric_after, update);
  result.gauss_evolution_residual = gauss_evolution_residual(
      electric_before, electric_after, current);

  MatchedFaceFlux potential_delta = potential_after;
  add_scaled(potential_delta, potential_before, -1.0);
  result.field_action = result.beta/(
      result.temporal_scale*result.temporal_scale)
      *quadratic_energy(potential_delta)
      -result.beta*quadratic_energy(magnetic_after);

  const double displacement = result.coupled.displacement_magnitude;
  const double gamma_denominator = std::sqrt(
      1.0-displacement*displacement/
          (result.temporal_scale*result.temporal_scale));
  result.matter_legendre_momentum = E_REST*displacement/
      (C_SPEED*result.temporal_scale*gamma_denominator);
  result.matter_action = -2.0*E_REST*result.temporal_scale/C_SPEED
      *gamma_denominator;

  for (const auto& carrier : result.coupled.rebase.bounce_preimage.carrier) {
    const Vec3 begin = position(carrier);
    const Vec3 unit = carrier.velocity*(1.0/speed);
    const Vec3 finish = begin+unit*displacement;
    const EndpointDerivatives derivative = endpoint_derivatives(
        begin, finish, carrier.polarity, slab,
        result.coupling, derivative_step);
    if (!derivative.valid) return result;
    result.endpoint_derivative_convergence = std::max(
        result.endpoint_derivative_convergence, derivative.convergence);
    result.interaction_action += segment_interaction_action(
        begin, finish, carrier.polarity, slab, result.coupling);
    const Vec3 matter = unit*result.matter_legendre_momentum;
    const Vec3 canonical_start = matter-derivative.start;
    const Vec3 canonical_end = matter+derivative.end;
    const double charge_coupling = carrier.polarity*result.coupling;
    const Vec3 kinetic_start = canonical_start
        -interpolate_connection(potential_before, begin)*charge_coupling;
    const Vec3 kinetic_end = canonical_end
        -interpolate_connection(potential_after, finish)*charge_coupling;
    const Vec3 start_residual = kinetic_start
        -unit*result.momentum_before;
    const Vec3 end_residual = kinetic_end
        -unit*result.momentum_after;
    result.kinetic_start_residual = std::max(
        result.kinetic_start_residual, start_residual.mag());
    result.kinetic_end_residual = std::max(
        result.kinetic_end_residual, end_residual.mag());
    result.longitudinal_start_residual = std::max(
        result.longitudinal_start_residual,
        std::abs(start_residual.dot(unit)));
    result.longitudinal_end_residual = std::max(
        result.longitudinal_end_residual,
        std::abs(end_residual.dot(unit)));
    result.transverse_start_residual = std::max(
        result.transverse_start_residual,
        transverse_residual(start_residual, unit));
    result.transverse_end_residual = std::max(
        result.transverse_end_residual,
        transverse_residual(end_residual, unit));
  }
  result.total_action = result.matter_action+result.field_action
      +result.interaction_action;
  result.matter_energy_change = 2.0*(
      energy(result.momentum_after)-energy(result.momentum_before));
  const double field_energy_before = result.beta*(
      quadratic_energy(electric_before)+quadratic_energy(magnetic_before));
  const double field_energy_after = result.beta*(
      quadratic_energy(electric_after)+quadratic_energy(magnetic_after));
  result.field_energy_change = field_energy_after-field_energy_before;
  result.total_energy_defect = result.matter_energy_change
      +result.field_energy_change;

  const bool field_algebra = result.current_split_residual <= tolerance
      && result.continuity_residual <= tolerance
      && result.field_start_equation_residual <= tolerance
      && result.field_end_equation_residual <= tolerance
      && result.field_update_residual <= tolerance
      && result.gauss_evolution_residual <= tolerance;
  result.scalar_root_stationary = field_algebra
      && result.endpoint_derivative_convergence <= 1e-7
      && result.kinetic_start_residual <= 1e-7
      && result.kinetic_end_residual <= 1e-7
      && std::abs(result.total_energy_defect) <= tolerance;
  result.valid = field_algebra
      && result.endpoint_derivative_convergence <= 1e-7
      && result.inherited_endpoint_residual <= 1e-10
      && std::isfinite(result.matter_action)
      && std::isfinite(result.field_action)
      && std::isfinite(result.interaction_action)
      && std::isfinite(result.total_action)
      && std::isfinite(result.kinetic_start_residual)
      && std::isfinite(result.kinetic_end_residual)
      && std::isfinite(result.total_energy_defect);
  return result;
}

AtomicFaceEndpointTrialResult evaluate_atomic_face_endpoint_trial(
    const std::array<Vec3, 2>& start_position,
    const std::array<Vec3, 2>& end_position,
    const std::array<int, 2>& charge,
    const std::array<Vec3, 2>& prescribed_kinetic_start,
    const MatchedFaceFlux& potential_before,
    const MatchedFaceFlux& electric_before,
    double beta, double temporal_scale, double rest_energy,
    double c_speed, double derivative_step, double tolerance,
    bool chart_contained_derivative) {
  AtomicFaceEndpointTrialResult result(potential_before.L);
  result.carrier_count = 2;
  result.beta = beta;
  result.temporal_scale = temporal_scale;
  result.coupling = beta/temporal_scale;
  result.start_position = start_position;
  result.end_position = end_position;
  result.charge = charge;
  result.prescribed_kinetic_start = prescribed_kinetic_start;
  const std::size_t count = potential_before.L > 0
      ? static_cast<std::size_t>(potential_before.L
          *potential_before.L*potential_before.L) : 0;
  if (potential_before.L < 3 || electric_before.L != potential_before.L
      || potential_before.x.size() != count
      || electric_before.x.size() != count
      || !(beta > 0.0) || !std::isfinite(beta)
      || !(temporal_scale > 0.0) || !std::isfinite(temporal_scale)
      || !(rest_energy > 0.0) || !std::isfinite(rest_energy)
      || !(c_speed > 0.0) || !std::isfinite(c_speed)
      || !(derivative_step > 0.0) || !std::isfinite(derivative_step)
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;
  for (int carrier = 0; carrier < 2; ++carrier) {
    if ((charge[static_cast<std::size_t>(carrier)] != -1
         && charge[static_cast<std::size_t>(carrier)] != 1)
        || !finite(start_position[static_cast<std::size_t>(carrier)])
        || !finite(end_position[static_cast<std::size_t>(carrier)])
        || !finite(prescribed_kinetic_start[
            static_cast<std::size_t>(carrier)])) return result;
  }

  const AggregateCurrent current = make_current(
      start_position, end_position, charge,
      potential_before.L, temporal_scale);
  if (!current.valid) return result;
  result.current_split_residual = current.split_residual;
  result.continuity_residual = current.continuity_residual;
  result.potential_after = potential_before;
  add_scaled(result.potential_after, electric_before, -temporal_scale);
  add_scaled(result.potential_after, current.start, temporal_scale);
  DualGaugePotentialSlab slab(potential_before.L, temporal_scale);
  slab.A_start = potential_before;
  slab.A_end = result.potential_after;
  const MatchedFaceFlux slab_electric = slab_electric_field(slab);
  const MatchedEdgeField magnetic_before =
      matched_curl_adjoint(potential_before);
  result.magnetic_after = matched_curl_adjoint(result.potential_after);
  const MatchedFaceFlux magnetic_curl = matched_curl(result.magnetic_after);
  result.electric_after = slab_electric;
  add_scaled(result.electric_after, magnetic_curl, temporal_scale);
  add_scaled(result.electric_after, current.end, -1.0);

  MatchedFaceFlux predicted_start = slab_electric;
  add(predicted_start, current.start);
  result.field_start_equation_residual = matched_face_max_difference(
      electric_before, predicted_start);
  MatchedFaceFlux predicted_end = slab_electric;
  add_scaled(predicted_end, magnetic_curl, temporal_scale);
  add_scaled(predicted_end, current.end, -1.0);
  result.field_end_equation_residual = matched_face_max_difference(
      result.electric_after, predicted_end);
  MatchedFaceFlux update = electric_before;
  add_scaled(update, magnetic_curl, temporal_scale);
  add_scaled(update, current.total, -1.0);
  result.field_update_residual = matched_face_max_difference(
      result.electric_after, update);
  result.gauss_evolution_residual = gauss_evolution_residual(
      electric_before, result.electric_after, current);

  MatchedFaceFlux potential_delta = result.potential_after;
  add_scaled(potential_delta, potential_before, -1.0);
  result.field_action = beta/(temporal_scale*temporal_scale)
      *quadratic_energy(potential_delta)
      -beta*quadratic_energy(result.magnetic_after);

  double initial_matter_energy = 0.0;
  double final_matter_energy = 0.0;
  for (int carrier = 0; carrier < 2; ++carrier) {
    const std::size_t i = static_cast<std::size_t>(carrier);
    const Vec3 displacement = end_position[i]-start_position[i];
    const double displacement_squared = displacement.mag2();
    result.causal_excess = std::max(result.causal_excess,
        std::sqrt(displacement_squared)-temporal_scale);
    if (!(displacement_squared < temporal_scale*temporal_scale))
      return result;
    const double gamma_denominator = std::sqrt(
        1.0-displacement_squared/(temporal_scale*temporal_scale));
    const Vec3 matter_momentum = displacement*(
        rest_energy/(c_speed*temporal_scale*gamma_denominator));
    result.matter_action += -rest_energy*temporal_scale/c_speed
        *gamma_denominator;
    const EndpointDerivatives derivative = endpoint_derivatives(
        start_position[i], end_position[i], charge[i], slab,
        result.coupling, derivative_step, chart_contained_derivative);
    if (i == 0) {
      result.minimum_endpoint_chart_clearance =
          derivative.minimum_chart_clearance;
      result.minimum_endpoint_derivative_step =
          std::isfinite(derivative.minimum_step)
          ? derivative.minimum_step : 0.0;
    } else {
      result.minimum_endpoint_chart_clearance = std::min(
          result.minimum_endpoint_chart_clearance,
          derivative.minimum_chart_clearance);
      if (std::isfinite(derivative.minimum_step)) {
        result.minimum_endpoint_derivative_step = std::min(
            result.minimum_endpoint_derivative_step,
            derivative.minimum_step);
      }
    }
    if (!derivative.valid) return result;
    result.endpoint_derivative_convergence = std::max(
        result.endpoint_derivative_convergence, derivative.convergence);
    result.interaction_action += segment_interaction_action(
        start_position[i], end_position[i], charge[i], slab,
        result.coupling);
    const Vec3 canonical_start = matter_momentum-derivative.start;
    const Vec3 canonical_end = matter_momentum+derivative.end;
    const double charge_coupling = charge[i]*result.coupling;
    result.kinetic_start[i] = canonical_start
        -interpolate_connection(potential_before,
                                start_position[i])*charge_coupling;
    result.kinetic_end[i] = canonical_end
        -interpolate_connection(result.potential_after,
                                end_position[i])*charge_coupling;
    result.start_residual[i] = result.kinetic_start[i]
        -prescribed_kinetic_start[i];
    result.residual_infinity_norm = std::max(
        result.residual_infinity_norm,
        max_component(result.start_residual[i]));
    initial_matter_energy += std::sqrt(rest_energy*rest_energy
        +c_speed*c_speed*prescribed_kinetic_start[i].mag2());
    final_matter_energy += std::sqrt(rest_energy*rest_energy
        +c_speed*c_speed*result.kinetic_end[i].mag2());
  }
  result.total_action = result.matter_action+result.field_action
      +result.interaction_action;
  result.matter_energy_change = final_matter_energy-initial_matter_energy;
  result.ordinary_field_energy_change = beta*(
      quadratic_energy(result.electric_after)
      +quadratic_energy(result.magnetic_after)
      -quadratic_energy(electric_before)
      -quadratic_energy(magnetic_before));
  result.modified_field_energy_change = beta*(
      matched_modified_energy(result.electric_after,
                              result.magnetic_after, temporal_scale)
      -matched_modified_energy(electric_before,
                              magnetic_before, temporal_scale));
  result.ordinary_total_energy_defect = result.matter_energy_change
      +result.ordinary_field_energy_change;
  result.modified_total_energy_defect = result.matter_energy_change
      +result.modified_field_energy_change;

  const bool field_algebra = result.current_split_residual <= tolerance
      && result.continuity_residual <= tolerance
      && result.field_start_equation_residual <= tolerance
      && result.field_end_equation_residual <= tolerance
      && result.field_update_residual <= tolerance
      && result.gauss_evolution_residual <= tolerance;
  // `valid` means the trial is algebraically evaluable.  Derivative
  // convergence is a registered acceptance metric, not a domain restriction;
  // Newton's centered Jacobian must be allowed to sample nearby trials that
  // do not themselves satisfy the final convergence gate.
  result.valid = field_algebra && result.causal_excess <= tolerance
      && std::isfinite(result.residual_infinity_norm)
      && std::isfinite(result.matter_action)
      && std::isfinite(result.field_action)
      && std::isfinite(result.interaction_action)
      && std::isfinite(result.ordinary_total_energy_defect)
      && std::isfinite(result.modified_total_energy_defect);
  return result;
}

AtomicFaceOneSidedNormalResult evaluate_atomic_face_one_sided_normal(
    const std::array<Vec3, 2>& start_position,
    const std::array<Vec3, 2>& end_position,
    const std::array<int, 2>& charge,
    const std::array<Vec3, 2>& prescribed_kinetic_start,
    const MatchedFaceFlux& potential_before,
    const MatchedFaceFlux& electric_before,
    double beta, double temporal_scale, double rest_energy,
    double c_speed, int normal_axis,
    double derivative_step, double tolerance) {
  AtomicFaceOneSidedNormalResult result;
  result.normal_axis = normal_axis;
  if (normal_axis < 0 || normal_axis > 2
      || !(derivative_step > 0.0)
      || !std::isfinite(derivative_step)) return result;
  const auto ordinary = evaluate_atomic_face_endpoint_trial(
      start_position, end_position, charge, prescribed_kinetic_start,
      potential_before, electric_before, beta, temporal_scale,
      rest_energy, c_speed, derivative_step, tolerance, false);
  if (!ordinary.valid) return result;
  for (int carrier = 0; carrier < 2; ++carrier) {
    const std::size_t i = static_cast<std::size_t>(carrier);
    if (endpoint_chart_clearance(component(start_position[i], normal_axis))
            > tolerance
        || endpoint_chart_clearance(component(end_position[i], normal_axis))
            > tolerance) return result;
  }
  DualGaugePotentialSlab slab(potential_before.L, temporal_scale);
  slab.A_start = potential_before;
  slab.A_end = ordinary.potential_after;
  for (int carrier = 0; carrier < 2; ++carrier) {
    const std::size_t i = static_cast<std::size_t>(carrier);
    double coarse[2][2]{};
    double fine[2][2]{};
    for (int endpoint = 0; endpoint < 2; ++endpoint) {
      for (int side_index = 0; side_index < 2; ++side_index) {
        const int side = side_index == 0 ? -1 : +1;
        coarse[endpoint][side_index] = one_sided_endpoint_derivative(
            start_position[i], end_position[i], charge[i], slab,
            beta/temporal_scale, endpoint, normal_axis,
            derivative_step, side);
        fine[endpoint][side_index] = one_sided_endpoint_derivative(
            start_position[i], end_position[i], charge[i], slab,
            beta/temporal_scale, endpoint, normal_axis,
            derivative_step/2.0, side);
        if (!std::isfinite(coarse[endpoint][side_index])
            || !std::isfinite(fine[endpoint][side_index])) return result;
        result.derivative_convergence = std::max(
            result.derivative_convergence,
            std::abs(fine[endpoint][side_index]
                     -coarse[endpoint][side_index]));
      }
    }
    result.start_derivative_left[i] = fine[0][0];
    result.start_derivative_right[i] = fine[0][1];
    result.end_derivative_left[i] = fine[1][0];
    result.end_derivative_right[i] = fine[1][1];
    result.maximum_derivative_jump = std::max({
        result.maximum_derivative_jump,
        std::abs(fine[0][1]-fine[0][0]),
        std::abs(fine[1][1]-fine[1][0])});

    const Vec3 displacement = end_position[i]-start_position[i];
    const double denominator = std::sqrt(
        1.0-displacement.mag2()/(temporal_scale*temporal_scale));
    if (!(denominator > 0.0)) return result;
    const Vec3 matter_momentum = displacement*(
        rest_energy/(c_speed*temporal_scale*denominator));
    const double connection = component(
        interpolate_connection(potential_before, start_position[i]),
        normal_axis);
    const double prescribed = component(prescribed_kinetic_start[i],
                                        normal_axis);
    const double common = component(matter_momentum, normal_axis)
        -charge[i]*(beta/temporal_scale)*connection-prescribed;
    result.incoming_residual_left[i] = common-fine[0][0];
    result.incoming_residual_right[i] = common-fine[0][1];
  }
  result.valid = true;
  return result;
}

}  // namespace ftd::eft
