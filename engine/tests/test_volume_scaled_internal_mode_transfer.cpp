// FTD-0664: volume-scaled pre-return transfer of the first internal doublet.
#define FTD_0661_EMBEDDED
#include "test_internal_mode_action_transfer_v2.cpp"
#undef FTD_0661_EMBEDDED

namespace {

constexpr char volume_protocol_sha256[] =
    "B6C7E2632884FA6CC98499D42EE6E4CE1AE790C9B6261E034278ABABB2FFB933";
constexpr char volume_parent_protocol_sha256[] =
    "F517C08CB66B6AE2388CBE3C04E1EE5429C4B596723C41732A443649C542136D";
constexpr std::array<int, 3> volume_sizes{{17, 25, 33}};
constexpr int pre_return_tick = 16;

struct VolumeTick {
  int tick = 0;
  double doublet_ratio = 0.0;
  double dynamic_energy_ratio = 0.0;
  double dynamic_norm_ratio = 0.0;
  double radius_second_moment = 0.0;
  double decomposition_residual = 0.0;
  double energy_drift = 0.0;
  double common_residual = 0.0;
};

struct VolumeArm {
  int volume = 0;
  int sign = 0;
  bool initialized = false;
  bool executed = false;
  bool redressed = false;
  bool sector = false;
  int return_tick = -1;
  double initial_doublet = 0.0;
  double recovery = INFINITY;
  double max_energy_drift = 0.0;
  double max_common = 0.0;
  double max_decomposition = 0.0;
  std::vector<VolumeTick> ticks;
};

struct VolumeSummary {
  bool parent = false;
  bool normalization = false;
  bool eigenspace = false;
  bool execution = false;
  bool locality = false;
  bool emission = false;
  bool outward = false;
  double beta = 0.0;
  double locality_residual = INFINITY;
  double return_scaled_cv = INFINITY;
  std::string return_classification = "MIXED_RETURN";
  std::string verdict = "VOLUME_SCALED_INTERNAL_TRANSFER_EXECUTION_INVALID";
  FullModes modes;
  std::vector<VolumeArm> arms;
};

int wrap_volume(int value, int size) {
  const int result = value % size;
  return result < 0 ? result + size : result;
}

ftd::eft::MatchedMatterPoint point_at_volume(const Vec3& x, int size) {
  ftd::eft::MatchedMatterPoint point;
  const long long ax = std::llround(x.x);
  const long long ay = std::llround(x.y);
  const long long az = std::llround(x.z);
  point.anchor = {wrap_volume(static_cast<int>(ax), size),
                  wrap_volume(static_cast<int>(ay), size),
                  wrap_volume(static_cast<int>(az), size)};
  point.remainder = {x.x - ax, x.y - ay, x.z - az};
  return point;
}

double periodic_delta(double value, int size) {
  while (value > 0.5 * size) value -= size;
  while (value < -0.5 * size) value += size;
  return value;
}

bool volume_parent_fingerprint() {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0662/ftd_0662_internal_mode_action_transfer_v3.json";
  std::ifstream input(path, std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)), {});
  return bytes.find(volume_parent_protocol_sha256) != std::string::npos
      && bytes.find("INTERNAL_MODE_DYNAMIC_FIELD_TRANSFER_CONSTRUCTIVE")
          != std::string::npos;
}

ftd::eft::ConnectedMooreBlockState volume_reference(int size) {
  const auto base = load_refined_state(0);
  const auto initialized = ftd::eft::initialize_connected_moore_block(
      size, 2, 0, 0, 0.5, 1e-13, 4096);
  if (base.electric.L != L || !initialized.valid) {
    return ftd::eft::ConnectedMooreBlockState{};
  }
  auto geometry = initialized.state;
  const Vec3 base_center = center(base);
  const Vec3 target_center = center(geometry);
  for (int particle = 0; particle < count; ++particle) {
    const Vec3 x = target_center + (position(base.constituents[particle])
                                    - base_center);
    geometry.constituents[particle] = point_at_volume(x, size);
    geometry.constituents[particle].momentum = {};
  }
  const auto dressed = ftd::eft::redress_connected_moore_block_with_fibre_limit(
      geometry, 8, 1e-13, 4096);
  return dressed.valid ? dressed.state
                       : ftd::eft::ConnectedMooreBlockState{};
}

