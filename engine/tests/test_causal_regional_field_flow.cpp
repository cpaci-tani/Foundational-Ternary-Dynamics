// FTD-0672: causal nested-radius field-flow discriminator.
#define FTD_0664_EMBEDDED
#include "test_volume_scaled_internal_mode_transfer.cpp"
#undef FTD_0664_EMBEDDED

#include "ftd/eft/matched_regional_energy_transport.h"

namespace {

constexpr char regional_protocol_sha256[] =
    "F0A2F895C07ADD99FC0BF4E39B95CD2FCEEE4BEBC10A4EE16CE4E47324B9C971";
constexpr int regional_volume = 97;
constexpr int regional_horizon = 80;
constexpr int regional_source_radius = 8;
constexpr int regional_contact_tick =
    regional_volume - 2 * regional_source_radius;
constexpr std::array<double, 3> regional_radii{{8.0, 16.0, 24.0}};
constexpr double regional_amplitude_scale = 0.25;
constexpr double dynamic_support_squared = 1e-28;

struct RegionalLedgerTick {
  double radius = 0.0;
  double energy_before = 0.0;
  double energy_pre_current = 0.0;
  double energy_after = 0.0;
  double boundary_transport_into = 0.0;
  double source_exchange_into_field = 0.0;
  double update_residual = 0.0;
  double partition_residual = 0.0;
  double ledger_residual = 0.0;
  bool valid = false;
};

struct RegionalTick {
  int tick = 0;
  double doublet_ratio = 0.0;
  double positive_field_norm_ratio = 0.0;
  double near_fraction = 1.0;
  double radius_second_moment = 0.0;
  int dynamic_support_radius = -1;
  int source_support_radius = 0;
  double energy_drift = 0.0;
  double common_residual = 0.0;
  std::array<RegionalLedgerTick, 3> ledgers;
};

struct RegionalMetrics {
  std::array<double, 3> outward_before_68{};
  std::array<double, 3> inward_68_80{};
  std::array<double, 3> net_outward{};
  std::array<double, 3> exchange_68_80{};
};

struct RegionalArm {
  int sign = 0;
  bool initialized = false;
  bool executed = false;
  bool sector = false;
  double initial_doublet = 0.0;
  double recovery = INFINITY;
  double max_energy_drift = 0.0;
  double max_common = 0.0;
  double max_observer_residual = 0.0;
  double source_exchange_consistency = INFINITY;
  int max_source_radius = 0;
  std::vector<RegionalTick> ticks;
  std::vector<int> trough_ticks;
  int primary_tick = -1;
  double primary_ratio = INFINITY;
  double recovery_increment = -INFINITY;
  bool turning = false;
  RegionalMetrics metrics;
  std::string transport_class = "REGIONAL_FLOW_MIXED";
  std::string exchange_class = "RECOVERY_EXCHANGE_BALANCED";
};

bool file_contains(const std::filesystem::path& path,
                   const std::string& first,
                   const std::string& second) {
  std::ifstream input(path, std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)), {});
  return bytes.find(first) != std::string::npos
      && bytes.find(second) != std::string::npos;
}

bool regional_parent_fingerprint() {
  const auto root = std::filesystem::path(__FILE__).parent_path().parent_path();
  return file_contains(
             root / "results/ftd_0670/ftd_0670_causally_isolated_envelope_turning_v1.json",
             "92B98E746C02BAA980A43AF8C9E84B8CF6B5DC8161968511DBF14365D8237412",
             "CAUSALLY_ISOLATED_ENVELOPE_TURNING_CONSTRUCTIVE")
      && file_contains(
             root / "results/ftd_0670/ftd_0670_causally_isolated_envelope_turning_ticks_v1.csv",
             "ftd_id,sign,tick,doublet_ratio",
             "FTD-0670");
}

bool equal_face_bits(const ftd::eft::MatchedFaceFlux& left,
                     const ftd::eft::MatchedFaceFlux& right) {
  return left.L == right.L && left.x == right.x
      && left.y == right.y && left.z == right.z;
}

