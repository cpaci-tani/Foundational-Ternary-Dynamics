// FTD-0660: direct constituent/dressing/dynamic-field action-transfer ledger.
#define FTD_0659_EMBEDDED
#include "test_native_excited_matter_clock.cpp"
#undef FTD_0659_EMBEDDED

namespace {

constexpr char transfer_protocol_sha256[] =
    "7731CFC6D1C4C41FF9BD3118D2B78568E99E1D8D91126C92E744AAB91F2D9C9B";
constexpr int transfer_ticks = 128;

struct TransferTick {
  int tick = 0;
  bool redress = false;
  double doublet_energy = 0.0;
  double doublet_ratio = 0.0;
  double kinetic_excitation = 0.0;
  double binding_excitation = 0.0;
  double field_excitation = 0.0;
  double dressing_excitation = 0.0;
  double residual_field_energy = 0.0;
  double field_interference = 0.0;
  double total_excitation = 0.0;
  double field_decomposition_residual = 0.0;
  double total_energy_drift = 0.0;
  double other_matter_norm = 0.0;
  double residual_norm = 0.0;
  double near_norm = 0.0;
  double middle_norm = 0.0;
  double far_norm = 0.0;
  double far_fraction = 0.0;
  double common = 0.0;
};

struct TransferArm {
  ClockSpec spec;
  bool initialization = false;
  bool forward = false;
  bool reverse = false;
  bool bounded = false;
  bool redress = false;
  bool sector = false;
  bool no_hops = false;
  int max_multiplicity = 0;
  int total_hops = 0;
  int near_onset = -1;
  int middle_onset = -1;
  int far_onset = -1;
  double min_separation = INFINITY;
  double modal_amplitude = 0.0;
  double initial_excitation = 0.0;
  double min_doublet_ratio = INFINITY;
  double recovered_doublet_ratio = 0.0;
  double max_dynamic_ratio = 0.0;
  double max_far_fraction = 0.0;
  double max_common = 0.0;
  double max_energy_drift = 0.0;
  double max_decomposition_residual = 0.0;
  double recovery = INFINITY;
  std::vector<TransferTick> ticks;
};

struct TransferSummary {
  bool parent = false;
  bool normalization = false;
  bool eigenspace = false;
  bool coverage = false;
  bool execution = false;
  bool bounded = false;
  bool transfer = false;
  bool amplitude = false;
  bool sign = false;
  bool covariance = false;
  bool zero = false;
  bool dynamic_morphology = false;
  bool local_morphology = false;
  double beta = 0.0;
  double worst_common = 0.0;
  double worst_energy_drift = 0.0;
  double worst_decomposition_residual = 0.0;
  double worst_recovery = 0.0;
  double minimum_doublet_ratio = INFINITY;
  double minimum_dynamic_ratio = INFINITY;
  double minimum_far_fraction = INFINITY;
  double maximum_far_fraction = 0.0;
  double amplitude_residual = 0.0;
  double sign_residual = 0.0;
  double covariance_residual = 0.0;
  double zero_residual = 0.0;
  std::array<FullModes, 2> modes;
  std::vector<TransferArm> arms;
  std::string verdict = "INTERNAL_MODE_ACTION_TRANSFER_EXECUTION_INVALID";
};

struct EnergyParts {
  double kinetic = 0.0;
  double binding = 0.0;
  double field = 0.0;
  double total = 0.0;
};

EnergyParts energy_parts(
    const ftd::eft::ConnectedMooreBlockState& state,
    double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  EnergyParts result;
  for (const auto& point : state.constituents) {
    result.kinetic += ftd::eft::production_flat_energy_from_momentum(
        point.momentum);
  }
  result.binding = ftd::eft::connected_moore_block_binding_energy(state, options);
  result.field = beta * ftd::eft::matched_modified_energy(
      state.electric, state.magnetic_half, ftd::C_SPEED);
  result.total = result.kinetic + result.binding + result.field;
  return result;
}

ftd::eft::MatchedFaceFlux subtract_face(
    const ftd::eft::MatchedFaceFlux& a,
    const ftd::eft::MatchedFaceFlux& b) {
  ftd::eft::MatchedFaceFlux result(a.L);
  for (std::size_t index = 0; index < a.x.size(); ++index) {
    result.x[index] = a.x[index] - b.x[index];
    result.y[index] = a.y[index] - b.y[index];
    result.z[index] = a.z[index] - b.z[index];
  }
  return result;
}

double periodic_distance(double coordinate, double center_value, int size) {
  double delta = std::abs(coordinate - center_value);
  delta = std::fmod(delta, static_cast<double>(size));
  return std::min(delta, static_cast<double>(size) - delta);
}

std::array<double, 4> residual_shell_norms(
    const ftd::eft::MatchedFaceFlux& electric,
    const ftd::eft::MatchedEdgeField& magnetic,
    const Vec3& object_center,
    double beta) {
  std::array<long double, 4> sums{};
  const int size = electric.L;
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      for (int z = 0; z < size; ++z) {
        const std::size_t index = static_cast<std::size_t>(
            (x * size + y) * size + z);
        const long double density = 0.5L * beta * (
            static_cast<long double>(electric.x[index]) * electric.x[index]
            + static_cast<long double>(electric.y[index]) * electric.y[index]
            + static_cast<long double>(electric.z[index]) * electric.z[index]
            + static_cast<long double>(magnetic.x[index]) * magnetic.x[index]
            + static_cast<long double>(magnetic.y[index]) * magnetic.y[index]
            + static_cast<long double>(magnetic.z[index]) * magnetic.z[index]);
        const double dx = periodic_distance(x, object_center.x, size);
        const double dy = periodic_distance(y, object_center.y, size);
        const double dz = periodic_distance(z, object_center.z, size);
        const double radius = std::sqrt(dx * dx + dy * dy + dz * dz);
        sums[0] += density;
        if (radius < 3.0) sums[1] += density;
        else if (radius < 5.5) sums[2] += density;
        else sums[3] += density;
      }
    }
  }
  return {{static_cast<double>(sums[0]), static_cast<double>(sums[1]),
           static_cast<double>(sums[2]), static_cast<double>(sums[3])}};
}

