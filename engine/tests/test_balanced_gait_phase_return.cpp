// FTD-0620: internal phase-return discriminator for the balanced neutral gait.
#define FTD0618_NO_MAIN
#include "test_closed_symmetry_balanced_gait.cpp"

namespace {

constexpr char phase_protocol_sha256[] =
    "A5B97A9251C46736065A1DD4A0ECA0CCDC28ED0CB7B9EF8FE74ACC494CB8B78C";
constexpr char phase_parent_0618_sha256[] =
    "5F04E64DFD7CBFD10CE3AC779361C4124654C817320DFC81E6D5A482889F54D3";
constexpr char phase_parent_0619_sha256[] =
    "0FEE2158E3DCB5EED2F837D74E89127F4B01160335057115F095FDF3C724669D";
constexpr int phase_active_ticks = 512;
constexpr int phase_rest_ticks = 128;
constexpr int phase_window_ticks = 128;

using SixVectors0620 = std::array<Vec3, 6>;

SixVectors0620 relative_positions_0620(
    const ClosedNeutralTrimerPairState& state) {
  SixVectors0620 result{};
  for (std::size_t offset : {std::size_t{0}, std::size_t{3}}) {
    std::array<Vec3, 3> unwrapped{};
    unwrapped[0] = effective_position(state.constituents[offset]);
    for (std::size_t a = 1; a < 3; ++a) {
      const Vec3 raw = effective_position(state.constituents[offset + a])
          - unwrapped[0];
      unwrapped[a] = unwrapped[0] + Vec3{
          periodic_delta_0618(raw.x), periodic_delta_0618(raw.y),
          periodic_delta_0618(raw.z)};
    }
    Vec3 centre{};
    for (const auto& x : unwrapped) centre += x;
    centre *= 1.0 / 3.0;
    for (std::size_t a = 0; a < 3; ++a)
      result[offset + a] = unwrapped[a] - centre;
  }
  return result;
}

SixVectors0620 internal_momenta_0620(
    const ClosedNeutralTrimerPairState& state) {
  SixVectors0620 result{};
  for (std::size_t offset : {std::size_t{0}, std::size_t{3}}) {
    Vec3 mean{};
    for (std::size_t a = 0; a < 3; ++a)
      mean += state.constituents[offset + a].momentum;
    mean *= 1.0 / 3.0;
    for (std::size_t a = 0; a < 3; ++a)
      result[offset + a] = state.constituents[offset + a].momentum - mean;
  }
  return result;
}

double maximum_vector_difference_0620(
    const SixVectors0620& lhs, const SixVectors0620& rhs) {
  double result = 0.0;
  for (std::size_t a = 0; a < lhs.size(); ++a)
    result = std::max(result, (lhs[a] - rhs[a]).mag());
  return result;
}

double vector_norm_0620(const SixVectors0620& values) {
  double norm2 = 0.0;
  for (const auto& value : values) norm2 += value.mag2();
  return std::sqrt(norm2);
}

InternalPattern core_pattern_0620(
    const SixVectors0620& values, std::size_t offset) {
  InternalPattern result{};
  for (std::size_t a = 0; a < 3; ++a) result[a] = values[offset + a];
  return result;
}

struct PhaseTick0620 {
  int tick = 0;
  Vec3 displacement{};
  double position_return = 0.0;
  double momentum_return = 0.0;
  double phase_distance = 0.0;
  double internal_momentum_norm = 0.0;
  double shape_mode0 = 0.0;
  double shape_mode1 = 0.0;
  double momentum_mode0 = 0.0;
  double momentum_mode1 = 0.0;
  double phase_angle = 0.0;
  double gate = 0.0;
  double energy_drift = 0.0;
};

struct ReturnEvent0620 {
  int tick = 0;
  double phase_distance = INFINITY;
  double position_return = INFINITY;
  double momentum_return = INFINITY;
  double displacement_z = 0.0;
  double phase_angle = 0.0;
};

struct PhaseArm0620 {
  int sign = 0;
  bool initialized = false;
  bool complete = false;
  bool algebraic_pass = false;
  bool recurrent = false;
  bool persistent = false;
  bool one_time_relaxation = false;
  int requested_ticks = 0;
  int forward_ticks = 0;
  int reverse_ticks = 0;
  int anchor_changes = 0;
  int maximum_multiplicity = 1;
  double amplitude = 0.0;
  double initial_internal_momentum_norm = INFINITY;
  double final_internal_momentum_norm = INFINITY;
  double minimum_phase_distance_after_32 = INFINITY;
  int minimum_phase_tick = -1;
  double period_relative_residual = INFINITY;
  double minimum_window_axial_displacement = INFINITY;
  double final_window_axial_displacement = INFINITY;
  double maximum_transverse = 0.0;
  double worst_gate = INFINITY;
  double maximum_energy_drift = INFINITY;
  double minimum_internal_distance = INFINITY;
  double maximum_internal_distance = 0.0;
  double recovery = INFINITY;
  std::vector<PhaseTick0620> samples;
  std::vector<ReturnEvent0620> returns;
};

PhaseTick0620 observe_phase_0620(
    int tick, const ClosedNeutralTrimerPairState& state,
    const ClosedNeutralTrimerPairState& initial,
    const SixVectors0620& relative0, const SixVectors0620& momentum0,
    const InternalBasis& basis, double amplitude, double previous_angle) {
  PhaseTick0620 sample;
  sample.tick = tick;
  sample.displacement = (core_displacement_0618(state, initial, 0)
      + core_displacement_0618(state, initial, 3)) * 0.5;
  const auto relative = relative_positions_0620(state);
  const auto momentum = internal_momenta_0620(state);
  sample.position_return = maximum_vector_difference_0620(relative, relative0);
  sample.momentum_return = maximum_vector_difference_0620(momentum, momentum0);
  sample.phase_distance = amplitude > 0.0
      ? std::max(sample.position_return, sample.momentum_return) / amplitude
      : 0.0;
  sample.internal_momentum_norm = vector_norm_0620(momentum);
  InternalPattern shape_delta{};
  for (std::size_t a = 0; a < 3; ++a)
    shape_delta[a] = relative[a] - relative0[a];
  const InternalPattern p_core = core_pattern_0620(momentum, 0);
  sample.shape_mode0 = pattern_dot_0615(basis.patterns[0], shape_delta);
  sample.shape_mode1 = pattern_dot_0615(basis.patterns[1], shape_delta);
  sample.momentum_mode0 = pattern_dot_0615(basis.patterns[0], p_core);
  sample.momentum_mode1 = pattern_dot_0615(basis.patterns[1], p_core);
  double angle = std::atan2(sample.momentum_mode1, sample.momentum_mode0);
  if (tick > 0) {
    const double pi = std::acos(-1.0);
    while (angle - previous_angle > pi) angle -= 2.0 * pi;
    while (angle - previous_angle < -pi) angle += 2.0 * pi;
  }
  sample.phase_angle = angle;
  return sample;
}

void classify_returns_0620(PhaseArm0620& arm) {
  if (arm.sign == 0 || arm.samples.size() < 3) return;
  const double threshold = arm.amplitude / 20.0;
  int last_return_tick = -1000;
  for (std::size_t i = 1; i + 1 < arm.samples.size(); ++i) {
    const auto& sample = arm.samples[i];
    if (sample.tick < 32) continue;
    if (sample.phase_distance < arm.minimum_phase_distance_after_32) {
      arm.minimum_phase_distance_after_32 = sample.phase_distance;
      arm.minimum_phase_tick = sample.tick;
    }
    const bool minimum = sample.phase_distance
            <= arm.samples[i - 1].phase_distance
        && sample.phase_distance < arm.samples[i + 1].phase_distance;
    if (minimum && sample.position_return <= threshold
        && sample.momentum_return <= threshold
        && sample.tick - last_return_tick >= 4) {
      arm.returns.push_back({sample.tick, sample.phase_distance,
          sample.position_return, sample.momentum_return,
          sample.displacement.z, sample.phase_angle});
      last_return_tick = sample.tick;
    }
  }
  if (arm.returns.size() >= 2) {
    const double t1 = arm.returns[0].tick;
    const double t2 = arm.returns[1].tick - arm.returns[0].tick;
    arm.period_relative_residual = std::abs(t1 - t2)
        / (0.5 * (t1 + t2));
    arm.recurrent = arm.period_relative_residual <= 0.1;
  }

  arm.minimum_window_axial_displacement = INFINITY;
  for (int end = phase_window_ticks; end <= arm.requested_ticks;
       end += phase_window_ticks) {
    const double distance = std::abs(arm.samples[static_cast<std::size_t>(end)]
        .displacement.z - arm.samples[static_cast<std::size_t>(end
        - phase_window_ticks)].displacement.z);
    arm.minimum_window_axial_displacement = std::min(
        arm.minimum_window_axial_displacement, distance);
  }
  arm.final_window_axial_displacement = std::abs(
      arm.samples.back().displacement.z
      - arm.samples[arm.samples.size() - 1 - phase_window_ticks].displacement.z);
  arm.persistent = arm.minimum_window_axial_displacement >= 0.5;
  arm.one_time_relaxation = !arm.recurrent
      && arm.final_internal_momentum_norm
          <= 0.1 * arm.initial_internal_momentum_norm
      && arm.final_window_axial_displacement < 0.1;
}

PhaseArm0620 run_phase_arm_0620(
    int sign, const BalancedRestContext& rest,
    const ftd::eft::ClosedNeutralPairOptions& options) {
  PhaseArm0620 arm;
  arm.sign = sign;
  arm.requested_ticks = sign == 0 ? phase_rest_ticks : phase_active_ticks;
  const auto fixture = make_pair_fixture_0618(sign, rest);
  arm.initialized = fixture.valid;
  arm.amplitude = fixture.amplitude;
  if (!fixture.valid) return arm;

  const ClosedNeutralTrimerPairState initial = fixture.state;
  ClosedNeutralTrimerPairState current = initial;
  const auto relative0 = relative_positions_0620(initial);
  const auto momentum0 = internal_momenta_0620(initial);
  arm.initial_internal_momentum_norm = vector_norm_0620(momentum0);
  arm.worst_gate = 0.0;
  arm.maximum_energy_drift = 0.0;
  double baseline_energy = NAN;
  arm.samples.reserve(static_cast<std::size_t>(arm.requested_ticks + 1));
  arm.samples.push_back(observe_phase_0620(0, current, initial, relative0,
      momentum0, rest.basis, arm.amplitude, 0.0));

  for (int tick = 0; tick < arm.requested_ticks; ++tick) {
    const auto step = ftd::eft::solve_closed_neutral_pair_forward(
        current, options);
    if (!step.valid) break;
    if (!std::isfinite(baseline_energy))
      baseline_energy = balanced_energy_before_0618(step);
    ++arm.forward_ticks;
    for (std::size_t a = 0; a < current.constituents.size(); ++a) {
      const auto& lhs = current.constituents[a].anchor;
      const auto& rhs = step.later.constituents[a].anchor;
      if (lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z)
        ++arm.anchor_changes;
    }
    arm.maximum_multiplicity = std::max(arm.maximum_multiplicity,
        maximum_anchor_multiplicity_0618(step.later));
    arm.worst_gate = std::max(arm.worst_gate, balanced_common_gate_0618(step));
    arm.maximum_energy_drift = std::max(arm.maximum_energy_drift,
        std::max(std::abs(balanced_energy_before_0618(step) - baseline_energy),
                 std::abs(balanced_energy_after_0618(step) - baseline_energy)));
    arm.minimum_internal_distance = std::min(
        arm.minimum_internal_distance, step.minimum_internal_pair_distance);
    arm.maximum_internal_distance = std::max(
        arm.maximum_internal_distance, step.maximum_internal_pair_distance);
    current = step.later;
    auto sample = observe_phase_0620(tick + 1, current, initial, relative0,
        momentum0, rest.basis, arm.amplitude, arm.samples.back().phase_angle);
    sample.gate = balanced_common_gate_0618(step);
    sample.energy_drift = arm.maximum_energy_drift;
    arm.maximum_transverse = std::max(arm.maximum_transverse,
        std::hypot(sample.displacement.x, sample.displacement.y));
    arm.samples.push_back(sample);
  }

  if (arm.forward_ticks == arm.requested_ticks) {
    arm.final_internal_momentum_norm = arm.samples.back().internal_momentum_norm;
    for (int tick = 0; tick < arm.requested_ticks; ++tick) {
      const auto step = ftd::eft::solve_closed_neutral_pair_reverse(
          current, options);
      if (!step.valid) break;
      ++arm.reverse_ticks;
      arm.maximum_multiplicity = std::max(arm.maximum_multiplicity,
          maximum_anchor_multiplicity_0618(step.earlier));
      arm.worst_gate = std::max(
          arm.worst_gate, balanced_common_gate_0618(step));
      arm.maximum_energy_drift = std::max(arm.maximum_energy_drift,
          std::max(std::abs(balanced_energy_before_0618(step) - baseline_energy),
                   std::abs(balanced_energy_after_0618(step) - baseline_energy)));
      arm.minimum_internal_distance = std::min(
          arm.minimum_internal_distance, step.minimum_internal_pair_distance);
      arm.maximum_internal_distance = std::max(
          arm.maximum_internal_distance, step.maximum_internal_pair_distance);
      current = step.earlier;
    }
  }

  arm.complete = arm.forward_ticks == arm.requested_ticks
      && arm.reverse_ticks == arm.requested_ticks;
  if (arm.complete)
    arm.recovery = ftd::eft::closed_neutral_pair_state_max_difference(
        initial, current);
  arm.algebraic_pass = arm.complete && arm.worst_gate <= 1e-12
      && arm.maximum_energy_drift <= 1e-10 && arm.recovery <= 1e-8
      && arm.maximum_multiplicity <= 2
      && arm.minimum_internal_distance >= 0.5
      && arm.maximum_internal_distance <= 2.0;
  classify_returns_0620(arm);
  return arm;
}

struct PhaseSummary0620 {
  bool parent_0618 = false;
  bool parent_0619 = false;
  bool arm_coverage = false;
  bool algebraic = false;
  bool rest_pass = false;
  bool sign_mirror_pass = false;
  double maximum_sign_mirror_residual = INFINITY;
  std::vector<PhaseArm0620> arms;
  std::string verdict;
};

void evaluate_phase_summary_0620(PhaseSummary0620& summary) {
  const auto find = [&](int sign) {
    return std::find_if(summary.arms.begin(), summary.arms.end(),
        [&](const PhaseArm0620& arm) { return arm.sign == sign; });
  };
  const auto rest = find(0), plus = find(+1), minus = find(-1);
  summary.arm_coverage = summary.arms.size() == 3
      && rest != summary.arms.end() && plus != summary.arms.end()
      && minus != summary.arms.end();
  summary.algebraic = summary.parent_0618 && summary.parent_0619
      && summary.arm_coverage
      && std::all_of(summary.arms.begin(), summary.arms.end(),
          [](const PhaseArm0620& arm) { return arm.algebraic_pass; });
  if (!summary.arm_coverage) return;
  summary.rest_pass = rest->samples.back().displacement.mag() <= 1e-8;
  summary.maximum_sign_mirror_residual = 0.0;
  for (std::size_t i = 0; i < plus->samples.size(); ++i) {
    const auto& a = plus->samples[i];
    const auto& b = minus->samples[i];
    summary.maximum_sign_mirror_residual = std::max(
        summary.maximum_sign_mirror_residual,
        std::max({(a.displacement + b.displacement).mag(),
                  std::abs(a.position_return - b.position_return),
                  std::abs(a.momentum_return - b.momentum_return)}));
  }
  summary.sign_mirror_pass = summary.maximum_sign_mirror_residual <= 1e-8;
  if (!summary.algebraic)
    summary.verdict = "BALANCED_GAIT_PHASE_RETURN_NUMERICALLY_UNRESOLVED";
  else if (summary.rest_pass && summary.sign_mirror_pass
           && plus->recurrent && minus->recurrent
           && plus->persistent && minus->persistent)
    summary.verdict = "RECURRENT_INTERNAL_GAIT_TRANSLATOR";
  else if (plus->one_time_relaxation && minus->one_time_relaxation)
    summary.verdict = "BALANCED_GAIT_ONE_TIME_RELAXATION";
  else if (plus->persistent && minus->persistent
           && !plus->recurrent && !minus->recurrent)
    summary.verdict = "PHASE_RETURN_NOT_OBSERVED_PERSISTENT_GAIT";
  else
    summary.verdict = "BALANCED_GAIT_PHASE_BEHAVIOR_MIXED";
}

void write_phase_record_0620(const PhaseSummary0620& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0620";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0620_balanced_gait_phase_return_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0620\",\n"
       << "  \"protocol_sha256\": \"" << phase_protocol_sha256 << "\",\n"
       << "  \"parent_0618_sha256\": \"" << phase_parent_0618_sha256 << "\",\n"
       << "  \"parent_0619_sha256\": \"" << phase_parent_0619_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"parent_0618_pass\": " << summary.parent_0618 << ",\n"
       << "  \"parent_0619_pass\": " << summary.parent_0619 << ",\n"
       << "  \"arm_coverage\": " << summary.arm_coverage << ",\n"
       << "  \"algebraic_pass\": " << summary.algebraic << ",\n"
       << "  \"rest_pass\": " << summary.rest_pass << ",\n"
       << "  \"sign_mirror_pass\": " << summary.sign_mirror_pass << ",\n"
       << "  \"maximum_sign_mirror_residual\": "
       << json_number(summary.maximum_sign_mirror_residual) << ",\n"
       << "  \"arms\": [\n";
  for (std::size_t i = 0; i < summary.arms.size(); ++i) {
    const auto& arm = summary.arms[i];
    json << "    {\"sign\": " << arm.sign
         << ", \"initialized\": " << arm.initialized
         << ", \"complete\": " << arm.complete
         << ", \"algebraic_pass\": " << arm.algebraic_pass
         << ", \"recurrent\": " << arm.recurrent
         << ", \"persistent\": " << arm.persistent
         << ", \"one_time_relaxation\": " << arm.one_time_relaxation
         << ", \"requested_ticks\": " << arm.requested_ticks
         << ", \"forward_ticks\": " << arm.forward_ticks
         << ", \"reverse_ticks\": " << arm.reverse_ticks
         << ", \"anchor_changes\": " << arm.anchor_changes
         << ", \"maximum_anchor_multiplicity\": "
         << arm.maximum_multiplicity
         << ", \"amplitude\": " << json_number(arm.amplitude)
         << ", \"initial_internal_momentum_norm\": "
         << json_number(arm.initial_internal_momentum_norm)
         << ", \"final_internal_momentum_norm\": "
         << json_number(arm.final_internal_momentum_norm)
         << ", \"minimum_phase_distance_after_32\": "
         << json_number(arm.minimum_phase_distance_after_32)
         << ", \"minimum_phase_tick\": " << arm.minimum_phase_tick
         << ", \"return_count\": " << arm.returns.size()
         << ", \"period_relative_residual\": "
         << json_number(arm.period_relative_residual)
         << ", \"minimum_window_axial_displacement\": "
         << json_number(arm.minimum_window_axial_displacement)
         << ", \"final_window_axial_displacement\": "
         << json_number(arm.final_window_axial_displacement)
         << ", \"maximum_transverse\": "
         << json_number(arm.maximum_transverse)
         << ", \"worst_common_gate\": " << json_number(arm.worst_gate)
         << ", \"maximum_energy_drift\": "
         << json_number(arm.maximum_energy_drift)
         << ", \"minimum_internal_distance\": "
         << json_number(arm.minimum_internal_distance)
         << ", \"maximum_internal_distance\": "
         << json_number(arm.maximum_internal_distance)
         << ", \"reverse_recovery\": " << json_number(arm.recovery) << "}"
         << (i + 1 == summary.arms.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";

  std::ofstream arms(dir / "ftd_0620_balanced_gait_phase_arms_v1.csv");
  arms << "ftd_id,sign,initialized,complete,algebraic,recurrent,persistent,"
          "one_time,requested,forward,reverse,anchor_changes,max_multiplicity,"
          "amplitude,initial_internal_p,final_internal_p,min_phase_distance,"
          "min_phase_tick,return_count,period_residual,min_window_dz,"
          "final_window_dz,max_transverse,worst_gate,energy_drift,min_internal,"
          "max_internal,recovery\n";
  for (const auto& arm : summary.arms)
    arms << std::setprecision(17) << "FTD-0620," << arm.sign << ','
         << arm.initialized << ',' << arm.complete << ',' << arm.algebraic_pass
         << ',' << arm.recurrent << ',' << arm.persistent << ','
         << arm.one_time_relaxation << ',' << arm.requested_ticks << ','
         << arm.forward_ticks << ',' << arm.reverse_ticks << ','
         << arm.anchor_changes << ',' << arm.maximum_multiplicity << ','
         << arm.amplitude << ',' << arm.initial_internal_momentum_norm << ','
         << arm.final_internal_momentum_norm << ','
         << arm.minimum_phase_distance_after_32 << ',' << arm.minimum_phase_tick
         << ',' << arm.returns.size() << ',' << arm.period_relative_residual
         << ',' << arm.minimum_window_axial_displacement << ','
         << arm.final_window_axial_displacement << ',' << arm.maximum_transverse
         << ',' << arm.worst_gate << ',' << arm.maximum_energy_drift << ','
         << arm.minimum_internal_distance << ',' << arm.maximum_internal_distance
         << ',' << arm.recovery << '\n';

  std::ofstream ticks(dir / "ftd_0620_balanced_gait_phase_ticks_v1.csv");
  ticks << "ftd_id,sign,tick,dx,dy,dz,position_return,momentum_return,"
           "phase_distance,internal_momentum_norm,shape_mode0,shape_mode1,"
           "momentum_mode0,momentum_mode1,phase_angle,gate,energy_drift\n";
  for (const auto& arm : summary.arms)
    for (const auto& sample : arm.samples)
      ticks << std::setprecision(17) << "FTD-0620," << arm.sign << ','
            << sample.tick << ',' << sample.displacement.x << ','
            << sample.displacement.y << ',' << sample.displacement.z << ','
            << sample.position_return << ',' << sample.momentum_return << ','
            << sample.phase_distance << ',' << sample.internal_momentum_norm
            << ',' << sample.shape_mode0 << ',' << sample.shape_mode1 << ','
            << sample.momentum_mode0 << ',' << sample.momentum_mode1 << ','
            << sample.phase_angle << ',' << sample.gate << ','
            << sample.energy_drift << '\n';

  std::ofstream returns(dir / "ftd_0620_balanced_gait_phase_returns_v1.csv");
  returns << "ftd_id,sign,index,tick,phase_distance,position_return,"
             "momentum_return,displacement_z,phase_angle\n";
  for (const auto& arm : summary.arms)
    for (std::size_t i = 0; i < arm.returns.size(); ++i) {
      const auto& event = arm.returns[i];
      returns << std::setprecision(17) << "FTD-0620," << arm.sign << ',' << i
              << ',' << event.tick << ',' << event.phase_distance << ','
              << event.position_return << ',' << event.momentum_return << ','
              << event.displacement_z << ',' << event.phase_angle << '\n';
    }
}

bool result_fingerprint_0620(
    const std::filesystem::path& path, const std::string& id,
    const std::string& verdict) {
  std::ifstream stream(path, std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(stream)),
                          std::istreambuf_iterator<char>());
  return bytes.find("\"ftd_id\": \"" + id + "\"") != std::string::npos
      && bytes.find(verdict) != std::string::npos;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  PhaseSummary0620 summary;
  const auto results = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results";
  summary.parent_0618 = result_fingerprint_0620(results / "ftd_0618"
      / "ftd_0618_closed_symmetry_balanced_gait_v1.json", "FTD-0618",
      "SYMMETRY_BALANCED_GAIT_KINEMATIC_MOMENTUM_OPEN");
  summary.parent_0619 = result_fingerprint_0620(results / "ftd_0619"
      / "ftd_0619_spline_poynting_noether_defect_v1.json", "FTD-0619",
      "CONTINUOUS_TRANSLATION_DEFECT_MEASURED");

  ftd::eft::ClosedNeutralPairOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-13;
  options.max_iterations = 64;
  options.allow_shared_anchor_chart = true;
  const BalancedRestContext rest = summary.parent_0618 && summary.parent_0619
      ? make_balanced_rest_context_0618(options) : BalancedRestContext{};
  if (rest.valid)
    for (int sign : {0, +1, -1})
      summary.arms.push_back(run_phase_arm_0620(sign, rest, options));
  evaluate_phase_summary_0620(summary);
  write_phase_record_0620(summary);

  std::cout << "protocol_sha256=" << phase_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "algebraic=" << summary.algebraic
            << " rest=" << summary.rest_pass
            << " mirror=" << summary.maximum_sign_mirror_residual << '\n';
  for (const auto& arm : summary.arms)
    std::cout << "sign=" << arm.sign
              << " ticks=" << arm.forward_ticks << '/' << arm.reverse_ticks
              << " returns=" << arm.returns.size()
              << " minD=" << arm.minimum_phase_distance_after_32
              << " persistent=" << arm.persistent
              << " one_time=" << arm.one_time_relaxation
              << " window=" << arm.minimum_window_axial_displacement
              << " recovery=" << arm.recovery << '\n';
  return summary.arms.size() == 3 ? 0 : 1;
}
