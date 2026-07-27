/**
 * @file campaign_dynamical_flux_dressing.cpp
 * @brief FTD-0476 source-built dressing / movement / release campaign v2.
 */

#include "ftd/eft/dynamical_flux_dressing_observer.h"
#include "ftd/eft/history_event_journal.h"
#include "ftd/scenarios.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr std::array<int, 2> kLengths{{49, 65}};
constexpr int kBuildTick = 12;
constexpr int kFinalTick = 24;

enum class ArmKind { Stationary, Empty, Moving, Release };

struct Sample {
  int tick = 0;
  int source_index = -1;
  int source_x = -1;
  int source_y = -1;
  int source_z = -1;
  ftd::eft::DynamicalDressingObservation observation;
  double relative_release_energy_drift = 0.0;
};

struct Run {
  std::string arm;
  int length = 0;
  int polarity = +1;
  ArmKind kind = ArmKind::Stationary;
  std::vector<Sample> samples;
  int movement_events = 0;
  int reaction_events = 0;
  bool valid = false;
};

int find_unique_source(const ftd::RenderBridge& bridge) {
  int source = -1;
  for (int index = 0;
       index < static_cast<int>(bridge.voxels().size()); ++index) {
    if (bridge.voxels()[static_cast<std::size_t>(index)].state == 0) continue;
    if (source >= 0) return -2;
    source = index;
  }
  return source;
}

bool frozen_source_terms(const ftd::RenderBridge& bridge,
                         bool movement_expected) {
  const auto& t = bridge.toggles;
  return t.wave_propagation && t.coupling
      && t.movement == movement_expected
      && t.flux_boundary == ftd::FluxBoundaryMode::Periodic
      && !t.dual_substrate && !t.gauss_projection
      && !t.matched_gauss_dynamics && !t.damping && !t.forces
      && !t.gravity && !t.poisson_coulomb && !t.lorentz_force
      && !t.genesis && !t.evaporation && !t.pair_production
      && !t.weak_transmutation && !t.emergent_forces
      && !t.langevin && !t.symplectic_leapfrog
      && !t.verlet_wave_integrator && !t.lorentz_period2_floquet
      && !t.lorentz_bcc_time_floquet;
}

bool initialize_scenario(ftd::RenderBridge& bridge, int polarity) {
  bridge.force_cpu();
  if (!ftd::dispatch_scenario(
          bridge, "s0-seed-dynamical-flux-dressing")) return false;
  const int source = find_unique_source(bridge);
  if (source < 0) return false;
  bridge.set_state(source, static_cast<std::int8_t>(polarity));
  bridge.voxels()[static_cast<std::size_t>(source)].locked = true;
  return frozen_source_terms(bridge, false);
}

Sample observe(const ftd::RenderBridge& bridge, int tick, int source_index,
               int polarity, long double release_energy_reference = 0.0L,
               bool compare_release_energy = false) {
  Sample sample;
  sample.tick = tick;
  sample.source_index = source_index;
  const auto coordinate = bridge.lattice().coord(source_index);
  sample.source_x = coordinate.x;
  sample.source_y = coordinate.y;
  sample.source_z = coordinate.z;
  sample.observation = ftd::eft::observe_dynamical_flux_dressing(
      bridge, source_index, polarity, 4.0, 6.0, +1);
  if (compare_release_energy) {
    sample.relative_release_energy_drift = std::abs(static_cast<double>(
        sample.observation.exact_tick_energy - release_energy_reference))
        / std::max(1e-30,
                   std::abs(static_cast<double>(release_energy_reference)));
  }
  return sample;
}

