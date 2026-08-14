/**
 * @file campaign_held_out_pair_specific_phase_wedge_centrality.cpp
 * @brief FTD-0911 locked held-out pair-specificity/centrality census.
 *
 * Observation only. No production source, toggle, state, RNG, or tick phase
 * is changed by the observer. The protocol was locked before this file.
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
#include <iterator>
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
    "D0C7976FE334EA5D814D40DADEDBEF9CB8419B0A518AFE0492C2F3A183FF88FE";
constexpr int kTicks = 128;
constexpr int kMinimumPairRun = 8;
constexpr int kMinimumCommonSupport = 32;
constexpr int kCellSeedGate = 6;
constexpr int kCentralQualifiedSeedGate = 12;
constexpr double kTolerance = 1e-11;
constexpr double kControlFactor = 256.0;
constexpr double kInjectionAmplitude = 10.0;
constexpr double kLangevinTemperature = 0.005;
constexpr double kLangevinGamma = 0.02;
constexpr std::array<int, 2> kVolumes{{19, 23}};
constexpr std::array<std::uint32_t, 8> kSeeds{{
    0x09110001u, 0x09110002u, 0x09110003u, 0x09110004u,
    0x09110005u, 0x09110006u, 0x09110007u, 0x09110008u}};
constexpr std::array<int, 4> kChronologyLags{{1, 2, 4, 8}};

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

bool is_live_family(Family family) {
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

using PairHistory = std::map<int, PairSample>;
using ArmHistories = std::map<PairKey, PairHistory>;

struct TickObservation {
  bool finite = true;
  bool nonmutating = true;
  bool controls_pass = true;
  bool reconstructible = true;
  int valid_pairs = 0;
  int positive_count = 0;
  int negative_count = 0;
  long long genesis_events = 0;
  long long evaporation_events = 0;
  double native_wave_energy = 0.0;
  double worst_control_residual = 0.0;
  std::uint64_t voxel_hash_before = 0;
  std::uint64_t voxel_hash_after = 0;
  std::uint64_t rng_hash_before = 0;
  std::uint64_t rng_hash_after = 0;
  std::vector<PairSample> samples;
};

struct PairDiscriminatorResult {
  bool qualified = false;
  bool pass = false;
  bool all_pseudo_wedges_valid = true;
  int retained_pairs = 0;
  int common_start = -1;
  int common_end = -2;
  int common_length = 0;
  int actual_same = 0;
  int actual_flip = 0;
  int maximum_null_same = -1;
  int minimum_null_same = -1;
  int null_shift_count = 0;
};

struct CentralLedgerResult {
  bool identity_pass = true;
  bool exact_central_pass = true;
  long long transitions = 0;
  long long identity_failures = 0;
  long long central_failures = 0;
  double maximum_identity_residual = 0.0;
  double maximum_abs_delta_wedge = 0.0;
  double maximum_abs_torque_p = 0.0;
  double maximum_abs_torque_q = 0.0;
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
  int unique_pairs = 0;
  long long valid_pair_observations = 0;
  long long genesis_events = 0;
  long long evaporation_events = 0;
  PairDiscriminatorResult pair_discriminator{};
  CentralLedgerResult central{};
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

double dot(
    const NativeOrientationVector& left,
    const NativeOrientationVector& right) {
  return left[0] * right[0] + left[1] * right[1]
      + left[2] * right[2];
}

double norm(const NativeOrientationVector& value) {
  return std::sqrt(dot(value, value));
}

double max_abs(const NativeOrientationVector& value) {
  return std::max({std::abs(value[0]), std::abs(value[1]),
                   std::abs(value[2])});
}

double max_abs_difference(
    const NativeOrientationVector& left,
    const NativeOrientationVector& right) {
  return std::max({std::abs(left[0] - right[0]),
                   std::abs(left[1] - right[1]),
                   std::abs(left[2] - right[2])});
}

double wedge2(
    const std::array<double, 2>& left,
    const std::array<double, 2>& right) {
  return left[0] * right[1] - left[1] * right[0];
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
      1.0, std::abs(original.phase_wedge), max_abs(original.polar_axis)});
  const double accepted = kControlFactor * kTolerance * scale;
  bool pass = true;
  auto record = [&](double residual) {
    worst_residual = std::max(worst_residual, std::abs(residual));
    pass = pass && std::isfinite(residual)
        && std::abs(residual) <= accepted;
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
  record(original.gram_wedge_square_residual);
  record(original.swept_area_full_time_reversal_residual);
  pass = pass && !original.one_step_swept_area_time_odd_memory;
  return pass;
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

TickObservation observe_tick(const ftd::RenderBridge& bridge, int tick) {
  TickObservation observation;
  observation.voxel_hash_before = hash_voxels(bridge.voxels());
  observation.rng_hash_before = bridge.rng_state_hash();
  observation.genesis_events = bridge.genesis_events_this_tick();
  observation.evaporation_events = bridge.evaporation_events_this_tick();
  const auto& voxels = bridge.voxels();
  std::set<PairKey> seen;
  for (std::size_t index = 0; index < voxels.size(); ++index) {
    const auto& positive = voxels[index];
    observation.positive_count += positive.state > 0 ? 1 : 0;
    observation.negative_count += positive.state < 0 ? 1 : 0;
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
      if (!seen.insert(sample.key).second) continue;
      const double rebuilt = sample.q_positive * sample.p_negative
          - sample.q_negative * sample.p_positive;
      const double scale = std::max(1.0, std::abs(sample.phase_wedge));
      const double residual = rebuilt - sample.phase_wedge;
      observation.worst_control_residual = std::max(
          observation.worst_control_residual, std::abs(residual));
      observation.reconstructible = observation.reconstructible
          && std::isfinite(rebuilt)
          && std::abs(residual) <= kControlFactor * kTolerance * scale;
      observation.controls_pass = observation.controls_pass
          && audit_controls(
              sample, result, observation.worst_control_residual);
      observation.samples.push_back(sample);
    }
  }
  observation.valid_pairs = static_cast<int>(observation.samples.size());
  observation.finite = std::isfinite(observation.native_wave_energy)
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

int longest_presence_run(const PairHistory& history) {
  int maximum = 0;
  int current = 0;
  int previous = -2;
  for (const auto& item : history) {
    current = item.first == previous + 1 ? current + 1 : 1;
    maximum = std::max(maximum, current);
    previous = item.first;
  }
  return maximum;
}

std::pair<int, int> longest_common_interval(
    const std::vector<const PairHistory*>& histories) {
  if (histories.empty()) return {-1, -2};
  std::set<int> common;
  for (const auto& item : *histories.front()) common.insert(item.first);
  for (std::size_t index = 1; index < histories.size(); ++index) {
    for (auto iterator = common.begin(); iterator != common.end();) {
      if (histories[index]->count(*iterator) == 0) {
        iterator = common.erase(iterator);
      } else {
        ++iterator;
      }
    }
  }
  int best_start = -1;
  int best_end = -2;
  int start = -1;
  int previous = -2;
  for (int tick : common) {
    if (tick != previous + 1) start = tick;
    if (tick - start > best_end - best_start) {
      best_start = start;
      best_end = tick;
    }
    previous = tick;
  }
  return {best_start, best_end};
}

bool pseudo_chirality(
    const PairSample& positive_sample,
    const PairSample& negative_sample,
    int& chirality, double& wedge) {
  const double axis_norm = norm(positive_sample.separation);
  if (!std::isfinite(axis_norm) || axis_norm <= kTolerance) return false;
  NativeOrientationVector axis{
      positive_sample.separation[0] / axis_norm,
      positive_sample.separation[1] / axis_norm,
      positive_sample.separation[2] / axis_norm};
  const double q_positive = dot(axis, positive_sample.positive_flux);
  const double p_positive = dot(
      axis, positive_sample.positive_wave_velocity);
  const double q_negative = dot(axis, negative_sample.negative_flux);
  const double p_negative = dot(
      axis, negative_sample.negative_wave_velocity);
  wedge = q_positive * p_negative - q_negative * p_positive;
  const double scale = std::max({
      1.0, std::abs(q_positive * p_negative),
      std::abs(q_negative * p_positive)});
  if (!std::isfinite(wedge)
      || std::abs(wedge) <= kTolerance * scale) return false;
  chirality = wedge > 0.0 ? 1 : -1;
  return true;
}

PairDiscriminatorResult analyze_pair_specificity(
    const ArmHistories& histories,
    int volume, Family family, std::uint32_t seed,
    std::ofstream& derangement_stream,
    std::ofstream& chronology_stream) {
  PairDiscriminatorResult result;
  std::vector<std::pair<PairKey, const PairHistory*>> retained;
  for (const auto& item : histories) {
    if (longest_presence_run(item.second) >= kMinimumPairRun)
      retained.push_back({item.first, &item.second});
  }
  result.retained_pairs = static_cast<int>(retained.size());
  std::vector<const PairHistory*> retained_histories;
  for (const auto& item : retained) retained_histories.push_back(item.second);
  const auto interval = longest_common_interval(retained_histories);
  result.common_start = interval.first;
  result.common_end = interval.second;
  result.common_length = interval.second >= interval.first
      ? interval.second - interval.first + 1 : 0;
  result.qualified = result.retained_pairs >= 2
      && result.common_length >= kMinimumCommonSupport;
  if (!result.qualified) return result;

  for (const auto& item : retained) {
    for (int tick = result.common_start; tick < result.common_end; ++tick) {
      const int current = item.second->at(tick).chirality;
      const int next = item.second->at(tick + 1).chirality;
      result.actual_same += current == next ? 1 : 0;
      result.actual_flip += current != next ? 1 : 0;
    }
  }

  for (int shift = 1; shift < result.retained_pairs; ++shift) {
    int same = 0;
    int flips = 0;
    bool valid = true;
    for (int index = 0; index < result.retained_pairs; ++index) {
      const int negative_index = (index + shift) % result.retained_pairs;
      int previous_chirality = 0;
      for (int tick = result.common_start;
           tick <= result.common_end; ++tick) {
        int chirality = 0;
        double wedge = 0.0;
        valid = valid && pseudo_chirality(
            retained[static_cast<std::size_t>(index)].second->at(tick),
            retained[static_cast<std::size_t>(negative_index)].second->at(tick),
            chirality, wedge);
        if (!valid) break;
        if (tick > result.common_start) {
          same += chirality == previous_chirality ? 1 : 0;
          flips += chirality != previous_chirality ? 1 : 0;
        }
        previous_chirality = chirality;
      }
      if (!valid) break;
    }
    result.all_pseudo_wedges_valid =
        result.all_pseudo_wedges_valid && valid;
    if (valid) {
      result.maximum_null_same = std::max(result.maximum_null_same, same);
      result.minimum_null_same = result.minimum_null_same < 0
          ? same : std::min(result.minimum_null_same, same);
    }
    ++result.null_shift_count;
    derangement_stream << volume << ',' << family_name(family) << ','
        << seed << ',' << result.retained_pairs << ','
        << result.common_start << ',' << result.common_end << ','
        << shift << ',' << valid << ',' << same << ',' << flips << '\n';
  }
  result.pass = result.all_pseudo_wedges_valid
      && result.maximum_null_same >= 0
      && result.actual_same > result.maximum_null_same;

  for (int lag : kChronologyLags) {
    int same = 0;
    int flips = 0;
    int observations = 0;
    bool valid = true;
    for (const auto& item : retained) {
      int previous_chirality = 0;
      bool have_previous = false;
      for (int tick = result.common_start;
           tick + lag <= result.common_end; ++tick) {
        int chirality = 0;
        double wedge = 0.0;
        valid = valid && pseudo_chirality(
            item.second->at(tick), item.second->at(tick + lag),
            chirality, wedge);
        if (!valid) break;
        if (have_previous) {
          same += chirality == previous_chirality ? 1 : 0;
          flips += chirality != previous_chirality ? 1 : 0;
        }
        previous_chirality = chirality;
        have_previous = true;
        ++observations;
      }
      if (!valid) break;
    }
    chronology_stream << volume << ',' << family_name(family) << ','
        << seed << ',' << lag << ',' << valid << ',' << observations << ','
        << same << ',' << flips << '\n';
  }
  return result;
}

CentralLedgerResult analyze_centrality(
    const ArmHistories& histories,
    int volume, Family family, std::uint32_t seed,
    std::ofstream& transition_stream) {
  CentralLedgerResult result;
  for (const auto& history_item : histories) {
    const auto& history = history_item.second;
    if (history.size() < 2) continue;
    auto previous = history.begin();
    for (auto current = std::next(previous);
         current != history.end(); ++current) {
      if (current->first != previous->first + 1) {
        previous = current;
        continue;
      }
      const auto& before = previous->second;
      const auto& after = current->second;
      const std::array<double, 2> q_before{{
          before.q_positive, before.q_negative}};
      const std::array<double, 2> p_before{{
          before.p_positive, before.p_negative}};
      const std::array<double, 2> q_after{{
          after.q_positive, after.q_negative}};
      const std::array<double, 2> p_after{{
          after.p_positive, after.p_negative}};
      const std::array<double, 2> q_bar{{
          0.5 * (q_before[0] + q_after[0]),
          0.5 * (q_before[1] + q_after[1])}};
      const std::array<double, 2> p_bar{{
          0.5 * (p_before[0] + p_after[0]),
          0.5 * (p_before[1] + p_after[1])}};
      const std::array<double, 2> delta_q{{
          q_after[0] - q_before[0], q_after[1] - q_before[1]}};
      const std::array<double, 2> delta_p{{
          p_after[0] - p_before[0], p_after[1] - p_before[1]}};
      const double delta_wedge = after.phase_wedge - before.phase_wedge;
      const double torque_p = wedge2(q_bar, delta_p);
      const double torque_q = wedge2(delta_q, p_bar);
      const double identity_residual =
          delta_wedge - torque_p - torque_q;
      const double scale = std::max({
          1.0, std::abs(after.phase_wedge), std::abs(before.phase_wedge),
          std::abs(q_bar[0] * delta_p[1]),
          std::abs(q_bar[1] * delta_p[0]),
          std::abs(delta_q[0] * p_bar[1]),
          std::abs(delta_q[1] * p_bar[0])});
      const double accepted = kControlFactor * kTolerance * scale;
      const bool identity_pass = std::isfinite(identity_residual)
          && std::abs(identity_residual) <= accepted;
      const bool central_pass = identity_pass
          && std::abs(delta_wedge) <= accepted
          && std::abs(torque_p) <= accepted
          && std::abs(torque_q) <= accepted;
      ++result.transitions;
      result.identity_failures += identity_pass ? 0 : 1;
      result.central_failures += central_pass ? 0 : 1;
      result.identity_pass = result.identity_pass && identity_pass;
      result.exact_central_pass =
          result.exact_central_pass && central_pass;
      result.maximum_identity_residual = std::max(
          result.maximum_identity_residual, std::abs(identity_residual));
      result.maximum_abs_delta_wedge = std::max(
          result.maximum_abs_delta_wedge, std::abs(delta_wedge));
      result.maximum_abs_torque_p = std::max(
          result.maximum_abs_torque_p, std::abs(torque_p));
      result.maximum_abs_torque_q = std::max(
          result.maximum_abs_torque_q, std::abs(torque_q));
      transition_stream << volume << ',' << family_name(family) << ','
          << seed << ',' << history_item.first.positive_id << ','
          << history_item.first.negative_id << ',' << before.tick << ','
          << after.tick << ',' << before.phase_wedge << ','
          << after.phase_wedge << ',' << delta_wedge << ',' << torque_p << ','
          << torque_q << ',' << identity_residual << ',' << accepted << ','
          << identity_pass << ',' << central_pass << '\n';
      previous = current;
    }
  }
  if (result.transitions == 0) result.exact_central_pass = false;
  return result;
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
    std::ofstream& tick_stream, std::ofstream& pair_stream,
    std::ofstream& derangement_stream,
    std::ofstream& chronology_stream,
    std::ofstream& transition_stream) {
  ArmResult arm;
  arm.volume = volume;
  arm.family = family;
  arm.seed = seed;
  ftd::RenderBridge bridge(volume);
  configure(bridge, family, seed);
  ArmHistories histories;
  for (int tick = 0; tick < kTicks; ++tick) {
    bridge.run(1);
    const TickObservation observation = observe_tick(bridge, tick);
    ++arm.ticks;
    arm.finite = arm.finite && observation.finite;
    arm.nonmutating = arm.nonmutating && observation.nonmutating;
    arm.controls_pass = arm.controls_pass && observation.controls_pass;
    arm.reconstructible = arm.reconstructible
        && observation.reconstructible;
    arm.valid_pair_observations += observation.valid_pairs;
    arm.genesis_events += observation.genesis_events;
    arm.evaporation_events += observation.evaporation_events;
    for (const auto& sample : observation.samples)
      histories[sample.key][sample.tick] = sample;
    tick_stream << volume << ',' << family_name(family) << ',' << seed << ','
        << tick << ',' << observation.positive_count << ','
        << observation.negative_count << ',' << observation.genesis_events
        << ',' << observation.evaporation_events << ','
        << observation.valid_pairs << ',' << observation.native_wave_energy
        << ',' << observation.nonmutating << ',' << observation.controls_pass
        << ',' << observation.reconstructible << ','
        << observation.worst_control_residual << ','
        << observation.voxel_hash_before << ','
        << observation.voxel_hash_after << ','
        << observation.rng_hash_before << ','
        << observation.rng_hash_after << '\n';
    write_pair_samples(pair_stream, volume, family, seed,
                       observation.samples);
  }
  arm.unique_pairs = static_cast<int>(histories.size());
  arm.pair_discriminator = analyze_pair_specificity(
      histories, volume, family, seed,
      derangement_stream, chronology_stream);
  arm.central = analyze_centrality(
      histories, volume, family, seed, transition_stream);
  arm.executed = arm.ticks == kTicks;
  return arm;
}

std::filesystem::path result_directory() {
  return std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0911";
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  const auto directory = result_directory();
  std::filesystem::create_directories(directory);
  std::ofstream ticks(directory / "ftd_0911_tick_census_v1.csv");
  std::ofstream pairs(directory / "ftd_0911_pair_observations_v1.csv");
  std::ofstream derangements(directory / "ftd_0911_derangements_v1.csv");
  std::ofstream chronology(directory / "ftd_0911_chronology_controls_v1.csv");
  std::ofstream transitions(directory / "ftd_0911_central_transitions_v1.csv");
  ticks << std::setprecision(17);
  pairs << std::setprecision(17);
  derangements << std::setprecision(17);
  chronology << std::setprecision(17);
  transitions << std::setprecision(17);
  ticks << "volume,family,seed,tick,positive_count,negative_count,"
      "genesis_events,evaporation_events,valid_pairs,native_wave_energy,"
      "nonmutating,controls_pass,reconstructible,worst_control_residual,"
      "voxel_hash_before,voxel_hash_after,rng_hash_before,rng_hash_after\n";
  pairs << "volume,family,seed,tick,positive_id,negative_id,positive_site,"
      "negative_site,dx,dy,dz,jpx,jpy,jpz,jmx,jmy,jmz,wpx,wpy,wpz,"
      "wmx,wmy,wmz,q_plus,q_minus,p_plus,p_minus,ell,chi\n";
  derangements << "volume,family,seed,retained_pairs,common_start,common_end,"
      "shift,valid,same,flips\n";
  chronology << "volume,family,seed,lag,valid,observations,same,flips\n";
  transitions << "volume,family,seed,positive_id,negative_id,tick_before,"
      "tick_after,ell_before,ell_after,delta_ell,torque_p,torque_q,"
      "identity_residual,accepted,identity_pass,central_pass\n";

  std::vector<ArmResult> arms;
  arms.reserve(kVolumes.size() * kSeeds.size() * kFamilies.size());
  for (int volume : kVolumes) {
    for (std::uint32_t seed : kSeeds) {
      for (Family family : kFamilies) {
        std::cout << "RUN volume=" << volume
                  << " family=" << family_name(family)
                  << " seed=" << seed << '\n';
        arms.push_back(run_arm(
            volume, family, seed, ticks, pairs, derangements,
            chronology, transitions));
        const auto& arm = arms.back();
        std::cout << "ARM pairs=" << arm.unique_pairs
                  << " pair_qualified=" << arm.pair_discriminator.qualified
                  << " pair_pass=" << arm.pair_discriminator.pass
                  << " actual_same=" << arm.pair_discriminator.actual_same
                  << " null_max="
                  << arm.pair_discriminator.maximum_null_same
                  << " central_transitions=" << arm.central.transitions
                  << " central_failures=" << arm.central.central_failures
                  << '\n';
      }
    }
  }

  const bool matrix_complete = arms.size() == 64
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
        return arm.controls_pass && arm.reconstructible
            && arm.central.identity_pass;
      });
  const bool protocol_valid = matrix_complete && finite
      && nonmutating && controls;

  std::map<std::pair<int, Family>, int> pair_qualified_counts;
  std::map<std::pair<int, Family>, int> pair_pass_counts;
  int central_qualified_seeds = 0;
  bool central_all_pass = true;
  for (const auto& arm : arms) {
    if (is_live_family(arm.family) && arm.pair_discriminator.qualified) {
      ++pair_qualified_counts[{arm.volume, arm.family}];
      pair_pass_counts[{arm.volume, arm.family}] +=
          arm.pair_discriminator.pass ? 1 : 0;
    }
    if (arm.family == Family::AxialNoBath
        && arm.central.transitions > 0) {
      ++central_qualified_seeds;
      central_all_pass = central_all_pass
          && arm.central.exact_central_pass;
    }
  }

  bool pair_qualified = true;
  bool pair_pass = true;
  for (int volume : kVolumes) {
    for (Family family : {Family::AxialLive,
                          Family::DiagonalLive,
                          Family::AxialNoBath}) {
      pair_qualified = pair_qualified
          && pair_qualified_counts[{volume, family}] >= kCellSeedGate;
      pair_pass = pair_pass
          && pair_pass_counts[{volume, family}] >= kCellSeedGate;
    }
  }
  const bool central_qualified =
      central_qualified_seeds >= kCentralQualifiedSeedGate;
  const bool central_pass = central_qualified && central_all_pass;

  std::string verdict;
  if (!protocol_valid) {
    verdict = "PROTOCOL_INVALID_NO_PAIR_OR_CENTRALITY_VERDICT";
  } else if (!pair_qualified || !central_qualified) {
    verdict = "OUTCOME_U_UNQUALIFIED";
  } else if (pair_pass && central_pass) {
    verdict = "OUTCOME_A_PAIR_SPECIFIC_AND_EXACT_CENTRAL";
  } else if (pair_pass && !central_pass) {
    verdict = "OUTCOME_B_PAIR_SPECIFIC_NOT_EXACT_CENTRAL";
  } else if (!pair_pass && central_pass) {
    verdict = "OUTCOME_C_NOT_PAIR_SPECIFIC_BUT_EXACT_CENTRAL";
  } else {
    verdict = "OUTCOME_D_NOT_PAIR_SPECIFIC_NOT_EXACT_CENTRAL";
  }

  std::ofstream summary(directory / "ftd_0911_summary_v1.json");
  summary << std::setprecision(17)
      << "{\n"
      << "  \"identifier\": \"FTD-0911\",\n"
      << "  \"protocol_sha256\": \"" << kProtocolSha256 << "\",\n"
      << "  \"arm_count\": " << arms.size() << ",\n"
      << "  \"ticks_per_arm\": " << kTicks << ",\n"
      << "  \"matrix_complete\": "
      << (matrix_complete ? "true" : "false") << ",\n"
      << "  \"finite\": " << (finite ? "true" : "false") << ",\n"
      << "  \"nonmutating\": "
      << (nonmutating ? "true" : "false") << ",\n"
      << "  \"controls_pass\": "
      << (controls ? "true" : "false") << ",\n"
      << "  \"protocol_valid\": "
      << (protocol_valid ? "true" : "false") << ",\n"
      << "  \"pair_qualified\": "
      << (pair_qualified ? "true" : "false") << ",\n"
      << "  \"pair_pass\": " << (pair_pass ? "true" : "false")
      << ",\n"
      << "  \"central_qualified_seeds\": "
      << central_qualified_seeds << ",\n"
      << "  \"central_qualified\": "
      << (central_qualified ? "true" : "false") << ",\n"
      << "  \"central_pass\": "
      << (central_pass ? "true" : "false") << ",\n"
      << "  \"verdict\": \"" << verdict << "\",\n"
      << "  \"arms\": [\n";
  for (std::size_t index = 0; index < arms.size(); ++index) {
    const auto& arm = arms[index];
    const auto& pair = arm.pair_discriminator;
    summary << "    {\"volume\": " << arm.volume
        << ", \"family\": \"" << family_name(arm.family) << "\""
        << ", \"seed\": " << arm.seed
        << ", \"valid_pair_observations\": "
        << arm.valid_pair_observations
        << ", \"unique_pairs\": " << arm.unique_pairs
        << ", \"genesis_events\": " << arm.genesis_events
        << ", \"evaporation_events\": " << arm.evaporation_events
        << ", \"pair_qualified\": "
        << (pair.qualified ? "true" : "false")
        << ", \"pair_pass\": " << (pair.pass ? "true" : "false")
        << ", \"retained_pairs\": " << pair.retained_pairs
        << ", \"common_start\": " << pair.common_start
        << ", \"common_end\": " << pair.common_end
        << ", \"common_length\": " << pair.common_length
        << ", \"actual_same\": " << pair.actual_same
        << ", \"actual_flip\": " << pair.actual_flip
        << ", \"maximum_null_same\": " << pair.maximum_null_same
        << ", \"minimum_null_same\": " << pair.minimum_null_same
        << ", \"null_shift_count\": " << pair.null_shift_count
        << ", \"all_pseudo_wedges_valid\": "
        << (pair.all_pseudo_wedges_valid ? "true" : "false")
        << ", \"central_transitions\": " << arm.central.transitions
        << ", \"central_failures\": " << arm.central.central_failures
        << ", \"identity_failures\": " << arm.central.identity_failures
        << ", \"maximum_identity_residual\": "
        << arm.central.maximum_identity_residual
        << ", \"maximum_abs_delta_wedge\": "
        << arm.central.maximum_abs_delta_wedge
        << ", \"maximum_abs_torque_p\": "
        << arm.central.maximum_abs_torque_p
        << ", \"maximum_abs_torque_q\": "
        << arm.central.maximum_abs_torque_q
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

  std::cout << "FTD-0911 protocol=" << kProtocolSha256
            << " arms=" << arms.size()
            << " matrix_complete=" << matrix_complete
            << " finite=" << finite
            << " nonmutating=" << nonmutating
            << " controls=" << controls
            << " pair_qualified=" << pair_qualified
            << " pair_pass=" << pair_pass
            << " central_qualified=" << central_qualified
            << " central_pass=" << central_pass
            << " verdict=" << verdict << '\n';
  std::cout << "PERTURBATION_APPLIED=FALSE\n";
  std::cout << "MAINTENANCE_ERASURE_WORK_CLOSED=FALSE\n";
  std::cout << "PRODUCTION_TICK_MODIFIED=FALSE\n";
  return protocol_valid ? 0 : 1;
}
