#include "ftd/eft/ternary_collision_vertex.h"

#include "ftd/eft/canonical_subcell_section.h"
#include "ftd/eft/discrete_legendre_worldline.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

int wrap(int value, int L) {
  value %= L;
  return value < 0 ? value + L : value;
}

int flat_index(int L, int x, int y, int z) {
  return (wrap(x, L) * L + wrap(y, L)) * L + wrap(z, L);
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

double maximum_difference(const Vec3& lhs, const Vec3& rhs) {
  return std::max({std::abs(lhs.x - rhs.x),
                   std::abs(lhs.y - rhs.y),
                   std::abs(lhs.z - rhs.z)});
}

double maximum_difference(const std::vector<double>& lhs,
                          const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    result = std::max(result, std::abs(lhs[i] - rhs[i]));
  }
  return result;
}

double field_l1(const std::vector<double>& field) {
  long double result = 0.0L;
  for (double value : field) result += std::abs(value);
  return static_cast<double>(result);
}

void deposit_shape(const SubcellPolarityShape& shape,
                   int L,
                   std::vector<double>& density) {
  for (std::size_t i = 0; i < shape.weight_count; ++i) {
    const auto& entry = shape.weights[i];
    density[static_cast<std::size_t>(flat_index(
        L, entry.site.x, entry.site.y, entry.site.z))] += entry.weight;
  }
}

Vec3 normalized(const Vec3& value) {
  const double magnitude = value.mag();
  return magnitude > 0.0 ? value * (1.0 / magnitude) : Vec3{};
}

double relativistic_momentum_magnitude(double speed,
                                       double rest_energy,
                                       double c_speed) {
  const double beta_squared = speed * speed / (c_speed * c_speed);
  return rest_energy * speed
      / (c_speed * c_speed * std::sqrt(1.0 - beta_squared));
}

double relativistic_energy(const Vec3& momentum,
                           double rest_energy,
                           double c_speed) {
  return std::sqrt(rest_energy * rest_energy
      + c_speed * c_speed * momentum.mag2());
}

struct PhasePoint {
  Vec3 position{};
  Vec3 momentum{};
};

bool phase_less(const PhasePoint& lhs, const PhasePoint& rhs) {
  const std::array<double, 6> a{{lhs.position.x, lhs.position.y,
      lhs.position.z, lhs.momentum.x, lhs.momentum.y, lhs.momentum.z}};
  const std::array<double, 6> b{{rhs.position.x, rhs.position.y,
      rhs.position.z, rhs.momentum.x, rhs.momentum.y, rhs.momentum.z}};
  return a < b;
}

double phase_multiset_residual(std::vector<PhasePoint> lhs,
                               std::vector<PhasePoint> rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  std::sort(lhs.begin(), lhs.end(), phase_less);
  std::sort(rhs.begin(), rhs.end(), phase_less);
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    result = std::max({result,
        maximum_difference(lhs[i].position, rhs[i].position),
        maximum_difference(lhs[i].momentum, rhs[i].momentum)});
  }
  return result;
}

double signature_difference(const PiecewiseCurrentSignature& lhs,
                            const PiecewiseCurrentSignature& rhs) {
  if (!lhs.valid || !rhs.valid) return INFINITY;
  return std::max({
      maximum_difference(lhs.rho_before, rhs.rho_before),
      maximum_difference(lhs.rho_after, rhs.rho_after),
      maximum_difference(lhs.current_x, rhs.current_x),
      maximum_difference(lhs.current_y, rhs.current_y),
      maximum_difference(lhs.current_z, rhs.current_z)});
}

std::vector<PiecewiseWorldline> reversed(
    const std::vector<PiecewiseWorldline>& input) {
  auto result = input;
  for (auto& line : result) {
    std::reverse(line.vertices.begin(), line.vertices.end());
  }
  return result;
}

