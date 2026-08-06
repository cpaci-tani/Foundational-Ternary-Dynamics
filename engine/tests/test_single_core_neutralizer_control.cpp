// FTD-0610: single compact core versus uniform and frozen neutralizers.
#define FTD0609_NO_MAIN
#include "test_shared_anchor_constituent_fibre_transport.cpp"

namespace {

constexpr char single_protocol_sha256[] =
    "DB4363D2A132BB84BFF10218FCE8B4B20BC4C677F6FE813815F368E38A4EED85";

using ftd::eft::ChargedTrimerState;
using ftd::eft::ChargedTrimerStepResult;

double single_maximum_gate(const ChargedTrimerStepResult& r) {
  return std::max({r.root_residual, r.continuity_residual,
      r.gauss_before_residual, r.gauss_after_residual, r.force_residual,
      r.kinematic_residual, r.kinetic_discrete_gradient_residual,
      r.electric_adjoint_residual, r.magnetic_work_residual,
      r.binding_work_residual, r.binding_impulse_sum_residual,
      r.matter_work_residual, r.field_work_residual,
      r.total_energy_residual, r.causal_speed_excess});
}

double single_energy_before(const ChargedTrimerStepResult& r) {
  return r.kinetic_energy_before + r.binding_energy_before
      + r.field_energy_before;
}

double single_energy_after(const ChargedTrimerStepResult& r) {
  return r.kinetic_energy_after + r.binding_energy_after
      + r.field_energy_after;
}

Vec3 single_center(const ChargedTrimerState& state) {
  Vec3 result{};
  for (const auto& point : state.constituents)
    result += effective_position(point);
  return result * (1.0 / 3.0);
}

Vec3 single_momentum(const ChargedTrimerState& state) {
  Vec3 result{};
  for (const auto& point : state.constituents) result += point.momentum;
  return result;
}

int single_anchor_changes(const ChargedTrimerState& before,
                          const ChargedTrimerState& after) {
  int result = 0;
  for (std::size_t a = 0; a < before.constituents.size(); ++a) {
    const auto& lhs = before.constituents[a].anchor;
    const auto& rhs = after.constituents[a].anchor;
    if (lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z) ++result;
  }
  return result;
}

int maximum_anchor_multiplicity(const ChargedTrimerState& state) {
  int result = 1;
  for (std::size_t a = 0; a < state.constituents.size(); ++a) {
    int count = 0;
    for (std::size_t b = 0; b < state.constituents.size(); ++b) {
      const auto& lhs = state.constituents[a].anchor;
      const auto& rhs = state.constituents[b].anchor;
      if (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z) ++count;
    }
    result = std::max(result, count);
  }
  return result;
}

std::vector<double> coat_density(const ChargedTrimerState& state) {
  std::vector<double> result(static_cast<std::size_t>(L) * L * L, 0.0);
  for (std::size_t a = 0; a < state.constituents.size(); ++a) {
    const auto coat = ftd::eft::make_quadratic_polarity_coat(
        effective_position(state.constituents[a]), state.charges[a]);
    if (!coat.valid) return {};
    for (std::size_t item = 0; item < coat.weight_count; ++item) {
      const auto& weight = coat.weights[item];
      result[static_cast<std::size_t>(index(
          weight.site.x, weight.site.y, weight.site.z))] += weight.weight;
    }
  }
  return result;
}

std::vector<double> frozen_partner_density(
    const ClosedNeutralTrimerPairState& state) {
  std::vector<double> result(static_cast<std::size_t>(L) * L * L, 0.0);
  for (std::size_t a = 0; a < 3; ++a) {
    const auto coat = ftd::eft::make_quadratic_polarity_coat(
        effective_position(state.constituents[a]), state.charges[a]);
    if (!coat.valid) return {};
    for (std::size_t item = 0; item < coat.weight_count; ++item) {
      const auto& weight = coat.weights[item];
      result[static_cast<std::size_t>(index(
          weight.site.x, weight.site.y, weight.site.z))] += weight.weight;
    }
  }
  return result;
}

double density_sum(const std::vector<double>& values) {
  long double result = 0.0L;
  for (double value : values) result += value;
  return static_cast<double>(result);
}

std::vector<double> translate_density_x(
    const std::vector<double>& source, int shift) {
  std::vector<double> target(source.size(), 0.0);
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z)
        target[static_cast<std::size_t>(index(x + shift, y, z))]
            = source[static_cast<std::size_t>(index(x, y, z))];
  return target;
}

