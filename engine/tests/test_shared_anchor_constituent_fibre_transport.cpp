// FTD-0609: two-record shared-anchor constituent-fibre transport.
#define FTD0608_NO_MAIN
#include "test_qualified_interior_compact_matter_transport.cpp"

namespace {

constexpr char fibre_protocol_sha256[] =
    "8CA3984F9E3FF2B8BE53BBBEA20028618EACFFC54C1B361994D10AD8B95D4D95";

int maximum_anchor_multiplicity(
    const ClosedNeutralTrimerPairState& state) {
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

double minimum_constituent_distance(
    const ClosedNeutralTrimerPairState& state) {
  double result = INFINITY;
  for (std::size_t a = 0; a < state.constituents.size(); ++a)
    for (std::size_t b = a + 1; b < state.constituents.size(); ++b)
      result = std::min(result,
          (effective_position(state.constituents[a])
           - effective_position(state.constituents[b])).mag());
  return result;
}

std::pair<int, int> first_shared_anchor_pair(
    const ClosedNeutralTrimerPairState& state) {
  for (std::size_t a = 0; a < state.constituents.size(); ++a)
    for (std::size_t b = a + 1; b < state.constituents.size(); ++b) {
      const auto& lhs = state.constituents[a].anchor;
      const auto& rhs = state.constituents[b].anchor;
      if (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z)
        return {static_cast<int>(a), static_cast<int>(b)};
    }
  return {-1, -1};
}

struct FibreTickRecord {
  int arm = 0;
  int direction = 0;
  int tick = 0;
  double common_gate = INFINITY;
  double energy_drift = INFINITY;
  int anchor_multiplicity = 0;
  double constituent_distance = INFINITY;
  double minimum_internal_distance = 0.0;
  double maximum_internal_distance = INFINITY;
  double separation = INFINITY;
};

struct FibreArmResult {
  bool valid = false;
  int ticks_requested = 0;
  int forward_ticks = 0;
  int reverse_ticks = 0;
  int site_hops = 0;
  int shared_anchor_states = 0;
  int shared_within_trimer_states = 0;
  int shared_cross_trimer_states = 0;
  int first_shared_a = -1;
  int first_shared_b = -1;
  int maximum_anchor_multiplicity = 1;
  double requested_velocity = 0.0;
  double nominal_displacement = 0.0;
  double longitudinal_displacement = 0.0;
  double transverse_drift = INFINITY;
  double maximum_separation_change = INFINITY;
  double minimum_constituent_distance = INFINITY;
  double minimum_internal_distance = INFINITY;
  double maximum_internal_distance = 0.0;
  double worst_common_gate = 0.0;
  double maximum_energy_drift = 0.0;
  double reverse_recovery = INFINITY;
  std::vector<FibreTickRecord> records{};
};

FibreArmResult run_fibre_arm(
    int arm_index, const ClosedNeutralTrimerPairState& static_state,
    double velocity, int ticks, const ClosedNeutralPairOptions& options) {
  FibreArmResult result;
  result.requested_velocity = velocity;
  result.ticks_requested = ticks;
  result.nominal_displacement = velocity * ticks;
  ClosedNeutralTrimerPairState initial = static_state;
  const Vec3 momentum = ftd::eft::production_flat_momentum(
      {velocity, 0.0, 0.0});
  for (auto& point : initial.constituents) point.momentum = momentum;
  ClosedNeutralTrimerPairState current = initial;
  const Vec3 center0 =
      (group_center(initial, 0) + group_center(initial, 1)) * 0.5;
  const double separation0 =
      (group_center(initial, 1) - group_center(initial, 0)).mag();
  double baseline_energy = NAN;
  bool forward_ok = true;
  result.maximum_separation_change = 0.0;

  for (int tick = 0; tick < ticks; ++tick) {
    const auto step = ftd::eft::solve_closed_neutral_pair_forward(
        current, options);
    FibreTickRecord record;
    record.arm = arm_index;
    record.direction = 1;
    record.tick = tick;
    record.common_gate = maximum_common_gate(step);
    if (!step.valid) {
      forward_ok = false;
      result.records.push_back(record);
      break;
    }
    const double total_before = step.kinetic_energy_before
        + step.binding_energy_before + step.field_energy_before;
    const double total_after = step.kinetic_energy_after
        + step.binding_energy_after + step.field_energy_after;
    if (!std::isfinite(baseline_energy)) baseline_energy = total_before;
    record.energy_drift = std::abs(total_after - baseline_energy);
    record.anchor_multiplicity = maximum_anchor_multiplicity(step.later);
    record.constituent_distance = minimum_constituent_distance(step.later);
    const auto distances = dynamic_distance_range(step.later);
    record.minimum_internal_distance = distances.first;
    record.maximum_internal_distance = distances.second;
    record.separation =
        (group_center(step.later, 1) - group_center(step.later, 0)).mag();
    result.site_hops += anchor_changes(current, step.later);
    ++result.forward_ticks;
    if (record.anchor_multiplicity > 1) {
      ++result.shared_anchor_states;
      const auto pair = first_shared_anchor_pair(step.later);
      if (result.first_shared_a < 0) {
        result.first_shared_a = pair.first;
        result.first_shared_b = pair.second;
      }
      if (pair.first / 3 == pair.second / 3)
        ++result.shared_within_trimer_states;
      else
        ++result.shared_cross_trimer_states;
    }
    result.maximum_anchor_multiplicity = std::max(
        result.maximum_anchor_multiplicity, record.anchor_multiplicity);
    result.minimum_constituent_distance = std::min(
        result.minimum_constituent_distance, record.constituent_distance);
    result.minimum_internal_distance = std::min(
        result.minimum_internal_distance, record.minimum_internal_distance);
    result.maximum_internal_distance = std::max(
        result.maximum_internal_distance, record.maximum_internal_distance);
    result.worst_common_gate = std::max(
        result.worst_common_gate, record.common_gate);
    result.maximum_energy_drift = std::max(
        result.maximum_energy_drift, record.energy_drift);
    result.maximum_separation_change = std::max(
        result.maximum_separation_change,
        std::abs(record.separation - separation0));
    result.records.push_back(record);
    forward_ok = step.common_action_gates_pass
        && record.common_gate <= gate
        && record.anchor_multiplicity <= 2
        && record.constituent_distance >= 1e-3;
    current = step.later;
    if (!forward_ok) break;
  }

  if (forward_ok && result.forward_ticks == ticks) {
    const Vec3 center1 =
        (group_center(current, 0) + group_center(current, 1)) * 0.5;
    const Vec3 displacement = center1 - center0;
    result.longitudinal_displacement = displacement.x;
    result.transverse_drift = std::sqrt(
        displacement.y * displacement.y + displacement.z * displacement.z);
    for (int tick = 0; tick < ticks; ++tick) {
      const auto step = ftd::eft::solve_closed_neutral_pair_reverse(
          current, options);
      FibreTickRecord record;
      record.arm = arm_index;
      record.direction = -1;
      record.tick = tick;
      record.common_gate = maximum_common_gate(step);
      if (!step.valid) {
        result.records.push_back(record);
        break;
      }
      record.anchor_multiplicity = maximum_anchor_multiplicity(step.earlier);
      record.constituent_distance = minimum_constituent_distance(step.earlier);
      const auto distances = dynamic_distance_range(step.earlier);
      record.minimum_internal_distance = distances.first;
      record.maximum_internal_distance = distances.second;
      record.separation =
          (group_center(step.earlier, 1) - group_center(step.earlier, 0)).mag();
      ++result.reverse_ticks;
      if (record.anchor_multiplicity > 1) {
        ++result.shared_anchor_states;
        const auto pair = first_shared_anchor_pair(step.earlier);
        if (result.first_shared_a < 0) {
          result.first_shared_a = pair.first;
          result.first_shared_b = pair.second;
        }
        if (pair.first / 3 == pair.second / 3)
          ++result.shared_within_trimer_states;
        else
          ++result.shared_cross_trimer_states;
      }
      result.maximum_anchor_multiplicity = std::max(
          result.maximum_anchor_multiplicity, record.anchor_multiplicity);
      result.minimum_constituent_distance = std::min(
          result.minimum_constituent_distance, record.constituent_distance);
      result.worst_common_gate = std::max(
          result.worst_common_gate, record.common_gate);
      result.records.push_back(record);
      current = step.earlier;
      if (!step.common_action_gates_pass || record.common_gate > gate
          || record.anchor_multiplicity > 2
          || record.constituent_distance < 1e-3) break;
    }
  }
  if (result.reverse_ticks == ticks)
    result.reverse_recovery =
        ftd::eft::closed_neutral_pair_state_max_difference(initial, current);

  result.valid = forward_ok && result.forward_ticks == ticks
      && result.reverse_ticks == ticks
      && result.longitudinal_displacement
          >= 0.75 * result.nominal_displacement
      && result.transverse_drift <= 0.25
      && result.site_hops >= 6
      && result.shared_anchor_states > 0
      && result.maximum_anchor_multiplicity <= 2
      && result.maximum_separation_change <= 0.25
      && result.minimum_constituent_distance >= 1e-3
      && result.minimum_internal_distance >= 0.5
      && result.maximum_internal_distance <= 2.0
      && result.worst_common_gate <= gate
      && result.maximum_energy_drift <= 1e-10
      && result.reverse_recovery <= 1e-9;
  return result;
}

struct FibreSummary {
  bool green_pass = false;
  bool static_seed_pass = false;
  bool strict_regression_pass = false;
  bool motion_execution_complete = false;
  bool integer_covariance_pass = false;
  int admissible_starts = 0;
  int terminated_starts = 0;
  int clustered_starts = 0;
  double energy = INFINITY;
  double chart_margin = -INFINITY;
  double gradient_inf = INFINITY;
  double minimum_eigenvalue = -INFINITY;
  double field_gate = INFINITY;
  double integer_covariance_residual = INFINITY;
  std::array<int, 2> strict_failure_ticks{{-1, -1}};
  std::array<FibreArmResult, 2> motion{};
  std::string verdict;
};

void write_fibre_record(const FibreSummary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0609";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0609_shared_anchor_fibre_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0609\",\n"
       << "  \"protocol_sha256\": \"" << fibre_protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"shared_anchor_option_default\": false,\n"
       << "  \"static_seed_pass\": "
       << (summary.static_seed_pass ? "true" : "false") << ",\n"
       << "  \"strict_regression_pass\": "
       << (summary.strict_regression_pass ? "true" : "false") << ",\n"
       << "  \"motion_execution_complete\": "
       << (summary.motion_execution_complete ? "true" : "false") << ",\n"
       << "  \"integer_covariance_pass\": "
       << (summary.integer_covariance_pass ? "true" : "false") << ",\n"
       << "  \"admissible_starts\": " << summary.admissible_starts << ",\n"
       << "  \"terminated_starts\": " << summary.terminated_starts << ",\n"
       << "  \"clustered_starts\": " << summary.clustered_starts << ",\n"
       << "  \"energy\": " << json_number(summary.energy) << ",\n"
       << "  \"chart_margin\": " << json_number(summary.chart_margin) << ",\n"
       << "  \"gradient_inf\": " << json_number(summary.gradient_inf) << ",\n"
       << "  \"minimum_eigenvalue\": "
       << json_number(summary.minimum_eigenvalue) << ",\n"
       << "  \"field_gate\": " << json_number(summary.field_gate) << ",\n"
       << "  \"strict_failure_ticks\": ["
       << summary.strict_failure_ticks[0] << ','
       << summary.strict_failure_ticks[1] << "],\n"
       << "  \"integer_covariance_residual\": "
       << json_number(summary.integer_covariance_residual) << ",\n"
       << "  \"motion_arms\": [\n";
  for (std::size_t i = 0; i < summary.motion.size(); ++i) {
    const auto& arm = summary.motion[i];
    json << "    {\"velocity\": " << arm.requested_velocity
         << ", \"ticks_requested\": " << arm.ticks_requested
         << ", \"forward_ticks\": " << arm.forward_ticks
         << ", \"reverse_ticks\": " << arm.reverse_ticks
         << ", \"site_hops\": " << arm.site_hops
         << ", \"shared_anchor_states\": " << arm.shared_anchor_states
         << ", \"shared_within_trimer_states\": "
         << arm.shared_within_trimer_states
         << ", \"shared_cross_trimer_states\": "
         << arm.shared_cross_trimer_states
         << ", \"first_shared_pair\": [" << arm.first_shared_a << ','
         << arm.first_shared_b << ']'
         << ", \"maximum_anchor_multiplicity\": "
         << arm.maximum_anchor_multiplicity
         << ", \"minimum_constituent_distance\": "
         << json_number(arm.minimum_constituent_distance)
         << ", \"longitudinal_displacement\": "
         << arm.longitudinal_displacement
         << ", \"nominal_displacement\": " << arm.nominal_displacement
         << ", \"transverse_drift\": " << json_number(arm.transverse_drift)
         << ", \"maximum_separation_change\": "
         << json_number(arm.maximum_separation_change)
         << ", \"minimum_internal_distance\": "
         << json_number(arm.minimum_internal_distance)
         << ", \"maximum_internal_distance\": "
         << arm.maximum_internal_distance
         << ", \"worst_common_gate\": " << arm.worst_common_gate
         << ", \"maximum_energy_drift\": " << arm.maximum_energy_drift
         << ", \"reverse_recovery\": "
         << json_number(arm.reverse_recovery)
         << ", \"valid\": " << (arm.valid ? "true" : "false") << "}"
         << (i + 1 == summary.motion.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";

  std::ofstream csv(dir / "ftd_0609_shared_anchor_fibre_ticks_v1.csv");
  csv << "ftd_id,arm,direction,tick,common_gate,energy_drift,"
         "anchor_multiplicity,constituent_distance,minimum_internal_distance,"
         "maximum_internal_distance,separation\n";
  for (const auto& arm : summary.motion)
    for (const auto& row : arm.records)
      csv << std::setprecision(17) << "FTD-0609," << row.arm << ','
          << row.direction << ',' << row.tick << ',' << row.common_gate << ','
          << row.energy_drift << ',' << row.anchor_multiplicity << ','
          << row.constituent_distance << ',' << row.minimum_internal_distance
          << ',' << row.maximum_internal_distance << ',' << row.separation
          << '\n';
}

}  // namespace

#ifndef FTD0609_NO_MAIN
int main() {
  std::cout << std::setprecision(17);
  ClosedNeutralPairOptions strict_options;
  strict_options.gate_tolerance = gate;
  strict_options.solve_tolerance = 2e-13;
  strict_options.max_iterations = 64;
  FibreSummary summary;
  const auto rotations = cubic_rotations();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const auto green = make_green_kernel();
  const double beta = normalization.mapped_field_work_coefficient;
  summary.green_pass = normalization.valid && green.valid
      && green.residual <= direct_tolerance;

  GlobalEvaluation selected;
  std::vector<SiteSearchResult> searches;
  if (summary.green_pass) {
    for (const auto& rotation : rotations) {
      auto search = search_site_cell(
          selected_phase, rotation, strict_options, green, beta);
      if (search.admissible_start) ++summary.admissible_starts;
      if (search.terminated && search.minimum.valid)
        ++summary.terminated_starts;
      searches.push_back(std::move(search));
    }
    for (const auto& search : searches)
      if (search.terminated && search.minimum.valid
          && (!selected.valid
              || search.minimum.total_energy < selected.total_energy))
        selected = search.minimum;
    if (selected.valid)
      for (const auto& search : searches)
        if (search.terminated && search.minimum.valid
            && std::abs(search.minimum.total_energy
                        - selected.total_energy) <= 1e-10)
          ++summary.clustered_starts;
    if (selected.valid) {
      summary.energy = selected.total_energy;
      summary.chart_margin = ::chart_margin(selected.state);
      const auto differential = differentiate_site(
          selected_phase, selected, strict_options, green, beta);
      summary.gradient_inf = differential.gradient_inf;
      summary.minimum_eigenvalue = differential.minimum_eigenvalue;
      const auto direct = initialize_minimum_energy(
          density_of(selected.state));
      const double energy_residual = direct.valid
          ? std::abs(selected.field_energy - beta * direct.raw_energy)
          : INFINITY;
      summary.field_gate = std::max({direct.solver_residual,
          direct.gauss_residual, direct.curl_residual, energy_residual});
      summary.static_seed_pass = summary.admissible_starts == 24
          && summary.terminated_starts >= 18
          && summary.clustered_starts >= 2
          && std::abs(summary.energy - prior_energy) <= 5e-10
          && duplicate_anchor_pairs(selected.state) == 0
          && summary.chart_margin >= reported_chart_margin
          && differential.valid && summary.gradient_inf <= 5e-7
          && summary.minimum_eigenvalue > 1e-6
          && differential.positive_modes == 6
          && direct.valid && summary.field_gate <= 1e-11;
      if (summary.static_seed_pass) selected.state.electric = direct.electric;
    }
  }

  if (summary.static_seed_pass) {
    const auto strict0 = diagnose_forward_failure(
        selected.state, 1.0 / 64.0, 128, strict_options);
    const auto strict1 = diagnose_forward_failure(
        selected.state, 1.0 / 32.0, 64, strict_options);
    summary.strict_failure_ticks = {{strict0.tick, strict1.tick}};
    summary.strict_regression_pass = strict0.observed && strict1.observed
        && strict0.tick == 4 && strict1.tick == 2
        && strict0.free_predictor_duplicate_anchors == 1
        && strict1.free_predictor_duplicate_anchors == 1;

    auto fibre_options = strict_options;
    fibre_options.allow_shared_anchor_chart = true;
    summary.motion[0] = run_fibre_arm(
        0, selected.state, 1.0 / 64.0, 128, fibre_options);
    summary.motion[1] = run_fibre_arm(
        1, selected.state, 1.0 / 32.0, 64, fibre_options);
    summary.motion_execution_complete =
        summary.motion[0].forward_ticks == 128
        && summary.motion[0].reverse_ticks == 128
        && summary.motion[1].forward_ticks == 64
        && summary.motion[1].reverse_ticks == 64;

    auto original = selected.state;
    const Vec3 momentum = ftd::eft::production_flat_momentum(
        {1.0 / 64.0, 0.0, 0.0});
    for (auto& point : original.constituents) point.momentum = momentum;
    const auto original_step = ftd::eft::solve_closed_neutral_pair_forward(
        original, fibre_options);
    const auto translated = translate_x(original, 1);
    const auto translated_step = ftd::eft::solve_closed_neutral_pair_forward(
        translated, fibre_options);
    if (original_step.valid && translated_step.valid)
      summary.integer_covariance_residual =
          ftd::eft::closed_neutral_pair_state_max_difference(
              translate_x(original_step.later, 1), translated_step.later);
    summary.integer_covariance_pass = original_step.valid
        && translated_step.valid && original_step.common_action_gates_pass
        && translated_step.common_action_gates_pass
        && summary.integer_covariance_residual <= gate;
  }

  const bool fibre_exercised = summary.motion[0].shared_anchor_states > 0
      && summary.motion[1].shared_anchor_states > 0;
  const bool motion_pass = summary.motion[0].valid
      && summary.motion[1].valid && summary.integer_covariance_pass;
  if (!summary.green_pass || searches.size() != 24
      || !summary.static_seed_pass || !summary.strict_regression_pass
      || !summary.motion_execution_complete) {
    summary.verdict = "SHARED_ANCHOR_FIBRE_NUMERICALLY_UNRESOLVED";
  } else if (motion_pass && !fibre_exercised) {
    summary.verdict = "SHARED_ANCHOR_FIBRE_EXTENSION_NOT_EXERCISED";
  } else if (motion_pass) {
    summary.verdict =
        "SHARED_ANCHOR_FIBRE_COMPACT_MATTER_MOBILE_CONSTRUCTIVE";
  } else {
    summary.verdict = "SHARED_ANCHOR_FIBRE_TRANSPORT_CLOSED_NEGATIVE";
  }

  write_fibre_record(summary);
  std::cout << "protocol_sha256=" << fibre_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "strict_regression_pass=" << summary.strict_regression_pass
            << '\n'
            << "arm0_forward_reverse=" << summary.motion[0].forward_ticks
            << '/' << summary.motion[0].reverse_ticks << '\n'
            << "arm0_hops_shared=" << summary.motion[0].site_hops << '/'
            << summary.motion[0].shared_anchor_states << '\n'
            << "arm1_forward_reverse=" << summary.motion[1].forward_ticks
            << '/' << summary.motion[1].reverse_ticks << '\n'
            << "arm1_hops_shared=" << summary.motion[1].site_hops << '/'
            << summary.motion[1].shared_anchor_states << '\n';
  return searches.size() == 24 ? 0 : 1;
}
#endif