TransferTick observe_transfer(
    int tick,
    const ftd::eft::ConnectedMooreBlockState& reference,
    const EnergyParts& reference_energy,
    const ftd::eft::ConnectedMooreBlockState& state,
    const FullModes& modes,
    double beta,
    double initial_total,
    double initial_doublet,
    double common,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  TransferTick record;
  record.tick = tick;
  const auto q = project_modes(reference, state, modes);
  const auto p = project_momenta(state, modes);
  const double omega = 0.5 * (modes.modes[6].omega + modes.modes[7].omega);
  const double q2 = q[6] * q[6] + q[7] * q[7];
  const double p2 = p[6] * p[6] + p[7] * p[7];
  record.doublet_energy = 0.5 * (p2 + omega * omega * q2);
  record.doublet_ratio = initial_doublet > 0.0
      ? record.doublet_energy / initial_doublet : 0.0;
  long double target_norm = q2 + p2 / (omega * omega);
  long double other_norm = 0.0;
  for (int mode = 0; mode < N; ++mode) {
    if (modes.modes[mode].group == modes.modes[6].group) continue;
    const double mode_omega = modes.modes[mode].omega;
    other_norm += static_cast<long double>(q[mode]) * q[mode]
        + static_cast<long double>(p[mode]) * p[mode]
            / (mode_omega * mode_omega);
  }
  record.other_matter_norm = target_norm > 0.0
      ? std::sqrt(static_cast<double>(other_norm / target_norm)) : 0.0;

  const auto current = energy_parts(state, beta, options);
  record.kinetic_excitation = current.kinetic - reference_energy.kinetic;
  record.binding_excitation = current.binding - reference_energy.binding;
  record.field_excitation = current.field - reference_energy.field;
  record.total_excitation = current.total - reference_energy.total;
  record.total_energy_drift = std::abs(current.total - initial_total);

  auto geometry = state;
  const auto dressed = ftd::eft::redress_connected_moore_block_with_fibre_limit(
      geometry, 8, 1e-13, 4096);
  record.redress = dressed.valid;
  if (!record.redress) return record;
  const auto residual_electric = subtract_face(
      state.electric, dressed.state.electric);
  const auto& residual_magnetic = state.magnetic_half;
  const double dressing_field = beta * ftd::eft::matched_modified_energy(
      dressed.state.electric, dressed.state.magnetic_half, ftd::C_SPEED);
  record.dressing_excitation = dressing_field - reference_energy.field;
  record.residual_field_energy = beta * ftd::eft::matched_modified_energy(
      residual_electric, residual_magnetic, ftd::C_SPEED);
  const auto curl_dressing = ftd::eft::matched_curl_adjoint(
      dressed.state.electric);
  record.field_interference = beta * static_cast<double>(
      ftd::eft::matched_face_dot(dressed.state.electric, residual_electric)
      - 0.5L * ftd::C_SPEED
          * ftd::eft::matched_edge_dot(residual_magnetic, curl_dressing));
  record.field_decomposition_residual = std::abs(
      current.field - dressing_field - record.residual_field_energy
      - record.field_interference);
  const auto shells = residual_shell_norms(
      residual_electric, residual_magnetic, center(state), beta);
  record.residual_norm = shells[0];
  record.near_norm = shells[1];
  record.middle_norm = shells[2];
  record.far_norm = shells[3];
  record.far_fraction = record.residual_norm > 0.0
      ? record.far_norm / record.residual_norm : 0.0;
  record.common = common;
  return record;
}

