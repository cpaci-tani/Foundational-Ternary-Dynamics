/** FTD-0602: minimum-energy neutral-pair force-sign discriminator. */

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
    "1ECB8957CCBA4AE5770FDB310E883357F745418DD36AD30CD5C7E7D35366F341";

using ftd::Coord;
using ftd::Vec3;
using ftd::eft::ClosedNeutralTrimerPairState;
using ftd::eft::ClosedNeutralTrimerPairStepResult;

int wrap(int value) {
  const int remainder = value % L;
  return remainder < 0 ? remainder + L : remainder;
}

int index(int x, int y, int z) {
  return (wrap(x) * L + wrap(y)) * L + wrap(z);
}

Vec3 effective_position(const ftd::eft::MatchedMatterPoint& point) {
  return {point.anchor.x + point.remainder.x,
          point.anchor.y + point.remainder.y,
          point.anchor.z + point.remainder.z};
}

Vec3 cyclic(const Vec3& value) {
  return {value.y, value.z, value.x};
}

std::vector<double> density_of(const ClosedNeutralTrimerPairState& state) {
  std::vector<double> density(static_cast<std::size_t>(L) * L * L, 0.0);
  for (std::size_t a = 0; a < state.constituents.size(); ++a) {
    const auto coat = ftd::eft::make_quadratic_polarity_coat(
        effective_position(state.constituents[a]), state.charges[a]);
    if (!coat.valid) return {};
    for (std::size_t item = 0; item < coat.weight_count; ++item) {
      const auto& weight = coat.weights[item];
      density[static_cast<std::size_t>(index(
          weight.site.x, weight.site.y, weight.site.z))] += weight.weight;
    }
  }
  return density;
}

long double dot(const std::vector<double>& lhs,
                const std::vector<double>& rhs) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result += static_cast<long double>(lhs[i]) * rhs[i];
  return result;
}

void apply_ddt(const std::vector<double>& scalar,
               std::vector<double>& result) {
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int i = index(x, y, z);
        result[static_cast<std::size_t>(i)] =
            6.0 * scalar[static_cast<std::size_t>(i)]
            - scalar[static_cast<std::size_t>(index(x + 1, y, z))]
            - scalar[static_cast<std::size_t>(index(x - 1, y, z))]
            - scalar[static_cast<std::size_t>(index(x, y + 1, z))]
            - scalar[static_cast<std::size_t>(index(x, y - 1, z))]
            - scalar[static_cast<std::size_t>(index(x, y, z + 1))]
            - scalar[static_cast<std::size_t>(index(x, y, z - 1))];
      }
}

struct MinimumEnergyInitialization {
  bool valid = false;
  int iterations = 0;
  double solver_residual = INFINITY;
  double gauss_residual = INFINITY;
  double curl_adjoint_residual = INFINITY;
  double energy = INFINITY;
  ftd::eft::MatchedFaceFlux electric{L};
};

MinimumEnergyInitialization initialize_minimum_energy(
    const std::vector<double>& density) {
  MinimumEnergyInitialization result;
  const std::size_t count = static_cast<std::size_t>(L) * L * L;
  if (density.size() != count) return result;
  long double total = 0.0L;
  for (double value : density) total += value;
  if (std::abs(static_cast<double>(total)) > 1e-12) return result;
  std::vector<double> phi(count, 0.0);
  std::vector<double> residual = density;
  std::vector<double> direction = density;
  std::vector<double> image(count, 0.0);
  long double rr = dot(residual, residual);
  for (int iteration = 1; iteration <= 20 * L; ++iteration) {
    apply_ddt(direction, image);
    const long double p_ap = dot(direction, image);
    if (!(p_ap > 0.0L)) break;
    const long double alpha = rr / p_ap;
    for (std::size_t i = 0; i < count; ++i) {
      phi[i] += static_cast<double>(alpha * direction[i]);
      residual[i] -= static_cast<double>(alpha * image[i]);
    }
    result.iterations = iteration;
    result.solver_residual = 0.0;
    for (double value : residual)
      result.solver_residual = std::max(result.solver_residual,
                                        std::abs(value));
    if (result.solver_residual <= 1e-13) break;
    const long double next = dot(residual, residual);
    const long double beta = next / rr;
    for (std::size_t i = 0; i < count; ++i)
      direction[i] = residual[i] + static_cast<double>(beta * direction[i]);
    rr = next;
  }
  if (result.solver_residual > 1e-13) return result;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int i = index(x, y, z);
        result.electric.x[static_cast<std::size_t>(i)] =
            phi[static_cast<std::size_t>(i)]
            - phi[static_cast<std::size_t>(index(x + 1, y, z))];
        result.electric.y[static_cast<std::size_t>(i)] =
            phi[static_cast<std::size_t>(i)]
            - phi[static_cast<std::size_t>(index(x, y + 1, z))];
        result.electric.z[static_cast<std::size_t>(i)] =
            phi[static_cast<std::size_t>(i)]
            - phi[static_cast<std::size_t>(index(x, y, z + 1))];
      }
  result.gauss_residual = ftd::eft::max_fractional_gauss_residual(
      result.electric, density);
  result.curl_adjoint_residual = ftd::eft::max_curl_adjoint(result.electric);
  result.energy = ftd::eft::quadratic_energy(result.electric);
  result.valid = result.gauss_residual <= gate
      && result.curl_adjoint_residual <= gate;
  return result;
}

