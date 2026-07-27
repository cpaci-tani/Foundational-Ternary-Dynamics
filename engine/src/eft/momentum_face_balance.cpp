#include "ftd/eft/momentum_face_balance.h"

#include "ftd/eft/discrete_legendre_worldline.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {
namespace {

double component(const Vec3& value, int axis) {
  if (axis == 0) return value.x;
  if (axis == 1) return value.y;
  return value.z;
}

void set_component(Vec3& value, int axis, double component_value) {
  if (axis == 0) value.x = component_value;
  else if (axis == 1) value.y = component_value;
  else value.z = component_value;
}

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double field_difference(const std::vector<double>& lhs,
                        const std::vector<double>& rhs,
                        double rhs_scale = 1.0) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result,
        std::abs(lhs[i] - rhs_scale * rhs[i]));
  return result;
}

double site_vector_difference(const SiteVectorField& lhs,
                              const SiteVectorField& rhs,
                              double rhs_scale = 1.0) {
  double result = 0.0;
  for (int i = 0; i < 3; ++i) {
    result = std::max(result, field_difference(
        lhs[static_cast<std::size_t>(i)],
        rhs[static_cast<std::size_t>(i)], rhs_scale));
  }
  return result;
}

double tensor_field_difference(const TensorFaceField& lhs,
                               const TensorFaceField& rhs,
                               double rhs_scale = 1.0) {
  double result = 0.0;
  for (int i = 0; i < 3; ++i)
    result = std::max(result, site_vector_difference(
        lhs[static_cast<std::size_t>(i)],
        rhs[static_cast<std::size_t>(i)], rhs_scale));
  return result;
}

double tensor_rows_difference(const TensorRows3& lhs,
                              const TensorRows3& rhs) {
  double result = 0.0;
  for (int i = 0; i < 3; ++i)
    result = std::max(result, max_abs(
        lhs[static_cast<std::size_t>(i)]
        - rhs[static_cast<std::size_t>(i)]));
  return result;
}

TensorRows3 outer_rows(const Vec3& lhs, const Vec3& rhs) {
  return {rhs * lhs.x, rhs * lhs.y, rhs * lhs.z};
}

TensorRows3 add_rows(const TensorRows3& lhs, const TensorRows3& rhs) {
  TensorRows3 result{};
  for (int i = 0; i < 3; ++i)
    result[static_cast<std::size_t>(i)] =
        lhs[static_cast<std::size_t>(i)]
        + rhs[static_cast<std::size_t>(i)];
  return result;
}

TensorRows3 stress_rows(const SymmetricTensor3& stress, double scale) {
  return {
      Vec3{stress.xx * scale, stress.xy * scale, stress.xz * scale},
      Vec3{stress.xy * scale, stress.yy * scale, stress.yz * scale},
      Vec3{stress.xz * scale, stress.yz * scale, stress.zz * scale}};
}

std::vector<double> zero_field(int L) {
  return std::vector<double>(
      static_cast<std::size_t>(L * L * L), 0.0);
}

SiteVectorField zero_site_vector(int L) {
  const auto zero = zero_field(L);
  return {{zero, zero, zero}};
}

TensorFaceField zero_tensor_field(int L) {
  return {{zero_site_vector(L), zero_site_vector(L),
           zero_site_vector(L)}};
}

void add_site_vector_in_place(SiteVectorField& target,
                              const SiteVectorField& source) {
  for (int i = 0; i < 3; ++i) {
    auto& out = target[static_cast<std::size_t>(i)];
    const auto& in = source[static_cast<std::size_t>(i)];
    for (std::size_t site = 0; site < out.size(); ++site)
      out[site] += in[site];
  }
}

void add_tensor_field_in_place(TensorFaceField& target,
                               const TensorFaceField& source) {
  for (int i = 0; i < 3; ++i)
    add_site_vector_in_place(target[static_cast<std::size_t>(i)],
                             source[static_cast<std::size_t>(i)]);
}

double site_vector_l1(const SiteVectorField& field) {
  long double result = 0.0L;
  for (const auto& component_field : field)
    for (double value : component_field) result += std::abs(value);
  return static_cast<double>(result);
}

double local_balance_residual(int L,
                              const SiteVectorField& before,
                              const SiteVectorField& after,
                              const TensorFaceField& flux,
                              const SiteVectorField& source) {
  const auto index = [L](int x, int y, int z) {
    const auto wrap = [L](int value) {
      value %= L;
      return value < 0 ? value + L : value;
    };
    return static_cast<std::size_t>(
        (wrap(x) * L + wrap(y)) * L + wrap(z));
  };
  double result = 0.0;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const std::size_t at = index(x, y, z);
        for (int i = 0; i < 3; ++i) {
          const auto& row = flux[static_cast<std::size_t>(i)];
          const double divergence =
              row[0][at] - row[0][index(x - 1, y, z)]
              + row[1][at] - row[1][index(x, y - 1, z)]
              + row[2][at] - row[2][index(x, y, z - 1)];
          result = std::max(result, std::abs(
              after[static_cast<std::size_t>(i)][at]
              - before[static_cast<std::size_t>(i)][at]
              + divergence
              - source[static_cast<std::size_t>(i)][at]));
        }
      }
    }
  }
  return result;
}

