/**
 * @file campaign_production_orientation_memory_census.cpp
 * @brief FTD-0908 locked observation-only production formation census.
 *
 * This runner does not alter RenderBridge physics. It observes Moore-adjacent
 * +/- production particles after each complete tick, evaluates the frozen
 * FTD-0907 dipole/phase-wedge analyzer, and tracks sign-stable runs by the
 * production particle IDs. The protocol was locked before this file existed.
 */

#include "ftd/constants.h"
#include "ftd/eft/native_ternary_dipole_phase_wedge_memory.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace {

using ftd::eft::NativeOrientationMemoryResult;
using ftd::eft::NativeOrientationMemorySite;
using ftd::eft::NativeOrientationVector;

constexpr char kProtocolSha256[] =
    "53348A90021C609E3EBA5DC7D565F6EA78832498C206D0D4B3F1964CCC7C4993";
constexpr int kTicks = 96;
constexpr int kPersistenceTicks = 8;
constexpr double kTolerance = 1e-11;
constexpr double kControlFactor = 256.0;
constexpr double kInjectionAmplitude = 10.0;
constexpr double kLangevinTemperature = 0.005;
constexpr double kLangevinGamma = 0.02;
constexpr std::array<int, 2> kVolumes{{17, 25}};
constexpr std::array<std::uint32_t, 4> kSeeds{{
    0x09080001u, 0x09080002u, 0x09080003u, 0x09080004u}};

enum class Family {
  AxialLive,
  DiagonalLive,
  AxialNoBath,
  EmptyControl,
};

constexpr std::array<Family, 4> kFamilies{{
    Family::AxialLive, Family::DiagonalLive,
    Family::AxialNoBath, Family::EmptyControl}};

const char* family_name(Family family) {
  switch (family) {
    case Family::AxialLive: return "axial_live";
    case Family::DiagonalLive: return "diagonal_live";
    case Family::AxialNoBath: return "axial_no_bath";
    case Family::EmptyControl: return "empty_control";
  }
  return "unknown";
}

bool is_live_injected(Family family) {
  return family != Family::EmptyControl;
}

struct PairKey {
  int positive_id = -1;
  int negative_id = -1;

  bool operator<(const PairKey& other) const {
    return std::tie(positive_id, negative_id)
        < std::tie(other.positive_id, other.negative_id);
  }
};

struct PairTrack {
  int last_tick = -2;
  int chirality = 0;
  int current_run = 0;
  int maximum_run = 0;
  int observations = 0;
};

struct PairSample {
  PairKey key{};
  int tick = -1;
  int positive_site = -1;
  int negative_site = -1;
  NativeOrientationVector separation{};
  NativeOrientationVector positive_flux{};
  NativeOrientationVector negative_flux{};
  NativeOrientationVector positive_wave_velocity{};
  NativeOrientationVector negative_wave_velocity{};
  double q_positive = 0.0;
  double q_negative = 0.0;
  double p_positive = 0.0;
  double p_negative = 0.0;
  double phase_wedge = 0.0;
  int chirality = 0;
};

struct TickObservation {
  bool finite = true;
  bool nonmutating = true;
  bool controls_pass = true;
  bool reconstructible = true;
  int positive_count = 0;
  int negative_count = 0;
  int valid_pairs = 0;
  int positive_chirality = 0;
  int negative_chirality = 0;
  int randomized_valid_pairs = 0;
  int longest_current_run = 0;
  long long genesis_events = 0;
  long long evaporation_events = 0;
  double maximum_abs_wedge = 0.0;
  double rms_wedge = 0.0;
  double randomized_maximum_abs_wedge = 0.0;
  double native_wave_energy = 0.0;
  double worst_control_residual = 0.0;
  std::uint64_t voxel_hash_before = 0;
  std::uint64_t voxel_hash_after = 0;
  std::uint64_t rng_hash_before = 0;
  std::uint64_t rng_hash_after = 0;
  std::vector<PairSample> samples;
};

struct ArmResult {
  int volume = 0;
  Family family = Family::AxialLive;
  std::uint32_t seed = 0;
  bool executed = false;
  bool finite = true;
  bool nonmutating = true;
  bool controls_pass = true;
  bool reconstructible = true;
  int ticks = 0;
  int ticks_with_pairs = 0;
  int unique_pairs = 0;
  int maximum_sign_stable_run = 0;
  int persistent_pair_count = 0;
  long long total_valid_pair_observations = 0;
  long long total_randomized_valid_pairs = 0;
  long long total_genesis_events = 0;
  long long total_evaporation_events = 0;
  double maximum_abs_wedge = 0.0;
  double randomized_maximum_abs_wedge = 0.0;
  double maximum_native_wave_energy = 0.0;
  double worst_control_residual = 0.0;
};