void set_onsets(TransferArm& arm) {
  double near_max = 0.0, middle_max = 0.0, far_max = 0.0;
  for (const auto& tick : arm.ticks) {
    near_max = std::max(near_max, tick.near_norm);
    middle_max = std::max(middle_max, tick.middle_norm);
    far_max = std::max(far_max, tick.far_norm);
  }
  for (const auto& tick : arm.ticks) {
    if (arm.near_onset < 0 && near_max > 0.0
        && tick.near_norm >= 0.25 * near_max) arm.near_onset = tick.tick;
    if (arm.middle_onset < 0 && middle_max > 0.0
        && tick.middle_norm >= 0.25 * middle_max) arm.middle_onset = tick.tick;
    if (arm.far_onset < 0 && far_max > 0.0
        && tick.far_norm >= 0.25 * far_max) arm.far_onset = tick.tick;
  }
}

TransferArm run_transfer_arm(
    const ClockSpec& spec,
    const FullModes& modes,
    double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  TransferArm arm;
  arm.spec = spec;
  const auto reference = load_refined_state(spec.orientation);
  if (reference.electric.L != L) return arm;
  const auto reference_energy = energy_parts(reference, beta, options);
  const double omega = 0.5 * (modes.modes[6].omega + modes.modes[7].omega);
  double q_error = INFINITY, p_error = INFINITY;
  auto initial = prepare_clock_state(
      spec, modes, omega, arm.modal_amplitude, q_error, p_error);
  arm.initialization = initial.electric.L == L
      && q_error <= 1e-12 && p_error <= 1e-12;
  if (!arm.initialization) return arm;
  const auto q0 = project_modes(reference, initial, modes);
  const auto p0 = project_momenta(initial, modes);
  const double initial_doublet = 0.5 * (
      p0[6] * p0[6] + p0[7] * p0[7]
      + omega * omega * (q0[6] * q0[6] + q0[7] * q0[7]));
  const auto initial_energy = energy_parts(initial, beta, options);
  arm.initial_excitation = initial_energy.total - reference_energy.total;
  const auto initial_sector = sector_signature(initial);
  auto state = initial;
  arm.forward = true;
  arm.sector = true;
  arm.ticks.push_back(observe_transfer(
      0, reference, reference_energy, state, modes, beta,
      initial_energy.total, initial_doublet, 0.0, options));
  arm.redress = arm.ticks.back().redress;
  ftd::eft::ConnectedMooreBlockSolveCache forward_cache, reverse_cache;

  for (int tick = 1; tick <= transfer_ticks; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state, options, &forward_cache);
    const double residual = common_residual(step);
    if (!step.valid || !step.common_action_gates_pass || residual > 1e-10) {
      arm.forward = false;
      break;
    }
    state = step.later;
    arm.total_hops += step.site_hops;
    arm.sector = arm.sector && sector_signature(state) == initial_sector;
    int multiplicity_value = 0;
    double separation = INFINITY;
    std::tie(multiplicity_value, separation) = mode_fibre(state);
    arm.max_multiplicity = std::max(arm.max_multiplicity, multiplicity_value);
    if (std::isfinite(separation)) {
      arm.min_separation = std::min(arm.min_separation, separation);
    }
    arm.ticks.push_back(observe_transfer(
        tick, reference, reference_energy, state, modes, beta,
        initial_energy.total, initial_doublet, residual, options));
    arm.redress = arm.redress && arm.ticks.back().redress;
  }
  arm.forward = arm.forward && arm.ticks.size() == transfer_ticks + 1;

  arm.reverse = arm.forward;
  for (int tick = transfer_ticks; arm.reverse && tick >= 1; --tick) {
    const auto step = ftd::eft::solve_connected_moore_block_reverse(
        state, options, &reverse_cache);
    const double residual = common_residual(step);
    arm.max_common = std::max(arm.max_common, residual);
    if (!step.valid || !step.common_action_gates_pass || residual > 1e-10) {
      arm.reverse = false;
      break;
    }
    state = step.earlier;
    arm.total_hops += step.site_hops;
    arm.sector = arm.sector && sector_signature(state) == initial_sector;
  }
  if (arm.reverse) {
    arm.recovery = ftd::eft::connected_moore_block_state_max_difference(
        initial, state);
  }
  arm.no_hops = arm.total_hops == 0;
  bool below = false;
  for (const auto& tick : arm.ticks) {
    arm.min_doublet_ratio = std::min(
        arm.min_doublet_ratio, tick.doublet_ratio);
    if (below) {
      arm.recovered_doublet_ratio = std::max(
          arm.recovered_doublet_ratio, tick.doublet_ratio);
    }
    if (tick.doublet_ratio < 0.60) below = true;
    if (arm.initial_excitation > 0.0) {
      arm.max_dynamic_ratio = std::max(
          arm.max_dynamic_ratio,
          std::max(std::abs(tick.residual_field_energy), tick.residual_norm)
              / arm.initial_excitation);
    }
    arm.max_far_fraction = std::max(
        arm.max_far_fraction, tick.far_fraction);
    arm.max_common = std::max(arm.max_common, tick.common);
    arm.max_energy_drift = std::max(
        arm.max_energy_drift, tick.total_energy_drift);
    arm.max_decomposition_residual = std::max(
        arm.max_decomposition_residual, tick.field_decomposition_residual);
  }
  set_onsets(arm);
  arm.bounded = arm.forward && arm.reverse && arm.redress && arm.sector
      && arm.no_hops && arm.max_multiplicity <= 8
      && (!std::isfinite(arm.min_separation) || arm.min_separation >= 0.9)
      && arm.max_common <= 1e-10 && arm.max_energy_drift <= 1e-12
      && arm.max_decomposition_residual <= 1e-12
      && arm.recovery <= 1e-10;
  return arm;
}

