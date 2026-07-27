/**
 * @file campaign_reciprocal_moving_source.cpp
 * @brief FTD-0477 selected-force moving-source reciprocity discriminator.
 *
 * Observation only.  Every arm runs the frozen production tick.  The source
 * begins at rest; no trajectory, velocity, force, recoil, or field update is
 * injected after initialization.
 */

#include "ftd/eft/fixed_j_recoil_capacity.h"
#include "ftd/eft/reciprocal_moving_source_observer.h"
#include "ftd/scenarios.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace {

constexpr int kL = 65;
constexpr int kFinalTick = 72;
constexpr int kCentre = kL / 2;
constexpr int kSourceY = kCentre + 3;
constexpr double kRepeatGate = 1e-12;
constexpr double kDriverGate = 1e-4;
constexpr double kCauseDisplacementGate = 0.5;
constexpr double kRestGate = 1e-9;
constexpr double kCoastSpeedGate = 1e-3;
constexpr double kCoastForceRatioGate = 0.25;
constexpr double kNearFractionGate = 0.75;
constexpr double kCorrelationGate = 0.80;
constexpr double kTrailingFractionGate = 0.15;
constexpr double kTrailingRatioGate = 2.0;
constexpr double kRadiusGrowthGate = 2.0;
constexpr double kNearDropGate = 0.20;
constexpr double kReciprocityGate = 1e-6;
constexpr std::array<int, 6> kSampleTicks{{0, 16, 32, 48, 56, 72}};

enum class ArmKind {
  DriverOnly,
  PositiveSourceOnly,
  NegativeSourceOnly,
  LockedPositiveCombined,
  MobilePositiveCombined,
  MobileNegativeCombined,
  PositiveRepeat,
};

const char* arm_name(ArmKind kind) {
  switch (kind) {
    case ArmKind::DriverOnly: return "driver_only";
    case ArmKind::PositiveSourceOnly: return "positive_source_only";
    case ArmKind::NegativeSourceOnly: return "negative_source_only";
    case ArmKind::LockedPositiveCombined: return "locked_positive_plus_driver";
    case ArmKind::MobilePositiveCombined: return "mobile_positive_plus_driver";
    case ArmKind::MobileNegativeCombined: return "mobile_negative_plus_driver";
    case ArmKind::PositiveRepeat: return "mobile_positive_plus_driver_repeat";
  }
  return "unknown";
}