ftd::eft::ConnectedMooreBlockState volume_excitation(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const FullModes& modes,
    int sign,
    double& initial_doublet) {
  const double omega = 0.5 * (modes.modes[6].omega + modes.modes[7].omega);
  double maximum_component = 0.0;
  for (int coordinate = 0; coordinate < N; ++coordinate) {
    maximum_component = std::max(
        maximum_component, std::abs(modes.modes[6].vector[coordinate]));
  }
  const double amplitude = 8e-6 / maximum_component;
  auto geometry = reference;
  for (int particle = 0; particle < count; ++particle) {
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
          state.constituents[particle].momentum, axis,
          sign * ftd::M_INERTIAL * omega * amplitude
              * modes.modes[6].vector[coordinate]);
    }
  }
  const double modal_momentum = omega * amplitude;
  initial_doublet = 0.5 * modal_momentum * modal_momentum;
  return state;
}

std::array<std::vector<double>, 2> paired_modal_coordinates(
    const ftd::eft::ConnectedMooreBlockState& control,
    const ftd::eft::ConnectedMooreBlockState& excited,
    const FullModes& modes) {
  std::array<std::vector<double>, 2> result{{
      std::vector<double>(N, 0.0), std::vector<double>(N, 0.0)}};
  const int size = control.electric.L;
  for (int mode = 0; mode < N; ++mode) {
    for (int particle = 0; particle < count; ++particle) {
      const Vec3 delta_x = position(excited.constituents[particle])
          - position(control.constituents[particle]);
      const Vec3 delta_p = excited.constituents[particle].momentum
          - control.constituents[particle].momentum;
      for (int axis = 0; axis < 3; ++axis) {
        const int coordinate = 3 * particle + axis;
        result[0][mode] += modes.modes[mode].vector[coordinate]
            * ftd::M_INERTIAL
            * periodic_delta(component(delta_x, axis), size);
        result[1][mode] += modes.modes[mode].vector[coordinate]
            * component(delta_p, axis);
      }
    }
  }
  return result;
}

ftd::eft::MatchedEdgeField subtract_edge(
    const ftd::eft::MatchedEdgeField& a,
    const ftd::eft::MatchedEdgeField& b) {
  ftd::eft::MatchedEdgeField result(a.L);
  for (std::size_t index = 0; index < a.x.size(); ++index) {
    result.x[index] = a.x[index] - b.x[index];
    result.y[index] = a.y[index] - b.y[index];
    result.z[index] = a.z[index] - b.z[index];
  }
  return result;
}

double state_decomposition_residual(
    const ftd::eft::ConnectedMooreBlockState& state,
    const ftd::eft::ConnectedMooreBlockState& dressed,
    double beta) {
  const auto residual_e = subtract_face(state.electric, dressed.electric);
  const auto residual_b = subtract_edge(
      state.magnetic_half, dressed.magnetic_half);
  const double actual = beta * ftd::eft::matched_modified_energy(
      state.electric, state.magnetic_half, ftd::C_SPEED);
  const double dressing = beta * ftd::eft::matched_modified_energy(
      dressed.electric, dressed.magnetic_half, ftd::C_SPEED);
  const double residual = beta * ftd::eft::matched_modified_energy(
      residual_e, residual_b, ftd::C_SPEED);
  const auto curl_dressing = ftd::eft::matched_curl_adjoint(dressed.electric);
  const double interference = beta * static_cast<double>(
      ftd::eft::matched_face_dot(dressed.electric, residual_e)
      - 0.5L * ftd::C_SPEED
          * ftd::eft::matched_edge_dot(residual_b, curl_dressing));
  return std::abs(actual - dressing - residual - interference);
}

