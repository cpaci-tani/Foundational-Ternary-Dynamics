/** FTD-0759: CUDA state-only support-ladder reduction parity. */

#include "ftd/eft/cuda_state_only_support_ladder.h"
#include "ftd/eft/matched_gauss_transport.h"

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
  const int remainder=value%L;
  return remainder<0?remainder+L:remainder;
}

ftd::eft::MatchedMatterPoint point_at(
    const ftd::Vec3& position,const ftd::Vec3& momentum,int L) {
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
      center-ftd::Vec3{0.0,0.0,0.5},{0.012,-0.004,0.015},L));
  state.constituents.push_back(point_at(
      center+ftd::Vec3{0.0,0.0,0.5},{-0.012,0.004,-0.015},L));
  state.charges={+1,-1};
  return state;
}

void add_gauss_free_plaquette(ftd::eft::ConnectedMooreBlockState& state) {
  const int L=state.electric.L;
  ftd::eft::MatchedEdgeField potential(L);
  const int c=L/2;
  potential.z[static_cast<std::size_t>(potential.index(c+7,c+5,c+3))]
      =2.5e-4;
  const auto curl=ftd::eft::matched_curl(potential);
  for(std::size_t i=0;i<state.electric.x.size();++i) {
    state.electric.x[i]+=curl.x[i];
    state.electric.y[i]+=curl.y[i];
    state.electric.z[i]+=curl.z[i];
  }
}

bool close(double a,double b,double tolerance=1e-12) {
  return std::abs(a-b)<=tolerance*std::max({1.0,std::abs(a),std::abs(b)});
}

