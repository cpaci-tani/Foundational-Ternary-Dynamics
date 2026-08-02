// FTD-0608: autonomous transport from a preregistered qualified interior core.
#define FTD0607_NO_MAIN
#include "test_site_admissible_compact_matter_motion.cpp"

namespace {

constexpr char transport_protocol_sha256[] =
    "B64BB90EF082EC8E47BE83BA1F9951D7B30C3C5904AE8E4C639B33543020C5E0";
constexpr int selected_phase_index = 15;
constexpr double selected_phase = 15.0 / 32.0;
constexpr double prior_energy = 0.0031781023845096961;

struct FailureDiagnosis {
  bool observed = false;
  int tick = -1;
  bool solve_attempted = false;
  bool solve_converged = false;
  int iterations = 0;
  int rejected_steps = 0;
  double residual = INFINITY;
  double step_residual = INFINITY;
  double minimum_abs_jacobian_determinant = 0.0;
  double input_chart_margin = -INFINITY;
  int input_duplicate_anchors = 0;
  int free_predictor_duplicate_anchors = 0;
  double free_predictor_chart_margin = -INFINITY;
  bool returned_site_projection_valid = false;
  double returned_chart_margin = -INFINITY;
  int returned_duplicate_anchors = 0;
};

struct TransportSummary {
  bool green_pass = false;
  bool static_coverage_pass = false;
  bool static_seed_pass = false;
  bool motion_execution_complete = false;
  bool integer_covariance_pass = false;
  int admissible_starts = 0;
  int terminated_starts = 0;
  int clustered_starts = 0;
  int total_evaluations = 0;
  double green_residual = INFINITY;
  double energy = INFINITY;
  double energy_fingerprint_residual = INFINITY;
  double chart_margin = -INFINITY;
  double gradient_inf = INFINITY;
  double minimum_eigenvalue = -INFINITY;
  int positive_modes = 0;
  double field_gate = INFINITY;
  double integer_covariance_residual = INFINITY;
  std::array<MotionArmResult, 2> motion{};
  std::array<FailureDiagnosis, 2> failure{};
  std::string verdict;
};

FailureDiagnosis diagnose_forward_failure(
    const ClosedNeutralTrimerPairState& static_state, double velocity,
    int ticks, const ClosedNeutralPairOptions& options) {
  FailureDiagnosis result;
  ClosedNeutralTrimerPairState current = static_state;
  const Vec3 momentum = ftd::eft::production_flat_momentum(
      {velocity, 0.0, 0.0});
  for (auto& point : current.constituents) point.momentum = momentum;
  for (int tick = 0; tick < ticks; ++tick) {
    const auto step = ftd::eft::solve_closed_neutral_pair_forward(
        current, options);
    if (!step.valid) {
      result.observed = true;
      result.tick = tick;
      result.solve_attempted = step.solve.attempted;
      result.solve_converged = step.solve.converged;
      result.iterations = step.solve.iterations;
      result.rejected_steps = step.solve.rejected_steps;
      result.residual = step.solve.residual;
      result.step_residual = step.solve.step_residual;
      result.minimum_abs_jacobian_determinant =
          step.solve.minimum_abs_jacobian_determinant;
      result.input_chart_margin = ::chart_margin(current);
      result.input_duplicate_anchors = duplicate_anchor_pairs(current);
      auto free_predictor = current;
      for (std::size_t a = 0; a < free_predictor.constituents.size(); ++a) {
        const Vec3 p = current.constituents[a].momentum;
        auto point = point_at(effective_position(current.constituents[a])
            + ftd::eft::production_flat_velocity_from_momentum(p));
        point.momentum = p;
        free_predictor.constituents[a] = point;
      }
      result.free_predictor_duplicate_anchors =
          duplicate_anchor_pairs(free_predictor);
      result.free_predictor_chart_margin = ::chart_margin(free_predictor);
      result.returned_site_projection_valid = step.site_projection_valid;
      if (step.later.electric.L > 0) {
        result.returned_chart_margin = ::chart_margin(step.later);
        result.returned_duplicate_anchors =
            duplicate_anchor_pairs(step.later);
      }
      return result;
    }
    current = step.later;
  }
  return result;
}

void write_transport_record(const TransportSummary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0608";
  std::filesystem::create_directories(dir);
  std::ofstream json(
      dir / "ftd_0608_qualified_interior_transport_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0608\",\n"
       << "  \"protocol_sha256\": \"" << transport_protocol_sha256
       << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"selected_phase_index\": " << selected_phase_index << ",\n"
       << "  \"selected_phase\": " << selected_phase << ",\n"
       << "  \"green_pass\": "
       << (summary.green_pass ? "true" : "false") << ",\n"
       << "  \"static_coverage_pass\": "
       << (summary.static_coverage_pass ? "true" : "false") << ",\n"
       << "  \"static_seed_pass\": "
       << (summary.static_seed_pass ? "true" : "false") << ",\n"
       << "  \"motion_execution_complete\": "
       << (summary.motion_execution_complete ? "true" : "false") << ",\n"
       << "  \"integer_covariance_pass\": "
       << (summary.integer_covariance_pass ? "true" : "false") << ",\n"
       << "  \"admissible_starts\": " << summary.admissible_starts << ",\n"
       << "  \"terminated_starts\": " << summary.terminated_starts << ",\n"
       << "  \"clustered_starts\": " << summary.clustered_starts << ",\n"
       << "  \"total_evaluations\": " << summary.total_evaluations << ",\n"
       << "  \"green_residual\": " << summary.green_residual << ",\n"
       << "  \"energy\": " << json_number(summary.energy) << ",\n"
       << "  \"energy_fingerprint_residual\": "
       << json_number(summary.energy_fingerprint_residual) << ",\n"
       << "  \"chart_margin\": " << json_number(summary.chart_margin)
       << ",\n"
       << "  \"gradient_inf\": " << json_number(summary.gradient_inf)
       << ",\n"
       << "  \"minimum_eigenvalue\": "
       << json_number(summary.minimum_eigenvalue) << ",\n"
       << "  \"positive_modes\": " << summary.positive_modes << ",\n"
       << "  \"field_gate\": " << json_number(summary.field_gate) << ",\n"
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
         << ", \"maximum_duplicate_anchors\": "
         << arm.maximum_duplicate_anchors
         << ", \"valid\": " << (arm.valid ? "true" : "false") << "}"
         << (i + 1 == summary.motion.size() ? "\n" : ",\n");
  }
  json << "  ],\n  \"failure_diagnostics\": [\n";
  for (std::size_t i = 0; i < summary.failure.size(); ++i) {
    const auto& failure = summary.failure[i];
    json << "    {\"observed\": "
         << (failure.observed ? "true" : "false")
         << ", \"tick\": " << failure.tick
         << ", \"solve_attempted\": "
         << (failure.solve_attempted ? "true" : "false")
         << ", \"solve_converged\": "
         << (failure.solve_converged ? "true" : "false")
         << ", \"iterations\": " << failure.iterations
         << ", \"rejected_steps\": " << failure.rejected_steps
         << ", \"residual\": " << json_number(failure.residual)
         << ", \"step_residual\": "
         << json_number(failure.step_residual)
         << ", \"minimum_abs_jacobian_determinant\": "
         << failure.minimum_abs_jacobian_determinant
         << ", \"input_chart_margin\": "
         << json_number(failure.input_chart_margin)
         << ", \"input_duplicate_anchors\": "
         << failure.input_duplicate_anchors
         << ", \"free_predictor_duplicate_anchors\": "
         << failure.free_predictor_duplicate_anchors
         << ", \"free_predictor_chart_margin\": "
         << json_number(failure.free_predictor_chart_margin)
         << ", \"returned_site_projection_valid\": "
         << (failure.returned_site_projection_valid ? "true" : "false")
         << ", \"returned_chart_margin\": "
         << json_number(failure.returned_chart_margin)
         << ", \"returned_duplicate_anchors\": "
         << failure.returned_duplicate_anchors << "}"
         << (i + 1 == summary.failure.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";

  std::ofstream tick_csv(
      dir / "ftd_0608_qualified_interior_transport_ticks_v1.csv");
  tick_csv << "ftd_id,arm,direction,tick,common_gate,energy_drift,"
              "duplicate_anchors,minimum_distance,maximum_distance,separation\n";
  for (const auto& arm : summary.motion)
    for (const auto& record : arm.records)
      tick_csv << std::setprecision(17) << "FTD-0608," << record.arm << ','
          << record.direction << ',' << record.tick << ','
          << record.common_gate << ',' << record.energy_drift << ','
          << record.duplicate_anchors << ',' << record.minimum_distance << ','
          << record.maximum_distance << ',' << record.separation << '\n';
}

}  // namespace

#ifndef FTD0608_NO_MAIN
int main() {
  std::cout << std::setprecision(17);
  ClosedNeutralPairOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-13;
  options.max_iterations = 64;
  TransportSummary summary;
  const auto rotations = cubic_rotations();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const auto green = make_green_kernel();
  const double beta = normalization.mapped_field_work_coefficient;
  summary.green_residual = green.residual;
  summary.green_pass = normalization.valid && green.valid
      && green.residual <= direct_tolerance;

  GlobalEvaluation selected;
  std::vector<SiteSearchResult> searches;
  if (summary.green_pass) {
    searches.reserve(rotations.size());
    for (const auto& rotation : rotations) {
      auto search = search_site_cell(
          selected_phase, rotation, options, green, beta);
      if (search.admissible_start) ++summary.admissible_starts;
      if (search.terminated && search.minimum.valid)
        ++summary.terminated_starts;
      summary.total_evaluations += search.evaluations;
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
    summary.static_coverage_pass = summary.admissible_starts == 24
        && summary.terminated_starts >= 18
        && summary.clustered_starts >= 2;

    if (selected.valid) {
      summary.energy = selected.total_energy;
      summary.energy_fingerprint_residual =
          std::abs(summary.energy - prior_energy);
      summary.chart_margin = ::chart_margin(selected.state);
      const auto differential = differentiate_site(
          selected_phase, selected, options, green, beta);
      summary.gradient_inf = differential.gradient_inf;
      summary.minimum_eigenvalue = differential.minimum_eigenvalue;
      summary.positive_modes = differential.positive_modes;
      const auto direct = initialize_minimum_energy(
          density_of(selected.state));
      const double energy_residual = direct.valid
          ? std::abs(selected.field_energy - beta * direct.raw_energy)
          : INFINITY;
      summary.field_gate = std::max({direct.solver_residual,
          direct.gauss_residual, direct.curl_residual, energy_residual});
      summary.static_seed_pass = summary.static_coverage_pass
          && summary.energy_fingerprint_residual <= 5e-10
          && duplicate_anchor_pairs(selected.state) == 0
          && summary.chart_margin >= reported_chart_margin
          && strain_max_abs(selected.strain) <= strain_basin - 1e-4
          && strain_minimum_eigenvalue(selected.strain) >= 0.70
          && selected.minimum_distance >= 0.5
          && selected.maximum_distance <= 2.0
          && differential.valid && summary.gradient_inf <= 5e-7
          && summary.minimum_eigenvalue >= -5e-6
          && summary.positive_modes == 6
          && direct.valid && summary.field_gate <= 1e-11;
      if (summary.static_seed_pass) selected.state.electric = direct.electric;
    }
  }

  if (summary.static_seed_pass) {
    summary.motion[0] = run_motion_arm(
        0, selected.state, 1.0 / 64.0, 128, options);
    summary.motion[1] = run_motion_arm(
        1, selected.state, 1.0 / 32.0, 64, options);
    summary.failure[0] = diagnose_forward_failure(
        selected.state, 1.0 / 64.0, 128, options);
    summary.failure[1] = diagnose_forward_failure(
        selected.state, 1.0 / 32.0, 64, options);
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
        original, options);
    const auto translated = translate_x(original, 1);
    const auto translated_step =
        ftd::eft::solve_closed_neutral_pair_forward(translated, options);
    if (original_step.valid && translated_step.valid)
      summary.integer_covariance_residual =
          ftd::eft::closed_neutral_pair_state_max_difference(
              translate_x(original_step.later, 1), translated_step.later);
    summary.integer_covariance_pass = original_step.valid
        && translated_step.valid && original_step.common_action_gates_pass
        && translated_step.common_action_gates_pass
        && summary.integer_covariance_residual <= gate;
  }

  const bool motion_pass = summary.motion[0].valid
      && summary.motion[1].valid && summary.integer_covariance_pass;
  if (!summary.green_pass || searches.size() != 24
      || !summary.static_coverage_pass) {
    summary.verdict =
        "QUALIFIED_INTERIOR_COMPACT_MATTER_NUMERICALLY_UNRESOLVED";
  } else if (!summary.static_seed_pass) {
    summary.verdict = "QUALIFIED_INTERIOR_STATIC_SEED_NOT_REPRODUCED";
  } else if (!summary.motion_execution_complete) {
    summary.verdict =
        "QUALIFIED_INTERIOR_COMPACT_MATTER_NUMERICALLY_UNRESOLVED";
  } else if (motion_pass) {
    summary.verdict =
        "QUALIFIED_INTERIOR_COMPACT_MATTER_MOBILE_CONSTRUCTIVE";
  } else {
    summary.verdict =
        "QUALIFIED_INTERIOR_COMPACT_TRANSPORT_CLOSED_NEGATIVE";
  }

  write_transport_record(summary);
  std::cout << "protocol_sha256=" << transport_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "static_seed_pass=" << summary.static_seed_pass << '\n'
            << "energy=" << summary.energy << '\n'
            << "chart_margin=" << summary.chart_margin << '\n'
            << "arm0_forward=" << summary.motion[0].forward_ticks << '\n'
            << "arm0_reverse=" << summary.motion[0].reverse_ticks << '\n'
            << "arm0_valid=" << summary.motion[0].valid << '\n'
            << "arm1_forward=" << summary.motion[1].forward_ticks << '\n'
            << "arm1_reverse=" << summary.motion[1].reverse_ticks << '\n'
            << "arm1_valid=" << summary.motion[1].valid << '\n'
            << "integer_covariance_residual="
            << summary.integer_covariance_residual << '\n';
  return searches.size() == 24 ? 0 : 1;
}
#endif
