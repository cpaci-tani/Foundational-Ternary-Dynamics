// FTD-0617: complete angular response map of the constructive two-mode
// internal rotational gait.
#define FTD0615_NO_MAIN
#include "test_zero_momentum_internal_mode_mobility.cpp"

namespace {

constexpr char gait_protocol_sha256[] =
    "3BBD327679EB34D2F4196D897EEEF3040E6A90899C489589612D707B833E1065";
constexpr char gait_parent_sha256[] =
    "9EB7E10D912FE290795BB78E150744EC508C360F50E3BC209AF20091156A6B40";
constexpr int gait_ticks = 256;
constexpr int gait_angles = 8;

struct GaitTick {
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

double gait_internal_q(const ChargedTrimerState& state,
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

double gait_internal_p(const ChargedTrimerState& state,
                       const InternalPattern& pattern) {
  double result = 0.0;
  for (std::size_t a = 0; a < state.constituents.size(); ++a)
    result += state.constituents[a].momentum.dot(pattern[a]);
  return result;
}

InternalPattern angular_pattern_0617(const InternalBasis& basis, int angle) {
  const double theta = (2.0 * ftd::PI * angle) / gait_angles;
  InternalPattern result{};
  for (std::size_t a = 0; a < result.size(); ++a)
    result[a] = basis.patterns[0][a] * std::cos(theta)
        + basis.patterns[1][a] * std::sin(theta);
  normalize_pattern_0615(result);
  return result;
}

struct GaitArm {
  bool main = false;
  bool excitation_valid = false;
  bool complete = false;
  bool base_pass = false;
  bool intact = false;
  bool covariance_pass = false;
  int angle = 0;
  int rotation = 0;
  int forward_ticks = 0;
  int reverse_ticks = 0;
  int anchor_changes = 0;
  int maximum_multiplicity = 1;
  double theta = 0.0;
  double amplitude = INFINITY;
  double excitation_residual = INFINITY;
  double initial_momentum_residual = INFINITY;
  Vec3 displacement{};
  double minimum_pair_distance = INFINITY;
  double maximum_pair_distance = 0.0;
  double worst_gate = INFINITY;
  double energy_drift = INFINITY;
  double maximum_pseudomomentum_defect = INFINITY;
  double recovery = INFINITY;
  double covariance_center_residual = INFINITY;
  double covariance_state_residual = INFINITY;
  std::vector<GaitTick> samples;
  ChargedTrimerState final_state{};

  GaitArm() : final_state(L) {}
};

GaitArm run_gait_arm_0617(
    int angle, int rotation, bool main, const InternalPattern& base_pattern,
    const ChargedTrimerState& rest, const std::vector<double>& uniform,
    const ftd::eft::ChargedTrimerOptions& options) {
  GaitArm arm;
  arm.main = main;
  arm.angle = angle;
  arm.rotation = rotation;
  arm.theta = (2.0 * ftd::PI * angle) / gait_angles;
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
    initial.constituents[a].momentum = pattern[a] * excitation.amplitude;
  arm.initial_momentum_residual = single_momentum(initial).mag();
  const Vec3 center0 = single_center(initial);
  ChargedTrimerState current = initial;
  arm.samples.reserve(gait_ticks + 1);
  arm.samples.push_back({0, center0, {}, single_momentum(initial), 0.0,
      gait_internal_p(initial, pattern), 0.0, 0.0, 0.0});
  arm.worst_gate = 0.0;
  arm.energy_drift = 0.0;
  arm.maximum_pseudomomentum_defect = 0.0;
  double baseline = NAN;

  for (int tick = 0; tick < gait_ticks; ++tick) {
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
        single_momentum(current), gait_internal_q(current, initial, pattern),
        gait_internal_p(current, pattern), single_maximum_gate(step),
        arm.energy_drift, step.pseudomomentum_defect_norm});
  }

