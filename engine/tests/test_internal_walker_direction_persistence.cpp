// FTD-0616: signed-vector and long-time persistence discriminator for the
// constructive FTD-0615 zero-centre-momentum internal walker.
#define FTD0615_NO_MAIN
#include "test_zero_momentum_internal_mode_mobility.cpp"

namespace {

constexpr char walker_protocol_sha256[] =
    "E55D5CFA92EB719569B2A8F6D4F19EDB9C90DE49BA4C2B1721AC06F0B0AA730B";
constexpr char walker_parent_sha256[] =
    "8B7DD5809DE70B5EEA3C398C4A58AE2B0F64EFD6FD0BF653FBA4F4F0569ABA2C";
constexpr int walker_ticks = 512;
constexpr int walker_window = 128;

double cosine_0616(const Vec3& lhs, const Vec3& rhs) {
  const double denominator = lhs.mag() * rhs.mag();
  return denominator > 0.0 ? lhs.dot(rhs) / denominator : -INFINITY;
}

struct WalkerTick {
  int tick = 0;
  Vec3 center{};
  Vec3 displacement{};
  Vec3 center_momentum{};
  double internal_q = 0.0;
  double internal_p = 0.0;
  double gate = 0.0;
  double energy_drift = 0.0;
  double pseudomomentum_defect = 0.0;
};

double internal_q_0616(const ChargedTrimerState& state,
                       const ChargedTrimerState& reference,
                       const InternalPattern& pattern) {
  const Vec3 center = single_center(state);
  const Vec3 reference_center = single_center(reference);
  double result = 0.0;
  for (std::size_t a = 0; a < state.constituents.size(); ++a) {
    const Vec3 relative = effective_position(state.constituents[a]) - center;
    const Vec3 reference_relative =
        effective_position(reference.constituents[a]) - reference_center;
    result += (relative - reference_relative).dot(pattern[a]);
  }
  return result;
}

double internal_p_0616(const ChargedTrimerState& state,
                       const InternalPattern& pattern) {
  double result = 0.0;
  for (std::size_t a = 0; a < state.constituents.size(); ++a)
    result += state.constituents[a].momentum.dot(pattern[a]);
  return result;
}

struct WalkerArm {
  bool excitation_valid = false;
  bool complete = false;
  bool base_pass = false;
  bool intact = false;
  bool persistent = false;
  bool covariance_pass = false;
  int mode = 0;
  int sign = 1;
  int rotation = 0;
  int forward_ticks = 0;
  int reverse_ticks = 0;
  int anchor_changes = 0;
  int maximum_multiplicity = 1;
  double amplitude = INFINITY;
  double excitation_residual = INFINITY;
  double initial_momentum_residual = INFINITY;
  Vec3 displacement{};
  std::array<Vec3, 4> window_displacements{};
  double minimum_window_displacement = INFINITY;
  double minimum_successive_cosine = INFINITY;
  double window_speed_cv = INFINITY;
  double minimum_pair_distance = INFINITY;
  double maximum_pair_distance = 0.0;
  double worst_gate = INFINITY;
  double energy_drift = INFINITY;
  double maximum_pseudomomentum_defect = INFINITY;
  double recovery = INFINITY;
  double covariance_center_residual = INFINITY;
  double covariance_state_residual = INFINITY;
  std::vector<WalkerTick> samples;
  ChargedTrimerState final_state{};

