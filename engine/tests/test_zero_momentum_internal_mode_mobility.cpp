// FTD-0615: zero-centre-momentum internal-mode mobility discriminator.
#define FTD0614_NO_MAIN
#include "test_refined_core_peierls_landscape.cpp"

namespace {

constexpr char internal_mode_protocol_sha256[] =
    "1F8B86C20FFAC79381F2DA4B69085E5DC4B360BFAC379281D3F272C87387104B";
constexpr char parent_result_sha256[] =
    "8A2866361FAECED8358DD8BB59A62F01CA583273D62235436A0600796520BA45";
constexpr double internal_delta_ref = 0.00011302707069732617;
constexpr int internal_mode_count = 6;
using InternalPattern = std::array<Vec3, 3>;
using Gram6 = std::array<std::array<double, internal_mode_count>,
                         internal_mode_count>;

Vec3 cross_0615(const Vec3& lhs, const Vec3& rhs) {
  return {lhs.y * rhs.z - lhs.z * rhs.y,
          lhs.z * rhs.x - lhs.x * rhs.z,
          lhs.x * rhs.y - lhs.y * rhs.x};
}

Vec3 apply_matrix_0615(const Matrix3& matrix, const Vec3& vector) {
  return {
      matrix[0][0] * vector.x + matrix[0][1] * vector.y
          + matrix[0][2] * vector.z,
      matrix[1][0] * vector.x + matrix[1][1] * vector.y
          + matrix[1][2] * vector.z,
      matrix[2][0] * vector.x + matrix[2][1] * vector.y
          + matrix[2][2] * vector.z};
}

double pattern_dot_0615(const InternalPattern& lhs,
                        const InternalPattern& rhs) {
  double result = 0.0;
  for (std::size_t a = 0; a < lhs.size(); ++a)
    result += lhs[a].dot(rhs[a]);
  return result;
}

Vec3 pattern_sum_0615(const InternalPattern& pattern) {
  Vec3 result{};
  for (const auto& entry : pattern) result += entry;
  return result;
}

bool normalize_pattern_0615(InternalPattern& pattern) {
  const Vec3 mean = pattern_sum_0615(pattern) * (1.0 / 3.0);
  for (auto& entry : pattern) entry -= mean;
  const double norm = std::sqrt(pattern_dot_0615(pattern, pattern));
  if (!(norm > 1e-12) || !std::isfinite(norm)) return false;
  for (auto& entry : pattern) entry *= 1.0 / norm;
  return true;
}

std::array<double, internal_mode_count> gram_eigenvalues_0615(Gram6 matrix) {
  for (int sweep = 0; sweep < 300; ++sweep) {
    int p = 0, q = 1;
    double largest = std::abs(matrix[0][1]);
    for (int i = 0; i < internal_mode_count; ++i)
      for (int j = i + 1; j < internal_mode_count; ++j)
        if (std::abs(matrix[i][j]) > largest) {
          largest = std::abs(matrix[i][j]); p = i; q = j;
        }
    if (largest <= 1e-14) break;
    const double angle = 0.5 * std::atan2(
        2.0 * matrix[p][q], matrix[q][q] - matrix[p][p]);
    const double c = std::cos(angle), s = std::sin(angle);
    const double app = matrix[p][p], aqq = matrix[q][q];
    const double apq = matrix[p][q];
    for (int k = 0; k < internal_mode_count; ++k) {
      if (k == p || k == q) continue;
      const double akp = matrix[k][p], akq = matrix[k][q];
      matrix[k][p] = matrix[p][k] = c * akp - s * akq;
      matrix[k][q] = matrix[q][k] = s * akp + c * akq;
    }
    matrix[p][p] = c*c*app - 2.0*s*c*apq + s*s*aqq;
    matrix[q][q] = s*s*app + 2.0*s*c*apq + c*c*aqq;
    matrix[p][q] = matrix[q][p] = 0.0;
  }
  std::array<double, internal_mode_count> result{};
  for (int i = 0; i < internal_mode_count; ++i) result[i] = matrix[i][i];
  std::sort(result.begin(), result.end());
  return result;
}

struct InternalBasis {
  bool valid = false;
  std::array<InternalPattern, internal_mode_count> patterns{};
  std::array<double, internal_mode_count> gram_eigenvalues{};
  double maximum_zero_sum_residual = INFINITY;
  double maximum_norm_residual = INFINITY;
  double minimum_gram_eigenvalue = -INFINITY;
};

InternalBasis make_internal_basis_0615(const StaticCoreEvaluation& rest) {
  InternalBasis result;
  if (!rest.valid) return result;
  std::array<Vec3, 3> positions{};
  Vec3 center{};
  for (std::size_t a = 0; a < 3; ++a) {
    positions[a] = effective_position(rest.state.constituents[a]);
    center += positions[a];
  }
  center *= 1.0 / 3.0;
  const std::array<Vec3, 3> axes{{{1,0,0},{0,1,0},{0,0,1}}};
  for (int mode = 0; mode < 3; ++mode)
    for (std::size_t a = 0; a < 3; ++a)
      result.patterns[mode][a] = cross_0615(
          axes[static_cast<std::size_t>(mode)], positions[a] - center);

  for (std::size_t a = 0; a < 3; ++a) {
    const double x = body_frame.coordinates[a][0];
    const double y = body_frame.coordinates[a][1];
    result.patterns[3][a] = apply_matrix_0615(
        rest.orientation, body_frame.e0 * (-x));
    result.patterns[4][a] = apply_matrix_0615(rest.orientation,
        (body_frame.e0 * y + body_frame.e1 * x) * -1.0);
    result.patterns[5][a] = apply_matrix_0615(
        rest.orientation, body_frame.e1 * (-y));
  }
  bool normalized = true;
  for (auto& pattern : result.patterns)
    normalized = normalize_pattern_0615(pattern) && normalized;
  result.maximum_zero_sum_residual = 0.0;
  result.maximum_norm_residual = 0.0;
  Gram6 gram{};
  for (int i = 0; i < internal_mode_count; ++i) {
    result.maximum_zero_sum_residual = std::max(
        result.maximum_zero_sum_residual,
        pattern_sum_0615(result.patterns[i]).mag());
    result.maximum_norm_residual = std::max(result.maximum_norm_residual,
        std::abs(pattern_dot_0615(result.patterns[i], result.patterns[i])
                 - 1.0));
    for (int j = 0; j < internal_mode_count; ++j)
      gram[i][j] = pattern_dot_0615(result.patterns[i], result.patterns[j]);
  }
  result.gram_eigenvalues = gram_eigenvalues_0615(gram);
  result.minimum_gram_eigenvalue = result.gram_eigenvalues.front();
  result.valid = normalized && result.maximum_zero_sum_residual <= 1e-12
      && result.maximum_norm_residual <= 1e-12
      && result.minimum_gram_eigenvalue > 1e-8;
  return result;
}

double internal_excitation_0615(const InternalPattern& pattern,
                                double amplitude) {
  double result = 0.0;
  for (const auto& entry : pattern)
    result += ftd::eft::production_flat_energy_from_momentum(
        entry * amplitude) - ftd::E_REST;
  return result;
}

struct ExcitationAmplitude {
  bool valid = false;
  double target = 0.0;
  double amplitude = INFINITY;
  double residual = INFINITY;
};

ExcitationAmplitude solve_excitation_0615(
    const InternalPattern& pattern, double target) {
  ExcitationAmplitude result;
  result.target = target;
  if (!(target > 0.0)) return result;
  double lo = 0.0, hi = 1.0;
  while (internal_excitation_0615(pattern, hi) < target && hi < 1e6)
    hi *= 2.0;
  if (!(hi < 1e6)) return result;
  for (int iteration = 0; iteration < 100; ++iteration) {
    const double mid = 0.5 * (lo + hi);
    if (internal_excitation_0615(pattern, mid) < target) lo = mid;
    else hi = mid;
  }
  result.amplitude = 0.5 * (lo + hi);
  result.residual = std::abs(
      internal_excitation_0615(pattern, result.amplitude) - target);
  result.valid = std::isfinite(result.amplitude)
      && result.residual <= 1e-12;
  return result;
}

struct InternalModeArm {
  bool excitation_valid = false;
  bool complete = false;
  bool base_pass = false;
  bool intact = false;
  bool walker = false;
  bool bounded = false;
  int mode = 0;
  int sign = 1;
  int ratio = 1;
  int forward_ticks = 0;
  int reverse_ticks = 0;
  int anchor_changes = 0;
  int maximum_multiplicity = 1;
  double target_excitation = 0.0;
  double amplitude = INFINITY;
  double excitation_residual = INFINITY;
  double initial_momentum_residual = INFINITY;
  double net_displacement = INFINITY;
  double maximum_excursion = INFINITY;
  double center_path_length = INFINITY;
  double maximum_center_momentum = INFINITY;
  double minimum_pair_distance = INFINITY;
  double maximum_pair_distance = 0.0;
  double worst_gate = INFINITY;
  double energy_drift = INFINITY;
  double maximum_pseudomomentum_defect = INFINITY;
  double recovery = INFINITY;
};

InternalModeArm run_internal_mode_arm_0615(
    int mode, int sign, int ratio, const InternalPattern& pattern,
    const ChargedTrimerState& rest, const std::vector<double>& uniform,
    const ftd::eft::ChargedTrimerOptions& options) {
  InternalModeArm arm;
  arm.mode = mode;
  arm.sign = sign;
  arm.ratio = ratio;
  arm.target_excitation = ratio * internal_delta_ref;
  const auto amplitude = solve_excitation_0615(
      pattern, arm.target_excitation);
  arm.amplitude = amplitude.amplitude;
  arm.excitation_residual = amplitude.residual;
  arm.excitation_valid = amplitude.valid;
  if (!amplitude.valid) return arm;
  ChargedTrimerState initial = rest;
  for (std::size_t a = 0; a < initial.constituents.size(); ++a)
    initial.constituents[a].momentum = pattern[a]
        * (sign * amplitude.amplitude);
  arm.initial_momentum_residual = single_momentum(initial).mag();
  ChargedTrimerState current = initial;
  const Vec3 center0 = single_center(initial);
  Vec3 previous_center = center0;
  arm.maximum_excursion = 0.0;
  arm.center_path_length = 0.0;
  arm.maximum_center_momentum = arm.initial_momentum_residual;
  arm.minimum_pair_distance = INFINITY;
  arm.worst_gate = 0.0;
  arm.energy_drift = 0.0;
  arm.maximum_pseudomomentum_defect = 0.0;
  double baseline = NAN;
  constexpr int ticks = 128;
  for (int tick = 0; tick < ticks; ++tick) {
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
        arm.maximum_pseudomomentum_defect,
        step.pseudomomentum_defect_norm);
    current = step.later;
    const Vec3 center = single_center(current);
    arm.center_path_length += (center - previous_center).mag();
    arm.maximum_excursion = std::max(
        arm.maximum_excursion, (center - center0).mag());
    arm.maximum_center_momentum = std::max(
        arm.maximum_center_momentum, single_momentum(current).mag());
    previous_center = center;
  }
  if (arm.forward_ticks == ticks) {
    arm.net_displacement = (single_center(current) - center0).mag();
    for (int tick = 0; tick < ticks; ++tick) {
      const auto step = ftd::eft::solve_charged_trimer_reverse(
          current, uniform, options);
      if (!step.valid) break;
      ++arm.reverse_ticks;
      arm.maximum_multiplicity = std::max(
          arm.maximum_multiplicity,
          maximum_anchor_multiplicity(step.earlier));
      arm.worst_gate = std::max(arm.worst_gate, single_maximum_gate(step));
      arm.energy_drift = std::max(arm.energy_drift,
          std::max(std::abs(single_energy_before(step) - baseline),
                   std::abs(single_energy_after(step) - baseline)));
      arm.minimum_pair_distance = std::min(
          arm.minimum_pair_distance, step.minimum_pair_distance);
      arm.maximum_pair_distance = std::max(
          arm.maximum_pair_distance, step.maximum_pair_distance);
      arm.maximum_pseudomomentum_defect = std::max(
          arm.maximum_pseudomomentum_defect,
          step.pseudomomentum_defect_norm);
      current = step.earlier;
    }
  }
  arm.complete = arm.forward_ticks == ticks && arm.reverse_ticks == ticks;
  if (arm.complete)
    arm.recovery = ftd::eft::charged_trimer_state_max_difference(
        initial, current);
  arm.base_pass = arm.complete && arm.excitation_valid
      && arm.initial_momentum_residual <= 1e-12
      && arm.worst_gate <= gate && arm.energy_drift <= 1e-10
      && arm.recovery <= 1e-9 && arm.maximum_multiplicity <= 2;
  arm.intact = arm.base_pass && arm.minimum_pair_distance >= 0.5
      && arm.maximum_pair_distance <= 2.0;
  arm.walker = arm.intact && arm.net_displacement >= 0.75
      && arm.maximum_excursion >= 1.0 && arm.anchor_changes >= 3;
  arm.bounded = arm.intact && arm.maximum_excursion < 0.5
      && arm.net_displacement < 0.25;
  return arm;
}