struct Fixture {
  ClosedNeutralTrimerPairState state{L};
  MinimumEnergyInitialization initialization{};
  bool valid = false;
};

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
  fixture.initialization = initialize_minimum_energy(density_of(fixture.state));
  fixture.state.electric = fixture.initialization.electric;
  fixture.valid = fixture.initialization.valid;
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
      }
  target.initialization = source.initialization;
  target.initialization.electric = target.state.electric;
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
      }
  target.initialization = source.initialization;
  target.initialization.electric = target.state.electric;
  target.valid = source.valid;
  return target;
}

ClosedNeutralTrimerPairState cyclic_state(
    const ClosedNeutralTrimerPairState& source) {
  Fixture fixture;
  fixture.state = source;
  return cyclic_fixture(fixture).state;
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
  }
  target.initialization = source.initialization;
  target.initialization.electric = target.state.electric;
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

struct Summary {
  int forward_arms = 0;
  int reverse_arms = 0;
  int repeated_forward_steps = 0;
  int repeated_reverse_steps = 0;
  int site_hops = 0;
  bool initializer_pass = true;
  bool minimum_control_pass = false;
  bool common_pass = true;
  bool momentum_pass = true;
  bool sign_pass = false;
  bool repeated_pass = false;
  double initializer_solver_residual = 0.0;
  double initializer_gauss_residual = 0.0;
  double initializer_curl_residual = 0.0;
  double transverse_gauss_residual = 0.0;
  double transverse_energy_gap = 0.0;
  double worst_common_gate = 0.0;
  double worst_inverse = 0.0;
  double worst_symmetry = 0.0;
  double worst_pseudomomentum_defect = 0.0;
  double state_recovery = 0.0;
  double energy_drift = 0.0;
  double inward_impulse = 0.0;
  double separation_before = 0.0;
  double separation_after_one_step = 0.0;
  double separation_after_repeated = 0.0;
  double minimum_internal_distance = INFINITY;
  double maximum_internal_distance = 0.0;
  std::string verdict;
};

