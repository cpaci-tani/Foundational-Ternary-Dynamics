// FTD-0613: directional boosts from the refined compact rest state.
#define FTD0612_NO_MAIN
#include "test_uniform_single_core_stationary_refinement.cpp"

namespace {

constexpr char boost_protocol_sha256[] =
    "1A750AA6C557294B6E252A0E77F4B33AD5791A251655EB5787CA946E74A92C35";
constexpr double locked_refined_energy = 0.0015517955076684577;

struct BoostArm {
  bool complete = false;
  bool base_pass = false;
  bool mobility_pass = false;
  int speed_index = 0;
  int direction_index = 0;
  int ticks = 0;
  int forward = 0;
  int reverse = 0;
  int hops = 0;
  int shared_states = 0;
  int maximum_multiplicity = 1;
  double speed = 0.0;
  Vec3 direction{};
  double projected_displacement = INFINITY;
  double transverse_drift = INFINITY;
  double worst_gate = 0.0;
  double energy_drift = 0.0;
  double minimum_distance = INFINITY;
  double maximum_distance = 0.0;
  double maximum_pseudomomentum_defect = 0.0;
  double recovery = INFINITY;
};

BoostArm run_boost_arm(
    int speed_index, int direction_index,
    const ChargedTrimerState& rest_state,
    const std::vector<double>& stationary, double speed, int ticks,
    const Vec3& direction,
    const ftd::eft::ChargedTrimerOptions& options) {
  BoostArm result;
  result.speed_index = speed_index;
  result.direction_index = direction_index;
  result.speed = speed;
  result.ticks = ticks;
  result.direction = direction;
  ChargedTrimerState initial = rest_state;
  const Vec3 velocity = direction * speed;
  const Vec3 launch = ftd::eft::production_flat_momentum(velocity);
  for (auto& point : initial.constituents) point.momentum = launch;
  ChargedTrimerState current = initial;
  const Vec3 center0 = single_center(initial);
  double baseline = NAN;
  for (int tick = 0; tick < ticks; ++tick) {
    const auto step = ftd::eft::solve_charged_trimer_forward(
        current, stationary, options);
    if (!step.valid) break;
    if (!std::isfinite(baseline)) baseline = single_energy_before(step);
    ++result.forward;
    result.hops += single_anchor_changes(current, step.later);
    const int multiplicity = maximum_anchor_multiplicity(step.later);
    result.maximum_multiplicity = std::max(
        result.maximum_multiplicity, multiplicity);
    if (multiplicity > 1) ++result.shared_states;
    result.worst_gate = std::max(
        result.worst_gate, single_maximum_gate(step));
    result.energy_drift = std::max(result.energy_drift,
        std::max(std::abs(single_energy_before(step) - baseline),
                 std::abs(single_energy_after(step) - baseline)));
    result.minimum_distance = std::min(
        result.minimum_distance, step.minimum_pair_distance);
    result.maximum_distance = std::max(
        result.maximum_distance, step.maximum_pair_distance);
    result.maximum_pseudomomentum_defect = std::max(
        result.maximum_pseudomomentum_defect,
        step.pseudomomentum_defect_norm);
    current = step.later;
  }
  if (result.forward == ticks) {
    const Vec3 displacement = single_center(current) - center0;
    result.projected_displacement = displacement.dot(direction);
    const Vec3 transverse = displacement
        - direction * result.projected_displacement;
    result.transverse_drift = transverse.mag();
    for (int tick = 0; tick < ticks; ++tick) {
      const auto step = ftd::eft::solve_charged_trimer_reverse(
          current, stationary, options);
      if (!step.valid) break;
      ++result.reverse;
      const int multiplicity = maximum_anchor_multiplicity(step.earlier);
      result.maximum_multiplicity = std::max(
          result.maximum_multiplicity, multiplicity);
      if (multiplicity > 1) ++result.shared_states;
      result.worst_gate = std::max(
          result.worst_gate, single_maximum_gate(step));
      result.energy_drift = std::max(result.energy_drift,
          std::max(std::abs(single_energy_before(step) - baseline),
                   std::abs(single_energy_after(step) - baseline)));
      result.minimum_distance = std::min(
          result.minimum_distance, step.minimum_pair_distance);
      result.maximum_distance = std::max(
          result.maximum_distance, step.maximum_pair_distance);
      result.maximum_pseudomomentum_defect = std::max(
          result.maximum_pseudomomentum_defect,
          step.pseudomomentum_defect_norm);
      current = step.earlier;
    }
  }
  result.complete = result.forward == ticks && result.reverse == ticks;
  if (result.complete)
    result.recovery =
        ftd::eft::charged_trimer_state_max_difference(initial, current);
  result.base_pass = result.complete && result.worst_gate <= gate
      && result.energy_drift <= 1e-10 && result.recovery <= 1e-9
      && result.minimum_distance >= 0.5 && result.maximum_distance <= 2.0
      && result.maximum_multiplicity <= 2;
  result.mobility_pass = result.base_pass
      && result.projected_displacement >= 1.5
      && result.transverse_drift <= 0.25 && result.hops >= 3;
  return result;
}

struct BoostSummary {
  bool rest_fingerprint = false;
  bool rest_gate = false;
  bool arm_coverage = false;
  bool symmetry_coverage = false;
  bool symmetry_pass = false;
  double refined_energy = INFINITY;
  double refined_gradient = INFINITY;
  double sign_residual = INFINITY;
  double axis_residual = INFINITY;
  SingleArm rest{};
  std::vector<BoostArm> arms{};
  std::string verdict;
};

void write_boost_record(const BoostSummary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0613";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0613_refined_directional_boost_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0613\",\n"
       << "  \"protocol_sha256\": \"" << boost_protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"rest_fingerprint_pass\": "
       << (summary.rest_fingerprint ? "true" : "false") << ",\n"
       << "  \"rest_gate_pass\": "
       << (summary.rest_gate ? "true" : "false") << ",\n"
       << "  \"arm_coverage\": "
       << (summary.arm_coverage ? "true" : "false") << ",\n"
       << "  \"symmetry_coverage\": "
       << (summary.symmetry_coverage ? "true" : "false") << ",\n"
       << "  \"symmetry_pass\": "
       << (summary.symmetry_pass ? "true" : "false") << ",\n"
       << "  \"refined_energy\": " << json_number(summary.refined_energy)
       << ",\n  \"refined_gradient\": "
       << json_number(summary.refined_gradient)
       << ",\n  \"sign_residual\": " << json_number(summary.sign_residual)
       << ",\n  \"axis_residual\": " << json_number(summary.axis_residual)
       << ",\n  \"rest_recovery\": " << json_number(summary.rest.recovery)
       << ",\n  \"arms\": [\n";
  for (std::size_t i = 0; i < summary.arms.size(); ++i) {
    const auto& arm = summary.arms[i];
    json << "    {\"speed_index\": " << arm.speed_index
         << ", \"direction_index\": " << arm.direction_index
         << ", \"speed\": " << arm.speed << ", \"ticks\": " << arm.ticks
         << ", \"forward_ticks\": " << arm.forward
         << ", \"reverse_ticks\": " << arm.reverse
         << ", \"execution_complete\": "
         << (arm.complete ? "true" : "false")
         << ", \"base_pass\": " << (arm.base_pass ? "true" : "false")
         << ", \"mobility_pass\": "
         << (arm.mobility_pass ? "true" : "false")
         << ", \"projected_displacement\": "
         << json_number(arm.projected_displacement)
         << ", \"transverse_drift\": " << json_number(arm.transverse_drift)
         << ", \"site_hops\": " << arm.hops
         << ", \"shared_anchor_states\": " << arm.shared_states
         << ", \"maximum_anchor_multiplicity\": "
         << arm.maximum_multiplicity
         << ", \"worst_common_gate\": " << arm.worst_gate
         << ", \"maximum_energy_drift\": " << arm.energy_drift
         << ", \"minimum_pair_distance\": "
         << json_number(arm.minimum_distance)
         << ", \"maximum_pair_distance\": "
         << json_number(arm.maximum_distance)
         << ", \"maximum_pseudomomentum_defect\": "
         << arm.maximum_pseudomomentum_defect
         << ", \"reverse_recovery\": " << json_number(arm.recovery) << "}"
         << (i + 1 == summary.arms.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";

  std::ofstream csv(dir / "ftd_0613_refined_directional_boost_arms_v1.csv");
  csv << "ftd_id,speed_index,direction_index,speed,ticks,complete,base_pass,"
         "mobility_pass,projected_displacement,transverse_drift,site_hops,"
         "shared_states,max_multiplicity,worst_gate,energy_drift,"
         "min_distance,max_distance,pseudomomentum_defect,recovery\n";
  for (const auto& arm : summary.arms)
    csv << std::setprecision(17) << "FTD-0613," << arm.speed_index << ','
        << arm.direction_index << ',' << arm.speed << ',' << arm.ticks << ','
        << (arm.complete ? 1 : 0) << ',' << (arm.base_pass ? 1 : 0) << ','
        << (arm.mobility_pass ? 1 : 0) << ',' << arm.projected_displacement
        << ',' << arm.transverse_drift << ',' << arm.hops << ','
        << arm.shared_states << ',' << arm.maximum_multiplicity << ','
        << arm.worst_gate << ',' << arm.energy_drift << ','
        << arm.minimum_distance << ',' << arm.maximum_distance << ','
        << arm.maximum_pseudomomentum_defect << ',' << arm.recovery << '\n';
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  BoostSummary summary;
  ftd::eft::ChargedTrimerOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-13;
  options.max_iterations = 64;
  options.allow_shared_anchor_chart = true;
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const auto green = make_green_kernel();
  const double beta = normalization.mapped_field_work_coefficient;
  const Matrix3 cyclic_orientation{{{0.0, 1.0, 0.0},
                                    {0.0, 0.0, 1.0},
                                    {1.0, 0.0, 0.0}}};
  std::vector<StaticSearch> searches;
  if (normalization.valid && green.valid
      && green.residual <= direct_tolerance) {
    for (double tx : {0.0, 0.5})
      for (double ty : {0.0, 0.5})
        for (double tz : {0.0, 0.5})
          for (int orientation = 0; orientation < 2; ++orientation)
            searches.push_back(search_static_core({tx, ty, tz},
                orientation == 0 ? identity_matrix() : cyclic_orientation,
                orientation, options, green, beta));
  }
  StaticCoreEvaluation initial;
  for (const auto& search : searches)
    if (search.terminated && search.minimum.valid
        && (!initial.valid || search.minimum.energy < initial.energy))
      initial = search.minimum;
  const auto refined = initial.valid
      ? refine_static_state(initial, options, green, beta) : RefineResult{};
  summary.refined_energy = refined.state.energy;
  summary.refined_gradient = refined.derivatives.gradient_inf;
  summary.rest_fingerprint = searches.size() == 16 && refined.coverage
      && refined.converged
      && std::abs(refined.state.energy - locked_refined_energy) <= 1e-15
      && refined.derivatives.gradient_inf <= 1e-10
      && refined.derivatives.positive_modes == static_dof;

  const std::size_t count = static_cast<std::size_t>(L) * L * L;
  std::vector<double> uniform(count, -1.0 / static_cast<double>(count));
  ChargedTrimerState rest_state = refined.state.state;
  if (summary.rest_fingerprint) {
    auto total = coat_density(rest_state);
    if (total.size() == count) {
      for (std::size_t i = 0; i < count; ++i) total[i] += uniform[i];
      const auto direct = initialize_minimum_energy(total);
      if (direct.valid) rest_state.electric = direct.electric;
      NeutralizerFixture fixture;
      fixture.name = "boost_rest";
      fixture.state = rest_state;
      fixture.stationary = uniform;
      fixture.valid = direct.valid;
      if (direct.valid)
        summary.rest = run_arm(0, 0, fixture, 0.0, 64, options);
      summary.rest_gate = direct.valid && summary.rest.complete
          && summary.rest.worst_gate <= gate
          && summary.rest.energy_drift <= 1e-10
          && std::abs(summary.rest.longitudinal) <= 1e-9
          && summary.rest.transverse <= 1e-9
          && summary.rest.momentum_change <= 1e-9
          && summary.rest.recovery <= 1e-9;
    }
  }

  const std::array<double, 3> speeds{{1.0/128.0, 1.0/64.0, 1.0/32.0}};
  const std::array<int, 3> ticks{{256, 128, 64}};
  const std::array<Vec3, 6> directions{{
      {+1,0,0},{-1,0,0},{0,+1,0},{0,-1,0},{0,0,+1},{0,0,-1}}};
  if (summary.rest_gate)
    for (int speed = 0; speed < 3; ++speed)
      for (int direction = 0; direction < 6; ++direction)
        summary.arms.push_back(run_boost_arm(speed, direction, rest_state,
            uniform, speeds[speed], ticks[speed], directions[direction],
            options));
  summary.arm_coverage = summary.arms.size() == 18
      && std::all_of(summary.arms.begin(), summary.arms.end(),
          [](const BoostArm& arm) { return arm.complete; });

  summary.sign_residual = 0.0;
  summary.axis_residual = 0.0;
  summary.symmetry_coverage = summary.arm_coverage;
  if (summary.arm_coverage) {
    for (int speed = 0; speed < 3; ++speed) {
      std::array<double, 3> projected_means{};
      std::array<double, 3> transverse_means{};
      for (int axis = 0; axis < 3; ++axis) {
        const auto& positive = summary.arms[6 * speed + 2 * axis];
        const auto& negative = summary.arms[6 * speed + 2 * axis + 1];
        summary.sign_residual = std::max(summary.sign_residual,
            std::max(std::abs(positive.projected_displacement
                              - negative.projected_displacement),
                     std::abs(positive.transverse_drift
                              - negative.transverse_drift)));
        projected_means[axis] = 0.5 * (positive.projected_displacement
                                       + negative.projected_displacement);
        transverse_means[axis] = 0.5 * (positive.transverse_drift
                                        + negative.transverse_drift);
      }
      const auto projected_range = std::minmax_element(
          projected_means.begin(), projected_means.end());
      const auto transverse_range = std::minmax_element(
          transverse_means.begin(), transverse_means.end());
      summary.axis_residual = std::max(summary.axis_residual,
          std::max(*projected_range.second - *projected_range.first,
                   *transverse_range.second - *transverse_range.first));
    }
  }
  summary.symmetry_pass = summary.symmetry_coverage
      && summary.sign_residual <= 0.25 && summary.axis_residual <= 0.25;

  const bool all_base = summary.arm_coverage
      && std::all_of(summary.arms.begin(), summary.arms.end(),
          [](const BoostArm& arm) { return arm.base_pass; });
  const bool all_mobile = all_base && summary.symmetry_pass
      && std::all_of(summary.arms.begin(), summary.arms.end(),
          [](const BoostArm& arm) { return arm.mobility_pass; });
  bool all_fast = true, lower_failure = false;
  for (const auto& arm : summary.arms) {
    if (arm.speed_index == 2) all_fast = all_fast && arm.mobility_pass;
    else lower_failure = lower_failure || !arm.mobility_pass;
  }
  const bool coverage = summary.rest_fingerprint && summary.rest_gate
      && summary.arm_coverage && summary.symmetry_coverage;
  if (!coverage)
    summary.verdict =
        "REFINED_COMPACT_CORE_DIRECTIONAL_BOOST_NUMERICALLY_UNRESOLVED";
  else if (all_mobile)
    summary.verdict =
        "REFINED_COMPACT_CORE_DIRECTIONALLY_MOBILE_CONSTRUCTIVE";
  else if (all_base && summary.symmetry_pass && all_fast && lower_failure)
    summary.verdict =
        "REFINED_COMPACT_CORE_DEPINNING_THRESHOLD_MEASURED";
  else
    summary.verdict =
        "REFINED_COMPACT_CORE_DIRECTIONAL_MOBILITY_CLOSED_NEGATIVE";
  write_boost_record(summary);
  std::cout << "protocol_sha256=" << boost_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "rest=" << summary.rest_gate
            << " arms=" << summary.arms.size()
            << " sign_residual=" << summary.sign_residual
            << " axis_residual=" << summary.axis_residual << '\n';
  for (const auto& arm : summary.arms)
    std::cout << "speed=" << arm.speed << " direction="
              << arm.direction_index << " displacement="
              << arm.projected_displacement << " transverse="
              << arm.transverse_drift << " pass=" << arm.mobility_pass
              << '\n';
  return summary.arms.size() == 18 ? 0 : 1;
}
