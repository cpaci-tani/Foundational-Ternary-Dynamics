/**
 * @file campaign_bound_pilot_wake_discriminator.cpp
 * @brief FTD-0475 bound-field / leading-response / wake discriminator v2.
 */

#include "ftd/eft/localized_transverse_packet.h"
#include "ftd/eft/wave_morphology_observer.h"
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
#include <numeric>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr double kSigmaX = 3.0;
constexpr double kSigmaT = 3.0;
constexpr double kCoreHalfWidth = 2.0 * kSigmaX;
constexpr int kFinalTick = 32;
constexpr std::array<int, 9> kSampleTicks{{0, 4, 8, 12, 16, 20, 24, 28, 32}};
constexpr std::array<int, 2> kLengths{{49, 65}};
constexpr std::array<int, 2> kDirections{{-1, +1}};
constexpr std::array<double, 2> kAmplitudes{{0.5, 1.0}};

void configure_wave_only(ftd::RenderBridge& bridge) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.wave_propagation = true;
  bridge.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
  bridge.toggles.dual_substrate = false;
  bridge.toggles.strict_validation = true;
}

bool is_sample_tick(int tick) {
  return std::find(kSampleTicks.begin(), kSampleTicks.end(), tick)
      != kSampleTicks.end();
}

double periodic_delta(double value, double center, int length) {
  double delta = value - center;
  while (delta > 0.5 * length) delta -= length;
  while (delta < -0.5 * length) delta += length;
  return delta;
}

struct MorphologySample {
  int tick = 0;
  ftd::eft::WaveMorphologyObservation observation;
  ftd::eft::WaveProfileComparison comparison;
  double displacement = 0.0;
  double relative_energy_drift = 0.0;
};

struct PacketRun {
  std::string arm;
  int length = 0;
  int direction = 1;
  double amplitude = 0.0;
  std::vector<MorphologySample> samples;
  bool valid = false;
  bool bound = false;
  bool trailing_wake = false;
  bool symmetric_dispersion = false;
};

PacketRun run_localized_packet(int length, int direction, double amplitude) {
  PacketRun out;
  out.arm = "localized_finite";
  out.length = length;
  out.direction = direction;
  out.amplitude = amplitude;
  ftd::RenderBridge bridge(length);
  configure_wave_only(bridge);
  ftd::eft::LocalizedPacketSpec spec;
  spec.x0 = 0.5 * length - direction * 10.0;
  spec.y0 = 0.5 * length;
  spec.z0 = 0.5 * length;
  spec.sigma_x = kSigmaX;
  spec.sigma_t = kSigmaT;
  spec.amplitude = amplitude;
  spec.direction = direction;
  spec.carrier_k = ftd::PI / 4.0;
  ftd::eft::seed_localized_transverse_packet(bridge, spec);

  ftd::eft::WaveMorphologyObservation reference;
  double centroid0 = 0.0;
  long double energy0 = 0.0L;
  for (int tick = 0; tick <= kFinalTick; ++tick) {
    if (is_sample_tick(tick)) {
      MorphologySample sample;
      sample.tick = tick;
      sample.observation = ftd::eft::observe_wave_morphology(
          bridge, direction, kCoreHalfWidth);
      if (tick == 0) {
        reference = sample.observation;
        centroid0 = reference.centroid_x;
        energy0 = reference.exact_tick_energy;
      }
      sample.comparison = ftd::eft::compare_wave_profiles(
          reference, sample.observation, direction, kCoreHalfWidth);
      sample.displacement = periodic_delta(
          sample.observation.centroid_x, centroid0, length);
      sample.relative_energy_drift = std::abs(static_cast<double>(
          sample.observation.exact_tick_energy - energy0))
          / std::max(1e-30, std::abs(static_cast<double>(energy0)));
      out.samples.push_back(std::move(sample));
    }
    if (tick < kFinalTick) bridge.tick();
  }

  out.valid = out.samples.size() == kSampleTicks.size();
  for (const auto& sample : out.samples)
    out.valid = out.valid && sample.observation.valid
        && sample.comparison.valid
        && std::isfinite(sample.displacement)
        && std::isfinite(sample.relative_energy_drift);
  const auto& first = out.samples.front();
  const auto& last = out.samples.back();
  out.bound = last.comparison.overlap >= 0.90
      && last.comparison.explained_fraction >= 0.85
      && last.observation.core_fraction
          >= 0.85 * first.observation.core_fraction
      && last.comparison.trailing_excess_fraction < 0.05
      && last.relative_energy_drift <= 1e-10;
  out.trailing_wake = last.comparison.trailing_excess_fraction >= 0.10
      && last.comparison.trailing_excess_fraction
          >= 2.0 * std::max(0.01,
                            last.comparison.leading_excess_fraction)
      && last.observation.core_fraction
          <= first.observation.core_fraction - 0.10;
  const double leading = last.comparison.leading_excess_fraction;
  const double trailing = last.comparison.trailing_excess_fraction;
  out.symmetric_dispersion = leading >= 0.05 && trailing >= 0.05
      && std::max(leading, trailing)
          <= 2.0 * std::max(1e-30, std::min(leading, trailing));
  return out;
}

