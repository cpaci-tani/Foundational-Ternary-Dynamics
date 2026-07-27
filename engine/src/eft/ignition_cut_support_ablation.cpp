#include "ftd/eft/ignition_cut_support_ablation.h"

#include "ftd/constants.h"
#include "ftd/eft/emergent_boundary_observer.h"
#include "ftd/eft/history_event_journal.h"
#include "ftd/poisson_solvers.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <numeric>
#include <tuple>
#include <vector>

namespace ftd::eft {
namespace {

constexpr int PREFIX_TICKS = 150;
constexpr int FINAL_TICK = 300;
constexpr std::array<int, 6> SAMPLE_TICKS{{150, 180, 210, 240, 270, 300}};
constexpr std::array<int, 2> VOLUMES{{24, 32}};
constexpr std::array<int, 3> AMPLITUDES{{12, 20, 40}};
constexpr std::array<std::uint32_t, 4> SEEDS{{
    0xE0102000u, 0xE0102001u, 0xE0102002u, 0xE0102003u}};
constexpr std::array<IgnitionCutArm, 6> ARMS{{
    IgnitionCutArm::IntactReservoir,
    IgnitionCutArm::IntactCausal,
    IgnitionCutArm::IntactProjected,
    IgnitionCutArm::ClearedControl,
    IgnitionCutArm::ClearedCausal,
    IgnitionCutArm::ClearedProjected}};

using PrefixKey = std::tuple<int, int, std::uint32_t>;

void hash_bytes(std::uint64_t& hash, const void* data, std::size_t size) {
  const auto* bytes = static_cast<const unsigned char*>(data);
  for (std::size_t i = 0; i < size; ++i) {
    hash ^= static_cast<std::uint64_t>(bytes[i]);
    hash *= 1099511628211ull;
  }
}

template <typename T>
void hash_value(std::uint64_t& hash, const T& value) {
  hash_bytes(hash, &value, sizeof(value));
}

std::uint64_t selected_state_hash(const RenderBridge& bridge) {
  std::uint64_t hash = 1469598103934665603ull;
  hash_value(hash, bridge.current_tick());
  hash_value(hash, bridge.physical_time());
  for (const auto& voxel : bridge.voxels()) {
    hash_value(hash, voxel.state);
    for (const auto& value : {voxel.flux, voxel.wave_vel, voxel.flux_L,
                             voxel.flux_R, voxel.wave_vel_L,
                             voxel.wave_vel_R, voxel.velocity,
                             voxel.remainder, voxel.flux_strong,
                             voxel.wave_vel_strong, voxel.flux_weak,
                             voxel.wave_vel_weak}) {
      hash_value(hash, value.x);
      hash_value(hash, value.y);
      hash_value(hash, value.z);
    }
    hash_value(hash, voxel.latency);
    hash_value(hash, voxel.tau);
    hash_value(hash, voxel.phase);
    hash_value(hash, voxel.locked);
    hash_value(hash, voxel.particle_id);
    hash_value(hash, voxel.pair_id);
    hash_value(hash, voxel.spin);
    hash_value(hash, voxel.color);
    hash_value(hash, voxel.flavor);
    hash_value(hash, voxel.accel_mag);
  }
  return hash;
}

std::uint64_t support_hash(const RenderBridge& bridge) {
  std::uint64_t hash = 1469598103934665603ull;
  for (const auto& voxel : bridge.voxels()) {
    hash_value(hash, voxel.state);
    hash_value(hash, voxel.locked);
    hash_value(hash, voxel.particle_id);
    hash_value(hash, voxel.pair_id);
    hash_value(hash, voxel.spin);
    hash_value(hash, voxel.color);
    hash_value(hash, voxel.flavor);
  }
  return hash;
}

double max_abs(Vec3 value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double mean(const std::vector<double>& values) {
  if (values.empty()) return 0.0;
  return std::accumulate(values.begin(), values.end(), 0.0)
      / static_cast<double>(values.size());
}

double coefficient_of_variation(const std::vector<double>& values) {
  if (values.empty()) return 0.0;
  const double average = mean(values);
  if (std::abs(average) <= 1e-30) return 0.0;
  double variance = 0.0;
  for (double value : values)
    variance += (value - average) * (value - average);
  variance /= static_cast<double>(values.size());
  return std::sqrt(variance) / std::abs(average);
}

double quadratic_field_norm(const RenderBridge& bridge) {
  double value = 0.0;
  for (const auto& voxel : bridge.voxels())
    value += 0.5 * (voxel.flux.mag2() + voxel.wave_vel.mag2());
  return value;
}

void maximum_field_components(const RenderBridge& bridge,
                              double& maximum_flux,
                              double& maximum_wave_velocity) {
  for (const auto& voxel : bridge.voxels()) {
    maximum_flux = std::max(maximum_flux, voxel.flux.mag());
    maximum_wave_velocity = std::max(
        maximum_wave_velocity, voxel.wave_vel.mag());
  }
}

void maximum_kinematics(const RenderBridge& bridge,
                        double& maximum_velocity,
                        double& maximum_remainder) {
  for (const auto& voxel : bridge.voxels()) {
    maximum_velocity = std::max(maximum_velocity,
                                max_abs(voxel.velocity));
    maximum_remainder = std::max(maximum_remainder,
                                 max_abs(voxel.remainder));
  }
}

void configure_prefix(RenderBridge& bridge, std::uint32_t seed) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.wave_propagation = true;
  bridge.toggles.gauss_projection = true;
  bridge.toggles.genesis = true;
  bridge.toggles.dual_substrate = false;
  bridge.toggles.reflective_boundary = false;
  bridge.toggles.flux_boundary = FluxBoundaryMode::Dispersal;
  bridge.toggles.langevin_seed = seed;
  bridge.seed_rng(seed);
}

bool is_cleared(IgnitionCutArm arm) {
  return arm == IgnitionCutArm::ClearedControl
      || arm == IgnitionCutArm::ClearedCausal
      || arm == IgnitionCutArm::ClearedProjected;
}

bool is_causal(IgnitionCutArm arm) {
  return arm == IgnitionCutArm::IntactCausal
      || arm == IgnitionCutArm::ClearedCausal;
}

bool is_projected(IgnitionCutArm arm) {
  return arm == IgnitionCutArm::IntactProjected
      || arm == IgnitionCutArm::ClearedProjected;
}

void configure_continuation(RenderBridge& bridge, IgnitionCutArm arm) {
  bridge.toggles.wave_propagation = true;
  bridge.toggles.genesis = true;
  bridge.toggles.coupling = is_causal(arm);
  bridge.toggles.gauss_projection = is_projected(arm);
}

void zero_field(RenderBridge& bridge) {
  for (auto& voxel : bridge.voxels()) {
    voxel.flux = {};
    voxel.wave_vel = {};
    voxel.flux_L = {};
    voxel.flux_R = {};
    voxel.wave_vel_L = {};
    voxel.wave_vel_R = {};
  }
}

void apply_registered_gauss_projection(RenderBridge& bridge) {
  std::vector<double> phi(bridge.lattice().total_sites(), 0.0);
  std::vector<double> source(bridge.lattice().total_sites(), 0.0);
  auto& voxels = bridge.voxels();
  const auto& state = bridge.ternary_field();
  gauss_project_cpu(voxels, state, phi, source, bridge.lattice(),
                    bridge.toggles.dual_substrate,
                    bridge.toggles.exact_dual_gauss,
                    bridge.toggles.coulomb_charge_coupling,
                    bridge.sor_iterations());
}

void rebase_kinematics(RenderBridge& bridge) {
  for (auto& voxel : bridge.voxels()) {
    voxel.velocity = {};
    voxel.remainder = {};
  }
}

bool is_sample_tick(int tick) {
  return std::find(SAMPLE_TICKS.begin(), SAMPLE_TICKS.end(), tick)
      != SAMPLE_TICKS.end();
}

void count_event(const HistoryEvent& event, IgnitionCutRunRecord& record) {
  switch (event.kind) {
    case HistoryEventKind::Genesis: ++record.genesis_events; break;
    case HistoryEventKind::Evaporation: ++record.evaporation_events; break;
    case HistoryEventKind::Movement: ++record.movement_events; break;
    case HistoryEventKind::Annihilation: ++record.annihilation_events; break;
    default: break;
  }
}

IgnitionCutRunRecord run_one(IgnitionCutArm arm, int L, int amplitude,
                             std::uint32_t seed) {
  IgnitionCutRunRecord record;
  record.arm = arm;
  record.lattice_size = L;
  record.amplitude = amplitude;
  record.seed = seed;

  RenderBridge bridge(L);
  configure_prefix(bridge, seed);
  if (!bridge.enable_history_journal(true)) return record;
  const int center = L / 2;
  bridge.inject_flux(center, center, center,
                     {amplitude * K_GENESIS, 0.0, 0.0});
  for (int tick = 1; tick <= PREFIX_TICKS; ++tick) {
    bridge.tick();
    bridge.clear_history_events();
  }

  record.prefix_state_hash = selected_state_hash(bridge);
  record.prefix_rng_hash = bridge.rng_state_hash();
  record.support_hash_before = support_hash(bridge);
  record.cut_quadratic_field_norm = quadratic_field_norm(bridge);
  maximum_kinematics(bridge, record.maximum_velocity_before_rebase,
                     record.maximum_remainder_before_rebase);
  record.prefix_kinematics_clean =
      record.maximum_velocity_before_rebase == 0.0
      && record.maximum_remainder_before_rebase == 0.0;
  rebase_kinematics(bridge);

  configure_continuation(bridge, arm);
  if (is_cleared(arm)) zero_field(bridge);
  if (arm == IgnitionCutArm::ClearedProjected)
    apply_registered_gauss_projection(bridge);

  record.support_hash_after_intervention = support_hash(bridge);
  record.support_preserved_by_intervention =
      record.support_hash_before == record.support_hash_after_intervention;
  record.post_intervention_field_norm = quadratic_field_norm(bridge);
  record.maximum_quadratic_field_norm =
      record.post_intervention_field_norm;
  maximum_field_components(bridge, record.maximum_flux,
                           record.maximum_wave_velocity);

  std::vector<EmergentBoundaryObservation> samples;
  samples.reserve(SAMPLE_TICKS.size());
  samples.push_back(observe_emergent_boundary(
      bridge, center, center, center));
  record.cut_occupancy = samples.back().occupancy;

  for (int tick = PREFIX_TICKS + 1; tick <= FINAL_TICK; ++tick) {
    bridge.tick();
    for (const auto& event : bridge.history_events()) count_event(event, record);
    bridge.clear_history_events();

    const double norm = quadratic_field_norm(bridge);
    record.maximum_quadratic_field_norm = std::max(
        record.maximum_quadratic_field_norm, norm);
    maximum_field_components(bridge, record.maximum_flux,
                             record.maximum_wave_velocity);
    const auto audit = bridge.energy_audit();
    record.maximum_gauss_error = std::max(
        record.maximum_gauss_error, audit.max_gauss_error);
    maximum_kinematics(bridge, record.maximum_velocity_after_rebase,
                       record.maximum_remainder_after_rebase);
    if (is_sample_tick(tick)) {
      samples.push_back(observe_emergent_boundary(
          bridge, center, center, center));
    }
  }

  const auto diagnostics = bridge.diagnostics();
  record.final_positive = diagnostics.positive_count;
  record.final_negative = diagnostics.negative_count;
  record.final_quadratic_field_norm = quadratic_field_norm(bridge);
  record.final_occupancy = samples.empty() ? 0 : samples.back().occupancy;
  record.sample_count = static_cast<int>(samples.size());
  record.minimum_sample_occupancy = std::numeric_limits<int>::max();
  record.maximum_sample_occupancy = 0;

  bool size_gate = samples.size() == SAMPLE_TICKS.size();
  bool finite = size_gate;
  bool all_samples_valid = size_gate;
  std::vector<double> occupancies;
  std::vector<double> radii;
  for (const auto& sample : samples) {
    all_samples_valid = all_samples_valid && sample.valid;
    record.minimum_sample_occupancy = std::min(
        record.minimum_sample_occupancy, sample.occupancy);
    record.maximum_sample_occupancy = std::max(
        record.maximum_sample_occupancy, sample.occupancy);
    size_gate = size_gate && sample.valid
        && sample.occupancy >= 4
        && sample.occupancy <= static_cast<int>(0.01 * L * L * L);
    occupancies.push_back(static_cast<double>(sample.occupancy));
    radii.push_back(sample.rms_radius);
    const std::array<double, 8> scalars{{
        sample.centroid_x, sample.centroid_y, sample.centroid_z,
        sample.rms_radius, sample.volume_radius, sample.area_coefficient,
        sample.laplace_coefficient, sample.wave_kinetic_energy}};
    for (double value : scalars) finite = finite && std::isfinite(value);
  }
  record.occupancy_cv = coefficient_of_variation(occupancies);
  record.radius_cv = coefficient_of_variation(radii);
  if (samples.empty()) record.minimum_sample_occupancy = 0;
  record.all_samples_valid = all_samples_valid;
  record.size_gate = size_gate;
  record.stable = size_gate && record.occupancy_cv <= 0.20
      && record.radius_cv <= 0.15;
  record.finite = finite
      && std::isfinite(record.cut_quadratic_field_norm)
      && std::isfinite(record.post_intervention_field_norm)
      && std::isfinite(record.final_quadratic_field_norm)
      && std::isfinite(record.maximum_gauss_error);
  return record;
}

bool observer_neutrality() {
  RenderBridge control(24);
  RenderBridge observed(24);
  constexpr std::uint32_t seed = SEEDS[0];
  configure_prefix(control, seed);
  configure_prefix(observed, seed);
  const int center = 12;
  control.inject_flux(center, center, center,
                      {20.0 * K_GENESIS, 0.0, 0.0});
  observed.inject_flux(center, center, center,
                       {20.0 * K_GENESIS, 0.0, 0.0});
  if (!observed.enable_history_journal(true)) return false;
  for (int tick = 0; tick < 64; ++tick) {
    control.tick();
    observed.tick();
    observed.clear_history_events();
  }
  return selected_state_hash(control) == selected_state_hash(observed)
      && control.rng_state_hash() == observed.rng_state_hash();
}

}  // namespace

const char* ignition_cut_arm_name(IgnitionCutArm arm) {
  switch (arm) {
    case IgnitionCutArm::IntactReservoir: return "intact_reservoir";
    case IgnitionCutArm::IntactCausal: return "intact_causal";
    case IgnitionCutArm::IntactProjected: return "intact_projected";
    case IgnitionCutArm::ClearedControl: return "cleared_control";
    case IgnitionCutArm::ClearedCausal: return "cleared_causal";
    case IgnitionCutArm::ClearedProjected: return "cleared_projected";
  }
  return "unknown";
}

IgnitionCutSupportAblationResult
analyze_ignition_cut_support_ablation() {
  IgnitionCutSupportAblationResult result;
  result.prefix_ticks = PREFIX_TICKS;
  result.continuation_ticks = FINAL_TICK - PREFIX_TICKS;
  result.runs.reserve(VOLUMES.size() * AMPLITUDES.size()
                      * SEEDS.size() * ARMS.size());

  std::map<PrefixKey, std::pair<std::uint64_t, std::uint64_t>> prefixes;
  for (int L : VOLUMES) {
    for (int amplitude : AMPLITUDES) {
      for (std::uint32_t seed : SEEDS) {
        const PrefixKey key{L, amplitude, seed};
        for (IgnitionCutArm arm : ARMS) {
          auto run = run_one(arm, L, amplitude, seed);
          const auto [it, inserted] = prefixes.emplace(
              key, std::make_pair(run.prefix_state_hash,
                                  run.prefix_rng_hash));
          if (!inserted) {
            if (it->second.first != run.prefix_state_hash)
              ++result.prefix_hash_mismatches;
            if (it->second.second != run.prefix_rng_hash)
              ++result.prefix_rng_mismatches;
          }
          if (!run.support_preserved_by_intervention)
            ++result.intervention_support_mismatches;
          if (!run.finite) ++result.nonfinite_runs;
          if (!run.prefix_kinematics_clean)
            ++result.dirty_prefix_kinematics_runs;
          if (run.maximum_velocity_after_rebase != 0.0
              || run.maximum_remainder_after_rebase != 0.0)
            ++result.post_rebase_kinematics_runs;
          if (run.movement_events != 0 || run.annihilation_events != 0)
            ++result.forbidden_event_runs;
          result.runs.push_back(run);
        }
      }
    }
  }
  result.distinct_prefix_cells = static_cast<int>(prefixes.size());
  result.run_count = static_cast<int>(result.runs.size());
  result.total_ticks = result.run_count * FINAL_TICK;

  for (std::size_t arm_index = 0; arm_index < ARMS.size(); ++arm_index) {
    auto& summary = result.arms[arm_index];
    summary.arm = ARMS[arm_index];
    std::vector<double> cut_norms;
    std::vector<double> post_norms;
    std::vector<double> final_norms;
    for (const auto& run : result.runs) {
      if (run.arm != summary.arm) continue;
      ++summary.run_count;
      if (run.stable) ++summary.stable_runs;
      summary.genesis_events += run.genesis_events;
      summary.evaporation_events += run.evaporation_events;
      summary.movement_events += run.movement_events;
      summary.annihilation_events += run.annihilation_events;
      summary.maximum_gauss_error = std::max(
          summary.maximum_gauss_error, run.maximum_gauss_error);
      cut_norms.push_back(run.cut_quadratic_field_norm);
      post_norms.push_back(run.post_intervention_field_norm);
      final_norms.push_back(run.final_quadratic_field_norm);
    }
    summary.mean_cut_field_norm = mean(cut_norms);
    summary.mean_post_intervention_field_norm = mean(post_norms);
    summary.mean_final_field_norm = mean(final_norms);
    for (int L : VOLUMES) {
      for (int amplitude : AMPLITUDES) {
        int stable_seeds = 0;
        for (const auto& run : result.runs) {
          if (run.arm == summary.arm && run.lattice_size == L
              && run.amplitude == amplitude && run.stable)
            ++stable_seeds;
        }
        if (stable_seeds >= 3) ++summary.passing_cells;
      }
    }
    summary.support_qualified = summary.passing_cells >= 5;
  }

  const auto& reservoir = result.arms[
      static_cast<std::size_t>(IgnitionCutArm::IntactReservoir)];
  const auto& projected = result.arms[
      static_cast<std::size_t>(IgnitionCutArm::IntactProjected)];
  const auto& cleared_control = result.arms[
      static_cast<std::size_t>(IgnitionCutArm::ClearedControl)];
  const auto& cleared_causal = result.arms[
      static_cast<std::size_t>(IgnitionCutArm::ClearedCausal)];
  const auto& cleared_projected = result.arms[
      static_cast<std::size_t>(IgnitionCutArm::ClearedProjected)];

  result.intact_projected_reproduced =
      projected.stable_runs == 20 && projected.passing_cells == 5;
  result.reservoir_sufficient = reservoir.support_qualified;
  result.causal_state_source_sufficient =
      cleared_causal.support_qualified
      && !cleared_control.support_qualified;
  result.gauss_constraint_sufficient =
      cleared_projected.support_qualified
      && !cleared_control.support_qualified;
  result.state_only_persistence = cleared_control.support_qualified;
  const int sufficient_count =
      static_cast<int>(result.reservoir_sufficient)
      + static_cast<int>(result.causal_state_source_sufficient)
      + static_cast<int>(result.gauss_constraint_sufficient)
      + static_cast<int>(result.state_only_persistence);
  const auto& intact_causal = result.arms[
      static_cast<std::size_t>(IgnitionCutArm::IntactCausal)];
  const bool interaction_without_isolated_component =
      (intact_causal.support_qualified
       && !reservoir.support_qualified
       && !cleared_causal.support_qualified)
      || (projected.support_qualified
          && !reservoir.support_qualified
          && !cleared_projected.support_qualified);
  result.mixed_or_unresolved = sufficient_count > 1
      || interaction_without_isolated_component;
  result.no_registered_support_mechanism = sufficient_count == 0;
  result.observer_neutral = observer_neutrality();
  result.production_changed = false;
  result.valid = result.run_count == 144
      && result.distinct_prefix_cells == 24
      && result.prefix_hash_mismatches == 0
      && result.prefix_rng_mismatches == 0
      && result.intervention_support_mismatches == 0
      && result.nonfinite_runs == 0
      && result.dirty_prefix_kinematics_runs == 0
      && result.post_rebase_kinematics_runs == 0
      && result.forbidden_event_runs == 0
      && result.intact_projected_reproduced
      && result.observer_neutral
      && !result.production_changed;
  return result;
}

}  // namespace ftd::eft