double reversal_signature_residual(const PiecewiseCurrentSignature& forward,
                                   const PiecewiseCurrentSignature& reverse) {
  if (!forward.valid || !reverse.valid) return INFINITY;
  double result = std::max(
      maximum_difference(forward.rho_before, reverse.rho_after),
      maximum_difference(forward.rho_after, reverse.rho_before));
  if (forward.current_x.size() != reverse.current_x.size()) return INFINITY;
  for (std::size_t i = 0; i < forward.current_x.size(); ++i) {
    result = std::max({result,
        std::abs(forward.current_x[i] + reverse.current_x[i]),
        std::abs(forward.current_y[i] + reverse.current_y[i]),
        std::abs(forward.current_z[i] + reverse.current_z[i])});
  }
  return result;
}

}  // namespace

TernaryCapacityResult analyze_ternary_same_sign_capacity(
    int multiplicity, int sign) {
  TernaryCapacityResult result;
  result.multiplicity = multiplicity;
  result.sign = sign;
  if (multiplicity < 2 || (sign != -1 && sign != +1)) return result;
  result.required_charge = sign * multiplicity;
  result.best_ternary_state = sign;
  result.minimum_charge_defect = multiplicity - 1;
  result.valid = true;
  return result;
}

bool physically_identical(const CarrierIntrinsicAttributes& lhs,
                          const CarrierIntrinsicAttributes& rhs) {
  return lhs.polarity == rhs.polarity
      && lhs.spin_twice == rhs.spin_twice
      && lhs.color == rhs.color
      && lhs.flavor == rhs.flavor
      && lhs.additional_physical_tags == rhs.additional_physical_tags;
}

int PiecewiseCurrentSignature::index(int x, int y, int z) const {
  return flat_index(L, x, y, z);
}

PiecewiseCurrentSignature make_piecewise_current_signature(
    int L, const std::vector<PiecewiseWorldline>& worldlines) {
  PiecewiseCurrentSignature result;
  result.L = L;
  result.carrier_count = static_cast<int>(worldlines.size());
  if (L < 3 || worldlines.empty()) return result;
  const std::size_t side = static_cast<std::size_t>(L);
  if (side > std::numeric_limits<std::size_t>::max() / side
      || side * side > std::numeric_limits<std::size_t>::max() / side) {
    return result;
  }
  const std::size_t volume = side * side * side;
  result.rho_before.assign(volume, 0.0);
  result.rho_after.assign(volume, 0.0);
  result.current_x.assign(volume, 0.0);
  result.current_y.assign(volume, 0.0);
  result.current_z.assign(volume, 0.0);

  for (const auto& worldline : worldlines) {
    if ((worldline.charge != -1 && worldline.charge != +1)
        || worldline.vertices.size() < 2) return result;
    for (const auto& vertex : worldline.vertices) {
      if (!finite(vertex)) return result;
    }
    const auto start_chart = centered_canonical_subcell_chart(
        worldline.vertices.front());
    const auto end_chart = centered_canonical_subcell_chart(
        worldline.vertices.back());
    if (!start_chart.valid || !end_chart.valid) return result;
    const auto start_shape = make_subcell_polarity_shape(
        start_chart.anchor, start_chart.remainder, worldline.charge);
    const auto end_shape = make_subcell_polarity_shape(
        end_chart.anchor, end_chart.remainder, worldline.charge);
    if (!start_shape.valid || !end_shape.valid) return result;
    deposit_shape(start_shape, L, result.rho_before);
    deposit_shape(end_shape, L, result.rho_after);
    result.total_charge += worldline.charge;

    for (std::size_t vertex = 1; vertex < worldline.vertices.size(); ++vertex) {
      const auto from = centered_canonical_subcell_chart(
          worldline.vertices[vertex - 1]);
      const auto to = centered_canonical_subcell_chart(
          worldline.vertices[vertex]);
      if (!from.valid || !to.valid) return result;
      const auto segment = make_face_current_segment(
          L, from.anchor, from.remainder,
          to.anchor, to.remainder, worldline.charge);
      if (!segment.valid || segment.current_x.size() != volume) return result;
      for (std::size_t i = 0; i < volume; ++i) {
        result.current_x[i] += segment.current_x[i];
        result.current_y[i] += segment.current_y[i];
        result.current_z[i] += segment.current_z[i];
      }
    }
  }

  double continuity = 0.0;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto at = [&result](const std::vector<double>& field,
                                  int sx, int sy, int sz) {
          return field[static_cast<std::size_t>(result.index(sx, sy, sz))];
        };
        const std::size_t i = static_cast<std::size_t>(result.index(x, y, z));
        const double divergence =
            at(result.current_x, x, y, z)
            - at(result.current_x, x - 1, y, z)
            + at(result.current_y, x, y, z)
            - at(result.current_y, x, y - 1, z)
            + at(result.current_z, x, y, z)
            - at(result.current_z, x, y, z - 1);
        continuity = std::max(continuity, std::abs(
            result.rho_after[i] - result.rho_before[i] + divergence));
      }
    }
  }
  result.continuity_residual = continuity;
  result.current_l1 = field_l1(result.current_x)
      + field_l1(result.current_y) + field_l1(result.current_z);
  result.valid = std::isfinite(continuity) && continuity <= 1e-12;
  return result;
}

