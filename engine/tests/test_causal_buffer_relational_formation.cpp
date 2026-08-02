/** FTD-0736: causal-buffer relational-formation discriminator. */

// Reuse the locked FTD-0731 geometry, energy, and morphology definitions
// without editing that source-of-record runner.
#define main ftd_0731_source_of_record_main
#include "test_multipass_formation_persistence.cpp"
#undef main

#include <future>

namespace {

constexpr char kCausalProtocolSha256[] =
    "955FC3331A64B6DB7C495AE6ACFFE82DBE9ADE42DE730B68A7E2610F885EFFAB";
constexpr int kCausalL = 129;
constexpr int kCausalTicks = 112;
constexpr int kSourceRadiusCap = 8;
constexpr int kLockedContactTick = kCausalL - 2*kSourceRadiusCap;
constexpr std::array<int, 3> kCausalMorphologyTicks{{48, 96, 112}};

struct CausalStepRecord {
  std::string family;
  std::string direction;
  std::string polarity;
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
  bool morphology_measured = false;
  bool morphology_valid = false;
  double dynamic_norm = 0.0;
  double magnetic_energy = 0.0;
  int doubled_median_radius = 0;
};

struct CausalArm {
  std::string family;
  std::string direction;
  std::string polarity;
  int expected_reentry_tick = -1;
  bool initialized = false;
  bool forward_executed = false;
  bool reverse_executed = false;
  bool identity_pass = false;
  bool recoil_pass = false;
  bool speed_pass = false;
  bool inverse_pass = false;
  bool support_pass = false;
  bool morphology_observers_pass = false;
  bool initial_unbound_pass = false;
  bool transition_pass = false;
  bool post_reentry_persistent = false;
  bool receiver_pass = false;
  bool bound_control_pass = false;
  int maximum_source_radius = 0;
  int measured_contact_tick = 0;
  int source_entries = 0;
  std::vector<int> transition_ticks;
  std::vector<double> separation_history;
  std::vector<double> internal_history;
  std::vector<double> field_history;
  double maximum_common_residual = 0.0;
  double maximum_total_energy_residual = 0.0;
  double maximum_recoil_defect = 0.0;
  double maximum_causal_speed_excess = 0.0;
  double pair_field_balance = INFINITY;
  double field_gain = -INFINITY;
  double inverse_recovery = INFINITY;
  std::vector<CausalStepRecord> steps;
};

int periodic_abs(int value, int center, int L) {
  const int direct = std::abs(value-center);
  return std::min(direct, L-direct);
}

int current_source_radius(
    const std::vector<ftd::eft::QuadraticCoatFaceCurrent>& segments,
    const Vec3& center, int& entries) {
  int maximum = 0;
  entries = 0;
  const int cx = static_cast<int>(std::llround(center.x));
  const int cy = static_cast<int>(std::llround(center.y));
  const int cz = static_cast<int>(std::llround(center.z));
  for (const auto& segment : segments) {
    for (const auto& entry : segment.sparse_current) {
      if (entry.value == 0.0) continue;
      ++entries;
      const int radius = 1 + std::max({
          periodic_abs(entry.face.x, cx, segment.L),
          periodic_abs(entry.face.y, cy, segment.L),
          periodic_abs(entry.face.z, cz, segment.L)});
      maximum = std::max(maximum, radius);
    }
  }
  return maximum;
}

CausalStepRecord make_step_record(
    const std::string& family, const Direction& direction, bool conjugate,
    const std::string& phase, int tick,
    const ftd::eft::ConnectedMooreBlockStepResult& step,
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& options, double interaction_scale,
    const Vec3& center) {
  CausalStepRecord result;
  result.family = family;
  result.direction = direction.label;
  result.polarity = conjugate ? "minus_plus" : "plus_minus";
  result.phase = phase;
  result.tick = tick;
  result.valid = step.valid;
  result.common = step.common_action_gates_pass;
  result.maximum_residual = maximum_step_residual(step);
  result.total_energy_residual = step.total_energy_residual;
  result.recoil_defect = std::max({
      step.matter_momentum_before.mag(), step.matter_momentum_after.mag(),
      step.spline_defect_norm});
  result.causal_speed_excess = step.causal_speed_excess;
  result.source_radius = current_source_radius(
      step.segments, center, result.source_entries);
  result.separation = pair_separation(state);
  result.pair_energy = pair_internal_energy(state, options);
  result.field_energy = field_energy(state, options, interaction_scale);
  result.graph_inside = graph_inside(result.separation, options);
  return result;
}

void measure_morphology(CausalStepRecord& record,
                        const ConnectedMooreBlockState& state,
                        const ConnectedMooreBlockOptions& options,
                        const Vec3& center, double interaction_scale) {
  const auto morphology = observe_dynamic_field(
      state, options, center, interaction_scale);
  record.morphology_measured = true;
  record.morphology_valid = morphology.valid;
  record.dynamic_norm = morphology.dynamic_norm;
  record.magnetic_energy = morphology.magnetic_energy;
  record.doubled_median_radius = morphology.doubled_median_radius;
}

CausalArm run_causal_arm(
    const std::string& family, const Direction& direction, bool conjugate,
    const ConnectedMooreBlockOptions& options, double interaction_scale) {
  CausalArm result;
  result.family = family;
  result.direction = direction.label;
  result.polarity = conjugate ? "minus_plus" : "plus_minus";
  result.expected_reentry_tick = direction.parent_reentry_tick;
  const bool unbound = family == "unbound";
  const double separation = unbound ? 1.30 : 1.00;
  const double momentum = unbound ? 0.0120 : kBoundMomentum;
  const Vec3 center{static_cast<double>(kCausalL/2),
                    static_cast<double>(kCausalL/2),
                    static_cast<double>(kCausalL/2)};
  const auto initial = ftd::eft::redress_derived_compact_pair(
      make_geometry(kCausalL, direction, conjugate, separation, momentum),
      options, 1e-13, 4096);
  result.initialized = initial.valid;
  if (!initial.valid) return result;

  ConnectedMooreBlockState state = initial.state;
  const ConnectedMooreBlockState original = state;
  result.separation_history.push_back(pair_separation(state));
  result.internal_history.push_back(pair_internal_energy(state, options));
  result.field_history.push_back(field_energy(
      state, options, interaction_scale));
  bool edge = graph_inside(result.separation_history.front(), options);
  CausalStepRecord initial_record;
  initial_record.family = family;
  initial_record.direction = direction.label;
  initial_record.polarity = conjugate ? "minus_plus" : "plus_minus";
  initial_record.phase = "forward";
  initial_record.tick = 0;
  initial_record.valid = true;
  initial_record.common = true;
  initial_record.maximum_residual = 0.0;
  initial_record.total_energy_residual = 0.0;
  initial_record.recoil_defect = 0.0;
  initial_record.causal_speed_excess = 0.0;
  initial_record.separation = result.separation_history.front();
  initial_record.pair_energy = result.internal_history.front();
  initial_record.field_energy = result.field_history.front();
  initial_record.graph_inside = edge;
  result.steps.push_back(std::move(initial_record));
  result.initial_unbound_pass = !edge
      && result.internal_history.front() > 1e-6;
  bool common = true;
  bool recoil = true;
  bool speed = true;
  bool forward_valid = true;
  ConnectedMooreBlockSolveCache forward_cache;

  for (int tick = 1; tick <= kCausalTicks; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state, options, &forward_cache);
    forward_valid = forward_valid && step.valid;
    if (!step.valid) break;
    state = step.later;
    auto record = make_step_record(
        family, direction, conjugate, "forward", tick, step, state,
        options, interaction_scale, center);
    if (unbound && std::find(kCausalMorphologyTicks.begin(),
                            kCausalMorphologyTicks.end(), tick)
                       != kCausalMorphologyTicks.end())
      measure_morphology(record, state, options, center, interaction_scale);
    result.maximum_source_radius = std::max(
        result.maximum_source_radius, record.source_radius);
    result.source_entries += record.source_entries;
    result.maximum_common_residual = std::max(
        result.maximum_common_residual, record.maximum_residual);
    result.maximum_total_energy_residual = std::max(
        result.maximum_total_energy_residual,
        record.total_energy_residual);
    result.maximum_recoil_defect = std::max(
        result.maximum_recoil_defect, record.recoil_defect);
    result.maximum_causal_speed_excess = std::max(
        result.maximum_causal_speed_excess, record.causal_speed_excess);
    common = common && record.common
        && record.maximum_residual <= kGate;
    recoil = recoil && record.recoil_defect <= 1e-9;
    speed = speed && record.causal_speed_excess <= 1e-12;
    if (step.relational_graph_changed)
      result.transition_ticks.push_back(tick);
    edge = step.relational_edge_after;
    result.separation_history.push_back(record.separation);
    result.internal_history.push_back(record.pair_energy);
    result.field_history.push_back(record.field_energy);
    result.steps.push_back(std::move(record));
  }

