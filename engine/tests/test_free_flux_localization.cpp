/** FTD-0557: free-flux localization obstruction. */

#include "ftd/constants.h"
#include "ftd/eft/free_flux_localization.h"
#include "ftd/eft/integer_bloch_transport.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr double identity_gate = 1e-12;
constexpr double engine_gate = 1e-10;
constexpr double variance_gate = 1e-8;
constexpr double broadening_gate = 1e-4;
constexpr double pi = 3.1415926535897932384626433832795;
constexpr int packet_L = 65;
constexpr int first_mode = 1;
constexpr int last_mode = 24;
constexpr double center_mode = 8.0;
constexpr double width_modes = 3.0;
constexpr int packet_center = 32;
constexpr int registered_ticks = 16;

int failures = 0;
int cpu_backend_checks = 0;

void check(const std::string& label, bool pass) {
  if (pass) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

int wrap(int value, int L) {
  const int reduced = value%L;
  return reduced < 0 ? reduced+L : reduced;
}

void configure_isolated_wave(ftd::RenderBridge& bridge) {
  bridge.force_cpu();
  ++cpu_backend_checks;
  check("forced CPU backend",
      bridge.backend_kind() == ftd::Backend::Kind::Cpu);
  bridge.toggles.disable_all();
  bridge.toggles.wave_propagation = true;
  bridge.toggles.bcc_stencil = ftd::BccStencilMode::FULL;
  bridge.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
  std::string validation;
  check("isolated toggle combination", bridge.toggles.validate(&validation));
  check("no field source or matter transaction",
      !bridge.toggles.coupling
      && !bridge.toggles.damping
      && !bridge.toggles.genesis
      && !bridge.toggles.evaporation
      && !bridge.toggles.pair_production
      && !bridge.toggles.gauss_projection
      && !bridge.toggles.forces
      && !bridge.toggles.movement
      && !bridge.toggles.de_broglie_clock
      && !bridge.toggles.dual_substrate);
}

std::complex<double> flux_complex(const ftd::Voxel& voxel) {
  return {voxel.flux.x, voxel.flux.y};
}

std::complex<double> wave_complex(const ftd::Voxel& voxel) {
  return {voxel.wave_vel.x, voxel.wave_vel.y};
}

void set_complex_state(ftd::Voxel& voxel,
                       const std::complex<double>& flux,
                       const std::complex<double>& wave) {
  voxel.flux = {flux.real(), flux.imag(), 0.0};
  voxel.wave_vel = {wave.real(), wave.imag(), 0.0};
}

struct BranchPacket {
  ftd::Coord direction{};
  std::vector<std::complex<double>> amplitude;
  std::vector<double> phase;
  std::vector<double> kick;
};

BranchPacket make_packet(const ftd::Coord& direction) {
  BranchPacket packet;
  packet.direction = direction;
  packet.amplitude.assign(packet_L, {});
  packet.phase.assign(packet_L, 0.0);
  packet.kick.assign(packet_L, 0.0);
  long double norm = 0.0L;
  for (int mode = first_mode; mode <= last_mode; ++mode) {
    const double delta = (mode-center_mode)/width_modes;
    packet.amplitude[mode] = std::exp(-0.5*delta*delta);
    norm += std::norm(packet.amplitude[mode]);
  }
  const double inverse_norm = 1.0/std::sqrt(static_cast<double>(norm));
  for (auto& value : packet.amplitude) value *= inverse_norm;
  const double c2 = ftd::C_WAVE*ftd::C_WAVE;
  for (int mode = first_mode; mode <= last_mode; ++mode) {
    const double q = 2.0*pi*mode/packet_L;
    const std::array<double, 3> momentum{{
        q*direction.x, q*direction.y, q*direction.z}};
    const double symbol = ftd::eft::full_stencil_symbol(momentum);
    packet.kick[mode] = c2*symbol;
    packet.phase[mode] = ftd::eft::native_bloch_phase(symbol, c2);
  }
  return packet;
}

std::complex<double> packet_flux(
    const BranchPacket& packet, int coordinate, int tick) {
  std::complex<long double> sum{};
  for (int mode = first_mode; mode <= last_mode; ++mode) {
    const long double angle = 2.0L*pi*mode
        *(coordinate-packet_center)/packet_L
        -static_cast<long double>(packet.phase[mode])*tick;
    sum += std::complex<long double>{packet.amplitude[mode].real(),
                                    packet.amplitude[mode].imag()}
        *std::complex<long double>{std::cos(angle), std::sin(angle)};
  }
  sum /= static_cast<long double>(packet_L);
  return {static_cast<double>(sum.real()), static_cast<double>(sum.imag())};
}

std::complex<double> packet_wave(
    const BranchPacket& packet, int coordinate, int tick) {
  std::complex<long double> sum{};
  for (int mode = first_mode; mode <= last_mode; ++mode) {
    const std::complex<double> lambda =
        std::polar(1.0, -packet.phase[mode]);
    const std::complex<double> factor =
        lambda-(1.0-packet.kick[mode]);
    const long double angle = 2.0L*pi*mode
        *(coordinate-packet_center)/packet_L
        -static_cast<long double>(packet.phase[mode])*tick;
    const auto weighted = factor*packet.amplitude[mode];
    sum += std::complex<long double>{weighted.real(), weighted.imag()}
        *std::complex<long double>{std::cos(angle), std::sin(angle)};
  }
  sum /= static_cast<long double>(packet_L);
  return {static_cast<double>(sum.real()), static_cast<double>(sum.imag())};
}

struct DensityMoments {
  double norm = 0.0;
  double mean = 0.0;
  double variance = 0.0;
};

DensityMoments density_moments(
    const std::vector<std::complex<double>>& profile) {
  DensityMoments result;
  long double norm = 0.0L;
  long double first = 0.0L;
  long double second = 0.0L;
  for (int coordinate = 0; coordinate < static_cast<int>(profile.size());
       ++coordinate) {
    const long double density = std::norm(profile[coordinate]);
    norm += density;
    first += coordinate*density;
    second += coordinate*coordinate*density;
  }
  result.norm = static_cast<double>(norm);
  result.mean = static_cast<double>(first/norm);
  result.variance = std::max(0.0,
      static_cast<double>(second/norm)-result.mean*result.mean);
  return result;
}

std::vector<std::complex<double>> expected_profile(
    const BranchPacket& packet, int tick, bool wave) {
  std::vector<std::complex<double>> profile(packet_L);
  for (int coordinate = 0; coordinate < packet_L; ++coordinate) {
    profile[coordinate] = wave
        ? packet_wave(packet, coordinate, tick)
        : packet_flux(packet, coordinate, tick);
  }
  return profile;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  const std::vector<ftd::Coord> directions{
      {1,0,0}, {1,1,0}, {1,1,1}};
  const double c2 = ftd::C_WAVE*ftd::C_WAVE;
  const auto analysis = ftd::eft::analyze_free_flux_localization(
      packet_L, first_mode, last_mode, center_mode, width_modes,
      directions, c2);
  check("analytic observer valid", analysis.valid);
  check("real-analytic nonconstant transfer trace",
      analysis.finite_range_symbol_is_real_analytic
      && analysis.transfer_trace_is_nonconstant);
  check("no flat native band", analysis.native_band_is_not_flat);
  check("no l2 point spectrum", analysis.no_nonzero_l2_point_spectrum);
  check("no finite-time rigid l2 translate",
      analysis.no_nonzero_finite_time_rigid_l2_translate);
  check("exact branch second-moment identity",
      analysis.exact_branch_second_moment_identity);
  check("unchirped packet broadening theorem",
      analysis.unchirped_localized_packet_must_broaden);
  check("registered analytic packet arms", analysis.packets.size() == 3);
  check("positive velocity variance in every arm",
      analysis.minimum_velocity_variance > variance_gate);

  int arm_tick_replays = 0;
  long long manifested_sites_observed = 0;
  double worst_engine_replay_residual = 0.0;
  double worst_density_moment_residual = 0.0;
  double worst_spectral_norm_residual = 0.0;
  double maximum_variance_increase = 0.0;
  double minimum_seam_margin_in_rms =
      std::numeric_limits<double>::infinity();

  for (const auto& direction : directions) {
    const auto packet = make_packet(direction);
    ftd::RenderBridge bridge(packet_L);
    configure_isolated_wave(bridge);
    const auto initial_flux = expected_profile(packet, 0, false);
    const auto initial_wave = expected_profile(packet, 0, true);
    const auto initial_moments = density_moments(initial_flux);
    const double initial_rms = std::sqrt(initial_moments.variance);
    for (int x = 0; x < packet_L; ++x)
      for (int y = 0; y < packet_L; ++y)
        for (int z = 0; z < packet_L; ++z) {
          const int coordinate = wrap(
              direction.x*x+direction.y*y+direction.z*z, packet_L);
          set_complex_state(bridge.voxels()[bridge.lattice().index(x,y,z)],
                            initial_flux[coordinate],
                            initial_wave[coordinate]);
        }

    for (int tick = 1; tick <= registered_ticks; ++tick) {
      bridge.tick();
      ++arm_tick_replays;
      const auto predicted_flux = expected_profile(packet, tick, false);
      const auto predicted_wave = expected_profile(packet, tick, true);
      std::vector<std::complex<double>> direct_flux(packet_L);
      for (int coordinate = 0; coordinate < packet_L; ++coordinate) {
        direct_flux[coordinate] = flux_complex(
            bridge.voxel_at(coordinate, 0, 0));
      }
      for (int x = 0; x < packet_L; ++x)
        for (int y = 0; y < packet_L; ++y)
          for (int z = 0; z < packet_L; ++z) {
            const int coordinate = wrap(
                direction.x*x+direction.y*y+direction.z*z, packet_L);
            const auto& voxel = bridge.voxel_at(x,y,z);
            worst_engine_replay_residual = std::max({
                worst_engine_replay_residual,
                std::abs(flux_complex(voxel)-predicted_flux[coordinate]),
                std::abs(wave_complex(voxel)-predicted_wave[coordinate])});
            if (voxel.state != 0) ++manifested_sites_observed;
          }
      const auto direct_moments = density_moments(direct_flux);
      const auto predicted_moments = density_moments(predicted_flux);
      worst_density_moment_residual = std::max({
          worst_density_moment_residual,
          std::abs(direct_moments.norm-predicted_moments.norm),
          std::abs(direct_moments.mean-predicted_moments.mean),
          std::abs(direct_moments.variance-predicted_moments.variance)});
      worst_spectral_norm_residual = std::max(
          worst_spectral_norm_residual,
          std::abs(predicted_moments.norm-initial_moments.norm));
      maximum_variance_increase = std::max(
          maximum_variance_increase,
          direct_moments.variance-initial_moments.variance);
      const double seam_distance = std::min(
          direct_moments.mean,
          static_cast<double>(packet_L-1)-direct_moments.mean);
      minimum_seam_margin_in_rms = std::min(
          minimum_seam_margin_in_rms, seam_distance/initial_rms);
    }
  }

  check("locked CPU backend cardinality", cpu_backend_checks == 3);
  check("locked arm-tick replay cardinality", arm_tick_replays == 48);
  check("production branch replay",
      worst_engine_replay_residual <= engine_gate);
  check("direct versus spectral density moments",
      worst_density_moment_residual <= engine_gate);
  check("spectral density norm",
      worst_spectral_norm_residual <= identity_gate);
  check("no manifested primitive sites", manifested_sites_observed == 0);
  check("locked packet remains away from periodic seam",
      minimum_seam_margin_in_rms > 4.0);
  check("direct packet broadening witness",
      maximum_variance_increase > broadening_gate);

  const bool passed = failures == 0;
  std::cout << "registered_packet_arms=" << analysis.packets.size() << '\n'
            << "registered_arm_tick_replays=" << arm_tick_replays << '\n'
            << "cpu_backend_checks=" << cpu_backend_checks << '\n'
            << "manifested_sites_observed=" << manifested_sites_observed << '\n'
            << "minimum_group_velocity_variance="
            << analysis.minimum_velocity_variance << '\n'
            << "worst_engine_replay_residual="
            << worst_engine_replay_residual << '\n'
            << "worst_density_moment_residual="
            << worst_density_moment_residual << '\n'
            << "worst_spectral_norm_residual="
            << worst_spectral_norm_residual << '\n'
            << "maximum_variance_increase="
            << maximum_variance_increase << '\n'
            << "minimum_seam_margin_in_rms="
            << minimum_seam_margin_in_rms << '\n'
            << "verdict="
            << (passed
                ? "FREE_FLUX_TRANSPORT_HAS_NO_LOCALIZED_RIGID_CARRIER"
                : "FREE_FLUX_LOCALIZATION_OBSTRUCTION_FAILED") << '\n'
            << "free_flux_localization failures=" << failures << '\n';
  return passed ? 0 : 1;
}