ChargedTrimerState translate_single_x(
    const ChargedTrimerState& source, int shift) {
  ChargedTrimerState target(L);
  target.charges = source.charges;
  for (std::size_t a = 0; a < source.constituents.size(); ++a) {
    target.constituents[a] = source.constituents[a];
    target.constituents[a].anchor.x =
        (target.constituents[a].anchor.x + shift + L) % L;
  }
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int from = index(x, y, z);
        const int to = index(x + shift, y, z);
        target.electric.x[to] = source.electric.x[from];
        target.electric.y[to] = source.electric.y[from];
        target.electric.z[to] = source.electric.z[from];
        target.magnetic_half.x[to] = source.magnetic_half.x[from];
        target.magnetic_half.y[to] = source.magnetic_half.y[from];
        target.magnetic_half.z[to] = source.magnetic_half.z[from];
      }
  return target;
}

double diagnostic_difference(const ChargedTrimerStepResult& lhs,
                             const ChargedTrimerStepResult& rhs) {
  const std::array<double, 18> a{{lhs.kinetic_energy_before,
      lhs.kinetic_energy_after, lhs.binding_energy_before,
      lhs.binding_energy_after, lhs.field_energy_before,
      lhs.field_energy_after, lhs.current_work, lhs.root_residual,
      lhs.continuity_residual, lhs.gauss_before_residual,
      lhs.gauss_after_residual, lhs.force_residual, lhs.kinematic_residual,
      lhs.electric_adjoint_residual, lhs.magnetic_work_residual,
      lhs.matter_work_residual, lhs.field_work_residual,
      lhs.total_energy_residual}};
  const std::array<double, 18> b{{rhs.kinetic_energy_before,
      rhs.kinetic_energy_after, rhs.binding_energy_before,
      rhs.binding_energy_after, rhs.field_energy_before,
      rhs.field_energy_after, rhs.current_work, rhs.root_residual,
      rhs.continuity_residual, rhs.gauss_before_residual,
      rhs.gauss_after_residual, rhs.force_residual, rhs.kinematic_residual,
      rhs.electric_adjoint_residual, rhs.magnetic_work_residual,
      rhs.matter_work_residual, rhs.field_work_residual,
      rhs.total_energy_residual}};
  double result = 0.0;
  for (std::size_t i = 0; i < a.size(); ++i)
    result = std::max(result, std::abs(a[i] - b[i]));
  return result;
}

struct NeutralizerFixture {
  std::string name;
  ChargedTrimerState state{L};
  std::vector<double> stationary{};
  bool valid = false;
  double moving_charge = INFINITY;
  double stationary_charge = INFINITY;
  double total_charge = INFINITY;
  double poisson = INFINITY;
  double gauss = INFINITY;
  double curl = INFINITY;
  double energy_crosscheck = INFINITY;
};

NeutralizerFixture make_fixture(const ClosedNeutralTrimerPairState& pair,
                                bool uniform) {
  NeutralizerFixture result;
  result.name = uniform ? "uniform" : "frozen_partner";
  for (std::size_t a = 0; a < 3; ++a) {
    result.state.constituents[a] = pair.constituents[a + 3];
    result.state.charges[a] = pair.charges[a + 3];
  }
  const std::size_t count = static_cast<std::size_t>(L) * L * L;
  if (uniform)
    result.stationary.assign(count, -1.0 / static_cast<double>(count));
  else
    result.stationary = frozen_partner_density(pair);
  auto total = coat_density(result.state);
  if (total.size() != count || result.stationary.size() != count)
    return result;
  result.moving_charge = density_sum(total);
  result.stationary_charge = density_sum(result.stationary);
  for (std::size_t i = 0; i < count; ++i) total[i] += result.stationary[i];
  result.total_charge = density_sum(total);
  const auto field = initialize_minimum_energy(total);
  result.poisson = field.solver_residual;
  result.gauss = field.gauss_residual;
  result.curl = field.curl_residual;
  if (field.valid) {
    result.state.electric = field.electric;
    const auto normalization = ftd::eft::measure_face_flux_normalization();
    const double lambda = ftd::C_SPEED;
    result.energy_crosscheck = std::abs(
        normalization.mapped_field_work_coefficient
            * ftd::eft::matched_modified_energy(
                result.state.electric, result.state.magnetic_half, lambda)
        - normalization.mapped_field_work_coefficient * field.raw_energy);
  }
  result.valid = field.valid
      && std::abs(result.moving_charge - 1.0) <= 1e-12
      && std::abs(result.stationary_charge + 1.0) <= 1e-12
      && std::abs(result.total_charge) <= 1e-12
      && result.poisson <= 1e-11 && result.gauss <= 1e-11
      && result.curl <= 1e-11 && result.energy_crosscheck <= 1e-11;
  return result;
}

