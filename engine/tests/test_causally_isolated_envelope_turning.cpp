// FTD-0670: held-out half-amplitude action-envelope turning before periodic
// self-contact.
#define FTD_0664_EMBEDDED
#include "test_volume_scaled_internal_mode_transfer.cpp"
#undef FTD_0664_EMBEDDED

namespace {

constexpr int causal_volume = 97;
constexpr int causal_horizon = 80;
constexpr int causal_source_radius = 8;
constexpr int causal_contact_tick = causal_volume - 2 * causal_source_radius;
constexpr double dynamic_support_squared = 1e-28;

struct CausalTick {
  int tick = 0;
  double doublet_ratio = 0.0;
  double field_energy_ratio = 0.0;
  double positive_field_norm_ratio = 0.0;
  double near_fraction = 0.0;
  double radius_second_moment = 0.0;
  int dynamic_support_radius = -1;
  int source_support_radius = 0;
  double energy_drift = 0.0;
  double common_residual = 0.0;
};

struct CausalArm {
  int sign = 0;
  bool initialized = false;
  bool executed = false;
  bool sector = false;
  double initial_doublet = 0.0;
  double recovery = INFINITY;
  double max_energy_drift = 0.0;
  double max_common = 0.0;
  int max_source_radius = 0;
  std::vector<CausalTick> ticks;
};

bool file_has_fingerprint(const std::filesystem::path& path,
                          const std::string& protocol,
                          const std::string& verdict) {
  std::ifstream input(path, std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)), {});
  return bytes.find(protocol) != std::string::npos
      && bytes.find(verdict) != std::string::npos;
}

bool equal_face_bits(const ftd::eft::MatchedFaceFlux& a,
                     const ftd::eft::MatchedFaceFlux& b) {
  return a.L == b.L && a.x == b.x && a.y == b.y && a.z == b.z;
}

bool equal_edge_bits(const ftd::eft::MatchedEdgeField& a,
                     const ftd::eft::MatchedEdgeField& b) {
  return a.L == b.L && a.x == b.x && a.y == b.y && a.z == b.z;
}

ftd::eft::ConnectedMooreBlockState causal_reference(int size) {
  const auto base = load_refined_state(0);
  if (base.electric.L != L)
    return ftd::eft::ConnectedMooreBlockState{};
  auto geometry = base;
  geometry.electric = ftd::eft::MatchedFaceFlux(size);
  geometry.magnetic_half = ftd::eft::MatchedEdgeField(size);
  const Vec3 base_center = center(base);
  const Vec3 target_center{
      0.5 * static_cast<double>(size - 1),
      0.5 * static_cast<double>(size - 1),
      0.5 * static_cast<double>(size - 1)};
  for (int particle = 0; particle < count; ++particle) {
    const Vec3 x = target_center
        + (position(base.constituents[particle]) - base_center);
    geometry.constituents[particle] = point_at_volume(x, size);
    geometry.constituents[particle].momentum = {};
  }
  const auto dressed = ftd::eft::redress_connected_moore_block_with_fibre_limit(
      geometry, 8, 1e-13, 4096);
  return dressed.valid ? dressed.state
                       : ftd::eft::ConnectedMooreBlockState{};
}

double paired_doublet(
    const ftd::eft::ConnectedMooreBlockState& control,
    const ftd::eft::ConnectedMooreBlockState& excited,
    const FullModes& modes) {
  const auto modal = paired_modal_coordinates(control, excited, modes);
  const double omega = 0.5 * (modes.modes[6].omega + modes.modes[7].omega);
  const double q2 = modal[0][6] * modal[0][6]
      + modal[0][7] * modal[0][7];
  const double p2 = modal[1][6] * modal[1][6]
      + modal[1][7] * modal[1][7];
  return 0.5 * (p2 + omega * omega * q2);
}

double coordinate_distance(double coordinate, double origin, int size) {
  return std::abs(periodic_delta(coordinate - origin, size));
}

