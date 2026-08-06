/** FTD-0754 supplemental: complete-observer covariance on nontrivial fields. */

#include "ftd/eft/state_only_matter_field_observer.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

namespace {

int failures=0;

void check(const std::string& label,bool condition) {
  if(condition) return;
  ++failures;
  std::cerr<<"FAIL: "<<label<<'\n';
}

int wrap(int value,int L) {
  const int r=value%L;
  return r<0?r+L:r;
}

ftd::Vec3 rotate(const ftd::Vec3& value) {
  return {value.z,value.x,value.y};
}

ftd::eft::MatchedMatterPoint point_at(const ftd::Vec3& position,
                                      const ftd::Vec3& momentum,int L) {
  ftd::eft::MatchedMatterPoint result;
  const long long ax=std::llround(position.x);
  const long long ay=std::llround(position.y);
  const long long az=std::llround(position.z);
  result.anchor={wrap(static_cast<int>(ax),L),
                 wrap(static_cast<int>(ay),L),
                 wrap(static_cast<int>(az),L)};
  result.remainder={position.x-ax,position.y-ay,position.z-az};
  result.momentum=momentum;
  return result;
}

ftd::eft::ConnectedMooreBlockState make_geometry(int L) {
  ftd::eft::ConnectedMooreBlockState result(L);
  const ftd::Vec3 center{static_cast<double>(L/2),
                         static_cast<double>(L/2),
                         static_cast<double>(L/2)};
  const ftd::Vec3 unit=ftd::Vec3{1.0,-1.0,0.0}*(1.0/std::sqrt(2.0));
  result.constituents.push_back(point_at(
      center-unit*0.5,unit*0.013,L));
  result.constituents.push_back(point_at(
      center+unit*0.5,unit*(-0.013),L));
  result.charges={+1,-1};
  return result;
}

ftd::eft::ConnectedMooreBlockState translate_state(
    const ftd::eft::ConnectedMooreBlockState& state,int dx,int dy,int dz) {
  const int L=state.electric.L;
  ftd::eft::ConnectedMooreBlockState result(L);
  result.constituents=state.constituents;
  result.charges=state.charges;
  result.edges=state.edges;
  result.width=state.width;
  result.orientation_axis=state.orientation_axis;
  for(auto& point:result.constituents) {
    point.anchor.x=wrap(point.anchor.x+dx,L);
    point.anchor.y=wrap(point.anchor.y+dy,L);
    point.anchor.z=wrap(point.anchor.z+dz,L);
  }
  for(int x=0;x<L;++x) for(int y=0;y<L;++y) for(int z=0;z<L;++z) {
    const auto old=static_cast<std::size_t>(state.electric.index(x,y,z));
    const auto now=static_cast<std::size_t>(
        result.electric.index(x+dx,y+dy,z+dz));
    result.electric.x[now]=state.electric.x[old];
    result.electric.y[now]=state.electric.y[old];
    result.electric.z[now]=state.electric.z[old];
    result.magnetic_half.x[now]=state.magnetic_half.x[old];
    result.magnetic_half.y[now]=state.magnetic_half.y[old];
    result.magnetic_half.z[now]=state.magnetic_half.z[old];
  }
  return result;
}

ftd::eft::ConnectedMooreBlockState rotate_state(
    const ftd::eft::ConnectedMooreBlockState& state) {
  const int L=state.electric.L;
  ftd::eft::ConnectedMooreBlockState result(L);
  result.constituents=state.constituents;
  result.charges=state.charges;
  result.edges=state.edges;
  result.width=state.width;
  result.orientation_axis=state.orientation_axis<0?-1:
      (state.orientation_axis+1)%3;
  for(auto& point:result.constituents) {
    point.anchor={point.anchor.z,point.anchor.x,point.anchor.y};
    point.remainder=rotate(point.remainder);
    point.momentum=rotate(point.momentum);
  }
  for(int x=0;x<L;++x) for(int y=0;y<L;++y) for(int z=0;z<L;++z) {
    const auto old=static_cast<std::size_t>(state.electric.index(x,y,z));
    const auto now=static_cast<std::size_t>(result.electric.index(z,x,y));
    result.electric.x[now]=state.electric.z[old];
    result.electric.y[now]=state.electric.x[old];
    result.electric.z[now]=state.electric.y[old];
    result.magnetic_half.x[now]=state.magnetic_half.z[old];
    result.magnetic_half.y[now]=state.magnetic_half.x[old];
    result.magnetic_half.z[now]=state.magnetic_half.y[old];
  }
  return result;
}

ftd::eft::ConnectedMooreBlockState conjugate_state(
    const ftd::eft::ConnectedMooreBlockState& state) {
  auto result=state;
  for(auto& charge:result.charges) charge=-charge;
  for(std::size_t i=0;i<result.electric.x.size();++i) {
    result.electric.x[i]=-result.electric.x[i];
    result.electric.y[i]=-result.electric.y[i];
    result.electric.z[i]=-result.electric.z[i];
    result.magnetic_half.x[i]=-result.magnetic_half.x[i];
    result.magnetic_half.y[i]=-result.magnetic_half.y[i];
    result.magnetic_half.z[i]=-result.magnetic_half.z[i];
  }
  return result;
}

bool close(double lhs,double rhs,double tolerance=5e-11) {
  return std::abs(lhs-rhs)<=tolerance*std::max({1.0,std::abs(lhs),std::abs(rhs)});
}

void compare_observations(
    const std::string& label,
    const ftd::eft::StateOnlyMatterFieldObservation& lhs,
    const ftd::eft::StateOnlyMatterFieldObservation& rhs) {
  check(label+" valid",lhs.valid&&rhs.valid);
  check(label+" kinetic",close(lhs.constituent_kinetic_energy,
                                rhs.constituent_kinetic_energy));
  check(label+" internal",close(lhs.pair_internal_energy,
                                 rhs.pair_internal_energy));
  check(label+" bound",close(lhs.bound_energy,rhs.bound_energy));
  check(label+" residual",close(lhs.residual_energy,rhs.residual_energy));
  check(label+" outgoing",close(lhs.outgoing_energy,rhs.outgoing_energy));
  check(label+" incoming",close(lhs.incoming_energy,rhs.incoming_energy));
  check(label+" radial",close(lhs.radial_energy,rhs.radial_energy));
  check(label+" background",close(lhs.background_energy,rhs.background_energy));
  check(label+" boundary ledger",
      lhs.boundary_energy_ledger_valid&&rhs.boundary_energy_ledger_valid);
  check(label+" total interference",close(lhs.bound_residual_interference,
                                            rhs.bound_residual_interference));
  check(label+" primitive interference",close(lhs.primitive_face_interference,
                                                rhs.primitive_face_interference));
  check(label+" induced boundary",close(lhs.induced_boundary_interference,
                                          rhs.induced_boundary_interference));
  check(label+" centering interference",close(lhs.centering_metric_interference,
                                                rhs.centering_metric_interference));
  check(label+" magnetic interference",close(lhs.centered_magnetic_interference,
                                               rhs.centered_magnetic_interference));
  check(label+" boundary flux",close(lhs.boundary_flux_sum,
                                      rhs.boundary_flux_sum));
  check(label+" signed flux",close(lhs.signed_radial_poynting,
                                    rhs.signed_radial_poynting));
  check(label+" shell count",lhs.shells.size()==rhs.shells.size());
  for(std::size_t i=0;i<std::min(lhs.shells.size(),rhs.shells.size());++i) {
    check(label+" shell outgoing "+std::to_string(i),
          close(lhs.shells[i].outgoing_energy,rhs.shells[i].outgoing_energy));
    check(label+" shell incoming "+std::to_string(i),
          close(lhs.shells[i].incoming_energy,rhs.shells[i].incoming_energy));
    check(label+" shell radial "+std::to_string(i),
          close(lhs.shells[i].radial_energy,rhs.shells[i].radial_energy));
    check(label+" shell signed "+std::to_string(i),
          close(lhs.shells[i].signed_radial_poynting,
                rhs.shells[i].signed_radial_poynting));
  }
}

void compare_ladders(
    const std::string& label,
    const ftd::eft::StateOnlySupportLadderObservation& lhs,
    const ftd::eft::StateOnlySupportLadderObservation& rhs) {
  check(label+" valid",lhs.valid&&rhs.valid);
  check(label+" scale count",lhs.scales.size()==rhs.scales.size());
  check(label+" transition count",
      lhs.transitions.size()==rhs.transitions.size());
  for(std::size_t i=0;i<std::min(lhs.scales.size(),rhs.scales.size());++i) {
    check(label+" bound energy "+std::to_string(i),
        close(lhs.scales[i].bound_face_energy,
              rhs.scales[i].bound_face_energy));
    check(label+" residual energy "+std::to_string(i),
        close(lhs.scales[i].residual_face_energy,
              rhs.scales[i].residual_face_energy));
    check(label+" primitive cross "+std::to_string(i),
        close(lhs.scales[i].primitive_interference,
              rhs.scales[i].primitive_interference));
  }
  for(std::size_t i=0;
      i<std::min(lhs.transitions.size(),rhs.transitions.size());++i) {
    check(label+" relaxation "+std::to_string(i),
        close(lhs.transitions[i].relaxation_energy,
              rhs.transitions[i].relaxation_energy));
    check(label+" monotonic margin "+std::to_string(i),
        close(lhs.transitions[i].monotonicity_margin,
              rhs.transitions[i].monotonicity_margin));
  }
}

}  // namespace

