/** FTD-0737: precontact energetic-capture delay discriminator. */

#define main ftd_0731_source_of_record_main
#include "test_multipass_formation_persistence.cpp"
#undef main

#include <future>

namespace {

constexpr char kDelayProtocolSha256[] =
    "677B054C1C52470F85B272FBD575880274431EB2FF4CEDAB2A4A59C7EAC816C7";
constexpr int kDelayL = 129;
constexpr int kDelayTicks = 122;
constexpr int kDelaySourceCap = 3;
constexpr int kDelayContactTick = kDelayL-2*kDelaySourceCap;

struct DelayRow {
  std::string direction;
  std::string phase;
  int tick = 0;
  bool valid = false;
  bool common = false;
  double maximum_residual = INFINITY;
  double total_energy_residual = INFINITY;
  double recoil_defect = INFINITY;
  double causal_speed_excess = INFINITY;
  int source_radius = 0;
  int source_entries = 0;
  double separation = INFINITY;
  double pair_energy = INFINITY;
  double field_energy = INFINITY;
  bool graph_inside = false;
};

struct DelayArm {
  std::string direction;
  int expected_reentry = -1;
  bool initialized = false;
  bool forward_executed = false;
  bool reverse_executed = false;
  bool algebra_pass = false;
  bool support_pass = false;
  bool initial_pass = false;
  bool transition_pass = false;
  bool onset_exists = false;
  bool delay_pass = false;
  bool tail_pass = false;
  bool receiver_pass = false;
  int onset_tick = -1;
  int maximum_source_radius = 0;
  int measured_contact_tick = 0;
  std::vector<int> transition_ticks;
  std::vector<double> separation_history;
  std::vector<double> internal_history;
  std::vector<double> field_history;
  double maximum_common_residual = 0.0;
  double maximum_energy_residual = 0.0;
  double maximum_recoil_defect = 0.0;
  double maximum_speed_excess = 0.0;
  double pair_field_balance = INFINITY;
  double field_gain = -INFINITY;
  double inverse_recovery = INFINITY;
  std::vector<DelayRow> rows;
};

int delay_periodic_abs(int value, int center, int L) {
  const int direct = std::abs(value-center);
  return std::min(direct, L-direct);
}

int delay_source_radius(
    const std::vector<ftd::eft::QuadraticCoatFaceCurrent>& segments,
    const Vec3& center, int& entries) {
  int maximum = 0;
  entries = 0;
  const int cx = static_cast<int>(std::llround(center.x));
  const int cy = static_cast<int>(std::llround(center.y));
  const int cz = static_cast<int>(std::llround(center.z));
  for (const auto& segment : segments)
    for (const auto& entry : segment.sparse_current) {
      if (entry.value == 0.0) continue;
      ++entries;
      maximum = std::max(maximum, 1+std::max({
          delay_periodic_abs(entry.face.x, cx, segment.L),
          delay_periodic_abs(entry.face.y, cy, segment.L),
          delay_periodic_abs(entry.face.z, cz, segment.L)}));
    }
  return maximum;
}

DelayRow delay_step_row(
    const Direction& direction, const std::string& phase, int tick,
    const ftd::eft::ConnectedMooreBlockStepResult& step,
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& options, double interaction_scale,
    const Vec3& center) {
  DelayRow row;
  row.direction = direction.label;
  row.phase = phase;
  row.tick = tick;
  row.valid = step.valid;
  row.common = step.common_action_gates_pass;
  row.maximum_residual = maximum_step_residual(step);
  row.total_energy_residual = step.total_energy_residual;
  row.recoil_defect = std::max({
      step.matter_momentum_before.mag(), step.matter_momentum_after.mag(),
      step.spline_defect_norm});
  row.causal_speed_excess = step.causal_speed_excess;
  row.source_radius = delay_source_radius(
      step.segments, center, row.source_entries);
  row.separation = pair_separation(state);
  row.pair_energy = pair_internal_energy(state, options);
  row.field_energy = field_energy(state, options, interaction_scale);
  row.graph_inside = graph_inside(row.separation, options);
  return row;
}

void absorb_delay_diagnostics(DelayArm& arm, const DelayRow& row,
                              bool& common, bool& recoil, bool& speed) {
  arm.maximum_source_radius = std::max(
      arm.maximum_source_radius, row.source_radius);
  arm.maximum_common_residual = std::max(
      arm.maximum_common_residual, row.maximum_residual);
  arm.maximum_energy_residual = std::max(
      arm.maximum_energy_residual, row.total_energy_residual);
  arm.maximum_recoil_defect = std::max(
      arm.maximum_recoil_defect, row.recoil_defect);
  arm.maximum_speed_excess = std::max(
      arm.maximum_speed_excess, row.causal_speed_excess);
  common = common && row.common && row.maximum_residual <= kGate;
  recoil = recoil && row.recoil_defect <= 1e-9;
  speed = speed && row.causal_speed_excess <= 1e-12;
}

DelayArm run_delay_arm(
    const Direction& direction, const ConnectedMooreBlockOptions& options,
    double interaction_scale) {
  DelayArm arm;
  arm.direction = direction.label;
  arm.expected_reentry = direction.parent_reentry_tick;
  const Vec3 center{static_cast<double>(kDelayL/2),
                    static_cast<double>(kDelayL/2),
                    static_cast<double>(kDelayL/2)};
  const auto initial = ftd::eft::redress_derived_compact_pair(
      make_geometry(kDelayL, direction, false, 1.30, 0.0120),
      options, 1e-13, 4096);
  arm.initialized = initial.valid;
  if (!initial.valid) return arm;

  ConnectedMooreBlockState state = initial.state;
  const ConnectedMooreBlockState original = state;
  arm.separation_history.push_back(pair_separation(state));
  arm.internal_history.push_back(pair_internal_energy(state, options));
  arm.field_history.push_back(field_energy(state, options, interaction_scale));
  bool edge = graph_inside(arm.separation_history.front(), options);
  DelayRow initial_row;
  initial_row.direction = direction.label;
  initial_row.phase = "forward";
  initial_row.tick = 0;
  initial_row.valid = true;
  initial_row.common = true;
  initial_row.maximum_residual = 0.0;
  initial_row.total_energy_residual = 0.0;
  initial_row.recoil_defect = 0.0;
  initial_row.causal_speed_excess = 0.0;
  initial_row.separation = arm.separation_history.front();
  initial_row.pair_energy = arm.internal_history.front();
  initial_row.field_energy = arm.field_history.front();
  initial_row.graph_inside = edge;
  arm.rows.push_back(std::move(initial_row));
  arm.initial_pass = !edge && arm.internal_history.front() > 1e-6;

  bool common = true, recoil = true, speed = true, forward_valid = true;
  ConnectedMooreBlockSolveCache forward_cache;
  for (int tick = 1; tick <= kDelayTicks; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state, options, &forward_cache);
    forward_valid = forward_valid && step.valid;
    if (!step.valid) break;
    state = step.later;
    auto row = delay_step_row(
        direction, "forward", tick, step, state, options,
        interaction_scale, center);
    absorb_delay_diagnostics(arm, row, common, recoil, speed);
    if (step.relational_graph_changed) arm.transition_ticks.push_back(tick);
    edge = step.relational_edge_after;
    arm.separation_history.push_back(row.separation);
    arm.internal_history.push_back(row.pair_energy);
    arm.field_history.push_back(row.field_energy);
    arm.rows.push_back(std::move(row));
  }
  arm.forward_executed = forward_valid
      && arm.separation_history.size()
          == static_cast<std::size_t>(kDelayTicks+1);
  if (!arm.forward_executed) return arm;