bool equal_edge_bits(const ftd::eft::MatchedEdgeField& left,
                     const ftd::eft::MatchedEdgeField& right) {
  return left.L == right.L && left.x == right.x
      && left.y == right.y && left.z == right.z;
}

ftd::eft::ConnectedMooreBlockState regional_reference() {
  const auto base = load_refined_state(0);
  if (base.electric.L != L)
    return ftd::eft::ConnectedMooreBlockState{};
  auto geometry = base;
  geometry.electric = ftd::eft::MatchedFaceFlux(regional_volume);
  geometry.magnetic_half = ftd::eft::MatchedEdgeField(regional_volume);
  const Vec3 base_center = center(base);
  const Vec3 target_center{
      0.5 * static_cast<double>(regional_volume - 1),
      0.5 * static_cast<double>(regional_volume - 1),
      0.5 * static_cast<double>(regional_volume - 1)};
  for (int particle = 0; particle < count; ++particle) {
    const Vec3 x = target_center
        + (position(base.constituents[particle]) - base_center);
    geometry.constituents[particle] = point_at_volume(x, regional_volume);
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

ftd::eft::ConnectedMooreBlockState regional_excitation(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const FullModes& modes,
    int sign,
    double& nominal) {
  auto state = volume_excitation(reference, modes, sign, nominal);
  for (auto& constituent : state.constituents) {
    constituent.momentum.x *= regional_amplitude_scale;
    constituent.momentum.y *= regional_amplitude_scale;
    constituent.momentum.z *= regional_amplitude_scale;
  }
  nominal *= regional_amplitude_scale * regional_amplitude_scale;
  return state;
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

void source_free_intermediate(
    const ftd::eft::MatchedFaceFlux& electric_before,
    const ftd::eft::MatchedEdgeField& magnetic_after,
    ftd::eft::MatchedFaceFlux& electric_pre_current) {
  electric_pre_current = electric_before;
  const auto curl = ftd::eft::matched_curl(magnetic_after);
  for (std::size_t index = 0; index < electric_pre_current.x.size(); ++index) {
    electric_pre_current.x[index] += ftd::C_SPEED * curl.x[index];
    electric_pre_current.y[index] += ftd::C_SPEED * curl.y[index];
    electric_pre_current.z[index] += ftd::C_SPEED * curl.z[index];
  }
}

void observe_positive_morphology(
    RegionalTick& record,
    const ftd::eft::MatchedFaceFlux& difference_e,
    const ftd::eft::MatchedEdgeField& difference_b,
    double beta,
    double initial_doublet,
    const Vec3& origin) {
  long double total = 0.0L;
  long double near = 0.0L;
  long double radius2 = 0.0L;
  const int size = difference_e.L;
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      for (int z = 0; z < size; ++z) {
        const auto index = static_cast<std::size_t>(
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
        total += activity;
        radius2 += activity * (dx * dx + dy * dy + dz * dz);
        if (chebyshev <= regional_source_radius) near += activity;
        if (activity > dynamic_support_squared) {
          record.dynamic_support_radius = std::max(
              record.dynamic_support_radius,
              static_cast<int>(std::ceil(chebyshev)));
        }
      }
    }
  }
  record.positive_field_norm_ratio = static_cast<double>(total)
      / initial_doublet;
  record.near_fraction = total > 0.0L
      ? static_cast<double>(near / total) : 1.0;
  record.radius_second_moment = total > 0.0L
      ? static_cast<double>(radius2 / total) : 0.0;
}

RegionalTick observe_tick(
    int tick,
    const ftd::eft::ConnectedMooreBlockState& control_before,
    const ftd::eft::ConnectedMooreBlockState& excited_before,
    const ftd::eft::ConnectedMooreBlockState& control_after,
    const ftd::eft::ConnectedMooreBlockState& excited_after,
    const FullModes& modes,
    double initial_doublet,
    double beta,
    const Vec3& origin,
    int source_radius,
    double energy_drift,
    double common_residual_value) {
  RegionalTick record;
  record.tick = tick;
  record.doublet_ratio = paired_doublet(
      control_after, excited_after, modes) / initial_doublet;
  record.source_support_radius = source_radius;
  record.energy_drift = energy_drift;
  record.common_residual = common_residual_value;

  const auto electric_before = subtract_face(
      excited_before.electric, control_before.electric);
  const auto magnetic_before = subtract_edge(
      excited_before.magnetic_half, control_before.magnetic_half);
  const auto electric_after = subtract_face(
      excited_after.electric, control_after.electric);
  const auto magnetic_after = subtract_edge(
      excited_after.magnetic_half, control_after.magnetic_half);
  ftd::eft::MatchedFaceFlux electric_pre_current(regional_volume);
  source_free_intermediate(
      electric_before, magnetic_after, electric_pre_current);
  observe_positive_morphology(
      record, electric_after, magnetic_after, beta,
      initial_doublet, origin);

  const double scale = beta / initial_doublet;
  for (std::size_t radius = 0; radius < regional_radii.size(); ++radius) {
    const auto result = ftd::eft::evaluate_matched_regional_energy_transport(
        electric_before, magnetic_before, electric_pre_current,
        magnetic_after, electric_after, ftd::C_SPEED,
        origin, regional_radii[radius], 1e-10);
    auto& ledger = record.ledgers[radius];
    ledger.radius = regional_radii[radius];
    ledger.energy_before = scale * result.energy_before;
    ledger.energy_pre_current = scale * result.energy_pre_current;
    ledger.energy_after = scale * result.energy_after;
    ledger.boundary_transport_into =
        scale * result.boundary_transport_into;
    ledger.source_exchange_into_field =
        scale * result.source_exchange_into_field;
    ledger.update_residual = std::max({
        result.magnetic_update_residual,
        result.electric_pre_update_residual,
        result.global_source_free_residual});
    ledger.partition_residual = result.partition_residual;
    ledger.ledger_residual = result.regional_ledger_residual;
    ledger.valid = result.valid;
  }
  return record;
}

RegionalTick observe_initial(
    const ftd::eft::ConnectedMooreBlockState& control,
    const ftd::eft::ConnectedMooreBlockState& excited,
    const FullModes& modes,
    double initial_doublet,
    double beta,
    const Vec3& origin) {
  RegionalTick record;
  record.tick = 0;
  record.doublet_ratio = paired_doublet(control, excited, modes)
      / initial_doublet;
  const auto difference_e = subtract_face(excited.electric, control.electric);
  const auto difference_b = subtract_edge(
      excited.magnetic_half, control.magnetic_half);
  observe_positive_morphology(
      record, difference_e, difference_b, beta, initial_doublet, origin);
  for (std::size_t radius = 0; radius < regional_radii.size(); ++radius) {
    record.ledgers[radius].radius = regional_radii[radius];
    record.ledgers[radius].valid = true;
  }
  return record;
}

void classify_turning(RegionalArm& arm) {
  for (std::size_t index = 1; index + 1 < arm.ticks.size(); ++index) {
    if (arm.ticks[index].tick < 60 || arm.ticks[index].tick > 79) continue;
    if (arm.ticks[index].doublet_ratio < arm.ticks[index - 1].doublet_ratio
        && arm.ticks[index].doublet_ratio
            < arm.ticks[index + 1].doublet_ratio) {
      arm.trough_ticks.push_back(arm.ticks[index].tick);
      if (arm.ticks[index].doublet_ratio < arm.primary_ratio) {
        arm.primary_ratio = arm.ticks[index].doublet_ratio;
        arm.primary_tick = arm.ticks[index].tick;
      }
    }
  }
  if (arm.primary_tick < 71 || arm.primary_tick > 73) return;
  std::vector<const RegionalTick*> before;
  std::vector<const RegionalTick*> after;
  for (int tick : arm.trough_ticks) {
    if (tick < arm.primary_tick)
      before.push_back(&arm.ticks[static_cast<std::size_t>(tick)]);
    if (tick > arm.primary_tick)
      after.push_back(&arm.ticks[static_cast<std::size_t>(tick)]);
  }
  if (before.size() < 3 || after.size() < 2) return;
  const std::size_t n = before.size();
  const bool descending = before[n - 3]->doublet_ratio
          > before[n - 2]->doublet_ratio
      && before[n - 2]->doublet_ratio > before[n - 1]->doublet_ratio
      && before[n - 1]->doublet_ratio > arm.primary_ratio;
  const bool ascending = arm.primary_ratio < after[0]->doublet_ratio
      && after[0]->doublet_ratio < after[1]->doublet_ratio;
  arm.recovery_increment = after[1]->doublet_ratio - arm.primary_ratio;
  arm.turning = descending && ascending && arm.recovery_increment >= 0.05;
}

void classify_flow(RegionalArm& arm) {
  for (const auto& tick : arm.ticks) {
    if (tick.tick == 0) continue;
    for (std::size_t radius = 0; radius < regional_radii.size(); ++radius) {
      const auto& ledger = tick.ledgers[radius];
      if (tick.tick <= 67) {
        arm.metrics.outward_before_68[radius] += std::max(
            -ledger.boundary_transport_into, 0.0);
      }
      if (tick.tick >= 68) {
        arm.metrics.inward_68_80[radius] += std::max(
            ledger.boundary_transport_into, 0.0);
        arm.metrics.exchange_68_80[radius] +=
            ledger.source_exchange_into_field;
      }
      arm.metrics.net_outward[radius] -= ledger.boundary_transport_into;
    }
  }
  arm.source_exchange_consistency = std::max({
      std::abs(arm.metrics.exchange_68_80[0]
               - arm.metrics.exchange_68_80[1]),
      std::abs(arm.metrics.exchange_68_80[0]
               - arm.metrics.exchange_68_80[2]),
      std::abs(arm.metrics.exchange_68_80[1]
               - arm.metrics.exchange_68_80[2])});

  bool bidirectional = arm.metrics.outward_before_68[2] >= 0.05;
  bool any_return = false;
  bool one_pass = arm.metrics.outward_before_68[2] >= 0.05;
  for (std::size_t radius = 0; radius < regional_radii.size(); ++radius) {
    const double outward = arm.metrics.outward_before_68[radius];
    const double inward = arm.metrics.inward_68_80[radius];
    any_return = any_return
        || (inward >= 0.01 && outward > 0.0 && inward / outward >= 0.05);
    one_pass = one_pass && outward > 0.0 && inward <= 0.001
        && inward / outward <= 0.01;
  }
  bidirectional = bidirectional && any_return;
  const bool near_bound = !arm.ticks.empty()
      && arm.ticks.back().near_fraction >= 0.50
      && arm.metrics.net_outward[2] <= 0.01;
  if (bidirectional) arm.transport_class = "BIDIRECTIONAL_CAUSAL_FLOW";
  else if (one_pass) arm.transport_class = "ONE_PASS_OUTGOING_FLOW";
  else if (near_bound) arm.transport_class = "NEAR_BOUND_FLOW";

  const double exchange = arm.metrics.exchange_68_80[0];
  if (exchange <= -0.01)
    arm.exchange_class = "DYNAMIC_FIELD_TO_CURRENT";
  else if (exchange >= 0.01)
    arm.exchange_class = "CURRENT_TO_DYNAMIC_FIELD";
}

std::array<RegionalArm, 2> run_regional(
    const FullModes& modes,
    double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    bool& initial_fields_equal) {
  std::array<RegionalArm, 2> arms;
  arms[0].sign = -1;
  arms[1].sign = +1;
  const auto control_initial = regional_reference();
  std::array<ftd::eft::ConnectedMooreBlockState, 3> initial;
  initial[0] = control_initial;
  for (int sign = 0; sign < 2; ++sign) {
    double nominal = 0.0;
    initial[sign + 1] = regional_excitation(
        control_initial, modes, arms[sign].sign, nominal);
    arms[sign].initial_doublet = paired_doublet(
        control_initial, initial[sign + 1], modes);
    arms[sign].initialized = initial[sign + 1].electric.L == regional_volume
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
    arms[sign].ticks.push_back(observe_initial(
        state[0], state[sign + 1], modes,
        arms[sign].initial_doublet, beta, origin));
  }

  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> forward_cache;
  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> reverse_cache;
  bool forward = true;
  for (int tick = 1; tick <= regional_horizon && forward; ++tick) {
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
        arm.sector = arm.sector
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
      auto record = observe_tick(
          tick, steps[0].earlier, steps[path].earlier,
          state[0], state[path], modes,
          arms[sign].initial_doublet, beta, origin,
          source_radius, drift, common);
      for (const auto& ledger : record.ledgers) {
        arms[sign].max_observer_residual = std::max({
            arms[sign].max_observer_residual,
            ledger.update_residual,
            ledger.partition_residual,
            ledger.ledger_residual});
      }
      arms[sign].max_energy_drift = std::max(
          arms[sign].max_energy_drift, drift);
      arms[sign].max_common = std::max(
          arms[sign].max_common, common);
      arms[sign].max_source_radius = std::max(
          arms[sign].max_source_radius, source_radius);
      arms[sign].ticks.push_back(std::move(record));
    }
    for (const auto& arm : arms) {
      if (arm.ticks.empty()) {
        forward = false;
        break;
      }
      for (const auto& ledger : arm.ticks.back().ledgers) {
        if (!ledger.valid) {
          forward = false;
          break;
        }
      }
    }
    if (source_radius > regional_source_radius) forward = false;
    if (tick % 10 == 0)
      std::cout << "completed regional tick " << tick << std::endl;
  }

  bool reverse = forward;
  for (int tick = regional_horizon; tick >= 1 && reverse; --tick) {
    std::array<ftd::eft::ConnectedMooreBlockStepResult, 3> steps;
#pragma omp parallel for num_threads(3)
    for (int path = 0; path < 3; ++path) {
      steps[path] = ftd::eft::solve_connected_moore_block_reverse(
          state[path], options, &reverse_cache[path]);
    }
    for (int path = 0; path < 3; ++path) {
      const double residual = common_residual(steps[path]);
      for (auto& arm : arms)
        arm.max_common = std::max(arm.max_common, residual);
      if (!steps[path].valid || !steps[path].common_action_gates_pass
          || residual > 1e-10) {
        reverse = false;
        break;
      }
      state[path] = steps[path].earlier;
    }
    if (tick % 10 == 0)
      std::cout << "reversed regional tick " << tick << std::endl;
  }

  for (int sign = 0; sign < 2; ++sign) {
    const int path = sign + 1;
    auto& arm = arms[sign];
    arm.recovery = std::max(
        ftd::eft::connected_moore_block_state_max_difference(
            initial[0], state[0]),
        ftd::eft::connected_moore_block_state_max_difference(
            initial[path], state[path]));
    arm.executed = forward && reverse && arm.initialized && arm.sector
        && static_cast<int>(arm.ticks.size()) == regional_horizon + 1
        && arm.max_source_radius <= regional_source_radius
        && arm.max_observer_residual <= 1e-10
        && arm.max_common <= 1e-10 && arm.max_energy_drift <= 1e-10
        && arm.recovery <= 1e-8;
    classify_turning(arm);
    classify_flow(arm);
    arm.executed = arm.executed && arm.source_exchange_consistency <= 1e-10;
  }
  return arms;
}

bool polarity_consistent(const std::array<RegionalArm, 2>& arms) {
  if (arms[0].primary_tick < 0 || arms[1].primary_tick < 0
      || std::abs(arms[0].primary_tick - arms[1].primary_tick) > 1
      || std::abs(arms[0].primary_ratio - arms[1].primary_ratio) > 1e-4
      || std::abs(arms[0].recovery_increment
                  - arms[1].recovery_increment) > 1e-4
      || arms[0].ticks.empty() || arms[1].ticks.empty()
      || std::abs(arms[0].ticks.back().near_fraction
                  - arms[1].ticks.back().near_fraction) > 1e-4) {
    return false;
  }
  for (std::size_t radius = 0; radius < regional_radii.size(); ++radius) {
    if (std::abs(arms[0].metrics.outward_before_68[radius]
                 - arms[1].metrics.outward_before_68[radius]) > 1e-4
        || std::abs(arms[0].metrics.inward_68_80[radius]
                    - arms[1].metrics.inward_68_80[radius]) > 1e-4
        || std::abs(arms[0].metrics.net_outward[radius]
                    - arms[1].metrics.net_outward[radius]) > 1e-4
        || std::abs(arms[0].metrics.exchange_68_80[radius]
                    - arms[1].metrics.exchange_68_80[radius]) > 1e-4) {
      return false;
    }
  }
  return true;
}

void write_regional(const std::array<RegionalArm, 2>& arms,
                    bool initial_fields_equal,
                    bool polarity,
                    const std::string& verdict) {
  const auto directory = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0672";
  std::filesystem::create_directories(directory);
  std::ofstream json(
      directory / "ftd_0672_causal_regional_field_flow_v1.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0672\",\n"
       << "  \"protocol_sha256\": \"" << regional_protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"volume\": " << regional_volume << ",\n"
       << "  \"horizon\": " << regional_horizon << ",\n"
       << "  \"maximum_constituent_momentum_amplitude\": 2e-6,\n"
       << "  \"causal_contact_tick\": " << regional_contact_tick << ",\n"
       << "  \"initial_fields_bitwise_equal\": "
       << (initial_fields_equal ? "true" : "false") << ",\n"
       << "  \"polarity_consistent\": "
       << (polarity ? "true" : "false") << ",\n";
  for (int sign = 0; sign < 2; ++sign) {
    const auto& arm = arms[sign];
    const std::string prefix = sign == 0 ? "negative" : "positive";
    json << "  \"" << prefix << "_executed\": "
         << (arm.executed ? "true" : "false") << ",\n"
         << "  \"" << prefix << "_turning\": "
         << (arm.turning ? "true" : "false") << ",\n"
         << "  \"" << prefix << "_primary_tick\": "
         << arm.primary_tick << ",\n"
         << "  \"" << prefix << "_primary_ratio\": "
         << arm.primary_ratio << ",\n"
         << "  \"" << prefix << "_recovery_increment\": "
         << arm.recovery_increment << ",\n"
         << "  \"" << prefix << "_transport_class\": \""
         << arm.transport_class << "\",\n"
         << "  \"" << prefix << "_exchange_class\": \""
         << arm.exchange_class << "\",\n"
         << "  \"" << prefix << "_source_exchange_consistency\": "
         << arm.source_exchange_consistency << ",\n"
         << "  \"" << prefix << "_max_observer_residual\": "
         << arm.max_observer_residual << ",\n"
         << "  \"" << prefix << "_recovery\": " << arm.recovery
         << ",\n";
    for (std::size_t radius = 0; radius < regional_radii.size(); ++radius) {
      const int r = static_cast<int>(regional_radii[radius]);
      json << "  \"" << prefix << "_outward_before_68_r" << r << "\": "
           << arm.metrics.outward_before_68[radius] << ",\n"
           << "  \"" << prefix << "_inward_68_80_r" << r << "\": "
           << arm.metrics.inward_68_80[radius] << ",\n"
           << "  \"" << prefix << "_net_outward_r" << r << "\": "
           << arm.metrics.net_outward[radius] << ",\n"
           << "  \"" << prefix << "_exchange_68_80_r" << r << "\": "
           << arm.metrics.exchange_68_80[radius] << ",\n";
    }
  }
  json << "  \"schema_complete\": true\n}\n";

  std::ofstream csv(
      directory / "ftd_0672_causal_regional_field_flow_ticks_v1.csv");
  csv << "ftd_id,protocol_sha256,sign,tick,radius,doublet_ratio,"
         "positive_field_norm_ratio,near_fraction,radius_second_moment,"
         "dynamic_support_radius,source_support_radius,energy_drift,"
         "common_residual,regional_energy_before,regional_energy_pre_current,"
         "regional_energy_after,boundary_transport_into,"
         "source_exchange_into_field,update_residual,partition_residual,"
         "ledger_residual,observer_valid\n";
  for (const auto& arm : arms) {
    for (const auto& tick : arm.ticks) {
      for (const auto& ledger : tick.ledgers) {
        csv << std::setprecision(17) << "FTD-0672,"
            << regional_protocol_sha256 << ',' << arm.sign << ','
            << tick.tick << ',' << ledger.radius << ','
            << tick.doublet_ratio << ',' << tick.positive_field_norm_ratio
            << ',' << tick.near_fraction << ',' << tick.radius_second_moment
            << ',' << tick.dynamic_support_radius << ','
            << tick.source_support_radius << ',' << tick.energy_drift << ','
            << tick.common_residual << ',' << ledger.energy_before << ','
            << ledger.energy_pre_current << ',' << ledger.energy_after << ','
            << ledger.boundary_transport_into << ','
            << ledger.source_exchange_into_field << ','
            << ledger.update_residual << ',' << ledger.partition_residual << ','
            << ledger.ledger_residual << ',' << (ledger.valid ? 1 : 0) << '\n';
      }
    }
  }
}

}  // namespace

#ifdef FTD_0672_EMBEDDED
int ftd_0672_embedded_main() {
#else
int main() {
#endif
  const bool parent = regional_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  options.use_sparse_local_current = true;
  FullModes modes;
  if (parent && normalization.valid) {
    const auto reference = load_refined_state(0);
    const auto analytic = analytic_at(
        "causal_regional_field_flow", 0, reference,
        normalization.mapped_field_work_coefficient, options);
    if (analytic.valid) modes = full_modes(analytic.hessian);
  }
  bool initial_fields_equal = false;
  std::array<RegionalArm, 2> arms;
  if (modes.valid && modes.modes[6].group == modes.modes[7].group) {
    arms = run_regional(
        modes, normalization.mapped_field_work_coefficient,
        options, initial_fields_equal);
  }
  const bool execution = parent && normalization.valid && modes.valid
      && initial_fields_equal && regional_horizon < regional_contact_tick
      && arms[0].executed && arms[1].executed;
  const bool polarity = execution && polarity_consistent(arms)
      && arms[0].transport_class == arms[1].transport_class
      && arms[0].exchange_class == arms[1].exchange_class;
  const bool turning = polarity && arms[0].turning && arms[1].turning;
  std::string verdict = "CAUSAL_REGIONAL_FIELD_FLOW_MIXED";
  if (!execution || !polarity) {
    verdict = "CAUSAL_REGIONAL_FIELD_FLOW_EXECUTION_INVALID";
  } else if (turning
             && arms[0].transport_class == "BIDIRECTIONAL_CAUSAL_FLOW"
             && arms[0].exchange_class == "DYNAMIC_FIELD_TO_CURRENT") {
    verdict = "CAUSAL_BIDIRECTIONAL_FIELD_MEMORY_CONSTRUCTIVE";
  } else if (turning
             && arms[0].transport_class == "ONE_PASS_OUTGOING_FLOW") {
    verdict = "CAUSAL_ONE_PASS_OUTGOING_FIELD_CONSTRUCTIVE";
  } else if (turning
             && arms[0].transport_class == "NEAR_BOUND_FLOW") {
    verdict = "CAUSAL_NEAR_BOUND_DRESSING_CONSTRUCTIVE";
  }
  write_regional(arms, initial_fields_equal, polarity, verdict);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << regional_protocol_sha256 << '\n'
            << "verdict=" << verdict << '\n'
            << "execution=" << execution << " polarity=" << polarity
            << " turning=" << turning << '\n';
  for (const auto& arm : arms) {
    std::cout << "sign=" << arm.sign
              << " primary_tick=" << arm.primary_tick
              << " recovery_increment=" << arm.recovery_increment
              << " transport=" << arm.transport_class
              << " exchange=" << arm.exchange_class
              << " O24=" << arm.metrics.outward_before_68[2]
              << " I8=" << arm.metrics.inward_68_80[0]
              << " I16=" << arm.metrics.inward_68_80[1]
              << " I24=" << arm.metrics.inward_68_80[2]
              << " X=" << arm.metrics.exchange_68_80[0]
              << " observer=" << arm.max_observer_residual
              << " inverse=" << arm.recovery << '\n';
  }
  return execution ? 0 : 1;
}