const TransferArm* find_transfer_arm(
    const TransferSummary& summary,
    int orientation,
    int polarization,
    int amplitude,
    int quadrature) {
  for (const auto& arm : summary.arms) {
    if (!arm.spec.zero && arm.spec.orientation == orientation
        && arm.spec.polarization == polarization
        && arm.spec.amplitude == amplitude
        && arm.spec.quadrature == quadrature) return &arm;
  }
  return nullptr;
}

double normalized_history_residual(
    const TransferArm& a,
    const TransferArm& b,
    double scale_a,
    double scale_b) {
  if (a.ticks.size() != b.ticks.size()) return INFINITY;
  long double difference = 0.0, norm_a = 0.0, norm_b = 0.0;
  auto add = [&](double x, double y) {
    x /= scale_a;
    y /= scale_b;
    difference += static_cast<long double>(x - y) * (x - y);
    norm_a += static_cast<long double>(x) * x;
    norm_b += static_cast<long double>(y) * y;
  };
  for (std::size_t tick = 0; tick < a.ticks.size(); ++tick) {
    const auto& x = a.ticks[tick];
    const auto& y = b.ticks[tick];
    add(x.doublet_energy, y.doublet_energy);
    add(x.kinetic_excitation, y.kinetic_excitation);
    add(x.binding_excitation, y.binding_excitation);
    add(x.field_excitation, y.field_excitation);
    add(x.dressing_excitation, y.dressing_excitation);
    add(x.residual_field_energy, y.residual_field_energy);
    add(x.field_interference, y.field_interference);
    add(x.residual_norm, y.residual_norm);
    add(x.near_norm, y.near_norm);
    add(x.middle_norm, y.middle_norm);
    add(x.far_norm, y.far_norm);
  }
  return std::sqrt(static_cast<double>(difference))
      / std::max({1e-300, std::sqrt(static_cast<double>(norm_a)),
                  std::sqrt(static_cast<double>(norm_b))});
}