VolumeTick observe_volume(
    int tick,
    const ftd::eft::ConnectedMooreBlockState& control,
    const ftd::eft::ConnectedMooreBlockState& excited,
    const FullModes& modes,
    double initial_doublet,
    double beta,
    double energy_drift,
    double common) {
  VolumeTick record;
  record.tick = tick;
  const auto modal = paired_modal_coordinates(control, excited, modes);
  const double omega = 0.5 * (modes.modes[6].omega + modes.modes[7].omega);
  const double q2 = modal[0][6] * modal[0][6]
      + modal[0][7] * modal[0][7];
  const double p2 = modal[1][6] * modal[1][6]
      + modal[1][7] * modal[1][7];
  const double doublet = 0.5 * (p2 + omega * omega * q2);
  record.doublet_ratio = initial_doublet > 0.0
      ? doublet / initial_doublet : 0.0;

  auto control_geometry = control;
  auto excited_geometry = excited;
  const auto control_dressed =
      ftd::eft::redress_connected_moore_block_with_fibre_limit(
          control_geometry, 8, 1e-13, 4096);
  const auto excited_dressed =
      ftd::eft::redress_connected_moore_block_with_fibre_limit(
          excited_geometry, 8, 1e-13, 4096);
  if (!control_dressed.valid || !excited_dressed.valid) {
    record.decomposition_residual = INFINITY;
    return record;
  }
  const auto control_re = subtract_face(
      control.electric, control_dressed.state.electric);
  const auto excited_re = subtract_face(
      excited.electric, excited_dressed.state.electric);
  const auto dynamic_e = subtract_face(excited_re, control_re);
  const auto control_rb = subtract_edge(
      control.magnetic_half, control_dressed.state.magnetic_half);
  const auto excited_rb = subtract_edge(
      excited.magnetic_half, excited_dressed.state.magnetic_half);
  const auto dynamic_b = subtract_edge(excited_rb, control_rb);

  const double dynamic_energy = beta * ftd::eft::matched_modified_energy(
      dynamic_e, dynamic_b, ftd::C_SPEED);
  record.dynamic_energy_ratio = initial_doublet > 0.0
      ? dynamic_energy / initial_doublet : 0.0;
  long double norm = 0.0;
  long double radius2 = 0.0;
  const Vec3 object_center = center(control);
  const int size = dynamic_e.L;
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      for (int z = 0; z < size; ++z) {
        const std::size_t index = static_cast<std::size_t>(
            (x * size + y) * size + z);
        const long double density = 0.5L * beta * (
            static_cast<long double>(dynamic_e.x[index]) * dynamic_e.x[index]
            + static_cast<long double>(dynamic_e.y[index]) * dynamic_e.y[index]
            + static_cast<long double>(dynamic_e.z[index]) * dynamic_e.z[index]
            + static_cast<long double>(dynamic_b.x[index]) * dynamic_b.x[index]
            + static_cast<long double>(dynamic_b.y[index]) * dynamic_b.y[index]
            + static_cast<long double>(dynamic_b.z[index]) * dynamic_b.z[index]);
        const double dx = periodic_distance(x, object_center.x, size);
        const double dy = periodic_distance(y, object_center.y, size);
        const double dz = periodic_distance(z, object_center.z, size);
        const double r2 = dx * dx + dy * dy + dz * dz;
        norm += density;
        radius2 += density * r2;
      }
    }
  }
  record.dynamic_norm_ratio = initial_doublet > 0.0
      ? static_cast<double>(norm) / initial_doublet : 0.0;
  record.radius_second_moment = norm > 0.0
      ? static_cast<double>(radius2 / norm) : 0.0;
  record.decomposition_residual = std::max(
      state_decomposition_residual(control, control_dressed.state, beta),
      state_decomposition_residual(excited, excited_dressed.state, beta));
  record.energy_drift = energy_drift;
  record.common_residual = common;
  return record;
}

