/** FTD-0600: constituent-complete charged-trimer common-action gate. */

#include "ftd/eft/constituent_complete_charged_trimer.h"

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
    "F24CC0BFBF0741B0F1A07DCE3B719EA6452E3DC81BB0E9F76013F211D25F6328";

using ftd::Coord;
using ftd::Vec3;
using ftd::eft::ChargedTrimerState;
using ftd::eft::ChargedTrimerStepResult;

struct Fixture {
  ChargedTrimerState state{L};
  std::vector<double> stationary_density = std::vector<double>(
      static_cast<std::size_t>(L) * L * L, 0.0);
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

Fixture make_fixture(int net_charge, const Vec3& common_velocity) {
  Fixture fixture;
  fixture.state.charges = net_charge < 0
      ? std::array<int, 3>{{-1, -1, +1}}
      : std::array<int, 3>{{+1, +1, -1}};
  const std::array<Coord, 3> anchors{{
      {7, 7, 7}, {8, 8, 7}, {8, 7, 8}}};
  const Vec3 remainder{0.173, -0.219, 0.287};
  const Vec3 momentum = ftd::eft::production_flat_momentum(common_velocity);
  for (std::size_t a = 0; a < anchors.size(); ++a) {
    fixture.state.constituents[a].anchor = anchors[a];
    fixture.state.constituents[a].remainder = remainder;
    fixture.state.constituents[a].momentum = momentum;
  }
  const Coord sink{2, 3, 1};
  const int sink_index = fixture.state.electric.index(sink.x, sink.y, sink.z);
  fixture.stationary_density[static_cast<std::size_t>(sink_index)] =
      -net_charge;
  bool seeded = true;
  std::vector<double> density = fixture.stationary_density;
  for (std::size_t a = 0; a < anchors.size(); ++a) {
    const auto coat = ftd::eft::make_quadratic_polarity_coat(
        effective_position(fixture.state.constituents[a]),
        fixture.state.charges[a]);
    seeded = seeded && coat.valid;
    for (std::size_t item = 0; item < coat.weight_count; ++item) {
      const auto& weight = coat.weights[item];
      const int source_index = fixture.state.electric.index(
          weight.site.x, weight.site.y, weight.site.z);
      seeded = seeded && ftd::eft::seed_dipole_path(
          fixture.state.electric, source_index, sink_index, weight.weight);
      density[static_cast<std::size_t>(source_index)] += weight.weight;
    }
  }
  fixture.valid = seeded
      && ftd::eft::max_fractional_gauss_residual(
          fixture.state.electric, density) <= gate;
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
        target.state.magnetic_half.x[to] =
            source.state.magnetic_half.x[from];
        target.state.magnetic_half.y[to] =
            source.state.magnetic_half.y[from];
        target.state.magnetic_half.z[to] =
            source.state.magnetic_half.z[from];
        target.stationary_density[to] = source.stationary_density[from];
      }
  target.valid = source.valid;
  return target;
}

ChargedTrimerState translate_state(const ChargedTrimerState& source,
                                   const Coord& shift) {
  Fixture fixture;
  fixture.state = source;
  Fixture translated = translate_fixture(fixture, shift);
  return translated.state;
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
        target.state.magnetic_half.x[to] =
            source.state.magnetic_half.y[from];
        target.state.magnetic_half.y[to] =
            source.state.magnetic_half.z[from];
        target.state.magnetic_half.z[to] =
            source.state.magnetic_half.x[from];
        target.stationary_density[to] = source.stationary_density[from];
      }
  target.valid = source.valid;
  return target;
}

ChargedTrimerState cyclic_state(const ChargedTrimerState& source) {
  Fixture fixture;
  fixture.state = source;
  return cyclic_fixture(fixture).state;
}

Fixture swapped_fixture(const Fixture& source) {
  Fixture target = source;
  std::swap(target.state.constituents[0], target.state.constituents[1]);
  std::swap(target.state.charges[0], target.state.charges[1]);
  return target;
}

ChargedTrimerState swapped_state(const ChargedTrimerState& source) {
  ChargedTrimerState target = source;
  std::swap(target.constituents[0], target.constituents[1]);
  std::swap(target.charges[0], target.charges[1]);
  return target;
}

