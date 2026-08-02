/** FTD-0760: relational-chart fresh held-out M3 validation. */

#define FTD_0754_MAIN_NAME ftd_0760_global_observer_main
#include "campaign_state_only_observer_discovery_cuda.cpp"
#undef FTD_0754_MAIN_NAME

#include "ftd/eft/support_invariant_matter_predicate.h"

#include <bitset>
#include <cstring>
#include <memory>

namespace ftd0760_embedded {

#include "campaign_m3_support_invariant_validation_cuda.cpp"

namespace {

constexpr char kM3RelationalValidationProtocolSha256[]=
    "681FA36CCE4479D268D37651E4CD58AA6C1D5A4809F989EA4FF2AA24B7B40722";
constexpr double kM3FreshCornerImpulse=1.0/2048.0;
constexpr double kM3FreshFieldScale=15.0/16.0;
constexpr double kM3FreshShellFraction=5.0/8.0;
constexpr int kM3FreshFibreDisplacement=104;
constexpr double kM3FreshFibreAmplitude=1.0/1024.0;
constexpr bool kM3AllowSharedAnchorChart=true;

class M3FixedChartCudaStepper {
 public:
  M3FixedChartCudaStepper(ConnectedMooreBlockState initial,
                          ConnectedMooreBlockOptions input_options,
                          double interaction_scale,bool measure_regularity)
      : state_(std::move(initial)),options_(std::move(input_options)),
        interaction_scale_(interaction_scale),pipeline_(state_.electric.L),
        prepared_magnetic_(state_.electric.L),
        prepared_electric_(state_.electric.L) {
    const double c=static_cast<double>(state_.electric.L/2);
    fixed_center_={c,c,c};
    options_.defer_volume_diagnostics=true;
    options_.measure_final_root_regularity=measure_regularity;
    valid_=pipeline_.valid()
        &&pipeline_.upload(state_.electric,state_.magnetic_half);
  }

  bool valid() const { return valid_; }
  const ConnectedMooreBlockState& state() const { return state_; }
  ConnectedMooreBlockState release_state() { return std::move(state_); }

  M3StepDiagnostics advance() {
    M3StepDiagnostics record;
    if(!valid_) return record;
    const double lambda=options_.wave_speed*options_.dt;
    if(!pipeline_.prepare_forward(lambda)
        ||!pipeline_.download_prepared(prepared_magnetic_,prepared_electric_)) {
      record.failure_stage=1;
      valid_=false;
      return record;
    }
    auto step=ftd::eft::solve_connected_moore_block_forward_prepared(
        state_,std::move(prepared_magnetic_),std::move(prepared_electric_),
        options_,&cache_);
    record.solve_attempted=step.solve.attempted;
    record.solve_converged=step.solve.converged;
    record.solve_iterations=step.solve.iterations;
    record.solve_residual=step.solve.residual;
    if(!step.volume_diagnostics_pending) {
      record.failure_stage=2;
      valid_=false;
      return record;
    }
    if(!pipeline_.apply_ordered_sparse_current(
            step.segments,options_.polarity_scale)) {
      record.failure_stage=3;
      valid_=false;
      return record;
    }
    const auto profile=pipeline_.observe_deterministic(
        lambda,fixed_center_,{8},kM3CommonGate);
    if(!profile.valid) {
      record.failure_stage=4;
      valid_=false;
      return record;
    }
    const auto diagnostics=pipeline_.diagnose_common_action(
        step.segments,options_.polarity_scale,interaction_scale_,
        options_.wave_speed,options_.dt,kM3CommonGate);
    step=ftd::eft::complete_connected_moore_block_volume_diagnostics(
        std::move(step),diagnostics,options_);
    record.valid=step.valid;
    record.common=step.common_action_gates_pass;
    record.site_hops=step.site_hops;
    record.graph_connected=step.graph_connected;
    record.graph_local=step.graph_local;
    record.relational_edge_before=step.relational_edge_before;
    record.relational_edge_after=step.relational_edge_after;
    record.relational_graph_changed=step.relational_graph_changed;
    record.site_projection_valid=step.site_projection_valid;
    record.maximum_residual=maximum_step_residual(step);
    record.energy_residual=std::abs(step.total_energy_residual);
    record.recoil_defect=std::max({step.matter_momentum_before.mag(),
        step.matter_momentum_after.mag(),step.spline_defect_norm});
    record.speed_excess=step.causal_speed_excess;
    record.regularity_measured=step.solve.final_root_regularity_measured;
    record.minimum_singular_value=step.solve.final_minimum_singular_value;
    record.condition_number=step.solve.final_condition_number;
    record.scale_difference=step.solve.regularity_scale_relative_difference;
    std::swap(state_.electric,prepared_electric_);
    std::swap(state_.magnetic_half,prepared_magnetic_);
    state_=std::move(step.later);
    if(!pipeline_.advance()) valid_=false;
    if(!valid_) record.failure_stage=5;
    valid_=valid_&&record.valid;
    return record;
  }

