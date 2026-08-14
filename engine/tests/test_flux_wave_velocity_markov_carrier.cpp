/** FTD-0876 native flux/wave-velocity canonical-carrier verifier. */

#include "ftd/eft/flux_wave_velocity_markov_carrier.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

int checks = 0;
int failures = 0;

void check(const std::string& label, bool condition) {
  ++checks;
  if (!condition) {
    ++failures;
    std::cerr << "FAIL  " << label << '\n';
  }
}

bool close(double first, double second, double tolerance = 1e-11) {
  return std::abs(first - second)
      <= tolerance * std::max({1.0, std::abs(first), std::abs(second)});
}

bool close_vec(const ftd::Vec3& first, const ftd::Vec3& second) {
  return close(first.x, second.x) && close(first.y, second.y)
      && close(first.z, second.z);
}

}  // namespace

int main() {
  using namespace ftd;
  using namespace ftd::eft;

  Voxel voxel;
  voxel.flux = {1.25, -2.5, 3.75};
  voxel.wave_vel = {-0.5, 0.25, 1.0};
  const auto native = carrier_from_voxel(voxel);
  check("native Voxel flux copied exactly", close_vec(native.flux, voxel.flux));
  check("native Voxel wave velocity copied exactly",
        close_vec(native.wave_velocity, voxel.wave_vel));

  const Vec3 previous{-1.0, 2.0, 0.5};
  const Vec3 current{0.5, -1.0, 2.0};
  const auto chart = flux_history_to_markov_carrier(
      previous, current, 0.25);
  check("history chart valid", chart.valid());
  check("history chart current flux exact", close_vec(chart.carrier.flux, current));
  check("history chart velocity exact",
        close_vec(chart.carrier.wave_velocity, {6.0, -12.0, 6.0}));
  check("history chart recovers prior flux",
        chart.exact_roundtrip && close_vec(chart.recovered_previous_flux, previous));

  FreeWaveKickDriftInput input;
  input.step = 0.125;
  input.sites = {
      {{1.0, -0.5, 0.25}, {0.2, -0.1, 0.4}},
      {{-0.25, 0.75, 1.5}, {-0.3, 0.5, -0.2}},
      {{0.5, 0.1, -1.0}, {0.7, -0.4, 0.3}},
  };
  input.stiffness = {
      1.0, -1.0, 0.0,
      -1.0, 2.0, -1.0,
      0.0, -1.0, 1.0,
  };
  const auto evolved = evolve_free_wave_kick_drift(input);
  check("symmetric free-wave witness valid", evolved.valid());
  check("stiffness accepted as symmetric", evolved.stiffness_symmetric);
  check("free-wave inverse exact", evolved.exact_inverse_verified);
  check("free-wave symplectic scope flag", evolved.free_wave_symplectic);
  check("inverse residual within tolerance",
        evolved.maximum_inverse_residual <= 1e-11);
  bool recovered = evolved.recovered.size() == input.sites.size();
  for (std::size_t index = 0; recovered && index < input.sites.size(); ++index) {
    recovered &= close_vec(evolved.recovered[index].flux,
                           input.sites[index].flux);
    recovered &= close_vec(evolved.recovered[index].wave_velocity,
                           input.sites[index].wave_velocity);
  }
  check("generic three-site vector state recovered", recovered);
  check("three native canonical pairs per site",
        evolved.native_canonical_pairs_per_site == 3);
  check("history/Markov equivalence retained",
        evolved.history_markov_equivalent);
  check("production Voxel pair is the source", evolved.production_voxel_pair_used);

  const FluxWaveVelocityCarrierSite left{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
  const FluxWaveVelocityCarrierSite right{{-2.0, 1.0, 0.5}, {3.0, -1.0, 2.0}};
  const double components =
      left.flux.x * right.wave_velocity.x
      - right.flux.x * left.wave_velocity.x
      + left.flux.y * right.wave_velocity.y
      - right.flux.y * left.wave_velocity.y
      + left.flux.z * right.wave_velocity.z
      - right.flux.z * left.wave_velocity.z;
  check("vector bond generator equals component sum",
        close(vector_canonical_bond_generator(left, right), components));

  check("damping symplectic pullback scale exact",
        close(uniform_damping_symplectic_scale(0.75), 0.5625));
  check("one-site damping phase determinant exact",
        close(uniform_damping_phase_determinant(0.5, 1), 1.0 / 64.0));
  check("zero damping factor is phase-volume singular",
        close(uniform_damping_phase_determinant(0.0, 2), 0.0));

  auto nonsymmetric = input;
  nonsymmetric.stiffness[1] = -0.5;
  check("nonsymmetric stiffness fails closed",
        evolve_free_wave_kick_drift(nonsymmetric).status
            == FluxWaveVelocityCarrierStatus::NonsymmetricStiffness);
  auto bad_shape = input;
  bad_shape.stiffness.pop_back();
  check("bad stiffness shape fails closed",
        evolve_free_wave_kick_drift(bad_shape).status
            == FluxWaveVelocityCarrierStatus::InvalidStiffnessShape);
  auto zero_step = input;
  zero_step.step = 0.0;
  check("zero time step fails closed",
        evolve_free_wave_kick_drift(zero_step).status
            == FluxWaveVelocityCarrierStatus::InvalidStep);
  check("zero-step history chart fails closed",
        flux_history_to_markov_carrier(previous, current, 0.0).status
            == FluxWaveVelocityCarrierStatus::InvalidStep);
  auto nonfinite = input;
  nonfinite.sites[0].flux.x = std::numeric_limits<double>::infinity();
  check("nonfinite carrier fails closed",
        evolve_free_wave_kick_drift(nonfinite).status
            == FluxWaveVelocityCarrierStatus::NonFiniteInput);

  check("complete production tick not promoted",
        !evolved.complete_production_tick_symplectic);
  check("production parity actuator not supplied",
        !evolved.production_parity_actuator_supplied);
  check("native record preparation not supplied",
        !evolved.native_record_preparation_supplied);
  check("native G-star synchronization not supplied",
        !evolved.native_gstar_synchronization_supplied);

  std::cout << "FTD-0876 flux/wave-velocity Markov carrier EFT: "
            << (checks - failures) << "/" << checks << " PASS\n";
  return failures == 0 ? 0 : 1;
}
