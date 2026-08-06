/** FTD-0754: state-only bound/characteristic observer algebra. */

#include "ftd/eft/state_only_matter_field_observer.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

namespace {

constexpr double kGate = 1e-12;
int failures = 0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

double difference(const ftd::Vec3& a, const ftd::Vec3& b) {
  return (a-b).mag();
}

ftd::Vec3 rotate_cyclic(const ftd::Vec3& value) {
  return {value.z,value.x,value.y};
}

int wrap(int value, int L) {
  const int remainder = value%L;
  return remainder < 0 ? remainder+L : remainder;
}

ftd::eft::MatchedMatterPoint point_at(const ftd::Vec3& position,
                                      const ftd::Vec3& momentum, int L) {
  ftd::eft::MatchedMatterPoint point;
  const long long ax=std::llround(position.x);
  const long long ay=std::llround(position.y);
  const long long az=std::llround(position.z);
  point.anchor={wrap(static_cast<int>(ax),L),
                wrap(static_cast<int>(ay),L),
                wrap(static_cast<int>(az),L)};
  point.remainder={position.x-ax,position.y-ay,position.z-az};
  point.momentum=momentum;
  return point;
}

ftd::eft::ConnectedMooreBlockState make_pair(int L) {
  ftd::eft::ConnectedMooreBlockState state(L);
  const ftd::Vec3 center{static_cast<double>(L/2),
                         static_cast<double>(L/2),
                         static_cast<double>(L/2)};
  state.constituents.push_back(point_at(
      center-ftd::Vec3{0.0,0.0,0.5},{0.0,0.0,0.015},L));
  state.constituents.push_back(point_at(
      center+ftd::Vec3{0.0,0.0,0.5},{0.0,0.0,-0.015},L));
  state.charges={+1,-1};
  return state;
}

}  // namespace