Vec3 global_momentum(const SiteVectorField& field) {
  Vec3 result{};
  for (int i = 0; i < 3; ++i) {
    long double total = 0.0L;
    for (double value : field[static_cast<std::size_t>(i)]) total += value;
    set_component(result, i, static_cast<double>(total));
  }
  return result;
}

PiecewiseCurrentSignature static_density(int L, const Vec3& position) {
  return make_piecewise_current_signature(
      L, {{+1, {position, position}}});
}

SiteVectorField impulse_density(const PiecewiseCurrentSignature& density,
                                const Vec3& impulse) {
  SiteVectorField result = zero_site_vector(density.L);
  for (int i = 0; i < 3; ++i) {
    const double p = component(impulse, i);
    auto& out = result[static_cast<std::size_t>(i)];
    for (std::size_t site = 0; site < out.size(); ++site)
      out[site] = p * density.rho_before[site];
  }
  return result;
}

}  // namespace

MomentumWorldlineBalance make_momentum_worldline_balance(
    int L,
    const Vec3& start_position,
    const Vec3& end_position,
    const Vec3& momentum,
    double tolerance) {
  MomentumWorldlineBalance result;
  result.L = L;
  result.start_position = start_position;
  result.end_position = end_position;
  result.displacement = end_position - start_position;
  result.momentum = momentum;
  if (L < 3 || !std::isfinite(momentum.x)
      || !std::isfinite(momentum.y) || !std::isfinite(momentum.z)
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;
  result.scalar_transport = make_piecewise_current_signature(
      L, {{+1, {start_position, end_position}}});
  if (!result.scalar_transport.valid) return result;
  result.momentum_before = zero_site_vector(L);
  result.momentum_after = zero_site_vector(L);
  result.tensor_face_flux = zero_tensor_field(L);
  for (int i = 0; i < 3; ++i) {
    const double p = component(momentum, i);
    for (std::size_t site = 0;
         site < result.scalar_transport.rho_before.size(); ++site) {
      result.momentum_before[static_cast<std::size_t>(i)][site] =
          p * result.scalar_transport.rho_before[site];
      result.momentum_after[static_cast<std::size_t>(i)][site] =
          p * result.scalar_transport.rho_after[site];
      result.tensor_face_flux[static_cast<std::size_t>(i)][0][site] =
          p * result.scalar_transport.current_x[site];
      result.tensor_face_flux[static_cast<std::size_t>(i)][1][site] =
          p * result.scalar_transport.current_y[site];
      result.tensor_face_flux[static_cast<std::size_t>(i)][2][site] =
          p * result.scalar_transport.current_z[site];
    }
  }
  result.local_balance_residual = local_balance_residual(
      L, result.momentum_before, result.momentum_after,
      result.tensor_face_flux, zero_site_vector(L));
  result.global_momentum_residual = max_abs(
      global_momentum(result.momentum_after)
      - global_momentum(result.momentum_before));
  for (int i = 0; i < 3; ++i) {
    Vec3 row{};
    for (int j = 0; j < 3; ++j) {
      long double sum = 0.0L;
      for (double value : result.tensor_face_flux[
               static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]) {
        sum += value;
      }
      set_component(row, j, static_cast<double>(sum));
    }
    result.integrated_flux_moment[static_cast<std::size_t>(i)] = row;
  }
  result.expected_outer_moment = outer_rows(momentum,
                                             result.displacement);
  result.face_first_moment_residual = tensor_rows_difference(
      result.integrated_flux_moment, result.expected_outer_moment);
  result.valid = result.scalar_transport.valid
      && result.local_balance_residual <= tolerance
      && result.global_momentum_residual <= tolerance
      && result.face_first_moment_residual <= tolerance;
  return result;
}

FreeMomentumTransportBalance analyze_free_momentum_transport_balance(
    int L,
    const Vec3& start_position,
    const Vec3& momentum,
    double rest_energy,
    double c_speed,
    double dt,
    double tolerance) {
  FreeMomentumTransportBalance result;
  result.dt = dt;
  result.rest_energy = rest_energy;
  result.c_speed = c_speed;
  if (!std::isfinite(dt) || dt <= 0.0) return result;
  const Vec3 displacement = free_displacement_from_momentum(
      momentum, rest_energy, c_speed, c_speed * dt);
  result.velocity = displacement * (1.0 / dt);
  result.worldline = make_momentum_worldline_balance(
      L, start_position, start_position + displacement,
      momentum, tolerance);
  result.stress = make_constituent_stress_moment(
      {momentum}, rest_energy, c_speed, tolerance);
  result.stress_bridge_residual = tensor_rows_difference(
      result.worldline.integrated_flux_moment,
      stress_rows(result.stress.stress, dt));
  result.causal_residual = std::max(
      0.0, displacement.mag() - c_speed * dt);
  result.valid = result.worldline.valid && result.stress.valid
      && result.stress_bridge_residual <= tolerance
      && result.causal_residual <= tolerance;
  return result;
}

CollisionMomentumFaceBalance analyze_collision_momentum_face_balance(
    int L,
    const Vec3& collision_position,
    Coord chart_direction,
    int polarity,
    double speed,
    double rest_energy,
    double c_speed,
    double segment_distance,
    double tolerance) {
  CollisionMomentumFaceBalance result;
  result.L = L;
  result.polarity = polarity;
  result.chart_direction = chart_direction;
  result.collision_position = collision_position;
  if (!std::isfinite(segment_distance) || segment_distance <= 0.0)
    return result;
  result.collision = analyze_constituent_relative_collision(
      L, collision_position, chart_direction, polarity, speed,
      rest_energy, c_speed, segment_distance, tolerance);
  if (!result.collision.valid) return result;
  const Vec3 n = result.collision.chart_normal;
  const Vec3 first_start = collision_position - n * segment_distance;
  const Vec3 second_start = collision_position + n * segment_distance;
  const Vec3 first_end = first_start;
  const Vec3 second_end = second_start;
  result.first_incoming = make_momentum_worldline_balance(
      L, first_start, collision_position,
      result.collision.momentum_first_before, tolerance);
  result.second_incoming = make_momentum_worldline_balance(
      L, second_start, collision_position,
      result.collision.momentum_second_before, tolerance);
  result.first_outgoing = make_momentum_worldline_balance(
      L, collision_position, first_end,
      result.collision.momentum_first_after, tolerance);
  result.second_outgoing = make_momentum_worldline_balance(
      L, collision_position, second_end,
      result.collision.momentum_second_after, tolerance);
  result.individual_segment_residual = std::max({
      result.first_incoming.local_balance_residual,
      result.second_incoming.local_balance_residual,
      result.first_outgoing.local_balance_residual,
      result.second_outgoing.local_balance_residual});

  result.aggregate_momentum_before = zero_site_vector(L);
  result.aggregate_momentum_after = zero_site_vector(L);
  result.aggregate_tensor_flux = zero_tensor_field(L);
  add_site_vector_in_place(result.aggregate_momentum_before,
                           result.first_incoming.momentum_before);
  add_site_vector_in_place(result.aggregate_momentum_before,
                           result.second_incoming.momentum_before);
  add_site_vector_in_place(result.aggregate_momentum_after,
                           result.first_outgoing.momentum_after);
  add_site_vector_in_place(result.aggregate_momentum_after,
                           result.second_outgoing.momentum_after);
  add_tensor_field_in_place(result.aggregate_tensor_flux,
                            result.first_incoming.tensor_face_flux);
  add_tensor_field_in_place(result.aggregate_tensor_flux,
                            result.second_incoming.tensor_face_flux);
  add_tensor_field_in_place(result.aggregate_tensor_flux,
                            result.first_outgoing.tensor_face_flux);
  add_tensor_field_in_place(result.aggregate_tensor_flux,
                            result.second_outgoing.tensor_face_flux);

  const auto vertex = static_density(L, collision_position);
  const SiteVectorField first_source = impulse_density(
      vertex, result.collision.impulse_first);
  const SiteVectorField second_source = impulse_density(
      vertex, result.collision.impulse_second);
  result.aggregate_impulse_source = zero_site_vector(L);
  add_site_vector_in_place(result.aggregate_impulse_source, first_source);
  add_site_vector_in_place(result.aggregate_impulse_source, second_source);
  result.aggregate_impulse_source_l1 = site_vector_l1(
      result.aggregate_impulse_source);
  result.individual_impulse_source_l1 = site_vector_l1(first_source)
      + site_vector_l1(second_source);

  const SiteVectorField first_vertex_before = impulse_density(
      vertex, result.collision.momentum_first_before);
  const SiteVectorField first_vertex_after = impulse_density(
      vertex, result.collision.momentum_first_after);
  const SiteVectorField second_vertex_before = impulse_density(
      vertex, result.collision.momentum_second_before);
  const SiteVectorField second_vertex_after = impulse_density(
      vertex, result.collision.momentum_second_after);
  result.constituent_impulse_residual = std::max(
      local_balance_residual(L, first_vertex_before, first_vertex_after,
                             zero_tensor_field(L), first_source),
      local_balance_residual(L, second_vertex_before, second_vertex_after,
                             zero_tensor_field(L), second_source));
  result.aggregate_local_balance_residual = local_balance_residual(
      L, result.aggregate_momentum_before, result.aggregate_momentum_after,
      result.aggregate_tensor_flux, result.aggregate_impulse_source);
  result.aggregate_global_momentum_residual = max_abs(
      global_momentum(result.aggregate_momentum_after)
      - global_momentum(result.aggregate_momentum_before)
      - global_momentum(result.aggregate_impulse_source));
  result.energy_residual = result.collision.matter_energy_residual;

  result.integrated_flux_moment = add_rows(add_rows(
      result.first_incoming.integrated_flux_moment,
      result.second_incoming.integrated_flux_moment), add_rows(
      result.first_outgoing.integrated_flux_moment,
      result.second_outgoing.integrated_flux_moment));
  result.expected_piecewise_outer_moment = add_rows(add_rows(
      result.first_incoming.expected_outer_moment,
      result.second_incoming.expected_outer_moment), add_rows(
      result.first_outgoing.expected_outer_moment,
      result.second_outgoing.expected_outer_moment));
  result.tensor_moment_residual = tensor_rows_difference(
      result.integrated_flux_moment,
      result.expected_piecewise_outer_moment);

  const auto reverse_first_incoming = make_momentum_worldline_balance(
      L, first_end, collision_position,
      result.collision.momentum_first_after * -1.0, tolerance);
  const auto reverse_second_incoming = make_momentum_worldline_balance(
      L, second_end, collision_position,
      result.collision.momentum_second_after * -1.0, tolerance);
  const auto reverse_first_outgoing = make_momentum_worldline_balance(
      L, collision_position, first_start,
      result.collision.momentum_first_before * -1.0, tolerance);
  const auto reverse_second_outgoing = make_momentum_worldline_balance(
      L, collision_position, second_start,
      result.collision.momentum_second_before * -1.0, tolerance);
  SiteVectorField reverse_before = zero_site_vector(L);
  SiteVectorField reverse_after = zero_site_vector(L);
  TensorFaceField reverse_flux = zero_tensor_field(L);
  add_site_vector_in_place(reverse_before,
                           reverse_first_incoming.momentum_before);
  add_site_vector_in_place(reverse_before,
                           reverse_second_incoming.momentum_before);
  add_site_vector_in_place(reverse_after,
                           reverse_first_outgoing.momentum_after);
  add_site_vector_in_place(reverse_after,
                           reverse_second_outgoing.momentum_after);
  add_tensor_field_in_place(reverse_flux,
                            reverse_first_incoming.tensor_face_flux);
  add_tensor_field_in_place(reverse_flux,
                            reverse_second_incoming.tensor_face_flux);
  add_tensor_field_in_place(reverse_flux,
                            reverse_first_outgoing.tensor_face_flux);
  add_tensor_field_in_place(reverse_flux,
                            reverse_second_outgoing.tensor_face_flux);
  const Vec3 reverse_first_impulse =
      result.collision.momentum_first_before * -1.0
      - result.collision.momentum_first_after * -1.0;
  const Vec3 reverse_second_impulse =
      result.collision.momentum_second_before * -1.0
      - result.collision.momentum_second_after * -1.0;
  SiteVectorField reverse_source = impulse_density(
      vertex, reverse_first_impulse);
  add_site_vector_in_place(reverse_source,
      impulse_density(vertex, reverse_second_impulse));
  result.reversal_endpoint_residual = std::max(
      site_vector_difference(reverse_before,
                             result.aggregate_momentum_after, -1.0),
      site_vector_difference(reverse_after,
                             result.aggregate_momentum_before, -1.0));
  result.reversal_tensor_flux_residual = tensor_field_difference(
      reverse_flux, result.aggregate_tensor_flux);
  result.reversal_impulse_source_residual = site_vector_difference(
      reverse_source, result.aggregate_impulse_source);

  result.valid = result.collision.valid
      && result.first_incoming.valid && result.second_incoming.valid
      && result.first_outgoing.valid && result.second_outgoing.valid
      && result.individual_segment_residual <= tolerance
      && result.constituent_impulse_residual <= tolerance
      && result.aggregate_impulse_source_l1 <= tolerance
      && result.individual_impulse_source_l1 > tolerance
      && result.aggregate_local_balance_residual <= tolerance
      && result.aggregate_global_momentum_residual <= tolerance
      && result.energy_residual <= tolerance
      && result.tensor_moment_residual <= tolerance
      && result.reversal_endpoint_residual <= tolerance
      && result.reversal_tensor_flux_residual <= tolerance
      && result.reversal_impulse_source_residual <= tolerance;
  return result;
}

}  // namespace ftd::eft