  WalkerArm() : final_state(L) {}
};

WalkerArm run_walker_arm_0616(
    int mode, int sign, int rotation, const InternalPattern& base_pattern,
    const ChargedTrimerState& rest, const std::vector<double>& uniform,
    const ftd::eft::ChargedTrimerOptions& options) {
  WalkerArm arm;
  arm.mode = mode;
  arm.sign = sign;
  arm.rotation = rotation;
  InternalPattern pattern{};
  for (std::size_t a = 0; a < pattern.size(); ++a)
    pattern[a] = cycle_vec_0614(base_pattern[a], rotation);
  const auto excitation = solve_excitation_0615(
      pattern, 4.0 * internal_delta_ref);
  arm.amplitude = excitation.amplitude;
  arm.excitation_residual = excitation.residual;
  arm.excitation_valid = excitation.valid;
  if (!excitation.valid) return arm;

  ChargedTrimerState initial = cycle_state_0614(rest, rotation);
  for (std::size_t a = 0; a < initial.constituents.size(); ++a)
    initial.constituents[a].momentum =
        pattern[a] * (sign * excitation.amplitude);
  arm.initial_momentum_residual = single_momentum(initial).mag();
  const Vec3 center0 = single_center(initial);
  ChargedTrimerState current = initial;
  arm.samples.reserve(walker_ticks + 1);
  arm.samples.push_back({0, center0, {}, single_momentum(initial),
      0.0, internal_p_0616(initial, pattern), 0.0, 0.0, 0.0});
  arm.worst_gate = 0.0;
  arm.energy_drift = 0.0;
  arm.maximum_pseudomomentum_defect = 0.0;
  double baseline = NAN;

  for (int tick = 0; tick < walker_ticks; ++tick) {
    const auto step = ftd::eft::solve_charged_trimer_forward(
        current, uniform, options);
    if (!step.valid) break;
    if (!std::isfinite(baseline)) baseline = single_energy_before(step);
    ++arm.forward_ticks;
    arm.anchor_changes += single_anchor_changes(current, step.later);
    arm.maximum_multiplicity = std::max(
        arm.maximum_multiplicity, maximum_anchor_multiplicity(step.later));
    arm.worst_gate = std::max(arm.worst_gate, single_maximum_gate(step));
    arm.energy_drift = std::max(arm.energy_drift,
        std::max(std::abs(single_energy_before(step) - baseline),
                 std::abs(single_energy_after(step) - baseline)));
    arm.minimum_pair_distance = std::min(
        arm.minimum_pair_distance, step.minimum_pair_distance);
    arm.maximum_pair_distance = std::max(
        arm.maximum_pair_distance, step.maximum_pair_distance);
    arm.maximum_pseudomomentum_defect = std::max(
        arm.maximum_pseudomomentum_defect, step.pseudomomentum_defect_norm);
    current = step.later;
    const Vec3 center = single_center(current);
    arm.samples.push_back({tick + 1, center, center - center0,
        single_momentum(current), internal_q_0616(current, initial, pattern),
        internal_p_0616(current, pattern), single_maximum_gate(step),
        arm.energy_drift, step.pseudomomentum_defect_norm});
  }

  if (arm.forward_ticks == walker_ticks) {
    arm.final_state = current;
    arm.displacement = single_center(current) - center0;
    for (int window = 0; window < 4; ++window) {
      const int begin = window * walker_window;
      const int end = begin + walker_window;
      arm.window_displacements[static_cast<std::size_t>(window)] =
          arm.samples[static_cast<std::size_t>(end)].center
          - arm.samples[static_cast<std::size_t>(begin)].center;
    }
    arm.minimum_window_displacement = INFINITY;
    std::array<double, 4> speeds{};
    double speed_mean = 0.0;
    for (int window = 0; window < 4; ++window) {
      speeds[static_cast<std::size_t>(window)] =
          arm.window_displacements[static_cast<std::size_t>(window)].mag()
          / walker_window;
      speed_mean += speeds[static_cast<std::size_t>(window)];
      arm.minimum_window_displacement = std::min(
          arm.minimum_window_displacement,
          arm.window_displacements[static_cast<std::size_t>(window)].mag());
    }
    speed_mean /= 4.0;
    double speed_variance = 0.0;
    for (double speed : speeds)
      speed_variance += (speed - speed_mean) * (speed - speed_mean);
    speed_variance /= 4.0;
    arm.window_speed_cv = speed_mean > 0.0
        ? std::sqrt(speed_variance) / speed_mean : INFINITY;
    arm.minimum_successive_cosine = 1.0;
    for (int window = 0; window < 3; ++window)
      arm.minimum_successive_cosine = std::min(
          arm.minimum_successive_cosine,
          cosine_0616(arm.window_displacements[static_cast<std::size_t>(window)],
                      arm.window_displacements[static_cast<std::size_t>(window + 1)]));

    for (int tick = 0; tick < walker_ticks; ++tick) {
      const auto step = ftd::eft::solve_charged_trimer_reverse(
          current, uniform, options);
      if (!step.valid) break;
      ++arm.reverse_ticks;
      arm.maximum_multiplicity = std::max(
          arm.maximum_multiplicity, maximum_anchor_multiplicity(step.earlier));
      arm.worst_gate = std::max(arm.worst_gate, single_maximum_gate(step));
      arm.energy_drift = std::max(arm.energy_drift,
          std::max(std::abs(single_energy_before(step) - baseline),
                   std::abs(single_energy_after(step) - baseline)));
      arm.minimum_pair_distance = std::min(
          arm.minimum_pair_distance, step.minimum_pair_distance);
      arm.maximum_pair_distance = std::max(
          arm.maximum_pair_distance, step.maximum_pair_distance);
      arm.maximum_pseudomomentum_defect = std::max(
          arm.maximum_pseudomomentum_defect, step.pseudomomentum_defect_norm);
      current = step.earlier;
    }
  }

  arm.complete = arm.forward_ticks == walker_ticks
      && arm.reverse_ticks == walker_ticks;
  if (arm.complete)
    arm.recovery = ftd::eft::charged_trimer_state_max_difference(
        initial, current);
  arm.base_pass = arm.complete && arm.excitation_valid
      && arm.initial_momentum_residual <= 1e-12
      && arm.worst_gate <= 1e-12 && arm.energy_drift <= 1e-10
      && arm.recovery <= 1e-8 && arm.maximum_multiplicity <= 2;
  arm.intact = arm.base_pass && arm.minimum_pair_distance >= 0.5
      && arm.maximum_pair_distance <= 2.0;
  arm.persistent = arm.intact && arm.minimum_window_displacement >= 0.5
      && arm.minimum_successive_cosine >= 0.95
      && arm.window_speed_cv <= 0.25;
  return arm;
}

struct WalkerSummary {
  bool parent_fingerprint = false;
  bool rest_fingerprint = false;
  bool rest_gate = false;
  bool basis_coverage = false;
  bool arm_coverage = false;
  bool covariance = false;
  bool direction_control = false;
  bool persistence = false;
  double maximum_covariance_residual = INFINITY;
  double maximum_sign_antipode_cosine = INFINITY;
  double maximum_sign_magnitude_mismatch = INFINITY;
  RefineResult refined{};
  SingleArm rest{};
  InternalBasis basis{};
  std::vector<WalkerArm> arms;
  std::string verdict;
};

void apply_covariance_0616(WalkerSummary& summary) {
  summary.maximum_covariance_residual = 0.0;
  bool coverage = summary.arms.size() == 12;
  for (auto& arm : summary.arms) {
    if (arm.rotation == 0) {
      arm.covariance_center_residual = 0.0;
      arm.covariance_state_residual = 0.0;
      arm.covariance_pass = true;
      continue;
    }
    const auto base = std::find_if(summary.arms.begin(), summary.arms.end(),
        [&](const WalkerArm& candidate) {
          return candidate.mode == arm.mode && candidate.sign == arm.sign
              && candidate.rotation == 0;
        });
    if (base == summary.arms.end()
        || base->samples.size() != arm.samples.size()) {
      coverage = false;
      continue;
    }
    arm.covariance_center_residual = 0.0;
    for (std::size_t tick = 0; tick < arm.samples.size(); ++tick)
      arm.covariance_center_residual = std::max(
          arm.covariance_center_residual,
          (cycle_vec_0614(base->samples[tick].center, arm.rotation)
           - arm.samples[tick].center).mag());
    arm.covariance_state_residual =
        ftd::eft::charged_trimer_state_max_difference(
            cycle_state_0614(base->final_state, arm.rotation), arm.final_state);
    arm.covariance_pass = arm.covariance_center_residual <= 1e-8
        && arm.covariance_state_residual <= 1e-8;
    summary.maximum_covariance_residual = std::max(
        summary.maximum_covariance_residual,
        std::max(arm.covariance_center_residual,
                 arm.covariance_state_residual));
  }
  summary.covariance = coverage
      && std::all_of(summary.arms.begin(), summary.arms.end(),
          [](const WalkerArm& arm) { return arm.covariance_pass; });
}

void apply_direction_0616(WalkerSummary& summary) {
  summary.maximum_sign_antipode_cosine = -INFINITY;
  summary.maximum_sign_magnitude_mismatch = 0.0;
  bool pass = true;
  for (int mode : {0, 1}) {
    const auto positive = std::find_if(summary.arms.begin(), summary.arms.end(),
        [&](const WalkerArm& arm) {
          return arm.mode == mode && arm.sign == 1 && arm.rotation == 0;
        });
    const auto negative = std::find_if(summary.arms.begin(), summary.arms.end(),
        [&](const WalkerArm& arm) {
          return arm.mode == mode && arm.sign == -1 && arm.rotation == 0;
        });
    if (positive == summary.arms.end() || negative == summary.arms.end()) {
      pass = false;
      continue;
    }
    const double positive_magnitude = positive->displacement.mag();
    const double negative_magnitude = negative->displacement.mag();
    const double antipode = cosine_0616(
        positive->displacement, negative->displacement);
    const double mismatch = negative_magnitude > 0.0
        ? std::abs(positive_magnitude / negative_magnitude - 1.0) : INFINITY;
    summary.maximum_sign_antipode_cosine = std::max(
        summary.maximum_sign_antipode_cosine, antipode);
    summary.maximum_sign_magnitude_mismatch = std::max(
        summary.maximum_sign_magnitude_mismatch, mismatch);
    pass = pass && positive_magnitude >= 2.0 && negative_magnitude >= 2.0
        && antipode <= -0.99 && mismatch <= 0.05;
  }
  summary.direction_control = pass;
}

void write_walker_record_0616(const WalkerSummary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0616";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0616_internal_walker_direction_persistence_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0616\",\n"
       << "  \"protocol_sha256\": \"" << walker_protocol_sha256 << "\",\n"
       << "  \"parent_result_sha256\": \"" << walker_parent_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"parent_record_fingerprint_pass\": "
       << (summary.parent_fingerprint ? "true" : "false") << ",\n"
       << "  \"rest_fingerprint_pass\": "
       << (summary.rest_fingerprint ? "true" : "false") << ",\n"
       << "  \"rest_gate_pass\": "
       << (summary.rest_gate ? "true" : "false") << ",\n"
       << "  \"basis_coverage\": "
       << (summary.basis_coverage ? "true" : "false") << ",\n"
       << "  \"arm_coverage\": "
       << (summary.arm_coverage ? "true" : "false") << ",\n"
       << "  \"covariance_pass\": "
       << (summary.covariance ? "true" : "false") << ",\n"
       << "  \"direction_control_pass\": "
       << (summary.direction_control ? "true" : "false") << ",\n"
       << "  \"persistence_pass\": "
       << (summary.persistence ? "true" : "false") << ",\n"
       << "  \"maximum_covariance_residual\": "
       << json_number(summary.maximum_covariance_residual) << ",\n"
       << "  \"maximum_sign_antipode_cosine\": "
       << json_number(summary.maximum_sign_antipode_cosine) << ",\n"
       << "  \"maximum_sign_magnitude_mismatch\": "
       << json_number(summary.maximum_sign_magnitude_mismatch) << ",\n"
       << "  \"arms\": [\n";
  for (std::size_t i = 0; i < summary.arms.size(); ++i) {
    const auto& arm = summary.arms[i];
    json << "    {\"mode\": " << arm.mode << ", \"sign\": " << arm.sign
         << ", \"rotation\": " << arm.rotation
         << ", \"complete\": " << (arm.complete ? "true" : "false")
         << ", \"base_pass\": " << (arm.base_pass ? "true" : "false")
         << ", \"intact\": " << (arm.intact ? "true" : "false")
         << ", \"persistent\": " << (arm.persistent ? "true" : "false")
         << ", \"covariance_pass\": "
         << (arm.covariance_pass ? "true" : "false")
         << ", \"forward_ticks\": " << arm.forward_ticks
         << ", \"reverse_ticks\": " << arm.reverse_ticks
         << ", \"anchor_changes\": " << arm.anchor_changes
         << ", \"maximum_anchor_multiplicity\": " << arm.maximum_multiplicity
         << ", \"amplitude\": " << json_number(arm.amplitude)
         << ", \"excitation_residual\": "
         << json_number(arm.excitation_residual)
         << ", \"initial_momentum_residual\": "
         << json_number(arm.initial_momentum_residual)
         << ", \"displacement\": [" << arm.displacement.x << ','
         << arm.displacement.y << ',' << arm.displacement.z << ']'
         << ", \"minimum_window_displacement\": "
         << json_number(arm.minimum_window_displacement)
         << ", \"minimum_successive_cosine\": "
         << json_number(arm.minimum_successive_cosine)
         << ", \"window_speed_cv\": " << json_number(arm.window_speed_cv)
         << ", \"minimum_pair_distance\": "
         << json_number(arm.minimum_pair_distance)
         << ", \"maximum_pair_distance\": "
         << json_number(arm.maximum_pair_distance)
         << ", \"worst_common_gate\": " << json_number(arm.worst_gate)
         << ", \"maximum_energy_drift\": "
         << json_number(arm.energy_drift)
         << ", \"maximum_pseudomomentum_defect\": "
         << json_number(arm.maximum_pseudomomentum_defect)
         << ", \"reverse_recovery\": " << json_number(arm.recovery)
         << ", \"covariance_center_residual\": "
         << json_number(arm.covariance_center_residual)
         << ", \"covariance_state_residual\": "
         << json_number(arm.covariance_state_residual) << "}"
         << (i + 1 == summary.arms.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";

  std::ofstream arms(dir / "ftd_0616_internal_walker_arms_v1.csv");
  arms << "ftd_id,mode,sign,rotation,complete,base_pass,intact,persistent,"
          "covariance_pass,forward_ticks,reverse_ticks,anchor_changes,"
          "max_multiplicity,amplitude,excitation_residual,initial_momentum,"
          "dx,dy,dz,min_window,min_cosine,speed_cv,min_distance,max_distance,"
          "worst_gate,energy_drift,pseudomomentum_defect,recovery,"
          "covariance_center,covariance_state\n";
  for (const auto& arm : summary.arms)
    arms << std::setprecision(17) << "FTD-0616," << arm.mode << ',' << arm.sign
         << ',' << arm.rotation << ',' << arm.complete << ',' << arm.base_pass
         << ',' << arm.intact << ',' << arm.persistent << ','
         << arm.covariance_pass << ',' << arm.forward_ticks << ','
         << arm.reverse_ticks << ',' << arm.anchor_changes << ','
         << arm.maximum_multiplicity << ',' << arm.amplitude << ','
         << arm.excitation_residual << ',' << arm.initial_momentum_residual
         << ',' << arm.displacement.x << ',' << arm.displacement.y << ','
         << arm.displacement.z << ',' << arm.minimum_window_displacement << ','
         << arm.minimum_successive_cosine << ',' << arm.window_speed_cv << ','
         << arm.minimum_pair_distance << ',' << arm.maximum_pair_distance << ','
         << arm.worst_gate << ',' << arm.energy_drift << ','
         << arm.maximum_pseudomomentum_defect << ',' << arm.recovery << ','
         << arm.covariance_center_residual << ','
         << arm.covariance_state_residual << '\n';

  std::ofstream ticks(dir / "ftd_0616_internal_walker_ticks_v1.csv");
  ticks << "ftd_id,mode,sign,rotation,tick,cx,cy,cz,dx,dy,dz,px,py,pz,"
           "internal_q,internal_p,gate,energy_drift,pseudomomentum_defect\n";
  for (const auto& arm : summary.arms)
    for (const auto& sample : arm.samples)
      ticks << std::setprecision(17) << "FTD-0616," << arm.mode << ','
            << arm.sign << ',' << arm.rotation << ',' << sample.tick << ','
            << sample.center.x << ',' << sample.center.y << ','
            << sample.center.z << ',' << sample.displacement.x << ','
            << sample.displacement.y << ',' << sample.displacement.z << ','
            << sample.center_momentum.x << ',' << sample.center_momentum.y
            << ',' << sample.center_momentum.z << ',' << sample.internal_q
            << ',' << sample.internal_p << ',' << sample.gate << ','
            << sample.energy_drift << ',' << sample.pseudomomentum_defect
            << '\n';
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  WalkerSummary summary;
  const auto parent_path = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results" / "ftd_0615"
      / "ftd_0615_zero_momentum_internal_modes_v1.json";
  std::ifstream parent(parent_path, std::ios::binary);
  std::string parent_bytes((std::istreambuf_iterator<char>(parent)),
                           std::istreambuf_iterator<char>());
  summary.parent_fingerprint =
      parent_bytes.find("\"ftd_id\": \"FTD-0615\"") != std::string::npos
      && parent_bytes.find("\"walker_count\": 4") != std::string::npos
      && parent_bytes.find("ZERO_MOMENTUM_INTERNAL_WALKER_CONSTRUCTIVE")
          != std::string::npos;

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
  if (summary.parent_fingerprint && normalization.valid && green.valid
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
  summary.refined = initial.valid
      ? refine_static_state(initial, options, green, beta) : RefineResult{};
  summary.rest_fingerprint = searches.size() == 16
      && summary.refined.coverage && summary.refined.converged
      && std::abs(summary.refined.state.energy
                  - locked_landscape_rest_energy) <= 1e-15
      && summary.refined.derivatives.gradient_inf <= 1e-10
      && summary.refined.derivatives.positive_modes == static_dof;

  const std::size_t count = static_cast<std::size_t>(L) * L * L;
  std::vector<double> uniform(count, -1.0 / static_cast<double>(count));
  ChargedTrimerState rest_state = summary.refined.state.state;
  if (summary.rest_fingerprint) {
    auto total = coat_density(rest_state);
    if (total.size() == count) {
      for (std::size_t i = 0; i < count; ++i) total[i] += uniform[i];
      const auto direct = initialize_minimum_energy(total);
      if (direct.valid) rest_state.electric = direct.electric;
      NeutralizerFixture fixture;
      fixture.name = "walker_rest";
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
  if (summary.rest_gate)
    summary.basis = make_internal_basis_0615(summary.refined.state);
  summary.basis_coverage = summary.basis.valid;
  if (summary.basis_coverage)
    for (int mode : {0, 1})
      for (int sign : {-1, 1})
        for (int rotation : {0, 1, 2})
          summary.arms.push_back(run_walker_arm_0616(
              mode, sign, rotation, summary.basis.patterns[mode], rest_state,
              uniform, options));
  summary.arm_coverage = summary.arms.size() == 12
      && std::all_of(summary.arms.begin(), summary.arms.end(),
          [](const WalkerArm& arm) { return arm.base_pass && arm.intact; });
  apply_covariance_0616(summary);
  apply_direction_0616(summary);
  summary.persistence = summary.arms.size() == 12
      && std::all_of(summary.arms.begin(), summary.arms.end(),
          [](const WalkerArm& arm) { return arm.persistent; });
  const bool coverage = summary.parent_fingerprint && summary.rest_fingerprint
      && summary.rest_gate && summary.basis_coverage && summary.arm_coverage
      && summary.covariance;
  if (!coverage)
    summary.verdict =
        "INTERNAL_WALKER_DIRECTION_PERSISTENCE_NUMERICALLY_UNRESOLVED";
  else if (summary.direction_control && summary.persistence)
    summary.verdict = "INTERNAL_WALKER_DIRECTION_CONTROLLED_PERSISTENT";
  else
    summary.verdict = "INTERNAL_WALKER_TRANSIENT_OR_UNCONTROLLED";
  write_walker_record_0616(summary);

  std::cout << "protocol_sha256=" << walker_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "arms=" << summary.arms.size()
            << " coverage=" << summary.arm_coverage
            << " covariance=" << summary.covariance
            << " direction=" << summary.direction_control
            << " persistence=" << summary.persistence << '\n';
  for (const auto& arm : summary.arms)
    std::cout << "mode=" << arm.mode << " sign=" << arm.sign
              << " rotation=" << arm.rotation
              << " d=(" << arm.displacement.x << ',' << arm.displacement.y
              << ',' << arm.displacement.z << ')'
              << " min_window=" << arm.minimum_window_displacement
              << " min_cos=" << arm.minimum_successive_cosine
              << " cv=" << arm.window_speed_cv
              << " recovery=" << arm.recovery
              << " pass=" << arm.base_pass << '\n';
  return summary.arms.size() == 12 ? 0 : 1;
}