void evaluate_transfer(TransferSummary& summary) {
  summary.coverage = summary.arms.size() == 18;
  summary.execution = summary.coverage && std::all_of(
      summary.arms.begin(), summary.arms.end(),
      [](const TransferArm& arm) {
        return arm.initialization && arm.forward && arm.reverse && arm.redress;
      });
  summary.bounded = summary.execution && std::all_of(
      summary.arms.begin(), summary.arms.end(),
      [](const TransferArm& arm) { return arm.bounded; });
  summary.transfer = summary.bounded;
  summary.dynamic_morphology = summary.bounded;
  summary.local_morphology = summary.bounded;
  for (const auto& arm : summary.arms) {
    summary.worst_common = std::max(summary.worst_common, arm.max_common);
    summary.worst_energy_drift = std::max(
        summary.worst_energy_drift, arm.max_energy_drift);
    summary.worst_decomposition_residual = std::max(
        summary.worst_decomposition_residual,
        arm.max_decomposition_residual);
    if (std::isfinite(arm.recovery)) {
      summary.worst_recovery = std::max(summary.worst_recovery, arm.recovery);
    }
    if (arm.spec.zero) {
      for (const auto& tick : arm.ticks) {
        summary.zero_residual = std::max({
            summary.zero_residual,
            std::abs(tick.kinetic_excitation),
            std::abs(tick.binding_excitation),
            std::abs(tick.field_excitation),
            std::abs(tick.dressing_excitation),
            std::abs(tick.residual_field_energy),
            std::abs(tick.field_interference),
            tick.residual_norm});
      }
      continue;
    }
    summary.minimum_doublet_ratio = std::min(
        summary.minimum_doublet_ratio, arm.min_doublet_ratio);
    summary.minimum_dynamic_ratio = std::min(
        summary.minimum_dynamic_ratio, arm.max_dynamic_ratio);
    summary.minimum_far_fraction = std::min(
        summary.minimum_far_fraction, arm.max_far_fraction);
    summary.maximum_far_fraction = std::max(
        summary.maximum_far_fraction, arm.max_far_fraction);
    summary.transfer = summary.transfer && arm.min_doublet_ratio <= 0.60
        && arm.max_dynamic_ratio >= 0.05;
    summary.dynamic_morphology = summary.dynamic_morphology
        && arm.max_far_fraction >= 0.10
        && arm.near_onset >= 0 && arm.middle_onset >= arm.near_onset
        && arm.far_onset >= arm.middle_onset;
    summary.local_morphology = summary.local_morphology
        && arm.max_far_fraction < 0.10
        && arm.recovered_doublet_ratio >= 0.80;
  }
  summary.zero = summary.execution && summary.zero_residual <= 1e-20;

  summary.amplitude = summary.sign = summary.covariance = summary.execution;
  for (int orientation = 0; orientation < 2; ++orientation) {
    for (int polarization : {0, 2}) {
      for (int quadrature : {1, 3}) {
        const auto* half = find_transfer_arm(
            summary, orientation, polarization, 1, quadrature);
        const auto* full = find_transfer_arm(
            summary, orientation, polarization, 2, quadrature);
        const double residual = half && full
            ? normalized_history_residual(
                *half, *full,
                clock_targets[1] * clock_targets[1],
                clock_targets[2] * clock_targets[2])
            : INFINITY;
        summary.amplitude_residual = std::max(
            summary.amplitude_residual, residual);
        summary.amplitude = summary.amplitude && residual <= 0.05;
      }
      for (int amplitude : {1, 2}) {
        const auto* positive = find_transfer_arm(
            summary, orientation, polarization, amplitude, 1);
        const auto* negative = find_transfer_arm(
            summary, orientation, polarization, amplitude, 3);
        const double residual = positive && negative
            ? normalized_history_residual(*positive, *negative, 1.0, 1.0)
            : INFINITY;
        summary.sign_residual = std::max(summary.sign_residual, residual);
        summary.sign = summary.sign && residual <= 0.05;
      }
    }
  }
  for (int polarization : {0, 2}) {
    for (int amplitude : {1, 2}) {
      for (int quadrature : {1, 3}) {
        const auto* x = find_transfer_arm(
            summary, 0, polarization, amplitude, quadrature);
        const auto* y = find_transfer_arm(
            summary, 1, polarization, amplitude, quadrature);
        const double residual = x && y
            ? normalized_history_residual(*x, *y, 1.0, 1.0) : INFINITY;
        summary.covariance_residual = std::max(
            summary.covariance_residual, residual);
        summary.covariance = summary.covariance && residual <= 0.05;
      }
    }
  }

  if (!summary.parent || !summary.normalization || !summary.eigenspace
      || !summary.coverage || !summary.execution) {
    summary.verdict = "INTERNAL_MODE_ACTION_TRANSFER_EXECUTION_INVALID";
  } else if (!summary.bounded) {
    summary.verdict = "INTERNAL_MODE_ACTION_TRANSFER_CLOSED_NEGATIVE";
  } else if (summary.transfer && summary.amplitude && summary.sign
             && summary.covariance && summary.zero
             && summary.dynamic_morphology) {
    summary.verdict = "INTERNAL_MODE_DYNAMIC_FIELD_TRANSFER_CONSTRUCTIVE";
  } else if (summary.transfer && summary.amplitude && summary.sign
             && summary.covariance && summary.zero
             && summary.local_morphology) {
    summary.verdict = "INTERNAL_MODE_LOCAL_HYBRID_TRANSFER_CONSTRUCTIVE";
  } else {
    summary.verdict = "INTERNAL_MODE_ACTION_TRANSFER_MIXED";
  }
}

