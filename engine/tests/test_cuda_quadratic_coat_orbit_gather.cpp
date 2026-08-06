/** FTD-0759: resident CUDA quadratic-coat orbit-gather parity. */

#include "ftd/eft/cuda_matched_field_pipeline.h"
#include "ftd/eft/cuda_quadratic_coat_orbit_gather.h"
#include "ftd/eft/cuda_state_only_support_ladder.h"
#include "ftd/eft/connected_moore_block_action.h"
#include "ftd/eft/quadratic_coat_face_current.h"
#include "ftd/eft/quadratic_coat_orbit_gather.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures=0;
std::string active_context;

void check(const std::string& label,bool condition) {
  if(condition) return;
  ++failures;
  std::cerr<<"FAIL: "<<label;
  if(!active_context.empty()) std::cerr<<" ["<<active_context<<']';
  std::cerr<<'\n';
}

bool close(double a,double b,double tolerance=1e-12) {
  return std::abs(a-b)<=tolerance*std::max({1.0,std::abs(a),std::abs(b)});
}

bool close(const ftd::Vec3& a,const ftd::Vec3& b,double tolerance=1e-12) {
  return close(a.x,b.x,tolerance)&&close(a.y,b.y,tolerance)
      &&close(a.z,b.z,tolerance);
}

bool same_coord(const ftd::Coord& a,const ftd::Coord& b) {
  return a.x==b.x&&a.y==b.y&&a.z==b.z;
}

bool same_ordered_segments(
    const std::vector<ftd::eft::QuadraticCoatFaceCurrent>& a,
    const std::vector<ftd::eft::QuadraticCoatFaceCurrent>& b) {
  if(a.size()!=b.size()) return false;
  for(std::size_t segment=0;segment<a.size();++segment) {
    const auto& left=a[segment];
    const auto& right=b[segment];
    if(left.L!=right.L||left.charge!=right.charge
        ||left.sparse_current.size()!=right.sparse_current.size()
        ||!close(left.start_effective_position,
                  right.start_effective_position,1e-13)
        ||!close(left.end_effective_position,
                  right.end_effective_position,1e-13)) return false;
    for(std::size_t entry=0;entry<left.sparse_current.size();++entry) {
      const auto& x=left.sparse_current[entry];
      const auto& y=right.sparse_current[entry];
      if(x.axis!=y.axis||!same_coord(x.face,y.face)
          ||x.value!=y.value) return false;
    }
  }
  return true;
}

ftd::Vec3 effective_position(const ftd::eft::MatchedMatterPoint& point) {
  return {point.anchor.x+point.remainder.x,
          point.anchor.y+point.remainder.y,
          point.anchor.z+point.remainder.z};
}

void seed_fields(int L,ftd::eft::MatchedFaceFlux& electric,
                 ftd::eft::MatchedEdgeField& magnetic) {
  const int c=L/2;
  for(int dx=-5;dx<=5;++dx)
    for(int dy=-5;dy<=5;++dy)
      for(int dz=-5;dz<=5;++dz) {
        const auto item=static_cast<std::size_t>(electric.index(
            c+dx,c+dy,c+dz));
        const double radius=1.0+dx*dx+dy*dy+dz*dz;
        electric.x[item]=2.0e-4*(1.0+0.07*dy)/radius;
        electric.y[item]=-1.3e-4*(1.0-0.05*dz)/radius;
        electric.z[item]=1.7e-4*(1.0+0.03*dx)/radius;
        magnetic.x[item]=-0.9e-4*(1.0+0.02*dz)/radius;
        magnetic.y[item]=1.1e-4*(1.0-0.04*dx)/radius;
        magnetic.z[item]=0.8e-4*(1.0+0.06*dy)/radius;
      }
}