int segment_support_radius(
    const ftd::eft::ConnectedMooreBlockStepResult& step,
    const Vec3& origin) {
  int maximum = 0;
  const int size = step.later.electric.L;
  for (const auto& segment : step.segments) {
    for (const auto& entry : segment.sparse_current) {
      const double distance = std::max({
          coordinate_distance(entry.face.x, origin.x, size),
          coordinate_distance(entry.face.y, origin.y, size),
          coordinate_distance(entry.face.z, origin.z, size)});
      maximum = std::max(maximum,
          1 + static_cast<int>(std::ceil(distance)));
    }
  }
  return maximum;
}

CausalTick observe_causal(
    int tick,
    const ftd::eft::ConnectedMooreBlockState& control,
    const ftd::eft::ConnectedMooreBlockState& excited,
    const FullModes& modes,
    double initial_doublet,
    double beta,
    const Vec3& origin,
    int source_radius,
    double energy_drift,
    double common_residual_value) {
  CausalTick record;
  record.tick = tick;
  record.doublet_ratio = paired_doublet(control, excited, modes)
      / initial_doublet;
  const auto difference_e = subtract_face(excited.electric, control.electric);
  const auto difference_b = subtract_edge(
      excited.magnetic_half, control.magnetic_half);
  record.field_energy_ratio = beta * ftd::eft::matched_modified_energy(
      difference_e, difference_b, ftd::C_SPEED) / initial_doublet;

  long double total = 0.0L;
  long double near = 0.0L;
  long double radius2 = 0.0L;
  const int size = difference_e.L;
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      for (int z = 0; z < size; ++z) {
        const std::size_t index = static_cast<std::size_t>(
            (x * size + y) * size + z);
        const long double activity = 0.5L * beta * (
            static_cast<long double>(difference_e.x[index])
                * difference_e.x[index]
            + static_cast<long double>(difference_e.y[index])
                * difference_e.y[index]
            + static_cast<long double>(difference_e.z[index])
                * difference_e.z[index]
            + static_cast<long double>(difference_b.x[index])
                * difference_b.x[index]
            + static_cast<long double>(difference_b.y[index])
                * difference_b.y[index]
            + static_cast<long double>(difference_b.z[index])
                * difference_b.z[index]);
        const double dx = coordinate_distance(x, origin.x, size);
        const double dy = coordinate_distance(y, origin.y, size);
        const double dz = coordinate_distance(z, origin.z, size);
        const double chebyshev = std::max({dx, dy, dz});
        const double squared_radius = dx * dx + dy * dy + dz * dz;
        total += activity;
        radius2 += activity * squared_radius;
        if (chebyshev <= causal_source_radius) near += activity;
        if (activity > dynamic_support_squared)
          record.dynamic_support_radius = std::max(
              record.dynamic_support_radius,
              static_cast<int>(std::ceil(chebyshev)));
      }
    }
  }
  record.positive_field_norm_ratio = static_cast<double>(total)
      / initial_doublet;
  record.near_fraction = total > 0.0L
      ? static_cast<double>(near / total) : 1.0;
  record.radius_second_moment = total > 0.0L
      ? static_cast<double>(radius2 / total) : 0.0;
  record.source_support_radius = source_radius;
  record.energy_drift = energy_drift;
  record.common_residual = common_residual_value;
  return record;
}

constexpr char turning_protocol_sha256[] =
    "92B98E746C02BAA980A43AF8C9E84B8CF6B5DC8161968511DBF14365D8237412";
constexpr char turning_parent_json_sha256[] =
    "D1EF53978C9B04F9EEC2FF34954D7D04CA9163AAE6FAD6833D7CCF352CEAE0D2";
constexpr char turning_parent_csv_sha256[] =
    "E34AC8AAE7FC703B037D9F1B730A2A97213419A9A5D01996D5C9716999256FDB";
constexpr double turning_amplitude_scale = 0.5;