std::uint64_t hash_voxels(const std::vector<ftd::Voxel>& voxels) {
  constexpr std::uint64_t offset = 1469598103934665603ull;
  constexpr std::uint64_t prime = 1099511628211ull;
  std::uint64_t hash = offset;
  const auto* bytes = reinterpret_cast<const unsigned char*>(voxels.data());
  const std::size_t count = voxels.size() * sizeof(ftd::Voxel);
  for (std::size_t i = 0; i < count; ++i) {
    hash ^= static_cast<std::uint64_t>(bytes[i]);
    hash *= prime;
  }
  return hash;
}

NativeOrientationVector to_array(const ftd::Vec3& value) {
  return {value.x, value.y, value.z};
}

NativeOrientationVector signed_cubic(const NativeOrientationVector& value) {
  return {-value[1], value[2], -value[0]};
}

NativeOrientationVector negate(const NativeOrientationVector& value) {
  return {-value[0], -value[1], -value[2]};
}

double max_abs_difference(
    const NativeOrientationVector& left,
    const NativeOrientationVector& right) {
  return std::max({std::abs(left[0] - right[0]),
                   std::abs(left[1] - right[1]),
                   std::abs(left[2] - right[2])});
}

double max_abs(const NativeOrientationVector& value) {
  return std::max({std::abs(value[0]), std::abs(value[1]),
                   std::abs(value[2])});
}

int minimum_image_delta(int positive, int negative, int volume) {
  int delta = positive - negative;
  if (delta > volume / 2) delta -= volume;
  if (delta < -volume / 2) delta += volume;
  return delta;
}

std::vector<NativeOrientationMemorySite> pair_sites(
    const PairSample& sample) {
  return {
      NativeOrientationMemorySite{
          sample.separation, +1,
          sample.positive_flux, sample.positive_wave_velocity},
      NativeOrientationMemorySite{
          {0.0, 0.0, 0.0}, -1,
          sample.negative_flux, sample.negative_wave_velocity},
  };
}

std::vector<NativeOrientationMemorySite> transform_sites(
    const std::vector<NativeOrientationMemorySite>& sites,
    bool apply_signed_cubic, bool apply_inversion, bool reverse_time) {
  auto transformed = sites;
  for (auto& site : transformed) {
    if (apply_signed_cubic) {
      site.position = signed_cubic(site.position);
      site.flux = signed_cubic(site.flux);
      site.wave_velocity = signed_cubic(site.wave_velocity);
    }
    if (apply_inversion) {
      site.position = negate(site.position);
      site.flux = negate(site.flux);
      site.wave_velocity = negate(site.wave_velocity);
    }
    if (reverse_time) site.wave_velocity = negate(site.wave_velocity);
  }
  return transformed;
}

bool audit_controls(
    const PairSample& sample,
    const NativeOrientationMemoryResult& original,
    double& worst_residual) {
  const double scale = std::max({
      1.0, std::abs(original.phase_wedge),
      max_abs(original.polar_axis)});
  const double accepted = kControlFactor * kTolerance * scale;
  bool pass = true;
  auto record = [&](double residual) {
    worst_residual = std::max(worst_residual, std::abs(residual));
    pass = pass && std::isfinite(residual) && std::abs(residual) <= accepted;
  };

  const auto sites = pair_sites(sample);
  const auto cubic = ftd::eft::analyze_native_ternary_dipole_phase_wedge_memory(
      transform_sites(sites, true, false, false));
  pass = pass && cubic.valid();
  if (cubic.valid()) {
    record(cubic.phase_wedge - original.phase_wedge);
    record(max_abs_difference(
        cubic.polar_axis, signed_cubic(original.polar_axis)));
  }

  const auto inversion =
      ftd::eft::analyze_native_ternary_dipole_phase_wedge_memory(
          transform_sites(sites, false, true, false));
  pass = pass && inversion.valid();
  if (inversion.valid()) {
    record(inversion.phase_wedge - original.phase_wedge);
    record(max_abs_difference(
        inversion.polar_axis, negate(original.polar_axis)));
  }

  const auto reversed =
      ftd::eft::analyze_native_ternary_dipole_phase_wedge_memory(
          transform_sites(sites, false, false, true));
  pass = pass && reversed.valid();
  if (reversed.valid()) {
    record(reversed.phase_wedge + original.phase_wedge);
    pass = pass && reversed.chirality == -original.chirality;
  }

  const auto& d = original.ternary_dipole;
  const auto minus_d = negate(d);
  for (int row = 0; row < 3; ++row)
    for (int col = 0; col < 3; ++col)
      record(d[row] * d[col] - minus_d[row] * minus_d[col]);
  record(original.gram_wedge_square_residual);
  record(original.swept_area_full_time_reversal_residual);
  pass = pass && !original.one_step_swept_area_time_odd_memory;
  return pass;
}

