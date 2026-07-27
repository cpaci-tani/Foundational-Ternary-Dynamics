/** FTD-0601: closed neutral pair of constituent-complete charged trimers. */

#include "ftd/eft/closed_neutral_trimer_pair.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double gate = 1e-12;
constexpr const char* protocol_sha256 =
    "89979BF190B8A5FD36DF6642356E455F13ED01C9A2C42E20777B150996C1C1F3";

using ftd::Coord;
using ftd::Vec3;
using ftd::eft::ClosedNeutralTrimerPairState;
using ftd::eft::ClosedNeutralTrimerPairStepResult;

struct Fixture {
  ClosedNeutralTrimerPairState state{L};
  bool valid = false;
};

Vec3 effective_position(const ftd::eft::MatchedMatterPoint& point) {
  return {point.anchor.x + point.remainder.x,
          point.anchor.y + point.remainder.y,
          point.anchor.z + point.remainder.z};
}

Vec3 cyclic(const Vec3& value) {
  return {value.y, value.z, value.x};
}

std::vector<double> density_of(const ClosedNeutralTrimerPairState& state) {
  const std::size_t count = static_cast<std::size_t>(L) * L * L;
  std::vector<double> density(count, 0.0);
  for (std::size_t a = 0; a < state.constituents.size(); ++a) {
    const auto coat = ftd::eft::make_quadratic_polarity_coat(
        effective_position(state.constituents[a]), state.charges[a]);
    if (!coat.valid) return {};
    for (std::size_t item = 0; item < coat.weight_count; ++item) {
      const auto& weight = coat.weights[item];
      const int index = state.electric.index(
          weight.site.x, weight.site.y, weight.site.z);
      density[static_cast<std::size_t>(index)] += weight.weight;
    }
  }
  return density;
}

bool seed_neutral_field(ClosedNeutralTrimerPairState& state) {
  const std::vector<double> density = density_of(state);
  if (density.empty()) return false;
  double total = 0.0;
  int reference = -1;
  for (std::size_t i = 0; i < density.size(); ++i) {
    total += density[i];
    if (std::abs(density[i]) > 0.0) reference = static_cast<int>(i);
  }
  if (reference < 0 || std::abs(total) > gate) return false;
  bool seeded = true;
  for (std::size_t i = 0; i < density.size(); ++i) {
    if (static_cast<int>(i) == reference || density[i] == 0.0) continue;
    seeded = seeded && ftd::eft::seed_dipole_path(
        state.electric, static_cast<int>(i), reference, density[i]);
  }
  return seeded && ftd::eft::max_fractional_gauss_residual(
      state.electric, density) <= gate;
}

Fixture make_fixture(const Vec3& velocity_a, const Vec3& velocity_b) {
  Fixture fixture;
  const std::array<Coord, 6> anchors{{
      {4, 7, 7}, {5, 8, 7}, {5, 7, 8},
      {12, 9, 9}, {11, 8, 9}, {11, 9, 8}}};
  const Vec3 remainder_a{0.173, -0.219, 0.287};
  const Vec3 remainder_b{-0.137, 0.191, -0.233};
  const Vec3 momentum_a = ftd::eft::production_flat_momentum(velocity_a);
  const Vec3 momentum_b = ftd::eft::production_flat_momentum(velocity_b);
  for (std::size_t a = 0; a < anchors.size(); ++a) {
    fixture.state.constituents[a].anchor = anchors[a];
    fixture.state.constituents[a].remainder = a < 3
        ? remainder_a : remainder_b;
    fixture.state.constituents[a].momentum = a < 3
        ? momentum_a : momentum_b;
  }
  fixture.valid = seed_neutral_field(fixture.state);
  return fixture;
}

