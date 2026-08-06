/** FTD-0764: transported-chart morphology and momentum replay on CUDA. */

#pragma push_macro("main")
#undef main
#define main ftd0763_reference_main
#include "campaign_fractional_center_dressing_observer_cuda.cpp"
#undef main
#pragma pop_macro("main")

#include "ftd/eft/cuda_transported_chart_morphology.h"

namespace {

using namespace ftd;
using namespace ftd::eft;

constexpr char kFtd0764ProtocolSha256[] =
    "4F68CCD8A037363438CF94B728C56059066BFA9B2B3D8C0F82A6A5DDF3D7BDF8";
constexpr int kFtd0764Volume=321;
constexpr int kFtd0764Ticks=64;
constexpr double kFtd0764CommonGate=1e-10;
constexpr std::array<int,5> kFtd0764Checkpoints{{160,176,192,208,224}};

std::array<std::array<int,3>,3> morphology_basis(const std::string& slug) {
  if(slug=="face") return {{{0,0,1},{1,0,0},{0,1,0}}};
  if(slug=="edge") return {{{0,1,-1},{1,0,0},{0,1,1}}};
  return {{{1,1,1},{1,-1,0},{1,1,-2}}};
}

bool morphology_checkpoint(int tick) {
  return std::find(kFtd0764Checkpoints.begin(),kFtd0764Checkpoints.end(),tick)
      !=kFtd0764Checkpoints.end();
}

double common_residual_0764(const ConnectedMooreBlockStepResult& step) {
  return std::max({step.root_residual,step.continuity_residual,
      step.gauss_before_residual,step.gauss_after_residual,
      step.force_residual,step.kinematic_residual,
      step.kinetic_discrete_gradient_residual,
      step.electric_adjoint_residual,step.magnetic_work_residual,
      step.binding_work_residual,step.binding_impulse_sum_residual,
      step.matter_work_residual,step.field_work_residual,
      step.total_energy_residual});
}

struct MorphologyStep {
  bool valid=false;
  bool common=false;
  bool inverse_valid=false;
  bool regularity_measured=false;
  Vec3 local_before{},local_after{};
  Vec3 spline_before{},spline_after{};
  double common_residual=INFINITY;
  double energy_residual=INFINITY;
  double energy_before=INFINITY;
  double energy_after=INFINITY;
  double speed_excess=INFINITY;
  double sigma_min=0.0;
  double condition=INFINITY;
  double inverse_residual=INFINITY;
};

class MorphologyCudaStepper {
 public:
  MorphologyCudaStepper(ConnectedMooreBlockState initial,
                        ConnectedMooreBlockOptions options,double beta)
      : state_(std::move(initial)),options_(std::move(options)),beta_(beta),
        pipeline_(state_.electric.L),prepared_b_(state_.electric.L),
        prepared_e_(state_.electric.L) {
    const double c=static_cast<double>(state_.electric.L/2);
    fixed_center_={c,c,c};
    options_.defer_volume_diagnostics=true;
    valid_=pipeline_.valid()
        &&pipeline_.upload(state_.electric,state_.magnetic_half);
  }

  bool valid() const { return valid_; }
  const ConnectedMooreBlockState& state() const { return state_; }
  ConnectedMooreBlockState release_state() { return std::move(state_); }