struct TurningArm {
  CausalArm causal;
  std::vector<int> trough_ticks;
  int primary_tick = -1;
  double primary_ratio = INFINITY;
  int second_post_tick = -1;
  double second_post_ratio = INFINITY;
  double recovery_increment = -INFINITY;
  bool primary_window = false;
  bool descending = false;
  bool ascending = false;
  bool recovery = false;
  bool morphology = false;
};

bool turning_parent_fingerprint() {
  const auto root = std::filesystem::path(__FILE__).parent_path().parent_path();
  return file_has_fingerprint(
             root / "results/ftd_0668/ftd_0668_causally_isolated_internal_recurrence_v1.json",
             "FD959EADB5B50D237D78929295A45BC507DE37843DECA151705856F2359FA70C",
             "CAUSALLY_ISOLATED_INTERNAL_RECURRENCE_MIXED")
      && file_has_fingerprint(
             root / "results/ftd_0668/ftd_0668_causally_isolated_internal_recurrence_ticks_v1.csv",
             "ftd_id,sign,tick,doublet_ratio",
             "FTD-0668");
}

ftd::eft::ConnectedMooreBlockState turning_excitation(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const FullModes& modes,
    int sign,
    double& nominal) {
  auto state = volume_excitation(reference, modes, sign, nominal);
  for (auto& constituent : state.constituents) {
    constituent.momentum.x *= turning_amplitude_scale;
    constituent.momentum.y *= turning_amplitude_scale;
    constituent.momentum.z *= turning_amplitude_scale;
  }
  nominal *= turning_amplitude_scale * turning_amplitude_scale;
  return state;
}

void classify_turning(TurningArm& arm) {
  const auto& ticks = arm.causal.ticks;
  for (std::size_t i = 1; i + 1 < ticks.size(); ++i) {
    if (ticks[i].tick < 60 || ticks[i].tick > 79) continue;
    if (ticks[i].doublet_ratio < ticks[i - 1].doublet_ratio
        && ticks[i].doublet_ratio < ticks[i + 1].doublet_ratio) {
      arm.trough_ticks.push_back(ticks[i].tick);
      if (ticks[i].doublet_ratio < arm.primary_ratio) {
        arm.primary_ratio = ticks[i].doublet_ratio;
        arm.primary_tick = ticks[i].tick;
      }
    }
  }
  arm.primary_window = arm.primary_tick >= 71 && arm.primary_tick <= 73;
  if (arm.primary_tick < 0) return;

  std::vector<const CausalTick*> before;
  std::vector<const CausalTick*> after;
  for (const int tick : arm.trough_ticks) {
    const auto* record = &ticks[static_cast<std::size_t>(tick)];
    if (tick < arm.primary_tick) before.push_back(record);
    if (tick > arm.primary_tick) after.push_back(record);
  }
  if (before.size() >= 3) {
    const std::size_t n = before.size();
    arm.descending = before[n - 3]->doublet_ratio
            > before[n - 2]->doublet_ratio
        && before[n - 2]->doublet_ratio > before[n - 1]->doublet_ratio
        && before[n - 1]->doublet_ratio > arm.primary_ratio;
  }
  if (after.size() >= 2) {
    arm.second_post_tick = after[1]->tick;
    arm.second_post_ratio = after[1]->doublet_ratio;
    arm.recovery_increment = arm.second_post_ratio - arm.primary_ratio;
    arm.ascending = arm.primary_ratio < after[0]->doublet_ratio
        && after[0]->doublet_ratio < after[1]->doublet_ratio;
    arm.recovery = arm.recovery_increment >= 0.05;
  }
  if (!ticks.empty()) {
    const auto& final = ticks.back();
    arm.morphology = final.positive_field_norm_ratio > 0.0
        && final.near_fraction < 0.40
        && final.radius_second_moment > 300.0;
  }
}