Fixture translate_fixture(const Fixture& source, const Coord& shift) {
  Fixture target;
  target.state.charges = source.state.charges;
  for (std::size_t a = 0; a < source.state.constituents.size(); ++a) {
    target.state.constituents[a] = source.state.constituents[a];
    target.state.constituents[a].anchor = {
        (source.state.constituents[a].anchor.x + shift.x + L) % L,
        (source.state.constituents[a].anchor.y + shift.y + L) % L,
        (source.state.constituents[a].anchor.z + shift.z + L) % L};
  }
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int from = source.state.electric.index(x, y, z);
        const int to = target.state.electric.index(
            x + shift.x, y + shift.y, z + shift.z);
        target.state.electric.x[to] = source.state.electric.x[from];
        target.state.electric.y[to] = source.state.electric.y[from];
        target.state.electric.z[to] = source.state.electric.z[from];
        target.state.magnetic_half.x[to] = source.state.magnetic_half.x[from];
        target.state.magnetic_half.y[to] = source.state.magnetic_half.y[from];
        target.state.magnetic_half.z[to] = source.state.magnetic_half.z[from];
      }
  target.valid = source.valid;
  return target;
}

ClosedNeutralTrimerPairState translate_state(
    const ClosedNeutralTrimerPairState& source, const Coord& shift) {
  Fixture fixture;
  fixture.state = source;
  return translate_fixture(fixture, shift).state;
}

Fixture cyclic_fixture(const Fixture& source) {
  Fixture target;
  target.state.charges = source.state.charges;
  for (std::size_t a = 0; a < source.state.constituents.size(); ++a) {
    const auto& point = source.state.constituents[a];
    target.state.constituents[a].anchor = {
        point.anchor.y, point.anchor.z, point.anchor.x};
    target.state.constituents[a].remainder = cyclic(point.remainder);
    target.state.constituents[a].momentum = cyclic(point.momentum);
  }
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int from = source.state.electric.index(x, y, z);
        const int to = target.state.electric.index(y, z, x);
        target.state.electric.x[to] = source.state.electric.y[from];
        target.state.electric.y[to] = source.state.electric.z[from];
        target.state.electric.z[to] = source.state.electric.x[from];
        target.state.magnetic_half.x[to] = source.state.magnetic_half.y[from];
        target.state.magnetic_half.y[to] = source.state.magnetic_half.z[from];
        target.state.magnetic_half.z[to] = source.state.magnetic_half.x[from];
      }
  target.valid = source.valid;
  return target;
}

ClosedNeutralTrimerPairState cyclic_state(
    const ClosedNeutralTrimerPairState& source) {
  Fixture fixture;
  fixture.state = source;
  return cyclic_fixture(fixture).state;
}

Fixture swapped_fixture(const Fixture& source) {
  Fixture target = source;
  std::swap(target.state.constituents[0], target.state.constituents[1]);
  std::swap(target.state.charges[0], target.state.charges[1]);
  std::swap(target.state.constituents[3], target.state.constituents[4]);
  std::swap(target.state.charges[3], target.state.charges[4]);
  return target;
}

ClosedNeutralTrimerPairState swapped_state(
    const ClosedNeutralTrimerPairState& source) {
  ClosedNeutralTrimerPairState target = source;
  std::swap(target.constituents[0], target.constituents[1]);
  std::swap(target.charges[0], target.charges[1]);
  std::swap(target.constituents[3], target.constituents[4]);
  std::swap(target.charges[3], target.charges[4]);
  return target;
}

Fixture conjugate_exchange_fixture(const Fixture& source) {
  Fixture target;
  for (std::size_t a = 0; a < 3; ++a) {
    target.state.constituents[a] = source.state.constituents[a + 3];
    target.state.constituents[a + 3] = source.state.constituents[a];
    target.state.charges[a] = -source.state.charges[a + 3];
    target.state.charges[a + 3] = -source.state.charges[a];
  }
  for (std::size_t i = 0; i < source.state.electric.x.size(); ++i) {
    target.state.electric.x[i] = -source.state.electric.x[i];
    target.state.electric.y[i] = -source.state.electric.y[i];
    target.state.electric.z[i] = -source.state.electric.z[i];
    target.state.magnetic_half.x[i] = -source.state.magnetic_half.x[i];
    target.state.magnetic_half.y[i] = -source.state.magnetic_half.y[i];
    target.state.magnetic_half.z[i] = -source.state.magnetic_half.z[i];
  }
  target.valid = source.valid;
  return target;
}

ClosedNeutralTrimerPairState conjugate_exchange_state(
    const ClosedNeutralTrimerPairState& source) {
  Fixture fixture;
  fixture.state = source;
  return conjugate_exchange_fixture(fixture).state;
}