PacketRun run_exact_scenario(int length) {
  PacketRun out;
  out.arm = "s0_vacuum_photon";
  out.length = length;
  out.direction = +1;
  out.amplitude = 1.0;
  ftd::RenderBridge bridge(length);
  bridge.force_cpu();
  const bool dispatched = ftd::dispatch_scenario(bridge, "s0-vacuum-photon");
  bridge.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
  bridge.toggles.strict_validation = true;

  ftd::eft::WaveMorphologyObservation reference;
  double centroid0 = 0.0;
  long double energy0 = 0.0L;
  for (int tick = 0; tick <= kFinalTick; ++tick) {
    if (is_sample_tick(tick)) {
      MorphologySample sample;
      sample.tick = tick;
      sample.observation = ftd::eft::observe_wave_morphology(
          bridge, +1, kCoreHalfWidth);
      if (tick == 0) {
        reference = sample.observation;
        centroid0 = reference.centroid_x;
        energy0 = reference.exact_tick_energy;
      }
      sample.comparison = ftd::eft::compare_wave_profiles(
          reference, sample.observation, +1, kCoreHalfWidth);
      sample.displacement = periodic_delta(
          sample.observation.centroid_x, centroid0, length);
      sample.relative_energy_drift = std::abs(static_cast<double>(
          sample.observation.exact_tick_energy - energy0))
          / std::max(1e-30, std::abs(static_cast<double>(energy0)));
      out.samples.push_back(std::move(sample));
    }
    if (tick < kFinalTick) bridge.tick();
  }
  out.valid = dispatched && out.samples.size() == kSampleTicks.size();
  for (const auto& sample : out.samples)
    out.valid = out.valid && sample.observation.valid
        && sample.comparison.valid
        && std::isfinite(sample.relative_energy_drift);
  const auto& first = out.samples.front();
  const auto& last = out.samples.back();
  out.bound = last.comparison.overlap >= 0.90
      && last.comparison.explained_fraction >= 0.85
      && last.observation.core_fraction
          >= 0.85 * first.observation.core_fraction
      && last.comparison.trailing_excess_fraction < 0.05
      && last.relative_energy_drift <= 1e-10;
  out.trailing_wake = last.comparison.trailing_excess_fraction >= 0.10
      && last.comparison.trailing_excess_fraction
          >= 2.0 * std::max(0.01,
                            last.comparison.leading_excess_fraction)
      && last.observation.core_fraction
          <= first.observation.core_fraction - 0.10;
  const double leading = last.comparison.leading_excess_fraction;
  const double trailing = last.comparison.trailing_excess_fraction;
  out.symmetric_dispersion = leading >= 0.05 && trailing >= 0.05
      && std::max(leading, trailing)
          <= 2.0 * std::max(1e-30, std::min(leading, trailing));
  return out;
}

struct ProbeSample {
  int tick = 0;
  double packet_centroid = 0.0;
  double directed_probe_distance = 0.0;
  double force_x = 0.0;
  double force_longitudinal = 0.0;
};

struct ProbeRun {
  int length = 0;
  int direction = 1;
  double amplitude = 0.0;
  int polarity = 1;
  std::vector<ProbeSample> samples;
  bool valid = false;
  double max_leading_force = 0.0;
  double integrated_longitudinal_force = 0.0;
};