  arm.transition_pass = arm.transition_ticks.size() == 3
      && arm.transition_ticks[0] == 7 && arm.transition_ticks[1] == 26
      && arm.transition_ticks[2] == direction.parent_reentry_tick;
  if (arm.transition_pass) {
    for (int candidate = arm.transition_ticks[2];
         candidate <= kDelayTicks; ++candidate) {
      bool tail = true;
      for (int tick = candidate; tick <= kDelayTicks; ++tick)
        tail = tail && graph_inside(
            arm.separation_history[static_cast<std::size_t>(tick)], options)
            && arm.internal_history[static_cast<std::size_t>(tick)] < -1e-6;
      if (tail) {
        arm.onset_tick = candidate;
        arm.onset_exists = true;
        break;
      }
    }
  }
  arm.delay_pass = arm.onset_exists
      && arm.onset_tick == direction.parent_reentry_tick+15;
  arm.tail_pass = arm.onset_exists;
  arm.field_gain = arm.field_history.back()-arm.field_history.front();
  arm.receiver_pass = arm.field_gain > 1e-6;
  arm.pair_field_balance = std::abs(
      arm.field_history.back()-arm.field_history.front()
      +arm.internal_history.back()-arm.internal_history.front());

  ConnectedMooreBlockState recovered = state;
  bool reverse_valid = true;
  ConnectedMooreBlockSolveCache reverse_cache;
  for (int tick = 1; tick <= kDelayTicks; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_reverse(
        recovered, options, &reverse_cache);
    reverse_valid = reverse_valid && step.valid;
    if (!step.valid) break;
    recovered = step.earlier;
    auto row = delay_step_row(
        direction, "reverse", tick, step, recovered, options,
        interaction_scale, center);
    absorb_delay_diagnostics(arm, row, common, recoil, speed);
    arm.rows.push_back(std::move(row));
  }
  arm.reverse_executed = reverse_valid
      && arm.rows.size() == static_cast<std::size_t>(2*kDelayTicks+1);
  arm.inverse_recovery = arm.reverse_executed
      ? ftd::eft::connected_moore_block_state_max_difference(
          original, recovered) : INFINITY;
  arm.measured_contact_tick = kDelayL-2*arm.maximum_source_radius;
  arm.support_pass = arm.maximum_source_radius <= kDelaySourceCap
      && kDelayTicks < arm.measured_contact_tick
      && kDelayTicks < kDelayContactTick;
  arm.algebra_pass = arm.forward_executed && arm.reverse_executed
      && common && recoil && speed && arm.pair_field_balance <= 1e-8
      && arm.inverse_recovery <= 1e-8;
  return arm;
}