PairSample make_sample(
    const ftd::RenderBridge& bridge, int tick,
    int positive_site, int negative_site,
    const NativeOrientationMemoryResult& result) {
  PairSample sample;
  const auto positive_coord = bridge.lattice().coord(positive_site);
  const auto negative_coord = bridge.lattice().coord(negative_site);
  const int volume = bridge.lattice().size();
  const auto& positive = bridge.voxels()[
      static_cast<std::size_t>(positive_site)];
  const auto& negative = bridge.voxels()[
      static_cast<std::size_t>(negative_site)];
  sample.key = {positive.particle_id, negative.particle_id};
  sample.tick = tick;
  sample.positive_site = positive_site;
  sample.negative_site = negative_site;
  sample.separation = {
      static_cast<double>(minimum_image_delta(
          positive_coord.x, negative_coord.x, volume)),
      static_cast<double>(minimum_image_delta(
          positive_coord.y, negative_coord.y, volume)),
      static_cast<double>(minimum_image_delta(
          positive_coord.z, negative_coord.z, volume)),
  };
  sample.positive_flux = to_array(positive.flux);
  sample.negative_flux = to_array(negative.flux);
  sample.positive_wave_velocity = to_array(positive.wave_vel);
  sample.negative_wave_velocity = to_array(negative.wave_vel);
  sample.q_positive = result.positive_coordinate;
  sample.q_negative = result.negative_coordinate;
  sample.p_positive = result.positive_momentum;
  sample.p_negative = result.negative_momentum;
  sample.phase_wedge = result.phase_wedge;
  sample.chirality = result.chirality;
  return sample;
}

NativeOrientationMemoryResult analyze_pair(
    const ftd::RenderBridge& bridge,
    int positive_site, int negative_site) {
  const auto positive_coord = bridge.lattice().coord(positive_site);
  const auto negative_coord = bridge.lattice().coord(negative_site);
  const int volume = bridge.lattice().size();
  const auto& positive = bridge.voxels()[
      static_cast<std::size_t>(positive_site)];
  const auto& negative = bridge.voxels()[
      static_cast<std::size_t>(negative_site)];
  std::vector<NativeOrientationMemorySite> sites{
      NativeOrientationMemorySite{
          {static_cast<double>(minimum_image_delta(
               positive_coord.x, negative_coord.x, volume)),
           static_cast<double>(minimum_image_delta(
               positive_coord.y, negative_coord.y, volume)),
           static_cast<double>(minimum_image_delta(
               positive_coord.z, negative_coord.z, volume))},
          +1, to_array(positive.flux), to_array(positive.wave_vel)},
      NativeOrientationMemorySite{
          {0.0, 0.0, 0.0}, -1,
          to_array(negative.flux), to_array(negative.wave_vel)},
  };
  ftd::eft::NativeOrientationMemoryParameters parameters;
  parameters.tolerance = kTolerance;
  return ftd::eft::analyze_native_ternary_dipole_phase_wedge_memory(
      sites, parameters);
}

