/** FTD-0763: fractional-center state-only observer CPU/CUDA qualification. */

#include "ftd/eft/cuda_state_only_support_ladder.h"
#include "ftd/eft/matched_gauss_transport.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

using namespace ftd;
using namespace ftd::eft;

int failures=0;

void check(const std::string& label,bool condition) {
  if(condition) return;
  ++failures;
  std::cerr<<"FAIL: "<<label<<'\n';
}

int wrap(int value,int L) {
  value%=L;
  return value<0?value+L:value;
}

MatchedMatterPoint point_at(const Vec3& position,
                            const Vec3& momentum,int L) {
  MatchedMatterPoint point;
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

ConnectedMooreBlockState make_pair(int L,const Vec3& offset,
                                   const Vec3& direction,int polarity) {
  ConnectedMooreBlockState state(L);
  const double c=static_cast<double>(L/2);
  const Vec3 center{c+offset.x,c+offset.y,c+offset.z};
  const Vec3 unit=direction*(1.0/direction.mag());
  state.constituents.push_back(point_at(
      center-unit*0.55,unit*0.011,L));
  state.constituents.push_back(point_at(
      center+unit*0.55,unit*(-0.011),L));
  state.charges={polarity,-polarity};
  return state;
}

Vec3 rotate(const Vec3& value) {
  return {value.z,value.x,value.y};
}

ConnectedMooreBlockState translate_state(
    const ConnectedMooreBlockState& state,int dx,int dy,int dz) {
  const int L=state.electric.L;
  ConnectedMooreBlockState result(L);
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

ConnectedMooreBlockState rotate_state(
    const ConnectedMooreBlockState& state) {
  const int L=state.electric.L;
  ConnectedMooreBlockState result(L);
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

ConnectedMooreBlockState conjugate_state(
    const ConnectedMooreBlockState& state) {
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

void add_challenge(ConnectedMooreBlockState& state,int polarity) {
  const int L=state.electric.L;
  MatchedEdgeField potential(L);
  const int c=L/2;
  const std::size_t index=static_cast<std::size_t>(
      potential.index(c+2,c-1,c+1));
  potential.x[index]=polarity*1.7e-4;
  potential.y[index]=polarity*(-2.1e-4);
  potential.z[index]=polarity*2.9e-4;
  const auto curl=matched_curl(potential);
  for(std::size_t i=0;i<state.electric.x.size();++i) {
    state.electric.x[i]+=curl.x[i];
    state.electric.y[i]+=curl.y[i];
    state.electric.z[i]+=curl.z[i];
    state.magnetic_half.x[i]+=0.7*potential.x[i];
    state.magnetic_half.y[i]+=0.7*potential.y[i];
    state.magnetic_half.z[i]+=0.7*potential.z[i];
  }
}

bool close(double lhs,double rhs,double tolerance=1e-12) {
  return std::abs(lhs-rhs)
      <=tolerance*std::max({1.0,std::abs(lhs),std::abs(rhs)});
}

void compare_case(int L,const Vec3& offset,const Vec3& direction,
                  int polarity,const std::string& label) {
  ConnectedMooreBlockOptions action;
  action.binding_law=ConnectedBindingLaw::DerivedCompactPair;
  const auto geometry=make_pair(L,offset,direction,polarity);
  const auto legacy=prepare_finite_support_derived_compact_pair(
      geometry,action,4,1e-13,4096);
  check(label+" legacy rejection",!legacy.valid);
  const auto preparation=prepare_finite_support_derived_compact_pair(
      geometry,action,4,1e-13,4096,true);
  check(label+" fractional preparation",preparation.valid
      &&preparation.compact_support&&preparation.zero_boundary_crossing
      &&preparation.fractional_center_enabled
      &&preparation.fractional_center_offset.mag()>1e-4);
  if(!preparation.valid) return;
  auto state=preparation.state;
  add_challenge(state,polarity);

  StateOnlyMatterFieldObserverOptions observer;
  observer.support_half_width=4;
  observer.shell_radii={2,4,6};
  observer.allow_fractional_center=true;
  const auto cpu=observe_state_only_matter_field(state,action,observer);
  CudaStateOnlySupportLadderTelemetry field_telemetry;
  const auto gpu=observe_state_only_matter_field_cuda(
      state,action,observer,&field_telemetry);
  check(label+" field valid",cpu.valid&&gpu.valid&&field_telemetry.valid
      &&cpu.boundary_energy_ledger_valid
      &&gpu.boundary_energy_ledger_valid);
  check(label+" field chart parity",
      cpu.fractional_center_enabled&&gpu.fractional_center_enabled
      &&(cpu.center-gpu.center).mag()<=1e-13
      &&(cpu.support_center-gpu.support_center).mag()<=1e-13
      &&(cpu.fractional_center_offset
          -gpu.fractional_center_offset).mag()<=1e-13);
  check(label+" field scalar parity",
      close(cpu.bound_energy,gpu.bound_energy)
      &&close(cpu.residual_energy,gpu.residual_energy)
      &&close(cpu.outgoing_energy,gpu.outgoing_energy)
      &&close(cpu.incoming_energy,gpu.incoming_energy)
      &&close(cpu.radial_energy,gpu.radial_energy)
      &&close(cpu.background_energy,gpu.background_energy)
      &&close(cpu.signed_radial_poynting,gpu.signed_radial_poynting)
      &&close(cpu.primitive_face_interference,
               gpu.primitive_face_interference)
      &&close(cpu.induced_boundary_interference,
               gpu.induced_boundary_interference)
      &&close(cpu.maximum_reconstruction_residual,
               gpu.maximum_reconstruction_residual)
      &&close(cpu.actual_gauss_compatibility_residual,
               gpu.actual_gauss_compatibility_residual));
  check(label+" shell parity",cpu.shells.size()==gpu.shells.size());
  for(std::size_t i=0;i<std::min(cpu.shells.size(),gpu.shells.size());++i)
    check(label+" shell "+std::to_string(i),
        cpu.shells[i].samples==gpu.shells[i].samples
        &&close(cpu.shells[i].residual_energy,
                 gpu.shells[i].residual_energy)
        &&close(cpu.shells[i].outgoing_energy,
                 gpu.shells[i].outgoing_energy)
        &&close(cpu.shells[i].incoming_energy,
                 gpu.shells[i].incoming_energy)
        &&close(cpu.shells[i].signed_radial_poynting,
                 gpu.shells[i].signed_radial_poynting));

  const std::vector<int> supports=L==17
      ?std::vector<int>{3,4,5}:std::vector<int>{4,6,8};
  const auto cpu_ladder=observe_state_only_support_ladder(
      state,action,supports,1e-13,4096,1e-12,true);
  CudaStateOnlySupportLadderTelemetry ladder_telemetry;
  const auto gpu_ladder=observe_state_only_support_ladder_cuda(
      state,action,supports,1e-13,4096,1e-12,&ladder_telemetry,true);
  check(label+" ladder valid",cpu_ladder.valid&&gpu_ladder.valid
      &&ladder_telemetry.valid);
  check(label+" ladder chart parity",
      cpu_ladder.fractional_center_enabled
      &&gpu_ladder.fractional_center_enabled
      &&(cpu_ladder.center-gpu_ladder.center).mag()<=1e-13
      &&(cpu_ladder.support_center
          -gpu_ladder.support_center).mag()<=1e-13);
  check(label+" ladder parity",
      cpu_ladder.scales.size()==gpu_ladder.scales.size()
      &&cpu_ladder.transitions.size()==gpu_ladder.transitions.size()
      &&close(cpu_ladder.maximum_energy_reconstruction_residual,
               gpu_ladder.maximum_energy_reconstruction_residual)
      &&close(cpu_ladder.maximum_projection_residual,
               gpu_ladder.maximum_projection_residual));
  for(std::size_t i=0;i<std::min(
      cpu_ladder.scales.size(),gpu_ladder.scales.size());++i)
    check(label+" scale "+std::to_string(i),
        close(cpu_ladder.scales[i].actual_face_energy,
              gpu_ladder.scales[i].actual_face_energy)
        &&close(cpu_ladder.scales[i].bound_face_energy,
                 gpu_ladder.scales[i].bound_face_energy)
        &&close(cpu_ladder.scales[i].residual_face_energy,
                 gpu_ladder.scales[i].residual_face_energy)
        &&close(cpu_ladder.scales[i].primitive_interference,
                 gpu_ladder.scales[i].primitive_interference));
  check(label+" CUDA scalar-only transfer",
      field_telemetry.complete_field_downloads==0
      &&ladder_telemetry.complete_field_downloads==0
      &&field_telemetry.device_to_host_bytes<1024*1024
      &&ladder_telemetry.device_to_host_bytes<1024*1024);
}

bool same_observation(const StateOnlyMatterFieldObservation& lhs,
                      const StateOnlyMatterFieldObservation& rhs) {
  if(!lhs.valid||!rhs.valid||lhs.shells.size()!=rhs.shells.size())
    return false;
  bool same=close(lhs.bound_energy,rhs.bound_energy,1e-11)
      &&close(lhs.residual_energy,rhs.residual_energy,1e-11)
      &&close(lhs.outgoing_energy,rhs.outgoing_energy,1e-11)
      &&close(lhs.incoming_energy,rhs.incoming_energy,1e-11)
      &&close(lhs.radial_energy,rhs.radial_energy,1e-11)
      &&close(lhs.background_energy,rhs.background_energy,1e-11)
      &&close(lhs.primitive_face_interference,
               rhs.primitive_face_interference,1e-11)
      &&close(lhs.induced_boundary_interference,
               rhs.induced_boundary_interference,1e-11)
      &&close(lhs.signed_radial_poynting,
               rhs.signed_radial_poynting,1e-11);
  for(std::size_t i=0;i<lhs.shells.size();++i)
    same=same&&lhs.shells[i].samples==rhs.shells[i].samples
        &&close(lhs.shells[i].residual_energy,
                 rhs.shells[i].residual_energy,1e-11)
        &&close(lhs.shells[i].outgoing_energy,
                 rhs.shells[i].outgoing_energy,1e-11)
        &&close(lhs.shells[i].incoming_energy,
                 rhs.shells[i].incoming_energy,1e-11)
        &&close(lhs.shells[i].signed_radial_poynting,
                 rhs.shells[i].signed_radial_poynting,1e-11);
  return same;
}

bool same_ladder(const StateOnlySupportLadderObservation& lhs,
                 const StateOnlySupportLadderObservation& rhs) {
  if(!lhs.valid||!rhs.valid||lhs.scales.size()!=rhs.scales.size()
      ||lhs.transitions.size()!=rhs.transitions.size()) return false;
  bool same=true;
  for(std::size_t i=0;i<lhs.scales.size();++i)
    same=same&&close(lhs.scales[i].actual_face_energy,
                    rhs.scales[i].actual_face_energy,1e-11)
        &&close(lhs.scales[i].bound_face_energy,
                 rhs.scales[i].bound_face_energy,1e-11)
        &&close(lhs.scales[i].residual_face_energy,
                 rhs.scales[i].residual_face_energy,1e-11)
        &&close(lhs.scales[i].primitive_interference,
                 rhs.scales[i].primitive_interference,1e-11);
  for(std::size_t i=0;i<lhs.transitions.size();++i)
    same=same&&close(lhs.transitions[i].relaxation_energy,
                    rhs.transitions[i].relaxation_energy,1e-11)
        &&close(lhs.transitions[i].monotonicity_margin,
                 rhs.transitions[i].monotonicity_margin,1e-11);
  return same;
}

double qualify_covariance_and_seam() {
  constexpr int L=17;
  ConnectedMooreBlockOptions action;
  action.binding_law=ConnectedBindingLaw::DerivedCompactPair;
  const auto preparation=prepare_finite_support_derived_compact_pair(
      make_pair(L,{0.21,-0.17,0.29},{1,1,0},+1),
      action,4,1e-13,4096,true);
  check("covariance preparation",preparation.valid);
  if(!preparation.valid) return INFINITY;
  auto state=preparation.state;
  add_challenge(state,+1);
  StateOnlyMatterFieldObserverOptions observer;
  observer.support_half_width=4;
  observer.shell_radii={2,4,6};
  observer.allow_fractional_center=true;
  const auto base=observe_state_only_matter_field(state,action,observer);
  const auto translated=observe_state_only_matter_field(
      translate_state(state,2,-1,3),action,observer);
  const auto rotated=observe_state_only_matter_field(
      rotate_state(state),action,observer);
  const auto conjugated=observe_state_only_matter_field(
      conjugate_state(state),action,observer);
  check("fractional integer translation covariance",
      same_observation(base,translated));
  check("fractional proper cubic covariance",
      same_observation(base,rotated));
  check("fractional polarity conjugation",
      same_observation(base,conjugated));
  const std::vector<int> supports{3,4,5};
  const auto base_ladder=observe_state_only_support_ladder(
      state,action,supports,1e-13,4096,1e-12,true);
  check("fractional ladder translation covariance",same_ladder(
      base_ladder,observe_state_only_support_ladder(
          translate_state(state,2,-1,3),action,supports,
          1e-13,4096,1e-12,true)));
  check("fractional ladder proper cubic covariance",same_ladder(
      base_ladder,observe_state_only_support_ladder(
          rotate_state(state),action,supports,
          1e-13,4096,1e-12,true)));
  check("fractional ladder polarity conjugation",same_ladder(
      base_ladder,observe_state_only_support_ladder(
          conjugate_state(state),action,supports,
          1e-13,4096,1e-12,true)));

  const auto lower=prepare_finite_support_derived_compact_pair(
      make_pair(L,{0.499,0.13,-0.19},{1,0,0},+1),
      action,4,1e-13,4096,true);
  const auto upper=prepare_finite_support_derived_compact_pair(
      make_pair(L,{0.501,0.13,-0.19},{1,0,0},+1),
      action,4,1e-13,4096,true);
  check("fractional chart seam representatives",lower.valid&&upper.valid
      &&std::abs(lower.support_center.x-upper.support_center.x)==1.0);
  return lower.valid&&upper.valid
      ?std::abs(lower.electric_energy-upper.electric_energy):INFINITY;
}

}  // namespace

int main() {
  const std::array<Vec3,8> directions{{
      {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},
      {0,0,1},{0,0,-1},{1,1,0},{1,1,1}}};
  const std::array<Vec3,2> offsets{{
      {0.21,-0.17,0.29},{-0.31,0.23,-0.11}}};
  for(const int L:{17,33}) for(const int polarity:{-1,+1})
    for(std::size_t i=0;i<directions.size();++i) {
      const auto& offset=offsets[(i+static_cast<std::size_t>(polarity>0))%2];
      compare_case(L,offset,directions[i],polarity,
          "L="+std::to_string(L)+" p="+std::to_string(polarity)
          +" d="+std::to_string(i));
    }
  const double seam_difference=qualify_covariance_and_seam();
  std::cout.precision(17);
  std::cout<<"FTD-0763 seam representative energy difference="
           <<seam_difference<<'\n';
  std::cout<<"FTD-0763 fractional-center CUDA qualification failures="
           <<failures<<'\n';
  return failures==0?0:1;
}