struct SingleTick {
  int control = 0, arm = 0, direction = 0, tick = 0;
  bool valid = false;
  double gate_value = INFINITY, energy_drift = INFINITY;
  double momentum_defect = INFINITY, min_distance = 0.0;
  double max_distance = INFINITY;
  int multiplicity = 0;
};

struct SingleArm {
  bool pass = false, complete = false, rest = false;
  int ticks = 0, forward = 0, reverse = 0, hops = 0, shared = 0;
  int max_multiplicity = 1;
  double velocity = 0.0, nominal = 0.0, longitudinal = INFINITY;
  double transverse = INFINITY, momentum_change = INFINITY;
  double min_distance = INFINITY, max_distance = 0.0;
  double worst_gate = 0.0, energy_drift = 0.0;
  double momentum_defect = 0.0, recovery = INFINITY;
  std::vector<SingleTick> records{};
};

void observe_state(SingleArm& result, const ChargedTrimerState& state) {
  const int multiplicity = maximum_anchor_multiplicity(state);
  result.max_multiplicity = std::max(result.max_multiplicity, multiplicity);
  if (multiplicity > 1) ++result.shared;
}

void observe_step(SingleArm& result, SingleTick& row,
                  const ChargedTrimerStepResult& step,
                  const ChargedTrimerState& returned,
                  double baseline) {
  row.gate_value = single_maximum_gate(step);
  row.energy_drift = std::max(
      std::abs(single_energy_before(step) - baseline),
      std::abs(single_energy_after(step) - baseline));
  row.momentum_defect = step.pseudomomentum_defect_norm;
  row.min_distance = step.minimum_pair_distance;
  row.max_distance = step.maximum_pair_distance;
  row.multiplicity = maximum_anchor_multiplicity(returned);
  result.worst_gate = std::max(result.worst_gate, row.gate_value);
  result.energy_drift = std::max(result.energy_drift, row.energy_drift);
  result.momentum_defect = std::max(
      result.momentum_defect, row.momentum_defect);
  result.min_distance = std::min(result.min_distance, row.min_distance);
  result.max_distance = std::max(result.max_distance, row.max_distance);
  observe_state(result, returned);
}

