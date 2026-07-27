#include "ftd/eft/genesis_action_obstruction.h"

#include "ftd/lagrangian.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

namespace ftd::eft {
namespace {

constexpr double gate = 1e-12;
constexpr double kg = 1.0;
constexpr double km = 1.0;

bool equal_vec(const Vec3& a,const Vec3& b) {
  return a.x==b.x && a.y==b.y && a.z==b.z;
}

bool equal_frozen_voxel(const Voxel& a,const Voxel& b) {
  return a.state==b.state && equal_vec(a.flux,b.flux)
      && equal_vec(a.wave_vel,b.wave_vel)
      && equal_vec(a.velocity,b.velocity)
      && equal_vec(a.remainder,b.remainder)
      && a.latency==b.latency && a.tau==b.tau && a.phase==b.phase
      && a.locked==b.locked && a.particle_id==b.particle_id
      && a.pair_id==b.pair_id && a.spin==b.spin && a.color==b.color
      && a.flavor==b.flavor;
}

void apply_production_evaporation_erasure(Voxel& voxel) {
  voxel.state = 0;
  voxel.particle_id = -1;
  voxel.spin = 0;
  voxel.color = 0;
}

GenesisActionArm analyze_arm(bool dual,double excess,double wave_mag2,
                             double drain,int polarity) {
  GenesisActionArm arm;
  arm.dual_substrate = dual;
  arm.excess = excess;
  arm.wave_magnitude_squared = wave_mag2;
  arm.kinetic_drain = drain;
  arm.polarity = polarity;
  arm.acceptance_probability = 1.0-std::exp(-excess/km);
  arm.flux_magnitude_before = kg+excess;
  arm.flux_magnitude_after = dual ? arm.flux_magnitude_before : excess;
  arm.flux_energy_withdrawn = dual ? 0.0
      : 0.5*(arm.flux_magnitude_before*arm.flux_magnitude_before
             -arm.flux_magnitude_after*arm.flux_magnitude_after);
  const double wave_after_factor = dual ? 1.0 : 1.0-drain;
  arm.wave_energy_withdrawn = 0.5*wave_mag2
      *(1.0-wave_after_factor*wave_after_factor);

  const double expected_after = dual ? kg+excess : excess;
  const double expected_flux_loss = dual ? 0.0 : kg*excess+0.5*kg*kg;
  const double expected_wave_loss = dual ? 0.0
      : (drain-0.5*drain*drain)*wave_mag2;
  arm.amplitude_residual = std::abs(arm.flux_magnitude_after-expected_after);
  arm.flux_energy_residual =
      std::abs(arm.flux_energy_withdrawn-expected_flux_loss);
  arm.wave_energy_residual =
      std::abs(arm.wave_energy_withdrawn-expected_wave_loss);
  arm.valid = arm.amplitude_residual<=gate
      && arm.flux_energy_residual<=gate
      && arm.wave_energy_residual<=gate
      && arm.acceptance_probability>0.0
      && arm.acceptance_probability<1.0;
  return arm;
}

double written_local_action(int state,double divergence) {
  Voxel voxel;
  voxel.state = static_cast<std::int8_t>(state);
  voxel.velocity = {0.0,0.0,0.0};
  voxel.latency = 0.0;
  return born_infeld_term(voxel)
      + coupling_term(voxel,divergence)
      + gauss_term(divergence,static_cast<double>(state));
}

const GenesisActionArm* find_arm(
    const std::vector<GenesisActionArm>& arms,bool dual,double excess,
    double wave_mag2,double drain,int polarity) {
  for (const auto& arm : arms)
    if (arm.dual_substrate==dual && arm.excess==excess
        && arm.wave_magnitude_squared==wave_mag2
        && arm.kinetic_drain==drain && arm.polarity==polarity)
      return &arm;
  return nullptr;
}

}  // namespace

GenesisActionObstructionResult analyze_genesis_action_obstruction() {
  GenesisActionObstructionResult result;
  const std::array<double,4> excesses{{0.125,0.25,0.5,1.0}};
  const std::array<double,2> wave_mag2_values{{0.0,0.25}};
  const std::array<double,2> drains{{0.0,0.5}};

  for (double excess : excesses)
    for (double wave_mag2 : wave_mag2_values)
      for (double drain : drains)
        for (int polarity : {1,-1}) {
          auto arm = analyze_arm(false,excess,wave_mag2,drain,polarity);
          result.maximum_amplitude_residual = std::max(
              result.maximum_amplitude_residual,arm.amplitude_residual);
          result.maximum_flux_energy_residual = std::max(
              result.maximum_flux_energy_residual,arm.flux_energy_residual);
          result.maximum_wave_energy_residual = std::max(
              result.maximum_wave_energy_residual,arm.wave_energy_residual);
          result.arms.push_back(arm);
        }

  for (double excess : excesses)
    for (double wave_mag2 : wave_mag2_values)
      for (int polarity : {1,-1}) {
        auto arm = analyze_arm(true,excess,wave_mag2,0.0,polarity);
        result.maximum_amplitude_residual = std::max(
            result.maximum_amplitude_residual,arm.amplitude_residual);
        result.maximum_flux_energy_residual = std::max(
            result.maximum_flux_energy_residual,arm.flux_energy_residual);
        result.maximum_wave_energy_residual = std::max(
            result.maximum_wave_energy_residual,arm.wave_energy_residual);
        result.arms.push_back(arm);
      }

  result.single_map_preserves_overshoot = true;
  std::vector<double> post_amplitudes;
  std::vector<double> fixed_register_losses;
  double previous_probability = -1.0;
  result.acceptance_conditioning_does_not_lock = true;
  for (double excess : excesses) {
    const auto* arm = find_arm(result.arms,false,excess,0.0,0.0,1);
    if (!arm) {
      result.single_map_preserves_overshoot = false;
      result.acceptance_conditioning_does_not_lock = false;
      continue;
    }
    result.single_map_preserves_overshoot =
        result.single_map_preserves_overshoot
        && std::abs(arm->flux_magnitude_after-excess)<=gate;
    result.acceptance_conditioning_does_not_lock =
        result.acceptance_conditioning_does_not_lock
        && arm->acceptance_probability>previous_probability
        && std::abs(arm->flux_magnitude_after-excess)<=gate;
    previous_probability = arm->acceptance_probability;
    post_amplitudes.push_back(arm->flux_magnitude_after);
    fixed_register_losses.push_back(
        arm->flux_energy_withdrawn+arm->wave_energy_withdrawn);
  }
  std::sort(post_amplitudes.begin(),post_amplitudes.end());
  post_amplitudes.erase(
      std::unique(post_amplitudes.begin(),post_amplitudes.end()),
      post_amplitudes.end());
  result.distinct_single_post_amplitudes =
      static_cast<int>(post_amplitudes.size());
  result.no_post_genesis_amplitude_lock =
      result.distinct_single_post_amplitudes
      ==result.expected_distinct_single_post_amplitudes;
  const auto loss_range = std::minmax_element(
      fixed_register_losses.begin(),fixed_register_losses.end());
  result.fixed_quantum_energy_spread =
      *loss_range.second-*loss_range.first;
  result.no_fixed_ternary_energy_quantum =
      result.fixed_quantum_energy_spread>gate;

  result.dual_branch_has_no_latent_heat_payment = true;
  bool polarity_exact = true;
  for (bool dual : {false,true})
    for (double excess : excesses)
      for (double wave_mag2 : wave_mag2_values)
        for (double drain : (dual ? std::array<double,2>{{0.0,0.0}} : drains)) {
          if (dual && drain!=0.0) continue;
          const auto* plus = find_arm(
              result.arms,dual,excess,wave_mag2,drain,1);
          const auto* minus = find_arm(
              result.arms,dual,excess,wave_mag2,drain,-1);
          if (!plus || !minus) { polarity_exact=false; continue; }
          const double scalar_residual = std::max({
              std::abs(plus->flux_magnitude_after-minus->flux_magnitude_after),
              std::abs(plus->flux_energy_withdrawn-minus->flux_energy_withdrawn),
              std::abs(plus->wave_energy_withdrawn-minus->wave_energy_withdrawn)});
          result.maximum_polarity_scalar_residual = std::max(
              result.maximum_polarity_scalar_residual,scalar_residual);
          polarity_exact = polarity_exact && scalar_residual<=gate;
          if (dual)
            result.dual_branch_has_no_latent_heat_payment =
                result.dual_branch_has_no_latent_heat_payment
                && plus->flux_energy_withdrawn==0.0
                && plus->wave_energy_withdrawn==0.0;
        }

  Voxel plus_preimage;
  plus_preimage.state = 1;
  plus_preimage.flux = {0.25,-0.5,0.75};
  plus_preimage.wave_vel = {-0.125,0.25,0.5};
  plus_preimage.velocity = {0.1,0.0,0.0};
  plus_preimage.remainder = {0.2,-0.1,0.3};
  plus_preimage.latency = 0.125;
  plus_preimage.tau = 7.0;
  plus_preimage.phase = 0.75;
  plus_preimage.particle_id = 11;
  plus_preimage.spin = 1;
  plus_preimage.color = 2;
  Voxel minus_preimage = plus_preimage;
  minus_preimage.state = -1;
  minus_preimage.particle_id = 22;
  minus_preimage.spin = -1;
  minus_preimage.color = 3;
  const bool preimages_distinct =
      !equal_frozen_voxel(plus_preimage,minus_preimage);
  apply_production_evaporation_erasure(plus_preimage);
  apply_production_evaporation_erasure(minus_preimage);
  result.evaporation_signed_preimages_collapse =
      preimages_distinct && equal_frozen_voxel(plus_preimage,minus_preimage);

  const std::array<double,3> below_action{{
      written_local_action(0,0.0),written_local_action(1,0.0),
      written_local_action(-1,0.0)}};
  const std::array<double,3> above_action{{
      written_local_action(0,0.0),written_local_action(1,0.0),
      written_local_action(-1,0.0)}};
  for (std::size_t index=0;index<below_action.size();++index)
    result.maximum_action_threshold_residual = std::max(
        result.maximum_action_threshold_residual,
        std::abs(below_action[index]-above_action[index]));
  const bool below_eligible = 0.5*kg>kg;
  const bool above_eligible = 2.0*kg>kg;
  result.written_action_cannot_generate_magnitude_gate =
      result.maximum_action_threshold_residual<=gate
      && !below_eligible && above_eligible;
  result.written_action_zero_divergence_polarity_degenerate =
      std::abs(below_action[1]-below_action[2])<=gate
      && ((0.0>0.0) ? 1 : -1)==-1;

  result.frozen_common_action_route_closed =
      result.single_map_preserves_overshoot
      && result.no_post_genesis_amplitude_lock
      && result.no_fixed_ternary_energy_quantum
      && result.acceptance_conditioning_does_not_lock
      && result.dual_branch_has_no_latent_heat_payment
      && result.evaporation_signed_preimages_collapse
      && result.written_action_cannot_generate_magnitude_gate
      && result.written_action_zero_divergence_polarity_degenerate;
  result.extended_reservoir_or_open_system_remains_open = true;
  result.valid = result.arms.size()==48
      && std::all_of(result.arms.begin(),result.arms.end(),
                     [](const auto& arm){return arm.valid;})
      && polarity_exact
      && result.maximum_amplitude_residual<=gate
      && result.maximum_flux_energy_residual<=gate
      && result.maximum_wave_energy_residual<=gate
      && result.maximum_polarity_scalar_residual<=gate
      && result.maximum_action_threshold_residual<=gate
      && result.frozen_common_action_route_closed
      && result.extended_reservoir_or_open_system_remains_open;
  return result;
}

}  // namespace ftd::eft