std::array<TurningArm, 2> run_turning(
    const FullModes& modes,
    double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    bool& initial_fields_equal) {
  std::array<TurningArm, 2> arms;
  arms[0].causal.sign = -1;
  arms[1].causal.sign = +1;
  const auto control_initial = causal_reference(causal_volume);
  std::array<ftd::eft::ConnectedMooreBlockState, 3> initial;
  initial[0] = control_initial;
  for (int sign = 0; sign < 2; ++sign) {
    double nominal = 0.0;
    initial[sign + 1] = turning_excitation(
        control_initial, modes, arms[sign].causal.sign, nominal);
    arms[sign].causal.initial_doublet = paired_doublet(
        control_initial, initial[sign + 1], modes);
    arms[sign].causal.initialized = initial[sign + 1].electric.L == causal_volume
        && arms[sign].causal.initial_doublet > 0.0
        && std::isfinite(nominal);
  }
  initial_fields_equal = arms[0].causal.initialized
      && arms[1].causal.initialized;
  for (int path = 1; path < 3 && initial_fields_equal; ++path) {
    initial_fields_equal = equal_face_bits(
        initial[0].electric, initial[path].electric)
        && equal_edge_bits(
            initial[0].magnetic_half, initial[path].magnetic_half);
  }
  if (!initial_fields_equal) return arms;

  std::array<ftd::eft::ConnectedMooreBlockState, 3> state = initial;
  std::array<double, 3> initial_energy{};
  std::array<std::vector<int>, 3> initial_sector;
  for (int path = 0; path < 3; ++path) {
    initial_energy[path] = energy_parts(state[path], beta, options).total;
    initial_sector[path] = sector_signature(state[path]);
  }
  const Vec3 origin = center(control_initial);
  for (int sign = 0; sign < 2; ++sign) {
    arms[sign].causal.sector = true;
    arms[sign].causal.ticks.push_back(observe_causal(
        0, state[0], state[sign + 1], modes,
        arms[sign].causal.initial_doublet, beta, origin, 0, 0.0, 0.0));
  }

  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> forward_cache;
  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> reverse_cache;
  bool forward = true;
  for (int tick = 1; tick <= causal_horizon && forward; ++tick) {
    double common = 0.0;
    int source_radius = 0;
    std::array<ftd::eft::ConnectedMooreBlockStepResult, 3> steps;
#pragma omp parallel for num_threads(3)
    for (int path = 0; path < 3; ++path) {
      steps[path] = ftd::eft::solve_connected_moore_block_forward(
          state[path], options, &forward_cache[path]);
    }
    for (int path = 0; path < 3; ++path) {
      const auto& step = steps[path];
      const double residual = common_residual(step);
      common = std::max(common, residual);
      if (!step.valid || !step.common_action_gates_pass || residual > 1e-10) {
        forward = false;
        break;
      }
      source_radius = std::max(
          source_radius, segment_support_radius(step, origin));
      state[path] = step.later;
      for (auto& arm : arms) {
        arm.causal.sector = arm.causal.sector
            && sector_signature(state[path]) == initial_sector[path];
      }
    }
    if (!forward) break;
#pragma omp parallel for num_threads(2)
    for (int sign = 0; sign < 2; ++sign) {
      const int path = sign + 1;
      const double drift = std::max(
          std::abs(energy_parts(state[0], beta, options).total
                   - initial_energy[0]),
          std::abs(energy_parts(state[path], beta, options).total
                   - initial_energy[path]));
      const auto record = observe_causal(
          tick, state[0], state[path], modes, arms[sign].causal.initial_doublet,
          beta, origin, source_radius, drift, common);
      arms[sign].causal.max_energy_drift = std::max(
          arms[sign].causal.max_energy_drift, drift);
      arms[sign].causal.max_common = std::max(
          arms[sign].causal.max_common, common);
      arms[sign].causal.max_source_radius = std::max(
          arms[sign].causal.max_source_radius, source_radius);
      arms[sign].causal.ticks.push_back(record);
    }
    if (source_radius > causal_source_radius) forward = false;
    if (tick % 10 == 0)
      std::cout << "completed turning tick " << tick << std::endl;
  }

  bool reverse = forward;
  for (int tick = causal_horizon; tick >= 1 && reverse; --tick) {
    std::array<ftd::eft::ConnectedMooreBlockStepResult, 3> steps;
#pragma omp parallel for num_threads(3)
    for (int path = 0; path < 3; ++path) {
      steps[path] = ftd::eft::solve_connected_moore_block_reverse(
          state[path], options, &reverse_cache[path]);
    }
    for (int path = 0; path < 3; ++path) {
      const auto& step = steps[path];
      const double residual = common_residual(step);
      for (auto& arm : arms)
        arm.causal.max_common = std::max(arm.causal.max_common, residual);
      if (!step.valid || !step.common_action_gates_pass || residual > 1e-10) {
        reverse = false;
        break;
      }
      state[path] = step.earlier;
    }
    if (tick % 10 == 0)
      std::cout << "reversed turning tick " << tick << std::endl;
  }

  for (int sign = 0; sign < 2; ++sign) {
    const int path = sign + 1;
    auto& causal = arms[sign].causal;
    causal.recovery = std::max(
        ftd::eft::connected_moore_block_state_max_difference(
            initial[0], state[0]),
        ftd::eft::connected_moore_block_state_max_difference(
            initial[path], state[path]));
    causal.executed = forward && reverse && causal.initialized && causal.sector
        && static_cast<int>(causal.ticks.size()) == causal_horizon + 1
        && causal.max_source_radius <= causal_source_radius
        && causal.max_common <= 1e-10
        && causal.max_energy_drift <= 1e-10
        && causal.recovery <= 1e-8;
    classify_turning(arms[sign]);
  }
  return arms;
}