void compare_volume(int L) {
  ftd::eft::MatchedFaceFlux electric(L);
  ftd::eft::MatchedEdgeField magnetic(L);
  seed_fields(L,electric,magnetic);
  ftd::eft::CudaMatchedFieldPipeline pipeline(L);
  check("pipeline valid L="+std::to_string(L),pipeline.valid());
  if(!pipeline.valid()) return;
  constexpr double dt=0.25;
  const double lambda=ftd::C_SPEED*dt;
  check("upload L="+std::to_string(L),pipeline.upload(electric,magnetic));
  check("prepare L="+std::to_string(L),pipeline.prepare_forward(lambda));
  const auto views=pipeline.resident_views();
  check("resident views L="+std::to_string(L),views.prepared
      &&views.electric_before.valid()
      &&views.electric_pre_current.valid()
      &&views.magnetic_prepared.valid());

  ftd::eft::MatchedFaceFlux prepared_electric(L);
  ftd::eft::MatchedEdgeField prepared_magnetic(L);
  check("parity checkpoint download L="+std::to_string(L),
      pipeline.download_prepared(prepared_magnetic,prepared_electric));

  const double c=static_cast<double>(L/2);
  const std::vector<ftd::Vec3> start{
      {c+0.08,c-0.06,c-0.42},
      {c-0.08,c+0.06,c+0.42}};
  const std::vector<ftd::Vec3> displacement{
      {0.031,-0.019,0.027},
      {-0.024,0.022,-0.029}};
  std::vector<ftd::eft::QuadraticCoatFaceCurrent> segments;
  std::vector<ftd::Vec3> velocities;
  for(std::size_t item=0;item<start.size();++item) {
    const int charge=item==0?+1:-1;
    segments.push_back(ftd::eft::make_quadratic_coat_face_current(
        L,start[item],start[item]+displacement[item],charge,false));
    velocities.push_back(displacement[item]*(1.0/dt));
    check("segment valid L="+std::to_string(L)
          +" item="+std::to_string(item),segments.back().valid
          &&!segments.back().dense_materialized);
  }

  constexpr double current_scale=-1.0;
  const auto cpu=
      ftd::eft::evaluate_quadratic_coat_orbit_gather_sparse_midpoint_batch_prevalidated_fields(
          segments,electric,prepared_electric,current_scale,
          prepared_magnetic,velocities,dt,1.0,1.0);
  ftd::eft::CudaQuadraticCoatOrbitGatherTelemetry telemetry;
  const auto gpu=
      ftd::eft::evaluate_quadratic_coat_orbit_gather_sparse_midpoint_batch_cuda_resident(
          segments,views.electric_before,views.electric_pre_current,
          current_scale,views.magnetic_prepared,velocities,dt,1.0,1.0,
          &telemetry);
  check("gather telemetry L="+std::to_string(L),telemetry.valid);
  if(!telemetry.valid)
    std::cerr<<"CUDA gather error L="<<L<<": "<<telemetry.error<<'\n';
  check("gather shape L="+std::to_string(L),
      cpu.size()==segments.size()&&gpu.size()==cpu.size());
  check("no complete download L="+std::to_string(L),
      telemetry.complete_field_downloads==0
      &&telemetry.device_to_host_bytes<1024);
  for(std::size_t item=0;item<std::min(cpu.size(),gpu.size());++item) {
    const auto& a=cpu[item];
    const auto& b=gpu[item];
    const std::string prefix="L="+std::to_string(L)
        +" item="+std::to_string(item);
    check(prefix+" discrete parity",a.valid==b.valid&&a.L==b.L
        &&a.charge==b.charge&&a.quadrature_pieces==b.quadrature_pieces);
    check(prefix+" vector parity",
        close(a.start_effective_position,b.start_effective_position)
        &&close(a.end_effective_position,b.end_effective_position)
        &&close(a.displacement,b.displacement)
        &&close(a.discrete_gradient_velocity,b.discrete_gradient_velocity)
        &&close(a.electric_force,b.electric_force)
        &&close(a.magnetic_average,b.magnetic_average)
        &&close(a.magnetic_impulse,b.magnetic_impulse));
    check(prefix+" scalar parity",
        close(a.temporal_scale,b.temporal_scale)
        &&close(a.beta,b.beta)
        &&close(a.current_work,b.current_work)
        &&close(a.electric_work,b.electric_work)
        &&close(a.electric_adjoint_residual,b.electric_adjoint_residual)
        &&close(a.magnetic_work_residual,b.magnetic_work_residual)
        &&close(a.kinematic_residual,b.kinematic_residual)
        &&close(a.causal_excess,b.causal_excess));
  }
}

