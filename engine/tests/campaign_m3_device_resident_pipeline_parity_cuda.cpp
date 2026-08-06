/** FTD-0759: frozen small-volume device-resident parity matrix. */

#define main ftd_0759_resident_slice_unit_main
#include "test_cuda_quadratic_coat_orbit_gather.cpp"
#undef main

#include <array>
#include <chrono>

namespace {

struct ResidentParityRay {
  const char* name;
  ftd::Vec3 direction;
};

constexpr std::array<int,2> kParityVolumes{{33,65}};
const std::array<ResidentParityRay,3> kParityRays{{
    {"face",{0.0,0.0,1.0}},
    {"edge",{0.0,1.0,-1.0}},
    {"body",{1.0,1.0,1.0}},
}};

int parity_wrap(int value,int L) {
  const int remainder=value%L;
  return remainder<0?remainder+L:remainder;
}

ftd::eft::MatchedMatterPoint parity_point_at(
    const ftd::Vec3& position,const ftd::Vec3& momentum,int L) {
  ftd::eft::MatchedMatterPoint point;
  const long long ax=std::llround(position.x);
  const long long ay=std::llround(position.y);
  const long long az=std::llround(position.z);
  point.anchor={parity_wrap(static_cast<int>(ax),L),
                parity_wrap(static_cast<int>(ay),L),
                parity_wrap(static_cast<int>(az),L)};
  point.remainder={position.x-ax,position.y-ay,position.z-az};
  point.momentum=momentum;
  return point;
}

void add_remote_gauss_free_plaquette(
    ftd::eft::ConnectedMooreBlockState& state) {
  const int L=state.electric.L;
  const int c=L/2;
  const int x=c+std::max(7,L/3);
  const int y=c+std::max(5,L/4);
  const int z=c+std::max(3,L/5);
  constexpr double amplitude=1e-3;
  state.electric.x[static_cast<std::size_t>(
      state.electric.index(x,y,z))]+=amplitude;
  state.electric.z[static_cast<std::size_t>(
      state.electric.index(x+1,y,z))]+=amplitude;
  state.electric.x[static_cast<std::size_t>(
      state.electric.index(x,y,z+1))]-=amplitude;
  state.electric.z[static_cast<std::size_t>(
      state.electric.index(x,y,z))]-=amplitude;
}

ftd::eft::ConnectedMooreBlockState prepared_ray_pair(
    int L,const ResidentParityRay& ray,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  ftd::eft::ConnectedMooreBlockState geometry(L);
  const ftd::Vec3 center{static_cast<double>(L/2),
                         static_cast<double>(L/2),
                         static_cast<double>(L/2)};
  const auto unit=ray.direction*(1.0/ray.direction.mag());
  geometry.constituents={
      parity_point_at(center-unit*0.65,unit*0.012,L),
      parity_point_at(center+unit*0.65,unit*(-0.012),L)};
  geometry.charges={+1,-1};
  const auto preparation=ftd::eft::prepare_finite_support_derived_compact_pair(
      geometry,options,4,1e-13,4096);
  check("matrix preparation L="+std::to_string(L),
      preparation.valid&&preparation.density_contained
      &&preparation.compact_support&&preparation.zero_boundary_crossing);
  return preparation.state;
}

bool advance_reference_parent(
    ftd::eft::ConnectedMooreBlockState& state,
    const ftd::eft::ConnectedMooreBlockOptions& options,int ticks) {
  const int L=state.electric.L;
  const double lambda=options.wave_speed*options.dt;
  const auto normalization=ftd::eft::measure_face_flux_normalization();
  if(!normalization.valid) return false;
  const double interaction_scale=
      normalization.mapped_field_work_coefficient;
  const ftd::Vec3 center{static_cast<double>(L/2),
                         static_cast<double>(L/2),
                         static_cast<double>(L/2)};
  ftd::eft::CudaMatchedFieldPipeline pipeline(L);
  if(!pipeline.valid()||!pipeline.upload(
      state.electric,state.magnetic_half)) return false;
  ftd::eft::ConnectedMooreBlockSolveCache cache;
  for(int tick=1;tick<=ticks;++tick) {
    if(!pipeline.prepare_forward(lambda)) return false;
    ftd::eft::MatchedFaceFlux prepared_electric(L);
    ftd::eft::MatchedEdgeField prepared_magnetic(L);
    if(!pipeline.download_prepared(
        prepared_magnetic,prepared_electric)) return false;
    auto step=ftd::eft::solve_connected_moore_block_forward_prepared(
        state,std::move(prepared_magnetic),std::move(prepared_electric),
        options,&cache);
    if(!step.volume_diagnostics_pending
        ||!pipeline.apply_ordered_sparse_current(
            step.segments,options.polarity_scale)) return false;
    const auto profile=pipeline.observe_deterministic(
        lambda,center,{8},1e-10);
    const auto diagnostics=pipeline.diagnose_common_action(
        step.segments,options.polarity_scale,interaction_scale,
        options.wave_speed,options.dt,1e-10);
    step=ftd::eft::complete_connected_moore_block_volume_diagnostics(
        std::move(step),diagnostics,options);
    if(!profile.valid||!step.valid||!step.common_action_gates_pass)
      return false;
    state=std::move(step.later);
    if(!pipeline.advance()) return false;
  }
  return true;
}

struct ResidentSmokeResult {
  bool valid=false;
  int ticks=0;
  double seconds=0.0;
  std::size_t host_to_device_bytes=0;
  std::size_t device_to_host_bytes=0;
  std::size_t complete_field_downloads=0;
};

ResidentSmokeResult run_resident_smoke(
    const ftd::eft::ConnectedMooreBlockState& initial,
    const ftd::eft::ConnectedMooreBlockOptions& options,int ticks) {
  ResidentSmokeResult result;
  const int L=initial.electric.L;
  const double lambda=options.wave_speed*options.dt;
  const auto normalization=ftd::eft::measure_face_flux_normalization();
  if(!normalization.valid) return result;
  const double interaction_scale=
      normalization.mapped_field_work_coefficient;
  const ftd::Vec3 center{static_cast<double>(L/2),
                         static_cast<double>(L/2),
                         static_cast<double>(L/2)};
  ftd::eft::CudaMatchedFieldPipeline pipeline(L);
  if(!pipeline.valid()||!pipeline.upload(
      initial.electric,initial.magnetic_half)) return result;
  ftd::eft::ConnectedMooreBlockState state;
  state.electric.L=L;
  state.magnetic_half.L=L;
  state.constituents=initial.constituents;
  state.charges=initial.charges;
  state.edges=initial.edges;
  state.width=initial.width;
  state.orientation_axis=initial.orientation_axis;
  ftd::eft::ConnectedMooreBlockSolveCache cache;
  ftd::eft::StateOnlyMatterFieldObserverOptions observer;
  observer.support_half_width=4;
  observer.shell_radii={8,12,16,24,32,48};
  const auto start=std::chrono::steady_clock::now();
  for(int tick=1;tick<=ticks;++tick) {
    if(!pipeline.prepare_forward(lambda)) return result;
    const auto views=pipeline.resident_views();
    bool gather_valid=true;
    auto resident_options=options;
    resident_options.resident_local_orbit_gather=
        [&](const std::vector<ftd::eft::QuadraticCoatFaceCurrent>& segments,
            const std::vector<ftd::Vec3>& velocities,double current_scale,
            double temporal_scale,double beta,double polarity_scale) {
          ftd::eft::CudaQuadraticCoatOrbitGatherTelemetry telemetry;
          auto value=
              ftd::eft::evaluate_quadratic_coat_orbit_gather_sparse_midpoint_batch_cuda_resident(
                  segments,views.electric_before,views.electric_pre_current,
                  current_scale,views.magnetic_prepared,velocities,
                  temporal_scale,beta,polarity_scale,&telemetry);
          gather_valid=gather_valid&&telemetry.valid;
          result.host_to_device_bytes+=telemetry.host_to_device_bytes;
          result.device_to_host_bytes+=telemetry.device_to_host_bytes;
          result.complete_field_downloads+=telemetry.complete_field_downloads;
          return value;
        };
    auto step=ftd::eft::solve_connected_moore_block_forward_resident(
        state,resident_options,&cache);
    if(!gather_valid||!step.volume_diagnostics_pending
        ||!pipeline.apply_ordered_sparse_current(
            step.segments,options.polarity_scale)) return result;
    const auto profile=pipeline.observe_deterministic(
        lambda,center,{8,24},1e-10);
    const auto diagnostics=pipeline.diagnose_common_action(
        step.segments,options.polarity_scale,interaction_scale,
        options.wave_speed,options.dt,1e-10);
    step=ftd::eft::complete_connected_moore_block_volume_diagnostics(
        std::move(step),diagnostics,resident_options);
    ftd::eft::CudaStateOnlySupportLadderTelemetry field_telemetry;
    const auto field=
        ftd::eft::observe_state_only_matter_field_cuda_resident(
            step.later,resident_options,views.electric_after,
            views.magnetic_prepared,observer,&field_telemetry);
    ftd::eft::CudaStateOnlySupportLadderTelemetry ladder_telemetry;
    const auto ladder=
        ftd::eft::observe_state_only_support_ladder_cuda_resident(
            step.later,resident_options,views.electric_after,{4,6,8},
            1e-13,4096,1e-12,&ladder_telemetry);
    result.host_to_device_bytes+=field_telemetry.host_to_device_bytes
        +ladder_telemetry.host_to_device_bytes;
    result.device_to_host_bytes+=field_telemetry.device_to_host_bytes
        +ladder_telemetry.device_to_host_bytes;
    result.complete_field_downloads+=field_telemetry.complete_field_downloads
        +ladder_telemetry.complete_field_downloads;
    if(!profile.valid||!step.valid||!step.common_action_gates_pass
        ||!field.valid||!field_telemetry.valid
        ||!ladder.valid||!ladder_telemetry.valid
        ||!pipeline.advance()) return result;
    state=std::move(step.later);
    ++result.ticks;
  }
  result.seconds=std::chrono::duration<double>(
      std::chrono::steady_clock::now()-start).count();
  result.valid=result.ticks==ticks&&result.complete_field_downloads==0;
  return result;
}

ResidentSmokeResult run_baseline_smoke(
    ftd::eft::ConnectedMooreBlockState state,
    const ftd::eft::ConnectedMooreBlockOptions& options,int ticks) {
  ResidentSmokeResult result;
  const int L=state.electric.L;
  const double lambda=options.wave_speed*options.dt;
  const auto normalization=ftd::eft::measure_face_flux_normalization();
  if(!normalization.valid) return result;
  const double interaction_scale=
      normalization.mapped_field_work_coefficient;
  const ftd::Vec3 center{static_cast<double>(L/2),
                         static_cast<double>(L/2),
                         static_cast<double>(L/2)};
  ftd::eft::CudaMatchedFieldPipeline pipeline(L);
  if(!pipeline.valid()||!pipeline.upload(
      state.electric,state.magnetic_half)) return result;
  ftd::eft::ConnectedMooreBlockSolveCache cache;
  ftd::eft::StateOnlyMatterFieldObserverOptions observer;
  observer.support_half_width=4;
  observer.shell_radii={8,12,16,24,32,48};
  const std::size_t complete_field_bytes=
      6*static_cast<std::size_t>(L)*L*L*sizeof(double);
  const auto start=std::chrono::steady_clock::now();
  for(int tick=1;tick<=ticks;++tick) {
    if(!pipeline.prepare_forward(lambda)) return result;
    ftd::eft::MatchedFaceFlux prepared_electric(L);
    ftd::eft::MatchedEdgeField prepared_magnetic(L);
    if(!pipeline.download_prepared(
        prepared_magnetic,prepared_electric)) return result;
    result.device_to_host_bytes+=complete_field_bytes;
    ++result.complete_field_downloads;
    auto step=ftd::eft::solve_connected_moore_block_forward_prepared(
        state,std::move(prepared_magnetic),std::move(prepared_electric),
        options,&cache);
    if(!step.volume_diagnostics_pending
        ||!pipeline.apply_ordered_sparse_current(
            step.segments,options.polarity_scale)) return result;
    const auto profile=pipeline.observe_deterministic(
        lambda,center,{8,24},1e-10);
    const auto diagnostics=pipeline.diagnose_common_action(
        step.segments,options.polarity_scale,interaction_scale,
        options.wave_speed,options.dt,1e-10);
    step=ftd::eft::complete_connected_moore_block_volume_diagnostics(
        std::move(step),diagnostics,options);
    const auto field=ftd::eft::observe_state_only_matter_field(
        step.later,options,observer);
    const auto ladder=ftd::eft::observe_state_only_support_ladder(
        step.later,options,{4,6,8},1e-13,4096,1e-12);
    if(!profile.valid||!step.valid||!step.common_action_gates_pass
        ||!field.valid||!ladder.valid||!pipeline.advance()) return result;
    state=std::move(step.later);
    ++result.ticks;
  }
  result.seconds=std::chrono::duration<double>(
      std::chrono::steady_clock::now()-start).count();
  result.valid=result.ticks==ticks;
  return result;
}

}  // namespace

