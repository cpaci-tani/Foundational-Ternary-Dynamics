/** FTD-0768: paired field-response and regional-ledger CPU/CUDA qualification. */

#define main ftd0763_fractional_observer_reference_main
#include "test_cuda_fractional_center_state_only_observer.cpp"
#undef main

#include "ftd/eft/cuda_matched_field_pipeline.h"
#include "ftd/eft/cuda_paired_field_response.h"
#include "ftd/eft/paired_field_response.h"

#include <array>
#include <iomanip>
#include <utility>

namespace {

using namespace ftd;
using namespace ftd::eft;

bool response_close(double lhs,double rhs,double tolerance=1e-11) {
  return std::abs(lhs-rhs)
      <=tolerance*std::max({1.0,std::abs(lhs),std::abs(rhs)});
}

double periodic_coordinate_delta(double coordinate,double center,int L) {
  double result=coordinate-center;
  const double half=0.5*static_cast<double>(L);
  if(result>half) result-=static_cast<double>(L);
  if(result<-half) result+=static_cast<double>(L);
  return result;
}

Vec3 component_position(int family,int axis,int x,int y,int z) {
  if(family==0) {
    if(axis==0) return {x+0.5,static_cast<double>(y),static_cast<double>(z)};
    if(axis==1) return {static_cast<double>(x),y+0.5,static_cast<double>(z)};
    return {static_cast<double>(x),static_cast<double>(y),z+0.5};
  }
  if(axis==0) return {static_cast<double>(x),y+0.5,z+0.5};
  if(axis==1) return {x+0.5,static_cast<double>(y),z+0.5};
  return {x+0.5,y+0.5,static_cast<double>(z)};
}

Vec3 region_delta(const Vec3& position,const Vec3& center,int L) {
  return {periodic_coordinate_delta(position.x,center.x,L),
          periodic_coordinate_delta(position.y,center.y,L),
          periodic_coordinate_delta(position.z,center.z,L)};
}

bool region_contains_direct(const FieldResponseRegionSpec& region,
                            const Vec3& position,int L) {
  const auto delta=region_delta(position,region.center,L);
  if(region.kind==FieldResponseRegionKind::ChebyshevCube)
    return std::max({std::abs(delta.x),std::abs(delta.y),
                     std::abs(delta.z)})<=region.chebyshev_radius;
  return std::abs(delta.dot(region.longitudinal))
          <=region.longitudinal_half_width
      &&std::abs(delta.dot(region.transverse_u))
          <=region.transverse_half_width
      &&std::abs(delta.dot(region.transverse_v))
          <=region.transverse_half_width;
}

const std::vector<double>& face_component(
    const MatchedFaceFlux& field,int axis) {
  return axis==0?field.x:(axis==1?field.y:field.z);
}

const std::vector<double>& edge_component(
    const MatchedEdgeField& field,int axis) {
  return axis==0?field.x:(axis==1?field.y:field.z);
}

struct DirectDifference {
  double energy=0.0;
  double first_moment=0.0;
};

DirectDifference direct_difference_field(
    const ConnectedMooreBlockState& moving,
    const ConnectedMooreBlockState& rest,
    const FieldResponseRegionSpec& region,
    const PairedFieldResponseOptions& options) {
  DirectDifference result;
  const int L=moving.electric.L;
  const auto moving_b=matched_integer_time_magnetic(
      moving.electric,moving.magnetic_half,options.wave_speed,options.dt);
  const auto rest_b=matched_integer_time_magnetic(
      rest.electric,rest.magnetic_half,options.wave_speed,options.dt);
  for(int x=0;x<L;++x) for(int y=0;y<L;++y) for(int z=0;z<L;++z) {
    const auto index=static_cast<std::size_t>(moving.electric.index(x,y,z));
    for(int family=0;family<2;++family) for(int axis=0;axis<3;++axis) {
      const auto position=component_position(family,axis,x,y,z);
      if(!region_contains_direct(region,position,L)) continue;
      const double moving_value=family==0
          ?face_component(moving.electric,axis)[index]
          :edge_component(moving_b,axis)[index];
      const double rest_value=family==0
          ?face_component(rest.electric,axis)[index]
          :edge_component(rest_b,axis)[index];
      const double energy=0.5*(moving_value-rest_value)
          *(moving_value-rest_value);
      const double longitudinal=region_delta(position,region.center,L)
          .dot(region.longitudinal);
      result.energy+=energy;
      result.first_moment+=energy*longitudinal;
    }
  }
  return result;
}

void compare_channel(const std::string& label,
                     const QuadraticFieldDifferenceChannel& cpu,
                     const QuadraticFieldDifferenceChannel& gpu) {
  check(label+" moving",response_close(cpu.moving_energy,gpu.moving_energy));
  check(label+" rest",response_close(cpu.rest_energy,gpu.rest_energy));
  check(label+" difference",
      response_close(cpu.energy_difference,gpu.energy_difference));
  check(label+" norm",response_close(
      cpu.difference_field_energy,gpu.difference_field_energy));
  check(label+" cross",response_close(cpu.cross_energy,gpu.cross_energy));
  check(label+" identity",std::abs(cpu.energy_identity_residual)<=1e-12
      &&std::abs(gpu.energy_identity_residual)<=1e-12);
  check(label+" difference moment",response_close(
      cpu.energy_difference_first_moment,
      gpu.energy_difference_first_moment));
  check(label+" norm moment",response_close(
      cpu.difference_field_first_moment,
      gpu.difference_field_first_moment));
  check(label+" cross moment",response_close(
      cpu.cross_first_moment,gpu.cross_first_moment));
}

void compare_response(const std::string& label,
                      const PairedFieldResponseObservation& cpu,
                      const PairedFieldResponseObservation& gpu,
                      const CudaPairedFieldResponseTelemetry& telemetry) {
  check(label+" valid",cpu.valid&&gpu.valid&&telemetry.valid);
  check(label+" scalar-only",telemetry.complete_field_downloads==0
      &&telemetry.device_to_host_bytes>0
      &&telemetry.device_to_host_bytes<1024*1024);
  check(label+" bound centers",
      (cpu.moving_bound_center-gpu.moving_bound_center).mag()<=1e-13
      &&(cpu.rest_bound_center-gpu.rest_bound_center).mag()<=1e-13);
  check(label+" bound Gauss",response_close(
      cpu.moving_bound_gauss_residual,gpu.moving_bound_gauss_residual,1e-12)
      &&response_close(cpu.rest_bound_gauss_residual,
                        gpu.rest_bound_gauss_residual,1e-12));
  for(std::size_t i=0;i<cpu.regions.size();++i) {
    compare_channel(label+" region "+std::to_string(i)+" actual",
                    cpu.regions[i].actual,gpu.regions[i].actual);
    compare_channel(label+" region "+std::to_string(i)+" residual",
                    cpu.regions[i].residual,gpu.regions[i].residual);
  }
}

void compare_symmetry(const std::string& label,
                      const PairedFieldResponseObservation& lhs,
                      const PairedFieldResponseObservation& rhs) {
  check(label+" valid",lhs.valid&&rhs.valid);
  for(std::size_t i=0;i<lhs.regions.size();++i) {
    const auto compare=[&](const std::string& suffix,
        const QuadraticFieldDifferenceChannel& a,
        const QuadraticFieldDifferenceChannel& b) {
      check(label+" "+suffix,response_close(a.moving_energy,b.moving_energy)
          &&response_close(a.rest_energy,b.rest_energy)
          &&response_close(a.energy_difference,b.energy_difference)
          &&response_close(a.difference_field_energy,
                            b.difference_field_energy)
          &&response_close(a.cross_energy,b.cross_energy)
          &&response_close(a.energy_difference_first_moment,
                            b.energy_difference_first_moment)
          &&response_close(a.difference_field_first_moment,
                            b.difference_field_first_moment)
          &&response_close(a.cross_first_moment,b.cross_first_moment));
    };
    compare("actual "+std::to_string(i),
            lhs.regions[i].actual,rhs.regions[i].actual);
    compare("residual "+std::to_string(i),
            lhs.regions[i].residual,rhs.regions[i].residual);
  }
}

PairedFieldResponseOptions response_options(
    int L,const Vec3& center,const Vec3& direction) {
  PairedFieldResponseOptions result;
  result.laboratory_center=center;
  result.moving_center=center;
  result.longitudinal=direction*(1.0/direction.mag());
  if(std::abs(result.longitudinal.z)>0.9) {
    result.transverse_u={1,0,0}; result.transverse_v={0,1,0};
  } else if(std::abs(result.longitudinal.x)>0.9) {
    result.transverse_u={0,1,0}; result.transverse_v={0,0,1};
  } else {
    result.transverse_u={0,0,1}; result.transverse_v={1,0,0};
  }
  result.near_radius=L==17?6:8;
  result.outer_radius=L==17?8:12;
  return result;
}

void add_polynomial_field(ConnectedMooreBlockState& state,int fixture,
                          double scale) {
  const int L=state.electric.L;
  const double center=0.5*static_cast<double>(L-1);
  for(int x=0;x<L;++x) for(int y=0;y<L;++y) for(int z=0;z<L;++z) {
    const auto index=static_cast<std::size_t>(state.electric.index(x,y,z));
    const double u=(static_cast<double>(x)-center)/L;
    const double v=(static_cast<double>(y)-center)/L;
    const double w=(static_cast<double>(z)-center)/L;
    const double affine=0.75*u-0.50*v+0.25*w;
    const double quadratic=u*u-0.60*v*v+0.35*w*w+0.40*u*v;
    const double mixed=affine+quadratic+0.30*v*w-0.20*w*u;
    const double p=fixture==0?affine:(fixture==1?quadratic:mixed);
    const double q=fixture==0?(-0.25*u+0.55*v+0.35*w)
        :(fixture==1?(0.45*u*u+0.20*v*v-0.30*w*w-0.25*u*w)
                    :(0.50*affine-0.35*quadratic+0.20*u*w));
    state.electric.x[index]+=scale*p;
    state.electric.y[index]+=scale*(0.40*p-0.30*q);
    state.electric.z[index]+=scale*(-0.20*p+0.60*q);
    state.magnetic_half.x[index]+=scale*(0.35*p+0.15*q);
    state.magnetic_half.y[index]+=scale*(-0.45*p+0.25*q);
    state.magnetic_half.z[index]+=scale*(0.20*p-0.55*q);
  }
}

void qualify_polynomial_fixtures(
    const std::string& label,const ConnectedMooreBlockState& prepared,
    const ConnectedMooreBlockOptions& action,
    const PairedFieldResponseOptions& options) {
  const std::array<std::string,3> names{"affine","quadratic","mixed"};
  for(int fixture=0;fixture<3;++fixture) {
    auto rest=prepared;
    add_polynomial_field(rest,0,1.0e-5);
    auto moving=rest;
    add_polynomial_field(moving,fixture,2.0e-5);
    const auto cpu=observe_paired_field_response(
        moving,rest,action,options);
    CudaPairedFieldResponseTelemetry telemetry;
    const auto gpu=observe_paired_field_response_cuda(
        moving,rest,action,options,&telemetry);
    const std::string fixture_label=label+" "+names[fixture];
    compare_response(fixture_label,cpu,gpu,telemetry);
    for(std::size_t i=0;i<cpu.regions.size();++i) {
      for(const auto* channel:{&cpu.regions[i].actual,
                               &cpu.regions[i].residual}) {
        const double scale=std::max({1.0,std::abs(channel->moving_energy),
                                    std::abs(channel->rest_energy)});
        check(fixture_label+" quadratic identity "+std::to_string(i),
            std::abs(channel->energy_identity_residual)<=1e-12*scale);
      }
    }
  }
}

void qualify_zero_and_fixtures(int L,int polarity) {
  ConnectedMooreBlockOptions action;
  action.binding_law=ConnectedBindingLaw::DerivedCompactPair;
  const Vec3 offset=polarity>0?Vec3{0.21,-0.17,0.29}
      :Vec3{-0.31,0.23,-0.11};
  const auto preparation=prepare_finite_support_derived_compact_pair(
      make_pair(L,offset,{0,0,1},polarity),action,4,1e-13,4096,true);
  const std::string label="L="+std::to_string(L)
      +" p="+std::to_string(polarity);
  check(label+" preparation",preparation.valid);
  if(!preparation.valid) return;
  const auto options=response_options(L,preparation.center,{0,0,1});

  const auto identical_cpu=observe_paired_field_response(
      preparation.state,preparation.state,action,options);
  CudaPairedFieldResponseTelemetry identical_telemetry;
  const auto identical_gpu=observe_paired_field_response_cuda(
      preparation.state,preparation.state,action,options,
      &identical_telemetry);
  compare_response(label+" identical",identical_cpu,identical_gpu,
                   identical_telemetry);
  for(std::size_t i=0;i<identical_cpu.regions.size();++i) {
    check(label+" zero actual "+std::to_string(i),
        identical_cpu.regions[i].actual.energy_difference==0.0
        &&identical_cpu.regions[i].actual.difference_field_energy==0.0
        &&identical_cpu.regions[i].actual.cross_energy==0.0);
    check(label+" zero residual "+std::to_string(i),
        identical_cpu.regions[i].residual.energy_difference==0.0
        &&identical_cpu.regions[i].residual.difference_field_energy==0.0
        &&identical_cpu.regions[i].residual.cross_energy==0.0);
  }

  auto electric_moving=preparation.state;
  const int c=L/2;
  const auto electric_index=static_cast<std::size_t>(
      electric_moving.electric.index(c,c,c));
  constexpr double electric_delta=2.0e-4;
  electric_moving.electric.x[electric_index]+=electric_delta;
  const auto electric_cpu=observe_paired_field_response(
      electric_moving,preparation.state,action,options);
  CudaPairedFieldResponseTelemetry electric_telemetry;
  const auto electric_gpu=observe_paired_field_response_cuda(
      electric_moving,preparation.state,action,options,&electric_telemetry);
  compare_response(label+" one-face",electric_cpu,electric_gpu,
                   electric_telemetry);
  for(std::size_t i=0;i<electric_cpu.regions.size();++i) {
    const auto direct=direct_difference_field(
        electric_moving,preparation.state,electric_cpu.regions[i].spec,
        options);
    check(label+" one-face direct norm "+std::to_string(i),
        direct.energy>0.0&&response_close(
            electric_cpu.regions[i].actual.difference_field_energy,
            direct.energy,1e-12)
        &&response_close(
            electric_cpu.regions[i].residual.difference_field_energy,
            direct.energy,1e-12));
    check(label+" one-face direct moment "+std::to_string(i),
        response_close(
            electric_cpu.regions[i].actual.difference_field_first_moment,
            direct.first_moment,1e-12));
  }

  auto magnetic_moving=preparation.state;
  const auto magnetic_index=static_cast<std::size_t>(
      magnetic_moving.magnetic_half.index(c,c,c));
  constexpr double magnetic_delta=-3.0e-4;
  magnetic_moving.magnetic_half.z[magnetic_index]+=magnetic_delta;
  const auto magnetic_cpu=observe_paired_field_response(
      magnetic_moving,preparation.state,action,options);
  CudaPairedFieldResponseTelemetry magnetic_telemetry;
  const auto magnetic_gpu=observe_paired_field_response_cuda(
      magnetic_moving,preparation.state,action,options,&magnetic_telemetry);
  compare_response(label+" one-edge",magnetic_cpu,magnetic_gpu,
                   magnetic_telemetry);
  for(std::size_t i=0;i<magnetic_cpu.regions.size();++i) {
    const auto direct=direct_difference_field(
        magnetic_moving,preparation.state,magnetic_cpu.regions[i].spec,
        options);
    check(label+" one-edge exact norm "+std::to_string(i),
        response_close(direct.energy,0.5*magnetic_delta*magnetic_delta,1e-14)
        &&response_close(
            magnetic_cpu.regions[i].actual.difference_field_energy,
            direct.energy,1e-12)
        &&response_close(
            magnetic_cpu.regions[i].residual.difference_field_energy,
            direct.energy,1e-12));
  }
  qualify_polynomial_fixtures(
      label,preparation.state,action,options);
}

PairedFieldResponseOptions translate_options(
    PairedFieldResponseOptions options,const Vec3& shift) {
  options.laboratory_center+=shift;
  options.moving_center+=shift;
  return options;
}

PairedFieldResponseOptions rotate_options(
    PairedFieldResponseOptions options) {
  options.laboratory_center=rotate(options.laboratory_center);
  options.moving_center=rotate(options.moving_center);
  options.longitudinal=rotate(options.longitudinal);
  options.transverse_u=rotate(options.transverse_u);
  options.transverse_v=rotate(options.transverse_v);
  return options;
}

Vec3 reflect_z(const Vec3& value) {
  return {value.x,value.y,-value.z};
}

ConnectedMooreBlockState reflect_z_state(
    const ConnectedMooreBlockState& state) {
  const int L=state.electric.L;
  ConnectedMooreBlockState result(L);
  result.constituents=state.constituents;
  result.charges=state.charges;
  result.edges=state.edges;
  result.width=state.width;
  result.orientation_axis=state.orientation_axis;
  for(auto& point:result.constituents) {
    const Vec3 position{static_cast<double>(point.anchor.x)+point.remainder.x,
        static_cast<double>(point.anchor.y)+point.remainder.y,
        static_cast<double>(point.anchor.z)+point.remainder.z};
    point=point_at({position.x,position.y,-position.z},
        reflect_z(point.momentum),L);
  }
  for(int x=0;x<L;++x) for(int y=0;y<L;++y) for(int z=0;z<L;++z) {
    const auto old=static_cast<std::size_t>(state.electric.index(x,y,z));
    const auto face_xy=static_cast<std::size_t>(result.electric.index(x,y,-z));
    const auto face_z=static_cast<std::size_t>(result.electric.index(x,y,-z-1));
    result.electric.x[face_xy]=state.electric.x[old];
    result.electric.y[face_xy]=state.electric.y[old];
    result.electric.z[face_z]=-state.electric.z[old];
    const auto edge_xy=static_cast<std::size_t>(
        result.magnetic_half.index(x,y,-z-1));
    const auto edge_z=static_cast<std::size_t>(
        result.magnetic_half.index(x,y,-z));
    result.magnetic_half.x[edge_xy]=-state.magnetic_half.x[old];
    result.magnetic_half.y[edge_xy]=-state.magnetic_half.y[old];
    result.magnetic_half.z[edge_z]=state.magnetic_half.z[old];
  }
  return result;
}

PairedFieldResponseOptions reflect_z_options(
    PairedFieldResponseOptions options,int L) {
  options.laboratory_center.z=static_cast<double>(L)
      -options.laboratory_center.z;
  options.moving_center.z=static_cast<double>(L)-options.moving_center.z;
  options.longitudinal=reflect_z(options.longitudinal);
  options.transverse_u=reflect_z(options.transverse_u);
  options.transverse_v=reflect_z(options.transverse_v);
  return options;
}

void qualify_covariance() {
  constexpr int L=17;
  ConnectedMooreBlockOptions action;
  action.binding_law=ConnectedBindingLaw::DerivedCompactPair;
  const auto preparation=prepare_finite_support_derived_compact_pair(
      make_pair(L,{0.21,-0.17,0.29},{0,0,1},+1),
      action,4,1e-13,4096,true);
  check("covariance preparation",preparation.valid);
  if(!preparation.valid) return;
  auto rest=preparation.state;
  auto moving=rest;
  add_challenge(rest,+1);
  moving=rest;
  const int c=L/2;
  const auto index=static_cast<std::size_t>(moving.electric.index(c,c,c));
  moving.electric.x[index]+=2e-4;
  moving.magnetic_half.z[index]-=3e-4;
  const auto options=response_options(L,preparation.center,{0,0,1});
  const auto baseline=observe_paired_field_response(
      moving,rest,action,options);

  const Vec3 shift{2,-1,3};
  const auto translated=observe_paired_field_response(
      translate_state(moving,2,-1,3),translate_state(rest,2,-1,3),action,
      translate_options(options,shift));
  compare_symmetry("integer translation",baseline,translated);

  const auto conjugated=observe_paired_field_response(
      conjugate_state(moving),conjugate_state(rest),action,options);
  compare_symmetry("charge conjugation",baseline,conjugated);

  const auto rotated=observe_paired_field_response(
      rotate_state(moving),rotate_state(rest),action,rotate_options(options));
  compare_symmetry("proper cubic rotation",baseline,rotated);

  const auto reflected=observe_paired_field_response(
      reflect_z_state(moving),reflect_z_state(rest),action,
      reflect_z_options(options,L));
  compare_symmetry("reflected signed pair",baseline,reflected);
}

QuadraticCoatFaceCurrent sparse_segment(
    int L,const QuadraticCoatSparseCurrentEntry& entry) {
  QuadraticCoatFaceCurrent result;
  result.L=L;
  result.sparse_current={entry};
  result.dense_materialized=false;
  result.valid=true;
  return result;
}

void qualify_resident_transport() {
  constexpr int L=17;
  constexpr double lambda=0.25*C_SPEED;
  ConnectedMooreBlockOptions action;
  action.binding_law=ConnectedBindingLaw::DerivedCompactPair;
  const auto preparation=prepare_finite_support_derived_compact_pair(
      make_pair(L,{0.21,-0.17,0.29},{0,0,1},+1),
      action,4,1e-13,4096,true);
  check("transport preparation",preparation.valid);
  if(!preparation.valid) return;
  auto state=preparation.state;
  add_challenge(state,+1);
  auto options=response_options(L,preparation.center,{0,0,1});
  const auto region=make_ftd0768_response_regions(options)[0];

  CudaMatchedFieldPipeline pipeline(L);
  check("transport pipeline",pipeline.valid()
      &&pipeline.upload(state.electric,state.magnetic_half)
      &&pipeline.prepare_forward(lambda));
  MatchedEdgeField magnetic_after;
  MatchedFaceFlux electric_pre;
  check("transport prepared download",
      pipeline.download_prepared(magnetic_after,electric_pre));
  const int c=L/2;
  const std::vector<QuadraticCoatFaceCurrent> segments{
      sparse_segment(L,{{c,c,c},0,2.5e-4})};
  constexpr double scale=0.75;
  check("transport current",pipeline.apply_sparse_current(segments,scale));
  const auto views=pipeline.resident_views();
  CudaPairedFieldResponseTelemetry telemetry;
  const auto gpu=observe_regional_modified_energy_transport_cuda(
      views,lambda,region,1e-12,&telemetry);
  MatchedFaceFlux electric_after;
  MatchedEdgeField magnetic_download;
  check("transport after download",
      pipeline.download_after(electric_after,magnetic_download));
  const auto cpu=observe_regional_modified_energy_transport(
      state.electric,state.magnetic_half,electric_pre,magnetic_after,
      electric_after,lambda,region,1e-12);
  check("transport observers valid",cpu.valid&&gpu.valid&&telemetry.valid);
  check("transport scalar-only",telemetry.complete_field_downloads==0
      &&telemetry.host_to_device_bytes==0
      &&telemetry.device_to_host_bytes>0
      &&telemetry.device_to_host_bytes<64*1024);
  check("transport CPU/CUDA parity",
      response_close(cpu.energy_before,gpu.energy_before)
      &&response_close(cpu.energy_pre_current,gpu.energy_pre_current)
      &&response_close(cpu.energy_after,gpu.energy_after)
      &&response_close(cpu.outside_energy_before,
                        gpu.outside_energy_before)
      &&response_close(cpu.outside_energy_pre_current,
                        gpu.outside_energy_pre_current)
      &&response_close(cpu.outside_energy_after,
                        gpu.outside_energy_after)
      &&response_close(cpu.boundary_transport_into,
                        gpu.boundary_transport_into)
      &&response_close(cpu.boundary_transport_into_complement,
                        gpu.boundary_transport_into_complement)
      &&response_close(cpu.source_exchange_into_field,
                        gpu.source_exchange_into_field)
      &&response_close(cpu.energy_change,gpu.energy_change)
      &&response_close(cpu.global_source_free_residual,
                        gpu.global_source_free_residual)
      &&response_close(cpu.boundary_quadrature_residual,
                        gpu.boundary_quadrature_residual));
  check("transport nontrivial channels",
      std::abs(cpu.boundary_transport_into)>1e-16
      &&std::abs(cpu.source_exchange_into_field)>1e-16);
  check("transport oriented complement",
      cpu.boundary_transport_into
          *cpu.boundary_transport_into_complement<0.0
      &&gpu.boundary_transport_into
          *gpu.boundary_transport_into_complement<0.0);
  check("transport independent boundary quadrature",
      std::abs(cpu.global_source_free_residual)<=1e-12
      &&std::abs(gpu.global_source_free_residual)<=1e-12
      &&std::abs(cpu.boundary_quadrature_residual)<=1e-12
      &&std::abs(gpu.boundary_quadrature_residual)<=1e-12);
  check("transport ledger",std::abs(cpu.ledger_residual)<=1e-12
      &&std::abs(gpu.ledger_residual)<=1e-12);

  auto shifted_region=region;
  shifted_region.center.z+=1.0;
  CudaPairedFieldResponseTelemetry shifted_telemetry;
  const auto gpu_shifted=observe_regional_modified_energy_transport_cuda(
      views,lambda,shifted_region,1e-12,&shifted_telemetry);
  const auto cpu_shifted=observe_regional_modified_energy_transport(
      state.electric,state.magnetic_half,electric_pre,magnetic_after,
      electric_after,lambda,shifted_region,1e-12);
  const auto cpu_transport=derive_regional_control_volume_transport(
      cpu,cpu_shifted,1e-12);
  const auto gpu_transport=derive_regional_control_volume_transport(
      gpu,gpu_shifted,1e-12);
  check("moving-control-volume observers valid",
      cpu_shifted.valid&&gpu_shifted.valid&&shifted_telemetry.valid
      &&cpu_transport.valid&&gpu_transport.valid);
  check("moving-control-volume scalar-only",
      shifted_telemetry.complete_field_downloads==0
      &&shifted_telemetry.host_to_device_bytes==0
      &&shifted_telemetry.device_to_host_bytes>0
      &&shifted_telemetry.device_to_host_bytes<64*1024);
  check("moving-control-volume nonzero sweep",
      std::abs(cpu_transport.mask_sweep_into)>1e-16
      &&std::abs(gpu_transport.mask_sweep_into)>1e-16);
  check("moving-control-volume CPU/CUDA parity",
      response_close(cpu_transport.mask_sweep_into,
                     gpu_transport.mask_sweep_into)
      &&response_close(cpu_transport.mask_sweep_into_complement,
                        gpu_transport.mask_sweep_into_complement)
      &&response_close(cpu_transport.transported_energy_change,
                        gpu_transport.transported_energy_change)
      &&response_close(cpu_transport.transport_identity_residual,
                        gpu_transport.transport_identity_residual));
  check("moving-control-volume complementary sweep",
      std::abs(cpu_transport.mask_sweep_quadrature_residual)<=1e-12
      &&std::abs(gpu_transport.mask_sweep_quadrature_residual)<=1e-12);
  check("moving-control-volume exact Reynolds identity",
      std::abs(cpu_transport.transport_identity_residual)<=1e-12
      &&std::abs(gpu_transport.transport_identity_residual)<=1e-12
      &&response_close(
          cpu_transport.transported_energy_change,
          cpu_shifted.energy_change+cpu_transport.mask_sweep_into));
}

}  // namespace

int main() {
  for(const int L:{17,33}) for(const int polarity:{-1,+1})
    qualify_zero_and_fixtures(L,polarity);
  qualify_covariance();
  qualify_resident_transport();
  std::cout<<std::setprecision(17)
      <<"FTD-0768 paired-response CUDA qualification failures="
      <<failures<<'\n';
  return failures==0?0:1;
}