double maximum_common_gate(const ClosedNeutralTrimerPairStepResult& result) {
  return std::max({result.root_residual, result.continuity_residual,
      result.gauss_before_residual, result.gauss_after_residual,
      result.force_residual, result.kinematic_residual,
      result.kinetic_discrete_gradient_residual,
      result.electric_adjoint_residual, result.magnetic_work_residual,
      result.binding_work_residual, result.binding_impulse_sum_residual,
      result.matter_work_residual, result.field_work_residual,
      result.total_energy_residual, result.causal_speed_excess});
}

double total_energy_before(const ClosedNeutralTrimerPairStepResult& result) {
  return result.kinetic_energy_before + result.binding_energy_before
      + result.field_energy_before;
}

double total_energy_after(const ClosedNeutralTrimerPairStepResult& result) {
  return result.kinetic_energy_after + result.binding_energy_after
      + result.field_energy_after;
}

int count_anchor_changes(const ClosedNeutralTrimerPairState& before,
                         const ClosedNeutralTrimerPairState& after) {
  int result = 0;
  for (std::size_t a = 0; a < before.constituents.size(); ++a) {
    const auto& lhs = before.constituents[a].anchor;
    const auto& rhs = after.constituents[a].anchor;
    if (lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z) ++result;
  }
  return result;
}

struct Summary {
  int one_step_forward_arms = 0;
  int one_step_reverse_arms = 0;
  int one_step_forward_converged = 0;
  int one_step_reverse_converged = 0;
  int repeated_forward_steps = 0;
  int repeated_reverse_steps = 0;
  int site_hops = 0;
  bool common_one_step_pass = true;
  bool momentum_pass = true;
  bool inward_response_pass = false;
  bool repeated_run = false;
  bool repeated_pass = true;
  bool fail_closed_controls = true;
  double worst_one_step_gate = 0.0;
  double worst_one_step_inverse = 0.0;
  double worst_translation = 0.0;
  double worst_rotation = 0.0;
  double worst_permutation = 0.0;
  double worst_charge_conjugation = 0.0;
  double worst_pseudomomentum_defect = 0.0;
  double worst_cumulative_pseudomomentum_drift = 0.0;
  double worst_repeated_gate = 0.0;
  double repeated_state_recovery = 0.0;
  double repeated_energy_drift = 0.0;
  double rest_inward_impulse = 0.0;
  double rest_separation_before = 0.0;
  double rest_separation_after_one_step = 0.0;
  double rest_separation_after_repeated = 0.0;
  double minimum_internal_pair_distance = INFINITY;
  double maximum_internal_pair_distance = 0.0;
  std::string verdict;
};