TickObservation observe_tick(
    const ftd::RenderBridge& bridge, int tick,
    std::map<PairKey, PairTrack>& tracks) {
  TickObservation observation;
  observation.voxel_hash_before = hash_voxels(bridge.voxels());
  observation.rng_hash_before = bridge.rng_state_hash();
  observation.genesis_events = bridge.genesis_events_this_tick();
  observation.evaporation_events = bridge.evaporation_events_this_tick();

  long double wedge_square_sum = 0.0L;
  const auto& voxels = bridge.voxels();
  std::set<PairKey> seen_this_tick;
  for (std::size_t index = 0; index < voxels.size(); ++index) {
    const auto& positive = voxels[index];
    if (positive.state > 0) ++observation.positive_count;
    if (positive.state < 0) ++observation.negative_count;
    observation.native_wave_energy += 0.5
        * (positive.flux.mag2() + positive.wave_vel.mag2());
    if (positive.state != +1 || positive.particle_id < 0) continue;
    for (int neighbor : bridge.lattice().neighbors_26(
             static_cast<int>(index))) {
      const auto& negative = voxels[static_cast<std::size_t>(neighbor)];
      if (negative.state != -1 || negative.particle_id < 0) continue;
      const auto result = analyze_pair(
          bridge, static_cast<int>(index), neighbor);
      if (!result.valid()) continue;
      PairSample sample = make_sample(
          bridge, tick, static_cast<int>(index), neighbor, result);
      if (!seen_this_tick.insert(sample.key).second) continue;

      const double reconstructed = sample.q_positive * sample.p_negative
          - sample.q_negative * sample.p_positive;
      const double reconstruction_scale = std::max(
          1.0, std::abs(sample.phase_wedge));
      const double reconstruction_residual =
          reconstructed - sample.phase_wedge;
      observation.reconstructible = observation.reconstructible
          && std::isfinite(reconstructed)
          && std::abs(reconstruction_residual)
              <= kControlFactor * kTolerance * reconstruction_scale;
      observation.worst_control_residual = std::max(
          observation.worst_control_residual,
          std::abs(reconstruction_residual));
      observation.controls_pass = observation.controls_pass
          && audit_controls(
              sample, result, observation.worst_control_residual);

      auto& track = tracks[sample.key];
      if (track.last_tick == tick - 1
          && track.chirality == sample.chirality) {
        ++track.current_run;
      } else {
        track.current_run = 1;
      }
      track.last_tick = tick;
      track.chirality = sample.chirality;
      ++track.observations;
      track.maximum_run = std::max(track.maximum_run, track.current_run);
      observation.longest_current_run = std::max(
          observation.longest_current_run, track.current_run);

      ++observation.valid_pairs;
      observation.positive_chirality += sample.chirality > 0 ? 1 : 0;
      observation.negative_chirality += sample.chirality < 0 ? 1 : 0;
      observation.maximum_abs_wedge = std::max(
          observation.maximum_abs_wedge, std::abs(sample.phase_wedge));
      wedge_square_sum += static_cast<long double>(sample.phase_wedge)
          * static_cast<long double>(sample.phase_wedge);
      observation.samples.push_back(sample);
    }
  }

  if (observation.valid_pairs > 0) {
    observation.rms_wedge = std::sqrt(static_cast<double>(
        wedge_square_sum / observation.valid_pairs));
  }

  std::vector<std::size_t> order(observation.samples.size());
  for (std::size_t i = 0; i < order.size(); ++i) order[i] = i;
  std::sort(order.begin(), order.end(), [&](std::size_t left, std::size_t right) {
    const auto& a = observation.samples[left];
    const auto& b = observation.samples[right];
    return std::tie(a.key.negative_id, a.key.positive_id)
        < std::tie(b.key.negative_id, b.key.positive_id);
  });
  if (!order.empty()) {
    for (std::size_t rank = 0; rank < order.size(); ++rank) {
      const auto& positive_sample = observation.samples[order[rank]];
      const auto& rotated_negative = observation.samples[
          order[(rank + 1) % order.size()]];
      PairSample randomized = positive_sample;
      randomized.negative_flux = rotated_negative.negative_flux;
      randomized.negative_wave_velocity =
          rotated_negative.negative_wave_velocity;
      const auto randomized_result =
          ftd::eft::analyze_native_ternary_dipole_phase_wedge_memory(
              pair_sites(randomized));
      if (!randomized_result.valid()) continue;
      ++observation.randomized_valid_pairs;
      observation.randomized_maximum_abs_wedge = std::max(
          observation.randomized_maximum_abs_wedge,
          std::abs(randomized_result.phase_wedge));
    }
  }

  observation.finite = std::isfinite(observation.native_wave_energy)
      && std::isfinite(observation.maximum_abs_wedge)
      && std::isfinite(observation.rms_wedge)
      && std::isfinite(observation.randomized_maximum_abs_wedge)
      && std::isfinite(observation.worst_control_residual);
  observation.voxel_hash_after = hash_voxels(bridge.voxels());
  observation.rng_hash_after = bridge.rng_state_hash();
  observation.nonmutating =
      observation.voxel_hash_before == observation.voxel_hash_after
      && observation.rng_hash_before == observation.rng_hash_after;
  return observation;
}

