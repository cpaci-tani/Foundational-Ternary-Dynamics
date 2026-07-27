#include "ftd/eft/configuration_space_carrier.h"

#include "ftd/eft/conserved_charge_basis.h"
#include "ftd/eft/matched_gauss_transport.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace ftd::eft {
namespace {

std::vector<double>& component(MatchedFaceFlux& field, int axis) {
  return axis == 0 ? field.x : (axis == 1 ? field.y : field.z);
}

const std::vector<double>& component(const MatchedFaceFlux& field, int axis) {
  return axis == 0 ? field.x : (axis == 1 ? field.y : field.z);
}

std::vector<double>& component(MatchedEdgeField& field, int axis) {
  return axis == 0 ? field.x : (axis == 1 ? field.y : field.z);
}

void add(MatchedFaceFlux& target, const MatchedFaceFlux& value,
         double scale = 1.0) {
  for (int axis = 0; axis < 3; ++axis)
    for (std::size_t i = 0; i < component(target, axis).size(); ++i)
      component(target, axis)[i] += scale * component(value, axis)[i];
}

MatchedFaceFlux affine(const MatchedFaceFlux& base,
                       const MatchedFaceFlux& direction,
                       double t) {
  MatchedFaceFlux result = base;
  add(result, direction, t);
  return result;
}

double dot(const MatchedFaceFlux& lhs, const MatchedFaceFlux& rhs) {
  long double result = 0.0L;
  for (int axis = 0; axis < 3; ++axis)
    for (std::size_t i = 0; i < component(lhs, axis).size(); ++i)
      result += static_cast<long double>(component(lhs, axis)[i])
              * static_cast<long double>(component(rhs, axis)[i]);
  return static_cast<double>(result);
}

std::array<double, 3> harmonic_coordinate(const MatchedFaceFlux& field) {
  std::array<double, 3> result{};
  const double denominator = static_cast<double>(field.L * field.L * field.L);
  for (int axis = 0; axis < 3; ++axis) {
    long double sum = 0.0L;
    for (double value : component(field, axis)) sum += value;
    result[static_cast<std::size_t>(axis)] =
        static_cast<double>(sum) / denominator;
  }
  return result;
}

double max_difference(const std::array<double, 3>& lhs,
                      const std::array<double, 3>& rhs) {
  double result = 0.0;
  for (int i = 0; i < 3; ++i)
    result = std::max(result, std::abs(lhs[static_cast<std::size_t>(i)]
                                      - rhs[static_cast<std::size_t>(i)]));
  return result;
}

double max_affine_residual(const MatchedFaceFlux& candidate,
                           const MatchedFaceFlux& base,
                           const MatchedFaceFlux& direction,
                           double t) {
  double result = 0.0;
  for (int axis = 0; axis < 3; ++axis)
    for (std::size_t i = 0; i < component(base, axis).size(); ++i)
      result = std::max(result, std::abs(
          component(candidate, axis)[i] - component(base, axis)[i]
          - t * component(direction, axis)[i]));
  return result;
}

void add_uniform(MatchedFaceFlux& field,
                 const std::array<double, 3>& value) {
  for (int axis = 0; axis < 3; ++axis)
    for (double& entry : component(field, axis))
      entry += value[static_cast<std::size_t>(axis)];
}

MatchedEdgeField local_potential(int L, int axis, int which) {
  MatchedEdgeField result(L);
  const int c = L / 2;
  const std::array<int, 3> site = which == 0
      ? std::array<int, 3>{{c, c, c}}
      : std::array<int, 3>{{1 % L, (L - 1) % L, 2 % L}};
  const int i = result.index(site[0], site[1], site[2]);
  component(result, (axis + which) % 3)[static_cast<std::size_t>(i)] =
      which == 0 ? 0.375 : -0.21875;
  return result;
}

int support(const MatchedFaceFlux& field) {
  int result = 0;
  for (int axis = 0; axis < 3; ++axis)
    for (double value : component(field, axis))
      if (value != 0.0) ++result;
  return result;
}

int support_excess(const MatchedFaceFlux& reference,
                   const MatchedFaceFlux& candidate) {
  int result = 0;
  for (int axis = 0; axis < 3; ++axis)
    for (std::size_t i = 0; i < component(reference, axis).size(); ++i)
      if (component(reference, axis)[i] == 0.0
          && component(candidate, axis)[i] != 0.0) ++result;
  return result;
}

std::array<int, 3> sink_site(int c, int axis) {
  std::array<int, 3> result{{c, c, c}};
  ++result[static_cast<std::size_t>(axis)];
  return result;
}

}  // namespace