  MorphologyStep advance(bool registered_checkpoint) {
    MorphologyStep result;
    if(!valid_) return result;
    options_.measure_final_root_regularity=registered_checkpoint;
    const double lambda=options_.wave_speed*options_.dt;
    if(!pipeline_.prepare_forward(lambda)
        ||!pipeline_.download_prepared(prepared_b_,prepared_e_)) {
      valid_=false; return result;
    }
    auto step=solve_connected_moore_block_forward_prepared(
        state_,std::move(prepared_b_),std::move(prepared_e_),
        options_,&forward_cache_);
    if(!step.volume_diagnostics_pending
        ||!pipeline_.apply_ordered_sparse_current(
            step.segments,options_.polarity_scale)) {
      valid_=false; return result;
    }
    const auto profile=pipeline_.observe_deterministic(
        lambda,fixed_center_,{8},kFtd0764CommonGate);
    if(!profile.valid) { valid_=false; return result; }
    const auto diagnostics=pipeline_.diagnose_common_action(
        step.segments,options_.polarity_scale,beta_,
        options_.wave_speed,options_.dt,kFtd0764CommonGate);
    step=complete_connected_moore_block_volume_diagnostics(
        std::move(step),diagnostics,options_);
    result.valid=step.valid&&step.common_action_gates_pass;
    result.common=step.common_action_gates_pass;
    result.local_before=step.local_field_momentum_before;
    result.local_after=step.local_field_momentum_after;
    result.spline_before=step.spline_field_momentum_before;
    result.spline_after=step.spline_field_momentum_after;
    result.common_residual=common_residual_0764(step);
    result.energy_residual=std::abs(step.total_energy_residual);
    result.energy_before=step.kinetic_energy_before+step.binding_energy_before
        +step.field_energy_before;
    result.energy_after=step.kinetic_energy_after+step.binding_energy_after
        +step.field_energy_after;
    result.speed_excess=step.causal_speed_excess;
    result.regularity_measured=step.solve.final_root_regularity_measured;
    result.sigma_min=step.solve.final_minimum_singular_value;
    result.condition=step.solve.final_condition_number;
    if(registered_checkpoint&&result.valid) {
      auto reverse_options=options_;
      reverse_options.defer_volume_diagnostics=false;
      reverse_options.measure_final_root_regularity=false;
      ConnectedMooreBlockSolveCache reverse_cache;
      const auto reverse=solve_connected_moore_block_reverse(
          step.later,reverse_options,&reverse_cache);
      result.inverse_valid=reverse.valid&&reverse.common_action_gates_pass;
      if(result.inverse_valid)
        result.inverse_residual=connected_moore_block_state_max_difference(
            state_,reverse.earlier);
    } else {
      result.inverse_valid=true;
      result.inverse_residual=0.0;
    }
    state_=std::move(step.later);
    if(!pipeline_.advance()) valid_=false;
    valid_=valid_&&result.valid;
    return result;
  }