void write_delay_records(const std::vector<DelayArm>& arms,
                         const std::string& verdict) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0737";
  std::filesystem::create_directories(directory);
  std::ofstream csv(directory/
      "ftd_0737_precontact_energetic_capture_delay_v1.csv");
  csv << "direction,phase,tick,valid,common,max_residual,"
         "total_energy_residual,recoil_defect,causal_speed_excess,"
         "source_radius,source_entries,separation,pair_energy,field_energy,"
         "graph_inside\n" << std::setprecision(17);
  for (const auto& arm : arms)
    for (const auto& row : arm.rows)
      csv << row.direction << ',' << row.phase << ',' << row.tick << ','
          << row.valid << ',' << row.common << ',' << row.maximum_residual
          << ',' << row.total_energy_residual << ',' << row.recoil_defect
          << ',' << row.causal_speed_excess << ',' << row.source_radius
          << ',' << row.source_entries << ',' << row.separation << ','
          << row.pair_energy << ',' << row.field_energy << ','
          << row.graph_inside << '\n';

  int delay_passes = 0, tail_passes = 0, receiver_passes = 0;
  int maximum_source = 0, minimum_contact = kDelayL;
  double maximum_common = 0.0, maximum_energy = 0.0,
      maximum_recoil = 0.0, maximum_speed = 0.0,
      maximum_inverse = 0.0, maximum_balance = 0.0;
  for (const auto& arm : arms) {
    delay_passes += arm.delay_pass ? 1 : 0;
    tail_passes += arm.tail_pass ? 1 : 0;
    receiver_passes += arm.receiver_pass ? 1 : 0;
    maximum_source = std::max(maximum_source, arm.maximum_source_radius);
    minimum_contact = std::min(minimum_contact, arm.measured_contact_tick);
    maximum_common = std::max(maximum_common, arm.maximum_common_residual);
    maximum_energy = std::max(maximum_energy, arm.maximum_energy_residual);
    maximum_recoil = std::max(maximum_recoil, arm.maximum_recoil_defect);
    maximum_speed = std::max(maximum_speed, arm.maximum_speed_excess);
    maximum_inverse = std::max(maximum_inverse, arm.inverse_recovery);
    maximum_balance = std::max(maximum_balance, arm.pair_field_balance);
  }
  std::ofstream json(directory/
      "ftd_0737_precontact_energetic_capture_delay_v1.json");
  json << std::setprecision(17)
       << "{\n"
       << "  \"ftd_id\": \"FTD-0737\",\n"
       << "  \"protocol_sha256\": \"" << kDelayProtocolSha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"volume\": " << kDelayL << ",\n"
       << "  \"horizon\": " << kDelayTicks << ",\n"
       << "  \"source_radius_cap\": " << kDelaySourceCap << ",\n"
       << "  \"contact_tick\": " << kDelayContactTick << ",\n"
       << "  \"history_count\": " << arms.size() << ",\n"
       << "  \"row_count\": " << 3*(2*kDelayTicks+1) << ",\n"
       << "  \"delay_passes\": " << delay_passes << ",\n"
       << "  \"tail_passes\": " << tail_passes << ",\n"
       << "  \"receiver_passes\": " << receiver_passes << ",\n"
       << "  \"maximum_source_radius\": " << maximum_source << ",\n"
       << "  \"minimum_measured_contact_tick\": "
       << minimum_contact << ",\n"
       << "  \"maximum_common_residual\": " << maximum_common << ",\n"
       << "  \"maximum_energy_residual\": " << maximum_energy << ",\n"
       << "  \"maximum_recoil_defect\": " << maximum_recoil << ",\n"
       << "  \"maximum_speed_excess\": " << maximum_speed << ",\n"
       << "  \"maximum_inverse_recovery\": " << maximum_inverse << ",\n"
       << "  \"maximum_pair_field_balance\": " << maximum_balance << ",\n"
       << "  \"arms\": [\n";
  for (std::size_t i = 0; i < arms.size(); ++i) {
    const auto& arm = arms[i];
    std::ostringstream transition_text;
    for (std::size_t j = 0; j < arm.transition_ticks.size(); ++j) {
      if (j != 0) transition_text << ';';
      transition_text << arm.transition_ticks[j];
    }
    json << "    {\"direction\": \"" << arm.direction
         << "\", \"initialized\": " << arm.initialized
         << ", \"forward_executed\": " << arm.forward_executed
         << ", \"reverse_executed\": " << arm.reverse_executed
         << ", \"algebra_pass\": " << arm.algebra_pass
         << ", \"support_pass\": " << arm.support_pass
         << ", \"initial_pass\": " << arm.initial_pass
         << ", \"transition_pass\": " << arm.transition_pass
         << ", \"transition_ticks\": \"" << transition_text.str()
         << "\", \"onset_exists\": " << arm.onset_exists
         << ", \"onset_tick\": " << arm.onset_tick
         << ", \"delay_pass\": " << arm.delay_pass
         << ", \"tail_pass\": " << arm.tail_pass
         << ", \"receiver_pass\": " << arm.receiver_pass
         << ", \"maximum_source_radius\": "
         << arm.maximum_source_radius
         << ", \"measured_contact_tick\": "
         << arm.measured_contact_tick
         << ", \"field_gain\": " << arm.field_gain
         << ", \"inverse_recovery\": " << arm.inverse_recovery << "}"
         << (i+1 == arms.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";
}

}  // namespace