void write_transfer(const TransferSummary& summary) {
  const auto directory = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0660";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory / "ftd_0660_internal_mode_action_transfer_v1.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0660\",\n"
       << "  \"protocol_sha256\": \"" << transfer_protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"arm_count\": " << summary.arms.size() << ",\n"
       << "  \"ticks_each_direction\": " << transfer_ticks << ",\n"
       << "  \"coverage_pass\": " << summary.coverage << ",\n"
       << "  \"execution_pass\": " << summary.execution << ",\n"
       << "  \"bounded_pass\": " << summary.bounded << ",\n"
       << "  \"transfer_pass\": " << summary.transfer << ",\n"
       << "  \"amplitude_pass\": " << summary.amplitude << ",\n"
       << "  \"sign_pass\": " << summary.sign << ",\n"
       << "  \"covariance_pass\": " << summary.covariance << ",\n"
       << "  \"zero_control_pass\": " << summary.zero << ",\n"
       << "  \"dynamic_morphology_pass\": " << summary.dynamic_morphology << ",\n"
       << "  \"local_morphology_pass\": " << summary.local_morphology << ",\n"
       << "  \"worst_common_residual\": " << summary.worst_common << ",\n"
       << "  \"worst_energy_drift\": " << summary.worst_energy_drift << ",\n"
       << "  \"worst_decomposition_residual\": "
       << summary.worst_decomposition_residual << ",\n"
       << "  \"worst_recovery\": " << summary.worst_recovery << ",\n"
       << "  \"minimum_doublet_ratio\": " << summary.minimum_doublet_ratio << ",\n"
       << "  \"minimum_dynamic_ratio\": " << summary.minimum_dynamic_ratio << ",\n"
       << "  \"minimum_far_fraction\": " << summary.minimum_far_fraction << ",\n"
       << "  \"maximum_far_fraction\": " << summary.maximum_far_fraction << ",\n"
       << "  \"amplitude_residual\": " << summary.amplitude_residual << ",\n"
       << "  \"sign_residual\": " << summary.sign_residual << ",\n"
       << "  \"covariance_residual\": " << summary.covariance_residual << ",\n"
       << "  \"zero_residual\": " << summary.zero_residual << "\n}\n";

  std::ofstream arms(directory / "ftd_0660_internal_mode_action_transfer_arms_v1.csv");
  arms << "ftd_id,label,orientation,polarization,amplitude,quadrature,zero,"
          "initialization,forward,reverse,bounded,redress,initial_excitation,"
          "min_doublet_ratio,recovered_doublet_ratio,max_dynamic_ratio,"
          "max_far_fraction,near_onset,middle_onset,far_onset,max_common,"
          "max_energy_drift,max_decomposition_residual,recovery\n";
  for (const auto& arm : summary.arms) {
    arms << std::setprecision(17) << "FTD-0660," << arm.spec.label << ','
         << arm.spec.orientation << ',' << arm.spec.polarization << ','
         << arm.spec.amplitude << ',' << arm.spec.quadrature << ','
         << arm.spec.zero << ',' << arm.initialization << ',' << arm.forward
         << ',' << arm.reverse << ',' << arm.bounded << ',' << arm.redress << ','
         << arm.initial_excitation << ',' << arm.min_doublet_ratio << ','
         << arm.recovered_doublet_ratio << ',' << arm.max_dynamic_ratio << ','
         << arm.max_far_fraction << ',' << arm.near_onset << ','
         << arm.middle_onset << ',' << arm.far_onset << ',' << arm.max_common
         << ',' << arm.max_energy_drift << ','
         << arm.max_decomposition_residual << ',' << arm.recovery << '\n';
  }

  std::ofstream ticks(directory / "ftd_0660_internal_mode_action_transfer_ticks_v1.csv");
  ticks << "ftd_id,label,tick,redress,doublet_energy,doublet_ratio,"
           "kinetic_excitation,binding_excitation,field_excitation,"
           "dressing_excitation,residual_field_energy,field_interference,"
           "total_excitation,field_decomposition_residual,total_energy_drift,"
           "other_matter_norm,residual_norm,near_norm,middle_norm,far_norm,"
           "far_fraction,common\n";
  for (const auto& arm : summary.arms) {
    for (const auto& tick : arm.ticks) {
      ticks << std::setprecision(17) << "FTD-0660," << arm.spec.label << ','
            << tick.tick << ',' << tick.redress << ',' << tick.doublet_energy
            << ',' << tick.doublet_ratio << ',' << tick.kinetic_excitation
            << ',' << tick.binding_excitation << ',' << tick.field_excitation
            << ',' << tick.dressing_excitation << ','
            << tick.residual_field_energy << ',' << tick.field_interference
            << ',' << tick.total_excitation << ','
            << tick.field_decomposition_residual << ','
            << tick.total_energy_drift << ',' << tick.other_matter_norm << ','
            << tick.residual_norm << ',' << tick.near_norm << ','
            << tick.middle_norm << ',' << tick.far_norm << ','
            << tick.far_fraction << ',' << tick.common << '\n';
    }
  }
}

}  // namespace