int main(int argc,char** argv) {
  const auto options=action_options();
  if(argc==3&&std::string(argv[1])=="--smoke") {
    const int L=std::stoi(argv[2]);
    if(L<33||L%2==0) return 2;
    active_context="face/tick0/smoke/L"+std::to_string(L);
    const auto start=std::chrono::steady_clock::now();
    compare_resident_root_state(
        L,prepared_ray_pair(L,kParityRays.front(),options));
    const double seconds=std::chrono::duration<double>(
        std::chrono::steady_clock::now()-start).count();
    active_context.clear();
    std::cout<<"FTD-0759 large-volume smoke L="<<L
        <<" seconds="<<seconds<<" pass="<<(failures==0)<<'\n';
    return failures==0?0:1;
  }
  if(argc==4&&std::string(argv[1])=="--resident-smoke") {
    const int L=std::stoi(argv[2]);
    const int ticks=std::stoi(argv[3]);
    if(L<33||L%2==0||ticks<1) return 2;
    const auto state=prepared_ray_pair(
        L,kParityRays.front(),options);
    const auto value=run_resident_smoke(state,options,ticks);
    std::cout<<"FTD-0759 resident smoke L="<<L
        <<" ticks="<<value.ticks<<" seconds="<<value.seconds
        <<" h2d="<<value.host_to_device_bytes
        <<" d2h="<<value.device_to_host_bytes
        <<" complete_downloads="<<value.complete_field_downloads
        <<" pass="<<value.valid<<'\n';
    return value.valid?0:1;
  }
  if(argc==4&&std::string(argv[1])=="--baseline-smoke") {
    const int L=std::stoi(argv[2]);
    const int ticks=std::stoi(argv[3]);
    if(L<33||L%2==0||ticks<1) return 2;
    const auto state=prepared_ray_pair(
        L,kParityRays.front(),options);
    const auto value=run_baseline_smoke(state,options,ticks);
    std::cout<<"FTD-0759 baseline smoke L="<<L
        <<" ticks="<<value.ticks<<" seconds="<<value.seconds
        <<" h2d="<<value.host_to_device_bytes
        <<" d2h="<<value.device_to_host_bytes
        <<" complete_downloads="<<value.complete_field_downloads
        <<" pass="<<value.valid<<'\n';
    return value.valid?0:1;
  }
  if(argc!=1) return 2;
  int cases=0;
  for(const int L:kParityVolumes) for(const auto& ray:kParityRays) {
    auto tick0=prepared_ray_pair(L,ray,options);
    active_context=std::string(ray.name)+"/tick0/L"+std::to_string(L);
    compare_resident_root_state(L,tick0);
    ++cases;

    auto parent=tick0;
    active_context=std::string(ray.name)+"/parent-build/L"+std::to_string(L);
    check("tick-160 reference parent",
        advance_reference_parent(parent,options,160));
    active_context=std::string(ray.name)+"/tick160/L"+std::to_string(L);
    compare_resident_root_state(L,parent);
    ++cases;

    auto remote=parent;
    add_remote_gauss_free_plaquette(remote);
    active_context=std::string(ray.name)+"/remote/L"+std::to_string(L);
    compare_resident_root_state(L,remote);
    ++cases;
  }
  active_context.clear();
  if(failures==0) {
    std::cout<<"FTD-0759 resident parity matrix: PASS cases="<<cases
        <<" transactions="<<2*cases<<'\n';
    return 0;
  }
  std::cerr<<failures<<" FTD-0759 resident parity checks failed\n";
  return 1;
}