int main() {
  ConnectedMooreBlockOptions options;
  options.dt = 0.25;
  options.binding_law = ConnectedBindingLaw::DerivedCompactPair;
  options.compact_pair_well_depth = 0.01;
  options.compact_pair_cutoff_distance_squared = 1.5;
  options.allow_shared_anchor_chart = true;
  options.gate_tolerance = kGate;
  options.solve_tolerance = 2e-14;
  options.max_iterations = 384;
  options.use_sparse_local_current = true;
  options.use_local_residual_evaluation = true;
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const double interaction_scale =
      normalization.mapped_field_work_coefficient;

  std::vector<std::future<DelayArm>> futures;
  for (const auto& direction : kDirections)
    futures.push_back(std::async(std::launch::async, [=]() {
      return run_delay_arm(direction, options, interaction_scale);
    }));
  std::vector<DelayArm> arms;
  for (auto& future : futures) arms.push_back(future.get());
  std::sort(arms.begin(), arms.end(), [](const DelayArm& a,
                                        const DelayArm& b) {
    return a.direction < b.direction;
  });

  const bool infrastructure = normalization.valid && arms.size() == 3
      && std::all_of(arms.begin(), arms.end(), [](const DelayArm& arm) {
        return arm.initialized && arm.forward_executed
            && arm.reverse_executed && arm.algebra_pass
            && arm.support_pass && arm.initial_pass
            && arm.rows.size() == static_cast<std::size_t>(2*kDelayTicks+1);
      });
  const bool transitions = infrastructure && std::all_of(
      arms.begin(), arms.end(), [](const DelayArm& arm) {
        return arm.transition_pass;
      });
  const bool onset_exists = transitions && std::all_of(
      arms.begin(), arms.end(), [](const DelayArm& arm) {
        return arm.onset_exists;
      });
  const bool delay = onset_exists && std::all_of(
      arms.begin(), arms.end(), [](const DelayArm& arm) {
        return arm.delay_pass;
      });
  const bool tail = delay && std::all_of(
      arms.begin(), arms.end(), [](const DelayArm& arm) {
        return arm.tail_pass;
      });
  const bool receiver = tail && std::all_of(
      arms.begin(), arms.end(), [](const DelayArm& arm) {
        return arm.receiver_pass;
      });

  std::string verdict;
  if (!infrastructure)
    verdict = "PRECONTACT_ENERGETIC_CAPTURE_EXECUTION_INVALID";
  else if (!transitions)
    verdict = "PRECONTACT_REENTRY_SEQUENCE_NOT_REPRODUCED";
  else if (!onset_exists || !delay)
    verdict = "PRECONTACT_ENERGETIC_DELAY_NOT_REPRODUCED";
  else if (!tail)
    verdict = "PRECONTACT_ENERGETIC_CORE_RELEASES";
  else if (!receiver)
    verdict = "PRECONTACT_CAPTURE_WITHOUT_FIELD_ENERGY_RECEIVER";
  else
    verdict = "PRECONTACT_DELAYED_ENERGETIC_CAPTURE_CONSTRUCTIVE";

  write_delay_records(arms, verdict);
  std::cout << "FTD-0737 " << verdict;
  for (const auto& arm : arms)
    std::cout << ' ' << arm.direction << '=' << arm.onset_tick;
  std::cout << " contact=" << kDelayContactTick << '\n';
  return infrastructure ? 0 : 1;
}