#ifdef FTD_0660_EMBEDDED
int ftd_0660_embedded_main() {
#else
int main() {
#endif
  TransferSummary summary;
  summary.parent = mode_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  summary.normalization = normalization.valid;
  summary.beta = normalization.mapped_field_work_coefficient;
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  if (summary.parent && summary.normalization) {
    for (int orientation = 0; orientation < 2; ++orientation) {
      const auto state = load_refined_state(orientation);
      const auto analytic = analytic_at(
          orientation == 0 ? "transfer_x" : "transfer_y",
          orientation, state, summary.beta, options);
      if (analytic.valid) summary.modes[orientation] = full_modes(analytic.hessian);
    }
    summary.eigenspace = summary.modes[0].valid && summary.modes[1].valid
        && summary.modes[0].modes[6].group == summary.modes[0].modes[7].group
        && relative_value(summary.modes[0].modes[6].hessian_eigen,
                          summary.modes[0].modes[7].hessian_eigen) <= 1e-9;
  }

  std::vector<ClockSpec> specs;
  if (summary.eigenspace) {
    for (int orientation = 0; orientation < 2; ++orientation) {
      for (int polarization : {0, 2}) {
        for (int amplitude : {1, 2}) {
          for (int quadrature : {1, 3}) {
            ClockSpec spec;
            spec.orientation = orientation;
            spec.polarization = polarization;
            spec.amplitude = amplitude;
            spec.quadrature = quadrature;
            spec.label = "o" + std::to_string(orientation)
                + "_p" + std::to_string(polarization)
                + "_a" + std::to_string(amplitude)
                + "_q" + std::to_string(quadrature);
            specs.push_back(spec);
          }
        }
      }
      ClockSpec zero;
      zero.orientation = orientation;
      zero.zero = true;
      zero.label = "o" + std::to_string(orientation) + "_zero";
      specs.push_back(zero);
    }
  }

  std::vector<std::future<TransferArm>> futures;
  for (const auto& spec : specs) {
    futures.push_back(std::async(
        std::launch::async,
        [&, spec] {
          return run_transfer_arm(
              spec, summary.modes[spec.orientation], summary.beta, options);
        }));
  }
  for (std::size_t index = 0; index < specs.size(); ++index) {
    summary.arms.push_back(futures[index].get());
    std::cout << "completed " << specs[index].label << std::endl;
  }

  evaluate_transfer(summary);
  write_transfer(summary);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << transfer_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "coverage=" << summary.coverage
            << " execution=" << summary.execution
            << " bounded=" << summary.bounded
            << " transfer=" << summary.transfer
            << " amplitude=" << summary.amplitude
            << " sign=" << summary.sign
            << " covariance=" << summary.covariance
            << " zero=" << summary.zero
            << " dynamic=" << summary.dynamic_morphology
            << " local=" << summary.local_morphology << '\n'
            << "doublet_ratio=" << summary.minimum_doublet_ratio
            << " dynamic_ratio=" << summary.minimum_dynamic_ratio
            << " far_fraction=" << summary.minimum_far_fraction
            << ".." << summary.maximum_far_fraction << '\n';
  return summary.verdict == "INTERNAL_MODE_ACTION_TRANSFER_EXECUTION_INVALID"
      ? 1 : 0;
}