void configure(ftd::RenderBridge& bridge, Family family, std::uint32_t seed) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.strict_validation = true;
  bridge.toggles.wave_propagation = true;
  bridge.toggles.gauss_projection = true;
  bridge.toggles.genesis = true;
  bridge.toggles.coupling = true;
  bridge.toggles.dual_substrate = false;
  bridge.toggles.langevin = family != Family::AxialNoBath;
  if (bridge.toggles.langevin) {
    bridge.toggles.langevin_T = kLangevinTemperature;
    bridge.toggles.langevin_gamma = kLangevinGamma;
  }
  bridge.seed_rng(seed);

  const int center = bridge.lattice().size() / 2;
  if (family == Family::AxialLive || family == Family::AxialNoBath) {
    bridge.inject_flux(center, center, center,
                       {kInjectionAmplitude * ftd::K_GENESIS, 0.0, 0.0});
  } else if (family == Family::DiagonalLive) {
    const double component = kInjectionAmplitude * ftd::K_GENESIS
        / std::sqrt(3.0);
    bridge.inject_flux(center, center, center,
                       {component, component, component});
  }
}

void write_pair_header(std::ofstream& stream) {
  stream << "volume,family,seed,tick,positive_id,negative_id,positive_site,"
      "negative_site,dx,dy,dz,jpx,jpy,jpz,jmx,jmy,jmz,wpx,wpy,wpz,"
      "wmx,wmy,wmz,q_plus,q_minus,p_plus,p_minus,ell,chi\n";
}

void write_pair_samples(
    std::ofstream& stream, int volume, Family family, std::uint32_t seed,
    const std::vector<PairSample>& samples) {
  for (const auto& sample : samples) {
    stream << volume << ',' << family_name(family) << ',' << seed << ','
        << sample.tick << ',' << sample.key.positive_id << ','
        << sample.key.negative_id << ',' << sample.positive_site << ','
        << sample.negative_site << ',' << sample.separation[0] << ','
        << sample.separation[1] << ',' << sample.separation[2] << ','
        << sample.positive_flux[0] << ',' << sample.positive_flux[1] << ','
        << sample.positive_flux[2] << ',' << sample.negative_flux[0] << ','
        << sample.negative_flux[1] << ',' << sample.negative_flux[2] << ','
        << sample.positive_wave_velocity[0] << ','
        << sample.positive_wave_velocity[1] << ','
        << sample.positive_wave_velocity[2] << ','
        << sample.negative_wave_velocity[0] << ','
        << sample.negative_wave_velocity[1] << ','
        << sample.negative_wave_velocity[2] << ','
        << sample.q_positive << ',' << sample.q_negative << ','
        << sample.p_positive << ',' << sample.p_negative << ','
        << sample.phase_wedge << ',' << sample.chirality << '\n';
  }
}