SingleArm run_arm(int control, int arm, const NeutralizerFixture& fixture,
                  double velocity, int ticks,
                  const ftd::eft::ChargedTrimerOptions& options) {
  SingleArm result;
  result.rest = velocity == 0.0;
  result.velocity = velocity;
  result.ticks = ticks;
  result.nominal = velocity * ticks;
  ChargedTrimerState initial = fixture.state;
  const Vec3 launch = ftd::eft::production_flat_momentum(
      {velocity, 0.0, 0.0});
  for (auto& point : initial.constituents) point.momentum = launch;
  ChargedTrimerState current = initial;
  const Vec3 center0 = single_center(initial);
  const Vec3 momentum0 = single_momentum(initial);
  double baseline = NAN;
  for (int tick = 0; tick < ticks; ++tick) {
    const auto step = ftd::eft::solve_charged_trimer_forward(
        current, fixture.stationary, options);
    SingleTick row{control, arm, 1, tick};
    row.valid = step.valid;
    if (!step.valid) { result.records.push_back(row); break; }
    if (!std::isfinite(baseline)) baseline = single_energy_before(step);
    observe_step(result, row, step, step.later, baseline);
    ++result.forward;
    result.hops += single_anchor_changes(current, step.later);
    result.records.push_back(row);
    current = step.later;
  }
  if (result.forward == ticks) {
    const Vec3 displacement = single_center(current) - center0;
    result.longitudinal = displacement.x;
    result.transverse = std::sqrt(
        displacement.y * displacement.y + displacement.z * displacement.z);
    result.momentum_change = (single_momentum(current) - momentum0).mag();
    for (int tick = 0; tick < ticks; ++tick) {
      const auto step = ftd::eft::solve_charged_trimer_reverse(
          current, fixture.stationary, options);
      SingleTick row{control, arm, -1, tick};
      row.valid = step.valid;
      if (!step.valid) { result.records.push_back(row); break; }
      observe_step(result, row, step, step.earlier, baseline);
      ++result.reverse;
      result.records.push_back(row);
      current = step.earlier;
    }
  }
  result.complete = result.forward == ticks && result.reverse == ticks;
  if (result.complete)
    result.recovery =
        ftd::eft::charged_trimer_state_max_difference(initial, current);
  const bool common = result.complete && result.worst_gate <= gate
      && result.max_multiplicity <= 2 && result.min_distance >= 0.5
      && result.max_distance <= 2.0 && result.energy_drift <= 1e-10
      && result.recovery <= 1e-9;
  if (result.rest)
    result.pass = common && std::abs(result.longitudinal) <= 1e-10
        && result.transverse <= 1e-10 && result.momentum_change <= 1e-10;
  else
    result.pass = common && result.longitudinal >= 0.75 * result.nominal
        && result.transverse <= 0.25 && result.hops >= 3
        && result.shared > 0;
  return result;
}

struct ControlResult {
  NeutralizerFixture fixture{};
  std::array<SingleArm, 3> arms{};
  bool covariance = false;
  double state_covariance = INFINITY, diagnostic_covariance = INFINITY;
};

ControlResult run_control(int id, NeutralizerFixture fixture,
                          const ftd::eft::ChargedTrimerOptions& options) {
  ControlResult result;
  result.fixture = std::move(fixture);
  if (!result.fixture.valid) return result;
  result.arms[0] = run_arm(id, 0, result.fixture, 0.0, 16, options);
  result.arms[1] = run_arm(id, 1, result.fixture, 1.0 / 64.0, 128, options);
  result.arms[2] = run_arm(id, 2, result.fixture, 1.0 / 32.0, 64, options);
  auto original = result.fixture.state;
  const Vec3 launch = ftd::eft::production_flat_momentum(
      {1.0 / 64.0, 0.0, 0.0});
  for (auto& point : original.constituents) point.momentum = launch;
  const auto step = ftd::eft::solve_charged_trimer_forward(
      original, result.fixture.stationary, options);
  const auto shifted = translate_single_x(original, 1);
  const auto shifted_density = translate_density_x(result.fixture.stationary, 1);
  const auto shifted_step = ftd::eft::solve_charged_trimer_forward(
      shifted, shifted_density, options);
  if (step.valid && shifted_step.valid) {
    result.state_covariance =
        ftd::eft::charged_trimer_state_max_difference(
            translate_single_x(step.later, 1), shifted_step.later);
    result.diagnostic_covariance = diagnostic_difference(step, shifted_step);
  }
  result.covariance = step.valid && shifted_step.valid
      && step.gates_pass && shifted_step.gates_pass
      && result.state_covariance <= gate
      && result.diagnostic_covariance <= gate;
  return result;
}

struct SingleSummary {
  bool search_complete = false, static_seed = false;
  int admissible = 0, terminated = 0, clustered = 0;
  double energy = INFINITY;
  std::array<ControlResult, 2> controls{};
  std::string verdict;
};