ftd::eft::ConnectedMooreBlockOptions action_options() {
  ftd::eft::ConnectedMooreBlockOptions options;
  options.dt=0.25;
  options.binding_law=ftd::eft::ConnectedBindingLaw::DerivedCompactPair;
  options.compact_pair_well_depth=0.01;
  options.compact_pair_cutoff_distance_squared=1.5;
  options.allow_shared_anchor_chart=true;
  options.gate_tolerance=1e-10;
  options.solve_tolerance=2e-14;
  options.max_iterations=384;
  options.use_sparse_local_current=true;
  options.use_local_residual_evaluation=true;
  options.defer_volume_diagnostics=true;
  return options;
}

ftd::eft::ConnectedMooreBlockState prepared_pair(
    int L,const ftd::eft::ConnectedMooreBlockOptions& options) {
  ftd::eft::ConnectedMooreBlockState geometry(L);
  const int c=L/2;
  ftd::eft::MatchedMatterPoint positive;
  positive.anchor={c,c,c};
  positive.remainder={0.0,0.0,-0.45};
  positive.momentum={0.012,-0.004,0.015};
  ftd::eft::MatchedMatterPoint negative;
  negative.anchor={c,c,c};
  negative.remainder={0.0,0.0,0.45};
  negative.momentum={-0.012,0.004,-0.015};
  geometry.constituents={positive,negative};
  geometry.charges={+1,-1};
  const auto preparation=ftd::eft::prepare_finite_support_derived_compact_pair(
      geometry,options,4,1e-13,4096);
  check("root preparation L="+std::to_string(L),preparation.valid);
  return preparation.state;
}

