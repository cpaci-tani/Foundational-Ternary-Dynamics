/** FTD-0756: read-only forensics for the FTD-0755 parent replay failure. */

#define FTD_0754_MAIN_NAME ftd_0756_embedded_observer_main
#include "campaign_state_only_observer_discovery_cuda.cpp"
#undef FTD_0754_MAIN_NAME

#include "ftd/eft/support_invariant_matter_predicate.h"

namespace {

constexpr char kM3ParentForensicsProtocolSha256[]=
    "773BDB791B06A0250C980945A1B52EF9F2A6F119EF8905E9AC57DC83A6FB5CFC";
constexpr double kForensicsCommonGate=1e-10;

struct ForensicsStepDiagnostics {
  bool valid=false;
  bool common=false;
  int failure_stage=0;
  bool solve_attempted=false;
  bool solve_converged=false;
  int solve_iterations=0;
  double solve_residual=INFINITY;
  double maximum_residual=INFINITY;
  double energy_residual=INFINITY;
  double recoil_defect=INFINITY;
  double speed_excess=INFINITY;
};

class ForensicsCudaStepper {
 public:
  ForensicsCudaStepper(ConnectedMooreBlockState initial,
                       ConnectedMooreBlockOptions input_options,
                       double interaction_scale)
      : state_(std::move(initial)),options_(std::move(input_options)),
        interaction_scale_(interaction_scale),pipeline_(state_.electric.L),
        prepared_magnetic_(state_.electric.L),
        prepared_electric_(state_.electric.L) {
    options_.defer_volume_diagnostics=true;
    options_.measure_final_root_regularity=false;
    valid_=pipeline_.valid()
        &&pipeline_.upload(state_.electric,state_.magnetic_half);
  }

  bool valid() const { return valid_; }
  const ConnectedMooreBlockState& state() const { return state_; }