  result.forward_executed = forward_valid
      && result.separation_history.size()
          == static_cast<std::size_t>(kCausalTicks+1);
  if (!result.forward_executed) return result;

  result.field_gain = result.field_history.back()
      - result.field_history.front();
  result.pair_field_balance = std::abs(
      result.field_history.back()-result.field_history.front()
      + result.internal_history.back()-result.internal_history.front());
  result.measured_contact_tick = kCausalL-2*result.maximum_source_radius;

  if (unbound) {
    result.transition_pass = result.transition_ticks.size() == 3
        && std::abs(result.transition_ticks[0]-7) <= 2
        && std::abs(result.transition_ticks[1]-26) <= 2
        && std::abs(result.transition_ticks[2]
                    -direction.parent_reentry_tick) <= 2;
    result.post_reentry_persistent = result.transition_pass;
    if (result.transition_pass) {
      for (int tick = result.transition_ticks[2]; tick <= kCausalTicks; ++tick)
        result.post_reentry_persistent = result.post_reentry_persistent
            && graph_inside(result.separation_history[
                                static_cast<std::size_t>(tick)], options)
            && result.internal_history[static_cast<std::size_t>(tick)] < -1e-6;
    }
    int measured = 0;
    bool receiver = false;
    for (const auto& record : result.steps) {
      if (!record.morphology_measured) continue;
      ++measured;
      receiver = receiver || (record.morphology_valid
          && record.dynamic_norm > 1e-8
          && record.magnetic_energy > 1e-10
          && record.doubled_median_radius >= 5);
    }
    result.morphology_observers_pass = measured == 3
        && std::all_of(result.steps.begin(), result.steps.end(),
            [](const CausalStepRecord& record) {
              return !record.morphology_measured || record.morphology_valid;
            });
    result.receiver_pass = result.field_gain > 1e-6 && receiver;
  } else {
    result.morphology_observers_pass = true;
    result.bound_control_pass = result.transition_ticks.empty();
    for (std::size_t tick = 0; tick < result.internal_history.size(); ++tick)
      result.bound_control_pass = result.bound_control_pass
          && graph_inside(result.separation_history[tick], options)
          && result.internal_history[tick] < -1e-6;
  }