 private:
  ConnectedMooreBlockState state_;
  ConnectedMooreBlockOptions options_;
  double interaction_scale_=0.0;
  Vec3 fixed_center_{};
  ftd::eft::CudaMatchedFieldPipeline pipeline_;
  ftd::eft::MatchedEdgeField prepared_magnetic_;
  ftd::eft::MatchedFaceFlux prepared_electric_;
  ConnectedMooreBlockSolveCache cache_;
  bool valid_=false;
};

M3ParentCheckpoint ftd0760_build_checkpoint(
    int L,const Direction& direction,
    const ConnectedMooreBlockOptions& options,double interaction_scale,
    int tick_limit=kM3FormationTick) {
  M3ParentCheckpoint result(L);
  result.direction=direction.label;
  auto preparation=ftd::eft::prepare_finite_support_derived_compact_pair(
      make_geometry(L,direction,false,1.30,0.0120),options,4,1e-13,4096);
  if(!preparation.valid||!preparation.density_contained
      ||!preparation.compact_support||!preparation.zero_boundary_crossing)
    return result;
  M3FixedChartCudaStepper stepper(
      std::move(preparation.state),options,interaction_scale,false);
  if(!stepper.valid()) return result;
  for(int tick=1;tick<=tick_limit;++tick) {
    const auto step=stepper.advance();
    if(!step.valid||!step.common) return result;
  }
  result.state=stepper.release_state();
  const auto core=ftd::eft::observe_support_invariant_matter(
      result.state,options);
  result.valid=core.valid&&core.member
      &&core.graph_margin>=kM3CoreMargin
      &&core.energy_margin>=kM3CoreMargin;
  return result;
}

M3CornerSpec ftd0760_corner_spec(
    const std::string& direction,const std::string& variant) {
  if(variant=="center") return {0,0,0,"center"};
  if(direction=="0_0_1"&&variant=="energy_hostile")
    return {-1,+1,+1,"srm_s1p_s2p_r15o16"};
  if(direction=="0_0_1"&&variant=="graph_hostile")
    return {-1,-1,+1,"srm_s1m_s2p_r15o16"};
  if(direction=="0_1_-1"&&variant=="energy_hostile")
    return {-1,+1,+1,"srm_s1p_s2p_r15o16"};
  if(direction=="0_1_-1"&&variant=="graph_hostile")
    return {-1,+1,-1,"srm_s1p_s2m_r15o16"};
  if(direction=="1_1_1"&&variant=="energy_hostile")
    return {-1,+1,+1,"srm_s1p_s2p_r15o16"};
  if(direction=="1_1_1"&&variant=="graph_hostile")
    return {-1,-1,+1,"srm_s1m_s2p_r15o16"};
  return {};
}

M3VariantState ftd0760_make_variant(
    const M3ParentCheckpoint& parent,const Direction& direction,
    const std::string& variant,const ConnectedMooreBlockOptions& options) {
  const int L=parent.volume;
  M3VariantState result(L);
  const auto spec=ftd0760_corner_spec(direction.label,variant);
  result.registered_name=spec.name;
  if(!parent.valid||spec.name.empty()) return result;
  if(variant=="center") {
    result.state=parent.state;
    result.root_residual=0.0;
    result.nearest_shell_margin=INFINITY;
    result.valid=true;
    return result;
  }

  const auto bound_parent=ftd::eft::prepare_finite_support_derived_compact_pair(
      parent.state,options,4,1e-13,4096);
  if(!bound_parent.valid) return result;
  const Vec3 x0=effective_position(parent.state.constituents[0]);
  const Vec3 x1=effective_position(parent.state.constituents[1]);
  const Vec3 center=(x0+x1)*0.5;
  const Vec3 relative=x1-x0;
  const double parent_d=relative.mag2();
  if(!(parent_d>0.0)) return result;
  const Vec3 radial=relative*(1.0/std::sqrt(parent_d));
  const auto tangents=m3_tangents(direction);
  const Vec3 impulse=(radial*static_cast<double>(spec.sigma_r)
      +tangents.first*static_cast<double>(spec.sigma_1)
      +tangents.second*static_cast<double>(spec.sigma_2))
      *(kM3FreshCornerImpulse/std::sqrt(3.0));
  auto geometry=parent.state;
  geometry.constituents[0].momentum-=impulse;
  geometry.constituents[1].momentum+=impulse;
  const double kinetic=m3_kinetic(geometry,options);
  const auto inner=m3_root(kinetic,0.75,1.0,options);
  const auto outer=m3_root(kinetic,1.0,1.5,options);
  result.root_residual=std::max(inner.second,outer.second);
  if(!(std::isfinite(inner.first)&&std::isfinite(outer.first)
      &&inner.first<parent_d&&parent_d<outer.first
      &&result.root_residual<=1e-12)) return result;
  result.nearest_shell_margin=std::min(
      parent_d-inner.first,outer.first-parent_d);
  const double target_d=parent_d
      -kM3FreshShellFraction*result.nearest_shell_margin;
  const double position_scale=std::sqrt(target_d/parent_d);
  geometry.constituents[0]=point_at(
      center-relative*(0.5*position_scale),
      geometry.constituents[0].momentum,L);
  geometry.constituents[1]=point_at(
      center+relative*(0.5*position_scale),
      geometry.constituents[1].momentum,L);
  auto bound_perturbed=ftd::eft::prepare_finite_support_derived_compact_pair(
      geometry,options,4,1e-13,4096);
  if(!bound_perturbed.valid) return result;
  result.state=std::move(bound_perturbed.state);
  m3_add_scaled_residual(result.state.electric,parent.state.electric,
      bound_parent.state.electric,kM3FreshFieldScale);
  for(std::size_t i=0;i<result.state.magnetic_half.x.size();++i) {
    result.state.magnetic_half.x[i]=
        kM3FreshFieldScale*parent.state.magnetic_half.x[i];
    result.state.magnetic_half.y[i]=
        kM3FreshFieldScale*parent.state.magnetic_half.y[i];
    result.state.magnetic_half.z[i]=
        kM3FreshFieldScale*parent.state.magnetic_half.z[i];
  }
  const auto core=ftd::eft::observe_support_invariant_matter(
      result.state,options);
  ftd::eft::StateOnlyMatterFieldObserverOptions observer;
  observer.support_half_width=4;
  observer.shell_radii={8,12,16,24,32,48};
  observer.wave_speed=options.wave_speed;
  observer.dt=options.dt;
  const auto field=ftd::eft::observe_state_only_matter_field(
      result.state,options,observer);
  result.valid=core.valid&&core.member
      &&core.graph_margin>=kM3CoreMargin
      &&core.energy_margin>=kM3CoreMargin&&field.valid;
  return result;
}

M3History ftd0760_run_history(
    int L,const Direction& direction,const std::string& variant,
    ConnectedMooreBlockState initial,
    const ConnectedMooreBlockOptions& options,double interaction_scale,
    int ticks=kM3ContinuationTicks,bool capture_every_tick=false,
    int local_radius=48,bool require_regularity=true,
    bool require_member=true) {
  M3History result;
  result.volume=L;
  result.direction=direction.label;
  result.variant=variant;
  result.initialized=true;
  result.rows.reserve(static_cast<std::size_t>(ticks+1));
  result.checkpoints.reserve(capture_every_tick
      ?static_cast<std::size_t>(ticks+1):kM3ObserverTicks.size());
  auto initial_row=m3_make_row(L,kM3FormationTick,initial,options);
  initial_row.checkpoint=m3_observer_tick(kM3FormationTick)||capture_every_tick;
  if(initial_row.checkpoint) {
    auto checkpoint=m3_make_checkpoint_record(
        kM3FormationTick,initial,options,local_radius);
    initial_row.observer_valid=checkpoint.observer_valid;
    initial_row.ladder_valid=checkpoint.ladder_valid;
    result.checkpoints.push_back(std::move(checkpoint));
  }
  result.rows.push_back(initial_row);
  M3FixedChartCudaStepper stepper(
      std::move(initial),options,interaction_scale,require_regularity);
  if(!stepper.valid()) return result;
  bool pass=(!require_member||(initial_row.member
      &&initial_row.graph_margin>=kM3CoreMargin
      &&initial_row.energy_margin>=kM3CoreMargin))
      &&initial_row.observer_valid&&initial_row.ladder_valid;
  result.minimum_graph_margin=initial_row.graph_margin;
  result.minimum_energy_margin=initial_row.energy_margin;
  for(int offset=1;offset<=ticks;++offset) {
    const int tick=kM3FormationTick+offset;
    const auto step=stepper.advance();
    auto row=m3_make_row(L,tick,stepper.state(),options,&step);
    row.checkpoint=m3_observer_tick(tick)||capture_every_tick;
    if(row.checkpoint) {
      auto checkpoint=m3_make_checkpoint_record(
          tick,stepper.state(),options,local_radius);
      row.observer_valid=checkpoint.observer_valid;
      row.ladder_valid=checkpoint.ladder_valid;
      result.checkpoints.push_back(std::move(checkpoint));
    }
    result.rows.push_back(row);
    result.minimum_graph_margin=std::min(
        result.minimum_graph_margin,row.graph_margin);
    result.minimum_energy_margin=std::min(
        result.minimum_energy_margin,row.energy_margin);
    result.minimum_sigma=std::min(
        result.minimum_sigma,step.minimum_singular_value);
    result.maximum_condition=std::max(
        result.maximum_condition,step.condition_number);
    result.maximum_scale_difference=std::max(
        result.maximum_scale_difference,step.scale_difference);
    result.maximum_common=std::max(
        result.maximum_common,step.maximum_residual);
    result.maximum_energy=std::max(
        result.maximum_energy,step.energy_residual);
    result.maximum_recoil=std::max(
        result.maximum_recoil,step.recoil_defect);
    result.maximum_speed=std::max(
        result.maximum_speed,step.speed_excess);
    pass=pass&&(!require_member||(row.member
        &&row.graph_margin>=kM3CoreMargin
        &&row.energy_margin>=kM3CoreMargin))&&step.valid&&step.common
        &&step.maximum_residual<=kM3CommonGate
        &&step.energy_residual<=kM3EnergyGate
        &&step.recoil_defect<=kM3RecoilGate
        &&step.speed_excess<=kM3SpeedGate
        &&(!require_regularity||(step.regularity_measured
        &&step.minimum_singular_value>=kM3SigmaGate
        &&step.condition_number<=kM3ConditionGate
        &&step.scale_difference<=kM3ScaleGate))
        &&row.observer_valid&&row.ladder_valid;
    if(!stepper.valid()) break;
  }
  result.executed=result.rows.size()==static_cast<std::size_t>(ticks+1);
  result.passed=result.executed&&pass;
  return result;
}

void ftd0760_add_remote_plaquette(
    ConnectedMooreBlockState& state,const Direction& direction,
    const ConnectedMooreBlockOptions& options) {
  const auto core=ftd::eft::observe_support_invariant_matter(state,options);
  int dx=0,dy=0,dz=0;
  if(direction.label=="0_0_1") dz=kM3FreshFibreDisplacement;
  else if(direction.label=="0_1_-1") {
    dy=kM3FreshFibreDisplacement;
    dz=-kM3FreshFibreDisplacement;
  } else {
    dx=kM3FreshFibreDisplacement;
    dy=kM3FreshFibreDisplacement;
    dz=kM3FreshFibreDisplacement;
  }
  const int x=static_cast<int>(std::llround(core.center.x))+dx;
  const int y=static_cast<int>(std::llround(core.center.y))+dy;
  const int z=static_cast<int>(std::llround(core.center.z))+dz;
  state.electric.x[state.electric.index(x,y,z)]+=kM3FreshFibreAmplitude;
  state.electric.z[state.electric.index(x+1,y,z)]+=kM3FreshFibreAmplitude;
  state.electric.x[state.electric.index(x,y,z+1)]-=kM3FreshFibreAmplitude;
  state.electric.z[state.electric.index(x,y,z)]-=kM3FreshFibreAmplitude;
}

M3FibreComparison ftd0760_run_fibre(
    int L,const Direction& direction,
    const ConnectedMooreBlockOptions& options,double interaction_scale) {
  M3FibreComparison result;
  result.volume=L;
  auto parent=ftd0760_build_checkpoint(L,direction,options,interaction_scale);
  if(!parent.valid) return result;
  auto remote=parent.state;
  ftd0760_add_remote_plaquette(remote,direction,options);
  result.initial_global_energy_difference=std::abs(
      ftd::eft::matched_modified_energy(
          remote.electric,remote.magnetic_half,options.wave_speed*options.dt)
      -ftd::eft::matched_modified_energy(
          parent.state.electric,parent.state.magnetic_half,
          options.wave_speed*options.dt));
  result.baseline=ftd0760_run_history(L,direction,"fibre_baseline",
      parent.state,options,interaction_scale,kM3FibreTicks,true,24);
  result.remote=ftd0760_run_history(L,direction,"fibre_remote",
      std::move(remote),options,interaction_scale,kM3FibreTicks,true,24);
  if(result.baseline.rows.size()!=result.remote.rows.size()
      ||result.baseline.checkpoints.size()!=result.remote.checkpoints.size())
    return result;
  for(std::size_t i=0;i<result.baseline.rows.size();++i) {
    const auto& a=result.baseline.rows[i];
    const auto& b=result.remote.rows[i];
    if(a.member!=b.member) ++result.class_mismatches;
    result.maximum_core_difference=std::max({
        result.maximum_core_difference,std::abs(a.graph_margin-b.graph_margin),
        std::abs(a.energy_margin-b.energy_margin),
        std::abs(a.pair_energy-b.pair_energy)});
    result.maximum_constituent_difference=std::max({
        result.maximum_constituent_difference,
        m3_vector_difference(a.relative_position,b.relative_position),
        m3_vector_difference(a.p0,b.p0),m3_vector_difference(a.p1,b.p1)});
    const auto& ca=result.baseline.checkpoints[i];
    const auto& cb=result.remote.checkpoints[i];
    result.maximum_local_field_difference=std::max(
        result.maximum_local_field_difference,
        m3_array_difference(ca.local_field,cb.local_field));
    result.maximum_bound_energy_difference=std::max(
        result.maximum_bound_energy_difference,
        m3_array_difference(ca.bound_energies,cb.bound_energies));
  }
  result.valid=result.baseline.passed&&result.remote.passed
      &&result.initial_global_energy_difference>1e-12
      &&result.class_mismatches==0
      &&result.maximum_core_difference<=kM3VolumeGate
      &&result.maximum_constituent_difference<=kM3VolumeGate
      &&result.maximum_local_field_difference<=kM3VolumeGate
      &&result.maximum_bound_energy_difference<=kM3VolumeGate;
  return result;
}

void ftd0760_write_candidate(
    const M3History& small,const M3History& large,
    const M3VolumeComparison& comparison) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0760";
  std::filesystem::create_directories(directory);
  const auto stem="ftd_0760_m3_candidate_v1_"+small.direction+"_"
      +small.variant;
  std::ofstream csv(directory/(stem+".csv"));
  csv<<"volume,direction,variant,registered_name,tick,member,graph_margin,"
      "energy_margin,pair_energy,rx,ry,rz,p0x,p0y,p0z,p1x,p1y,p1z,"
      "step_valid,common,max_residual,energy_residual,recoil_defect,"
      "speed_excess,regularity_measured,sigma_min,condition_number,"
      "scale_difference,site_hops,graph_connected,graph_local,"
      "relational_edge_before,relational_edge_after,"
      "relational_graph_changed,site_projection_valid,"
      "allow_shared_anchor_chart,chart_admissible,checkpoint,"
      "observer_valid,ladder_valid\n"<<std::setprecision(17);
  for(const auto* history:{&small,&large}) for(const auto& row:history->rows)
    csv<<history->volume<<','<<history->direction<<','<<history->variant<<','
       <<history->registered_name<<','<<row.tick<<','<<row.member<<','
       <<row.graph_margin<<','<<row.energy_margin<<','<<row.pair_energy<<','
       <<row.relative_position.x<<','<<row.relative_position.y<<','
       <<row.relative_position.z<<','<<row.p0.x<<','<<row.p0.y<<','
       <<row.p0.z<<','<<row.p1.x<<','<<row.p1.y<<','<<row.p1.z<<','
       <<row.step_valid<<','<<row.common<<','<<row.step.maximum_residual<<','
       <<row.step.energy_residual<<','<<row.step.recoil_defect<<','
       <<row.step.speed_excess<<','<<row.step.regularity_measured<<','
       <<row.step.minimum_singular_value<<','<<row.step.condition_number<<','
       <<row.step.scale_difference<<','<<row.step.site_hops<<','
       <<row.step.graph_connected<<','<<row.step.graph_local<<','
       <<row.step.relational_edge_before<<','<<row.step.relational_edge_after
       <<','<<row.step.relational_graph_changed<<','
       <<row.step.site_projection_valid<<','<<kM3AllowSharedAnchorChart<<','
       <<(kM3AllowSharedAnchorChart||row.step.site_projection_valid)<<','
       <<row.checkpoint<<','
       <<row.observer_valid<<','<<row.ladder_valid<<'\n';
  std::ofstream json(directory/(stem+".json"));
  json<<std::setprecision(17)
      <<"{\n  \"ftd_id\": \"FTD-0760\",\n"
      <<"  \"protocol_sha256\": \""<<kM3RelationalValidationProtocolSha256
      <<"\",\n  \"direction\": \""<<small.direction<<"\",\n"
      <<"  \"variant\": \""<<small.variant<<"\",\n"
      <<"  \"registered_name\": \""<<small.registered_name<<"\",\n"
      <<"  \"small_initialized\": "<<small.initialized<<",\n"
      <<"  \"small_executed\": "<<small.executed<<",\n"
      <<"  \"small_pass\": "<<small.passed<<",\n"
      <<"  \"large_initialized\": "<<large.initialized<<",\n"
      <<"  \"large_executed\": "<<large.executed<<",\n"
      <<"  \"large_pass\": "<<large.passed<<",\n"
      <<"  \"volume_comparison_pass\": "<<comparison.valid<<",\n"
      <<"  \"maximum_core_difference\": "
      <<comparison.maximum_core_difference<<",\n"
      <<"  \"maximum_constituent_difference\": "
      <<comparison.maximum_constituent_difference<<",\n"
      <<"  \"maximum_local_field_difference\": "
      <<comparison.maximum_local_field_difference<<",\n"
      <<"  \"class_mismatches\": "<<comparison.class_mismatches<<",\n"
      <<"  \"branch_mismatches\": "<<comparison.branch_mismatches<<",\n"
      <<"  \"allow_shared_anchor_chart\": true,\n"
      <<"  \"held_out_validation\": true,\n"
      <<"  \"dynamics_changed\": false\n}\n";
}

void ftd0760_write_fibre(const std::string& direction,
                         const std::vector<M3FibreComparison>& values) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0760";
  std::filesystem::create_directories(directory);
  const auto stem="ftd_0760_m3_causal_fibre_v1_"+direction;
  std::ofstream csv(directory/(stem+".csv"));
  csv<<"volume,tick,baseline_member,remote_member,graph_difference,"
      "energy_difference,constituent_difference,local_field_difference,"
      "bound_energy_difference,initial_global_energy_difference\n"
      <<std::setprecision(17);
  for(const auto& value:values)
    for(std::size_t i=0;i<value.baseline.rows.size();++i) {
      const auto& a=value.baseline.rows[i];
      const auto& b=value.remote.rows[i];
      csv<<value.volume<<','<<a.tick<<','<<a.member<<','<<b.member<<','
         <<std::abs(a.graph_margin-b.graph_margin)<<','
         <<std::abs(a.energy_margin-b.energy_margin)<<','
         <<std::max({m3_vector_difference(a.relative_position,b.relative_position),
                     m3_vector_difference(a.p0,b.p0),
                     m3_vector_difference(a.p1,b.p1)})<<','
         <<m3_array_difference(value.baseline.checkpoints[i].local_field,
                               value.remote.checkpoints[i].local_field)<<','
         <<m3_array_difference(value.baseline.checkpoints[i].bound_energies,
                               value.remote.checkpoints[i].bound_energies)<<','
         <<value.initial_global_energy_difference<<'\n';
    }
  bool pass=true;
  for(const auto& value:values) pass=pass&&value.valid;
  std::ofstream json(directory/(stem+".json"));
  json<<std::setprecision(17)
      <<"{\n  \"ftd_id\": \"FTD-0760\",\n"
      <<"  \"protocol_sha256\": \""<<kM3RelationalValidationProtocolSha256
      <<"\",\n  \"direction\": \""<<direction<<"\",\n"
      <<"  \"volumes\": [321,385],\n"
      <<"  \"fibre_pass\": "<<pass<<",\n"
      <<"  \"records\": [\n";
  for(std::size_t i=0;i<values.size();++i) {
    const auto& value=values[i];
    json<<"    {\"volume\": "<<value.volume
        <<", \"valid\": "<<value.valid
        <<", \"baseline_initialized\": "<<value.baseline.initialized
        <<", \"baseline_executed\": "<<value.baseline.executed
        <<", \"baseline_pass\": "<<value.baseline.passed
        <<", \"remote_initialized\": "<<value.remote.initialized
        <<", \"remote_executed\": "<<value.remote.executed
        <<", \"remote_pass\": "<<value.remote.passed
        <<", \"initial_global_energy_difference\": "
        <<value.initial_global_energy_difference
        <<", \"class_mismatches\": "<<value.class_mismatches
        <<", \"maximum_core_difference\": "
        <<value.maximum_core_difference
        <<", \"maximum_constituent_difference\": "
        <<value.maximum_constituent_difference
        <<", \"maximum_local_field_difference\": "
        <<value.maximum_local_field_difference
        <<", \"maximum_bound_energy_difference\": "
        <<value.maximum_bound_energy_difference<<"}"
        <<(i+1==values.size()?"\n":",\n");
  }
  json<<"  ],\n  \"allow_shared_anchor_chart\": true,\n"
      <<"  \"held_out_validation\": true,\n"
      <<"  \"dynamics_changed\": false\n}\n";
}

}  // namespace