ArmResult run_arm(
    int volume, Family family, std::uint32_t seed,
    std::ofstream& tick_stream, std::ofstream& pair_stream) {
  ArmResult arm;
  arm.volume = volume;
  arm.family = family;
  arm.seed = seed;
  ftd::RenderBridge bridge(volume);
  configure(bridge, family, seed);
  std::map<PairKey, PairTrack> tracks;

  for (int tick = 0; tick < kTicks; ++tick) {
    bridge.run(1);
    const TickObservation observation = observe_tick(bridge, tick, tracks);
    ++arm.ticks;
    arm.finite = arm.finite && observation.finite;
    arm.nonmutating = arm.nonmutating && observation.nonmutating;
    arm.controls_pass = arm.controls_pass && observation.controls_pass;
    arm.reconstructible = arm.reconstructible
        && observation.reconstructible;
    arm.ticks_with_pairs += observation.valid_pairs > 0 ? 1 : 0;
    arm.total_valid_pair_observations += observation.valid_pairs;
    arm.total_randomized_valid_pairs += observation.randomized_valid_pairs;
    arm.total_genesis_events += observation.genesis_events;
    arm.total_evaporation_events += observation.evaporation_events;
    arm.maximum_abs_wedge = std::max(
        arm.maximum_abs_wedge, observation.maximum_abs_wedge);
    arm.randomized_maximum_abs_wedge = std::max(
        arm.randomized_maximum_abs_wedge,
        observation.randomized_maximum_abs_wedge);
    arm.maximum_native_wave_energy = std::max(
        arm.maximum_native_wave_energy, observation.native_wave_energy);
    arm.worst_control_residual = std::max(
        arm.worst_control_residual, observation.worst_control_residual);

    tick_stream << volume << ',' << family_name(family) << ',' << seed << ','
        << tick << ',' << observation.positive_count << ','
        << observation.negative_count << ',' << observation.genesis_events
        << ',' << observation.evaporation_events << ','
        << observation.valid_pairs << ',' << observation.positive_chirality
        << ',' << observation.negative_chirality << ','
        << observation.maximum_abs_wedge << ',' << observation.rms_wedge
        << ',' << observation.native_wave_energy << ','
        << observation.longest_current_run << ','
        << observation.randomized_valid_pairs << ','
        << observation.randomized_maximum_abs_wedge << ','
        << observation.nonmutating << ',' << observation.controls_pass << ','
        << observation.reconstructible << ','
        << observation.worst_control_residual << ','
        << observation.voxel_hash_before << ','
        << observation.voxel_hash_after << ','
        << observation.rng_hash_before << ','
        << observation.rng_hash_after << '\n';
    write_pair_samples(pair_stream, volume, family, seed,
                       observation.samples);
  }

  arm.unique_pairs = static_cast<int>(tracks.size());
  for (const auto& item : tracks) {
    arm.maximum_sign_stable_run = std::max(
        arm.maximum_sign_stable_run, item.second.maximum_run);
    arm.persistent_pair_count +=
        item.second.maximum_run >= kPersistenceTicks ? 1 : 0;
  }
  arm.executed = arm.ticks == kTicks;
  return arm;
}