  ForensicsStepDiagnostics advance() {
    ForensicsStepDiagnostics record;
    if(!valid_) return record;
    const double lambda=options_.wave_speed*options_.dt;
    if(!pipeline_.prepare_forward(lambda)
        ||!pipeline_.download_prepared(
            prepared_magnetic_,prepared_electric_)) {
      record.failure_stage=1; valid_=false; return record;
    }
    auto step=ftd::eft::solve_connected_moore_block_forward_prepared(
        state_,std::move(prepared_magnetic_),std::move(prepared_electric_),
        options_,&cache_);
    record.solve_attempted=step.solve.attempted;
    record.solve_converged=step.solve.converged;
    record.solve_iterations=step.solve.iterations;
    record.solve_residual=step.solve.residual;
    if(!step.volume_diagnostics_pending) {
      record.failure_stage=2; valid_=false; return record;
    }
    if(!pipeline_.apply_ordered_sparse_current(
            step.segments,options_.polarity_scale)) {
      record.failure_stage=3; valid_=false; return record;
    }
    const Vec3 observer_center=state_.constituents.size()==2
        ?(effective_position(state_.constituents[0])
          +effective_position(state_.constituents[1]))*0.5
        :Vec3{};
    const auto profile=pipeline_.observe_deterministic(
        lambda,observer_center,{8},kForensicsCommonGate);
    if(!profile.valid) {
      record.failure_stage=4; valid_=false; return record;
    }
    const auto diagnostics=pipeline_.diagnose_common_action(
        step.segments,options_.polarity_scale,interaction_scale_,
        options_.wave_speed,options_.dt,kForensicsCommonGate);
    step=ftd::eft::complete_connected_moore_block_volume_diagnostics(
        std::move(step),diagnostics,options_);
    record.valid=step.valid;
    record.common=step.common_action_gates_pass;
    record.maximum_residual=maximum_step_residual(step);
    record.energy_residual=std::abs(step.total_energy_residual);
    record.recoil_defect=std::max({step.matter_momentum_before.mag(),
        step.matter_momentum_after.mag(),step.spline_defect_norm});
    record.speed_excess=step.causal_speed_excess;
    std::swap(state_.electric,prepared_electric_);
    std::swap(state_.magnetic_half,prepared_magnetic_);
    state_=std::move(step.later);
    if(!pipeline_.advance()) {
      valid_=false; record.failure_stage=5;
    }
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

ConnectedMooreBlockOptions forensics_options() {
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
  return options;
}

struct ParentForensicsRow {
  int tick=0;
  bool has_step=false;
  bool pipeline_alive=true;
  ForensicsStepDiagnostics step{};
  ftd::eft::SupportInvariantMatterPredicate core{};
};

struct ParentForensicsResult {
  int volume=0;
  std::string arm;
  std::string direction;
  bool preparation_valid=false;
  bool density_contained=false;
  bool compact_support=false;
  bool zero_boundary_crossing=false;
  double poisson_residual=INFINITY;
  double gauss_residual=INFINITY;
  double outside_maximum=INFINITY;
  double boundary_crossing_maximum=INFINITY;
  bool replay_started=false;
  bool reached_tick_160=false;
  int first_failure_tick=-1;
  int first_failure_stage=0;
  std::vector<ParentForensicsRow> rows;
};

std::string forensic_arm(const std::string& direction) {
  if(direction=="0_0_1") return "face";
  if(direction=="0_1_-1") return "edge";
  return "body";
}

ParentForensicsResult run_parent_forensics(
    int L,const Direction& direction,
    const ConnectedMooreBlockOptions& options,double interaction_scale,
    int tick_limit) {
  ParentForensicsResult result;
  result.volume=L;
  result.direction=direction.label;
  result.arm=forensic_arm(result.direction);
  auto preparation=ftd::eft::prepare_finite_support_derived_compact_pair(
      make_geometry(L,direction,false,1.30,0.0120),options,4,1e-13,4096);
  result.preparation_valid=preparation.valid;
  result.density_contained=preparation.density_contained;
  result.compact_support=preparation.compact_support;
  result.zero_boundary_crossing=preparation.zero_boundary_crossing;
  result.poisson_residual=preparation.poisson_residual;
  result.gauss_residual=preparation.gauss_residual;
  result.outside_maximum=preparation.outside_maximum;
  result.boundary_crossing_maximum=preparation.boundary_crossing_maximum;
  ParentForensicsRow initial;
  initial.tick=0;
  initial.core=ftd::eft::observe_support_invariant_matter(
      preparation.state,options);
  result.rows.push_back(initial);
  if(!preparation.valid||!preparation.density_contained
      ||!preparation.compact_support||!preparation.zero_boundary_crossing)
    return result;

  ForensicsCudaStepper stepper(
      std::move(preparation.state),options,interaction_scale);
  result.replay_started=stepper.valid();
  if(!result.replay_started) {
    result.first_failure_tick=0;
    result.first_failure_stage=-1;
    return result;
  }
  for(int tick=1;tick<=tick_limit;++tick) {
    ParentForensicsRow row;
    row.tick=tick;
    row.has_step=true;
    row.step=stepper.advance();
    row.pipeline_alive=stepper.valid();
    row.core=ftd::eft::observe_support_invariant_matter(
        stepper.state(),options);
    result.rows.push_back(row);
    if(!row.step.valid||!row.step.common||!row.pipeline_alive) {
      result.first_failure_tick=tick;
      result.first_failure_stage=row.step.failure_stage;
      break;
    }
  }
  result.reached_tick_160=result.rows.back().tick==160
      &&result.rows.back().step.valid&&result.rows.back().step.common
      &&result.rows.back().pipeline_alive;
  return result;
}

void write_parent_forensics(const ParentForensicsResult& result) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0756";
  std::filesystem::create_directories(directory);
  const auto stem="ftd_0756_m3_parent_forensics_v1_"+result.arm+"_L"
      +std::to_string(result.volume);
  std::ofstream csv(directory/(stem+".csv"));
  csv<<"volume,arm,direction,tick,has_step,pipeline_alive,step_valid,common,"
      "failure_stage,solve_attempted,solve_converged,solve_iterations,"
      "solve_residual,max_residual,energy_residual,recoil_defect,speed_excess,"
      "sector_valid,member,separation_squared,separation,graph_margin,"
      "energy_margin,pair_energy\n"<<std::setprecision(17);
  for(const auto& row:result.rows) {
    const double separation=std::sqrt(row.core.separation_squared);
    csv<<result.volume<<','<<result.arm<<','<<result.direction<<','<<row.tick
       <<','<<row.has_step<<','<<row.pipeline_alive<<','
       <<(row.has_step?row.step.valid:false)<<','
       <<(row.has_step?row.step.common:false)<<','
       <<(row.has_step?row.step.failure_stage:0)<<','
       <<(row.has_step?row.step.solve_attempted:false)<<','
       <<(row.has_step?row.step.solve_converged:false)<<','
       <<(row.has_step?row.step.solve_iterations:0)<<','
       <<(row.has_step?row.step.solve_residual:0.0)<<','
       <<(row.has_step?row.step.maximum_residual:0.0)<<','
       <<(row.has_step?row.step.energy_residual:0.0)<<','
       <<(row.has_step?row.step.recoil_defect:0.0)<<','
       <<(row.has_step?row.step.speed_excess:0.0)<<','
       <<row.core.sector_valid<<','<<row.core.member<<','
       <<row.core.separation_squared<<','<<separation<<','
       <<row.core.graph_margin<<','<<row.core.energy_margin<<','
       <<row.core.pair_energy<<'\n';
  }
  const auto& final=result.rows.back().core;
  std::ofstream json(directory/(stem+".json"));
  json<<std::setprecision(17)
      <<"{\n  \"ftd_id\": \"FTD-0756\",\n"
      <<"  \"protocol_sha256\": \""<<kM3ParentForensicsProtocolSha256
      <<"\",\n  \"volume\": "<<result.volume<<",\n"
      <<"  \"arm\": \""<<result.arm<<"\",\n"
      <<"  \"direction\": \""<<result.direction<<"\",\n"
      <<"  \"preparation_valid\": "<<result.preparation_valid<<",\n"
      <<"  \"density_contained\": "<<result.density_contained<<",\n"
      <<"  \"compact_support\": "<<result.compact_support<<",\n"
      <<"  \"zero_boundary_crossing\": "
      <<result.zero_boundary_crossing<<",\n"
      <<"  \"poisson_residual\": "<<result.poisson_residual<<",\n"
      <<"  \"gauss_residual\": "<<result.gauss_residual<<",\n"
      <<"  \"outside_maximum\": "<<result.outside_maximum<<",\n"
      <<"  \"boundary_crossing_maximum\": "
      <<result.boundary_crossing_maximum<<",\n"
      <<"  \"replay_started\": "<<result.replay_started<<",\n"
      <<"  \"reached_tick_160\": "<<result.reached_tick_160<<",\n"
      <<"  \"first_failure_tick\": "<<result.first_failure_tick<<",\n"
      <<"  \"first_failure_stage\": "<<result.first_failure_stage<<",\n"
      <<"  \"row_count\": "<<result.rows.size()<<",\n"
      <<"  \"final_sector_valid\": "<<final.sector_valid<<",\n"
      <<"  \"final_member\": "<<final.member<<",\n"
      <<"  \"final_graph_margin\": "<<final.graph_margin<<",\n"
      <<"  \"final_energy_margin\": "<<final.energy_margin<<",\n"
      <<"  \"dynamics_changed\": false\n}\n";
}

}  // namespace

int main(int argc,char** argv) {
  const bool qualification=argc==5&&std::string(argv[1])=="--qualify";
  const bool registered=argc==4&&std::string(argv[1])=="--run";
  if(!qualification&&!registered) {
    std::cout<<"FTD-0756: --qualify face|edge|body L 1; "
        "--run face|edge|body 321|385\n";
    return argc==1?0:2;
  }
  Direction direction;
  if(!select_horizon_direction(argv[2],direction)) return 2;
  const int L=std::stoi(argv[3]);
  const int tick_limit=qualification?std::stoi(argv[4]):160;
  if(qualification) {
    if(tick_limit!=1||L!=321) return 2;
  } else if(L!=321&&L!=385) return 2;
  if(registered
      &&std::string(kM3ParentForensicsProtocolSha256)=="UNLOCKED") {
    std::cerr<<"FTD-0756 registered execution refused before lock\n";
    return 3;
  }
  const auto normalization=ftd::eft::measure_face_flux_normalization();
  if(!normalization.valid) return 1;
  const auto result=run_parent_forensics(
      L,direction,forensics_options(),
      normalization.mapped_field_work_coefficient,
      tick_limit);
  if(registered) write_parent_forensics(result);
  const bool complete=result.reached_tick_160;
  std::cout<<"FTD-0756 arm="<<result.arm<<" L="<<L
      <<" rows="<<result.rows.size()
      <<" first_failure_tick="<<result.first_failure_tick
      <<" stage="<<result.first_failure_stage
      <<" reached160="<<result.reached_tick_160
      <<" final_member="<<result.rows.back().core.member<<'\n';
  return registered?0:(result.rows.size()==2?0:1);
}