Run run_arm(int length, ArmKind kind, int polarity) {
  Run out;
  out.length = length;
  out.polarity = polarity;
  out.kind = kind;
  out.arm = kind == ArmKind::Stationary ?
      (polarity > 0 ? "stationary_plus" : "stationary_minus")
      : (kind == ArmKind::Empty ? "empty_control"
         : (kind == ArmKind::Moving ? "production_moving" : "source_off"));

  ftd::RenderBridge bridge(length);
  bool initialized = initialize_scenario(bridge, polarity);
  const int original_source = find_unique_source(bridge);
  if (kind == ArmKind::Empty && original_source >= 0) {
    bridge.set_state(original_source, 0);
    bridge.voxels()[static_cast<std::size_t>(original_source)].locked = false;
  }
  if (kind == ArmKind::Moving)
    initialized = initialized && bridge.enable_history_journal(true);

  long double release_energy = 0.0L;
  bool exact_state_history = true;
  for (int tick = 0; tick <= kFinalTick; ++tick) {
    int source = find_unique_source(bridge);
    const int observer_source = source >= 0 ? source : original_source;
    out.samples.push_back(observe(
        bridge, tick, observer_source, polarity, release_energy,
        kind == ArmKind::Release && tick > kBuildTick));

    if (kind == ArmKind::Stationary) {
      exact_state_history = exact_state_history && source == original_source
          && bridge.voxels()[static_cast<std::size_t>(source)].locked;
    } else if (kind == ArmKind::Empty ||
               (kind == ArmKind::Release && tick > kBuildTick)) {
      exact_state_history = exact_state_history && source == -1;
    } else if (kind == ArmKind::Moving) {
      exact_state_history = exact_state_history && source >= 0;
    }

    if (tick == kFinalTick) break;
    if (tick == kBuildTick) {
      if (kind == ArmKind::Moving) {
        auto& voxel = bridge.voxels()[static_cast<std::size_t>(original_source)];
        voxel.locked = false;
        voxel.velocity = {ftd::C_SPEED, 0.0, 0.0};
        voxel.remainder = {};
        bridge.toggles.movement = true;
      } else if (kind == ArmKind::Release) {
        release_energy = out.samples.back().observation.exact_tick_energy;
        bridge.set_state(original_source, 0);
        auto& voxel = bridge.voxels()[static_cast<std::size_t>(original_source)];
        voxel.locked = false;
        voxel.velocity = {};
        voxel.remainder = {};
      }
    }
    bridge.tick();
    if (kind == ArmKind::Moving) {
      // The production bridge clears the immutable observer journal at the
      // start of each tick.  Consume the completed tick's events now; reading
      // only after the run silently discards every earlier movement record.
      for (const auto& event : bridge.history_events()) {
        if (event.kind == ftd::eft::HistoryEventKind::Movement)
          ++out.movement_events;
        else
          ++out.reaction_events;
      }
    }
  }
  bool observations_valid = out.samples.size() == kFinalTick + 1;
  for (const auto& sample : out.samples)
    observations_valid = observations_valid && sample.observation.valid
        && std::isfinite(sample.relative_release_energy_drift);
  const bool terms_valid = frozen_source_terms(
      bridge, kind == ArmKind::Moving);
  bool journal_consistent = true;
  if (kind == ArmKind::Moving && out.samples.size() == kFinalTick + 1) {
    const auto& start = out.samples[static_cast<std::size_t>(kBuildTick)];
    const auto& final = out.samples.back();
    const int net_x_hops = final.source_x - start.source_x;
    journal_consistent = net_x_hops >= 0
        && final.source_y == start.source_y
        && final.source_z == start.source_z
        && out.movement_events == net_x_hops
        && out.reaction_events == 0;
  }
  out.valid = initialized && observations_valid && exact_state_history
      && terms_valid && journal_consistent;
  return out;
}

const Run* find_run(const std::vector<Run>& runs, int length,
                    const std::string& arm) {
  for (const auto& run : runs)
    if (run.length == length && run.arm == arm) return &run;
  return nullptr;
}

bool observer_neutrality() {
  ftd::RenderBridge observed(33), control(33);
  if (!initialize_scenario(observed, +1)
      || !initialize_scenario(control, +1)) return false;
  const int source = find_unique_source(observed);
  for (int tick = 0; tick < kFinalTick; ++tick) {
    (void)ftd::eft::observe_dynamical_flux_dressing(
        observed, source, +1);
    observed.tick();
    control.tick();
  }
  return ftd::eft::dynamical_dressing_state_hash(observed)
          == ftd::eft::dynamical_dressing_state_hash(control)
      && observed.rng_state_hash() == control.rng_state_hash();
}

}  // namespace