std::array<VolumeArm, 2> run_volume(
    int size,
    const FullModes& modes,
    double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  std::array<VolumeArm, 2> arms;
  for (int sign = 0; sign < 2; ++sign) {
    arms[sign].volume = size;
    arms[sign].sign = sign == 0 ? -1 : 1;
  }
  const auto control_initial = volume_reference(size);
  std::array<ftd::eft::ConnectedMooreBlockState, 3> initial;
  initial[0] = control_initial;
  for (int sign = 0; sign < 2; ++sign) {
    initial[sign + 1] = volume_excitation(
        control_initial, modes, arms[sign].sign, arms[sign].initial_doublet);
    arms[sign].initialized = control_initial.electric.L == size
        && initial[sign + 1].electric.L == size
        && arms[sign].initial_doublet > 0.0;
  }
  if (!arms[0].initialized || !arms[1].initialized) return arms;

  std::array<ftd::eft::ConnectedMooreBlockState, 3> state = initial;
  std::array<double, 3> initial_energy{};
  for (int path = 0; path < 3; ++path) {
    initial_energy[path] = energy_parts(state[path], beta, options).total;
  }
  const std::array<std::vector<int>, 3> initial_sector{{
      sector_signature(state[0]), sector_signature(state[1]),
      sector_signature(state[2])}};
  for (auto& arm : arms) {
    arm.sector = true;
    arm.redressed = true;
    arm.ticks.push_back(observe_volume(
        0, state[0], state[arm.sign < 0 ? 1 : 2], modes,
        arm.initial_doublet, beta, 0.0, 0.0));
  }

  const int horizon = 4 * size;
  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> forward_cache;
  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> reverse_cache;
  bool forward = true;
  for (int tick = 1; tick <= horizon && forward; ++tick) {
    double common = 0.0;
    for (int path = 0; path < 3; ++path) {
      const auto step = ftd::eft::solve_connected_moore_block_forward(
          state[path], options, &forward_cache[path]);
      const double residual = common_residual(step);
      common = std::max(common, residual);
      if (!step.valid || !step.common_action_gates_pass || residual > 1e-10) {
        forward = false;
        break;
      }
      state[path] = step.later;
      for (auto& arm : arms) {
        arm.sector = arm.sector
            && sector_signature(state[path]) == initial_sector[path];
      }
    }
    if (!forward) break;
    for (int sign = 0; sign < 2; ++sign) {
      const int path = sign + 1;
      const double drift = std::max(
          std::abs(energy_parts(state[0], beta, options).total
                   - initial_energy[0]),
          std::abs(energy_parts(state[path], beta, options).total
                   - initial_energy[path]));
      auto record = observe_volume(
          tick, state[0], state[path], modes, arms[sign].initial_doublet,
          beta, drift, common);
      arms[sign].redressed = arms[sign].redressed
          && std::isfinite(record.decomposition_residual);
      arms[sign].max_energy_drift = std::max(
          arms[sign].max_energy_drift, record.energy_drift);
      arms[sign].max_common = std::max(
          arms[sign].max_common, record.common_residual);
      arms[sign].max_decomposition = std::max(
          arms[sign].max_decomposition, record.decomposition_residual);
      arms[sign].ticks.push_back(record);
    }
  }

  bool reverse = forward;
  for (int tick = horizon; tick >= 1 && reverse; --tick) {
    for (int path = 0; path < 3; ++path) {
      const auto step = ftd::eft::solve_connected_moore_block_reverse(
          state[path], options, &reverse_cache[path]);
      const double residual = common_residual(step);
      for (auto& arm : arms) arm.max_common = std::max(arm.max_common, residual);
      if (!step.valid || !step.common_action_gates_pass || residual > 1e-10) {
        reverse = false;
        break;
      }
      state[path] = step.earlier;
    }
  }
  for (int sign = 0; sign < 2; ++sign) {
    const int path = sign + 1;
    arms[sign].recovery = std::max(
        ftd::eft::connected_moore_block_state_max_difference(
            initial[0], state[0]),
        ftd::eft::connected_moore_block_state_max_difference(
            initial[path], state[path]));
    arms[sign].executed = forward && reverse && arms[sign].redressed
        && arms[sign].sector
        && static_cast<int>(arms[sign].ticks.size()) == horizon + 1
        && arms[sign].max_common <= 1e-10
        && arms[sign].max_energy_drift <= 1e-10
        && arms[sign].max_decomposition <= 1e-10
        && arms[sign].recovery <= 1e-10;
    bool below = false;
    for (const auto& tick : arms[sign].ticks) {
      if (tick.doublet_ratio < 0.60) below = true;
      if (below && tick.tick >= size && tick.doublet_ratio > 0.80) {
        arms[sign].return_tick = tick.tick;
        break;
      }
    }
  }
  return arms;
}

const VolumeArm* find_volume_arm(
    const VolumeSummary& summary, int size, int sign) {
  for (const auto& arm : summary.arms) {
    if (arm.volume == size && arm.sign == sign) return &arm;
  }
  return nullptr;
}