double maximum_gate(const ChargedTrimerStepResult& result) {
  return std::max({result.root_residual, result.continuity_residual,
      result.gauss_before_residual, result.gauss_after_residual,
      result.force_residual, result.kinematic_residual,
      result.kinetic_discrete_gradient_residual,
      result.electric_adjoint_residual, result.magnetic_work_residual,
      result.binding_work_residual, result.binding_impulse_sum_residual,
      result.matter_work_residual, result.field_work_residual,
      result.total_energy_residual, result.causal_speed_excess});
}

double total_energy_before(const ChargedTrimerStepResult& result) {
  return result.kinetic_energy_before + result.binding_energy_before
      + result.field_energy_before;
}

double total_energy_after(const ChargedTrimerStepResult& result) {
  return result.kinetic_energy_after + result.binding_energy_after
      + result.field_energy_after;
}

int count_anchor_changes(const ChargedTrimerState& before,
                         const ChargedTrimerState& after) {
  int changes = 0;
  for (std::size_t a = 0; a < before.constituents.size(); ++a) {
    const auto& lhs = before.constituents[a].anchor;
    const auto& rhs = after.constituents[a].anchor;
    if (lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z) ++changes;
  }
  return changes;
}

struct Summary {
  int one_step_forward_arms = 0;
  int one_step_reverse_arms = 0;
  int one_step_forward_converged = 0;
  int one_step_reverse_converged = 0;
  int repeated_forward_steps = 0;
  int repeated_reverse_steps = 0;
  int site_hops = 0;
  bool one_step_pass = true;
  bool repeated_run = false;
  bool repeated_pass = true;
  bool fail_closed_controls = true;
  double worst_one_step_gate = 0.0;
  double worst_one_step_inverse = 0.0;
  double worst_translation = 0.0;
  double worst_rotation = 0.0;
  double worst_permutation = 0.0;
  double worst_pseudomomentum_defect = 0.0;
  double worst_repeated_gate = 0.0;
  double repeated_state_recovery = 0.0;
  double repeated_energy_drift = 0.0;
  double minimum_pair_distance = INFINITY;
  double maximum_pair_distance = 0.0;
  std::string verdict;
};