int main() {
  const fs::path output_dir = fs::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0476";
  fs::create_directories(output_dir);

  std::vector<Run> runs;
  for (const int length : kLengths) {
    runs.push_back(run_arm(length, ArmKind::Stationary, +1));
    runs.push_back(run_arm(length, ArmKind::Stationary, -1));
    runs.push_back(run_arm(length, ArmKind::Empty, +1));
    runs.push_back(run_arm(length, ArmKind::Moving, +1));
    runs.push_back(run_arm(length, ArmKind::Release, +1));
  }

  std::ofstream csv(output_dir / "dynamical_flux_dressing_v2.csv");
  csv << std::setprecision(17)
      << "arm,L,polarity,tick,source_index,source_x,source_y,source_z,"
         "activity,field_norm2,wave_norm2,mean_radius,near_activity,"
         "near_fraction,leading_activity,trailing_activity,"
         "transverse_activity,leading_fraction,trailing_fraction,"
         "transverse_fraction,radial_alignment,signed_source_divergence,"
         "manifested_count,max_support_radius,exact_tick_energy,"
         "relative_release_energy_drift,movement_events,reaction_events,valid\n";
  int rows = 0;
  bool structural_valid = runs.size() == 10;
  for (const auto& run : runs) {
    structural_valid = structural_valid && run.valid;
    for (const auto& sample : run.samples) {
      const auto& o = sample.observation;
      csv << run.arm << ',' << run.length << ',' << run.polarity << ','
          << sample.tick << ',' << sample.source_index << ','
          << sample.source_x << ',' << sample.source_y << ','
          << sample.source_z << ',' << o.activity << ',' << o.field_norm2
          << ',' << o.wave_norm2 << ',' << o.mean_radius << ','
          << o.near_activity << ',' << o.near_fraction << ','
          << o.leading_activity << ',' << o.trailing_activity << ','
          << o.transverse_activity << ',' << o.leading_fraction << ','
          << o.trailing_fraction << ',' << o.transverse_fraction << ','
          << o.radial_alignment << ',' << o.signed_source_divergence << ','
          << o.manifested_count << ',' << o.max_support_radius << ','
          << static_cast<double>(o.exact_tick_energy) << ','
          << sample.relative_release_energy_drift << ','
          << run.movement_events << ',' << run.reaction_events << ','
          << run.valid << '\n';
      ++rows;
    }
  }
  structural_valid = structural_valid && rows == 250;

  bool source_built = true;
  bool attached = true;
  bool wake = true;
  bool released = true;
  double worst_mirror = 0.0;
  double worst_release_energy_drift = 0.0;
  double minimum_radial_alignment =
      std::numeric_limits<double>::infinity();
  double minimum_source_activity =
      std::numeric_limits<double>::infinity();
  double minimum_release_radius_growth =
      std::numeric_limits<double>::infinity();
  double minimum_release_near_drop =
      std::numeric_limits<double>::infinity();
  const bool neutral = observer_neutrality();

  for (const int length : kLengths) {
    const Run* plus = find_run(runs, length, "stationary_plus");
    const Run* minus = find_run(runs, length, "stationary_minus");
    const Run* empty = find_run(runs, length, "empty_control");
    const Run* moving = find_run(runs, length, "production_moving");
    const Run* release = find_run(runs, length, "source_off");
    if (!plus || !minus || !empty || !moving || !release) {
      structural_valid = source_built = attached = wake = released = false;
      continue;
    }

    // Rebuild the two mirror bridges at the classification time so the metric
    // is evaluated on primitive fields rather than reconstructed CSV values.
    ftd::RenderBridge mirror_plus(length), mirror_minus(length);
    bool mirrors_valid = initialize_scenario(mirror_plus, +1)
        && initialize_scenario(mirror_minus, -1);
    for (int tick = 0; tick < kBuildTick; ++tick) {
      mirror_plus.tick();
      mirror_minus.tick();
    }
    const double mirror = ftd::eft::flux_odd_mirror_residual(
        mirror_plus, mirror_minus);
    worst_mirror = std::max(worst_mirror, mirror);

    const auto& p12 = plus->samples[static_cast<std::size_t>(kBuildTick)]
                          .observation;
    const auto& e12 = empty->samples[static_cast<std::size_t>(kBuildTick)]
                          .observation;
    minimum_radial_alignment = std::min(
        minimum_radial_alignment, p12.radial_alignment);
    minimum_source_activity = std::min(
        minimum_source_activity, p12.activity);
    source_built = source_built && mirrors_valid && e12.activity <= 1e-15
        && p12.activity > 1e-8 && mirror <= 1e-12
        && p12.radial_alignment >= 0.75
        && p12.signed_source_divergence > 0.0
        && p12.manifested_count == 1;

    const auto& moved_final = moving->samples.back().observation;
    attached = attached && moving->movement_events >= 4
        && moving->reaction_events == 0
        && moved_final.near_activity >= 0.5 * p12.near_activity
        && moved_final.radial_alignment >= 0.50 && neutral;
    wake = wake && moved_final.trailing_fraction >= 0.15
        && moved_final.trailing_activity
            >= 2.0 * moved_final.leading_activity;

    const auto& release_start =
        release->samples[static_cast<std::size_t>(kBuildTick)].observation;
    const auto& release_final = release->samples.back().observation;
    const double radius_growth = release_final.mean_radius
        - release_start.mean_radius;
    const double near_drop = release_start.near_fraction
        - release_final.near_fraction;
    minimum_release_radius_growth = std::min(
        minimum_release_radius_growth, radius_growth);
    minimum_release_near_drop = std::min(
        minimum_release_near_drop, near_drop);
    double run_energy_drift = 0.0;
    for (std::size_t i = kBuildTick + 1; i < release->samples.size(); ++i)
      run_energy_drift = std::max(
          run_energy_drift,
          release->samples[i].relative_release_energy_drift);
    worst_release_energy_drift = std::max(
        worst_release_energy_drift, run_energy_drift);
    released = released && radius_growth >= 2.0 && near_drop >= 0.20
        && run_energy_drift <= 1e-10
        && release_final.manifested_count == 0;
  }

  const std::string source_verdict = source_built
      ? "SOURCE_BUILT_RADIAL_DRESSING"
      : "NO_QUALIFIED_SOURCE_BUILT_DRESSING";
  const std::string movement_verdict = attached && wake
      ? "ATTACHED_COMPONENT_WITH_TRAILING_WAKE"
      : (attached ? "ATTACHED_COMPONENT_NO_QUALIFIED_WAKE"
         : (wake ? "TRAILING_WAKE_WITHOUT_QUALIFIED_ATTACHMENT"
            : "NO_QUALIFIED_ATTACHMENT_OR_WAKE"));
  const std::string release_verdict = released
      ? "RELEASED_OUTGOING_FIELD"
      : "NO_QUALIFIED_RELEASED_OUTGOING_FIELD";

  std::ofstream verdict(output_dir / "verdict_v2.txt");
  verdict << std::setprecision(17)
      << "source_verdict=" << source_verdict << '\n'
      << "movement_verdict=" << movement_verdict << '\n'
      << "release_verdict=" << release_verdict << '\n'
      << "structural_valid=" << structural_valid << '\n'
      << "observer_neutral=" << neutral << '\n'
      << "rows=" << rows << '\n'
      << "worst_mirror_residual=" << worst_mirror << '\n'
      << "minimum_tick12_activity=" << minimum_source_activity << '\n'
      << "minimum_tick12_radial_alignment=" << minimum_radial_alignment << '\n'
      << "minimum_release_radius_growth=" << minimum_release_radius_growth << '\n'
      << "minimum_release_near_fraction_drop=" << minimum_release_near_drop << '\n'
      << "worst_release_energy_drift=" << worst_release_energy_drift << '\n';

  std::cout << "FTD-0476 source=" << source_verdict
            << " movement=" << movement_verdict
            << " release=" << release_verdict << '\n'
            << "  rows=" << rows << " mirror=" << worst_mirror
            << " activity=" << minimum_source_activity
            << " radial=" << minimum_radial_alignment << '\n'
            << "  release dr=" << minimum_release_radius_growth
            << " df_near=" << minimum_release_near_drop
            << " energy_drift=" << worst_release_energy_drift << '\n'
            << "  structural_valid=" << structural_valid
            << " observer_neutral=" << neutral << std::endl;
  return structural_valid ? 0 : 1;
}