void compare_resident_root_state(
    int L,const ftd::eft::ConnectedMooreBlockState& initial) {
  const auto options=action_options();
  if(initial.electric.x.empty()) return;
  const double lambda=options.wave_speed*options.dt;
  auto reference_state=initial;
  ftd::eft::ConnectedMooreBlockState resident_state;
  resident_state.electric.L=L;
  resident_state.magnetic_half.L=L;
  resident_state.constituents=initial.constituents;
  resident_state.charges=initial.charges;
  resident_state.edges=initial.edges;
  resident_state.width=initial.width;
  resident_state.orientation_axis=initial.orientation_axis;
  ftd::eft::ConnectedMooreBlockSolveCache reference_cache,resident_cache;
  ftd::eft::CudaMatchedFieldPipeline reference_pipeline(L);
  ftd::eft::CudaMatchedFieldPipeline resident_pipeline(L);
  check("paired pipeline upload L="+std::to_string(L),
      reference_pipeline.valid()&&resident_pipeline.valid()
      &&reference_pipeline.upload(initial.electric,initial.magnetic_half)
      &&resident_pipeline.upload(initial.electric,initial.magnetic_half));
  const auto normalization=ftd::eft::measure_face_flux_normalization();
  const double interaction_scale=normalization.mapped_field_work_coefficient;
  const ftd::Vec3 center{static_cast<double>(L/2),
                         static_cast<double>(L/2),
                         static_cast<double>(L/2)};
  for(int tick=1;tick<=2;++tick) {
    check("prepare pair L="+std::to_string(L)
          +" tick="+std::to_string(tick),
        reference_pipeline.prepare_forward(lambda)
        &&resident_pipeline.prepare_forward(lambda));
    ftd::eft::MatchedFaceFlux prepared_electric(L);
    ftd::eft::MatchedEdgeField prepared_magnetic(L);
    check("reference preparation checkpoint L="+std::to_string(L)
          +" tick="+std::to_string(tick),
        reference_pipeline.download_prepared(
            prepared_magnetic,prepared_electric));
    auto reference=ftd::eft::solve_connected_moore_block_forward_prepared(
        reference_state,std::move(prepared_magnetic),
        std::move(prepared_electric),options,&reference_cache);

    const auto views=resident_pipeline.resident_views();
    std::size_t root_upload=0,root_download=0,root_complete_downloads=0;
    bool callback_valid=true;
    auto resident_options=options;
    resident_options.resident_local_orbit_gather=
        [&](const std::vector<ftd::eft::QuadraticCoatFaceCurrent>& segments,
            const std::vector<ftd::Vec3>& velocities,double current_scale,
            double temporal_scale,double beta,double polarity_scale) {
          ftd::eft::CudaQuadraticCoatOrbitGatherTelemetry telemetry;
          auto result=
              ftd::eft::evaluate_quadratic_coat_orbit_gather_sparse_midpoint_batch_cuda_resident(
                  segments,views.electric_before,views.electric_pre_current,
                  current_scale,views.magnetic_prepared,velocities,
                  temporal_scale,beta,polarity_scale,&telemetry);
          callback_valid=callback_valid&&telemetry.valid;
          root_upload+=telemetry.host_to_device_bytes;
          root_download+=telemetry.device_to_host_bytes;
          root_complete_downloads+=telemetry.complete_field_downloads;
          return result;
        };
    auto resident=ftd::eft::solve_connected_moore_block_forward_resident(
        resident_state,resident_options,&resident_cache);
    const std::string prefix="L="+std::to_string(L)
        +" tick="+std::to_string(tick);
    check(prefix+" root pending",reference.solve.converged
        &&resident.solve.converged&&reference.volume_diagnostics_pending
        &&resident.volume_diagnostics_pending&&callback_valid);
    check(prefix+" root transfer boundary",root_complete_downloads==0
        &&root_upload<1024*1024&&root_download<1024*1024);
    check(prefix+" ordered segment parity",
        same_ordered_segments(reference.segments,resident.segments));
    check(prefix+" root scalar parity",
        close(reference.root_residual,resident.root_residual)
        &&close(reference.force_residual,resident.force_residual)
        &&close(reference.continuity_residual,resident.continuity_residual)
        &&close(reference.current_work,resident.current_work)
        &&close(reference.matter_work_residual,resident.matter_work_residual)
        &&reference.site_hops==resident.site_hops
        &&reference.site_projection_valid==resident.site_projection_valid);

    check(prefix+" apply ordered current",
        reference_pipeline.apply_ordered_sparse_current(
            reference.segments,options.polarity_scale)
        &&resident_pipeline.apply_ordered_sparse_current(
            resident.segments,options.polarity_scale));
    const auto reference_profile=reference_pipeline.observe_deterministic(
        lambda,center,{8,24},1e-10);
    const auto resident_profile=resident_pipeline.observe_deterministic(
        lambda,center,{8,24},1e-10);
    const auto reference_diagnostics=
        reference_pipeline.diagnose_common_action(
            reference.segments,options.polarity_scale,interaction_scale,
            options.wave_speed,options.dt,1e-10);
    const auto resident_diagnostics=
        resident_pipeline.diagnose_common_action(
            resident.segments,options.polarity_scale,interaction_scale,
            options.wave_speed,options.dt,1e-10);
    reference=ftd::eft::complete_connected_moore_block_volume_diagnostics(
        std::move(reference),reference_diagnostics,options);
    resident=ftd::eft::complete_connected_moore_block_volume_diagnostics(
        std::move(resident),resident_diagnostics,resident_options);
    check(prefix+" completed transaction parity",
        reference_profile.valid&&resident_profile.valid
        &&reference.valid==resident.valid
        &&reference.common_action_gates_pass
            ==resident.common_action_gates_pass
        &&close(reference.total_energy_residual,
                 resident.total_energy_residual)
        &&close(reference.gauss_before_residual,
                 resident.gauss_before_residual)
        &&close(reference.gauss_after_residual,
                 resident.gauss_after_residual)
        &&close(reference.spline_defect_norm,resident.spline_defect_norm));

    ftd::eft::MatchedFaceFlux resident_electric(L);
    ftd::eft::MatchedEdgeField resident_magnetic(L);
    check(prefix+" deliberate parity checkpoint",
        resident_pipeline.download_after(
            resident_electric,resident_magnetic));
    auto resident_full=resident.later;
    resident_full.electric=std::move(resident_electric);
    resident_full.magnetic_half=std::move(resident_magnetic);
    check(prefix+" complete state parity",
        ftd::eft::connected_moore_block_state_max_difference(
            reference.later,resident_full)<=1e-12);

    ftd::eft::StateOnlyMatterFieldObserverOptions observer;
    observer.support_half_width=4;
    observer.shell_radii=L==33?std::vector<int>{8,12,16}
        :std::vector<int>{8,12,16,24,32};
    ftd::eft::CudaStateOnlySupportLadderTelemetry observer_telemetry;
    const auto cpu_observer=ftd::eft::observe_state_only_matter_field(
        reference.later,options,observer);
    const auto resident_observer=
        ftd::eft::observe_state_only_matter_field_cuda_resident(
            resident.later,resident_options,views.electric_after,
            views.magnetic_prepared,observer,&observer_telemetry);
    check(prefix+" resident state-only observer",
        cpu_observer.valid&&resident_observer.valid
        &&observer_telemetry.valid
        &&observer_telemetry.complete_field_downloads==0
        &&close(cpu_observer.bound_energy,resident_observer.bound_energy)
        &&close(cpu_observer.residual_energy,
                 resident_observer.residual_energy)
        &&close(cpu_observer.outgoing_energy,
                 resident_observer.outgoing_energy)
        &&close(cpu_observer.boundary_flux_sum,
                 resident_observer.boundary_flux_sum));
    ftd::eft::CudaStateOnlySupportLadderTelemetry ladder_telemetry;
    const auto cpu_ladder=ftd::eft::observe_state_only_support_ladder(
        reference.later,options,{4,6,8},1e-13,4096,1e-12);
    const auto resident_ladder=
        ftd::eft::observe_state_only_support_ladder_cuda_resident(
            resident.later,resident_options,views.electric_after,
            {4,6,8},1e-13,4096,1e-12,&ladder_telemetry);
    check(prefix+" resident support ladder",
        cpu_ladder.valid&&resident_ladder.valid&&ladder_telemetry.valid
        &&ladder_telemetry.complete_field_downloads==0
        &&close(cpu_ladder.maximum_energy_reconstruction_residual,
                 resident_ladder.maximum_energy_reconstruction_residual)
        &&close(cpu_ladder.maximum_projection_residual,
                 resident_ladder.maximum_projection_residual));

    reference_state=std::move(reference.later);
    resident_state=std::move(resident.later);
    check(prefix+" device-only advance",
        reference_pipeline.advance()&&resident_pipeline.advance());
  }
  auto recovered=reference_state;
  auto reverse_options=options;
  reverse_options.defer_volume_diagnostics=false;
  reverse_options.resident_local_orbit_gather={};
  ftd::eft::ConnectedMooreBlockSolveCache reverse_cache;
  bool reverse_valid=true;
  for(int tick=2;tick>=1;--tick) {
    const auto reverse=ftd::eft::solve_connected_moore_block_reverse(
        recovered,reverse_options,&reverse_cache);
    reverse_valid=reverse_valid&&reverse.valid
        &&reverse.common_action_gates_pass;
    if(!reverse_valid) break;
    recovered=reverse.earlier;
  }
  check("two-step state-only inverse L="+std::to_string(L),
      reverse_valid
      &&ftd::eft::connected_moore_block_state_max_difference(
          initial,recovered)<=1e-10);
}

void compare_resident_root(int L) {
  active_context="two_step_default_L"+std::to_string(L);
  const auto options=action_options();
  compare_resident_root_state(L,prepared_pair(L,options));
  active_context.clear();
}

}  // namespace

int main() {
  compare_volume(33);
  compare_volume(65);
  compare_resident_root(33);
  compare_resident_root(65);
  if(failures==0) {
    std::cout<<"FTD-0759 resident CUDA orbit gather parity: PASS\n";
    return 0;
  }
  std::cerr<<failures<<" resident CUDA orbit-gather checks failed\n";
  return 1;
}
