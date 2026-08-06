/** FTD-0747 candidate: CUDA successor to the aborted FTD-0746 CPU run. */

#define FTD_CAUSAL_HORIZON_MAIN ftd_0746_cpu_source_main
#include "test_causal_horizon_environmental_persistence.cpp"
#undef FTD_CAUSAL_HORIZON_MAIN

#include "ftd/eft/cuda_matched_field_pipeline.h"

#include <chrono>

namespace {

constexpr char kCudaHorizonProtocolSha256[] =
    "1FB4A49897D8FEC333C686A54D44A90EA6E51D799EDBD9168F8D313287F4FD5F";

HorizonRow make_horizon_cuda_record(
    int tick,const ftd::eft::ConnectedMooreBlockStepResult& step,
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& options,double interaction_scale,
    const ftd::eft::BatchedRegionalEnergyProfile& profile,
    const Vec3& center,std::array<double,6>& cumulative_outward) {
  HorizonRow row;
  row.tick=tick; row.valid=step.valid;
  row.common=step.common_action_gates_pass;
  row.maximum_residual=maximum_step_residual(step);
  row.total_energy_residual=step.total_energy_residual;
  row.recoil_defect=std::max({step.matter_momentum_before.mag(),
      step.matter_momentum_after.mag(),step.spline_defect_norm});
  row.speed_excess=step.causal_speed_excess;
  row.source_radius=horizon_source_radius(step.segments,center,row.source_entries);
  row.separation=pair_separation(state);
  row.pair_energy=pair_internal_energy(state,options);
  row.field_energy=interaction_scale*profile.energy_after;
  row.graph_inside=graph_inside(row.separation,options);
  const std::vector<int> radii(kHorizonRadii.begin(),kHorizonRadii.end());
  row.regional_valid=profile.valid&&profile.regions.size()==radii.size();
  row.regional_residual=profile.maximum_scalar_equivalence_residual;
  row.outside_source_residual=0.0;
  if(!row.regional_valid) return row;
  const double total_after=profile.energy_after;
  const double total_pre=profile.energy_pre_current;
  const double total_source=total_after-total_pre;
  for(std::size_t i=0;i<radii.size();++i) {
    const auto& region=profile.regions[i];
    row.regional_residual=std::max(
        row.regional_residual,horizon_regional_residual(region));
    row.inside[i]=interaction_scale*region.energy_after;
    row.outside[i]=interaction_scale*(total_after-region.energy_after);
    row.transport_into[i]=interaction_scale*region.boundary_transport_into;
    row.source_exchange[i]=interaction_scale*region.source_exchange_into_field;
    const double outside_source=interaction_scale
        *(total_source-region.source_exchange_into_field);
    row.outside_source_residual=std::max(
        row.outside_source_residual,std::abs(outside_source));
    cumulative_outward[i]-=row.transport_into[i];
    row.cumulative_outward[i]=cumulative_outward[i];
  }
  return row;
}

HorizonArm run_horizon_cuda_arm(
    const std::string& slug,const Direction& direction,
    const ConnectedMooreBlockOptions& input_options,double interaction_scale,
    int tick_limit) {
  HorizonArm arm;
  arm.slug=slug; arm.direction=direction.label;
  arm.minimum_outward_increment.fill(INFINITY);
  arm.rows.reserve(static_cast<std::size_t>(tick_limit+1));
  const Vec3 center{static_cast<double>(kHorizonL/2),
                    static_cast<double>(kHorizonL/2),
                    static_cast<double>(kHorizonL/2)};
  const auto prep=ftd::eft::prepare_finite_support_derived_compact_pair(
      make_geometry(kHorizonL,direction,false,1.30,0.0120),input_options,
      kHorizonSupportRadius,1e-13,4096);
  arm.initialized=prep.valid;
  arm.preparation_pass=prep.valid&&prep.density_contained&&prep.compact_support
      &&prep.zero_boundary_crossing&&prep.poisson_residual<=1e-13
      &&prep.gauss_residual<=1e-12&&prep.outside_maximum==0.0
      &&prep.boundary_crossing_maximum==0.0;
  if(!arm.preparation_pass) return arm;

  ConnectedMooreBlockState state=prep.state;
  auto initial=make_horizon_initial(state,input_options,interaction_scale,center);
  arm.initial_pass=!initial.graph_inside&&initial.pair_energy>1e-6
      &&initial.outside[5]<=1e-12;
  arm.rows.push_back(std::move(initial));

  auto options=input_options;
  options.defer_volume_diagnostics=true;
  ftd::eft::CudaMatchedFieldPipeline pipeline(kHorizonL);
  if(!pipeline.valid()||!pipeline.upload(state.electric,state.magnetic_half))
    return arm;
  const double lambda=options.wave_speed*options.dt;
  const std::vector<int> radii(kHorizonRadii.begin(),kHorizonRadii.end());
  ftd::eft::MatchedEdgeField prepared_magnetic(kHorizonL);
  ftd::eft::MatchedFaceFlux prepared_electric(kHorizonL);
  bool valid=true,exact=true;
  std::array<double,6> cumulative_outward{};
  ConnectedMooreBlockSolveCache cache;
  for(int tick=1;tick<=tick_limit;++tick) {
    if(!pipeline.prepare_forward(lambda)
        ||!pipeline.download_prepared(prepared_magnetic,prepared_electric)) {
      valid=false; break;
    }
    auto step=ftd::eft::solve_connected_moore_block_forward_prepared(
        state,std::move(prepared_magnetic),std::move(prepared_electric),
        options,&cache);
    if(!step.volume_diagnostics_pending
        ||!pipeline.apply_sparse_current(step.segments,options.polarity_scale)) {
      valid=false; break;
    }
    const auto profile=pipeline.observe(
        lambda,center,radii,kHorizonGate);
    const auto diagnostics=pipeline.diagnose_common_action(
        step.segments,options.polarity_scale,interaction_scale,
        options.wave_speed,options.dt,kHorizonGate);
    step=ftd::eft::complete_connected_moore_block_volume_diagnostics(
        std::move(step),diagnostics,options);
    valid=valid&&step.valid&&profile.valid;
    if(!step.valid||!profile.valid) break;
    std::swap(state.electric,prepared_electric);
    std::swap(state.magnetic_half,prepared_magnetic);
    state=std::move(step.later);
    auto row=make_horizon_cuda_record(tick,step,state,options,
        interaction_scale,profile,center,cumulative_outward);
    arm.maximum_source_radius=std::max(
        arm.maximum_source_radius,row.source_radius);
    arm.maximum_common_residual=std::max(
        arm.maximum_common_residual,row.maximum_residual);
    arm.maximum_energy_residual=std::max(
        arm.maximum_energy_residual,row.total_energy_residual);
    arm.maximum_recoil_defect=std::max(
        arm.maximum_recoil_defect,row.recoil_defect);
    arm.maximum_speed_excess=std::max(
        arm.maximum_speed_excess,row.speed_excess);
    arm.maximum_regional_residual=std::max(
        arm.maximum_regional_residual,row.regional_residual);
    arm.maximum_outside_source=std::max(
        arm.maximum_outside_source,row.outside_source_residual);
    exact=exact&&row.common&&row.regional_valid
        &&row.maximum_residual<=kHorizonGate
        &&row.total_energy_residual<=1e-8
        &&row.recoil_defect<=1e-9&&row.speed_excess<=1e-12
        &&row.regional_residual<=kHorizonGate
        &&row.outside_source_residual<=kHorizonGate;
    for(std::size_t i=0;i<kHorizonRadii.size();++i) {
      arm.maximum_outside[i]=std::max(arm.maximum_outside[i],row.outside[i]);
      if(arm.first_tail_tick[i]<0&&row.outside[i]>kHorizonTailThreshold)
        arm.first_tail_tick[i]=tick;
      if(arm.first_tail_tick[i]>=0)
        arm.minimum_outward_increment[i]=std::min(
            arm.minimum_outward_increment[i],-row.transport_into[i]);
    }
    arm.rows.push_back(std::move(row));
    if(!pipeline.advance()) { valid=false; break; }
  }
  arm.forward_executed=valid
      &&arm.rows.size()==static_cast<std::size_t>(tick_limit+1);
  if(!arm.forward_executed||tick_limit!=kHorizonTicks) return arm;

  for(std::size_t i=0;i<kHorizonRadii.size();++i)
    arm.final_outside[i]=arm.rows.back().outside[i];
  arm.pair_field_balance=std::abs(
      arm.rows.back().pair_energy-arm.rows.front().pair_energy
      +arm.rows.back().field_energy-arm.rows.front().field_energy);
  arm.exact_pass=exact&&arm.pair_field_balance<=1e-8;
  arm.support_pass=arm.maximum_source_radius<=3
      &&kHorizonTicks<kHorizonContactTick;
  bool baseline_valid=false;
  const auto baseline=load_horizon_baseline(direction.label,baseline_valid);
  arm.prefix_scalar_difference=baseline_valid
      ?horizon_prefix_difference(arm,baseline,arm.prefix_discrete_pass):INFINITY;
  arm.prefix_pass=baseline_valid&&arm.prefix_discrete_pass
      &&arm.prefix_scalar_difference<=kHorizonGate;
  arm.energetic_onset_tick=horizon_negative_onset(arm,options);
  arm.core_pass=arm.initial_pass&&arm.energetic_onset_tick>=0
      &&kHorizonTicks-arm.energetic_onset_tick+1>=160;
  for(int tick=kHorizonLateBegin;tick<=kHorizonTicks;++tick) {
    const double value=arm.rows[static_cast<std::size_t>(tick)].inside[0];
    arm.late_inside_8_minimum=std::min(arm.late_inside_8_minimum,value);
    arm.late_inside_8_maximum=std::max(arm.late_inside_8_maximum,value);
  }
  arm.near_field_pass=arm.late_inside_8_minimum>=kHorizonNearMinimum
      &&arm.late_inside_8_maximum
          <=kHorizonNearDynamicRange*arm.late_inside_8_minimum;
  constexpr std::size_t r48=5;
  arm.arrival_pass=arm.rows.front().outside[r48]<=1e-12
      &&arm.maximum_outside_source<=kHorizonGate
      &&arm.maximum_outside[r48]>kHorizonTailThreshold
      &&arm.first_tail_tick[r48]>=0
      &&arm.first_tail_tick[r48]<=kHorizonArrivalDeadline;
  arm.post_arrival_pass=arm.arrival_pass
      &&arm.minimum_outward_increment[r48]>=-kHorizonGate
      &&arm.final_outside[r48]>kHorizonTailFinalThreshold;
  for(int tick=kHorizonPostArrivalBegin;tick<=kHorizonTicks;++tick) {
    const double value=arm.rows[static_cast<std::size_t>(tick)].outside[r48];
    arm.post_arrival_48_minimum=std::min(
        arm.post_arrival_48_minimum,value);
    arm.post_arrival_pass=arm.post_arrival_pass
        &&value>kHorizonTailFinalThreshold;
  }
  return arm;
}

}  // namespace