struct InternalModeSummary {
  bool parent_hash = false;
  bool rest_fingerprint = false;
  bool rest_gate = false;
  bool basis_coverage = false;
  bool arm_coverage = false;
  int walkers = 0;
  int bounded = 0;
  int broken_geometry = 0;
  int intermediate = 0;
  InternalBasis basis{};
  RefineResult refined{};
  SingleArm rest{};
  std::vector<InternalModeArm> arms;
  std::string verdict;
};

void write_internal_mode_record_0615(const InternalModeSummary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0615";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0615_zero_momentum_internal_modes_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0615\",\n"
       << "  \"protocol_sha256\": \"" << internal_mode_protocol_sha256
       << "\",\n  \"parent_result_sha256\": \"" << parent_result_sha256
       << "\",\n  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"parent_hash_pass\": "
       << (summary.parent_hash ? "true" : "false") << ",\n"
       << "  \"rest_fingerprint_pass\": "
       << (summary.rest_fingerprint ? "true" : "false") << ",\n"
       << "  \"rest_gate_pass\": "
       << (summary.rest_gate ? "true" : "false") << ",\n"
       << "  \"basis_coverage\": "
       << (summary.basis_coverage ? "true" : "false") << ",\n"
       << "  \"arm_coverage\": "
       << (summary.arm_coverage ? "true" : "false") << ",\n"
       << "  \"delta_ref\": " << internal_delta_ref << ",\n"
       << "  \"basis_zero_sum_residual\": "
       << json_number(summary.basis.maximum_zero_sum_residual) << ",\n"
       << "  \"basis_norm_residual\": "
       << json_number(summary.basis.maximum_norm_residual) << ",\n"
       << "  \"minimum_gram_eigenvalue\": "
       << json_number(summary.basis.minimum_gram_eigenvalue) << ",\n"
       << "  \"gram_eigenvalues\": [";
  for (int i = 0; i < internal_mode_count; ++i)
    json << (i ? "," : "") << summary.basis.gram_eigenvalues[i];
  json << "],\n  \"walker_count\": " << summary.walkers
       << ",\n  \"bounded_count\": " << summary.bounded
       << ",\n  \"broken_geometry_count\": " << summary.broken_geometry
       << ",\n  \"intermediate_count\": " << summary.intermediate
       << ",\n  \"arms\": [\n";
  for (std::size_t i = 0; i < summary.arms.size(); ++i) {
    const auto& arm = summary.arms[i];
    json << "    {\"mode\": " << arm.mode << ", \"sign\": " << arm.sign
         << ", \"ratio\": " << arm.ratio
         << ", \"excitation_valid\": "
         << (arm.excitation_valid ? "true" : "false")
         << ", \"complete\": " << (arm.complete ? "true" : "false")
         << ", \"base_pass\": " << (arm.base_pass ? "true" : "false")
         << ", \"intact\": " << (arm.intact ? "true" : "false")
         << ", \"walker\": " << (arm.walker ? "true" : "false")
         << ", \"bounded\": " << (arm.bounded ? "true" : "false")
         << ", \"forward_ticks\": " << arm.forward_ticks
         << ", \"reverse_ticks\": " << arm.reverse_ticks
         << ", \"anchor_changes\": " << arm.anchor_changes
         << ", \"maximum_anchor_multiplicity\": "
         << arm.maximum_multiplicity
         << ", \"target_excitation\": " << arm.target_excitation
         << ", \"amplitude\": " << json_number(arm.amplitude)
         << ", \"excitation_residual\": "
         << json_number(arm.excitation_residual)
         << ", \"initial_momentum_residual\": "
         << json_number(arm.initial_momentum_residual)
         << ", \"net_displacement\": "
         << json_number(arm.net_displacement)
         << ", \"maximum_excursion\": "
         << json_number(arm.maximum_excursion)
         << ", \"center_path_length\": "
         << json_number(arm.center_path_length)
         << ", \"maximum_center_momentum\": "
         << json_number(arm.maximum_center_momentum)
         << ", \"minimum_pair_distance\": "
         << json_number(arm.minimum_pair_distance)
         << ", \"maximum_pair_distance\": "
         << json_number(arm.maximum_pair_distance)
         << ", \"worst_common_gate\": " << json_number(arm.worst_gate)
         << ", \"maximum_energy_drift\": "
         << json_number(arm.energy_drift)
         << ", \"maximum_pseudomomentum_defect\": "
         << json_number(arm.maximum_pseudomomentum_defect)
         << ", \"reverse_recovery\": " << json_number(arm.recovery) << "}"
         << (i + 1 == summary.arms.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";

  std::ofstream csv(dir / "ftd_0615_zero_momentum_internal_mode_arms_v1.csv");
  csv << "ftd_id,mode,sign,ratio,complete,base_pass,intact,walker,bounded,"
         "forward_ticks,reverse_ticks,anchor_changes,max_multiplicity,"
         "target_excitation,amplitude,excitation_residual,initial_momentum,"
         "net_displacement,max_excursion,path_length,max_center_momentum,"
         "min_distance,max_distance,worst_gate,energy_drift,"
         "pseudomomentum_defect,recovery\n";
  for (const auto& arm : summary.arms)
    csv << std::setprecision(17) << "FTD-0615," << arm.mode << ',' << arm.sign
        << ',' << arm.ratio << ',' << arm.complete << ',' << arm.base_pass
        << ',' << arm.intact << ',' << arm.walker << ',' << arm.bounded << ','
        << arm.forward_ticks << ',' << arm.reverse_ticks << ','
        << arm.anchor_changes << ',' << arm.maximum_multiplicity << ','
        << arm.target_excitation << ',' << arm.amplitude << ','
        << arm.excitation_residual << ',' << arm.initial_momentum_residual
        << ',' << arm.net_displacement << ',' << arm.maximum_excursion << ','
        << arm.center_path_length << ',' << arm.maximum_center_momentum << ','
        << arm.minimum_pair_distance << ',' << arm.maximum_pair_distance << ','
        << arm.worst_gate << ',' << arm.energy_drift << ','
        << arm.maximum_pseudomomentum_defect << ',' << arm.recovery << '\n';
}

}  // namespace

#ifndef FTD0615_NO_MAIN
int main() {
  std::cout << std::setprecision(17);
  InternalModeSummary summary;
  const auto parent_path = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results" / "ftd_0614"
      / "ftd_0614_refined_core_peierls_landscape_v1.json";
  std::ifstream parent(parent_path, std::ios::binary);
  std::string parent_bytes((std::istreambuf_iterator<char>(parent)),
                           std::istreambuf_iterator<char>());
  // The independent certificate performs the cryptographic comparison.  The
  // compiled runner requires the exact locked record fingerprint fields.
  summary.parent_hash = parent_bytes.find("\"ftd_id\": \"FTD-0614\"")
          != std::string::npos
      && parent_bytes.find("\"minimum_relaxed_barrier\": 0.00011302707069732617")
          != std::string::npos
      && parent_bytes.find("REFINED_CORE_PEIERLS_LANDSCAPE_NUMERICALLY_UNRESOLVED")
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
  if (summary.parent_hash && normalization.valid && green.valid
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
      fixture.name = "internal_mode_rest";
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
  if (summary.rest_gate) summary.basis = make_internal_basis_0615(
      summary.refined.state);
  summary.basis_coverage = summary.basis.valid;
  if (summary.basis_coverage)
    for (int mode = 0; mode < internal_mode_count; ++mode)
      for (int ratio : {1, 4})
        for (int sign : {-1, +1})
          summary.arms.push_back(run_internal_mode_arm_0615(
              mode, sign, ratio, summary.basis.patterns[mode], rest_state,
              uniform, options));
  summary.arm_coverage = summary.arms.size() == 24
      && std::all_of(summary.arms.begin(), summary.arms.end(),
          [](const InternalModeArm& arm) { return arm.base_pass; });
  for (const auto& arm : summary.arms) {
    if (arm.walker) ++summary.walkers;
    else if (arm.bounded) ++summary.bounded;
    else if (arm.base_pass && !arm.intact) ++summary.broken_geometry;
    else ++summary.intermediate;
  }
  const bool coverage = summary.parent_hash && summary.rest_fingerprint
      && summary.rest_gate && summary.basis_coverage && summary.arm_coverage;
  if (!coverage)
    summary.verdict =
        "ZERO_MOMENTUM_INTERNAL_MODE_MOBILITY_NUMERICALLY_UNRESOLVED";
  else if (summary.walkers > 0)
    summary.verdict = "ZERO_MOMENTUM_INTERNAL_WALKER_CONSTRUCTIVE";
  else
    summary.verdict = "REGISTERED_INTERNAL_MODES_NO_STABLE_WALKER";
  write_internal_mode_record_0615(summary);
  std::cout << "protocol_sha256=" << internal_mode_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "basis=" << summary.basis_coverage
            << " min_gram=" << summary.basis.minimum_gram_eigenvalue
            << " arms=" << summary.arms.size() << '\n'
            << "walkers=" << summary.walkers
            << " bounded=" << summary.bounded
            << " broken=" << summary.broken_geometry
            << " intermediate=" << summary.intermediate << '\n';
  for (const auto& arm : summary.arms)
    std::cout << "mode=" << arm.mode << " sign=" << arm.sign
              << " ratio=" << arm.ratio
              << " net=" << arm.net_displacement
              << " max=" << arm.maximum_excursion
              << " path=" << arm.center_path_length
              << " distances=" << arm.minimum_pair_distance << ','
              << arm.maximum_pair_distance
              << " walker=" << arm.walker
              << " bounded=" << arm.bounded
              << " pass=" << arm.base_pass << '\n';
  return summary.arms.size() == 24 ? 0 : 1;
}
#endif