int main() {
  constexpr int L=17;
  ftd::eft::ConnectedMooreBlockOptions action;
  action.binding_law=ftd::eft::ConnectedBindingLaw::DerivedCompactPair;
  const auto preparation=ftd::eft::prepare_finite_support_derived_compact_pair(
      make_geometry(L),action,4,1e-13,4096);
  check("preparation",preparation.valid);
  auto state=preparation.state;
  const auto edge=ftd::eft::make_transverse_challenge(L,3e-4);
  const auto curl=ftd::eft::matched_curl(edge);
  for(std::size_t i=0;i<state.electric.x.size();++i) {
    state.electric.x[i]+=0.7*curl.x[i];
    state.electric.y[i]+=0.7*curl.y[i];
    state.electric.z[i]+=0.7*curl.z[i];
    state.magnetic_half.x[i]+=0.6*edge.x[i];
    state.magnetic_half.y[i]+=0.6*edge.y[i];
    state.magnetic_half.z[i]+=0.6*edge.z[i];
  }
  ftd::eft::StateOnlyMatterFieldObserverOptions options;
  options.shell_radii={2,4,6};
  const auto base=ftd::eft::observe_state_only_matter_field(
      state,action,options);
  const auto translated=ftd::eft::observe_state_only_matter_field(
      translate_state(state,2,-1,3),action,options);
  const auto rotated=ftd::eft::observe_state_only_matter_field(
      rotate_state(state),action,options);
  const auto conjugated=ftd::eft::observe_state_only_matter_field(
      conjugate_state(state),action,options);
  compare_observations("translation",base,translated);
  compare_observations("proper cubic rotation",base,rotated);
  compare_observations("polarity conjugation",base,conjugated);
  check("translation center",
      translated.center.x==wrap(static_cast<int>(base.center.x)+2,L)
      &&translated.center.y==wrap(static_cast<int>(base.center.y)-1,L)
      &&translated.center.z==wrap(static_cast<int>(base.center.z)+3,L));
  check("rotation center",(rotated.center-rotate(base.center)).mag()<=1e-12);

  const std::vector<int> supports{3,4,5};
  const auto base_ladder=ftd::eft::observe_state_only_support_ladder(
      state,action,supports);
  const auto translated_ladder=ftd::eft::observe_state_only_support_ladder(
      translate_state(state,2,-1,3),action,supports);
  const auto rotated_ladder=ftd::eft::observe_state_only_support_ladder(
      rotate_state(state),action,supports);
  const auto conjugated_ladder=ftd::eft::observe_state_only_support_ladder(
      conjugate_state(state),action,supports);
  compare_ladders("ladder translation",base_ladder,translated_ladder);
  compare_ladders("ladder proper cubic rotation",base_ladder,rotated_ladder);
  compare_ladders("ladder polarity conjugation",base_ladder,conjugated_ladder);

  std::cout.precision(17);
  std::cout<<"base_residual="<<base.residual_energy<<'\n'
           <<"base_outgoing="<<base.outgoing_energy<<'\n'
           <<"base_incoming="<<base.incoming_energy<<'\n'
           <<"base_radial="<<base.radial_energy<<'\n'
           <<"state_only_observer_covariance failures="<<failures<<'\n';
  return failures==0?0:1;
}