void write_record(const Summary& s) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0602";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0602_minimum_energy_force_sign_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0602\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << s.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"forward_arms\": " << s.forward_arms << ",\n"
       << "  \"reverse_arms\": " << s.reverse_arms << ",\n"
       << "  \"initializer_pass\": " << (s.initializer_pass ? "true" : "false") << ",\n"
       << "  \"minimum_control_pass\": " << (s.minimum_control_pass ? "true" : "false") << ",\n"
       << "  \"common_pass\": " << (s.common_pass ? "true" : "false") << ",\n"
       << "  \"momentum_pass\": " << (s.momentum_pass ? "true" : "false") << ",\n"
       << "  \"sign_pass\": " << (s.sign_pass ? "true" : "false") << ",\n"
       << "  \"repeated_pass\": " << (s.repeated_pass ? "true" : "false") << ",\n"
       << "  \"repeated_forward_steps\": " << s.repeated_forward_steps << ",\n"
       << "  \"repeated_reverse_steps\": " << s.repeated_reverse_steps << ",\n"
       << "  \"site_hops\": " << s.site_hops << ",\n"
       << "  \"initializer_solver_residual\": " << s.initializer_solver_residual << ",\n"
       << "  \"initializer_gauss_residual\": " << s.initializer_gauss_residual << ",\n"
       << "  \"initializer_curl_residual\": " << s.initializer_curl_residual << ",\n"
       << "  \"transverse_gauss_residual\": " << s.transverse_gauss_residual << ",\n"
       << "  \"transverse_energy_gap\": " << s.transverse_energy_gap << ",\n"
       << "  \"worst_common_gate\": " << s.worst_common_gate << ",\n"
       << "  \"worst_inverse\": " << s.worst_inverse << ",\n"
       << "  \"worst_symmetry\": " << s.worst_symmetry << ",\n"
       << "  \"worst_pseudomomentum_defect\": " << s.worst_pseudomomentum_defect << ",\n"
       << "  \"state_recovery\": " << s.state_recovery << ",\n"
       << "  \"energy_drift\": " << s.energy_drift << ",\n"
       << "  \"inward_impulse\": " << s.inward_impulse << ",\n"
       << "  \"separation_before\": " << s.separation_before << ",\n"
       << "  \"separation_after_one_step\": " << s.separation_after_one_step << ",\n"
       << "  \"separation_after_repeated\": " << s.separation_after_repeated << ",\n"
       << "  \"minimum_internal_distance\": " << s.minimum_internal_distance << ",\n"
       << "  \"maximum_internal_distance\": " << s.maximum_internal_distance << "\n}\n";
  std::ofstream csv(dir / "ftd_0602_minimum_energy_force_sign_v1.csv");
  csv << "ftd_id,verdict,initializer_pass,minimum_control_pass,common_pass,"
         "momentum_pass,sign_pass,repeated_pass,inward_impulse,"
         "separation_before,separation_after_one_step,separation_after_repeated,"
         "worst_pseudomomentum_defect,state_recovery,energy_drift\n";
  csv << std::setprecision(17) << "FTD-0602," << s.verdict << ','
      << s.initializer_pass << ',' << s.minimum_control_pass << ','
      << s.common_pass << ',' << s.momentum_pass << ',' << s.sign_pass << ','
      << s.repeated_pass << ',' << s.inward_impulse << ','
      << s.separation_before << ',' << s.separation_after_one_step << ','
      << s.separation_after_repeated << ',' << s.worst_pseudomomentum_defect
      << ',' << s.state_recovery << ',' << s.energy_drift << '\n';
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  ftd::eft::ClosedNeutralPairOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-13;
  options.max_iterations = 64;
  const std::array<std::array<Vec3, 2>, 3> velocities{{
      {{{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}},
      {{{0.04, 0.0, 0.0}, {0.04, 0.0, 0.0}}},
      {{{0.04, 0.0, 0.0}, {-0.04, 0.0, 0.0}}}}};
  const Coord shift{1, 1, -2};
  Summary summary;

  const Fixture rest_fixture = make_fixture(velocities[0][0], velocities[0][1]);
  summary.initializer_solver_residual = rest_fixture.initialization.solver_residual;
  summary.initializer_gauss_residual = rest_fixture.initialization.gauss_residual;
  summary.initializer_curl_residual = rest_fixture.initialization.curl_adjoint_residual;
  summary.initializer_pass = rest_fixture.valid;
  const auto transverse_edge = ftd::eft::make_transverse_challenge(L, 1e-3);
  const auto transverse_face = ftd::eft::matched_curl(transverse_edge);
  auto challenged = rest_fixture.state.electric;
  for (std::size_t i = 0; i < challenged.x.size(); ++i) {
    challenged.x[i] += transverse_face.x[i];
    challenged.y[i] += transverse_face.y[i];
    challenged.z[i] += transverse_face.z[i];
  }
  summary.transverse_gauss_residual = ftd::eft::max_fractional_gauss_residual(
      challenged, density_of(rest_fixture.state));
  summary.transverse_energy_gap = ftd::eft::quadratic_energy(challenged)
      - ftd::eft::quadratic_energy(rest_fixture.state.electric);
  summary.minimum_control_pass = summary.transverse_gauss_residual <= gate
      && ftd::eft::quadratic_energy(transverse_face) > 0.0
      && summary.transverse_energy_gap > 0.0;

  for (std::size_t mode = 0; mode < velocities.size(); ++mode) {
    const Fixture base = make_fixture(velocities[mode][0], velocities[mode][1]);
    const Fixture translated = translate_fixture(base, shift);
    const Fixture rotated = cyclic_fixture(base);
    const Fixture conjugated = conjugate_exchange_fixture(base);
    const std::array<const Fixture*, 4> fixtures{{
        &base, &translated, &rotated, &conjugated}};
    std::array<ClosedNeutralTrimerPairStepResult, 4> forward{};
    for (std::size_t arm = 0; arm < fixtures.size(); ++arm) {
      summary.initializer_pass = summary.initializer_pass && fixtures[arm]->valid;
      forward[arm] = ftd::eft::solve_closed_neutral_pair_forward(
          fixtures[arm]->state, options);
      ++summary.forward_arms;
      summary.worst_common_gate = std::max(summary.worst_common_gate,
          maximum_common_gate(forward[arm]));
      summary.worst_pseudomomentum_defect = std::max(
          summary.worst_pseudomomentum_defect,
          forward[arm].pseudomomentum_defect_norm);
      summary.minimum_internal_distance = std::min(
          summary.minimum_internal_distance,
          forward[arm].minimum_internal_pair_distance);
      summary.maximum_internal_distance = std::max(
          summary.maximum_internal_distance,
          forward[arm].maximum_internal_pair_distance);
      summary.common_pass = summary.common_pass
          && forward[arm].common_action_gates_pass;
      summary.momentum_pass = summary.momentum_pass
          && forward[arm].isolated_momentum_gate_pass;
      if (!forward[arm].valid) continue;
      const auto reverse = ftd::eft::solve_closed_neutral_pair_reverse(
          forward[arm].later, options);
      ++summary.reverse_arms;
      const double inverse = reverse.valid
          ? ftd::eft::closed_neutral_pair_state_max_difference(
              fixtures[arm]->state, reverse.earlier)
          : INFINITY;
      summary.worst_inverse = std::max(summary.worst_inverse, inverse);
      summary.common_pass = summary.common_pass
          && reverse.common_action_gates_pass && inverse <= 1e-10;
    }
    if (std::all_of(forward.begin(), forward.end(),
        [](const auto& value) { return value.valid; })) {
      const auto translated_back = translate_state(
          forward[1].later, {-shift.x, -shift.y, -shift.z});
      const auto rotated_back = cyclic_state(cyclic_state(forward[2].later));
      const auto conjugated_back = conjugate_exchange_state(forward[3].later);
      summary.worst_symmetry = std::max({summary.worst_symmetry,
          ftd::eft::closed_neutral_pair_state_max_difference(
              forward[0].later, translated_back),
          ftd::eft::closed_neutral_pair_state_max_difference(
              forward[0].later, rotated_back),
          ftd::eft::closed_neutral_pair_state_max_difference(
              forward[0].later, conjugated_back)});
      if (mode == 0) {
        summary.inward_impulse = forward[0].inward_impulse;
        summary.separation_before = forward[0].center_separation_before;
        summary.separation_after_one_step = forward[0].center_separation_after;
      }
    }
  }
  summary.common_pass = summary.common_pass && summary.worst_symmetry <= gate;
  summary.sign_pass = summary.inward_impulse > 1e-10
      && summary.separation_after_one_step < summary.separation_before;

  if (summary.initializer_pass && summary.minimum_control_pass
      && summary.common_pass) {
    ClosedNeutralTrimerPairState state = rest_fixture.state;
    const auto initial = state;
    double energy_initial = NAN;
    double energy_final = NAN;
    bool pass = true;
    for (int tick = 0; tick < 16 && pass; ++tick) {
      const auto step = ftd::eft::solve_closed_neutral_pair_forward(state, options);
      ++summary.repeated_forward_steps;
      pass = step.valid && maximum_common_gate(step) <= 1e-10
          && step.minimum_internal_pair_distance >= 1.35
          && step.maximum_internal_pair_distance <= 1.48;
      if (!pass) break;
      if (tick == 0) energy_initial = step.kinetic_energy_before
          + step.binding_energy_before + step.field_energy_before;
      energy_final = step.kinetic_energy_after
          + step.binding_energy_after + step.field_energy_after;
      for (std::size_t a = 0; a < state.constituents.size(); ++a) {
        const auto& lhs = state.constituents[a].anchor;
        const auto& rhs = step.later.constituents[a].anchor;
        if (lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z)
          ++summary.site_hops;
      }
      summary.momentum_pass = summary.momentum_pass
          && step.isolated_momentum_gate_pass;
      summary.worst_pseudomomentum_defect = std::max(
          summary.worst_pseudomomentum_defect,
          step.pseudomomentum_defect_norm);
      if (tick == 15) summary.separation_after_repeated =
          step.center_separation_after;
      state = step.later;
    }
    for (int tick = 0; tick < 16 && pass; ++tick) {
      const auto step = ftd::eft::solve_closed_neutral_pair_reverse(state, options);
      ++summary.repeated_reverse_steps;
      pass = step.valid && maximum_common_gate(step) <= 1e-10;
      if (pass) state = step.earlier;
    }
    summary.state_recovery = ftd::eft::closed_neutral_pair_state_max_difference(
        initial, state);
    summary.energy_drift = std::abs(energy_final - energy_initial);
    summary.repeated_pass = pass && summary.repeated_forward_steps == 16
        && summary.repeated_reverse_steps == 16
        && summary.state_recovery <= 1e-8 && summary.energy_drift <= 1e-9;
  }

  if (summary.initializer_pass && summary.minimum_control_pass
      && summary.common_pass && summary.repeated_pass
      && summary.sign_pass && summary.momentum_pass) {
    summary.verdict = "MINIMUM_ENERGY_NEUTRAL_PAIR_CLOSED_DYNAMICS_CONSTRUCTIVE";
  } else if (summary.initializer_pass && summary.minimum_control_pass
      && summary.common_pass && summary.repeated_pass
      && summary.sign_pass && !summary.momentum_pass) {
    summary.verdict = "MINIMUM_ENERGY_ATTRACTION_RESTORED_MOMENTUM_CHANNEL_MISSING";
  } else if (summary.initializer_pass && summary.minimum_control_pass
      && summary.common_pass && !summary.sign_pass) {
    summary.verdict = "MINIMUM_ENERGY_FIELD_DOES_NOT_REPAIR_FORCE_SIGN";
  } else if (summary.initializer_pass && summary.minimum_control_pass
      && summary.common_pass) {
    summary.verdict = "MINIMUM_ENERGY_NEUTRAL_PAIR_ONE_STEP_ONLY";
  } else if (!summary.initializer_pass || !summary.minimum_control_pass
      || !summary.common_pass) {
    summary.verdict = "MINIMUM_ENERGY_NEUTRAL_PAIR_CLOSED_NEGATIVE";
  } else {
    summary.verdict = "MINIMUM_ENERGY_NEUTRAL_PAIR_UNRESOLVED";
  }

  int failures = 0;
  const auto check = [&](bool condition, const std::string& label) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << label << '\n';
    if (!condition) ++failures;
  };
  check(summary.forward_arms == 12, "all 12 locked forward arms attempted");
  check(summary.initializer_pass, "minimum-energy initializer closes");
  check(summary.minimum_control_pass, "transverse minimum-energy control closes");
  check(!summary.verdict.empty(), "campaign produced a locked verdict");
  check(!(summary.repeated_forward_steps > 0 && !summary.common_pass),
        "repeated campaign obeys the common-action stop rule");
  write_record(summary);
  std::cout << "protocol_sha256=" << protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "initializer_solver_residual=" << summary.initializer_solver_residual << '\n'
            << "initializer_gauss_residual=" << summary.initializer_gauss_residual << '\n'
            << "initializer_curl_residual=" << summary.initializer_curl_residual << '\n'
            << "transverse_energy_gap=" << summary.transverse_energy_gap << '\n'
            << "common_pass=" << summary.common_pass << '\n'
            << "momentum_pass=" << summary.momentum_pass << '\n'
            << "sign_pass=" << summary.sign_pass << '\n'
            << "inward_impulse=" << summary.inward_impulse << '\n'
            << "separation_before=" << summary.separation_before << '\n'
            << "separation_after_one_step=" << summary.separation_after_one_step << '\n'
            << "separation_after_repeated=" << summary.separation_after_repeated << '\n'
            << "worst_pseudomomentum_defect="
            << summary.worst_pseudomomentum_defect << '\n'
            << "state_recovery=" << summary.state_recovery << '\n'
            << "energy_drift=" << summary.energy_drift << '\n'
            << "failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}