int run(int argc,char** argv) {
  const bool qualification=argc==4&&std::string(argv[1])=="--qualify";
  const bool candidates=argc==3&&std::string(argv[1])=="--candidates";
  const bool fibre=argc==3&&std::string(argv[1])=="--fibre";
  if(!qualification&&!candidates&&!fibre) {
    std::cout<<"FTD-0760 runner: --qualify face 1; registered "
        "--candidates face|edge|body; --fibre face|edge|body\n";
    return argc==1?0:2;
  }
  const std::string slug=argv[2];
  Direction direction;
  if(!select_horizon_direction(slug,direction)) return 2;
  if(qualification&&(slug!="face"||std::stoi(argv[3])!=1)) return 2;
  if((candidates||fibre)
      &&std::string(kM3RelationalValidationProtocolSha256)=="UNLOCKED") {
    std::cerr<<"FTD-0760 registered execution refused before protocol lock\n";
    return 3;
  }
  const auto normalization=ftd::eft::measure_face_flux_normalization();
  if(!normalization.valid) return 1;
  auto options=m3_options();
  const double interaction_scale=
      normalization.mapped_field_work_coefficient;
  if(qualification) {
    auto parent=ftd0760_build_checkpoint(
        321,direction,options,interaction_scale);
    if(!parent.valid) return 1;
    auto initial=ftd0760_make_variant(parent,direction,"center",options);
    if(!initial.valid) return 1;
    auto history=ftd0760_run_history(321,direction,"qualification",
        std::move(initial.state),options,interaction_scale,1,false,48,true,true);
    const bool pass=history.executed&&history.passed;
    std::cout<<"FTD-0760 qualification pass="<<pass
        <<" rows="<<history.rows.size()<<'\n';
    return pass?0:1;
  }
  if(candidates) {
    constexpr std::array<const char*,3> variants{{
        "center","energy_hostile","graph_hostile"}};
    std::array<std::array<M3History,2>,3> histories;
    for(std::size_t volume_index=0;volume_index<kM3Volumes.size();++volume_index) {
      const int L=kM3Volumes[volume_index];
      auto parent=ftd0760_build_checkpoint(
          L,direction,options,interaction_scale);
      for(std::size_t variant_index=0;variant_index<variants.size();++variant_index) {
        const std::string variant=variants[variant_index];
        auto initial=ftd0760_make_variant(parent,direction,variant,options);
        M3History history;
        if(initial.valid)
          history=ftd0760_run_history(L,direction,variant,
              std::move(initial.state),options,interaction_scale);
        else {
          history.volume=L;
          history.direction=direction.label;
          history.variant=variant;
        }
        history.initialized=initial.valid;
        history.registered_name=initial.registered_name;
        history.root_residual=initial.root_residual;
        history.nearest_shell_margin=initial.nearest_shell_margin;
        history.passed=history.passed&&initial.valid;
        histories[variant_index][volume_index]=std::move(history);
      }
    }
    bool pass=true;
    for(std::size_t variant_index=0;variant_index<variants.size();++variant_index) {
      const auto comparison=m3_compare_volumes(
          histories[variant_index][0],histories[variant_index][1]);
      ftd0760_write_candidate(histories[variant_index][0],
          histories[variant_index][1],comparison);
      pass=pass&&histories[variant_index][0].passed
          &&histories[variant_index][1].passed&&comparison.valid;
    }
    std::cout<<"FTD-0760 candidates direction="<<slug<<" pass="<<pass<<'\n';
    return pass?0:1;
  }
  std::vector<M3FibreComparison> values;
  values.reserve(2);
  for(int L:kM3Volumes)
    values.push_back(ftd0760_run_fibre(
        L,direction,options,interaction_scale));
  ftd0760_write_fibre(direction.label,values);
  const bool pass=std::all_of(values.begin(),values.end(),
      [](const auto& value) { return value.valid; });
  std::cout<<"FTD-0760 fibre direction="<<slug<<" pass="<<pass<<'\n';
  return pass?0:1;
}

}  // namespace ftd0760_embedded

int main(int argc,char** argv) {
  return ftd0760_embedded::run(argc,argv);
}