  if (arm.forward_ticks == gait_ticks) {
    arm.final_state = current;
    arm.displacement = single_center(current) - center0;
    for (int tick = 0; tick < gait_ticks; ++tick) {
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
  arm.complete = arm.forward_ticks == gait_ticks
      && arm.reverse_ticks == gait_ticks;
  if (arm.complete)
    arm.recovery = ftd::eft::charged_trimer_state_max_difference(
        initial, current);
  arm.base_pass = arm.complete && arm.excitation_valid
      && arm.initial_momentum_residual <= 1e-12
      && arm.worst_gate <= 1e-12 && arm.energy_drift <= 1e-10
      && arm.recovery <= 1e-8 && arm.maximum_multiplicity <= 2;
  arm.intact = arm.base_pass && arm.minimum_pair_distance >= 0.5
      && arm.maximum_pair_distance <= 2.0;
  return arm;
}

struct GaitSummary {
  bool parent_fingerprint = false;
  bool rest_fingerprint = false;
  bool rest_gate = false;
  bool basis_coverage = false;
  bool arm_coverage = false;
  bool covariance = false;
  bool dft_pass = false;
  double maximum_covariance_residual = INFINITY;
  double maximum_dft_residual = INFINITY;
  double even_rms = INFINITY;
  double odd_rms = INFINITY;
  std::array<Vec3, 5> cosine_coefficients{};
  std::array<Vec3, 5> sine_coefficients{};
  RefineResult refined{};
  SingleArm rest{};
  InternalBasis basis{};
  std::vector<GaitArm> arms;
  std::string verdict;
};

void evaluate_gait_response_0617(GaitSummary& summary) {
  std::array<Vec3, gait_angles> displacement{};
  bool coverage = true;
  for (int angle = 0; angle < gait_angles; ++angle) {
    const auto arm = std::find_if(summary.arms.begin(), summary.arms.end(),
        [&](const GaitArm& candidate) {
          return candidate.main && candidate.angle == angle
              && candidate.rotation == 0;
        });
    if (arm == summary.arms.end()) { coverage = false; continue; }
    displacement[static_cast<std::size_t>(angle)] = arm->displacement;
  }
  for (int harmonic = 0; harmonic <= 4; ++harmonic) {
    Vec3 cosine{}, sine{};
    for (int angle = 0; angle < gait_angles; ++angle) {
      const double theta = (2.0 * ftd::PI * angle) / gait_angles;
      cosine += displacement[static_cast<std::size_t>(angle)]
          * std::cos(harmonic * theta);
      sine += displacement[static_cast<std::size_t>(angle)]
          * std::sin(harmonic * theta);
    }
    summary.cosine_coefficients[static_cast<std::size_t>(harmonic)] =
        cosine * (1.0 / gait_angles);
    summary.sine_coefficients[static_cast<std::size_t>(harmonic)] =
        sine * (1.0 / gait_angles);
  }
  summary.maximum_dft_residual = 0.0;
  for (int angle = 0; angle < gait_angles; ++angle) {
    const double theta = (2.0 * ftd::PI * angle) / gait_angles;
    Vec3 reconstructed = summary.cosine_coefficients[0]
        + summary.cosine_coefficients[4] * std::cos(4.0 * theta);
    for (int harmonic = 1; harmonic <= 3; ++harmonic)
      reconstructed +=
          (summary.cosine_coefficients[static_cast<std::size_t>(harmonic)]
               * std::cos(harmonic * theta)
           + summary.sine_coefficients[static_cast<std::size_t>(harmonic)]
               * std::sin(harmonic * theta)) * 2.0;
    summary.maximum_dft_residual = std::max(
        summary.maximum_dft_residual,
        (reconstructed - displacement[static_cast<std::size_t>(angle)]).mag());
  }
  double even_squared = 0.0, odd_squared = 0.0;
  for (int angle = 0; angle < 4; ++angle) {
    const Vec3 even =
        (displacement[static_cast<std::size_t>(angle)]
         + displacement[static_cast<std::size_t>(angle + 4)]) * 0.5;
    const Vec3 odd =
        (displacement[static_cast<std::size_t>(angle)]
         - displacement[static_cast<std::size_t>(angle + 4)]) * 0.5;
    even_squared += even.mag2();
    odd_squared += odd.mag2();
  }
  summary.even_rms = std::sqrt(even_squared / 4.0);
  summary.odd_rms = std::sqrt(odd_squared / 4.0);
  summary.dft_pass = coverage && summary.maximum_dft_residual <= 1e-12;
}

void evaluate_gait_covariance_0617(GaitSummary& summary) {
  summary.maximum_covariance_residual = 0.0;
  bool coverage = true;
  for (auto& arm : summary.arms) {
    if (arm.rotation == 0) {
      arm.covariance_center_residual = 0.0;
      arm.covariance_state_residual = 0.0;
      arm.covariance_pass = true;
      continue;
    }
    const auto base = std::find_if(summary.arms.begin(), summary.arms.end(),
        [&](const GaitArm& candidate) {
          return candidate.main && candidate.angle == arm.angle
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
          [](const GaitArm& arm) { return arm.covariance_pass; });
}

void write_gait_record_0617(const GaitSummary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0617";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0617_internal_gait_angular_response_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0617\",\n"
       << "  \"protocol_sha256\": \"" << gait_protocol_sha256 << "\",\n"
       << "  \"parent_result_sha256\": \"" << gait_parent_sha256 << "\",\n"
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
       << "  \"dft_pass\": " << (summary.dft_pass ? "true" : "false")
       << ",\n  \"maximum_covariance_residual\": "
       << json_number(summary.maximum_covariance_residual)
       << ",\n  \"maximum_dft_residual\": "
       << json_number(summary.maximum_dft_residual)
       << ",\n  \"even_rms\": " << json_number(summary.even_rms)
       << ",\n  \"odd_rms\": " << json_number(summary.odd_rms)
       << ",\n  \"fourier\": [\n";
  for (int harmonic = 0; harmonic <= 4; ++harmonic) {
    const auto& c = summary.cosine_coefficients[static_cast<std::size_t>(harmonic)];
    const auto& s = summary.sine_coefficients[static_cast<std::size_t>(harmonic)];
    json << "    {\"harmonic\": " << harmonic << ", \"cosine\": ["
         << c.x << ',' << c.y << ',' << c.z << "], \"sine\": ["
         << s.x << ',' << s.y << ',' << s.z << "]}"
         << (harmonic == 4 ? "\n" : ",\n");
  }
  json << "  ],\n  \"arms\": [\n";
  for (std::size_t i = 0; i < summary.arms.size(); ++i) {
    const auto& arm = summary.arms[i];
    json << "    {\"angle\": " << arm.angle
         << ", \"rotation\": " << arm.rotation
         << ", \"main\": " << (arm.main ? "true" : "false")
         << ", \"complete\": " << (arm.complete ? "true" : "false")
         << ", \"base_pass\": " << (arm.base_pass ? "true" : "false")
         << ", \"intact\": " << (arm.intact ? "true" : "false")
         << ", \"covariance_pass\": "
         << (arm.covariance_pass ? "true" : "false")
         << ", \"forward_ticks\": " << arm.forward_ticks
         << ", \"reverse_ticks\": " << arm.reverse_ticks
         << ", \"anchor_changes\": " << arm.anchor_changes
         << ", \"maximum_anchor_multiplicity\": " << arm.maximum_multiplicity
         << ", \"theta\": " << arm.theta
         << ", \"amplitude\": " << json_number(arm.amplitude)
         << ", \"excitation_residual\": "
         << json_number(arm.excitation_residual)
         << ", \"initial_momentum_residual\": "
         << json_number(arm.initial_momentum_residual)
         << ", \"displacement\": [" << arm.displacement.x << ','
         << arm.displacement.y << ',' << arm.displacement.z << ']'
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

  std::ofstream arms(dir / "ftd_0617_internal_gait_arms_v1.csv");
  arms << "ftd_id,angle,rotation,main,complete,base_pass,intact,"
          "covariance_pass,forward_ticks,reverse_ticks,anchor_changes,"
          "max_multiplicity,theta,amplitude,excitation_residual,"
          "initial_momentum,dx,dy,dz,min_distance,max_distance,worst_gate,"
          "energy_drift,pseudomomentum_defect,recovery,covariance_center,"
          "covariance_state\n";
  for (const auto& arm : summary.arms)
    arms << std::setprecision(17) << "FTD-0617," << arm.angle << ','
         << arm.rotation << ',' << arm.main << ',' << arm.complete << ','
         << arm.base_pass << ',' << arm.intact << ',' << arm.covariance_pass
         << ',' << arm.forward_ticks << ',' << arm.reverse_ticks << ','
         << arm.anchor_changes << ',' << arm.maximum_multiplicity << ','
         << arm.theta << ',' << arm.amplitude << ',' << arm.excitation_residual
         << ',' << arm.initial_momentum_residual << ',' << arm.displacement.x
         << ',' << arm.displacement.y << ',' << arm.displacement.z << ','
         << arm.minimum_pair_distance << ',' << arm.maximum_pair_distance << ','
         << arm.worst_gate << ',' << arm.energy_drift << ','
         << arm.maximum_pseudomomentum_defect << ',' << arm.recovery << ','
         << arm.covariance_center_residual << ','
         << arm.covariance_state_residual << '\n';

  std::ofstream ticks(dir / "ftd_0617_internal_gait_ticks_v1.csv");
  ticks << "ftd_id,angle,rotation,main,tick,cx,cy,cz,dx,dy,dz,px,py,pz,"
           "internal_q,internal_p,gate,energy_drift,pseudomomentum_defect\n";
  for (const auto& arm : summary.arms)
    for (const auto& sample : arm.samples)
      ticks << std::setprecision(17) << "FTD-0617," << arm.angle << ','
            << arm.rotation << ',' << arm.main << ',' << sample.tick << ','
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

#ifndef FTD0617_NO_MAIN
int main() {
  std::cout << std::setprecision(17);
  GaitSummary summary;
  const auto parent_path = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results" / "ftd_0616"
      / "ftd_0616_internal_walker_direction_persistence_v1.json";
  std::ifstream parent(parent_path, std::ios::binary);
  std::string parent_bytes((std::istreambuf_iterator<char>(parent)),
                           std::istreambuf_iterator<char>());
  summary.parent_fingerprint =
      parent_bytes.find("\"ftd_id\": \"FTD-0616\"") != std::string::npos
      && parent_bytes.find("\"direction_control_pass\": false")
          != std::string::npos
      && parent_bytes.find("INTERNAL_WALKER_TRANSIENT_OR_UNCONTROLLED")
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
      fixture.name = "gait_rest";
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
  if (summary.basis_coverage) {
    for (int angle = 0; angle < gait_angles; ++angle) {
      const auto pattern = angular_pattern_0617(summary.basis, angle);
      summary.arms.push_back(run_gait_arm_0617(
          angle, 0, true, pattern, rest_state, uniform, options));
    }
    for (int angle : {0, 2}) {
      const auto pattern = angular_pattern_0617(summary.basis, angle);
      for (int rotation : {1, 2})
        summary.arms.push_back(run_gait_arm_0617(
            angle, rotation, false, pattern, rest_state, uniform, options));
    }
  }
  summary.arm_coverage = summary.arms.size() == 12
      && std::all_of(summary.arms.begin(), summary.arms.end(),
          [](const GaitArm& arm) { return arm.base_pass && arm.intact; });
  evaluate_gait_covariance_0617(summary);
  evaluate_gait_response_0617(summary);
  const bool coverage = summary.parent_fingerprint && summary.rest_fingerprint
      && summary.rest_gate && summary.basis_coverage && summary.arm_coverage
      && summary.covariance && summary.dft_pass;
  if (!coverage)
    summary.verdict = "INTERNAL_GAIT_ANGULAR_RESPONSE_NUMERICALLY_UNRESOLVED";
  else if (summary.even_rms > 0.25 && summary.odd_rms > 0.25)
    summary.verdict = "MIXED_PARITY_INTERNAL_GAIT_RESPONSE_RESOLVED";
  else
    summary.verdict = "SINGLE_PARITY_INTERNAL_RESPONSE_RESOLVED";
  write_gait_record_0617(summary);

  std::cout << "protocol_sha256=" << gait_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "arms=" << summary.arms.size()
            << " covariance=" << summary.maximum_covariance_residual
            << " dft=" << summary.maximum_dft_residual
            << " even_rms=" << summary.even_rms
            << " odd_rms=" << summary.odd_rms << '\n';
  for (const auto& arm : summary.arms)
    std::cout << "angle=" << arm.angle << " rotation=" << arm.rotation
              << " main=" << arm.main
              << " d=(" << arm.displacement.x << ',' << arm.displacement.y
              << ',' << arm.displacement.z << ')'
              << " recovery=" << arm.recovery
              << " pass=" << arm.base_pass << '\n';
  return summary.arms.size() == 12 ? 0 : 1;
}
#endif