ProbeRun run_probe(int length, int direction, double amplitude, int polarity) {
  ProbeRun out;
  out.length = length;
  out.direction = direction;
  out.amplitude = amplitude;
  out.polarity = polarity;
  ftd::RenderBridge bridge(length);
  configure_wave_only(bridge);
  bridge.toggles.forces = true;
  bridge.toggles.emergent_forces = true;
  ftd::eft::LocalizedPacketSpec spec;
  const int probe_x = length / 2;
  // Revision 2: the discrete-curl packet vanishes on its propagation axis.
  // Place the probe on the preregistered transverse lobe already used by
  // FTD-0457, rather than on the revision-1 nodal-line control.
  const int probe_y = length / 2 - static_cast<int>(kSigmaT);
  const int probe_z = length / 2;
  spec.x0 = static_cast<double>(probe_x - direction * 10);
  spec.y0 = static_cast<double>(probe_y);
  spec.z0 = static_cast<double>(probe_z);
  spec.sigma_x = kSigmaX;
  spec.sigma_t = kSigmaT;
  spec.amplitude = amplitude;
  spec.direction = direction;
  spec.carrier_k = ftd::PI / 4.0;
  ftd::eft::seed_localized_transverse_packet(bridge, spec);
  const int probe = bridge.lattice().index(probe_x, probe_y, probe_z);
  const ftd::Vec3 local_flux = bridge.voxels()[static_cast<std::size_t>(probe)].flux;
  bridge.inject_particle(probe_x, probe_y, probe_z,
                         static_cast<std::int8_t>(polarity), local_flux);
  bridge.voxels()[static_cast<std::size_t>(probe)].locked = true;

  for (int tick = 1; tick <= kFinalTick; ++tick) {
    bridge.tick();
    const auto observation = ftd::eft::observe_wave_morphology(
        bridge, direction, kCoreHalfWidth);
    const auto force = bridge.force_diag_at(probe).f_coulomb;
    ProbeSample sample;
    sample.tick = tick;
    sample.packet_centroid = observation.centroid_x;
    sample.directed_probe_distance = direction * periodic_delta(
        static_cast<double>(probe_x), observation.centroid_x, length);
    sample.force_x = force.x;
    sample.force_longitudinal = direction * force.x;
    if (sample.directed_probe_distance > kCoreHalfWidth)
      out.max_leading_force = std::max(
          out.max_leading_force, std::abs(sample.force_longitudinal));
    out.integrated_longitudinal_force += sample.force_longitudinal;
    out.samples.push_back(sample);
  }
  out.valid = out.samples.size() == static_cast<std::size_t>(kFinalTick);
  for (const auto& sample : out.samples)
    out.valid = out.valid && std::isfinite(sample.packet_centroid)
        && std::isfinite(sample.directed_probe_distance)
        && std::isfinite(sample.force_x);
  return out;
}

double no_field_control() {
  double maximum = 0.0;
  for (int polarity : {-1, +1}) {
    ftd::RenderBridge bridge(49);
    bridge.force_cpu();
    bridge.toggles.disable_all();
    bridge.toggles.forces = true;
    bridge.toggles.emergent_forces = true;
    bridge.toggles.strict_validation = true;
    const int center = 24;
    bridge.inject_particle(center, center, center,
                           static_cast<std::int8_t>(polarity), {});
    bridge.voxel_at(center, center, center).locked = true;
    for (int tick = 0; tick < 8; ++tick) {
      bridge.tick();
      maximum = std::max(maximum,
          bridge.force_diag_at(center, center, center).f_coulomb.mag());
    }
  }
  return maximum;
}

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

std::uint64_t wave_hash(const ftd::RenderBridge& bridge) {
  std::uint64_t hash = 1469598103934665603ull;
  hash_value(hash, bridge.current_tick());
  for (const auto& voxel : bridge.voxels()) {
    for (const auto& value : {voxel.flux, voxel.wave_vel}) {
      hash_value(hash, value.x);
      hash_value(hash, value.y);
      hash_value(hash, value.z);
    }
  }
  return hash;
}