 private:
  ConnectedMooreBlockState state_;
  ConnectedMooreBlockOptions options_;
  double beta_=0.0;
  Vec3 fixed_center_{};
  CudaMatchedFieldPipeline pipeline_;
  MatchedEdgeField prepared_b_;
  MatchedFaceFlux prepared_e_;
  ConnectedMooreBlockSolveCache forward_cache_;
  bool valid_=false;
};

struct MorphologyCheckpoint {
  int tick=0;
  bool valid=false;
  FractionalCheckpoint field;
  TransportedChartMorphologyObservation morphology;
  CudaTransportedChartMorphologyTelemetry telemetry;
  Vec3 matter_p{},local_p{},spline_p{};
  double local_defect=INFINITY;
  double spline_defect=INFINITY;
  double common_residual=0.0;
  double energy_residual=0.0;
  double energy_drift=0.0;
  double speed_excess=0.0;
  double sigma_min=INFINITY;
  double condition=0.0;
  double inverse_residual=0.0;
  bool inverse_valid=true;
  TransportedChartMorphologyComparison comparison;
  double longitudinal_near_moment=INFINITY;
  double longitudinal_combined_moment=INFINITY;
};

struct MorphologyArm {
  std::string name;
  bool initialized=false;
  bool executed=false;
  bool valid=false;
  Vec3 initial_matter_p{},initial_local_p{},initial_spline_p{};
  Vec3 initial_local_total{},initial_spline_total{};
  double initial_energy=INFINITY;
  std::vector<MorphologyCheckpoint> checkpoints;
};

TransportedChartMorphologyOptions morphology_options(
    const std::string& slug,const ConnectedMooreBlockOptions& action) {
  TransportedChartMorphologyOptions options;
  options.support_half_width=4;
  options.near_radius=8;
  options.outer_radius=48;
  options.wave_speed=action.wave_speed;
  options.dt=action.dt;
  options.modes=make_transport_modes(morphology_basis(slug));
  return options;
}

MorphologyCheckpoint observe_morphology_checkpoint(
    int tick,const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& action,
    const TransportedChartMorphologyOptions& morphology,
    const Vec3& direction) {
  MorphologyCheckpoint result;
  result.tick=tick;
  result.field=observe_fractional_checkpoint(state,action);
  result.morphology=observe_transported_chart_morphology_cuda(
      state,action,morphology,&result.telemetry);
  result.matter_p=matter_momentum(state);
  result.longitudinal_near_moment=
      result.morphology.near_residual_first_moment.dot(direction);
  const double near=result.morphology.near_residual_energy;
  const double outer=result.morphology.outer_residual_energy;
  if(near+outer>0.0)
    result.longitudinal_combined_moment=(
        result.morphology.near_residual_first_moment*near
        +result.morphology.outer_residual_first_moment*outer).dot(direction)
        /(near+outer);
  result.valid=result.field.valid&&result.morphology.valid
      &&result.telemetry.valid&&result.telemetry.complete_field_downloads==0;
  return result;
}

MorphologyArm run_morphology_arm(
    const ConnectedMooreBlockState& parent,const std::string& slug,
    const ForensicDirection& direction,int sign,
    const ConnectedMooreBlockOptions& options,double beta) {
  MorphologyArm arm;
  arm.name=sign==0?"rest":"plus";
  auto initial=parent;
  if(sign!=0) for(auto& point:initial.constituents)
    point.momentum+=direction_unit(direction)*(sign*kBoost);
  const auto core=observe_support_invariant_matter(initial,options);
  arm.initialized=core.valid&&core.member
      &&core.graph_margin>=1e-6&&core.energy_margin>=1e-6;
  if(!arm.initialized) return arm;
  const auto observer_options=morphology_options(slug,options);
  const Vec3 unit=direction_unit(direction);
  arm.checkpoints.push_back(observe_morphology_checkpoint(
      kFormationTick,initial,options,observer_options,unit));
  MorphologyCudaStepper stepper(std::move(initial),options,beta);
  if(!stepper.valid()) return arm;
  bool execution=true;
  for(int offset=1;offset<=kFtd0764Ticks;++offset) {
    const int tick=kFormationTick+offset;
    const bool checkpoint=morphology_checkpoint(tick);
    const auto step=stepper.advance(checkpoint);
    execution=execution&&step.valid&&step.common;
    if(offset==1) {
      arm.initial_matter_p=arm.checkpoints.front().matter_p;
      arm.initial_local_p=step.local_before;
      arm.initial_spline_p=step.spline_before;
      arm.initial_local_total=arm.initial_matter_p+arm.initial_local_p;
      arm.initial_spline_total=arm.initial_matter_p+arm.initial_spline_p;
      arm.initial_energy=step.energy_before;
      arm.checkpoints.front().local_p=step.local_before;
      arm.checkpoints.front().spline_p=step.spline_before;
      arm.checkpoints.front().local_defect=0.0;
      arm.checkpoints.front().spline_defect=0.0;
    }
    if(!step.valid) break;
    if(checkpoint) {
      auto record=observe_morphology_checkpoint(
          tick,stepper.state(),options,observer_options,unit);
      record.local_p=step.local_after;
      record.spline_p=step.spline_after;
      record.local_defect=(record.matter_p+record.local_p
          -arm.initial_local_total).mag();
      record.spline_defect=(record.matter_p+record.spline_p
          -arm.initial_spline_total).mag();
      record.common_residual=step.common_residual;
      record.energy_residual=step.energy_residual;
      record.energy_drift=std::abs(step.energy_after-arm.initial_energy);
      record.speed_excess=step.speed_excess;
      record.sigma_min=step.sigma_min;
      record.condition=step.condition;
      record.inverse_valid=step.inverse_valid;
      record.inverse_residual=step.inverse_residual;
      record.valid=record.valid&&step.regularity_measured
          &&step.sigma_min>=1e-3&&step.condition<=1e4
          &&step.inverse_valid&&step.inverse_residual<=1e-10
          &&step.common_residual<=1e-10&&step.energy_residual<=1e-8
          &&step.speed_excess<=1e-12;
      arm.checkpoints.push_back(std::move(record));
    }
  }
  arm.executed=execution&&arm.checkpoints.size()==kFtd0764Checkpoints.size();
  if(arm.executed) {
    const auto& reference=arm.checkpoints.front().morphology;
    for(std::size_t i=1;i<arm.checkpoints.size();++i)
      arm.checkpoints[i].comparison=compare_transported_chart_morphology(
          reference,arm.checkpoints[i].morphology);
  }
  arm.valid=arm.initialized&&arm.executed
      &&std::all_of(arm.checkpoints.begin(),arm.checkpoints.end(),
          [](const auto& value){return value.valid;});
  return arm;
}

struct MorphologyReplay {
  std::string slug;
  std::string direction;
  bool parent_valid=false;
  MorphologyArm rest;
  MorphologyArm plus;
  bool execution_valid=false;
  bool bound_control=false;
  bool near_coherent=false;
  bool detached_outgoing=false;
  bool trailing_wake=false;
  bool local_momentum_closes=false;
  bool spline_momentum_closes=false;
  std::string morphology_verdict="MORPHOLOGY_EXECUTION_INVALID";
};

MorphologyReplay run_morphology_replay(const std::string& slug,int L,
                                       int ticks) {
  MorphologyReplay result;
  result.slug=slug;
  ForensicDirection direction;
  if(!select_direction(slug,direction)) return result;
  result.direction=direction.label;
  const auto normalization=measure_face_flux_normalization();
  if(!normalization.valid) return result;
  auto options=forensic_options();
  auto parent=build_parent(
      L,direction,options,normalization.mapped_field_work_coefficient);
  result.parent_valid=parent.valid;
  if(!parent.valid) return result;
  result.rest=run_morphology_arm(parent.state,slug,direction,0,options,
      normalization.mapped_field_work_coefficient);
  result.plus=run_morphology_arm(parent.state,slug,direction,+1,options,
      normalization.mapped_field_work_coefficient);
  result.execution_valid=result.rest.valid&&result.plus.valid&&ticks==64;
  if(!result.execution_valid) return result;
  result.bound_control=true;
  result.near_coherent=true;
  result.local_momentum_closes=true;
  result.spline_momentum_closes=true;
  for(std::size_t i=1;i<result.plus.checkpoints.size();++i) {
    const auto& value=result.plus.checkpoints[i];
    result.bound_control=result.bound_control&&value.comparison.valid
        &&value.comparison.bound_distance<=0.02;
    result.near_coherent=result.near_coherent&&value.comparison.valid
        &&value.comparison.near_residual_distance<=0.10
        &&value.comparison.near_residual_energy_ratio>=0.8
        &&value.comparison.near_residual_energy_ratio<=1.2;
    result.local_momentum_closes=result.local_momentum_closes
        &&value.local_defect<=1e-9;
    result.spline_momentum_closes=result.spline_momentum_closes
        &&value.spline_defect<=1e-9;
  }
  result.near_coherent=result.near_coherent&&result.bound_control;
  if(result.bound_control&&result.near_coherent)
    result.morphology_verdict="TRANSPORTED_NEAR_FIELD_COHERENT";
  else if(result.bound_control)
    result.morphology_verdict="BOUND_CONTROL_ONLY";
  else result.morphology_verdict="NO_TRANSPORTED_FIELD_COHERENCE";

  const auto& c=result.plus.checkpoints;
  const auto shell48=[](const MorphologyCheckpoint& value) {
    const auto it=std::find_if(value.field.shells.begin(),
        value.field.shells.end(),[](const auto& shell){return shell.radius==48;});
    return it==value.field.shells.end()?nullptr:&*it;
  };
  result.detached_outgoing=c.size()==5
      &&c[1].morphology.outer_residual_energy
          <c[2].morphology.outer_residual_energy
      &&c[2].morphology.outer_residual_energy
          <c[3].morphology.outer_residual_energy
      &&c[3].morphology.outer_residual_energy
          <c[4].morphology.outer_residual_energy;
  for(const std::size_t i:{2u,3u,4u}) {
    const auto* shell= shell48(c[i]);
    result.detached_outgoing=result.detached_outgoing&&shell!=nullptr
        &&shell->signed_radial_poynting>0.0;
  }
  result.trailing_wake=true;
  int magnitude_increases=0;
  for(std::size_t i=1;i<c.size();++i) {
    result.trailing_wake=result.trailing_wake
        &&c[i].longitudinal_combined_moment<0.0;
    if(i>1&&std::abs(c[i].longitudinal_combined_moment)
        >std::abs(c[i-1].longitudinal_combined_moment))
      ++magnitude_increases;
  }
  result.trailing_wake=result.trailing_wake&&magnitude_increases>=3;
  return result;
}

std::filesystem::path ftd0764_results_directory() {
  return std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results"/"ftd_0764";
}

void write_complex(std::ostream& out,const std::complex<double>& value) {
  out<<'['<<json_number(value.real())<<", "<<json_number(value.imag())<<']';
}

void write_morphology_observation(
    std::ostream& out,const TransportedChartMorphologyObservation& value) {
  out<<"{\"valid\": "<<value.valid<<", \"center\": ";
  write_vec(out,value.center);
  out<<", \"support_center\": "; write_vec(out,value.support_center);
  out<<", \"actual_energy\": "<<json_number(value.actual_energy)
     <<", \"bound_energy\": "<<json_number(value.bound_energy)
     <<", \"residual_energy\": "<<json_number(value.residual_energy)
     <<", \"interference_energy\": "<<json_number(value.interference_energy)
     <<", \"near_residual_energy\": "
     <<json_number(value.near_residual_energy)
     <<", \"outer_residual_energy\": "
     <<json_number(value.outer_residual_energy)
     <<", \"near_first_moment\": "; write_vec(out,value.near_residual_first_moment);
  out<<", \"outer_first_moment\": "; write_vec(out,value.outer_residual_first_moment);
  out<<", \"near_rms_radius\": "<<json_number(value.near_residual_rms_radius)
     <<", \"outer_rms_radius\": "<<json_number(value.outer_residual_rms_radius)
     <<", \"energy_reconstruction_residual\": "
     <<json_number(value.energy_reconstruction_residual)
     <<", \"maximum_mode_reconstruction_residual\": "
     <<json_number(value.maximum_mode_reconstruction_residual)
     <<", \"modes\": [";
  for(std::size_t i=0;i<value.coefficients.size();++i) {
    if(i) out<<',';
    const auto& mode=value.coefficients[i];
    out<<"{\"n\": ["<<mode.mode.nx<<','<<mode.mode.ny<<','<<mode.mode.nz
       <<"], \"actual\": "; write_complex(out,mode.actual);
    out<<", \"bound\": "; write_complex(out,mode.bound);
    out<<", \"residual\": "; write_complex(out,mode.residual);
    out<<", \"interference\": "; write_complex(out,mode.interference);
    out<<", \"near_residual\": "; write_complex(out,mode.near_residual);
    out<<'}';
  }
  out<<"]}";
}

void write_fractional_field_evidence(
    std::ostream& out,const FractionalCheckpoint& value) {
  out<<"{\"valid\": "<<value.valid
     <<", \"observer_valid\": "<<value.observer_valid
     <<", \"boundary_ledger_valid\": "<<value.boundary_ledger_valid
     <<", \"ladder_valid\": "<<value.ladder_valid
     <<", \"cuda_scalar_only\": "<<value.cuda_scalar_only
     <<", \"actual_gauss_residual\": "
     <<json_number(value.actual_gauss_residual)
     <<", \"energy_partition_residual\": "
     <<json_number(value.energy_partition_residual)
     <<", \"boundary_identity_residual\": "
     <<json_number(value.boundary_identity_residual)
     <<", \"readout_reconstruction_residual\": "
     <<json_number(value.readout_reconstruction_residual)
     <<", \"characteristic_flux_residual\": "
     <<json_number(value.characteristic_flux_residual)
     <<", \"ladder_energy_residual\": "
     <<json_number(value.ladder_energy_residual)
     <<", \"ladder_projection_residual\": "
     <<json_number(value.ladder_projection_residual)
     <<", \"host_to_device_bytes\": "<<value.host_to_device_bytes
     <<", \"device_to_host_bytes\": "<<value.device_to_host_bytes
     <<", \"kernel_ms\": "<<json_number(value.kernel_ms)
     <<", \"shells\": [";
  for(std::size_t i=0;i<value.shells.size();++i) {
    if(i) out<<',';
    const auto& shell=value.shells[i];
    out<<"{\"radius\": "<<shell.radius
       <<", \"samples\": "<<shell.samples
       <<", \"residual_energy\": "<<json_number(shell.residual_energy)
       <<", \"outgoing_energy\": "<<json_number(shell.outgoing_energy)
       <<", \"incoming_energy\": "<<json_number(shell.incoming_energy)
       <<", \"radial_energy\": "<<json_number(shell.radial_energy)
       <<", \"background_energy\": "<<json_number(shell.background_energy)
       <<", \"signed_radial_poynting\": "
       <<json_number(shell.signed_radial_poynting)<<'}';
  }
  out<<"]}";
}

void write_arm(std::ostream& out,const MorphologyArm& arm) {
  out<<"{\"name\": \""<<arm.name<<"\", \"initialized\": "
     <<arm.initialized<<", \"executed\": "<<arm.executed
     <<", \"valid\": "<<arm.valid<<", \"checkpoints\": [";
  for(std::size_t i=0;i<arm.checkpoints.size();++i) {
    if(i) out<<',';
    const auto& c=arm.checkpoints[i];
    out<<"{\"tick\": "<<c.tick<<", \"valid\": "<<c.valid
       <<", \"matter_momentum\": "; write_vec(out,c.matter_p);
    out<<", \"local_field_momentum\": "; write_vec(out,c.local_p);
    out<<", \"spline_field_momentum\": "; write_vec(out,c.spline_p);
    out<<", \"local_defect\": "<<json_number(c.local_defect)
       <<", \"spline_defect\": "<<json_number(c.spline_defect)
       <<", \"common_residual\": "<<json_number(c.common_residual)
       <<", \"energy_residual\": "<<json_number(c.energy_residual)
       <<", \"energy_drift\": "<<json_number(c.energy_drift)
       <<", \"speed_excess\": "<<json_number(c.speed_excess)
       <<", \"sigma_min\": "<<json_number(c.sigma_min)
       <<", \"condition\": "<<json_number(c.condition)
       <<", \"inverse_valid\": "<<c.inverse_valid
       <<", \"inverse_residual\": "<<json_number(c.inverse_residual)
       <<", \"longitudinal_near_moment\": "
       <<json_number(c.longitudinal_near_moment)
       <<", \"longitudinal_combined_moment\": "
       <<json_number(c.longitudinal_combined_moment)
       <<", \"comparison\": {\"valid\": "<<c.comparison.valid
       <<", \"actual_distance\": "<<json_number(c.comparison.actual_distance)
       <<", \"bound_distance\": "<<json_number(c.comparison.bound_distance)
       <<", \"residual_distance\": "
       <<json_number(c.comparison.residual_distance)
       <<", \"near_residual_distance\": "
       <<json_number(c.comparison.near_residual_distance)
       <<", \"near_residual_energy_ratio\": "
       <<json_number(c.comparison.near_residual_energy_ratio)<<"}, "
       <<"\"fractional_observer_valid\": "<<c.field.valid
       <<", \"boundary_ledger_valid\": "
       <<c.field.boundary_ledger_valid
       <<", \"ladder_valid\": "<<c.field.ladder_valid
       <<", \"field_evidence\": ";
    write_fractional_field_evidence(out,c.field);
    out<<", \"morphology\": ";
    write_morphology_observation(out,c.morphology);
    out<<'}';
  }
  out<<"]}";
}

void write_morphology_result(const MorphologyReplay& value) {
  const auto directory=ftd0764_results_directory();
  std::filesystem::create_directories(directory);
  std::ofstream out(directory/
      ("ftd_0764_transported_chart_morphology_v1_"+value.slug+".json"));
  out<<std::boolalpha<<std::setprecision(17)
     <<"{\n  \"ftd_id\": \"FTD-0764\",\n"
     <<"  \"protocol_sha256\": \""<<kFtd0764ProtocolSha256<<"\",\n"
     <<"  \"slug\": \""<<value.slug<<"\",\n"
     <<"  \"direction\": \""<<value.direction<<"\",\n"
     <<"  \"volume\": "<<kFtd0764Volume<<",\n"
     <<"  \"parent_valid\": "<<value.parent_valid<<",\n"
     <<"  \"execution_valid\": "<<value.execution_valid<<",\n"
     <<"  \"morphology_verdict\": \""<<value.morphology_verdict<<"\",\n"
     <<"  \"bound_control\": "<<value.bound_control<<",\n"
     <<"  \"near_coherent\": "<<value.near_coherent<<",\n"
     <<"  \"detached_outgoing\": "<<value.detached_outgoing<<",\n"
     <<"  \"trailing_wake\": "<<value.trailing_wake<<",\n"
     <<"  \"local_momentum_closes\": "<<value.local_momentum_closes<<",\n"
     <<"  \"spline_momentum_closes\": "<<value.spline_momentum_closes<<",\n"
     <<"  \"rest\": "; write_arm(out,value.rest);
  out<<",\n  \"plus\": "; write_arm(out,value.plus);
  out<<",\n  \"production_changed\": false,\n"
     <<"  \"dynamics_changed\": false,\n"
     <<"  \"substrate_momentum_invented\": false\n}\n";
}

std::string read_string(const std::filesystem::path& path,
                        const std::string& key) {
  std::ifstream input(path);
  const std::string bytes((std::istreambuf_iterator<char>(input)),{});
  const std::string prefix="\""+key+"\": \"";
  const auto begin=bytes.find(prefix);
  if(begin==std::string::npos) return {};
  const auto first=begin+prefix.size();
  const auto end=bytes.find('"',first);
  return end==std::string::npos?std::string{}:bytes.substr(first,end-first);
}

void write_morphology_aggregate() {
  const auto directory=ftd0764_results_directory();
  const std::array<std::string,3> slugs{{"face","edge","body"}};
  bool complete=true,execution=true,bound=true,near=true,detached=true;
  bool wake=true,local=true,spline=true;
  std::vector<std::string> verdicts;
  for(const auto& slug:slugs) {
    const auto path=directory/
        ("ftd_0764_transported_chart_morphology_v1_"+slug+".json");
    complete=complete&&std::filesystem::is_regular_file(path);
    execution=execution&&read_bit(path,"execution_valid");
    bound=bound&&read_bit(path,"bound_control");
    near=near&&read_bit(path,"near_coherent");
    detached=detached&&read_bit(path,"detached_outgoing");
    wake=wake&&read_bit(path,"trailing_wake");
    local=local&&read_bit(path,"local_momentum_closes");
    spline=spline&&read_bit(path,"spline_momentum_closes");
    verdicts.push_back(read_string(path,"morphology_verdict"));
  }
  std::ofstream out(directory/"ftd_0764_transported_chart_morphology_v1.json");
  out<<std::boolalpha
     <<"{\n  \"ftd_id\": \"FTD-0764\",\n"
     <<"  \"protocol_sha256\": \""<<kFtd0764ProtocolSha256<<"\",\n"
     <<"  \"all_artifacts_present\": "<<complete<<",\n"
     <<"  \"all_execution_valid\": "<<execution<<",\n"
     <<"  \"all_bound_controls\": "<<bound<<",\n"
     <<"  \"all_near_coherent\": "<<near<<",\n"
     <<"  \"all_detached_outgoing\": "<<detached<<",\n"
     <<"  \"all_trailing_wake\": "<<wake<<",\n"
     <<"  \"all_local_momentum_close\": "<<local<<",\n"
     <<"  \"all_spline_momentum_close\": "<<spline<<",\n"
     <<"  \"ray_verdicts\": [";
  for(std::size_t i=0;i<verdicts.size();++i) {
    if(i) out<<", "; out<<'"'<<verdicts[i]<<'"';
  }
  out<<"],\n  \"production_changed\": false,\n"
     <<"  \"dynamics_changed\": false\n}\n";
}

int run_registered_morphology(const std::string& slug) {
  if(std::string(kFtd0764ProtocolSha256)=="UNLOCKED") return 3;
  if(slug=="body") for(const auto& prior:{"face","edge"}) {
    const auto path=ftd0764_results_directory()/
        (std::string("ftd_0764_transported_chart_morphology_v1_")
         +prior+".json");
    if(!std::filesystem::is_regular_file(path)) return 4;
  }
  const auto result=run_morphology_replay(slug,kFtd0764Volume,kFtd0764Ticks);
  write_morphology_result(result);
  if(slug=="body") write_morphology_aggregate();
  const auto& final=result.plus.checkpoints.empty()
      ?MorphologyCheckpoint{}:result.plus.checkpoints.back();
  std::cout<<std::boolalpha<<std::setprecision(17)
           <<"FTD-0764 direction="<<slug
           <<" execution="<<result.execution_valid
           <<" verdict="<<result.morphology_verdict
           <<" near_D="<<final.comparison.near_residual_distance
           <<" near_ratio="<<final.comparison.near_residual_energy_ratio
           <<" outer="<<final.morphology.outer_residual_energy
           <<" moment="<<final.longitudinal_combined_moment
           <<" local_defect="<<final.local_defect
           <<" spline_defect="<<final.spline_defect
           <<" detached="<<result.detached_outgoing
           <<" wake="<<result.trailing_wake<<'\n';
  return result.execution_valid?0:1;
}

}  // namespace

int main(int argc,char** argv) {
  if(argc==3&&std::string(argv[1])=="--run")
    return run_registered_morphology(argv[2]);
  std::cout<<"FTD-0764 runner: --run face|edge|body\n";
  return argc==1?0:2;
}