void write_record(const Summary& summary) {
  const std::filesystem::path result_dir =
      std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0600";
  std::filesystem::create_directories(result_dir);
  const auto json_path = result_dir / "ftd_0600_charged_trimer_v1.json";
  const auto csv_path = result_dir / "ftd_0600_charged_trimer_v1.csv";
  std::ofstream json(json_path);
  json << std::setprecision(17)
       << "{\n"
       << "  \"ftd_id\": \"FTD-0600\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"one_step_forward_arms\": "
       << summary.one_step_forward_arms << ",\n"
       << "  \"one_step_reverse_arms\": "
       << summary.one_step_reverse_arms << ",\n"
       << "  \"one_step_forward_converged\": "
       << summary.one_step_forward_converged << ",\n"
       << "  \"one_step_reverse_converged\": "
       << summary.one_step_reverse_converged << ",\n"
       << "  \"one_step_pass\": "
       << (summary.one_step_pass ? "true" : "false") << ",\n"
       << "  \"repeated_run\": "
       << (summary.repeated_run ? "true" : "false") << ",\n"
       << "  \"repeated_pass\": "
       << (summary.repeated_pass ? "true" : "false") << ",\n"
       << "  \"repeated_forward_steps\": "
       << summary.repeated_forward_steps << ",\n"
       << "  \"repeated_reverse_steps\": "
       << summary.repeated_reverse_steps << ",\n"
       << "  \"site_hops\": " << summary.site_hops << ",\n"
       << "  \"worst_one_step_gate\": "
       << summary.worst_one_step_gate << ",\n"
       << "  \"worst_one_step_inverse\": "
       << summary.worst_one_step_inverse << ",\n"
       << "  \"worst_translation_residual\": "
       << summary.worst_translation << ",\n"
       << "  \"worst_rotation_residual\": "
       << summary.worst_rotation << ",\n"
       << "  \"worst_permutation_residual\": "
       << summary.worst_permutation << ",\n"
       << "  \"worst_pseudomomentum_defect\": "
       << summary.worst_pseudomomentum_defect << ",\n"
       << "  \"worst_repeated_gate\": "
       << summary.worst_repeated_gate << ",\n"
       << "  \"repeated_state_recovery\": "
       << summary.repeated_state_recovery << ",\n"
       << "  \"repeated_energy_drift\": "
       << summary.repeated_energy_drift << ",\n"
       << "  \"minimum_pair_distance\": "
       << summary.minimum_pair_distance << ",\n"
       << "  \"maximum_pair_distance\": "
       << summary.maximum_pair_distance << ",\n"
       << "  \"production_state_defaults_rng_toggles_scenarios_changed\": false\n"
       << "}\n";
  std::ofstream csv(csv_path);
  csv << "ftd_id,verdict,one_step_forward_arms,one_step_reverse_arms,"
         "one_step_pass,repeated_run,repeated_pass,repeated_forward_steps,"
         "repeated_reverse_steps,site_hops,worst_one_step_gate,"
         "worst_one_step_inverse,worst_translation_residual,"
         "worst_rotation_residual,worst_permutation_residual,"
         "worst_pseudomomentum_defect,worst_repeated_gate,"
         "repeated_state_recovery,repeated_energy_drift,"
         "minimum_pair_distance,maximum_pair_distance\n";
  csv << std::setprecision(17)
      << "FTD-0600," << summary.verdict << ','
      << summary.one_step_forward_arms << ','
      << summary.one_step_reverse_arms << ','
      << summary.one_step_pass << ',' << summary.repeated_run << ','
      << summary.repeated_pass << ',' << summary.repeated_forward_steps << ','
      << summary.repeated_reverse_steps << ',' << summary.site_hops << ','
      << summary.worst_one_step_gate << ','
      << summary.worst_one_step_inverse << ','
      << summary.worst_translation << ',' << summary.worst_rotation << ','
      << summary.worst_permutation << ','
      << summary.worst_pseudomomentum_defect << ','
      << summary.worst_repeated_gate << ','
      << summary.repeated_state_recovery << ','
      << summary.repeated_energy_drift << ','
      << summary.minimum_pair_distance << ','
      << summary.maximum_pair_distance << '\n';
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  ftd::eft::ChargedTrimerOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-13;
  options.max_iterations = 64;

  Summary summary;
  const std::array<Vec3, 4> velocities{{
      {0.0, 0.0, 0.0}, {0.06, 0.0, 0.0},
      {0.05, 0.04, 0.0}, {0.04, 0.05, -0.03}}};
  const Coord shift{2, -1, 3};
  bool physical_one_step_failure = false;
  bool numerical_one_step_failure = false;

  for (int net_charge : {-1, +1}) {
    for (const Vec3& velocity : velocities) {
      const Fixture base_fixture = make_fixture(net_charge, velocity);
      const Fixture translated_fixture = translate_fixture(base_fixture, shift);
      const Fixture rotated_fixture = cyclic_fixture(base_fixture);
      const Fixture permuted_fixture = swapped_fixture(base_fixture);
      const std::array<const Fixture*, 4> fixtures{{
          &base_fixture, &translated_fixture,
          &rotated_fixture, &permuted_fixture}};
      std::array<ChargedTrimerStepResult, 4> forward{};
      std::array<ChargedTrimerStepResult, 4> reverse{};
      for (std::size_t arm = 0; arm < fixtures.size(); ++arm) {
        forward[arm] = ftd::eft::solve_charged_trimer_forward(
            fixtures[arm]->state, fixtures[arm]->stationary_density, options);
        ++summary.one_step_forward_arms;
        if (forward[arm].solve.converged)
          ++summary.one_step_forward_converged;
        summary.worst_one_step_gate = std::max(
            summary.worst_one_step_gate, maximum_gate(forward[arm]));
        summary.worst_pseudomomentum_defect = std::max(
            summary.worst_pseudomomentum_defect,
            forward[arm].pseudomomentum_defect_norm);
        summary.minimum_pair_distance = std::min(
            summary.minimum_pair_distance, forward[arm].minimum_pair_distance);
        summary.maximum_pair_distance = std::max(
            summary.maximum_pair_distance, forward[arm].maximum_pair_distance);
        if (!fixtures[arm]->valid || !forward[arm].solve.converged) {
          numerical_one_step_failure = true;
          summary.one_step_pass = false;
          continue;
        }
        if (!forward[arm].gates_pass) {
          physical_one_step_failure = true;
          summary.one_step_pass = false;
        }
        reverse[arm] = ftd::eft::solve_charged_trimer_reverse(
            forward[arm].later, fixtures[arm]->stationary_density, options);
        ++summary.one_step_reverse_arms;
        if (reverse[arm].solve.converged)
          ++summary.one_step_reverse_converged;
        if (!reverse[arm].solve.converged) {
          numerical_one_step_failure = true;
          summary.one_step_pass = false;
          continue;
        }
        const double inverse = ftd::eft::charged_trimer_state_max_difference(
            fixtures[arm]->state, reverse[arm].earlier);
        summary.worst_one_step_inverse = std::max(
            summary.worst_one_step_inverse, inverse);
        if (!reverse[arm].gates_pass || inverse > 1e-10) {
          physical_one_step_failure = true;
          summary.one_step_pass = false;
        }
      }
      if (std::all_of(forward.begin(), forward.end(),
          [](const auto& result) { return result.valid; })) {
        const ChargedTrimerState translated_back = translate_state(
            forward[1].later, {-shift.x, -shift.y, -shift.z});
        summary.worst_translation = std::max(summary.worst_translation,
            ftd::eft::charged_trimer_state_max_difference(
                forward[0].later, translated_back));
        const ChargedTrimerState rotated_back = cyclic_state(
            cyclic_state(forward[2].later));
        summary.worst_rotation = std::max(summary.worst_rotation,
            ftd::eft::charged_trimer_state_max_difference(
                forward[0].later, rotated_back));
        const ChargedTrimerState permuted_back = swapped_state(
            forward[3].later);
        summary.worst_permutation = std::max(summary.worst_permutation,
            ftd::eft::charged_trimer_state_max_difference(
                forward[0].later, permuted_back));
      }
    }
  }
  if (summary.worst_translation > gate || summary.worst_rotation > gate
      || summary.worst_permutation > gate) {
    physical_one_step_failure = true;
    summary.one_step_pass = false;
  }

  Fixture invalid = make_fixture(-1, velocities[3]);
  invalid.state.constituents[1].anchor =
      invalid.state.constituents[0].anchor;
  invalid.state.constituents[1].remainder =
      invalid.state.constituents[0].remainder;
  const auto invalid_result = ftd::eft::solve_charged_trimer_forward(
      invalid.state, invalid.stationary_density, options);
  auto impossible_options = options;
  impossible_options.solve_tolerance = 1e-30;
  impossible_options.max_iterations = 1;
  const Fixture impossible_fixture = make_fixture(+1, velocities[3]);
  const auto impossible_result = ftd::eft::solve_charged_trimer_forward(
      impossible_fixture.state, impossible_fixture.stationary_density,
      impossible_options);
  summary.fail_closed_controls = !invalid_result.valid
      && !invalid_result.gates_pass && impossible_result.solve.attempted
      && !impossible_result.solve.converged && !impossible_result.gates_pass;

  if (summary.one_step_pass) {
    summary.repeated_run = true;
    for (int net_charge : {-1, +1}) {
      const Fixture fixture = make_fixture(net_charge, velocities[3]);
      ChargedTrimerState state = fixture.state;
      const ChargedTrimerState initial = state;
      double energy_initial = NAN;
      double energy_final = NAN;
      bool arm_pass = fixture.valid;
      for (int tick = 0; tick < 32 && arm_pass; ++tick) {
        const auto step = ftd::eft::solve_charged_trimer_forward(
            state, fixture.stationary_density, options);
        ++summary.repeated_forward_steps;
        arm_pass = step.solve.converged && step.valid
            && maximum_gate(step) <= 1e-10;
        summary.worst_repeated_gate = std::max(
            summary.worst_repeated_gate, maximum_gate(step));
        summary.worst_pseudomomentum_defect = std::max(
            summary.worst_pseudomomentum_defect,
            step.pseudomomentum_defect_norm);
        summary.minimum_pair_distance = std::min(
            summary.minimum_pair_distance, step.minimum_pair_distance);
        summary.maximum_pair_distance = std::max(
            summary.maximum_pair_distance, step.maximum_pair_distance);
        if (!arm_pass) break;
        if (tick == 0) energy_initial = total_energy_before(step);
        energy_final = total_energy_after(step);
        summary.site_hops += count_anchor_changes(state, step.later);
        state = step.later;
      }
      for (int tick = 0; tick < 32 && arm_pass; ++tick) {
        const auto step = ftd::eft::solve_charged_trimer_reverse(
            state, fixture.stationary_density, options);
        ++summary.repeated_reverse_steps;
        arm_pass = step.solve.converged && step.valid
            && maximum_gate(step) <= 1e-10;
        summary.worst_repeated_gate = std::max(
            summary.worst_repeated_gate, maximum_gate(step));
        if (!arm_pass) break;
        state = step.earlier;
      }
      const double recovery = ftd::eft::charged_trimer_state_max_difference(
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
        && summary.repeated_forward_steps == 64
        && summary.repeated_reverse_steps == 64
        && summary.site_hops > 0;
  }

  if (summary.one_step_pass && summary.repeated_pass) {
    summary.verdict = "CHARGED_TRIMER_COMMON_ACTION_CONSTRUCTIVE";
  } else if (summary.one_step_pass) {
    summary.verdict = "CHARGED_TRIMER_ONE_STEP_ONLY";
  } else if (physical_one_step_failure) {
    summary.verdict = "CHARGED_TRIMER_ATOMIC_TRANSACTION_CLOSED_NEGATIVE";
  } else {
    summary.verdict = "CHARGED_TRIMER_TRANSACTION_UNRESOLVED";
  }

  int failures = 0;
  const auto check = [&](bool condition, const std::string& label) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << label << '\n';
    if (!condition) ++failures;
  };
  check(summary.one_step_forward_arms == 32,
        "all 32 locked forward one-step fixtures were attempted");
  check(summary.fail_closed_controls,
        "invalid projection and impossible solve budget fail closed");
  check(summary.verdict == "CHARGED_TRIMER_COMMON_ACTION_CONSTRUCTIVE"
        || summary.verdict == "CHARGED_TRIMER_ONE_STEP_ONLY"
        || summary.verdict == "CHARGED_TRIMER_ATOMIC_TRANSACTION_CLOSED_NEGATIVE"
        || summary.verdict == "CHARGED_TRIMER_TRANSACTION_UNRESOLVED",
        "verdict belongs to the preregistered set");
  check(!(summary.repeated_run && !summary.one_step_pass),
        "repeated campaign obeys the one-step stop rule");
  check(!summary.verdict.empty(), "campaign produced a classified verdict");

  write_record(summary);
  std::cout << "protocol_sha256=" << protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "one_step_forward_arms=" << summary.one_step_forward_arms << '\n'
            << "one_step_reverse_arms=" << summary.one_step_reverse_arms << '\n'
            << "one_step_forward_converged="
            << summary.one_step_forward_converged << '\n'
            << "one_step_reverse_converged="
            << summary.one_step_reverse_converged << '\n'
            << "one_step_pass=" << summary.one_step_pass << '\n'
            << "worst_one_step_gate=" << summary.worst_one_step_gate << '\n'
            << "worst_one_step_inverse="
            << summary.worst_one_step_inverse << '\n'
            << "worst_translation_residual="
            << summary.worst_translation << '\n'
            << "worst_rotation_residual=" << summary.worst_rotation << '\n'
            << "worst_permutation_residual="
            << summary.worst_permutation << '\n'
            << "worst_pseudomomentum_defect="
            << summary.worst_pseudomomentum_defect << '\n'
            << "repeated_run=" << summary.repeated_run << '\n'
            << "repeated_pass=" << summary.repeated_pass << '\n'
            << "repeated_forward_steps="
            << summary.repeated_forward_steps << '\n'
            << "repeated_reverse_steps="
            << summary.repeated_reverse_steps << '\n'
            << "site_hops=" << summary.site_hops << '\n'
            << "repeated_state_recovery="
            << summary.repeated_state_recovery << '\n'
            << "repeated_energy_drift="
            << summary.repeated_energy_drift << '\n'
            << "minimum_pair_distance="
            << summary.minimum_pair_distance << '\n'
            << "maximum_pair_distance="
            << summary.maximum_pair_distance << '\n'
            << "failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}