void write_turning(
    const std::array<TurningArm, 2>& arms,
    bool initial_fields_equal,
    const std::string& verdict) {
  const auto directory = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0670";
  std::filesystem::create_directories(directory);
  std::ofstream json(
      directory / "ftd_0670_causally_isolated_envelope_turning_v1.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0670\",\n"
       << "  \"protocol_sha256\": \"" << turning_protocol_sha256 << "\",\n"
       << "  \"parent_json_sha256\": \"" << turning_parent_json_sha256
       << "\",\n"
       << "  \"parent_csv_sha256\": \"" << turning_parent_csv_sha256
       << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"volume\": " << causal_volume << ",\n"
       << "  \"horizon\": " << causal_horizon << ",\n"
       << "  \"maximum_constituent_momentum_amplitude\": 4e-6,\n"
       << "  \"causal_contact_tick\": " << causal_contact_tick << ",\n"
       << "  \"initial_fields_bitwise_equal\": "
       << (initial_fields_equal ? "true" : "false") << ",\n"
       << "  \"negative_executed\": "
       << (arms[0].causal.executed ? "true" : "false") << ",\n"
       << "  \"positive_executed\": "
       << (arms[1].causal.executed ? "true" : "false") << ",\n"
       << "  \"negative_primary_tick\": " << arms[0].primary_tick << ",\n"
       << "  \"positive_primary_tick\": " << arms[1].primary_tick << ",\n"
       << "  \"negative_primary_ratio\": " << arms[0].primary_ratio << ",\n"
       << "  \"positive_primary_ratio\": " << arms[1].primary_ratio << ",\n"
       << "  \"negative_second_post_tick\": "
       << arms[0].second_post_tick << ",\n"
       << "  \"positive_second_post_tick\": "
       << arms[1].second_post_tick << ",\n"
       << "  \"negative_recovery_increment\": "
       << arms[0].recovery_increment << ",\n"
       << "  \"positive_recovery_increment\": "
       << arms[1].recovery_increment << ",\n"
       << "  \"negative_descending\": "
       << (arms[0].descending ? "true" : "false") << ",\n"
       << "  \"positive_descending\": "
       << (arms[1].descending ? "true" : "false") << ",\n"
       << "  \"negative_ascending\": "
       << (arms[0].ascending ? "true" : "false") << ",\n"
       << "  \"positive_ascending\": "
       << (arms[1].ascending ? "true" : "false") << ",\n"
       << "  \"negative_morphology\": "
       << (arms[0].morphology ? "true" : "false") << ",\n"
       << "  \"positive_morphology\": "
       << (arms[1].morphology ? "true" : "false") << ",\n"
       << "  \"negative_recovery\": " << arms[0].causal.recovery << ",\n"
       << "  \"positive_recovery\": " << arms[1].causal.recovery << "\n}\n";

  std::ofstream csv(
      directory / "ftd_0670_causally_isolated_envelope_turning_ticks_v1.csv");
  csv << "ftd_id,sign,tick,doublet_ratio,field_energy_ratio,"
         "positive_field_norm_ratio,near_fraction,radius_second_moment,"
         "dynamic_support_radius,source_support_radius,energy_drift,"
         "common_residual\n";
  for (const auto& arm : arms) {
    for (const auto& record : arm.causal.ticks) {
      csv << std::setprecision(17) << "FTD-0670," << arm.causal.sign << ','
          << record.tick << ',' << record.doublet_ratio << ','
          << record.field_energy_ratio << ','
          << record.positive_field_norm_ratio << ',' << record.near_fraction
          << ',' << record.radius_second_moment << ','
          << record.dynamic_support_radius << ',' << record.source_support_radius
          << ',' << record.energy_drift << ',' << record.common_residual << '\n';
    }
  }
}

}  // namespace

