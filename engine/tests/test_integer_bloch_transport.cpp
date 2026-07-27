/** FTD-0556: integer translation and native Bloch transport. */

#include "ftd/constants.h"
#include "ftd/eft/integer_bloch_transport.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr double identity_gate = 1e-12;
constexpr double engine_gate = 1e-10;
constexpr double support_gate = 1e-14;
constexpr double pi = 3.1415926535897932384626433832795;
int failures = 0;
int cpu_backend_checks = 0;

void check(const std::string& label, bool pass) {
  if (pass) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
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
}

int wrap(int value, int L) {
  const int reduced = value%L;
  return reduced < 0 ? reduced+L : reduced;
}

double periodic_distance(int lhs, int rhs, int L) {
  const int direct = std::abs(lhs-rhs);
  return static_cast<double>(std::min(direct, L-direct));
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

double max_voxel_state_difference(const ftd::Voxel& lhs,
                                  const ftd::Voxel& rhs) {
  return std::max((lhs.flux-rhs.flux).mag(),
                  (lhs.wave_vel-rhs.wave_vel).mag());
}

struct PacketSpectrum {
  int L = 0;
  std::vector<std::complex<double>> amplitude;
  std::vector<double> phase;
  std::vector<double> kick;
  double norm = 0.0;
};

PacketSpectrum make_packet_spectrum(int L) {
  PacketSpectrum packet;
  packet.L = L;
  packet.amplitude.assign(L, {});
  packet.phase.assign(L, 0.0);
  packet.kick.assign(L, 0.0);
  constexpr double center = 5.0;
  constexpr double width = 1.5;
  for (int mode = 1; mode <= 16; ++mode) {
    const double delta = (mode-center)/width;
    packet.amplitude[mode] = std::exp(-0.5*delta*delta);
  }
  for (int mode = 0; mode < L; ++mode) {
    const double k = 2.0*pi*mode/L;
    const std::array<double,3> momentum{{k,0.0,0.0}};
    const double symbol = ftd::eft::full_stencil_symbol(momentum);
    packet.kick[mode] = ftd::C_WAVE*ftd::C_WAVE*symbol;
    packet.phase[mode] = ftd::eft::native_bloch_phase(
        symbol, ftd::C_WAVE*ftd::C_WAVE);
    packet.norm += std::norm(packet.amplitude[mode]);
  }
  const double inverse_norm = 1.0/std::sqrt(packet.norm);
  for (auto& value : packet.amplitude) value *= inverse_norm;
  packet.norm = 1.0;
  return packet;
}

std::complex<double> packet_flux(const PacketSpectrum& packet,
                                 int x, int tick) {
  std::complex<long double> sum{};
  for (int mode = 0; mode < packet.L; ++mode) {
    if (packet.amplitude[mode] == std::complex<double>{}) continue;
    const long double angle = 2.0L*pi*mode*x/packet.L
        -static_cast<long double>(packet.phase[mode])*tick;
    sum += std::complex<long double>{packet.amplitude[mode].real(),
                                    packet.amplitude[mode].imag()}
        *std::complex<long double>{std::cos(angle), std::sin(angle)};
  }
  sum /= static_cast<long double>(packet.L);
  return {static_cast<double>(sum.real()), static_cast<double>(sum.imag())};
}

std::complex<double> packet_wave(const PacketSpectrum& packet,
                                 int x, int tick) {
  std::complex<long double> sum{};
  for (int mode = 0; mode < packet.L; ++mode) {
    if (packet.amplitude[mode] == std::complex<double>{}) continue;
    const std::complex<double> lambda =
        std::polar(1.0, -packet.phase[mode]);
    const std::complex<double> factor =
        lambda-(1.0-packet.kick[mode]);
    const long double angle = 2.0L*pi*mode*x/packet.L
        -static_cast<long double>(packet.phase[mode])*tick;
    sum += std::complex<long double>{
          (factor*packet.amplitude[mode]).real(),
          (factor*packet.amplitude[mode]).imag()}
        *std::complex<long double>{std::cos(angle), std::sin(angle)};
  }
  sum /= static_cast<long double>(packet.L);
  return {static_cast<double>(sum.real()), static_cast<double>(sum.imag())};
}

std::complex<double> normalized_centroid_from_row(
    const ftd::RenderBridge& bridge) {
  const int L = bridge.lattice().size();
  std::complex<long double> moment{};
  long double norm = 0.0L;
  for (int x = 0; x < L; ++x) {
    const auto value = flux_complex(bridge.voxels()[
        bridge.lattice().index(x,0,0)]);
    const long double density = std::norm(value);
    const long double angle = 2.0L*pi*x/L;
    moment += density*std::complex<long double>{
        std::cos(angle), std::sin(angle)};
    norm += density;
  }
  moment /= norm;
  return {static_cast<double>(moment.real()),
          static_cast<double>(moment.imag())};
}

std::complex<double> normalized_centroid_from_spectrum(
    const PacketSpectrum& packet, int tick) {
  std::complex<long double> moment{};
  long double norm = 0.0L;
  for (int mode = 0; mode < packet.L; ++mode) {
    const int next = (mode+1)%packet.L;
    const auto left = packet.amplitude[mode]
        *std::polar(1.0, -packet.phase[mode]*tick);
    const auto right = packet.amplitude[next]
        *std::polar(1.0, -packet.phase[next]*tick);
    moment += std::complex<long double>{
        (left*std::conj(right)).real(),
        (left*std::conj(right)).imag()};
    norm += std::norm(left);
  }
  // Z uses exp(+i*2pi*x/L), so the surviving pair is A_m*conj(A_{m+1}).
  moment /= norm;
  return {static_cast<double>(moment.real()),
          static_cast<double>(moment.imag())};
}

double centroid_displacement(const std::complex<double>& current,
                             const std::complex<double>& initial,
                             int L) {
  return L*std::arg(current*std::conj(initial))/(2.0*pi);
}

double spectral_norm_from_row(const ftd::RenderBridge& bridge) {
  const int L = bridge.lattice().size();
  long double norm = 0.0L;
  for (int mode = 0; mode < L; ++mode) {
    std::complex<long double> amplitude{};
    for (int x = 0; x < L; ++x) {
      const auto value = flux_complex(bridge.voxels()[
          bridge.lattice().index(x,0,0)]);
      const long double angle = -2.0L*pi*mode*x/L;
      amplitude += std::complex<long double>{value.real(), value.imag()}
          *std::complex<long double>{std::cos(angle), std::sin(angle)};
    }
    norm += std::norm(amplitude);
  }
  return static_cast<double>(norm);
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  const std::vector<int> mode_numbers{1,2,3};
  const std::vector<ftd::Coord> directions{
      {1,0,0}, {1,1,0}, {1,1,1}};
  const double c2 = ftd::C_WAVE*ftd::C_WAVE;
  const auto analysis = ftd::eft::analyze_integer_bloch_transport(
      17, mode_numbers, directions, c2);
  check("observer valid", analysis.valid);
  check("scalar finite-Laurent lemma",
      analysis.scalar_finite_laurent_unitary_is_monomial
      && analysis.scalar_dispersive_band_requires_type_escape);
  check("native pair symplectic", analysis.native_pair_is_symplectic);
  check("locked mode arms", analysis.modes.size() == 9);
  check("mode algebra", analysis.maximum_identity_residual <= identity_gate);
  check("mode causal speed", analysis.maximum_group_speed <= std::sqrt(3.0));

  double worst_cubic_orbit_residual = 0.0;
  for (int mode_number : mode_numbers) {
    for (int nonzero_components : {1,2,3}) {
      double reference_symbol = NAN;
      double reference_phase = NAN;
      for (int x : {-1,0,1}) for (int y : {-1,0,1})
        for (int z : {-1,0,1}) {
          if ((x != 0)+(y != 0)+(z != 0) != nonzero_components) continue;
          const double fundamental = 2.0*pi*mode_number/17.0;
          const std::array<double,3> momentum{{
              fundamental*x, fundamental*y, fundamental*z}};
          const double symbol = ftd::eft::full_stencil_symbol(momentum);
          const double phase = ftd::eft::native_bloch_phase(symbol, c2);
          if (!std::isfinite(reference_symbol)) {
            reference_symbol = symbol;
            reference_phase = phase;
          }
          worst_cubic_orbit_residual = std::max({
              worst_cubic_orbit_residual,
              std::abs(symbol-reference_symbol),
              std::abs(phase-reference_phase)});
        }
    }
  }
  check("cubic mode orbits", worst_cubic_orbit_residual <= identity_gate);

  int registered_mode_tick_replays = 0;
  double worst_engine_mode_residual = 0.0;
  for (const auto& mode : analysis.modes) {
    ftd::RenderBridge bridge(mode.L);
    configure_isolated_wave(bridge);
    const std::complex<double> lambda = std::polar(1.0, -mode.phase);
    const std::complex<double> factor = lambda-(1.0-mode.kick);
    constexpr double amplitude = 1e-3;
    for (int x = 0; x < mode.L; ++x)
      for (int y = 0; y < mode.L; ++y)
        for (int z = 0; z < mode.L; ++z) {
          const double angle = mode.momentum[0]*x
              +mode.momentum[1]*y+mode.momentum[2]*z;
          const auto flux = amplitude*std::polar(1.0, angle);
          set_complex_state(
              bridge.voxels()[bridge.lattice().index(x,y,z)],
              flux, factor*flux);
        }
    for (int tick = 1; tick <= 64; ++tick) {
      bridge.tick();
      ++registered_mode_tick_replays;
      for (int x = 0; x < mode.L; ++x)
        for (int y = 0; y < mode.L; ++y)
          for (int z = 0; z < mode.L; ++z) {
            const double angle = mode.momentum[0]*x
                +mode.momentum[1]*y+mode.momentum[2]*z
                -mode.phase*tick;
            const auto expected_flux = amplitude*std::polar(1.0, angle);
            const auto expected_wave = factor*expected_flux;
            const auto& voxel = bridge.voxel_at(x,y,z);
            worst_engine_mode_residual = std::max({
                worst_engine_mode_residual,
                std::abs(flux_complex(voxel)-expected_flux),
                std::abs(wave_complex(voxel)-expected_wave)});
          }
    }
  }
  check("locked mode-tick replay cardinality",
      registered_mode_tick_replays == 576);
  check("production mode replay", worst_engine_mode_residual <= engine_gate);

  const int support_L = 33;
  ftd::RenderBridge support_bridge(support_L);
  configure_isolated_wave(support_bridge);
  const int center = support_L/2;
  support_bridge.voxels()[support_bridge.lattice().index(
      center,center,center)].flux.x = 1.0;
  int registered_support_arms = 0;
  double worst_outside_support = 0.0;
  for (int tick = 1; tick <= 6; ++tick) {
    support_bridge.tick();
    ++registered_support_arms;
    for (int x = 0; x < support_L; ++x)
      for (int y = 0; y < support_L; ++y)
        for (int z = 0; z < support_L; ++z) {
          const double radius = std::max({
              periodic_distance(x,center,support_L),
              periodic_distance(y,center,support_L),
              periodic_distance(z,center,support_L)});
          if (radius <= tick) continue;
          const auto& voxel = support_bridge.voxel_at(x,y,z);
          worst_outside_support = std::max({
              worst_outside_support,
              voxel.flux.mag(), voxel.wave_vel.mag()});
        }
  }
  check("locked support cardinality", registered_support_arms == 6);
  check("one-Moore-shell support", worst_outside_support <= support_gate);

  const int covariance_L = 17;
  ftd::RenderBridge reference(covariance_L);
  ftd::RenderBridge translated(covariance_L);
  configure_isolated_wave(reference);
  configure_isolated_wave(translated);
  const ftd::Coord shift{3,-2,1};
  struct Seed { ftd::Coord site; ftd::Vec3 flux; ftd::Vec3 wave; };
  const std::vector<Seed> seeds{
      {{2,4,6}, {0.13,-0.07,0.02}, {0.03,0.11,-0.05}},
      {{3,4,6}, {-0.08,0.04,0.09}, {0.06,-0.02,0.01}},
      {{2,5,7}, {0.01,0.12,-0.03}, {-0.04,0.02,0.08}},
  };
  for (const auto& seed : seeds) {
    reference.voxels()[reference.lattice().index(
        seed.site.x,seed.site.y,seed.site.z)].flux = seed.flux;
    reference.voxels()[reference.lattice().index(
        seed.site.x,seed.site.y,seed.site.z)].wave_vel = seed.wave;
    translated.voxels()[translated.lattice().index(
        wrap(seed.site.x+shift.x,covariance_L),
        wrap(seed.site.y+shift.y,covariance_L),
        wrap(seed.site.z+shift.z,covariance_L))].flux = seed.flux;
    translated.voxels()[translated.lattice().index(
        wrap(seed.site.x+shift.x,covariance_L),
        wrap(seed.site.y+shift.y,covariance_L),
        wrap(seed.site.z+shift.z,covariance_L))].wave_vel = seed.wave;
  }
  for (int tick = 0; tick < 8; ++tick) {
    reference.tick();
    translated.tick();
  }
  double worst_translation_residual = 0.0;
  for (int x = 0; x < covariance_L; ++x)
    for (int y = 0; y < covariance_L; ++y)
      for (int z = 0; z < covariance_L; ++z) {
        const auto& lhs = reference.voxel_at(x,y,z);
        const auto& rhs = translated.voxel_at(
            wrap(x+shift.x,covariance_L),
            wrap(y+shift.y,covariance_L),
            wrap(z+shift.z,covariance_L));
        worst_translation_residual = std::max(
            worst_translation_residual,
            max_voxel_state_difference(lhs,rhs));
      }
  check("integer translation covariance",
      worst_translation_residual <= identity_gate);

  const int packet_L = 65;
  const auto packet = make_packet_spectrum(packet_L);
  ftd::RenderBridge packet_bridge(packet_L);
  configure_isolated_wave(packet_bridge);
  for (int x = 0; x < packet_L; ++x) {
    const auto flux = packet_flux(packet,x,0);
    const auto wave = packet_wave(packet,x,0);
    for (int y = 0; y < packet_L; ++y)
      for (int z = 0; z < packet_L; ++z)
        set_complex_state(packet_bridge.voxels()[
            packet_bridge.lattice().index(x,y,z)], flux, wave);
  }
  const auto initial_direct_centroid =
      normalized_centroid_from_row(packet_bridge);
  const auto initial_spectral_centroid =
      normalized_centroid_from_spectrum(packet,0);
  const double initial_spectral_norm = spectral_norm_from_row(packet_bridge);
  double worst_packet_replay_residual = 0.0;
  double worst_centroid_complex_residual = std::abs(
      initial_direct_centroid-initial_spectral_centroid);
  double worst_centroid_displacement_residual = 0.0;
  double worst_spectral_norm_residual = 0.0;
  double one_tick_displacement = NAN;
  int registered_centroid_ticks = 0;
  for (int tick = 1; tick <= 8; ++tick) {
    packet_bridge.tick();
    ++registered_centroid_ticks;
    for (int x = 0; x < packet_L; ++x) {
      const auto expected_flux = packet_flux(packet,x,tick);
      const auto expected_wave = packet_wave(packet,x,tick);
      for (int y = 0; y < packet_L; ++y)
        for (int z = 0; z < packet_L; ++z) {
          const auto& voxel = packet_bridge.voxel_at(x,y,z);
          worst_packet_replay_residual = std::max({
              worst_packet_replay_residual,
              std::abs(flux_complex(voxel)-expected_flux),
              std::abs(wave_complex(voxel)-expected_wave)});
        }
    }
    const auto direct = normalized_centroid_from_row(packet_bridge);
    const auto spectral = normalized_centroid_from_spectrum(packet,tick);
    worst_centroid_complex_residual = std::max(
        worst_centroid_complex_residual, std::abs(direct-spectral));
    const double direct_displacement = centroid_displacement(
        direct, initial_direct_centroid, packet_L);
    const double spectral_displacement = centroid_displacement(
        spectral, initial_spectral_centroid, packet_L);
    worst_centroid_displacement_residual = std::max(
        worst_centroid_displacement_residual,
        std::abs(direct_displacement-spectral_displacement));
    if (tick == 1) one_tick_displacement = direct_displacement;
    worst_spectral_norm_residual = std::max(
        worst_spectral_norm_residual,
        std::abs(spectral_norm_from_row(packet_bridge)
                 -initial_spectral_norm));
  }
  check("locked centroid cardinality", registered_centroid_ticks == 8);
  check("packet production replay", worst_packet_replay_residual <= engine_gate);
  check("direct/spectral centroid", worst_centroid_complex_residual <= engine_gate
      && worst_centroid_displacement_residual <= engine_gate);
  check("spectral amplitude norm", worst_spectral_norm_residual <= identity_gate);
  const double distance_to_integer = std::abs(
      one_tick_displacement-std::round(one_tick_displacement));
  check("noninteger one-tick centroid witness", distance_to_integer > 0.05);

  const bool passed = failures == 0;
  std::cout << "registered_mode_arms=" << analysis.modes.size() << '\n'
            << "cpu_backend_checks=" << cpu_backend_checks << '\n'
            << "registered_mode_tick_replays="
            << registered_mode_tick_replays << '\n'
            << "registered_support_arms=" << registered_support_arms << '\n'
            << "registered_translation_arms=1\n"
            << "registered_centroid_ticks=" << registered_centroid_ticks << '\n'
            << "maximum_mode_identity_residual="
            << analysis.maximum_identity_residual << '\n'
            << "maximum_group_speed=" << analysis.maximum_group_speed << '\n'
            << "maximum_ir_sixth_order_residual="
            << analysis.maximum_ir_sixth_order_residual << '\n'
            << "worst_cubic_orbit_residual="
            << worst_cubic_orbit_residual << '\n'
            << "worst_engine_mode_residual="
            << worst_engine_mode_residual << '\n'
            << "worst_outside_support=" << worst_outside_support << '\n'
            << "worst_translation_residual="
            << worst_translation_residual << '\n'
            << "worst_packet_replay_residual="
            << worst_packet_replay_residual << '\n'
            << "worst_centroid_complex_residual="
            << worst_centroid_complex_residual << '\n'
            << "worst_centroid_displacement_residual="
            << worst_centroid_displacement_residual << '\n'
            << "worst_spectral_norm_residual="
            << worst_spectral_norm_residual << '\n'
            << "one_tick_centroid_displacement="
            << one_tick_displacement << '\n'
            << "one_tick_distance_to_integer="
            << distance_to_integer << '\n'
            << "verdict="
            << (passed
                ? "INTEGER_TRANSLATION_SUPPORTS_CONTINUOUS_BLOCH_CENTROID_FLUX_ONLY"
                : "NATIVE_FLUX_BLOCH_REPLAY_FAILED") << '\n'
            << "integer_bloch_transport failures=" << failures << '\n';
  return passed ? 0 : 1;
}
