// FTD-0659: basis-independent action--angle phase of the first internal doublet.
#define FTD_0640_EMBEDDED
#include "test_connected_block_analytic_matter_modes.cpp"
#undef FTD_0640_EMBEDDED

namespace {

constexpr char clock_protocol_sha256[] =
    "FF9566F6D6B7BCAEB7970359043C62F643A6A8315AF43C01EE0C5CFD21ECC342";
constexpr char clock_parent_result_sha256[] =
    "AB43D342CFE48BEF452955E56B1EDC34F9EE51911F7D899932E7E542877E6B9A";
constexpr int clock_ticks = 256;
constexpr std::array<double, 3> clock_targets{{2e-6, 4e-6, 8e-6}};
constexpr std::array<double, 4> clock_quadratures{{
    0.0,
    1.57079632679489661923,
    3.14159265358979323846,
    4.71238898038468985769}};
constexpr std::array<std::array<double, 2>, 4> clock_polarizations{{
    {{1.0, 0.0}},
    {{0.0, 1.0}},
    {{0.70710678118654752440, 0.70710678118654752440}},
    {{0.70710678118654752440, -0.70710678118654752440}}}};

struct ClockSpec {
  std::string label;
  int orientation = 0;
  int polarization = 0;
  int amplitude = 0;
  int quadrature = 0;
  bool zero = false;
};

struct ClockTick {
  int tick = 0;
  double q0 = 0.0;
  double q1 = 0.0;
  double p0 = 0.0;
  double p1 = 0.0;
  double action = 0.0;
  double z_abs = 0.0;
  double support = 0.0;
  double raw_phase = NAN;
  double unwrapped_phase = NAN;
  double phase_step = NAN;
  double energy_drift = 0.0;
  double common = 0.0;
  std::vector<double> q;
  std::vector<double> p;
};

struct ClockArm {
  ClockSpec spec;
  bool initialization = false;
  bool forward = false;
  bool reverse = false;
  bool bounded = false;
  bool sector = false;
  bool no_hops = false;
  bool phase_defined = false;
  int max_multiplicity = 0;
  int total_hops = 0;
  double min_separation = INFINITY;
  double modal_amplitude = 0.0;
  double initial_action = 0.0;
  double max_action_drift = 0.0;
  double min_support = INFINITY;
  double mean_phase_step = NAN;
  double phase_step_rms = NAN;
  double phase_error = INFINITY;
  double leakage = INFINITY;
  double max_center = 0.0;
  double max_energy_drift = 0.0;
  double max_common = 0.0;
  double recovery = INFINITY;
  std::vector<ClockTick> ticks;
};

struct ClockSummary {
  bool parent = false;
  bool normalization = false;
  bool eigenspace = false;
  bool coverage = false;
  bool execution = false;
  bool bounded = false;
  bool clock = false;
  bool amplitude = false;
  bool quadrature = false;
  bool polarization = false;
  bool covariance = false;
  bool zero = false;
  double beta = 0.0;
  double stiffness_gap = 0.0;
  double doublet_split = INFINITY;
  double spectrum_covariance = INFINITY;
  double worst_common = 0.0;
  double worst_energy_drift = 0.0;
  double worst_recovery = 0.0;
  double worst_action_drift = 0.0;
  double minimum_support = INFINITY;
  double worst_phase_error = 0.0;
  double worst_phase_rms = 0.0;
  double amplitude_residual = 0.0;
  double action_scaling_residual = 0.0;
  double quadrature_residual = 0.0;
  double polarization_residual = 0.0;
  double covariance_residual = 0.0;
  double zero_action = 0.0;
  double zero_z = 0.0;
  std::array<FullModes, 2> modes;
  std::vector<ClockArm> arms;
  std::string verdict = "NATIVE_EXCITED_MATTER_CLOCK_EXECUTION_INVALID";
};

double wrap_near(double value, double target) {
  const double pi = std::acos(-1.0);
  const double tau = 2.0 * pi;
  while (value - target > pi) value -= tau;
  while (value - target <= -pi) value += tau;
  return value;
}

std::vector<double> project_momenta(
    const ftd::eft::ConnectedMooreBlockState& state,
    const FullModes& modes) {
  std::vector<double> projected(N, 0.0);
  for (int mode = 0; mode < N; ++mode) {
    for (int particle = 0; particle < count; ++particle) {
      for (int axis = 0; axis < 3; ++axis) {
        projected[mode] += modes.modes[mode].vector[3 * particle + axis]
            * component(state.constituents[particle].momentum, axis);
      }
    }
  }
  return projected;
}

double combined_component(const FullModes& modes,
                          const std::array<double, 2>& polarization,
                          int coordinate) {
  return polarization[0] * modes.modes[6].vector[coordinate]
      + polarization[1] * modes.modes[7].vector[coordinate];
}

ftd::eft::ConnectedMooreBlockState prepare_clock_state(
    const ClockSpec& spec,
    const FullModes& modes,
    double omega,
    double& modal_amplitude,
    double& q_error,
    double& p_error) {
  const auto reference = load_refined_state(spec.orientation);
  if (reference.electric.L != L) {
    return ftd::eft::ConnectedMooreBlockState{};
  }
  if (spec.zero) {
    modal_amplitude = 0.0;
    q_error = p_error = 0.0;
    return reference;
  }

  const auto& polarization = clock_polarizations[spec.polarization];
  double maximum_component = 0.0;
  for (int coordinate = 0; coordinate < N; ++coordinate) {
    maximum_component = std::max(
        maximum_component,
        std::abs(combined_component(modes, polarization, coordinate)));
  }
  modal_amplitude = clock_targets[spec.amplitude] / maximum_component;
  const double theta = clock_quadratures[spec.quadrature];
  const double q_amplitude = modal_amplitude * std::cos(theta);
  const double p_amplitude = -omega * modal_amplitude * std::sin(theta);

  auto geometry = reference;
  for (int particle = 0; particle < count; ++particle) {
    Vec3 x = position(reference.constituents[particle]);
    for (int axis = 0; axis < 3; ++axis) {
      const int coordinate = 3 * particle + axis;
      set_component(
          x, axis, component(x, axis)
              + q_amplitude
                  * combined_component(modes, polarization, coordinate));
    }
    geometry.constituents[particle] = point_at(x);
    geometry.constituents[particle].momentum = {};
  }
  const auto dressed = ftd::eft::redress_connected_moore_block_with_fibre_limit(
      geometry, 8, 1e-13, 4096);
  if (!dressed.valid) return ftd::eft::ConnectedMooreBlockState{};
  auto state = dressed.state;
  for (int particle = 0; particle < count; ++particle) {
    for (int axis = 0; axis < 3; ++axis) {
      const int coordinate = 3 * particle + axis;
      set_component(
          state.constituents[particle].momentum,
          axis,
          ftd::M_INERTIAL * p_amplitude
              * combined_component(modes, polarization, coordinate));
    }
  }

  const auto q = project_modes(reference, state, modes);
  const auto p = project_momenta(state, modes);
  q_error = std::hypot(q[6] - q_amplitude * polarization[0],
                       q[7] - q_amplitude * polarization[1]);
  p_error = std::hypot(p[6] - p_amplitude * polarization[0],
                       p[7] - p_amplitude * polarization[1]);
  return state;
}

ClockTick observe_clock(
    int tick,
    const ftd::eft::ConnectedMooreBlockState& reference,
    const ftd::eft::ConnectedMooreBlockState& state,
    const FullModes& modes,
    double omega,
    double energy_drift,
    double common,
    double previous_raw,
    double previous_unwrapped,
    double expected_step) {
  ClockTick record;
  record.tick = tick;
  record.q = project_modes(reference, state, modes);
  record.p = project_momenta(state, modes);
  record.q0 = record.q[6];
  record.q1 = record.q[7];
  record.p0 = record.p[6];
  record.p1 = record.p[7];
  const double q2 = record.q0 * record.q0 + record.q1 * record.q1;
  const double p2 = record.p0 * record.p0 + record.p1 * record.p1;
  const double qp = record.q0 * record.p0 + record.q1 * record.p1;
  record.action = (p2 + omega * omega * q2) / (2.0 * omega);
  const double z_real = omega * omega * q2 - p2;
  const double z_imag = -2.0 * omega * qp;
  record.z_abs = std::hypot(z_real, z_imag);
  record.support = record.action > 0.0
      ? record.z_abs / (2.0 * omega * record.action)
      : 0.0;
  if (record.z_abs > 1e-30) {
    record.raw_phase = std::atan2(z_imag, z_real);
    if (tick == 0 || !std::isfinite(previous_raw)) {
      record.unwrapped_phase = record.raw_phase;
    } else {
      record.phase_step = wrap_near(
          record.raw_phase - previous_raw, expected_step);
      record.unwrapped_phase = previous_unwrapped + record.phase_step;
    }
  }
  record.energy_drift = energy_drift;
  record.common = common;
  return record;
}

double clock_leakage(const ClockArm& arm, const FullModes& modes) {
  std::map<int, long double> group_norm;
  for (const auto& record : arm.ticks) {
    for (int mode = 0; mode < N; ++mode) {
      const double omega = modes.modes[mode].omega;
      group_norm[modes.modes[mode].group] +=
          static_cast<long double>(record.q[mode]) * record.q[mode]
          + static_cast<long double>(record.p[mode]) * record.p[mode]
              / (omega * omega);
    }
  }
  const int target_group = modes.modes[6].group;
  const long double target = group_norm[target_group];
  if (!(target > 0.0)) return INFINITY;
  long double leakage = 0.0;
  for (const auto& entry : group_norm) {
    if (entry.first != target_group) leakage = std::max(leakage, entry.second);
  }
  return std::sqrt(static_cast<double>(leakage / target));
}

ClockArm run_clock_arm(
    const ClockSpec& spec,
    const FullModes& modes,
    double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  ClockArm arm;
  arm.spec = spec;
  const auto reference = load_refined_state(spec.orientation);
  if (reference.electric.L != L) return arm;
  const double omega = 0.5 * (modes.modes[6].omega + modes.modes[7].omega);
  const double expected_step = 2.0 * modes.modes[6].phase;
  double q_error = INFINITY;
  double p_error = INFINITY;
  auto initial = prepare_clock_state(
      spec, modes, omega, arm.modal_amplitude, q_error, p_error);
  arm.initialization = initial.electric.L == L
      && q_error <= 1e-12 && p_error <= 1e-12;
  if (!arm.initialization) return arm;

  const auto initial_sector = sector_signature(initial);
  const double initial_energy = mode_energy(initial, beta, options);
  auto state = initial;
  arm.sector = true;
  arm.forward = true;
  arm.ticks.push_back(observe_clock(
      0, reference, state, modes, omega, 0.0, 0.0,
      NAN, NAN, expected_step));
  arm.initial_action = arm.ticks.front().action;
  ftd::eft::ConnectedMooreBlockSolveCache forward_cache, reverse_cache;

  for (int tick = 1; tick <= clock_ticks; ++tick) {
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
    const double drift = std::abs(
        mode_energy(state, beta, options) - initial_energy);
    const auto& previous = arm.ticks.back();
    arm.ticks.push_back(observe_clock(
        tick, reference, state, modes, omega, drift, residual,
        previous.raw_phase, previous.unwrapped_phase, expected_step));
    arm.max_center = std::max(
        arm.max_center, (center(state) - center(reference)).mag());
    arm.max_energy_drift = std::max(arm.max_energy_drift, drift);
    arm.max_common = std::max(arm.max_common, residual);
  }
  arm.forward = arm.forward && arm.ticks.size() == clock_ticks + 1;

  arm.reverse = arm.forward;
  for (int tick = clock_ticks; arm.reverse && tick >= 1; --tick) {
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

  if (!spec.zero && arm.forward) {
    arm.phase_defined = std::all_of(
        arm.ticks.begin(), arm.ticks.end(),
        [](const ClockTick& tick) {
          return std::isfinite(tick.unwrapped_phase);
        });
    long double step_sum = 0.0;
    int step_count = 0;
    for (const auto& record : arm.ticks) {
      arm.max_action_drift = std::max(
          arm.max_action_drift,
          std::abs(record.action - arm.initial_action)
              / std::max(arm.initial_action, 1e-300));
      arm.min_support = std::min(arm.min_support, record.support);
      if (std::isfinite(record.phase_step)) {
        step_sum += record.phase_step;
        ++step_count;
      }
    }
    if (step_count > 0) {
      arm.mean_phase_step = static_cast<double>(step_sum / step_count);
      long double variance = 0.0;
      for (const auto& record : arm.ticks) {
        if (std::isfinite(record.phase_step)) {
          const double delta = record.phase_step - arm.mean_phase_step;
          variance += static_cast<long double>(delta) * delta;
        }
      }
      arm.phase_step_rms = std::sqrt(
          static_cast<double>(variance / step_count));
      arm.phase_error = std::abs(arm.mean_phase_step - expected_step)
          / expected_step;
    }
    arm.leakage = clock_leakage(arm, modes);
  }

  arm.bounded = arm.forward && arm.reverse && arm.sector && arm.no_hops
      && arm.max_multiplicity <= 8
      && (!std::isfinite(arm.min_separation) || arm.min_separation >= 0.9)
      && arm.max_center <= 1e-4 && arm.max_energy_drift <= 1e-12
      && arm.max_common <= 1e-10 && arm.recovery <= 1e-10;
  return arm;
}

const ClockArm* find_clock_arm(
    const ClockSummary& summary,
    int orientation,
    int polarization,
    int amplitude,
    int quadrature) {
  for (const auto& arm : summary.arms) {
    if (!arm.spec.zero && arm.spec.orientation == orientation
        && arm.spec.polarization == polarization
        && arm.spec.amplitude == amplitude
        && arm.spec.quadrature == quadrature) {
      return &arm;
    }
  }
  return nullptr;
}

double phase_history_rms(const ClockArm& a, const ClockArm& b) {
  if (a.ticks.size() != b.ticks.size() || a.ticks.empty()) return INFINITY;
  const double a0 = a.ticks.front().unwrapped_phase;
  const double b0 = b.ticks.front().unwrapped_phase;
  long double sum = 0.0;
  int sample_count = 0;
  for (std::size_t tick = 0; tick < a.ticks.size(); ++tick) {
    if (!std::isfinite(a.ticks[tick].unwrapped_phase)
        || !std::isfinite(b.ticks[tick].unwrapped_phase)) {
      return INFINITY;
    }
    const double difference =
        (a.ticks[tick].unwrapped_phase - a0)
        - (b.ticks[tick].unwrapped_phase - b0);
    sum += static_cast<long double>(difference) * difference;
    ++sample_count;
  }
  return std::sqrt(static_cast<double>(sum / sample_count));
}

void evaluate_clock(ClockSummary& summary) {
  summary.coverage = summary.arms.size() == 74;
  summary.execution = summary.coverage && std::all_of(
      summary.arms.begin(), summary.arms.end(),
      [](const ClockArm& arm) {
        return arm.initialization && arm.forward && arm.reverse;
      });
  summary.bounded = summary.execution && std::all_of(
      summary.arms.begin(), summary.arms.end(),
      [](const ClockArm& arm) { return arm.bounded; });
  summary.clock = summary.bounded;
  summary.minimum_support = INFINITY;
  for (const auto& arm : summary.arms) {
    summary.worst_common = std::max(summary.worst_common, arm.max_common);
    summary.worst_energy_drift = std::max(
        summary.worst_energy_drift, arm.max_energy_drift);
    if (std::isfinite(arm.recovery)) {
      summary.worst_recovery = std::max(summary.worst_recovery, arm.recovery);
    }
    if (arm.spec.zero) {
      for (const auto& record : arm.ticks) {
        summary.zero_action = std::max(summary.zero_action, record.action);
        summary.zero_z = std::max(summary.zero_z, record.z_abs);
      }
      continue;
    }
    summary.worst_action_drift = std::max(
        summary.worst_action_drift, arm.max_action_drift);
    summary.minimum_support = std::min(summary.minimum_support, arm.min_support);
    summary.worst_phase_error = std::max(
        summary.worst_phase_error, arm.phase_error);
    summary.worst_phase_rms = std::max(
        summary.worst_phase_rms, arm.phase_step_rms);
    summary.clock = summary.clock && arm.phase_defined
        && arm.max_action_drift <= 0.02 && arm.min_support >= 0.90
        && arm.phase_error <= 0.02 && arm.phase_step_rms <= 0.05
        && arm.leakage <= 0.10;
  }

  summary.amplitude = summary.execution;
  for (int orientation = 0; orientation < 2; ++orientation) {
    for (int polarization = 0; polarization < 3; ++polarization) {
      for (int quadrature = 0; quadrature < 4; ++quadrature) {
        const auto* base = find_clock_arm(
            summary, orientation, polarization, 0, quadrature);
        if (!base) {
          summary.amplitude = false;
          continue;
        }
        for (int amplitude = 1; amplitude < 3; ++amplitude) {
          const auto* arm = find_clock_arm(
              summary, orientation, polarization, amplitude, quadrature);
          if (!arm) {
            summary.amplitude = false;
            continue;
          }
          const double phase_residual = relative_value(
              base->mean_phase_step, arm->mean_phase_step);
          const double target_ratio = std::pow(
              clock_targets[amplitude] / clock_targets[0], 2);
          const double action_residual = std::abs(
              arm->initial_action / base->initial_action / target_ratio - 1.0);
          summary.amplitude_residual = std::max(
              summary.amplitude_residual, phase_residual);
          summary.action_scaling_residual = std::max(
              summary.action_scaling_residual, action_residual);
          summary.amplitude = summary.amplitude
              && phase_residual <= 0.005 && action_residual <= 0.02;
        }
      }
    }
  }

  summary.quadrature = summary.execution;
  for (int orientation = 0; orientation < 2; ++orientation) {
    for (int polarization = 0; polarization < 3; ++polarization) {
      for (int amplitude = 0; amplitude < 3; ++amplitude) {
        const auto* base = find_clock_arm(
            summary, orientation, polarization, amplitude, 0);
        for (int quadrature = 1; quadrature < 4; ++quadrature) {
          const auto* arm = find_clock_arm(
              summary, orientation, polarization, amplitude, quadrature);
          const double residual = base && arm
              ? phase_history_rms(*base, *arm) : INFINITY;
          summary.quadrature_residual = std::max(
              summary.quadrature_residual, residual);
          summary.quadrature = summary.quadrature && residual <= 0.05;
        }
      }
    }
  }

  summary.polarization = summary.execution;
  for (int orientation = 0; orientation < 2; ++orientation) {
    for (int amplitude = 0; amplitude < 3; ++amplitude) {
      for (int quadrature = 0; quadrature < 4; ++quadrature) {
        const auto* base = find_clock_arm(
            summary, orientation, 0, amplitude, quadrature);
        for (int polarization = 1; polarization < 3; ++polarization) {
          const auto* arm = find_clock_arm(
              summary, orientation, polarization, amplitude, quadrature);
          const double residual = base && arm
              ? relative_value(base->mean_phase_step, arm->mean_phase_step)
              : INFINITY;
          summary.polarization_residual = std::max(
              summary.polarization_residual, residual);
          summary.polarization = summary.polarization && residual <= 0.005;
        }
      }
    }
  }

  summary.covariance = summary.execution;
  for (int polarization = 0; polarization < 3; ++polarization) {
    for (int amplitude = 0; amplitude < 3; ++amplitude) {
      for (int quadrature = 0; quadrature < 4; ++quadrature) {
        const auto* x = find_clock_arm(
            summary, 0, polarization, amplitude, quadrature);
        const auto* y = find_clock_arm(
            summary, 1, polarization, amplitude, quadrature);
        const double residual = x && y
            ? relative_value(x->mean_phase_step, y->mean_phase_step)
            : INFINITY;
        summary.covariance_residual = std::max(
            summary.covariance_residual, residual);
        summary.covariance = summary.covariance && residual <= 0.005;
      }
    }
  }
  summary.covariance = summary.covariance
      && summary.spectrum_covariance <= 1e-9;
  summary.zero = summary.execution && summary.zero_action <= 1e-20
      && summary.zero_z <= 1e-20;

  if (!summary.parent || !summary.normalization || !summary.eigenspace
      || !summary.coverage || !summary.execution) {
    summary.verdict = "NATIVE_EXCITED_MATTER_CLOCK_EXECUTION_INVALID";
  } else if (!summary.bounded) {
    summary.verdict = "NATIVE_EXCITED_MATTER_CLOCK_CLOSED_NEGATIVE";
  } else if (summary.clock && summary.amplitude && summary.quadrature
             && summary.polarization && summary.covariance && summary.zero) {
    summary.verdict = "NATIVE_EXCITED_MATTER_CLOCK_CONSTRUCTIVE";
  } else {
    summary.verdict = "NATIVE_EXCITED_MATTER_CLOCK_MIXED";
  }
}

void write_clock(const ClockSummary& summary) {
  const auto directory = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0659";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory / "ftd_0659_native_excited_matter_clock_v1.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0659\",\n"
       << "  \"protocol_sha256\": \"" << clock_protocol_sha256 << "\",\n"
       << "  \"parent_result_sha256\": \"" << clock_parent_result_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"arm_count\": " << summary.arms.size() << ",\n"
       << "  \"ticks_each_direction\": " << clock_ticks << ",\n"
       << "  \"eigenspace_pass\": " << summary.eigenspace << ",\n"
       << "  \"coverage_pass\": " << summary.coverage << ",\n"
       << "  \"execution_pass\": " << summary.execution << ",\n"
       << "  \"bounded_pass\": " << summary.bounded << ",\n"
       << "  \"clock_pass\": " << summary.clock << ",\n"
       << "  \"amplitude_pass\": " << summary.amplitude << ",\n"
       << "  \"quadrature_pass\": " << summary.quadrature << ",\n"
       << "  \"polarization_pass\": " << summary.polarization << ",\n"
       << "  \"covariance_pass\": " << summary.covariance << ",\n"
       << "  \"zero_control_pass\": " << summary.zero << ",\n"
       << "  \"stiffness_gap\": " << summary.stiffness_gap << ",\n"
       << "  \"doublet_split\": " << summary.doublet_split << ",\n"
       << "  \"spectrum_covariance\": " << summary.spectrum_covariance << ",\n"
       << "  \"worst_common_residual\": " << summary.worst_common << ",\n"
       << "  \"worst_energy_drift\": " << summary.worst_energy_drift << ",\n"
       << "  \"worst_recovery\": " << summary.worst_recovery << ",\n"
       << "  \"worst_action_drift\": " << summary.worst_action_drift << ",\n"
       << "  \"minimum_support\": " << summary.minimum_support << ",\n"
       << "  \"worst_phase_error\": " << summary.worst_phase_error << ",\n"
       << "  \"worst_phase_rms\": " << summary.worst_phase_rms << ",\n"
       << "  \"amplitude_residual\": " << summary.amplitude_residual << ",\n"
       << "  \"action_scaling_residual\": " << summary.action_scaling_residual << ",\n"
       << "  \"quadrature_residual\": " << summary.quadrature_residual << ",\n"
       << "  \"polarization_residual\": " << summary.polarization_residual << ",\n"
       << "  \"covariance_residual\": " << summary.covariance_residual << ",\n"
       << "  \"zero_action\": " << summary.zero_action << ",\n"
       << "  \"zero_z\": " << summary.zero_z << "\n}\n";

  std::ofstream arms(directory / "ftd_0659_native_excited_matter_clock_arms_v1.csv");
  arms << "ftd_id,label,orientation,polarization,amplitude,quadrature,zero,"
          "initialization,forward,reverse,bounded,phase_defined,modal_amplitude,"
          "initial_action,max_action_drift,min_support,mean_phase_step,"
          "phase_step_rms,phase_error,leakage,max_center,max_energy_drift,"
          "max_common,recovery\n";
  for (const auto& arm : summary.arms) {
    arms << std::setprecision(17) << "FTD-0659," << arm.spec.label << ','
         << arm.spec.orientation << ',' << arm.spec.polarization << ','
         << arm.spec.amplitude << ',' << arm.spec.quadrature << ','
         << arm.spec.zero << ',' << arm.initialization << ',' << arm.forward
         << ',' << arm.reverse << ',' << arm.bounded << ',' << arm.phase_defined
         << ',' << arm.modal_amplitude << ',' << arm.initial_action << ','
         << arm.max_action_drift << ',' << arm.min_support << ','
         << arm.mean_phase_step << ',' << arm.phase_step_rms << ','
         << arm.phase_error << ',' << arm.leakage << ',' << arm.max_center << ','
         << arm.max_energy_drift << ',' << arm.max_common << ',' << arm.recovery
         << '\n';
  }

  std::ofstream ticks(directory / "ftd_0659_native_excited_matter_clock_ticks_v1.csv");
  ticks << "ftd_id,label,tick,q0,q1,p0,p1,action,z_abs,support,raw_phase,"
           "unwrapped_phase,phase_step,energy_drift,common\n";
  for (const auto& arm : summary.arms) {
    for (const auto& record : arm.ticks) {
      ticks << std::setprecision(17) << "FTD-0659," << arm.spec.label << ','
            << record.tick << ',' << record.q0 << ',' << record.q1 << ','
            << record.p0 << ',' << record.p1 << ',' << record.action << ','
            << record.z_abs << ',' << record.support << ',' << record.raw_phase
            << ',' << record.unwrapped_phase << ',' << record.phase_step << ','
            << record.energy_drift << ',' << record.common << '\n';
    }
  }
}

}  // namespace

#ifdef FTD_0659_EMBEDDED
int ftd_0659_embedded_main() {
#else
int main() {
#endif
  ClockSummary summary;
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
          orientation == 0 ? "clock_x" : "clock_y",
          orientation, state, summary.beta, options);
      if (analytic.valid) {
        summary.modes[orientation] = full_modes(analytic.hessian);
      }
    }
    if (summary.modes[0].valid && summary.modes[1].valid) {
      summary.stiffness_gap = summary.modes[0].modes[6].hessian_eigen
          / summary.modes[0].modes[5].hessian_eigen;
      summary.doublet_split = relative_value(
          summary.modes[0].modes[6].hessian_eigen,
          summary.modes[0].modes[7].hessian_eigen);
      summary.spectrum_covariance = std::max(
          relative_value(summary.modes[0].modes[6].hessian_eigen,
                         summary.modes[1].modes[6].hessian_eigen),
          relative_value(summary.modes[0].modes[7].hessian_eigen,
                         summary.modes[1].modes[7].hessian_eigen));
      summary.eigenspace = summary.modes[0].modes[6].group
              == summary.modes[0].modes[7].group
          && summary.modes[0].modes[5].group
              != summary.modes[0].modes[6].group
          && summary.stiffness_gap > 100.0
          && summary.doublet_split <= 1e-9
          && summary.spectrum_covariance <= 1e-9;
    }
  }

  std::vector<ClockSpec> specs;
  if (summary.eigenspace) {
    for (int orientation = 0; orientation < 2; ++orientation) {
      for (int polarization = 0; polarization < 3; ++polarization) {
        for (int amplitude = 0; amplitude < 3; ++amplitude) {
          for (int quadrature = 0; quadrature < 4; ++quadrature) {
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

  constexpr std::size_t batch = 24;
  for (std::size_t start = 0; start < specs.size(); start += batch) {
    std::vector<std::future<ClockArm>> futures;
    const auto end = std::min(specs.size(), start + batch);
    for (std::size_t index = start; index < end; ++index) {
      futures.push_back(std::async(
          std::launch::async,
          [&, spec = specs[index]] {
            return run_clock_arm(
                spec, summary.modes[spec.orientation], summary.beta, options);
          }));
    }
    for (std::size_t index = start; index < end; ++index) {
      summary.arms.push_back(futures[index - start].get());
      std::cout << "completed " << specs[index].label << std::endl;
    }
  }

  evaluate_clock(summary);
  write_clock(summary);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << clock_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "eigenspace=" << summary.eigenspace
            << " coverage=" << summary.coverage
            << " execution=" << summary.execution
            << " bounded=" << summary.bounded
            << " clock=" << summary.clock
            << " amplitude=" << summary.amplitude
            << " quadrature=" << summary.quadrature
            << " polarization=" << summary.polarization
            << " covariance=" << summary.covariance
            << " zero=" << summary.zero << '\n'
            << "phase_error=" << summary.worst_phase_error
            << " phase_rms=" << summary.worst_phase_rms
            << " action_drift=" << summary.worst_action_drift
            << " support=" << summary.minimum_support
            << " quadrature_residual=" << summary.quadrature_residual << '\n';
  return summary.verdict == "NATIVE_EXCITED_MATTER_CLOCK_EXECUTION_INVALID"
      ? 1 : 0;
}