  ConnectedMooreBlockState recovered = state;
  bool reverse_valid = true;
  ConnectedMooreBlockSolveCache reverse_cache;
  for (int tick = 1; tick <= kCausalTicks; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_reverse(
        recovered, options, &reverse_cache);
    reverse_valid = reverse_valid && step.valid;
    if (!step.valid) break;
    recovered = step.earlier;
    auto record = make_step_record(
        family, direction, conjugate, "reverse", tick, step, recovered,
        options, interaction_scale, center);
    result.maximum_source_radius = std::max(
        result.maximum_source_radius, record.source_radius);
    result.source_entries += record.source_entries;
    result.maximum_common_residual = std::max(
        result.maximum_common_residual, record.maximum_residual);
    result.maximum_total_energy_residual = std::max(
        result.maximum_total_energy_residual,
        record.total_energy_residual);
    result.maximum_recoil_defect = std::max(
        result.maximum_recoil_defect, record.recoil_defect);
    result.maximum_causal_speed_excess = std::max(
        result.maximum_causal_speed_excess, record.causal_speed_excess);
    common = common && record.common
        && record.maximum_residual <= kGate;
    recoil = recoil && record.recoil_defect <= 1e-9;
    speed = speed && record.causal_speed_excess <= 1e-12;
    result.steps.push_back(std::move(record));
  }
  result.reverse_executed = reverse_valid
      && result.steps.size() == static_cast<std::size_t>(2*kCausalTicks+1);
  result.inverse_recovery = result.reverse_executed
      ? ftd::eft::connected_moore_block_state_max_difference(
          original, recovered)
      : INFINITY;
  result.identity_pass = result.forward_executed && result.reverse_executed
      && common;
  result.recoil_pass = recoil;
  result.speed_pass = speed;
  result.inverse_pass = result.reverse_executed
      && result.inverse_recovery <= 1e-8;
  result.measured_contact_tick = kCausalL-2*result.maximum_source_radius;
  result.support_pass = result.maximum_source_radius <= kSourceRadiusCap
      && kCausalTicks < result.measured_contact_tick
      && kCausalTicks < kLockedContactTick;
  return result;
}