int main(int argc,char** argv) {
  const bool qualification=argc==4&&std::string(argv[1])=="--qualify";
  const bool held_out=argc==2;
  if(!qualification&&!held_out) {
    std::cout<<"FTD-0747 CUDA: invoke once per face|edge|body after lock; "
        "pre-lock qualification uses --qualify face|edge|body N\n";
    return argc==1?0:2;
  }
  Direction direction;
  const std::string slug=argv[qualification?2:1];
  if(!select_horizon_direction(slug,direction)) return 2;
  const int ticks=qualification?std::stoi(argv[3]):kHorizonTicks;
  if(qualification&&(ticks<1||ticks>8)) return 2;
  if(held_out&&std::string(kCudaHorizonProtocolSha256)=="UNLOCKED") {
    std::cerr<<"FTD-0747 held-out execution refused before protocol lock\n";
    return 3;
  }
  ConnectedMooreBlockOptions options;
  options.dt=0.25;
  options.binding_law=ConnectedBindingLaw::DerivedCompactPair;
  options.compact_pair_well_depth=0.01;
  options.compact_pair_cutoff_distance_squared=1.5;
  options.allow_shared_anchor_chart=true;
  options.gate_tolerance=kGate;
  options.solve_tolerance=2e-14;
  options.max_iterations=384;
  options.use_sparse_local_current=true;
  options.use_local_residual_evaluation=true;
  const auto normalization=ftd::eft::measure_face_flux_normalization();
  const auto start=std::chrono::steady_clock::now();
  const auto arm=run_horizon_cuda_arm(slug,direction,options,
      normalization.mapped_field_work_coefficient,ticks);
  const double seconds=std::chrono::duration<double>(
      std::chrono::steady_clock::now()-start).count();
  if(qualification) {
    std::cout<<std::setprecision(17)<<"FTD-0747 qualification "<<slug
      <<" ticks="<<ticks<<" rows="<<arm.rows.size()
      <<" initialized="<<arm.initialized
      <<" preparation="<<arm.preparation_pass
      <<" initial="<<arm.initial_pass
      <<" executed="<<arm.forward_executed
      <<" max_residual="<<arm.maximum_common_residual
      <<" max_energy="<<arm.maximum_energy_residual
      <<" max_recoil="<<arm.maximum_recoil_defect
      <<" seconds="<<seconds
      <<" protocol="<<kCudaHorizonProtocolSha256<<'\n';
    return normalization.valid&&arm.initialized&&arm.preparation_pass
        &&arm.initial_pass&&arm.forward_executed?0:1;
  }
  auto completed=arm;
  if(!normalization.valid) completed.exact_pass=false;
  const auto verdict=horizon_verdict(completed);
  write_horizon_records(completed,verdict,"FTD-0747",
      kCudaHorizonProtocolSha256,"ftd_0747",
      "ftd_0747_causal_horizon_environmental_persistence_cuda_v2",
      "wsl2_cuda_matched_face_edge");
  std::cout<<"FTD-0747 "<<slug<<' '<<verdict
      <<" prefix="<<std::setprecision(8)<<completed.prefix_scalar_difference
      <<" r48_tick="<<completed.first_tail_tick[5]
      <<" seconds="<<seconds<<'\n';
  return verdict=="CAUSAL_HORIZON_EXECUTION_INVALID"?1:0;
}