int main() {
  const bool parent = turning_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  options.use_sparse_local_current = true;
  FullModes modes;
  if (parent && normalization.valid) {
    const auto reference = load_refined_state(0);
    const auto analytic = analytic_at(
        "envelope_turning", 0, reference,
        normalization.mapped_field_work_coefficient, options);
    if (analytic.valid) modes = full_modes(analytic.hessian);
  }
  bool initial_fields_equal = false;
  std::array<TurningArm, 2> arms;
  if (modes.valid && modes.modes[6].group == modes.modes[7].group) {
    arms = run_turning(
        modes, normalization.mapped_field_work_coefficient,
        options, initial_fields_equal);
  }
  const bool execution = parent && normalization.valid && modes.valid
      && initial_fields_equal && causal_horizon < causal_contact_tick
      && arms[0].causal.executed && arms[1].causal.executed;
  const bool polarity = execution
      && std::abs(arms[0].primary_tick - arms[1].primary_tick) <= 1
      && std::abs(arms[0].primary_ratio - arms[1].primary_ratio) <= 1e-4
      && std::abs(arms[0].recovery_increment - arms[1].recovery_increment)
          <= 1e-4;
  const bool constructive = polarity
      && arms[0].primary_window && arms[1].primary_window
      && arms[0].descending && arms[1].descending
      && arms[0].ascending && arms[1].ascending
      && arms[0].recovery && arms[1].recovery
      && arms[0].morphology && arms[1].morphology;
  const bool primary_present = execution
      && arms[0].primary_window && arms[1].primary_window;
  const std::string verdict = !execution
      ? "CAUSALLY_ISOLATED_ENVELOPE_TURNING_EXECUTION_INVALID"
      : constructive
          ? "CAUSALLY_ISOLATED_ENVELOPE_TURNING_CONSTRUCTIVE"
          : primary_present
              ? "CAUSALLY_ISOLATED_ENVELOPE_TURNING_MIXED"
              : "HELD_OUT_ENVELOPE_TURNING_CLOSED_NEGATIVE";
  write_turning(arms, initial_fields_equal, verdict);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << turning_protocol_sha256 << '\n'
            << "verdict=" << verdict << '\n'
            << "execution=" << execution
            << " primary_ticks=" << arms[0].primary_tick << ','
            << arms[1].primary_tick
            << " primary_ratios=" << arms[0].primary_ratio << ','
            << arms[1].primary_ratio
            << " recovery_increments=" << arms[0].recovery_increment << ','
            << arms[1].recovery_increment
            << " polarity=" << polarity
            << " recoveries=" << arms[0].causal.recovery << ','
            << arms[1].causal.recovery << '\n';
  return execution ? 0 : 1;
}
