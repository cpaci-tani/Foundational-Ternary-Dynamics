// FTD-0668: test the first internal-mode recurrence before periodic
// self-contact in a large causal buffer.
#define FTD_0664_EMBEDDED
#include "test_volume_scaled_internal_mode_transfer.cpp"
#undef FTD_0664_EMBEDDED

namespace {

constexpr char causal_protocol_sha256[] =
    "FD959EADB5B50D237D78929295A45BC507DE37843DECA151705856F2359FA70C";
constexpr char causal_parent_0665_sha256[] =
    "3D9C7F4601C4932458F351A1DE412A6E6E849E2514691C2C21093944BEE9B5B2";
constexpr char causal_parent_0666_sha256[] =
    "E89871BA5CE26D098AFB1063BD74084E6971D4E3426CCB4907009565AA9A0749";
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
  bool dipped = false;
  int return_tick = -1;
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

bool causal_parent_fingerprint() {
  const auto root = std::filesystem::path(__FILE__).parent_path().parent_path();
  return file_has_fingerprint(
             root / "results/ftd_0665/ftd_0665_volume_scaled_internal_mode_transfer_v2.json",
             "E8E627DEE418186A96A951290B61396D5C3D18B40C0AF6B18A37B26289FFE9B8",
             "VOLUME_SCALED_PRE_RETURN_TRANSFER_V2_CONSTRUCTIVE")
      && file_has_fingerprint(
             root / "results/ftd_0666/ftd_0666_internal_mode_return_time_v1.json",
             "4AFD79B3207C16A37EBDF96197EFCDA64ADFD5410DB0825D6085280791D8FDEC",
             "INTERNAL_MODE_RETURN_TIME_MIXED");
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

bool sparse_equivalence_gate(
    const FullModes& modes,
    const ftd::eft::ConnectedMooreBlockOptions& base_options,
    double& state_difference) {
  const auto reference = causal_reference(17);
  double nominal = 0.0;
  const auto excited = volume_excitation(reference, modes, +1, nominal);
  if (reference.electric.L != 17 || excited.electric.L != 17) return false;
  auto dense_options = base_options;
  auto sparse_options = base_options;
  dense_options.use_sparse_local_current = false;
  sparse_options.use_sparse_local_current = true;
  const auto dense = ftd::eft::solve_connected_moore_block_forward(
      excited, dense_options);
  const auto sparse = ftd::eft::solve_connected_moore_block_forward(
      excited, sparse_options);
  if (!dense.valid || !sparse.valid || !dense.common_action_gates_pass
      || !sparse.common_action_gates_pass
      || common_residual(dense) > 1e-10
      || common_residual(sparse) > 1e-10) return false;
  state_difference = ftd::eft::connected_moore_block_state_max_difference(
      dense.later, sparse.later);
  return state_difference <= 1e-10;
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

std::array<CausalArm, 2> run_causal(
    const FullModes& modes,
    double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    bool& initial_fields_equal) {
  std::array<CausalArm, 2> arms;
  arms[0].sign = -1;
  arms[1].sign = +1;
  const auto control_initial = causal_reference(causal_volume);
  std::array<ftd::eft::ConnectedMooreBlockState, 3> initial;
  initial[0] = control_initial;
  for (int sign = 0; sign < 2; ++sign) {
    double nominal = 0.0;
    initial[sign + 1] = volume_excitation(
        control_initial, modes, arms[sign].sign, nominal);
    arms[sign].initial_doublet = paired_doublet(
        control_initial, initial[sign + 1], modes);
    arms[sign].initialized = initial[sign + 1].electric.L == causal_volume
        && arms[sign].initial_doublet > 0.0 && std::isfinite(nominal);
  }
  initial_fields_equal = arms[0].initialized && arms[1].initialized;
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
    arms[sign].sector = true;
    arms[sign].ticks.push_back(observe_causal(
        0, state[0], state[sign + 1], modes,
        arms[sign].initial_doublet, beta, origin, 0, 0.0, 0.0));
  }

  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> forward_cache;
  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> reverse_cache;
  bool forward = true;
  for (int tick = 1; tick <= causal_horizon && forward; ++tick) {
    double common = 0.0;
    int source_radius = 0;
    for (int path = 0; path < 3; ++path) {
      const auto step = ftd::eft::solve_connected_moore_block_forward(
          state[path], options, &forward_cache[path]);
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
      auto record = observe_causal(
          tick, state[0], state[path], modes, arms[sign].initial_doublet,
          beta, origin, source_radius, drift, common);
      arms[sign].max_energy_drift = std::max(
          arms[sign].max_energy_drift, drift);
      arms[sign].max_common = std::max(arms[sign].max_common, common);
      arms[sign].max_source_radius = std::max(
          arms[sign].max_source_radius, source_radius);
      arms[sign].ticks.push_back(record);
    }
    if (source_radius > causal_source_radius) forward = false;
    if (tick % 10 == 0)
      std::cout << "completed causal tick " << tick << std::endl;
  }

  bool reverse = forward;
  for (int tick = causal_horizon; tick >= 1 && reverse; --tick) {
    for (int path = 0; path < 3; ++path) {
      const auto step = ftd::eft::solve_connected_moore_block_reverse(
          state[path], options, &reverse_cache[path]);
      const double residual = common_residual(step);
      for (auto& arm : arms)
        arm.max_common = std::max(arm.max_common, residual);
      if (!step.valid || !step.common_action_gates_pass || residual > 1e-10) {
        reverse = false;
        break;
      }
      state[path] = step.earlier;
    }
    if (tick % 10 == 0)
      std::cout << "reversed causal tick " << tick << std::endl;
  }

  for (int sign = 0; sign < 2; ++sign) {
    const int path = sign + 1;
    arms[sign].recovery = std::max(
        ftd::eft::connected_moore_block_state_max_difference(
            initial[0], state[0]),
        ftd::eft::connected_moore_block_state_max_difference(
            initial[path], state[path]));
    arms[sign].executed = forward && reverse && arms[sign].initialized
        && arms[sign].sector
        && static_cast<int>(arms[sign].ticks.size()) == causal_horizon + 1
        && arms[sign].max_source_radius <= causal_source_radius
        && arms[sign].max_common <= 1e-10
        && arms[sign].max_energy_drift <= 1e-10
        && arms[sign].recovery <= 1e-8;
    for (const auto& record : arms[sign].ticks) {
      if (record.doublet_ratio < 0.60) arms[sign].dipped = true;
      if (arms[sign].dipped && record.doublet_ratio > 0.80) {
        arms[sign].return_tick = record.tick;
        break;
      }
    }
  }
  return arms;
}

void write_causal(const std::array<CausalArm, 2>& arms,
                  bool initial_fields_equal,
                  bool sparse_equivalent,
                  double sparse_state_difference,
                  const std::string& verdict) {
  const auto directory = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0668";
  std::filesystem::create_directories(directory);
  std::ofstream json(
      directory / "ftd_0668_causally_isolated_internal_recurrence_v1.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0668\",\n"
       << "  \"protocol_sha256\": \"" << causal_protocol_sha256 << "\",\n"
       << "  \"parent_0665_sha256\": \"" << causal_parent_0665_sha256
       << "\",\n"
       << "  \"parent_0666_sha256\": \"" << causal_parent_0666_sha256
       << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"volume\": " << causal_volume << ",\n"
       << "  \"horizon\": " << causal_horizon << ",\n"
       << "  \"source_radius_limit\": " << causal_source_radius << ",\n"
       << "  \"causal_contact_tick\": " << causal_contact_tick << ",\n"
       << "  \"initial_fields_bitwise_equal\": "
       << (initial_fields_equal ? "true" : "false") << ",\n"
       << "  \"dense_sparse_equivalent\": "
       << (sparse_equivalent ? "true" : "false") << ",\n"
       << "  \"dense_sparse_state_difference\": "
       << sparse_state_difference << ",\n"
       << "  \"negative_executed\": "
       << (arms[0].executed ? "true" : "false") << ",\n"
       << "  \"positive_executed\": "
       << (arms[1].executed ? "true" : "false") << ",\n"
       << "  \"negative_return_tick\": " << arms[0].return_tick << ",\n"
       << "  \"positive_return_tick\": " << arms[1].return_tick << ",\n"
       << "  \"negative_recovery\": " << arms[0].recovery << ",\n"
       << "  \"positive_recovery\": " << arms[1].recovery << ",\n"
       << "  \"maximum_source_radius\": "
       << std::max(arms[0].max_source_radius, arms[1].max_source_radius)
       << "\n}\n";

  std::ofstream ticks(
      directory / "ftd_0668_causally_isolated_internal_recurrence_ticks_v1.csv");
  ticks << "ftd_id,sign,tick,doublet_ratio,field_energy_ratio,"
           "positive_field_norm_ratio,near_fraction,radius_second_moment,"
           "dynamic_support_radius,source_support_radius,energy_drift,"
           "common_residual\n";
  for (const auto& arm : arms) {
    for (const auto& record : arm.ticks) {
      ticks << std::setprecision(17) << "FTD-0668," << arm.sign << ','
            << record.tick << ',' << record.doublet_ratio << ','
            << record.field_energy_ratio << ','
            << record.positive_field_norm_ratio << ','
            << record.near_fraction << ',' << record.radius_second_moment << ','
            << record.dynamic_support_radius << ','
            << record.source_support_radius << ',' << record.energy_drift << ','
            << record.common_residual << '\n';
    }
  }
}

}  // namespace

int main() {
  const bool parent = causal_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  FullModes modes;
  if (parent && normalization.valid) {
    const auto reference = load_refined_state(0);
    const auto analytic = analytic_at(
        "causal_recurrence", 0, reference,
        normalization.mapped_field_work_coefficient, options);
    if (analytic.valid) modes = full_modes(analytic.hessian);
  }
  bool initial_fields_equal = false;
  double sparse_state_difference = INFINITY;
  const bool sparse_equivalent = modes.valid
      && sparse_equivalence_gate(modes, options, sparse_state_difference);
  options.use_sparse_local_current = true;
  std::array<CausalArm, 2> arms;
  if (sparse_equivalent && modes.valid
      && modes.modes[6].group == modes.modes[7].group) {
    arms = run_causal(
        modes, normalization.mapped_field_work_coefficient,
        options, initial_fields_equal);
  }
  const bool execution = parent && normalization.valid && modes.valid
      && sparse_equivalent && initial_fields_equal
      && causal_horizon < causal_contact_tick
      && arms[0].executed && arms[1].executed;
  const bool constructive = execution
      && arms[0].return_tick >= 68 && arms[0].return_tick <= causal_horizon
      && arms[1].return_tick >= 68 && arms[1].return_tick <= causal_horizon
      && arms[0].return_tick < causal_contact_tick
      && arms[1].return_tick < causal_contact_tick
      && std::abs(arms[0].return_tick - arms[1].return_tick) <= 2;
  const bool no_return = execution && arms[0].dipped && arms[1].dipped
      && arms[0].return_tick < 0 && arms[1].return_tick < 0;
  const std::string verdict = !execution
      ? "CAUSALLY_ISOLATED_RECURRENCE_EXECUTION_INVALID"
      : constructive
          ? "CAUSALLY_ISOLATED_INTERNAL_RECURRENCE_CONSTRUCTIVE"
          : no_return
              ? "NO_PRECONTACT_INTERNAL_RECURRENCE_IN_WINDOW"
              : "CAUSALLY_ISOLATED_INTERNAL_RECURRENCE_MIXED";
  write_causal(arms, initial_fields_equal, sparse_equivalent,
               sparse_state_difference, verdict);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << causal_protocol_sha256 << '\n'
            << "verdict=" << verdict << '\n'
            << "execution=" << execution
            << " sparse_equivalent=" << sparse_equivalent
            << " sparse_difference=" << sparse_state_difference
            << " initial_fields_equal=" << initial_fields_equal
            << " contact_tick=" << causal_contact_tick
            << " returns=" << arms[0].return_tick << ',' << arms[1].return_tick
            << " max_source="
            << std::max(arms[0].max_source_radius, arms[1].max_source_radius)
            << " recoveries=" << arms[0].recovery << ',' << arms[1].recovery
            << '\n';
  return execution ? 0 : 1;
}