const CausalArm* find_causal_arm(
    const std::vector<CausalArm>& arms, const std::string& family,
    const std::string& direction, const std::string& polarity) {
  const auto found = std::find_if(
      arms.begin(), arms.end(), [&](const CausalArm& arm) {
        return arm.family == family && arm.direction == direction
            && arm.polarity == polarity;
      });
  return found == arms.end() ? nullptr : &*found;
}

double scalar_history_difference(const CausalArm& first,
                                 const CausalArm& second) {
  if (first.separation_history.size() != second.separation_history.size()
      || first.internal_history.size() != second.internal_history.size()
      || first.field_history.size() != second.field_history.size())
    return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < first.separation_history.size(); ++i) {
    result = std::max(result, std::abs(
        first.separation_history[i]-second.separation_history[i]));
    result = std::max(result, std::abs(
        first.internal_history[i]-second.internal_history[i]));
    result = std::max(result, std::abs(
        first.field_history[i]-second.field_history[i]));
  }
  return result;
}

void write_causal_records(const std::vector<CausalArm>& arms,
                          const std::string& verdict,
                          double polarity_difference) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0736";
  std::filesystem::create_directories(directory);
  std::ofstream csv(directory/
      "ftd_0736_causal_buffer_relational_formation_v1.csv");
  csv << "family,direction,polarity,phase,tick,valid,common,max_residual,"
         "total_energy_residual,recoil_defect,causal_speed_excess,"
         "source_radius,source_entries,separation,pair_energy,field_energy,"
         "graph_inside,morphology_measured,morphology_valid,dynamic_norm,"
         "magnetic_energy,doubled_median_radius\n"
      << std::setprecision(17);
  for (const auto& arm : arms)
    for (const auto& row : arm.steps)
      csv << row.family << ',' << row.direction << ',' << row.polarity << ','
          << row.phase << ',' << row.tick << ',' << row.valid << ','
          << row.common << ',' << row.maximum_residual << ','
          << row.total_energy_residual << ',' << row.recoil_defect << ','
          << row.causal_speed_excess << ',' << row.source_radius << ','
          << row.source_entries << ',' << row.separation << ','
          << row.pair_energy << ',' << row.field_energy << ','
          << row.graph_inside << ',' << row.morphology_measured << ','
          << row.morphology_valid << ',' << row.dynamic_norm << ','
          << row.magnetic_energy << ',' << row.doubled_median_radius << '\n';

  int formed = 0, persistent = 0, receiver = 0, bound = 0;
  std::size_t step_rows = 0;
  int maximum_source_radius = 0;
  int minimum_contact_tick = kCausalL;
  double maximum_common = 0.0, maximum_energy = 0.0,
      maximum_recoil = 0.0, maximum_speed = 0.0,
      maximum_inverse = 0.0, maximum_balance = 0.0;
  for (const auto& arm : arms) {
    step_rows += arm.steps.size();
    if (arm.family == "unbound") {
      formed += arm.transition_pass ? 1 : 0;
      persistent += arm.post_reentry_persistent ? 1 : 0;
      receiver += arm.receiver_pass ? 1 : 0;
    } else {
      bound += arm.bound_control_pass ? 1 : 0;
    }
    maximum_source_radius = std::max(
        maximum_source_radius, arm.maximum_source_radius);
    minimum_contact_tick = std::min(
        minimum_contact_tick, arm.measured_contact_tick);
    maximum_common = std::max(maximum_common, arm.maximum_common_residual);
    maximum_energy = std::max(
        maximum_energy, arm.maximum_total_energy_residual);
    maximum_recoil = std::max(maximum_recoil, arm.maximum_recoil_defect);
    maximum_speed = std::max(
        maximum_speed, arm.maximum_causal_speed_excess);
    maximum_inverse = std::max(maximum_inverse, arm.inverse_recovery);
    maximum_balance = std::max(maximum_balance, arm.pair_field_balance);
  }

  std::ofstream json(directory/
      "ftd_0736_causal_buffer_relational_formation_v1.json");
  json << std::setprecision(17)
       << "{\n"
       << "  \"ftd_id\": \"FTD-0736\",\n"
       << "  \"protocol_sha256\": \"" << kCausalProtocolSha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"volume\": " << kCausalL << ",\n"
       << "  \"horizon\": " << kCausalTicks << ",\n"
       << "  \"source_radius_cap\": " << kSourceRadiusCap << ",\n"
       << "  \"locked_contact_tick\": " << kLockedContactTick << ",\n"
       << "  \"history_count\": " << arms.size() << ",\n"
       << "  \"step_row_count\": " << step_rows << ",\n"
       << "  \"unbound_formed\": " << formed << ",\n"
       << "  \"unbound_persistent\": " << persistent << ",\n"
       << "  \"unbound_receiver\": " << receiver << ",\n"
       << "  \"bound_controls\": " << bound << ",\n"
       << "  \"maximum_source_radius\": " << maximum_source_radius << ",\n"
       << "  \"minimum_measured_contact_tick\": "
       << minimum_contact_tick << ",\n"
       << "  \"polarity_scalar_difference\": "
       << polarity_difference << ",\n"
       << "  \"maximum_common_residual\": " << maximum_common << ",\n"
       << "  \"maximum_total_energy_residual\": "
       << maximum_energy << ",\n"
       << "  \"maximum_recoil_defect\": " << maximum_recoil << ",\n"
       << "  \"maximum_causal_speed_excess\": " << maximum_speed << ",\n"
       << "  \"maximum_inverse_recovery\": " << maximum_inverse << ",\n"
       << "  \"maximum_pair_field_balance\": " << maximum_balance << ",\n"
       << "  \"arms\": [\n";
  for (std::size_t i = 0; i < arms.size(); ++i) {
    const auto& arm = arms[i];
    std::ostringstream transitions;
    for (std::size_t j = 0; j < arm.transition_ticks.size(); ++j) {
      if (j != 0) transitions << ';';
      transitions << arm.transition_ticks[j];
    }
    json << "    {\"family\": \"" << arm.family
         << "\", \"direction\": \"" << arm.direction
         << "\", \"polarity\": \"" << arm.polarity
         << "\", \"initialized\": " << arm.initialized
         << ", \"forward_executed\": " << arm.forward_executed
         << ", \"reverse_executed\": " << arm.reverse_executed
         << ", \"identity_pass\": " << arm.identity_pass
         << ", \"recoil_pass\": " << arm.recoil_pass
         << ", \"speed_pass\": " << arm.speed_pass
         << ", \"inverse_pass\": " << arm.inverse_pass
         << ", \"support_pass\": " << arm.support_pass
         << ", \"morphology_observers_pass\": "
         << arm.morphology_observers_pass
         << ", \"initial_unbound_pass\": " << arm.initial_unbound_pass
         << ", \"transition_pass\": " << arm.transition_pass
         << ", \"post_reentry_persistent\": "
         << arm.post_reentry_persistent
         << ", \"receiver_pass\": " << arm.receiver_pass
         << ", \"bound_control_pass\": " << arm.bound_control_pass
         << ", \"transition_ticks\": \"" << transitions.str()
         << "\", \"maximum_source_radius\": "
         << arm.maximum_source_radius
         << ", \"measured_contact_tick\": "
         << arm.measured_contact_tick
         << ", \"field_gain\": " << arm.field_gain
         << ", \"pair_field_balance\": " << arm.pair_field_balance
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
  const Direction face = kDirections[0];
  const Direction edge = kDirections[1];
  const Direction body = kDirections[2];

  auto face_future = std::async(std::launch::async, [=]() {
    std::vector<CausalArm> result;
    result.push_back(run_causal_arm(
        "unbound", face, false, options, interaction_scale));
    result.push_back(run_causal_arm(
        "bound", face, false, options, interaction_scale));
    return result;
  });
  auto edge_future = std::async(std::launch::async, [=]() {
    std::vector<CausalArm> result;
    result.push_back(run_causal_arm(
        "unbound", edge, false, options, interaction_scale));
    return result;
  });
  auto body_future = std::async(std::launch::async, [=]() {
    std::vector<CausalArm> result;
    result.push_back(run_causal_arm(
        "unbound", body, false, options, interaction_scale));
    result.push_back(run_causal_arm(
        "unbound", body, true, options, interaction_scale));
    return result;
  });

  std::vector<CausalArm> arms;
  auto face_arms = face_future.get();
  auto edge_arms = edge_future.get();
  auto body_arms = body_future.get();
  for (auto* group : {&face_arms, &edge_arms, &body_arms})
    for (auto& arm : *group) arms.push_back(std::move(arm));
  std::sort(arms.begin(), arms.end(), [](const CausalArm& a,
                                        const CausalArm& b) {
    return std::tie(a.family, a.direction, a.polarity)
        < std::tie(b.family, b.direction, b.polarity);
  });

  const auto plus_body = find_causal_arm(
      arms, "unbound", "1_1_1", "plus_minus");
  const auto minus_body = find_causal_arm(
      arms, "unbound", "1_1_1", "minus_plus");
  const double polarity_difference = plus_body != nullptr
      && minus_body != nullptr
      ? scalar_history_difference(*plus_body, *minus_body) : INFINITY;
  const bool polarity_pass = plus_body != nullptr && minus_body != nullptr
      && plus_body->transition_ticks == minus_body->transition_ticks
      && polarity_difference <= 1e-9;

  const bool matrix = normalization.valid && arms.size() == 5
      && std::count_if(arms.begin(), arms.end(), [](const CausalArm& arm) {
           return arm.family == "unbound";
         }) == 4
      && std::count_if(arms.begin(), arms.end(), [](const CausalArm& arm) {
           return arm.family == "bound";
         }) == 1;
  const bool infrastructure = matrix && std::all_of(
      arms.begin(), arms.end(), [](const CausalArm& arm) {
        return arm.initialized && arm.forward_executed
            && arm.reverse_executed && arm.identity_pass
            && arm.recoil_pass && arm.speed_pass && arm.inverse_pass
            && arm.support_pass && arm.morphology_observers_pass
            && arm.pair_field_balance <= 1e-8;
      });
  const bool control = std::all_of(
      arms.begin(), arms.end(), [](const CausalArm& arm) {
        return arm.family != "bound" || arm.bound_control_pass;
      });
  const bool formed = std::all_of(
      arms.begin(), arms.end(), [](const CausalArm& arm) {
        return arm.family != "unbound"
            || (arm.initial_unbound_pass && arm.transition_pass);
      });
  const bool persistent = std::all_of(
      arms.begin(), arms.end(), [](const CausalArm& arm) {
        return arm.family != "unbound" || arm.post_reentry_persistent;
      });
  const bool receiver = std::all_of(
      arms.begin(), arms.end(), [](const CausalArm& arm) {
        return arm.family != "unbound" || arm.receiver_pass;
      });

  std::string verdict;
  if (!infrastructure)
    verdict = "CAUSAL_BUFFER_RELATIONAL_FORMATION_EXECUTION_INVALID";
  else if (!control)
    verdict = "CAUSAL_BUFFER_BOUND_CONTROL_UNSTABLE";
  else if (!polarity_pass)
    verdict = "CAUSAL_BUFFER_FORMATION_POLARITY_SENSITIVE";
  else if (!formed)
    verdict = "NO_PRECONTACT_RELATIONAL_FORMATION_ALL_RAYS";
  else if (!persistent)
    verdict = "PRECONTACT_REENTRY_WITHOUT_PERSISTENT_CORE";
  else if (!receiver)
    verdict = "PRECONTACT_CORE_WITHOUT_QUALIFIED_FIELD_RECEIVER";
  else
    verdict = "CAUSAL_BUFFER_RELATIONAL_FORMATION_CONSTRUCTIVE";

  write_causal_records(arms, verdict, polarity_difference);
  int formed_count = 0, persistent_count = 0, receiver_count = 0;
  int maximum_source = 0, minimum_contact = kCausalL;
  double maximum_inverse = 0.0;
  for (const auto& arm : arms) {
    if (arm.family == "unbound") {
      formed_count += arm.transition_pass ? 1 : 0;
      persistent_count += arm.post_reentry_persistent ? 1 : 0;
      receiver_count += arm.receiver_pass ? 1 : 0;
    }
    maximum_source = std::max(maximum_source, arm.maximum_source_radius);
    minimum_contact = std::min(minimum_contact, arm.measured_contact_tick);
    maximum_inverse = std::max(maximum_inverse, arm.inverse_recovery);
  }
  std::cout << "FTD-0736 " << verdict
            << " formed=" << formed_count << "/4"
            << " persistent=" << persistent_count << "/4"
            << " receiver=" << receiver_count << "/4"
            << " source_radius=" << maximum_source
            << " contact_tick=" << minimum_contact
            << " inverse=" << std::setprecision(8) << maximum_inverse
            << '\n';
  return infrastructure ? 0 : 1;
}