void evaluate_volume(VolumeSummary& summary) {
  summary.execution = summary.parent && summary.normalization
      && summary.eigenspace && summary.arms.size() == 6
      && std::all_of(summary.arms.begin(), summary.arms.end(),
          [](const VolumeArm& arm) { return arm.executed; });
  if (!summary.execution) return;

  long double locality_sum = 0.0;
  int locality_count = 0;
  summary.emission = true;
  summary.outward = true;
  for (int sign : {-1, 1}) {
    const auto* base = find_volume_arm(summary, 17, sign);
    for (int size : {25, 33}) {
      const auto* arm = find_volume_arm(summary, size, sign);
      for (int tick = 0; tick <= pre_return_tick; ++tick) {
        for (int channel = 0; channel < 2; ++channel) {
          const double a = channel == 0
              ? base->ticks[tick].doublet_ratio
              : base->ticks[tick].dynamic_energy_ratio;
          const double b = channel == 0
              ? arm->ticks[tick].doublet_ratio
              : arm->ticks[tick].dynamic_energy_ratio;
          locality_sum += static_cast<long double>(a - b) * (a - b);
          ++locality_count;
        }
      }
    }
    for (int size : volume_sizes) {
      const auto* arm = find_volume_arm(summary, size, sign);
      const auto& early = arm->ticks[4];
      const auto& pre = arm->ticks[pre_return_tick];
      summary.emission = summary.emission
          && pre.doublet_ratio < 0.95
          && pre.dynamic_energy_ratio > 0.0;
      summary.outward = summary.outward
          && pre.radius_second_moment - early.radius_second_moment >= 4.0;
    }
  }
  summary.locality_residual = std::sqrt(
      static_cast<double>(locality_sum / std::max(1, locality_count)));
  summary.locality = summary.locality_residual <= 0.05;

  std::vector<double> scaled_returns;
  int return_count = 0;
  for (const auto& arm : summary.arms) {
    if (arm.return_tick >= 0) {
      ++return_count;
      scaled_returns.push_back(
          static_cast<double>(arm.return_tick) / arm.volume);
    }
  }
  if (return_count == static_cast<int>(summary.arms.size())) {
    const double mean = std::accumulate(
        scaled_returns.begin(), scaled_returns.end(), 0.0)
        / scaled_returns.size();
    long double variance = 0.0;
    for (double value : scaled_returns) {
      variance += static_cast<long double>(value - mean) * (value - mean);
    }
    summary.return_scaled_cv = std::sqrt(
        static_cast<double>(variance / scaled_returns.size())) / mean;
    summary.return_classification = summary.return_scaled_cv <= 0.25
        ? "SCALED_RETURN" : "MIXED_RETURN";
  } else if (return_count == 0) {
    summary.return_classification = "NO_RETURN_IN_WINDOW";
  }

  summary.verdict = summary.locality && summary.emission && summary.outward
      ? "VOLUME_SCALED_PRE_RETURN_TRANSFER_CONSTRUCTIVE"
      : "VOLUME_SCALED_INTERNAL_TRANSFER_MIXED";
}