IdenticalCrossingResult analyze_identical_crossing(
    int L,
    const Vec3& center,
    const Vec3& direction,
    double half_separation,
    double speed,
    double dt,
    int charge,
    double rest_energy,
    double c_speed,
    const CarrierIntrinsicAttributes& lhs,
    const CarrierIntrinsicAttributes& rhs,
    double tolerance) {
  IdenticalCrossingResult result;
  result.attributes_identical = physically_identical(lhs, rhs);
  if (L < 3 || !finite(center) || !finite(direction)
      || direction.mag() == 0.0 || !std::isfinite(half_separation)
      || half_separation <= 0.0 || !std::isfinite(speed) || speed <= 0.0
      || !std::isfinite(dt) || dt <= 0.0
      || (charge != -1 && charge != +1)
      || !std::isfinite(rest_energy) || rest_energy <= 0.0
      || !std::isfinite(c_speed) || c_speed <= 0.0
      || !std::isfinite(tolerance) || tolerance < 0.0
      || speed > c_speed + tolerance) return result;

  const Vec3 axis = normalized(direction);
  result.collision_time = half_separation / speed;
  result.remaining_time = dt - result.collision_time;
  if (result.collision_time > dt + tolerance) return result;
  result.endpoint_capacity = analyze_ternary_same_sign_capacity(2, charge);
  if (std::abs(result.remaining_time) <= tolerance) {
    result.remaining_time = 0.0;
    result.boundary_overload = true;
    result.charge_residual = static_cast<double>(
        result.endpoint_capacity.minimum_charge_defect);
    result.valid = result.endpoint_capacity.valid;
    return result;
  }
  if (result.remaining_time < 0.0) return result;

  const Vec3 left_start = center - axis * half_separation;
  const Vec3 right_start = center + axis * half_separation;
  const double post_distance = speed * result.remaining_time;
  const Vec3 negative_end = center - axis * post_distance;
  const Vec3 positive_end = center + axis * post_distance;
  const double momentum_magnitude = relativistic_momentum_magnitude(
      speed, rest_energy, c_speed);
  const Vec3 positive_momentum = axis * momentum_magnitude;
  const Vec3 negative_momentum = positive_momentum * -1.0;

  const std::vector<PiecewiseWorldline> pass{{
      charge, {left_start, positive_end}},
      {charge, {right_start, negative_end}}};
  const std::vector<PiecewiseWorldline> bounce{{
      charge, {left_start, center, negative_end}},
      {charge, {right_start, center, positive_end}}};
  result.pass_through = make_piecewise_current_signature(L, pass);
  result.elastic_bounce = make_piecewise_current_signature(L, bounce);

  const std::vector<PhasePoint> pass_phase{{
      positive_end, positive_momentum},
      {negative_end, negative_momentum}};
  const std::vector<PhasePoint> bounce_phase{{
      negative_end, negative_momentum},
      {positive_end, positive_momentum}};
  result.phase_space_multiset_residual = phase_multiset_residual(
      pass_phase, bounce_phase);
  result.current_signature_residual = signature_difference(
      result.pass_through, result.elastic_bounce);

  const double incoming_energy = 2.0 * relativistic_energy(
      positive_momentum, rest_energy, c_speed);
  const double pass_energy = relativistic_energy(
      positive_momentum, rest_energy, c_speed)
      + relativistic_energy(negative_momentum, rest_energy, c_speed);
  const double bounce_energy = pass_energy;
  result.energy_residual = std::max(std::abs(pass_energy - incoming_energy),
                                    std::abs(bounce_energy - incoming_energy));
  result.momentum_residual = std::max(
      (positive_momentum + negative_momentum).mag(),
      (negative_momentum + positive_momentum).mag());
  result.charge_residual = 0.0;
  result.causal_residual = std::max(0.0, speed - c_speed);
  result.continuity_residual = std::max(
      result.pass_through.continuity_residual,
      result.elastic_bounce.continuity_residual);

  const auto reverse_pass = make_piecewise_current_signature(
      L, reversed(pass));
  const auto reverse_bounce = make_piecewise_current_signature(
      L, reversed(bounce));
  result.time_reversal_residual = std::max(
      reversal_signature_residual(result.pass_through, reverse_pass),
      reversal_signature_residual(result.elastic_bounce, reverse_bounce));
  const Vec3 reverse_positive = positive_end
      + free_displacement_from_momentum(
          negative_momentum, rest_energy, c_speed, c_speed * dt);
  const Vec3 reverse_negative = negative_end
      + free_displacement_from_momentum(
          positive_momentum, rest_energy, c_speed, c_speed * dt);
  result.time_reversal_residual = std::max({
      result.time_reversal_residual,
      maximum_difference(reverse_positive, left_start),
      maximum_difference(reverse_negative, right_start)});
  result.label_quotient_equivalent = result.attributes_identical
      && result.phase_space_multiset_residual <= tolerance
      && result.current_signature_residual <= tolerance;
  result.valid = result.pass_through.valid && result.elastic_bounce.valid
      && result.phase_space_multiset_residual <= tolerance
      && result.current_signature_residual <= tolerance
      && result.energy_residual <= tolerance
      && result.momentum_residual <= tolerance
      && result.charge_residual <= tolerance
      && result.causal_residual <= tolerance
      && result.continuity_residual <= tolerance
      && result.time_reversal_residual <= tolerance;
  return result;
}