bool observer_neutrality() {
  ftd::RenderBridge control(49);
  ftd::RenderBridge observed(49);
  configure_wave_only(control);
  configure_wave_only(observed);
  ftd::eft::LocalizedPacketSpec spec;
  spec.x0 = 14.5;
  spec.y0 = 24.5;
  spec.z0 = 24.5;
  ftd::eft::seed_localized_transverse_packet(control, spec);
  ftd::eft::seed_localized_transverse_packet(observed, spec);
  for (int tick = 0; tick < 16; ++tick) {
    (void)ftd::eft::observe_wave_morphology(
        observed, +1, kCoreHalfWidth);
    control.tick();
    observed.tick();
  }
  return wave_hash(control) == wave_hash(observed)
      && control.rng_state_hash() == observed.rng_state_hash();
}

}  // namespace

int main() {
  const fs::path output_dir = fs::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0475";
  fs::create_directories(output_dir);

  std::vector<PacketRun> packets;
  for (int length : kLengths)
    for (int direction : kDirections)
      for (double amplitude : kAmplitudes)
        packets.push_back(run_localized_packet(length, direction, amplitude));
  for (int length : kLengths)
    packets.push_back(run_exact_scenario(length));

  std::vector<ProbeRun> probes;
  for (int length : kLengths)
    for (int direction : kDirections)
      for (double amplitude : kAmplitudes)
        for (int polarity : {-1, +1})
          probes.push_back(run_probe(length, direction, amplitude, polarity));

  std::ofstream morphology_csv(output_dir / "morphology_v2.csv");
  morphology_csv << std::setprecision(17)
      << "arm,L,direction,amplitude,tick,activity,centroid_x,displacement,"
         "width_x,core_fraction,leading_fraction,trailing_fraction,"
         "normalized_divergence,exact_tick_energy,relative_energy_drift,"
         "best_shift,overlap,explained_fraction,leading_excess_fraction,"
         "trailing_excess_fraction,bound_clause,trailing_wake_clause,"
         "symmetric_dispersion_clause,valid\n";
  int morphology_rows = 0;
  for (const auto& run : packets)
    for (const auto& sample : run.samples) {
      const auto& observation = sample.observation;
      const auto& comparison = sample.comparison;
      morphology_csv << run.arm << ',' << run.length << ',' << run.direction
          << ',' << run.amplitude << ',' << sample.tick << ','
          << observation.activity << ',' << observation.centroid_x << ','
          << sample.displacement << ',' << observation.width_x << ','
          << observation.core_fraction << ',' << observation.leading_fraction
          << ',' << observation.trailing_fraction << ','
          << observation.normalized_divergence << ','
          << static_cast<double>(observation.exact_tick_energy) << ','
          << sample.relative_energy_drift << ',' << comparison.best_shift
          << ',' << comparison.overlap << ',' << comparison.explained_fraction
          << ',' << comparison.leading_excess_fraction << ','
          << comparison.trailing_excess_fraction << ',' << run.bound << ','
          << run.trailing_wake << ',' << run.symmetric_dispersion << ','
          << run.valid << '\n';
      ++morphology_rows;
    }

  std::ofstream probe_csv(output_dir / "probe_response_v2.csv");
  probe_csv << std::setprecision(17)
      << "L,direction,amplitude,polarity,tick,packet_centroid,"
         "directed_probe_distance,force_x,force_longitudinal,"
         "max_leading_force,integrated_longitudinal_force,valid\n";
  int probe_rows = 0;
  for (const auto& run : probes)
    for (const auto& sample : run.samples) {
      probe_csv << run.length << ',' << run.direction << ',' << run.amplitude
          << ',' << run.polarity << ',' << sample.tick << ','
          << sample.packet_centroid << ',' << sample.directed_probe_distance
          << ',' << sample.force_x << ',' << sample.force_longitudinal << ','
          << run.max_leading_force << ','
          << run.integrated_longitudinal_force << ',' << run.valid << '\n';
      ++probe_rows;
    }

  bool morphology_valid = packets.size() == 10 && morphology_rows == 90;
  int localized_bound = 0;
  int localized_wake = 0;
  int localized_dispersion = 0;
  int scenario_bound = 0;
  int scenario_wake = 0;
  double worst_energy_drift = 0.0;
  for (const auto& run : packets) {
    morphology_valid = morphology_valid && run.valid;
    if (run.arm == "localized_finite") {
      localized_bound += run.bound ? 1 : 0;
      localized_wake += run.trailing_wake ? 1 : 0;
      localized_dispersion += run.symmetric_dispersion ? 1 : 0;
    } else {
      scenario_bound += run.bound ? 1 : 0;
      scenario_wake += run.trailing_wake ? 1 : 0;
    }
    for (const auto& sample : run.samples)
      worst_energy_drift = std::max(
          worst_energy_drift, sample.relative_energy_drift);
  }

  bool probes_valid = probes.size() == 16 && probe_rows == 512;
  double minimum_leading_force = std::numeric_limits<double>::infinity();
  double worst_polarity_odd_residual = 0.0;
  for (const auto& run : probes) {
    probes_valid = probes_valid && run.valid;
    minimum_leading_force = std::min(
        minimum_leading_force, run.max_leading_force);
  }
  for (int length : kLengths)
    for (int direction : kDirections)
      for (double amplitude : kAmplitudes) {
        const ProbeRun* minus = nullptr;
        const ProbeRun* plus = nullptr;
        for (const auto& run : probes) {
          if (run.length != length || run.direction != direction
              || run.amplitude != amplitude) continue;
          if (run.polarity < 0) minus = &run;
          else plus = &run;
        }
        if (!minus || !plus || minus->samples.size() != plus->samples.size()) {
          probes_valid = false;
          continue;
        }
        for (std::size_t i = 0; i < plus->samples.size(); ++i) {
          const double scale = std::max({1e-30,
              std::abs(plus->samples[i].force_x),
              std::abs(minus->samples[i].force_x)});
          worst_polarity_odd_residual = std::max(
              worst_polarity_odd_residual,
              std::abs(plus->samples[i].force_x
                       + minus->samples[i].force_x) / scale);
        }
      }

  const double null_force = no_field_control();
  const bool leading_response = minimum_leading_force > 1e-10
      && worst_polarity_odd_residual <= 1e-12 && null_force <= 1e-15;
  const bool neutral = observer_neutrality();
  const bool structural_valid = morphology_valid && probes_valid && neutral;

  std::string morphology_verdict = "MIXED_OR_UNRESOLVED_MORPHOLOGY";
  if (localized_bound == 8 && localized_wake == 0)
    morphology_verdict = "CO_MOVING_BOUND_PACKET_NO_DETACHED_WAKE";
  else if (localized_wake >= 6)
    morphology_verdict = "TRAILING_WAKE_DOMINATES";
  else if (localized_dispersion >= 6 && localized_wake == 0)
    morphology_verdict = "SYMMETRIC_DISPERSION_NOT_WAKE";

  const std::string response_verdict = leading_response
      ? "ONE_WAY_POLARITY_ODD_LEADING_RESPONSE"
      : "NO_QUALIFIED_LEADING_RESPONSE";

  std::ofstream verdict(output_dir / "verdict_v2.txt");
  verdict << std::setprecision(17)
      << "morphology_verdict=" << morphology_verdict << '\n'
      << "response_verdict=" << response_verdict << '\n'
      << "structural_valid=" << structural_valid << '\n'
      << "observer_neutral=" << neutral << '\n'
      << "morphology_rows=" << morphology_rows << '\n'
      << "probe_rows=" << probe_rows << '\n'
      << "localized_bound_runs=" << localized_bound << '\n'
      << "localized_wake_runs=" << localized_wake << '\n'
      << "localized_symmetric_dispersion_runs=" << localized_dispersion << '\n'
      << "scenario_bound_runs=" << scenario_bound << '\n'
      << "scenario_wake_runs=" << scenario_wake << '\n'
      << "worst_relative_exact_energy_drift=" << worst_energy_drift << '\n'
      << "minimum_leading_force=" << minimum_leading_force << '\n'
      << "worst_polarity_odd_residual=" << worst_polarity_odd_residual << '\n'
      << "no_field_control_force=" << null_force << '\n';

  std::cout << "FTD-0475 morphology=" << morphology_verdict
            << " response=" << response_verdict << '\n'
            << "  localized bound/wake/dispersion=" << localized_bound << '/'
            << localized_wake << '/' << localized_dispersion
            << " scenario bound/wake=" << scenario_bound << '/'
            << scenario_wake << '\n'
            << "  energy drift=" << worst_energy_drift
            << " leading force min=" << minimum_leading_force
            << " polarity residual=" << worst_polarity_odd_residual
            << " null force=" << null_force << '\n'
            << "  structural_valid=" << structural_valid
            << " observer_neutral=" << neutral << std::endl;
  return structural_valid ? 0 : 1;
}