void write_volume(const VolumeSummary& summary) {
  const auto directory = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0664";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory / "ftd_0664_volume_scaled_internal_mode_transfer_v1.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0664\",\n"
       << "  \"protocol_sha256\": \"" << volume_protocol_sha256 << "\",\n"
       << "  \"parent_protocol_sha256\": \""
       << volume_parent_protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"return_classification\": \""
       << summary.return_classification << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"arm_count\": " << summary.arms.size() << ",\n"
       << "  \"execution_pass\": " << summary.execution << ",\n"
       << "  \"locality_pass\": " << summary.locality << ",\n"
       << "  \"emission_pass\": " << summary.emission << ",\n"
       << "  \"outward_pass\": " << summary.outward << ",\n"
       << "  \"locality_rms\": " << summary.locality_residual << ",\n"
       << "  \"return_scaled_cv\": " << summary.return_scaled_cv << "\n}\n";

  std::ofstream arms(directory / "ftd_0664_volume_scaled_internal_mode_transfer_arms_v1.csv");
  arms << "ftd_id,volume,sign,initialized,executed,redressed,sector,"
          "horizon,return_tick,initial_doublet,max_energy_drift,max_common,"
          "max_decomposition,recovery,pre_doublet_ratio,pre_dynamic_energy_ratio,"
          "pre_dynamic_norm_ratio,r2_tick4,r2_tick16\n";
  for (const auto& arm : summary.arms) {
    arms << std::setprecision(17) << "FTD-0664," << arm.volume << ','
         << arm.sign << ',' << arm.initialized << ',' << arm.executed << ','
         << arm.redressed << ',' << arm.sector << ',' << 4 * arm.volume << ','
         << arm.return_tick << ',' << arm.initial_doublet << ','
         << arm.max_energy_drift << ',' << arm.max_common << ','
         << arm.max_decomposition << ',' << arm.recovery << ','
         << arm.ticks[pre_return_tick].doublet_ratio << ','
         << arm.ticks[pre_return_tick].dynamic_energy_ratio << ','
         << arm.ticks[pre_return_tick].dynamic_norm_ratio << ','
         << arm.ticks[4].radius_second_moment << ','
         << arm.ticks[pre_return_tick].radius_second_moment << '\n';
  }
  std::ofstream ticks(directory / "ftd_0664_volume_scaled_internal_mode_transfer_ticks_v1.csv");
  ticks << "ftd_id,volume,sign,tick,doublet_ratio,dynamic_energy_ratio,"
           "dynamic_norm_ratio,radius_second_moment,decomposition_residual,"
           "energy_drift,common_residual\n";
  for (const auto& arm : summary.arms) {
    for (const auto& tick : arm.ticks) {
      ticks << std::setprecision(17) << "FTD-0664," << arm.volume << ','
            << arm.sign << ',' << tick.tick << ',' << tick.doublet_ratio << ','
            << tick.dynamic_energy_ratio << ',' << tick.dynamic_norm_ratio << ','
            << tick.radius_second_moment << ','
            << tick.decomposition_residual << ',' << tick.energy_drift << ','
            << tick.common_residual << '\n';
    }
  }
}

}  // namespace

#ifdef FTD_0664_EMBEDDED
int ftd_0664_embedded_main() {
#else
int main() {
#endif
  VolumeSummary summary;
  summary.parent = volume_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  summary.normalization = normalization.valid;
  summary.beta = normalization.mapped_field_work_coefficient;
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  if (summary.parent && summary.normalization) {
    const auto reference = load_refined_state(0);
    const auto analytic = analytic_at(
        "volume_transfer", 0, reference, summary.beta, options);
    if (analytic.valid) summary.modes = full_modes(analytic.hessian);
    summary.eigenspace = summary.modes.valid
        && summary.modes.modes[6].group == summary.modes.modes[7].group;
  }
  if (summary.eigenspace) {
    std::array<std::future<std::array<VolumeArm, 2>>, 3> futures;
    for (std::size_t index = 0; index < volume_sizes.size(); ++index) {
      futures[index] = std::async(
          std::launch::async,
          [&, size = volume_sizes[index]] {
            return run_volume(size, summary.modes, summary.beta, options);
          });
    }
    for (std::size_t index = 0; index < volume_sizes.size(); ++index) {
      const auto volume_arms = futures[index].get();
      summary.arms.push_back(volume_arms[0]);
      summary.arms.push_back(volume_arms[1]);
      std::cout << "completed L=" << volume_sizes[index] << std::endl;
    }
  }
  evaluate_volume(summary);
  write_volume(summary);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << volume_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "execution=" << summary.execution
            << " locality=" << summary.locality
            << " emission=" << summary.emission
            << " outward=" << summary.outward
            << " locality_rms=" << summary.locality_residual << '\n'
            << "return=" << summary.return_classification
            << " scaled_cv=" << summary.return_scaled_cv << '\n';
  for (const auto& arm : summary.arms) {
    if (arm.ticks.size() > static_cast<std::size_t>(pre_return_tick)) {
      std::cout << "L=" << arm.volume << " sign=" << arm.sign
                << " executed=" << arm.executed
                << " pre_doublet="
                << arm.ticks[pre_return_tick].doublet_ratio
                << " pre_dynamic="
                << arm.ticks[pre_return_tick].dynamic_energy_ratio
                << " r2=" << arm.ticks[4].radius_second_moment << "->"
                << arm.ticks[pre_return_tick].radius_second_moment
                << " return=" << arm.return_tick
                << " recovery=" << arm.recovery << '\n';
    }
  }
  return summary.arms.size() == 6 ? 0 : 1;
}