ElasticScatteringCounterfamily analyze_elastic_scattering_counterfamily(
    double speed, double rest_energy, double c_speed) {
  ElasticScatteringCounterfamily result;
  if (!std::isfinite(speed) || speed <= 0.0
      || !std::isfinite(rest_energy) || rest_energy <= 0.0
      || !std::isfinite(c_speed) || c_speed <= 0.0
      || speed >= c_speed) return result;
  const std::array<Vec3, 5> directions{{
      {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0},
      normalized({1.0, 1.0, 0.0}),
      normalized({1.0, 1.0, 1.0})}};
  result.output_count = static_cast<int>(directions.size());
  result.momentum_magnitude = relativistic_momentum_magnitude(
      speed, rest_energy, c_speed);
  const Vec3 reference = directions[0] * result.momentum_magnitude;
  const double reference_energy = 2.0 * relativistic_energy(
      reference, rest_energy, c_speed);
  result.minimum_direction_separation = INFINITY;
  for (std::size_t i = 0; i < directions.size(); ++i) {
    const Vec3 momentum = directions[i] * result.momentum_magnitude;
    result.maximum_total_momentum_residual = std::max(
        result.maximum_total_momentum_residual,
        (momentum + momentum * -1.0).mag());
    const double energy = 2.0 * relativistic_energy(
        momentum, rest_energy, c_speed);
    result.maximum_total_energy_residual = std::max(
        result.maximum_total_energy_residual,
        std::abs(energy - reference_energy));
    for (std::size_t j = i + 1; j < directions.size(); ++j) {
      result.minimum_direction_separation = std::min(
          result.minimum_direction_separation,
          (directions[i] - directions[j]).mag());
    }
  }
  result.valid = result.output_count >= 5
      && result.minimum_direction_separation > 0.0
      && result.maximum_total_momentum_residual <= 1e-12
      && result.maximum_total_energy_residual <= 1e-12;
  return result;
}

}  // namespace ftd::eft