void compare_ladder(int L) {
  ftd::eft::ConnectedMooreBlockOptions action;
  action.binding_law=ftd::eft::ConnectedBindingLaw::DerivedCompactPair;
  const auto preparation=ftd::eft::prepare_finite_support_derived_compact_pair(
      make_pair(L),action,4,1e-13,4096);
  check("preparation L="+std::to_string(L),preparation.valid);
  if(!preparation.valid) return;
  auto state=preparation.state;
  add_gauss_free_plaquette(state);
  const std::vector<int> supports{4,6,8};
  const auto cpu=ftd::eft::observe_state_only_support_ladder(
      state,action,supports,1e-13,4096,1e-12);
  ftd::eft::CudaStateOnlySupportLadderTelemetry telemetry;
  const auto gpu=ftd::eft::observe_state_only_support_ladder_cuda(
      state,action,supports,1e-13,4096,1e-12,&telemetry);
  check("CPU valid L="+std::to_string(L),cpu.valid);
  check("GPU valid L="+std::to_string(L),gpu.valid&&telemetry.valid);
  check("scope flags L="+std::to_string(L),gpu.state_only
      &&gpu.support_is_resolution_scale);
  check("shape parity L="+std::to_string(L),
      cpu.scales.size()==gpu.scales.size()
      &&cpu.transitions.size()==gpu.transitions.size());
  check("center parity L="+std::to_string(L),
      (cpu.center-gpu.center).mag()<=1e-13);
  check("maximum parity L="+std::to_string(L),
      close(cpu.maximum_energy_reconstruction_residual,
            gpu.maximum_energy_reconstruction_residual)
      &&close(cpu.maximum_projection_residual,
              gpu.maximum_projection_residual));
  for(std::size_t i=0;i<std::min(cpu.scales.size(),gpu.scales.size());++i) {
    const auto& a=cpu.scales[i];
    const auto& b=gpu.scales[i];
    check("scale parity L="+std::to_string(L)+" i="+std::to_string(i),
        a.valid==b.valid
        &&a.support_half_width==b.support_half_width
        &&close(a.actual_face_energy,b.actual_face_energy)
        &&close(a.bound_face_energy,b.bound_face_energy)
        &&close(a.residual_face_energy,b.residual_face_energy)
        &&close(a.primitive_interference,b.primitive_interference)
        &&close(a.energy_reconstruction_residual,
                 b.energy_reconstruction_residual)
        &&close(a.poisson_residual,b.poisson_residual)
        &&close(a.gauss_residual,b.gauss_residual));
  }
  for(std::size_t i=0;
      i<std::min(cpu.transitions.size(),gpu.transitions.size());++i) {
    const auto& a=cpu.transitions[i];
    const auto& b=gpu.transitions[i];
    check("transition parity L="+std::to_string(L)+" i="+std::to_string(i),
        a.valid==b.valid
        &&a.inner_half_width==b.inner_half_width
        &&a.outer_half_width==b.outer_half_width
        &&close(a.relaxation_energy,b.relaxation_energy)
        &&close(a.outer_difference_inner_product,
                 b.outer_difference_inner_product)
        &&close(a.pythagorean_residual,b.pythagorean_residual)
        &&close(a.monotonicity_margin,b.monotonicity_margin));
  }
  check("no full field downloads L="+std::to_string(L),
      telemetry.complete_field_downloads==0);
  check("partial-only download L="+std::to_string(L),
      telemetry.device_to_host_bytes<1024*1024);

  ftd::eft::CudaMatchedFieldPipeline pipeline(L);
  check("resident pipeline upload L="+std::to_string(L),
      pipeline.valid()&&pipeline.upload(state.electric,state.magnetic_half));
  const auto resident_views=pipeline.resident_views();
  check("resident before views L="+std::to_string(L),
      resident_views.electric_before.valid()
      &&resident_views.magnetic_before.valid());
  ftd::eft::ConnectedMooreBlockState matter_only;
  matter_only.electric.L=L;
  matter_only.magnetic_half.L=L;
  matter_only.constituents=state.constituents;
  matter_only.charges=state.charges;
  matter_only.edges=state.edges;
  matter_only.width=state.width;
  matter_only.orientation_axis=state.orientation_axis;
  ftd::eft::CudaStateOnlySupportLadderTelemetry resident_telemetry;
  const auto resident_gpu=
      ftd::eft::observe_state_only_support_ladder_cuda_resident(
          matter_only,action,resident_views.electric_before,supports,
          1e-13,4096,1e-12,&resident_telemetry);
  check("resident ladder valid L="+std::to_string(L),
      resident_gpu.valid&&resident_telemetry.valid);
  check("resident ladder shape L="+std::to_string(L),
      resident_gpu.scales.size()==gpu.scales.size()
      &&resident_gpu.transitions.size()==gpu.transitions.size());
  check("resident ladder scalar parity L="+std::to_string(L),
      close(resident_gpu.maximum_energy_reconstruction_residual,
            gpu.maximum_energy_reconstruction_residual)
      &&close(resident_gpu.maximum_projection_residual,
               gpu.maximum_projection_residual));
  for(std::size_t i=0;
      i<std::min(resident_gpu.scales.size(),gpu.scales.size());++i) {
    check("resident scale parity L="+std::to_string(L)
          +" i="+std::to_string(i),
        close(resident_gpu.scales[i].actual_face_energy,
              gpu.scales[i].actual_face_energy)
        &&close(resident_gpu.scales[i].bound_face_energy,
                 gpu.scales[i].bound_face_energy)
        &&close(resident_gpu.scales[i].residual_face_energy,
                 gpu.scales[i].residual_face_energy)
        &&close(resident_gpu.scales[i].primitive_interference,
                 gpu.scales[i].primitive_interference));
  }
  check("resident ladder transfer reduction L="+std::to_string(L),
      resident_telemetry.complete_field_downloads==0
      &&resident_telemetry.host_to_device_bytes
          <telemetry.host_to_device_bytes
      &&resident_telemetry.device_to_host_bytes<1024*1024);

  ftd::eft::StateOnlyMatterFieldObserverOptions observer;
  observer.support_half_width=4;
  observer.shell_radii=L==33?std::vector<int>{8,12,16}
      :std::vector<int>{8,12,16,24,32};
  const auto cpu_field=ftd::eft::observe_state_only_matter_field(
      state,action,observer);
  ftd::eft::CudaStateOnlySupportLadderTelemetry field_telemetry;
  const auto gpu_field=ftd::eft::observe_state_only_matter_field_cuda(
      state,action,observer,&field_telemetry);
  check("CPU field observer valid L="+std::to_string(L),cpu_field.valid);
  check("GPU field observer valid L="+std::to_string(L),
      gpu_field.valid&&field_telemetry.valid);
  check("field scope parity L="+std::to_string(L),
      gpu_field.state_only&&gpu_field.centered_readout_only
      &&!gpu_field.primitive_cochain_uniqueness_claimed
      &&cpu_field.boundary_energy_ledger_valid
          ==gpu_field.boundary_energy_ledger_valid);
  const auto field_scalars_close=[&]() {
    return close(cpu_field.constituent_kinetic_energy,
                 gpu_field.constituent_kinetic_energy)
        &&close(cpu_field.pair_internal_energy,gpu_field.pair_internal_energy)
        &&close(cpu_field.bound_energy,gpu_field.bound_energy)
        &&close(cpu_field.residual_energy,gpu_field.residual_energy)
        &&close(cpu_field.outgoing_energy,gpu_field.outgoing_energy)
        &&close(cpu_field.incoming_energy,gpu_field.incoming_energy)
        &&close(cpu_field.radial_energy,gpu_field.radial_energy)
        &&close(cpu_field.background_energy,gpu_field.background_energy)
        &&close(cpu_field.bound_residual_interference,
                 gpu_field.bound_residual_interference)
        &&close(cpu_field.primitive_face_interference,
                 gpu_field.primitive_face_interference)
        &&close(cpu_field.induced_boundary_interference,
                 gpu_field.induced_boundary_interference)
        &&close(cpu_field.centered_electric_interference,
                 gpu_field.centered_electric_interference)
        &&close(cpu_field.centered_magnetic_interference,
                 gpu_field.centered_magnetic_interference)
        &&close(cpu_field.centering_metric_interference,
                 gpu_field.centering_metric_interference)
        &&close(cpu_field.boundary_flux_sum,gpu_field.boundary_flux_sum)
        &&close(cpu_field.primitive_boundary_identity_residual,
                 gpu_field.primitive_boundary_identity_residual)
        &&close(cpu_field.readout_interference_reconstruction_residual,
                 gpu_field.readout_interference_reconstruction_residual)
        &&close(cpu_field.signed_radial_poynting,
                 gpu_field.signed_radial_poynting)
        &&close(cpu_field.maximum_reconstruction_residual,
                 gpu_field.maximum_reconstruction_residual)
        &&close(cpu_field.actual_gauss_compatibility_residual,
                 gpu_field.actual_gauss_compatibility_residual)
        &&close(cpu_field.energy_partition_residual,
                 gpu_field.energy_partition_residual)
        &&close(cpu_field.characteristic_flux_residual,
                 gpu_field.characteristic_flux_residual);
  };
  check("field scalar parity L="+std::to_string(L),field_scalars_close());
  check("shell count parity L="+std::to_string(L),
      cpu_field.shells.size()==gpu_field.shells.size());
  for(std::size_t i=0;
      i<std::min(cpu_field.shells.size(),gpu_field.shells.size());++i) {
    const auto& a=cpu_field.shells[i];
    const auto& b=gpu_field.shells[i];
    check("shell parity L="+std::to_string(L)+" i="+std::to_string(i),
        a.radius==b.radius&&a.samples==b.samples
        &&close(a.residual_energy,b.residual_energy)
        &&close(a.outgoing_energy,b.outgoing_energy)
        &&close(a.incoming_energy,b.incoming_energy)
        &&close(a.radial_energy,b.radial_energy)
        &&close(a.background_energy,b.background_energy)
        &&close(a.signed_radial_poynting,b.signed_radial_poynting));
  }
  check("field observer no full downloads L="+std::to_string(L),
      field_telemetry.complete_field_downloads==0
      &&field_telemetry.device_to_host_bytes<1024*1024);

  ftd::eft::CudaStateOnlySupportLadderTelemetry resident_field_telemetry;
  const auto resident_gpu_field=
      ftd::eft::observe_state_only_matter_field_cuda_resident(
          matter_only,action,resident_views.electric_before,
          resident_views.magnetic_before,observer,
          &resident_field_telemetry);
  check("resident field observer valid L="+std::to_string(L),
      resident_gpu_field.valid&&resident_field_telemetry.valid);
  check("resident field scalar parity L="+std::to_string(L),
      close(resident_gpu_field.bound_energy,gpu_field.bound_energy)
      &&close(resident_gpu_field.residual_energy,gpu_field.residual_energy)
      &&close(resident_gpu_field.outgoing_energy,gpu_field.outgoing_energy)
      &&close(resident_gpu_field.incoming_energy,gpu_field.incoming_energy)
      &&close(resident_gpu_field.radial_energy,gpu_field.radial_energy)
      &&close(resident_gpu_field.background_energy,
               gpu_field.background_energy)
      &&close(resident_gpu_field.boundary_flux_sum,
               gpu_field.boundary_flux_sum)
      &&close(resident_gpu_field.maximum_reconstruction_residual,
               gpu_field.maximum_reconstruction_residual)
      &&close(resident_gpu_field.actual_gauss_compatibility_residual,
               gpu_field.actual_gauss_compatibility_residual));
  check("resident field shell shape L="+std::to_string(L),
      resident_gpu_field.shells.size()==gpu_field.shells.size());
  for(std::size_t i=0;i<std::min(
      resident_gpu_field.shells.size(),gpu_field.shells.size());++i) {
    check("resident shell parity L="+std::to_string(L)
          +" i="+std::to_string(i),
        close(resident_gpu_field.shells[i].residual_energy,
              gpu_field.shells[i].residual_energy)
        &&close(resident_gpu_field.shells[i].outgoing_energy,
                 gpu_field.shells[i].outgoing_energy)
        &&close(resident_gpu_field.shells[i].background_energy,
                 gpu_field.shells[i].background_energy)
        &&close(resident_gpu_field.shells[i].signed_radial_poynting,
                 gpu_field.shells[i].signed_radial_poynting));
  }
  check("resident field transfer reduction L="+std::to_string(L),
      resident_field_telemetry.complete_field_downloads==0
      &&resident_field_telemetry.host_to_device_bytes
          <field_telemetry.host_to_device_bytes
      &&resident_field_telemetry.device_to_host_bytes<1024*1024);
}

}  // namespace

int main() {
  compare_ladder(33);
  compare_ladder(65);
  if(failures==0) {
    std::cout<<"FTD-0759 CUDA support ladder parity: PASS\n";
    return 0;
  }
  std::cerr<<failures<<" CUDA support ladder parity checks failed\n";
  return 1;
}