void write_record(const Summary& summary) {
  const std::filesystem::path result_dir =
      std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0601";
  std::filesystem::create_directories(result_dir);
  std::ofstream json(result_dir / "ftd_0601_closed_neutral_pair_v1.json");
  json << std::setprecision(17)
       << "{\n"
       << "  \"ftd_id\": \"FTD-0601\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"stationary_compensator_present\": false,\n"
       << "  \"one_step_forward_arms\": " << summary.one_step_forward_arms << ",\n"
       << "  \"one_step_reverse_arms\": " << summary.one_step_reverse_arms << ",\n"
       << "  \"one_step_forward_converged\": " << summary.one_step_forward_converged << ",\n"
       << "  \"one_step_reverse_converged\": " << summary.one_step_reverse_converged << ",\n"
       << "  \"common_one_step_pass\": " << (summary.common_one_step_pass ? "true" : "false") << ",\n"
       << "  \"momentum_pass\": " << (summary.momentum_pass ? "true" : "false") << ",\n"
       << "  \"inward_response_pass\": " << (summary.inward_response_pass ? "true" : "false") << ",\n"
       << "  \"repeated_run\": " << (summary.repeated_run ? "true" : "false") << ",\n"
       << "  \"repeated_pass\": " << (summary.repeated_pass ? "true" : "false") << ",\n"
       << "  \"repeated_forward_steps\": " << summary.repeated_forward_steps << ",\n"
       << "  \"repeated_reverse_steps\": " << summary.repeated_reverse_steps << ",\n"
       << "  \"site_hops\": " << summary.site_hops << ",\n"
       << "  \"worst_one_step_gate\": " << summary.worst_one_step_gate << ",\n"
       << "  \"worst_one_step_inverse\": " << summary.worst_one_step_inverse << ",\n"
       << "  \"worst_translation_residual\": " << summary.worst_translation << ",\n"
       << "  \"worst_rotation_residual\": " << summary.worst_rotation << ",\n"
       << "  \"worst_permutation_residual\": " << summary.worst_permutation << ",\n"
       << "  \"worst_charge_conjugation_residual\": " << summary.worst_charge_conjugation << ",\n"
       << "  \"worst_pseudomomentum_defect\": " << summary.worst_pseudomomentum_defect << ",\n"
       << "  \"worst_cumulative_pseudomomentum_drift\": " << summary.worst_cumulative_pseudomomentum_drift << ",\n"
       << "  \"worst_repeated_gate\": " << summary.worst_repeated_gate << ",\n"
       << "  \"repeated_state_recovery\": " << summary.repeated_state_recovery << ",\n"
       << "  \"repeated_energy_drift\": " << summary.repeated_energy_drift << ",\n"
       << "  \"rest_inward_impulse\": " << summary.rest_inward_impulse << ",\n"
       << "  \"rest_separation_before\": " << summary.rest_separation_before << ",\n"
       << "  \"rest_separation_after_one_step\": " << summary.rest_separation_after_one_step << ",\n"
       << "  \"rest_separation_after_repeated\": " << summary.rest_separation_after_repeated << ",\n"
       << "  \"minimum_internal_pair_distance\": " << summary.minimum_internal_pair_distance << ",\n"
       << "  \"maximum_internal_pair_distance\": " << summary.maximum_internal_pair_distance << "\n"
       << "}\n";
  std::ofstream csv(result_dir / "ftd_0601_closed_neutral_pair_v1.csv");
  csv << "ftd_id,verdict,common_one_step_pass,momentum_pass,inward_response_pass,"
         "one_step_forward_arms,one_step_reverse_arms,repeated_forward_steps,"
         "repeated_reverse_steps,site_hops,worst_one_step_gate,"
         "worst_one_step_inverse,worst_pseudomomentum_defect,"
         "worst_cumulative_pseudomomentum_drift,worst_repeated_gate,"
         "repeated_state_recovery,repeated_energy_drift,rest_inward_impulse,"
         "rest_separation_before,rest_separation_after_one_step,"
         "rest_separation_after_repeated,minimum_internal_pair_distance,"
         "maximum_internal_pair_distance\n";
  csv << std::setprecision(17) << "FTD-0601," << summary.verdict << ','
      << summary.common_one_step_pass << ',' << summary.momentum_pass << ','
      << summary.inward_response_pass << ',' << summary.one_step_forward_arms << ','
      << summary.one_step_reverse_arms << ',' << summary.repeated_forward_steps << ','
      << summary.repeated_reverse_steps << ',' << summary.site_hops << ','
      << summary.worst_one_step_gate << ',' << summary.worst_one_step_inverse << ','
      << summary.worst_pseudomomentum_defect << ','
      << summary.worst_cumulative_pseudomomentum_drift << ','
      << summary.worst_repeated_gate << ',' << summary.repeated_state_recovery << ','
      << summary.repeated_energy_drift << ',' << summary.rest_inward_impulse << ','
      << summary.rest_separation_before << ','
      << summary.rest_separation_after_one_step << ','
      << summary.rest_separation_after_repeated << ','
      << summary.minimum_internal_pair_distance << ','
      << summary.maximum_internal_pair_distance << '\n';
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  ftd::eft::ClosedNeutralPairOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-13;
  options.max_iterations = 64;

  const std::array<std::array<Vec3, 2>, 4> velocities{{
      {{{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}},
      {{{0.04, 0.0, 0.0}, {0.04, 0.0, 0.0}}},
      {{{0.04, 0.0, 0.0}, {-0.04, 0.0, 0.0}}},
      {{{0.04, 0.03, 0.0}, {-0.03, 0.02, -0.01}}}}};
  const Coord shift{1, 1, -2};
  Summary summary;
  bool physical_one_step_failure = false;
  bool numerical_one_step_failure = false;

  for (std::size_t mode = 0; mode < velocities.size(); ++mode) {
    const Fixture base = make_fixture(velocities[mode][0], velocities[mode][1]);
    const Fixture translated = translate_fixture(base, shift);
    const Fixture rotated = cyclic_fixture(base);
    const Fixture permuted = swapped_fixture(base);
    const Fixture conjugated = conjugate_exchange_fixture(base);
    const std::array<const Fixture*, 5> fixtures{{
        &base, &translated, &rotated, &permuted, &conjugated}};
    std::array<ClosedNeutralTrimerPairStepResult, 5> forward{};
    for (std::size_t arm = 0; arm < fixtures.size(); ++arm) {
      forward[arm] = ftd::eft::solve_closed_neutral_pair_forward(
          fixtures[arm]->state, options);
      ++summary.one_step_forward_arms;
      if (forward[arm].solve.converged) ++summary.one_step_forward_converged;
      summary.worst_one_step_gate = std::max(
          summary.worst_one_step_gate, maximum_common_gate(forward[arm]));
      summary.worst_pseudomomentum_defect = std::max(
          summary.worst_pseudomomentum_defect,
          forward[arm].pseudomomentum_defect_norm);
      summary.minimum_internal_pair_distance = std::min(
          summary.minimum_internal_pair_distance,
          forward[arm].minimum_internal_pair_distance);
      summary.maximum_internal_pair_distance = std::max(
          summary.maximum_internal_pair_distance,
          forward[arm].maximum_internal_pair_distance);
      if (!fixtures[arm]->valid || !forward[arm].solve.converged) {
        numerical_one_step_failure = true;
        summary.common_one_step_pass = false;
        continue;
      }
      if (!forward[arm].common_action_gates_pass) {
        physical_one_step_failure = true;
        summary.common_one_step_pass = false;
      }
      summary.momentum_pass = summary.momentum_pass
          && forward[arm].isolated_momentum_gate_pass;
      const auto reverse = ftd::eft::solve_closed_neutral_pair_reverse(
          forward[arm].later, options);
      ++summary.one_step_reverse_arms;
      if (reverse.solve.converged) ++summary.one_step_reverse_converged;
      if (!reverse.solve.converged) {
        numerical_one_step_failure = true;
        summary.common_one_step_pass = false;
        continue;
      }
      const double inverse = ftd::eft::closed_neutral_pair_state_max_difference(
          fixtures[arm]->state, reverse.earlier);
      summary.worst_one_step_inverse = std::max(
          summary.worst_one_step_inverse, inverse);
      if (!reverse.common_action_gates_pass || inverse > 1e-10) {
        physical_one_step_failure = true;
        summary.common_one_step_pass = false;
      }
    }
    if (std::all_of(forward.begin(), forward.end(),
        [](const auto& result) { return result.valid; })) {
      const auto translated_back = translate_state(
          forward[1].later, {-shift.x, -shift.y, -shift.z});
      summary.worst_translation = std::max(summary.worst_translation,
          ftd::eft::closed_neutral_pair_state_max_difference(
              forward[0].later, translated_back));
      const auto rotated_back = cyclic_state(cyclic_state(forward[2].later));
      summary.worst_rotation = std::max(summary.worst_rotation,
          ftd::eft::closed_neutral_pair_state_max_difference(
              forward[0].later, rotated_back));
      const auto permuted_back = swapped_state(forward[3].later);
      summary.worst_permutation = std::max(summary.worst_permutation,
          ftd::eft::closed_neutral_pair_state_max_difference(
              forward[0].later, permuted_back));
      const auto conjugated_back = conjugate_exchange_state(forward[4].later);
      summary.worst_charge_conjugation = std::max(
          summary.worst_charge_conjugation,
          ftd::eft::closed_neutral_pair_state_max_difference(
              forward[0].later, conjugated_back));
      if (mode == 0) {
        summary.rest_inward_impulse = forward[0].inward_impulse;
        summary.rest_separation_before = forward[0].center_separation_before;
        summary.rest_separation_after_one_step =
            forward[0].center_separation_after;
      }
    }
  }
  if (summary.worst_translation > gate || summary.worst_rotation > gate
      || summary.worst_permutation > gate
      || summary.worst_charge_conjugation > gate) {
    physical_one_step_failure = true;
    summary.common_one_step_pass = false;
  }
  summary.inward_response_pass = summary.rest_inward_impulse > 1e-10;

  Fixture invalid = make_fixture(velocities[3][0], velocities[3][1]);
  invalid.state.constituents[3].anchor = invalid.state.constituents[0].anchor;
  invalid.state.constituents[3].remainder = invalid.state.constituents[0].remainder;
  const auto invalid_result = ftd::eft::solve_closed_neutral_pair_forward(
      invalid.state, options);
  auto impossible_options = options;
  impossible_options.solve_tolerance = 1e-30;
  impossible_options.max_iterations = 1;
  const Fixture impossible = make_fixture(velocities[3][0], velocities[3][1]);
  const auto impossible_result = ftd::eft::solve_closed_neutral_pair_forward(
      impossible.state, impossible_options);
  summary.fail_closed_controls = !invalid_result.valid
      && !invalid_result.common_action_gates_pass
      && impossible_result.solve.attempted
      && !impossible_result.solve.converged
      && !impossible_result.common_action_gates_pass;

  if (summary.common_one_step_pass) {
    summary.repeated_run = true;
    for (std::size_t mode = 0; mode < 3; ++mode) {
      const Fixture fixture = make_fixture(velocities[mode][0], velocities[mode][1]);
      ClosedNeutralTrimerPairState state = fixture.state;
      const ClosedNeutralTrimerPairState initial = state;
      Vec3 initial_total_pseudomomentum{};
      double energy_initial = NAN;
      double energy_final = NAN;
      bool arm_pass = fixture.valid;
      for (int tick = 0; tick < 16 && arm_pass; ++tick) {
        const auto step = ftd::eft::solve_closed_neutral_pair_forward(
            state, options);
        ++summary.repeated_forward_steps;
        arm_pass = step.solve.converged && step.valid
            && maximum_common_gate(step) <= 1e-10
            && step.minimum_internal_pair_distance >= 1.35
            && step.maximum_internal_pair_distance <= 1.48;
        summary.worst_repeated_gate = std::max(
            summary.worst_repeated_gate, maximum_common_gate(step));
        summary.worst_pseudomomentum_defect = std::max(
            summary.worst_pseudomomentum_defect,
            step.pseudomomentum_defect_norm);
        summary.momentum_pass = summary.momentum_pass
            && step.isolated_momentum_gate_pass;
        summary.minimum_internal_pair_distance = std::min(
            summary.minimum_internal_pair_distance,
            step.minimum_internal_pair_distance);
        summary.maximum_internal_pair_distance = std::max(
            summary.maximum_internal_pair_distance,
            step.maximum_internal_pair_distance);
        if (!arm_pass) break;
        if (tick == 0) {
          energy_initial = total_energy_before(step);
          initial_total_pseudomomentum = step.total_pseudomomentum_before;
        }
        energy_final = total_energy_after(step);
        summary.worst_cumulative_pseudomomentum_drift = std::max(
            summary.worst_cumulative_pseudomomentum_drift,
            (step.total_pseudomomentum_after
              - initial_total_pseudomomentum).mag());
        summary.site_hops += count_anchor_changes(state, step.later);
        if (mode == 0 && tick == 15)
          summary.rest_separation_after_repeated = step.center_separation_after;
        state = step.later;
      }
      for (int tick = 0; tick < 16 && arm_pass; ++tick) {
        const auto step = ftd::eft::solve_closed_neutral_pair_reverse(
            state, options);
        ++summary.repeated_reverse_steps;
        arm_pass = step.solve.converged && step.valid
            && maximum_common_gate(step) <= 1e-10;
        summary.worst_repeated_gate = std::max(
            summary.worst_repeated_gate, maximum_common_gate(step));
        if (!arm_pass) break;
        state = step.earlier;
      }
      const double recovery = ftd::eft::closed_neutral_pair_state_max_difference(
          initial, state);
      summary.repeated_state_recovery = std::max(
          summary.repeated_state_recovery, recovery);
      const double drift = std::abs(energy_final - energy_initial);
      summary.repeated_energy_drift = std::max(
          summary.repeated_energy_drift, drift);
      arm_pass = arm_pass && recovery <= 1e-8 && drift <= 1e-9;
      summary.repeated_pass = summary.repeated_pass && arm_pass;
    }
    summary.repeated_pass = summary.repeated_pass
        && summary.repeated_forward_steps == 48
        && summary.repeated_reverse_steps == 48
        && summary.site_hops > 0;
  }

  if (summary.common_one_step_pass && summary.repeated_pass
      && summary.inward_response_pass && summary.momentum_pass) {
    summary.verdict = "NEUTRAL_TRIMER_PAIR_CLOSED_DYNAMICS_CONSTRUCTIVE";
  } else if (summary.common_one_step_pass && summary.repeated_pass
      && summary.inward_response_pass && !summary.momentum_pass) {
    summary.verdict =
        "NEUTRAL_TRIMER_PAIR_COMMON_ACTION_CONSTRUCTIVE_MOMENTUM_CHANNEL_MISSING";
  } else if (summary.common_one_step_pass && summary.repeated_pass
      && !summary.inward_response_pass) {
    summary.verdict = "NEUTRAL_TRIMER_PAIR_NONATTRACTIVE_SELECTED_DYNAMICS";
  } else if (summary.common_one_step_pass) {
    summary.verdict = "NEUTRAL_TRIMER_PAIR_ONE_STEP_ONLY";
  } else if (physical_one_step_failure) {
    summary.verdict = "NEUTRAL_TRIMER_PAIR_ATOMIC_TRANSACTION_CLOSED_NEGATIVE";
  } else {
    summary.verdict = "NEUTRAL_TRIMER_PAIR_UNRESOLVED";
  }

  int failures = 0;
  const auto check = [&](bool condition, const std::string& label) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << label << '\n';
    if (!condition) ++failures;
  };
  check(summary.one_step_forward_arms == 20,
        "all 20 locked forward fixtures were attempted");
  check(summary.fail_closed_controls,
        "invalid projection and impossible solve budget fail closed");
  check(!summary.verdict.empty(), "campaign produced a classified verdict");
  check(!(summary.repeated_run && !summary.common_one_step_pass),
        "repeated campaign obeys the common-action stop rule");
  check(numerical_one_step_failure || physical_one_step_failure
        || summary.common_one_step_pass,
        "one-step outcome has a complete classification");

  write_record(summary);
  std::cout << "protocol_sha256=" << protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "one_step_forward_arms=" << summary.one_step_forward_arms << '\n'
            << "one_step_reverse_arms=" << summary.one_step_reverse_arms << '\n'
            << "one_step_forward_converged=" << summary.one_step_forward_converged << '\n'
            << "one_step_reverse_converged=" << summary.one_step_reverse_converged << '\n'
            << "common_one_step_pass=" << summary.common_one_step_pass << '\n'
            << "momentum_pass=" << summary.momentum_pass << '\n'
            << "inward_response_pass=" << summary.inward_response_pass << '\n'
            << "worst_one_step_gate=" << summary.worst_one_step_gate << '\n'
            << "worst_one_step_inverse=" << summary.worst_one_step_inverse << '\n'
            << "worst_pseudomomentum_defect=" << summary.worst_pseudomomentum_defect << '\n'
            << "worst_cumulative_pseudomomentum_drift="
            << summary.worst_cumulative_pseudomomentum_drift << '\n'
            << "rest_inward_impulse=" << summary.rest_inward_impulse << '\n'
            << "rest_separation_before=" << summary.rest_separation_before << '\n'
            << "rest_separation_after_one_step="
            << summary.rest_separation_after_one_step << '\n'
            << "rest_separation_after_repeated="
            << summary.rest_separation_after_repeated << '\n'
            << "repeated_forward_steps=" << summary.repeated_forward_steps << '\n'
            << "repeated_reverse_steps=" << summary.repeated_reverse_steps << '\n'
            << "site_hops=" << summary.site_hops << '\n'
            << "repeated_state_recovery=" << summary.repeated_state_recovery << '\n'
            << "repeated_energy_drift=" << summary.repeated_energy_drift << '\n'
            << "failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}

