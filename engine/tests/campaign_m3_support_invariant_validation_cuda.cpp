/** FTD-0755: support-invariant finite-time matter-family validation runner. */

#define FTD_0754_MAIN_NAME ftd_0755_embedded_0754_main
#include "campaign_state_only_observer_discovery_cuda.cpp"
#undef FTD_0754_MAIN_NAME

#include "ftd/eft/support_invariant_matter_predicate.h"

#include <bitset>
#include <cstring>
#include <memory>

namespace {

constexpr char kM3ValidationProtocolSha256[]=
    "1E713DB4B997DAED0D55F098A6E7D63FC0F2D773391CE44FFE03AADD92A504BC";
constexpr int kM3FormationTick=160;
constexpr int kM3FinalTick=312;
constexpr int kM3ContinuationTicks=kM3FinalTick-kM3FormationTick;
constexpr std::array<int,2> kM3Volumes{{321,385}};
constexpr std::array<int,5> kM3ObserverTicks{{160,200,240,280,312}};
constexpr std::array<int,3> kM3Supports{{4,6,8}};
constexpr double kM3CoreMargin=1e-6;
constexpr double kM3CommonGate=1e-10;
constexpr double kM3RecoilGate=1e-9;
constexpr double kM3EnergyGate=1e-8;
constexpr double kM3SpeedGate=1e-12;
constexpr double kM3SigmaGate=1e-3;
constexpr double kM3ConditionGate=1e4;
constexpr double kM3ScaleGate=1e-5;
constexpr double kM3VolumeGate=2e-13;
constexpr double kM3CornerImpulse=0.0006;
constexpr double kM3FieldScale=0.95;
constexpr int kM3FibreTicks=64;
constexpr int kM3FibreDisplacement=96;
constexpr double kM3FibreAmplitude=1e-3;

bool m3_observer_tick(int tick) {
  return std::find(kM3ObserverTicks.begin(),kM3ObserverTicks.end(),tick)
      !=kM3ObserverTicks.end();
}

struct M3StepDiagnostics {
  bool valid=false;
  bool common=false;
  int failure_stage=0;
  bool solve_attempted=false;
  bool solve_converged=false;
  int solve_iterations=0;
  double solve_residual=INFINITY;
  int site_hops=0;
  bool graph_connected=false;
  bool graph_local=false;
  bool relational_edge_before=false;
  bool relational_edge_after=false;
  bool relational_graph_changed=false;
  bool site_projection_valid=false;
  double maximum_residual=INFINITY;
  double energy_residual=INFINITY;
  double recoil_defect=INFINITY;
  double speed_excess=INFINITY;
  bool regularity_measured=false;
  double minimum_singular_value=0.0;
  double condition_number=INFINITY;
  double scale_difference=INFINITY;
};

class M3CudaStepper {
 public:
  M3CudaStepper(ConnectedMooreBlockState initial,
                ConnectedMooreBlockOptions input_options,
                double interaction_scale,bool measure_regularity)
      : state_(std::move(initial)),options_(std::move(input_options)),
        interaction_scale_(interaction_scale),pipeline_(state_.electric.L),
        prepared_magnetic_(state_.electric.L),
        prepared_electric_(state_.electric.L) {
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
        ||!pipeline_.download_prepared(
            prepared_magnetic_,prepared_electric_)) {
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
    const Vec3 observer_center=state_.constituents.size()==2
        ?(effective_position(state_.constituents[0])
          +effective_position(state_.constituents[1]))*0.5
        :Vec3{};
    const auto profile=pipeline_.observe_deterministic(
        lambda,observer_center,{8},kM3CommonGate);
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
  ftd::eft::CudaMatchedFieldPipeline pipeline_;
  ftd::eft::MatchedEdgeField prepared_magnetic_;
  ftd::eft::MatchedFaceFlux prepared_electric_;
  ConnectedMooreBlockSolveCache cache_;
  bool valid_=false;
};

struct M3ParentCheckpoint {
  bool valid=false;
  int volume=0;
  std::string direction;
  ConnectedMooreBlockState state;
  explicit M3ParentCheckpoint(int L=0):volume(L),state(L) {}
};

M3ParentCheckpoint m3_build_checkpoint(
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
  M3CudaStepper stepper(
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

struct M3CornerSpec {
  int sigma_r=0,sigma_1=0,sigma_2=0;
  std::string name;
};

M3CornerSpec m3_corner_spec(
    const std::string& direction,const std::string& variant) {
  if(variant=="center") return {0,0,0,"center"};
  if(direction=="0_0_1"&&variant=="energy_hostile")
    return {+1,+1,-1,"srp_s1p_s2m_rin_fminus"};
  if(direction=="0_0_1"&&variant=="graph_hostile")
    return {+1,-1,-1,"srp_s1m_s2m_rin_fminus"};
  if(direction=="0_1_-1"&&variant=="energy_hostile")
    return {+1,-1,-1,"srp_s1m_s2m_rin_fminus"};
  if(direction=="0_1_-1"&&variant=="graph_hostile")
    return {+1,-1,+1,"srp_s1m_s2p_rin_fminus"};
  if(direction=="1_1_1"&&variant=="energy_hostile")
    return {+1,-1,-1,"srp_s1m_s2m_rin_fminus"};
  if(direction=="1_1_1"&&variant=="graph_hostile")
    return {+1,+1,-1,"srp_s1p_s2m_rin_fminus"};
  return {};
}

double m3_selected_potential(double d,
                             const ConnectedMooreBlockOptions& options) {
  if(d>=options.compact_pair_cutoff_distance_squared) return 0.0;
  return -16.0*options.compact_pair_well_depth
      *(d-1.5)*(d-1.5)*(d-0.75);
}

double m3_kinetic(const ConnectedMooreBlockState& state,
                  const ConnectedMooreBlockOptions& options) {
  long double value=0.0L;
  const double rest=options.constituent_mass_scale*ftd::E_REST;
  for(const auto& point:state.constituents)
    value+=std::sqrt(rest*rest+ftd::C_SPEED*ftd::C_SPEED
        *point.momentum.mag2())-rest;
  return static_cast<double>(value);
}

std::pair<Vec3,Vec3> m3_tangents(const Direction& direction) {
  if(direction.label==std::string("0_0_1"))
    return {{1.0,0.0,0.0},{0.0,1.0,0.0}};
  if(direction.label==std::string("0_1_-1"))
    return {{1.0,0.0,0.0},
            {0.0,1.0/std::sqrt(2.0),1.0/std::sqrt(2.0)}};
  return {{1.0/std::sqrt(2.0),-1.0/std::sqrt(2.0),0.0},
          {1.0/std::sqrt(6.0),1.0/std::sqrt(6.0),
           -2.0/std::sqrt(6.0)}};
}

std::pair<double,double> m3_root(
    double kinetic,double left,double right,
    const ConnectedMooreBlockOptions& options) {
  const auto value=[&](double d) {
    return kinetic+m3_selected_potential(d,options);
  };
  double f_left=value(left),f_right=value(right);
  if(!(std::isfinite(f_left)&&std::isfinite(f_right)
      &&f_left*f_right<0.0)) return {NAN,INFINITY};
  for(int iteration=0;iteration<160;++iteration) {
    const double middle=0.5*(left+right);
    const double f_middle=value(middle);
    if(f_left*f_middle<=0.0) {
      right=middle;
      f_right=f_middle;
    } else {
      left=middle;
      f_left=f_middle;
    }
  }
  const double root=0.5*(left+right);
  return {root,std::abs(value(root))};
}

struct M3VariantState {
  bool valid=false;
  std::string registered_name;
  double root_residual=INFINITY;
  double nearest_shell_margin=0.0;
  ConnectedMooreBlockState state;
  explicit M3VariantState(int L=0):state(L) {}
};

void m3_add_scaled_residual(
    ftd::eft::MatchedFaceFlux& output,
    const ftd::eft::MatchedFaceFlux& parent,
    const ftd::eft::MatchedFaceFlux& bound_parent,double scale) {
  for(std::size_t i=0;i<output.x.size();++i) {
    output.x[i]+=scale*(parent.x[i]-bound_parent.x[i]);
    output.y[i]+=scale*(parent.y[i]-bound_parent.y[i]);
    output.z[i]+=scale*(parent.z[i]-bound_parent.z[i]);
  }
}

M3VariantState m3_make_variant(
    const M3ParentCheckpoint& parent,const Direction& direction,
    const std::string& variant,const ConnectedMooreBlockOptions& options) {
  const int L=parent.volume;
  M3VariantState result(L);
  const auto spec=m3_corner_spec(direction.label,variant);
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
      *(kM3CornerImpulse/std::sqrt(3.0));
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
  const double target_d=parent_d-0.5*result.nearest_shell_margin;
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
      bound_parent.state.electric,kM3FieldScale);
  for(std::size_t i=0;i<result.state.magnetic_half.x.size();++i) {
    result.state.magnetic_half.x[i]=
        kM3FieldScale*parent.state.magnetic_half.x[i];
    result.state.magnetic_half.y[i]=
        kM3FieldScale*parent.state.magnetic_half.y[i];
    result.state.magnetic_half.z[i]=
        kM3FieldScale*parent.state.magnetic_half.z[i];
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

struct M3Row {
  int volume=0,tick=0;
  bool member=false,step_valid=true,common=true,checkpoint=false;
  bool observer_valid=true,ladder_valid=true;
  double graph_margin=0.0,energy_margin=0.0,pair_energy=0.0;
  Vec3 relative_position{},p0{},p1{};
  M3StepDiagnostics step{};
};

struct M3CheckpointRecord {
  int tick=0;
  bool observer_valid=false,ladder_valid=false;
  std::vector<double> local_field;
  std::vector<double> bound_energies;
};

std::vector<double> m3_local_field(
    const ConnectedMooreBlockState& state,const Vec3& center,int radius) {
  const int L=state.electric.L;
  const int cx=static_cast<int>(std::llround(center.x));
  const int cy=static_cast<int>(std::llround(center.y));
  const int cz=static_cast<int>(std::llround(center.z));
  const std::size_t sites=static_cast<std::size_t>(2*radius+1)
      *(2*radius+1)*(2*radius+1);
  std::vector<double> result;
  result.reserve(6*sites);
  for(int dx=-radius;dx<=radius;++dx)
    for(int dy=-radius;dy<=radius;++dy)
      for(int dz=-radius;dz<=radius;++dz) {
        const auto i=static_cast<std::size_t>(
            state.electric.index(cx+dx,cy+dy,cz+dz));
        result.push_back(state.electric.x[i]);
        result.push_back(state.electric.y[i]);
        result.push_back(state.electric.z[i]);
        result.push_back(state.magnetic_half.x[i]);
        result.push_back(state.magnetic_half.y[i]);
        result.push_back(state.magnetic_half.z[i]);
      }
  return result;
}

struct M3History {
  bool initialized=false,executed=false,passed=false;
  int volume=0;
  std::string direction,variant,registered_name;
  double root_residual=INFINITY,nearest_shell_margin=0.0;
  double minimum_graph_margin=INFINITY,minimum_energy_margin=INFINITY;
  double minimum_sigma=INFINITY,maximum_condition=0.0;
  double maximum_scale_difference=0.0,maximum_common=0.0;
  double maximum_energy=0.0,maximum_recoil=0.0,maximum_speed=0.0;
  std::vector<M3Row> rows;
  std::vector<M3CheckpointRecord> checkpoints;
};

M3Row m3_make_row(int volume,int tick,
                   const ConnectedMooreBlockState& state,
                   const ConnectedMooreBlockOptions& options,
                   const M3StepDiagnostics* step=nullptr) {
  M3Row row;
  row.volume=volume;
  row.tick=tick;
  const auto core=ftd::eft::observe_support_invariant_matter(state,options);
  row.member=core.member;
  row.graph_margin=core.graph_margin;
  row.energy_margin=core.energy_margin;
  row.pair_energy=core.pair_energy;
  row.relative_position=core.relative_position;
  if(state.constituents.size()==2) {
    row.p0=state.constituents[0].momentum;
    row.p1=state.constituents[1].momentum;
  }
  if(step) {
    row.step=*step;
    row.step_valid=step->valid;
    row.common=step->common;
  }
  return row;
}

M3CheckpointRecord m3_make_checkpoint_record(
    int tick,const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& options,int local_radius) {
  M3CheckpointRecord result;
  result.tick=tick;
  ftd::eft::StateOnlyMatterFieldObserverOptions observer;
  observer.support_half_width=4;
  observer.shell_radii=state.electric.L>=97
      ?std::vector<int>{8,12,16,24,32,48}
      :std::vector<int>{8,12};
  observer.wave_speed=options.wave_speed;
  observer.dt=options.dt;
  const auto field=ftd::eft::observe_state_only_matter_field(
      state,options,observer);
  const std::vector<int> supports(kM3Supports.begin(),kM3Supports.end());
  const auto ladder=ftd::eft::observe_state_only_support_ladder(
      state,options,supports,1e-13,4096,1e-12);
  result.observer_valid=field.valid&&field.boundary_energy_ledger_valid;
  result.ladder_valid=ladder.valid;
  for(const auto& scale:ladder.scales)
    result.bound_energies.push_back(scale.bound_face_energy);
  result.local_field=m3_local_field(state,field.center,local_radius);
  return result;
}

M3History m3_run_history(
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
  M3CudaStepper stepper(
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

double m3_vector_difference(const Vec3& a,const Vec3& b) {
  return std::max({std::abs(a.x-b.x),std::abs(a.y-b.y),std::abs(a.z-b.z)});
}

double m3_array_difference(
    const std::vector<double>& a,const std::vector<double>& b) {
  if(a.size()!=b.size()) return INFINITY;
  double maximum=0.0;
  for(std::size_t i=0;i<a.size();++i)
    maximum=std::max(maximum,std::abs(a[i]-b[i]));
  return maximum;
}

struct M3VolumeComparison {
  bool valid=false;
  double maximum_core_difference=0.0;
  double maximum_constituent_difference=0.0;
  double maximum_local_field_difference=0.0;
  int class_mismatches=0;
  int branch_mismatches=0;
};

M3VolumeComparison m3_compare_volumes(
    const M3History& small,const M3History& large) {
  M3VolumeComparison result;
  if(!small.initialized||!large.initialized
      ||!small.executed||!large.executed
      ||small.rows.size()!=large.rows.size()
      ||small.checkpoints.size()!=large.checkpoints.size()) return result;
  for(std::size_t i=0;i<small.rows.size();++i) {
    const auto& a=small.rows[i];
    const auto& b=large.rows[i];
    if(a.member!=b.member) ++result.class_mismatches;
    if(a.step.site_hops!=b.step.site_hops
        ||a.step.graph_connected!=b.step.graph_connected
        ||a.step.graph_local!=b.step.graph_local
        ||a.step.relational_edge_before!=b.step.relational_edge_before
        ||a.step.relational_edge_after!=b.step.relational_edge_after
        ||a.step.relational_graph_changed!=b.step.relational_graph_changed
        ||a.step.site_projection_valid!=b.step.site_projection_valid)
      ++result.branch_mismatches;
    result.maximum_core_difference=std::max({
        result.maximum_core_difference,
        std::abs(a.graph_margin-b.graph_margin),
        std::abs(a.energy_margin-b.energy_margin),
        std::abs(a.pair_energy-b.pair_energy)});
    result.maximum_constituent_difference=std::max({
        result.maximum_constituent_difference,
        m3_vector_difference(a.relative_position,b.relative_position),
        m3_vector_difference(a.p0,b.p0),m3_vector_difference(a.p1,b.p1)});
  }
  for(std::size_t i=0;i<small.checkpoints.size();++i)
    result.maximum_local_field_difference=std::max(
        result.maximum_local_field_difference,
        m3_array_difference(small.checkpoints[i].local_field,
                            large.checkpoints[i].local_field));
  result.valid=result.class_mismatches==0
      &&result.branch_mismatches==0
      &&result.maximum_core_difference<=kM3VolumeGate
      &&result.maximum_constituent_difference<=kM3VolumeGate
      &&result.maximum_local_field_difference<=kM3VolumeGate;
  return result;
}

void m3_write_candidate(
    const M3History& small,const M3History& large,
    const M3VolumeComparison& comparison) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0755";
  std::filesystem::create_directories(directory);
  const auto stem="ftd_0755_m3_candidate_v1_"+small.direction+"_"
      +small.variant;
  std::ofstream csv(directory/(stem+".csv"));
  csv<<"volume,direction,variant,registered_name,tick,member,graph_margin,"
      "energy_margin,pair_energy,rx,ry,rz,p0x,p0y,p0z,p1x,p1y,p1z,"
      "step_valid,common,max_residual,energy_residual,recoil_defect,"
      "speed_excess,regularity_measured,sigma_min,condition_number,"
      "scale_difference,site_hops,graph_connected,graph_local,"
      "relational_edge_before,relational_edge_after,"
      "relational_graph_changed,site_projection_valid,checkpoint,"
      "observer_valid,ladder_valid\n"
      <<std::setprecision(17);
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
       <<row.step.relational_edge_before<<','
       <<row.step.relational_edge_after<<','
       <<row.step.relational_graph_changed<<','
       <<row.step.site_projection_valid<<','<<row.checkpoint<<','
       <<row.observer_valid<<','<<row.ladder_valid<<'\n';
  std::ofstream json(directory/(stem+".json"));
  json<<std::setprecision(17)
      <<"{\n  \"ftd_id\": \"FTD-0755\",\n"
      <<"  \"protocol_sha256\": \""<<kM3ValidationProtocolSha256<<"\",\n"
      <<"  \"direction\": \""<<small.direction<<"\",\n"
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
      <<"  \"held_out_validation\": true,\n"
      <<"  \"dynamics_changed\": false\n}\n";
}

void m3_add_remote_plaquette(
    ConnectedMooreBlockState& state,const Direction& direction,
    const ConnectedMooreBlockOptions& options) {
  const auto core=ftd::eft::observe_support_invariant_matter(
      state,options);
  int dx=0,dy=0,dz=0;
  if(direction.label=="0_0_1") dz=kM3FibreDisplacement;
  else if(direction.label=="0_1_-1") {
    dy=kM3FibreDisplacement; dz=-kM3FibreDisplacement;
  } else {
    dx=kM3FibreDisplacement; dy=kM3FibreDisplacement;
    dz=kM3FibreDisplacement;
  }
  const int x=static_cast<int>(std::llround(core.center.x))+dx;
  const int y=static_cast<int>(std::llround(core.center.y))+dy;
  const int z=static_cast<int>(std::llround(core.center.z))+dz;
  state.electric.x[state.electric.index(x,y,z)]+=kM3FibreAmplitude;
  state.electric.z[state.electric.index(x+1,y,z)]+=kM3FibreAmplitude;
  state.electric.x[state.electric.index(x,y,z+1)]-=kM3FibreAmplitude;
  state.electric.z[state.electric.index(x,y,z)]-=kM3FibreAmplitude;
}

struct M3FibreComparison {
  bool valid=false;
  int volume=0;
  double initial_global_energy_difference=0.0;
  double maximum_core_difference=0.0;
  double maximum_constituent_difference=0.0;
  double maximum_local_field_difference=0.0;
  double maximum_bound_energy_difference=0.0;
  int class_mismatches=0;
  M3History baseline,remote;
};

M3FibreComparison m3_run_fibre(
    int L,const Direction& direction,
    const ConnectedMooreBlockOptions& options,double interaction_scale) {
  M3FibreComparison result;
  result.volume=L;
  auto parent=m3_build_checkpoint(L,direction,options,interaction_scale);
  if(!parent.valid) return result;
  auto remote=parent.state;
  m3_add_remote_plaquette(remote,direction,options);
  result.initial_global_energy_difference=std::abs(
      ftd::eft::matched_modified_energy(
          remote.electric,remote.magnetic_half,options.wave_speed*options.dt)
      -ftd::eft::matched_modified_energy(
          parent.state.electric,parent.state.magnetic_half,
          options.wave_speed*options.dt));
  result.baseline=m3_run_history(L,direction,"fibre_baseline",
      parent.state,options,interaction_scale,kM3FibreTicks,true,24);
  result.remote=m3_run_history(L,direction,"fibre_remote",
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

void m3_write_fibre(const std::string& direction,
                    const std::vector<M3FibreComparison>& values) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0755";
  std::filesystem::create_directories(directory);
  const auto stem="ftd_0755_m3_causal_fibre_v1_"+direction;
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
      <<"{\n  \"ftd_id\": \"FTD-0755\",\n"
      <<"  \"protocol_sha256\": \""<<kM3ValidationProtocolSha256<<"\",\n"
      <<"  \"direction\": \""<<direction<<"\",\n"
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
  json<<"  ],\n"
      <<"  \"held_out_validation\": true,\n"
      <<"  \"dynamics_changed\": false\n}\n";
}

ConnectedMooreBlockOptions m3_options(int max_iterations=384) {
  ConnectedMooreBlockOptions options;
  options.dt=0.25;
  options.binding_law=ConnectedBindingLaw::DerivedCompactPair;
  options.compact_pair_well_depth=0.01;
  options.compact_pair_cutoff_distance_squared=1.5;
  options.allow_shared_anchor_chart=true;
  options.gate_tolerance=kGate;
  options.solve_tolerance=2e-14;
  options.max_iterations=max_iterations;
  options.use_sparse_local_current=true;
  options.use_local_residual_evaluation=true;
  return options;
}

}  // namespace

int main(int argc,char** argv) {
  const bool qualification=argc==4&&std::string(argv[1])=="--qualify";
  const bool candidate=argc==4&&std::string(argv[1])=="--candidate";
  const bool fibre=argc==3&&std::string(argv[1])=="--fibre";
  if(!qualification&&!candidate&&!fibre) {
    std::cout<<"FTD-0755 runner: --qualify face|edge|body N; registered "
        "--candidate face|edge|body center|energy_hostile|graph_hostile; "
        "--fibre face|edge|body\n";
    return argc==1?0:2;
  }
  const std::string slug=argv[2];
  Direction direction;
  if(!select_horizon_direction(slug,direction)) return 2;
  if((candidate||fibre)
      &&std::string(kM3ValidationProtocolSha256)=="UNLOCKED") {
    std::cerr<<"FTD-0755 registered execution refused before protocol lock\n";
    return 3;
  }
  const auto normalization=ftd::eft::measure_face_flux_normalization();
  if(!normalization.valid) return 1;
  auto options=m3_options();
  const double interaction_scale=
      normalization.mapped_field_work_coefficient;
  if(qualification) {
    constexpr int qualification_volume=321;
    const int ticks=std::stoi(argv[3]);
    if(ticks<1||ticks>8) return 2;
    auto preparation=ftd::eft::prepare_finite_support_derived_compact_pair(
        make_geometry(qualification_volume,direction,false,1.30,0.0120),
        options,4,1e-13,4096);
    if(!preparation.valid) return 1;
    auto result=m3_run_history(qualification_volume,direction,"qualification",
        std::move(preparation.state),options,interaction_scale,
        ticks,false,48,false,false);
    std::cout<<"FTD-0755 qualification direction="<<slug
        <<" executed="<<result.executed<<" passed="<<result.passed
        <<" rows="<<result.rows.size();
    if(!result.rows.empty()) {
      const auto& row=result.rows.back();
      std::cout<<" last_tick="<<row.tick<<" member="<<row.member
          <<" graph_margin="<<row.graph_margin
          <<" energy_margin="<<row.energy_margin
          <<" step_valid="<<row.step_valid<<" common="<<row.common
          <<" failure_stage="<<row.step.failure_stage
          <<" solve_attempted="<<row.step.solve_attempted
          <<" solve_converged="<<row.step.solve_converged
          <<" solve_iterations="<<row.step.solve_iterations
          <<" solve_residual="<<row.step.solve_residual
          <<" residual="<<row.step.maximum_residual
          <<" energy_residual="<<row.step.energy_residual
          <<" recoil="<<row.step.recoil_defect
          <<" speed="<<row.step.speed_excess
          <<" regularity="<<row.step.regularity_measured
          <<" sigma="<<row.step.minimum_singular_value
          <<" condition="<<row.step.condition_number
          <<" scale_difference="<<row.step.scale_difference
          <<" observer="<<row.observer_valid
          <<" ladder="<<row.ladder_valid;
    }
    std::cout<<'\n';
    return result.passed?0:1;
  }
  if(candidate) {
    const std::string variant=argv[3];
    if(variant!="center"&&variant!="energy_hostile"
        &&variant!="graph_hostile") return 2;
    std::vector<M3History> histories;
    histories.reserve(2);
    for(int L:kM3Volumes) {
      auto parent=m3_build_checkpoint(
          L,direction,options,interaction_scale);
      auto initial=m3_make_variant(parent,direction,variant,options);
      M3History history;
      if(initial.valid)
        history=m3_run_history(L,direction,variant,
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
      histories.push_back(std::move(history));
    }
    const auto comparison=m3_compare_volumes(histories[0],histories[1]);
    m3_write_candidate(histories[0],histories[1],comparison);
    const bool pass=histories[0].passed&&histories[1].passed&&comparison.valid;
    std::cout<<"FTD-0755 candidate direction="<<slug
        <<" variant="<<variant<<" pass="<<pass<<'\n';
    return pass?0:1;
  }
  std::vector<M3FibreComparison> values;
  values.reserve(2);
  for(int L:kM3Volumes)
    values.push_back(m3_run_fibre(
        L,direction,options,interaction_scale));
  m3_write_fibre(direction.label,values);
  const bool pass=std::all_of(values.begin(),values.end(),
      [](const auto& value) { return value.valid; });
  std::cout<<"FTD-0755 fibre direction="<<slug<<" pass="<<pass<<'\n';
  return pass?0:1;
}