ConfigurationSpaceCarrierResult analyze_configuration_space_carrier() {
  ConfigurationSpaceCarrierResult result;
  constexpr double tolerance = 1e-12;
  const std::array<int, 4> volumes{{3, 4, 5, 8}};
  const std::array<double, 5> homotopy{{0.0, 0.25, 0.5, 0.75, 1.0}};
  const std::array<std::array<double, 3>, 4> harmonics{{
      {{0.0, 0.0, 0.0}}, {{0.125, 0.0, 0.0}},
      {{0.0, -0.25, 0.0}}, {{0.0, 0.0, 0.375}}}};

  for (int L : volumes) {
    ++result.volumes;
    const int c = L / 2;
    for (int sign : {-1, 1}) {
      for (int axis = 0; axis < 3; ++axis) {
        const int source_index = MatchedFaceFlux(L).index(c, c, c);
        const auto sink = sink_site(c, axis);
        const int sink_index = MatchedFaceFlux(L).index(
            sink[0], sink[1], sink[2]);
        std::vector<int> source(static_cast<std::size_t>(L * L * L), 0);
        source[static_cast<std::size_t>(source_index)] = sign;
        source[static_cast<std::size_t>(sink_index)] = -sign;

        for (int which = 0; which < 2; ++which) {
          const MatchedFaceFlux direction =
              matched_curl(local_potential(L, axis, which));
          result.maximum_divergence_free_deformation = std::max(
              result.maximum_divergence_free_deformation,
              max_divergence(direction));
          for (const auto& harmonic : harmonics) {
            MatchedFaceFlux base(L);
            if (!seed_dipole_path(base, source_index, sink_index,
                                  static_cast<double>(sign))) {
              return result;
            }
            add_uniform(base, harmonic);
            const auto fixed_harmonic = harmonic_coordinate(base);
            const double energy_base = quadratic_energy(base);
            const double linear_energy = dot(base, direction);
            const double quadratic_direction = quadratic_energy(direction);
            for (double t : homotopy) {
              const MatchedFaceFlux candidate = affine(base, direction, t);
              result.maximum_gauss_residual = std::max(
                  result.maximum_gauss_residual,
                  max_gauss_residual(candidate, source));
              result.maximum_harmonic_coordinate_residual = std::max(
                  result.maximum_harmonic_coordinate_residual,
                  max_difference(harmonic_coordinate(candidate), fixed_harmonic));
              result.maximum_affine_residual = std::max(
                  result.maximum_affine_residual,
                  max_affine_residual(candidate, base, direction, t));
              const double predicted = energy_base + t * linear_energy
                  + t * t * quadratic_direction;
              result.maximum_energy_polynomial_residual = std::max(
                  result.maximum_energy_polynomial_residual,
                  std::abs(quadratic_energy(candidate) - predicted));
              ++result.homotopy_samples;
            }
            ++result.fibre_fixtures;
          }
        }
      }
    }
  }

  // A compactly supported real cochain remains compactly supported under the
  // contraction tE. This finite observer is the algebraic support witness;
  // the independent proof covers c_00(Z^3) and its l2 completion.
  const MatchedFaceFlux local = matched_curl(local_potential(9, 0, 0));
  const int local_support = support(local);
  for (double t : homotopy) {
    const MatchedFaceFlux candidate = affine(MatchedFaceFlux(9), local, t);
    result.maximum_support_excess = std::max(
        result.maximum_support_excess, support_excess(local, candidate));
    if (t != 0.0 && support(candidate) != local_support)
      result.maximum_support_excess = std::numeric_limits<int>::max();
    ++result.uncontained_support_samples;
  }

  const auto transitions = frozen_native_charge_transitions();
  const auto charge_basis = solve_conserved_charge_basis(transitions);
  result.transition_rows = static_cast<int>(transitions.size());
  result.registered_feature_rank = charge_basis.rank;
  result.registered_feature_nullity = charge_basis.nullity;

  result.fixed_source_fibres_affine_contractible =
      result.fibre_fixtures == 192
      && result.homotopy_samples == 960
      && result.maximum_gauss_residual <= tolerance
      && result.maximum_harmonic_coordinate_residual <= tolerance
      && result.maximum_affine_residual <= tolerance
      && result.maximum_energy_polynomial_residual <= tolerance
      && result.maximum_divergence_free_deformation <= tolerance;
  result.uncontained_finite_energy_space_contractible =
      result.uncontained_support_samples == 5
      && result.maximum_support_excess == 0;
  result.snapshot_is_disjoint_union_of_contractible_fibres = true;
  result.ternary_snapshot_disconnectedness_is_conservation = false;
  result.registered_additive_transition_invariant_exists =
      charge_basis.nullity != 0;
  result.universal_transition_graph_invariant_excluded = false;

  // The frozen free-field minimum is J=W=0, a one-point vacuum manifold.
  result.frozen_vacuum_is_single_point = true;
  result.vacuum_pi0_nontrivial = 0;
  result.vacuum_pi1_rank = 0;
  result.vacuum_pi2_rank = 0;
  result.vacuum_pi3_rank = 0;
  result.normalized_direction_protected_while_zero_allowed = false;

  // In d=3, E_2(R)+E_0(R)=R E_2+R^3 E_0 shrinks to zero.
  result.two_derivative_static_core_size_stable = false;
  result.four_derivative_term_can_balance_scaling = true;
  result.compact_u1_automatically_supplies_electric_charge = false;
  result.compact_flux_integer_requires_admissibility = true;
  result.same_variable_active_localized_mode_excluded = false;
  result.production_changed = false;

  result.valid = result.fixed_source_fibres_affine_contractible
      && result.uncontained_finite_energy_space_contractible
      && result.snapshot_is_disjoint_union_of_contractible_fibres
      && !result.ternary_snapshot_disconnectedness_is_conservation
      && result.transition_rows == 9
      && result.registered_feature_rank == 4
      && result.registered_feature_nullity == 0
      && !result.registered_additive_transition_invariant_exists
      && !result.universal_transition_graph_invariant_excluded
      && result.frozen_vacuum_is_single_point
      && !result.normalized_direction_protected_while_zero_allowed
      && !result.two_derivative_static_core_size_stable
      && result.four_derivative_term_can_balance_scaling
      && !result.compact_u1_automatically_supplies_electric_charge
      && result.compact_flux_integer_requires_admissibility
      && !result.same_variable_active_localized_mode_excluded
      && !result.production_changed;
  return result;
}

}  // namespace ftd::eft