bool finite_vec(const ftd::Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

int wrapped_step(int current, int previous) {
  int delta = current - previous;
  if (delta > kL / 2) delta -= kL;
  if (delta < -kL / 2) delta += kL;
  return delta;
}

int find_particle(const ftd::RenderBridge& bridge, int particle_id) {
  if (particle_id < 0) return -1;
  for (int index = 0;
       index < static_cast<int>(bridge.voxels().size()); ++index) {
    const auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
    if (voxel.state != 0 && voxel.particle_id == particle_id) return index;
  }
  return -1;
}

bool profile_is_exact(const ftd::RenderBridge& bridge) {
  for (const auto& spec : ftd::TOGGLE_SPECS) {
    const std::string name(spec.name);
    const bool expected = name == "wave_propagation" || name == "coupling"
        || name == "forces" || name == "movement"
        || name == "emergent_forces" || name == "strict_validation";
    if ((bridge.toggles.*(spec.field)) != expected) return false;
  }
  return bridge.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic
      && bridge.toggles.bcc_stencil == ftd::BccStencilMode::FULL
      && bridge.dt() == 1.0;
}

void clear_field(ftd::RenderBridge& bridge) {
  for (auto& voxel : bridge.voxels()) {
    voxel.flux = {};
    voxel.wave_vel = {};
  }
}

void remove_source(ftd::RenderBridge& bridge, int source_index) {
  bridge.set_state(source_index, 0);
  auto& source = bridge.voxels()[static_cast<std::size_t>(source_index)];
  source.locked = false;
  source.velocity = {};
  source.remainder = {};
  source.particle_id = -1;
  source.pair_id = -1;
  source.spin = 0;
  source.color = 0;
}

struct Arm {
  ArmKind kind;
  std::unique_ptr<ftd::RenderBridge> bridge;
  int particle_id = -1;
  int initial_index = -1;
  int previous_x = 0;
  int previous_y = 0;
  int previous_z = 0;
  ftd::Vec3 integer_displacement{};
  ftd::Vec3 displacement{};
  ftd::Vec3 velocity{};
  std::vector<ftd::Vec3> force_history;
  int movement_events = 0;
  int reaction_events = 0;
  bool initialized = false;
  bool source_survives = true;
  bool finite = true;
  double initial_energy = 0.0;
  ftd::Vec3 initial_particle_momentum{};
  ftd::Vec3 initial_total_momentum{};
};

Arm make_arm(ArmKind kind) {
  Arm arm;
  arm.kind = kind;
  arm.bridge = std::make_unique<ftd::RenderBridge>(kL);
  arm.bridge->force_cpu();
  arm.initialized = ftd::dispatch_scenario(
      *arm.bridge, "s0-seed-moving-source-reciprocity");
  const int source_index = arm.bridge->lattice().index(
      kCentre, kSourceY, kCentre);
  auto& source = arm.bridge->voxels()[static_cast<std::size_t>(source_index)];

  if (kind == ArmKind::DriverOnly) {
    remove_source(*arm.bridge, source_index);
  } else {
    if (kind == ArmKind::NegativeSourceOnly
        || kind == ArmKind::MobileNegativeCombined)
      arm.bridge->set_state(source_index, -1);
    if (kind == ArmKind::PositiveSourceOnly
        || kind == ArmKind::NegativeSourceOnly)
      clear_field(*arm.bridge);
    if (kind == ArmKind::LockedPositiveCombined)
      source.locked = true;
    arm.particle_id = source.particle_id;
    arm.initial_index = source_index;
    const auto coordinate = arm.bridge->lattice().coord(source_index);
    arm.previous_x = coordinate.x;
    arm.previous_y = coordinate.y;
    arm.previous_z = coordinate.z;
    arm.initialized = arm.initialized
        && source.velocity.mag2() == 0.0 && source.remainder.mag2() == 0.0
        && arm.bridge->enable_history_journal(true);
  }

  arm.initialized = arm.initialized && profile_is_exact(*arm.bridge)
      && arm.bridge->backend_kind() == ftd::Backend::Kind::Cpu;
  const auto energy = arm.bridge->energy_audit();
  arm.initial_energy = energy.dynamic_energy;
  arm.initial_particle_momentum = energy.particle_momentum;
  arm.initial_total_momentum = energy.particle_momentum
      + ftd::eft::central_field_momentum(*arm.bridge);
  return arm;
}

void advance(Arm& arm) {
  const int before_index = find_particle(*arm.bridge, arm.particle_id);
  arm.bridge->tick();
  if (before_index >= 0) {
    arm.force_history.push_back(
        arm.bridge->force_diag_at(before_index).f_coulomb);
  }
  for (const auto& event : arm.bridge->history_events()) {
    if (event.kind == ftd::eft::HistoryEventKind::Movement)
      ++arm.movement_events;
    else
      ++arm.reaction_events;
  }

  if (arm.particle_id >= 0) {
    const int after_index = find_particle(*arm.bridge, arm.particle_id);
    if (after_index < 0) {
      arm.source_survives = false;
      arm.finite = false;
      return;
    }
    const auto coordinate = arm.bridge->lattice().coord(after_index);
    arm.integer_displacement.x += wrapped_step(
        coordinate.x, arm.previous_x);
    arm.integer_displacement.y += wrapped_step(
        coordinate.y, arm.previous_y);
    arm.integer_displacement.z += wrapped_step(
        coordinate.z, arm.previous_z);
    arm.previous_x = coordinate.x;
    arm.previous_y = coordinate.y;
    arm.previous_z = coordinate.z;
    const auto& source = arm.bridge->voxels()[
        static_cast<std::size_t>(after_index)];
    arm.displacement = arm.integer_displacement + source.remainder;
    arm.velocity = source.velocity;
    arm.finite = arm.finite && finite_vec(arm.displacement)
        && finite_vec(arm.velocity);
  }
}

double driver_activity_near_source(const ftd::RenderBridge& bridge) {
  const auto& voxels = bridge.voxels();
  const auto centre = bridge.lattice().coord(
      bridge.lattice().index(kCentre, kSourceY, kCentre));
  double result = 0.0;
  for (int index = 0; index < static_cast<int>(voxels.size()); ++index) {
    const auto coordinate = bridge.lattice().coord(index);
    const double dx = wrapped_step(coordinate.x, centre.x);
    const double dy = wrapped_step(coordinate.y, centre.y);
    const double dz = wrapped_step(coordinate.z, centre.z);
    if (dx * dx + dy * dy + dz * dz > 16.0) continue;
    const auto& voxel = voxels[static_cast<std::size_t>(index)];
    result += ftd::field_kinetic_term(voxel.wave_vel)
        - ftd::field_gradient_term(
            voxel.flux, bridge.lattice().neighbors_6(index),
            bridge.lattice().neighbors_12(index), voxels);
  }
  return result;
}

double bridge_repeat_residual(const ftd::RenderBridge& left,
                              const ftd::RenderBridge& right) {
  if (left.voxels().size() != right.voxels().size())
    return std::numeric_limits<double>::infinity();
  double result = 0.0;
  for (std::size_t index = 0; index < left.voxels().size(); ++index) {
    const auto& a = left.voxels()[index];
    const auto& b = right.voxels()[index];
    if (a.state != b.state || a.locked != b.locked
        || a.particle_id != b.particle_id)
      return std::numeric_limits<double>::infinity();
    result = std::max({result, (a.flux - b.flux).mag(),
                       (a.wave_vel - b.wave_vel).mag(),
                       (a.velocity - b.velocity).mag(),
                       (a.remainder - b.remainder).mag()});
  }
  return result;
}

ftd::Vec3 selected_total_momentum(const Arm& arm) {
  const auto audit = arm.bridge->energy_audit();
  return audit.particle_momentum
      + ftd::eft::central_field_momentum(*arm.bridge);
}

double dynamic_energy(const Arm& arm) {
  return arm.bridge->energy_audit().dynamic_energy;
}

struct ReciprocityTrace {
  double max_energy_residual = 0.0;
  double energy_scale = 0.0;
  double max_momentum_residual = 0.0;
  double momentum_scale = 0.0;
  double normalized_energy = 0.0;
  double normalized_momentum = 0.0;
  bool finite = true;
};

void update_reciprocity(ReciprocityTrace& trace, const Arm& combined,
                        const Arm& source, const Arm& driver) {
  const double energy_residual =
      (dynamic_energy(combined) - combined.initial_energy)
      - (dynamic_energy(source) - source.initial_energy)
      - (dynamic_energy(driver) - driver.initial_energy);
  const ftd::Vec3 momentum_residual =
      (selected_total_momentum(combined) - combined.initial_total_momentum)
      - (selected_total_momentum(source) - source.initial_total_momentum)
      - (selected_total_momentum(driver) - driver.initial_total_momentum);
  const auto combined_audit = combined.bridge->energy_audit();
  const double particle_impulse = (
      combined_audit.particle_momentum
      - combined.initial_particle_momentum).mag();
  trace.max_energy_residual = std::max(
      trace.max_energy_residual, std::abs(energy_residual));
  trace.energy_scale = std::max(
      trace.energy_scale,
      std::abs(source.initial_energy) + std::abs(driver.initial_energy));
  trace.max_momentum_residual = std::max(
      trace.max_momentum_residual, momentum_residual.mag());
  trace.momentum_scale = std::max({
      trace.momentum_scale,
      driver.initial_total_momentum.mag(), particle_impulse, 1e-30});
  trace.finite = trace.finite && std::isfinite(energy_residual)
      && finite_vec(momentum_residual) && std::isfinite(particle_impulse);
}

double force_peak(const Arm& combined, const Arm& source,
                  int first_tick, int last_tick) {
  double result = 0.0;
  for (int tick = first_tick; tick <= last_tick; ++tick) {
    const auto extra = combined.force_history[static_cast<std::size_t>(tick - 1)]
        - source.force_history[static_cast<std::size_t>(tick - 1)];
    result = std::max(result, extra.mag());
  }
  return result;
}

double force_rms(const Arm& combined, const Arm& source,
                 int first_tick, int last_tick) {
  double sum = 0.0;
  int count = 0;
  for (int tick = first_tick; tick <= last_tick; ++tick) {
    const auto extra = combined.force_history[static_cast<std::size_t>(tick - 1)]
        - source.force_history[static_cast<std::size_t>(tick - 1)];
    sum += extra.mag2();
    ++count;
  }
  return std::sqrt(sum / std::max(1, count));
}

struct Sample {
  int tick = 0;
  ftd::eft::ReciprocalMovingSourceObservation positive{};
  ftd::eft::ReciprocalMovingSourceObservation negative{};
};

void print_arm(const Arm& arm) {
  std::cout << "arm,name," << arm_name(arm.kind)
            << ",disp_x," << arm.displacement.x
            << ",disp_y," << arm.displacement.y
            << ",disp_z," << arm.displacement.z
            << ",vel_x," << arm.velocity.x
            << ",vel_y," << arm.velocity.y
            << ",vel_z," << arm.velocity.z
            << ",movement_events," << arm.movement_events
            << ",reaction_events," << arm.reaction_events
            << ",survives," << (arm.source_survives ? "true" : "false")
            << ",finite," << (arm.finite ? "true" : "false") << '\n';
}

void print_observation(
    int tick, const char* polarity,
    const ftd::eft::ReciprocalMovingSourceObservation& observation) {
  std::cout << "field,tick," << tick << ",polarity," << polarity
            << ",activity," << observation.activity
            << ",near_fraction," << observation.near_fraction
            << ",trailing_fraction," << observation.trailing_fraction
            << ",leading_fraction," << observation.leading_fraction
            << ",transverse_fraction," << observation.transverse_fraction
            << ",trailing_to_leading," << observation.trailing_to_leading
            << ",mean_radius," << observation.mean_radius
            << ",shifted_correlation,"
            << observation.shifted_source_correlation
            << ",detached_activity," << observation.detached_activity
            << ",valid," << (observation.valid ? "true" : "false")
            << '\n';
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0477 selected-force moving-source reciprocity v1\n";
  std::cout << "protocol,L," << kL << ",ticks," << kFinalTick
            << ",driver_amp,0.5,driver_x,12,source_x,32,source_y,35"
            << ",near_radius,4\n";
  std::cout << "scope,selected_G_C_s_grad_absJ_extension_not_qE_not_five_postulate_native\n";

  Arm driver = make_arm(ArmKind::DriverOnly);
  Arm source_positive = make_arm(ArmKind::PositiveSourceOnly);
  Arm source_negative = make_arm(ArmKind::NegativeSourceOnly);
  Arm locked_positive = make_arm(ArmKind::LockedPositiveCombined);
  Arm combined_positive = make_arm(ArmKind::MobilePositiveCombined);
  Arm combined_negative = make_arm(ArmKind::MobileNegativeCombined);
  Arm positive_repeat = make_arm(ArmKind::PositiveRepeat);
  std::array<Arm*, 7> arms{{
      &driver, &source_positive, &source_negative, &locked_positive,
      &combined_positive, &combined_negative, &positive_repeat}};

  const int source_index = combined_positive.initial_index;
  const auto& initial_source = combined_positive.bridge->voxels()[
      static_cast<std::size_t>(source_index)];
  const bool source_site_zero = initial_source.flux.mag2() == 0.0
      && initial_source.wave_vel.mag2() == 0.0;
  double initial_driver_norm2 = 0.0;
  for (const auto& voxel : driver.bridge->voxels())
    initial_driver_norm2 += voxel.flux.mag2() + voxel.wave_vel.mag2();

  std::vector<Sample> samples;
  double driver_near_peak = driver_activity_near_source(*driver.bridge);
  double repeat_residual = bridge_repeat_residual(
      *combined_positive.bridge, *positive_repeat.bridge);
  ReciprocityTrace positive_reciprocity;
  ReciprocityTrace negative_reciprocity;
  positive_reciprocity.energy_scale = std::abs(source_positive.initial_energy)
      + std::abs(driver.initial_energy);
  negative_reciprocity.energy_scale = std::abs(source_negative.initial_energy)
      + std::abs(driver.initial_energy);
  positive_reciprocity.momentum_scale = std::max(
      driver.initial_total_momentum.mag(), 1e-30);
  negative_reciprocity.momentum_scale = positive_reciprocity.momentum_scale;

  for (int tick = 0; tick <= kFinalTick; ++tick) {
    if (std::find(kSampleTicks.begin(), kSampleTicks.end(), tick)
        != kSampleTicks.end()) {
      std::cout << "trajectory,tick," << tick
                << ",positive_x," << combined_positive.displacement.x
                << ",positive_y," << combined_positive.displacement.y
                << ",positive_z," << combined_positive.displacement.z
                << ",negative_x," << combined_negative.displacement.x
                << ",negative_y," << combined_negative.displacement.y
                << ",negative_z," << combined_negative.displacement.z
                << ",driver_near_activity,"
                << driver_activity_near_source(*driver.bridge) << '\n';
    }

    if (tick == 56 || tick == 72) {
      const int positive_index = find_particle(
          *combined_positive.bridge, combined_positive.particle_id);
      const int negative_index = find_particle(
          *combined_negative.bridge, combined_negative.particle_id);
      Sample sample;
      sample.tick = tick;
      if (positive_index >= 0) {
        sample.positive = ftd::eft::observe_reciprocal_moving_source(
            *combined_positive.bridge, *driver.bridge,
            *source_positive.bridge, positive_index,
            combined_positive.integer_displacement,
            combined_positive.displacement, 4.0);
      }
      if (negative_index >= 0) {
        sample.negative = ftd::eft::observe_reciprocal_moving_source(
            *combined_negative.bridge, *driver.bridge,
            *source_negative.bridge, negative_index,
            combined_negative.integer_displacement,
            combined_negative.displacement, 4.0);
      }
      print_observation(tick, "positive", sample.positive);
      print_observation(tick, "negative", sample.negative);
      samples.push_back(sample);
    }

    if (tick == kFinalTick) break;
    for (Arm* arm : arms) advance(*arm);
    driver_near_peak = std::max(
        driver_near_peak, driver_activity_near_source(*driver.bridge));
    repeat_residual = std::max(
        repeat_residual, bridge_repeat_residual(
            *combined_positive.bridge, *positive_repeat.bridge));
    update_reciprocity(
        positive_reciprocity, combined_positive, source_positive, driver);
    update_reciprocity(
        negative_reciprocity, combined_negative, source_negative, driver);
  }

  for (Arm* arm : arms) print_arm(*arm);

  positive_reciprocity.normalized_energy =
      positive_reciprocity.max_energy_residual
      / std::max(1e-30, positive_reciprocity.energy_scale);
  positive_reciprocity.normalized_momentum =
      positive_reciprocity.max_momentum_residual
      / std::max(1e-30, positive_reciprocity.momentum_scale);
  negative_reciprocity.normalized_energy =
      negative_reciprocity.max_energy_residual
      / std::max(1e-30, negative_reciprocity.energy_scale);
  negative_reciprocity.normalized_momentum =
      negative_reciprocity.max_momentum_residual
      / std::max(1e-30, negative_reciprocity.momentum_scale);

  const ftd::Vec3 positive_caused = combined_positive.displacement
      - source_positive.displacement;
  const ftd::Vec3 negative_caused = combined_negative.displacement
      - source_negative.displacement;
  const ftd::Vec3 locked_displacement = locked_positive.displacement;
  const double positive_peak = force_peak(
      combined_positive, source_positive, 16, 48);
  const double negative_peak = force_peak(
      combined_negative, source_negative, 16, 48);
  const double positive_late_rms = force_rms(
      combined_positive, source_positive, 57, 72);
  const double negative_late_rms = force_rms(
      combined_negative, source_negative, 57, 72);

  bool all_initialized = true;
  bool all_finite = true;
  bool all_survive = true;
  bool profiles_exact = true;
  for (const Arm* arm : arms) {
    all_initialized = all_initialized && arm->initialized;
    all_finite = all_finite && arm->finite;
    profiles_exact = profiles_exact && profile_is_exact(*arm->bridge);
    if (arm->particle_id >= 0)
      all_survive = all_survive && arm->source_survives;
  }
  const bool source_only_rest = source_positive.displacement.mag() <= kRestGate
      && source_negative.displacement.mag() <= kRestGate;
  const bool caused_integer_motion = all_survive && source_only_rest
      && locked_displacement.mag() <= kRepeatGate
      && driver_near_peak > kDriverGate
      && positive_caused.mag() >= kCauseDisplacementGate
      && negative_caused.mag() >= kCauseDisplacementGate
      && combined_positive.movement_events >= 1
      && combined_negative.movement_events >= 1
      && repeat_residual <= kRepeatGate;
  const bool coast_interval = caused_integer_motion
      && combined_positive.velocity.mag() >= kCoastSpeedGate
      && combined_negative.velocity.mag() >= kCoastSpeedGate
      && positive_late_rms <= kCoastForceRatioGate
          * std::max(1e-30, positive_peak)
      && negative_late_rms <= kCoastForceRatioGate
          * std::max(1e-30, negative_peak);

  const bool observations_valid = samples.size() == 2
      && samples[0].positive.valid && samples[0].negative.valid
      && samples[1].positive.valid && samples[1].negative.valid;
  const bool dressing_candidate = observations_valid
      && samples[0].positive.near_fraction >= kNearFractionGate
      && samples[1].positive.near_fraction >= kNearFractionGate
      && samples[0].negative.near_fraction >= kNearFractionGate
      && samples[1].negative.near_fraction >= kNearFractionGate
      && samples[0].positive.shifted_source_correlation >= kCorrelationGate
      && samples[1].positive.shifted_source_correlation >= kCorrelationGate
      && samples[0].negative.shifted_source_correlation >= kCorrelationGate
      && samples[1].negative.shifted_source_correlation >= kCorrelationGate;
  const bool wake_candidate = observations_valid
      && samples[1].positive.trailing_fraction >= kTrailingFractionGate
      && samples[1].negative.trailing_fraction >= kTrailingFractionGate
      && samples[1].positive.trailing_to_leading >= kTrailingRatioGate
      && samples[1].negative.trailing_to_leading >= kTrailingRatioGate;
  const bool detached_candidate = observations_valid && coast_interval
      && samples[1].positive.mean_radius - samples[0].positive.mean_radius
          >= kRadiusGrowthGate
      && samples[1].negative.mean_radius - samples[0].negative.mean_radius
          >= kRadiusGrowthGate
      && samples[0].positive.near_fraction
          - samples[1].positive.near_fraction >= kNearDropGate
      && samples[0].negative.near_fraction
          - samples[1].negative.near_fraction >= kNearDropGate
      && samples[0].positive.detached_activity > 0.0
      && samples[1].positive.detached_activity > 0.0
      && samples[0].negative.detached_activity > 0.0
      && samples[1].negative.detached_activity > 0.0;
  const bool reciprocal = caused_integer_motion
      && positive_reciprocity.normalized_energy <= kReciprocityGate
      && negative_reciprocity.normalized_energy <= kReciprocityGate
      && positive_reciprocity.normalized_momentum <= kReciprocityGate
      && negative_reciprocity.normalized_momentum <= kReciprocityGate;

  const bool structural_valid = all_initialized && all_finite
      && all_survive && profiles_exact && source_site_zero
      && initial_driver_norm2 > 0.0 && repeat_residual <= kRepeatGate
      && positive_reciprocity.finite && negative_reciprocity.finite
      && observations_valid;

  const char* verdict = "UNCLASSIFIED";
  if (!structural_valid) {
    verdict = "INVALID_PROTOCOL";
  } else if (!caused_integer_motion) {
    verdict = "NO_DYNAMICAL_MOVING_SOURCE_IN_REGISTERED_PROTOCOL";
  } else if (!reciprocal) {
    verdict = "DYNAMICAL_MOTION_WITHOUT_CLOSED_RECIPROCITY";
  } else if (!dressing_candidate && !wake_candidate && !detached_candidate) {
    verdict = "RECIPROCAL_MOTION_WITHOUT_QUALIFIED_DRESSING_WAKE_OR_DETACHED_FIELD";
  } else {
    verdict = "SELECTED_EXTENSION_ONE_OR_MORE_MORPHOLOGY_LABELS_PASS";
  }

  std::cout << "causation,driver_near_peak," << driver_near_peak
            << ",positive_caused_displacement," << positive_caused.mag()
            << ",negative_caused_displacement," << negative_caused.mag()
            << ",locked_displacement," << locked_displacement.mag()
            << ",source_positive_displacement,"
            << source_positive.displacement.mag()
            << ",source_negative_displacement,"
            << source_negative.displacement.mag()
            << ",repeat_residual," << repeat_residual << '\n';
  std::cout << "coast,positive_peak_force," << positive_peak
            << ",positive_late_rms," << positive_late_rms
            << ",positive_ratio,"
            << positive_late_rms / std::max(1e-30, positive_peak)
            << ",negative_peak_force," << negative_peak
            << ",negative_late_rms," << negative_late_rms
            << ",negative_ratio,"
            << negative_late_rms / std::max(1e-30, negative_peak) << '\n';
  std::cout << "reciprocity,positive_energy,"
            << positive_reciprocity.normalized_energy
            << ",negative_energy," << negative_reciprocity.normalized_energy
            << ",positive_momentum,"
            << positive_reciprocity.normalized_momentum
            << ",negative_momentum,"
            << negative_reciprocity.normalized_momentum << '\n';
  std::cout << "gates,structural_valid,"
            << (structural_valid ? "true" : "false")
            << ",caused_integer_motion,"
            << (caused_integer_motion ? "true" : "false")
            << ",coast_interval," << (coast_interval ? "true" : "false")
            << ",dressing_candidate,"
            << (dressing_candidate ? "true" : "false")
            << ",wake_candidate," << (wake_candidate ? "true" : "false")
            << ",detached_candidate,"
            << (detached_candidate ? "true" : "false")
            << ",reciprocal_selected_extension,"
            << (reciprocal ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  std::cout << "interpretation,selected_extension_only_no_qE_no_photon_no_radiation_promotion\n";
  return structural_valid ? 0 : 1;
}