std::filesystem::path result_directory() {
  return std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0908";
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  const auto directory = result_directory();
  std::filesystem::create_directories(directory);
  std::ofstream ticks(directory / "ftd_0908_tick_census_v1.csv");
  std::ofstream pairs(directory / "ftd_0908_pair_observations_v1.csv");
  ticks << std::setprecision(17);
  pairs << std::setprecision(17);
  ticks << "volume,family,seed,tick,positive_count,negative_count,"
      "genesis_events,evaporation_events,valid_pairs,chi_positive,"
      "chi_negative,max_abs_ell,rms_ell,native_wave_energy,"
      "longest_current_run,randomized_valid_pairs,randomized_max_abs_ell,"
      "nonmutating,controls_pass,reconstructible,worst_control_residual,"
      "voxel_hash_before,voxel_hash_after,rng_hash_before,rng_hash_after\n";
  write_pair_header(pairs);

  std::vector<ArmResult> arms;
  arms.reserve(kVolumes.size() * kSeeds.size() * kFamilies.size());
  for (int volume : kVolumes) {
    for (std::uint32_t seed : kSeeds) {
      for (Family family : kFamilies) {
        std::cout << "RUN volume=" << volume
                  << " family=" << family_name(family)
                  << " seed=" << seed << '\n';
        arms.push_back(run_arm(volume, family, seed, ticks, pairs));
        const auto& arm = arms.back();
        std::cout << "ARM max_run=" << arm.maximum_sign_stable_run
                  << " valid_observations="
                  << arm.total_valid_pair_observations
                  << " unique_pairs=" << arm.unique_pairs
                  << " nonmutating=" << arm.nonmutating
                  << " controls=" << arm.controls_pass << '\n';
      }
    }
  }

  const bool matrix_complete = arms.size() == 32
      && std::all_of(arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.executed && arm.ticks == kTicks;
      });
  const bool finite = std::all_of(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.finite;
      });
  const bool nonmutating = std::all_of(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.nonmutating;
      });
  const bool controls = std::all_of(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.controls_pass && arm.reconstructible;
      });
  const bool protocol_valid = matrix_complete && finite
      && nonmutating && controls;

  std::map<std::pair<int, Family>, int> seed_pass_counts;
  bool any_live_pair = false;
  for (const auto& arm : arms) {
    if (is_live_injected(arm.family)) {
      any_live_pair = any_live_pair
          || arm.total_valid_pair_observations > 0;
      if (arm.maximum_sign_stable_run >= kPersistenceTicks)
        ++seed_pass_counts[{arm.volume, arm.family}];
    }
  }
  bool outcome_a = protocol_valid;
  for (int volume : kVolumes) {
    for (Family family : {Family::AxialLive,
                          Family::DiagonalLive,
                          Family::AxialNoBath}) {
      outcome_a = outcome_a
          && seed_pass_counts[{volume, family}] >= 3;
    }
  }

  std::string verdict;
  if (!protocol_valid) {
    verdict = "PROTOCOL_INVALID_NO_FORMATION_VERDICT";
  } else if (outcome_a) {
    verdict = "CROSS_VOLUME_PERSISTENT_ORIENTATION_MEMORY_CANDIDATES";
  } else if (any_live_pair) {
    verdict = "FORMATION_WITHOUT_CROSS_VOLUME_PERSISTENCE";
  } else {
    verdict = "NO_OBSERVED_LOCAL_ORIENTATION_MEMORY_FORMATION";
  }

  std::ofstream summary(directory / "ftd_0908_summary_v1.json");
  summary << std::setprecision(17)
      << "{\n"
      << "  \"identifier\": \"FTD-0908\",\n"
      << "  \"protocol_sha256\": \"" << kProtocolSha256 << "\",\n"
      << "  \"arm_count\": " << arms.size() << ",\n"
      << "  \"ticks_per_arm\": " << kTicks << ",\n"
      << "  \"persistence_threshold\": " << kPersistenceTicks << ",\n"
      << "  \"matrix_complete\": "
      << (matrix_complete ? "true" : "false") << ",\n"
      << "  \"finite\": " << (finite ? "true" : "false") << ",\n"
      << "  \"nonmutating\": "
      << (nonmutating ? "true" : "false") << ",\n"
      << "  \"controls_pass\": "
      << (controls ? "true" : "false") << ",\n"
      << "  \"protocol_valid\": "
      << (protocol_valid ? "true" : "false") << ",\n"
      << "  \"any_live_pair\": "
      << (any_live_pair ? "true" : "false") << ",\n"
      << "  \"verdict\": \"" << verdict << "\",\n"
      << "  \"arms\": [\n";
  for (std::size_t index = 0; index < arms.size(); ++index) {
    const auto& arm = arms[index];
    summary << "    {\"volume\": " << arm.volume
        << ", \"family\": \"" << family_name(arm.family) << "\""
        << ", \"seed\": " << arm.seed
        << ", \"maximum_sign_stable_run\": "
        << arm.maximum_sign_stable_run
        << ", \"persistent_pair_count\": " << arm.persistent_pair_count
        << ", \"valid_pair_observations\": "
        << arm.total_valid_pair_observations
        << ", \"unique_pairs\": " << arm.unique_pairs
        << ", \"ticks_with_pairs\": " << arm.ticks_with_pairs
        << ", \"genesis_events\": " << arm.total_genesis_events
        << ", \"evaporation_events\": " << arm.total_evaporation_events
        << ", \"maximum_abs_wedge\": " << arm.maximum_abs_wedge
        << ", \"randomized_valid_pairs\": "
        << arm.total_randomized_valid_pairs
        << ", \"randomized_maximum_abs_wedge\": "
        << arm.randomized_maximum_abs_wedge
        << ", \"maximum_native_wave_energy\": "
        << arm.maximum_native_wave_energy
        << ", \"worst_control_residual\": "
        << arm.worst_control_residual
        << ", \"finite\": " << (arm.finite ? "true" : "false")
        << ", \"nonmutating\": "
        << (arm.nonmutating ? "true" : "false")
        << ", \"controls_pass\": "
        << (arm.controls_pass ? "true" : "false")
        << ", \"reconstructible\": "
        << (arm.reconstructible ? "true" : "false") << "}"
        << (index + 1 == arms.size() ? "\n" : ",\n");
  }
  summary << "  ]\n}\n";

  std::cout << "FTD-0908 protocol=" << kProtocolSha256
            << " arms=" << arms.size()
            << " matrix_complete=" << matrix_complete
            << " finite=" << finite
            << " nonmutating=" << nonmutating
            << " controls=" << controls
            << " verdict=" << verdict << '\n';
  std::cout << "CENTRAL_MEMORY_LAW_TESTED=FALSE\n";
  std::cout << "MAINTENANCE_ERASURE_WORK_CLOSED=FALSE\n";
  std::cout << "PRODUCTION_TICK_MODIFIED=FALSE\n";
  return protocol_valid ? 0 : 1;
}