int main() {
  using ftd::Vec3;
  using ftd::eft::decompose_centered_characteristic_sample;

  const auto outgoing = decompose_centered_characteristic_sample(
      {0.0,2.0,0.0},{0.0,0.0,2.0},{1.0,0.0,0.0},kGate);
  check("pure outgoing valid",outgoing.valid);
  check("pure outgoing background zero",
      outgoing.background_energy <= kGate);
  check("pure outgoing positive flux",
      std::abs(outgoing.signed_radial_poynting-4.0) <= kGate);

  const auto incoming = decompose_centered_characteristic_sample(
      {0.0,2.0,0.0},{0.0,0.0,-2.0},{1.0,0.0,0.0},kGate);
  check("pure incoming valid",incoming.valid);
  check("pure incoming outgoing zero",incoming.outgoing_energy <= kGate);
  check("pure incoming negative flux",
      std::abs(incoming.signed_radial_poynting+4.0) <= kGate);

  const auto radial = decompose_centered_characteristic_sample(
      {3.0,0.0,0.0},{-2.0,0.0,0.0},{1.0,0.0,0.0},kGate);
  check("radial constraint valid",radial.valid);
  check("radial constraint is background",radial.outgoing_energy <= kGate
      && std::abs(radial.background_energy-6.5) <= kGate);

  const auto standing = decompose_centered_characteristic_sample(
      {0.0,2.0,0.0},{0.0,0.0,0.0},{1.0,0.0,0.0},kGate);
  check("standing wave valid",standing.valid);
  check("standing wave has both characteristics",
      standing.outgoing_energy > 0.0 && standing.incoming_energy > 0.0);
  check("standing wave net flux zero",
      std::abs(standing.signed_radial_poynting) <= kGate);

  const Vec3 e{0.31,-0.27,0.18};
  const Vec3 b{-0.22,0.14,0.29};
  const Vec3 n{1.0,2.0,-3.0};
  const auto base = decompose_centered_characteristic_sample(e,b,n,kGate);
  const auto rotated = decompose_centered_characteristic_sample(
      rotate_cyclic(e),rotate_cyclic(b),rotate_cyclic(n),kGate);
  check("proper cubic covariance valid",base.valid && rotated.valid);
  check("proper cubic covariance outgoing vector",
      difference(rotate_cyclic(base.outgoing_electric),
                 rotated.outgoing_electric) <= kGate);
  check("proper cubic covariance background vector",
      difference(rotate_cyclic(base.background_magnetic),
                 rotated.background_magnetic) <= kGate);
  check("proper cubic covariance scalars",
      std::abs(base.outgoing_energy-rotated.outgoing_energy) <= kGate
      && std::abs(base.background_energy-rotated.background_energy) <= kGate);

  const auto conjugate = decompose_centered_characteristic_sample(
      e*(-1.0),b*(-1.0),n,kGate);
  check("polarity conjugation valid",conjugate.valid);
  check("polarity conjugates field components",
      difference(conjugate.outgoing_electric,
                 base.outgoing_electric*(-1.0)) <= kGate
      && difference(conjugate.background_magnetic,
                    base.background_magnetic*(-1.0)) <= kGate);
  check("polarity preserves observer energies",
      std::abs(conjugate.outgoing_energy-base.outgoing_energy) <= kGate
      && std::abs(conjugate.background_energy-base.background_energy) <= kGate);

  const auto central = decompose_centered_characteristic_sample(
      e,b,{0.0,0.0,0.0},kGate);
  check("central sample valid",central.valid);
  check("central sample fails closed to background",
      central.outgoing_energy == 0.0
      && std::abs(central.background_energy-central.residual_energy) <= kGate);

  ftd::eft::ConnectedMooreBlockOptions action;
  action.binding_law=ftd::eft::ConnectedBindingLaw::DerivedCompactPair;
  const auto prepared=ftd::eft::prepare_finite_support_derived_compact_pair(
      make_pair(17),action,4,1e-13,4096);
  check("bound control preparation",prepared.valid);
  ftd::eft::StateOnlyMatterFieldObserverOptions options;
  options.shell_radii={2,4,6};
  const auto observation=ftd::eft::observe_state_only_matter_field(
      prepared.state,action,options);
  check("bound control observation valid",observation.valid);
  check("bound control exact residual zero",
      observation.residual_energy <= kGate
      && observation.outgoing_energy <= kGate
      && observation.background_energy <= kGate);
  check("bound control exact reconstruction",
      observation.maximum_reconstruction_residual <= kGate
      && std::abs(observation.energy_partition_residual) <= kGate
      && observation.actual_gauss_compatibility_residual <= kGate);
  check("bound control boundary ledger valid",
      observation.boundary_energy_ledger_valid
      && std::abs(observation.primitive_face_interference) <= kGate
      && std::abs(observation.induced_boundary_interference) <= kGate
      && std::abs(observation.centering_metric_interference) <= kGate
      && std::abs(observation.centered_magnetic_interference) <= kGate);
  check("bound control state-only scope flags",observation.state_only
      && observation.centered_readout_only
      && !observation.primitive_cochain_uniqueness_claimed);
  check("bound control shells populated",
      std::all_of(observation.shells.begin(),observation.shells.end(),
          [](const auto& shell) { return shell.samples > 0; }));

  const auto ladder=ftd::eft::observe_state_only_support_ladder(
      prepared.state,action,{3,4,5});
  check("support ladder valid",ladder.valid
      &&ladder.state_only&&ladder.support_is_resolution_scale);
  check("support ladder complete",
      ladder.scales.size()==3&&ladder.transitions.size()==2);
  check("support ladder monotone",
      std::all_of(ladder.transitions.begin(),ladder.transitions.end(),
          [](const auto& step) {
            return step.valid&&step.monotonicity_margin>=0.0;
          }));
  check("support ladder pythagorean",
      ladder.maximum_projection_residual<=kGate
      &&ladder.maximum_energy_reconstruction_residual<=kGate);
  check("registered support reconstructs bound control",
      ladder.scales.size()==3
      &&std::abs(ladder.scales[1].residual_face_energy)<=kGate
      &&std::abs(ladder.scales[1].primitive_interference)<=kGate);
  const auto unordered_ladder=ftd::eft::observe_state_only_support_ladder(
      prepared.state,action,{4,3,5});
  check("unordered support ladder fails closed",!unordered_ladder.valid);

  // A closed electric plaquette crossing the selected support boundary is
  // exactly Gauss-free.  It supplies a nontrivial boundary exchange without
  // changing the matter source, so all three readout-interference pieces can
  // be checked independently.
  auto boundary_loop=prepared.state;
  const int cx=8,cy=8,cz=8,w=4;
  const double amplitude=0.1;
  boundary_loop.electric.x[boundary_loop.electric.index(cx+w,cy,cz)]
      +=amplitude;
  boundary_loop.electric.z[boundary_loop.electric.index(cx+w+1,cy,cz)]
      +=amplitude;
  boundary_loop.electric.x[boundary_loop.electric.index(cx+w,cy,cz+1)]
      -=amplitude;
  boundary_loop.electric.z[boundary_loop.electric.index(cx+w,cy,cz)]
      -=amplitude;
  const auto boundary_observation=
      ftd::eft::observe_state_only_matter_field(
          boundary_loop,action,options);
  check("boundary-loop observation valid",boundary_observation.valid);
  check("boundary-loop ledger valid",
      boundary_observation.boundary_energy_ledger_valid);
  check("boundary-loop primitive exchange nonzero",
      std::abs(boundary_observation.primitive_face_interference)>1e-8);
  check("boundary-loop primitive equals induced boundary",
      std::abs(boundary_observation.primitive_boundary_identity_residual)
          <=kGate);
  check("boundary-loop readout cross reconstructs",
      std::abs(boundary_observation
          .readout_interference_reconstruction_residual)<=kGate
      && std::abs(boundary_observation.bound_residual_interference
          -boundary_observation.primitive_face_interference
          -boundary_observation.centering_metric_interference
          -boundary_observation.centered_magnetic_interference)<=kGate);

  auto gauss_bad=prepared.state;
  gauss_bad.electric.x[0]+=1e-3;
  const auto rejected=ftd::eft::observe_state_only_matter_field(
      gauss_bad,action,options);
  check("Gauss-incompatible state fails closed",!rejected.valid
      && rejected.actual_gauss_compatibility_residual > 1e-6);

  std::cout.precision(17);
  std::cout << "outgoing_flux=" << outgoing.signed_radial_poynting << '\n'
            << "incoming_flux=" << incoming.signed_radial_poynting << '\n'
            << "standing_out=" << standing.outgoing_energy << '\n'
            << "standing_in=" << standing.incoming_energy << '\n'
            << "bound_reconstruction="
            << observation.maximum_reconstruction_residual << '\n'
            << "bound_energy_partition="
            << observation.energy_partition_residual << '\n'
            << "boundary_primitive="
            << boundary_observation.primitive_face_interference << '\n'
            << "boundary_induced="
            << boundary_observation.induced_boundary_interference << '\n'
            << "boundary_centering="
            << boundary_observation.centering_metric_interference << '\n'
            << "boundary_magnetic="
            << boundary_observation.centered_magnetic_interference << '\n'
            << "support_ladder_projection="
            << ladder.maximum_projection_residual << '\n'
            << "gauss_negative="
            << rejected.actual_gauss_compatibility_residual << '\n'
            << "state_only_matter_field_observer failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}