void write_record(const SingleSummary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0610";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0610_single_core_neutralizer_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0610\",\n"
       << "  \"protocol_sha256\": \"" << single_protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"shared_anchor_option_default\": false,\n"
       << "  \"search_complete\": "
       << (summary.search_complete ? "true" : "false") << ",\n"
       << "  \"static_seed_pass\": "
       << (summary.static_seed ? "true" : "false") << ",\n"
       << "  \"admissible_starts\": " << summary.admissible << ",\n"
       << "  \"terminated_starts\": " << summary.terminated << ",\n"
       << "  \"clustered_starts\": " << summary.clustered << ",\n"
       << "  \"selected_energy\": " << json_number(summary.energy)
       << ",\n  \"controls\": [\n";
  for (std::size_t c = 0; c < summary.controls.size(); ++c) {
    const auto& control = summary.controls[c];
    const auto& f = control.fixture;
    json << "    {\"name\": \"" << f.name << "\""
         << ", \"fixture_valid\": " << (f.valid ? "true" : "false")
         << ", \"moving_charge\": " << json_number(f.moving_charge)
         << ", \"stationary_charge\": " << json_number(f.stationary_charge)
         << ", \"total_charge\": " << json_number(f.total_charge)
         << ", \"poisson_residual\": " << json_number(f.poisson)
         << ", \"gauss_residual\": " << json_number(f.gauss)
         << ", \"curl_residual\": " << json_number(f.curl)
         << ", \"energy_crosscheck\": " << json_number(f.energy_crosscheck)
         << ", \"covariance_pass\": "
         << (control.covariance ? "true" : "false")
         << ", \"covariance_state_residual\": "
         << json_number(control.state_covariance)
         << ", \"covariance_diagnostic_residual\": "
         << json_number(control.diagnostic_covariance)
         << ", \"arms\": [\n";
    for (std::size_t a = 0; a < control.arms.size(); ++a) {
      const auto& arm = control.arms[a];
      json << "      {\"rest\": " << (arm.rest ? "true" : "false")
           << ", \"velocity\": " << arm.velocity
           << ", \"ticks_requested\": " << arm.ticks
           << ", \"forward_ticks\": " << arm.forward
           << ", \"reverse_ticks\": " << arm.reverse
           << ", \"execution_complete\": "
           << (arm.complete ? "true" : "false")
           << ", \"physical_pass\": " << (arm.pass ? "true" : "false")
           << ", \"site_hops\": " << arm.hops
           << ", \"shared_anchor_states\": " << arm.shared
           << ", \"maximum_anchor_multiplicity\": "
           << arm.max_multiplicity
           << ", \"longitudinal_displacement\": "
           << json_number(arm.longitudinal)
           << ", \"nominal_displacement\": " << arm.nominal
           << ", \"transverse_drift\": " << json_number(arm.transverse)
           << ", \"center_momentum_change\": "
           << json_number(arm.momentum_change)
           << ", \"minimum_pair_distance\": "
           << json_number(arm.min_distance)
           << ", \"maximum_pair_distance\": "
           << json_number(arm.max_distance)
           << ", \"worst_common_gate\": " << arm.worst_gate
           << ", \"maximum_energy_drift\": " << arm.energy_drift
           << ", \"maximum_pseudomomentum_defect\": "
           << arm.momentum_defect
           << ", \"reverse_recovery\": " << json_number(arm.recovery)
           << "}" << (a + 1 == control.arms.size() ? "\n" : ",\n");
    }
    json << "    ]}" << (c + 1 == summary.controls.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";

  std::ofstream csv(dir / "ftd_0610_single_core_neutralizer_ticks_v1.csv");
  csv << "ftd_id,control,arm,direction,tick,valid,common_gate,energy_drift,"
         "pseudomomentum_defect,minimum_pair_distance,maximum_pair_distance,"
         "anchor_multiplicity\n";
  for (const auto& control : summary.controls)
    for (const auto& arm : control.arms)
      for (const auto& row : arm.records)
        csv << std::setprecision(17) << "FTD-0610," << row.control << ','
            << row.arm << ',' << row.direction << ',' << row.tick << ','
            << (row.valid ? 1 : 0) << ',' << row.gate_value << ','
            << row.energy_drift << ',' << row.momentum_defect << ','
            << row.min_distance << ',' << row.max_distance << ','
            << row.multiplicity << '\n';
}

}  // namespace

#ifndef FTD0610_NO_MAIN
int main() {
  std::cout << std::setprecision(17);
  SingleSummary summary;
  ClosedNeutralPairOptions search_options;
  search_options.gate_tolerance = gate;
  search_options.solve_tolerance = 2e-13;
  search_options.max_iterations = 64;
  const auto rotations = cubic_rotations();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const auto green = make_green_kernel();
  const double beta = normalization.mapped_field_work_coefficient;
  GlobalEvaluation selected;
  std::vector<SiteSearchResult> searches;
  if (normalization.valid && green.valid
      && green.residual <= direct_tolerance) {
    for (const auto& rotation : rotations) {
      auto search = search_site_cell(
          selected_phase, rotation, search_options, green, beta);
      if (search.admissible_start) ++summary.admissible;
      if (search.terminated && search.minimum.valid) ++summary.terminated;
      searches.push_back(std::move(search));
    }
    summary.search_complete = searches.size() == 24;
    for (const auto& search : searches)
      if (search.terminated && search.minimum.valid
          && (!selected.valid
              || search.minimum.total_energy < selected.total_energy))
        selected = search.minimum;
    if (selected.valid) {
      summary.energy = selected.total_energy;
      for (const auto& search : searches)
        if (search.terminated && search.minimum.valid
            && std::abs(search.minimum.total_energy
                        - selected.total_energy) <= 1e-10)
          ++summary.clustered;
      const auto differential = differentiate_site(
          selected_phase, selected, search_options, green, beta);
      const auto direct = initialize_minimum_energy(density_of(selected.state));
      const double field_gate = direct.valid
          ? std::max({direct.solver_residual, direct.gauss_residual,
              direct.curl_residual,
              std::abs(selected.field_energy - beta * direct.raw_energy)})
          : INFINITY;
      summary.static_seed = summary.admissible == 24
          && summary.terminated >= 18 && summary.clustered >= 2
          && std::abs(summary.energy - prior_energy) <= 5e-10
          && duplicate_anchor_pairs(selected.state) == 0
          && ::chart_margin(selected.state) >= reported_chart_margin
          && differential.valid && differential.gradient_inf <= 5e-7
          && differential.minimum_eigenvalue > 1e-6
          && differential.positive_modes == 6
          && direct.valid && field_gate <= 1e-11;
    }
  }

  ftd::eft::ChargedTrimerOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-13;
  options.max_iterations = 64;
  options.allow_shared_anchor_chart = true;
  if (summary.static_seed) {
    summary.controls[0] = run_control(
        0, make_fixture(selected.state, true), options);
    summary.controls[1] = run_control(
        1, make_fixture(selected.state, false), options);
  }
  bool coverage = summary.search_complete && summary.static_seed;
  for (const auto& control : summary.controls) {
    coverage = coverage && control.fixture.valid && control.covariance;
    for (const auto& arm : control.arms) coverage = coverage && arm.complete;
  }
  const auto& uniform = summary.controls[0];
  const auto& frozen = summary.controls[1];
  const bool uniform_motion =
      uniform.arms[1].pass && uniform.arms[2].pass;
  const bool frozen_motion = frozen.arms[1].pass && frozen.arms[2].pass;
  if (!coverage)
    summary.verdict = "SINGLE_CORE_NEUTRALIZER_CONTROL_NUMERICALLY_UNRESOLVED";
  else if (!uniform.arms[0].pass)
    summary.verdict = "SINGLE_CORE_STATIC_REFERENCE_NOT_ISOLATED";
  else if (!uniform_motion)
    summary.verdict = "SINGLE_CORE_COMPACT_TRANSPORT_CLOSED_NEGATIVE";
  else if (!frozen_motion)
    summary.verdict =
        "SINGLE_CORE_MOBILE_LOCALIZED_NEUTRALIZER_FORCE_ISOLATED";
  else if (!frozen.arms[0].pass)
    summary.verdict = "SINGLE_CORE_MOBILE_FROZEN_PARTNER_COMPATIBLE";
  else
    summary.verdict =
        "SINGLE_CORE_MOBILE_DYNAMIC_PARTNER_RESPONSE_ISOLATED";

  write_record(summary);
  std::cout << "protocol_sha256=" << single_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n';
  for (const auto& control : summary.controls) {
    std::cout << "control=" << control.fixture.name
              << " fixture=" << control.fixture.valid
              << " covariance=" << control.covariance << '\n';
    for (const auto& arm : control.arms)
      std::cout << "  velocity=" << arm.velocity
                << " complete=" << arm.complete << " pass=" << arm.pass
                << " displacement=" << arm.longitudinal
                << " hops=" << arm.hops << " shared=" << arm.shared << '\n';
  }
  return summary.search_complete ? 0 : 1;
}
#endif
