/** FTD-0757: fixed integer-chart qualification of the M3 parent replay. */

#define FTD_0754_MAIN_NAME ftd_0757_embedded_observer_main
#include "campaign_state_only_observer_discovery_cuda.cpp"
#undef FTD_0754_MAIN_NAME

#include "ftd/eft/support_invariant_matter_predicate.h"

namespace {

constexpr char kFixedChartProtocolSha256[]=
    "E867A86868E00673EDAA716F1D7CB021A2E9BFB6F798BDC8C552385C4EE6DB50";
constexpr double kFixedChartGate=1e-10;
constexpr int kFixedChartTickLimit=160;
constexpr std::array<int,6> kFixedChartRadii{{8,12,16,24,32,48}};

struct FixedChartStepDiagnostics {
  bool valid=false;
  bool common=false;
  bool observer_valid=false;
  int failure_stage=0;
  double maximum_residual=INFINITY;
  double energy_residual=INFINITY;
  double recoil_defect=INFINITY;
  double speed_excess=INFINITY;
};

Vec3 pair_midpoint(const ConnectedMooreBlockState& state) {
  if(state.constituents.size()!=2) return {};
  return (effective_position(state.constituents[0])
      +effective_position(state.constituents[1]))*0.5;
}

double integer_chart_residual(const Vec3& value) {
  return std::max({std::abs(value.x-std::round(value.x)),
      std::abs(value.y-std::round(value.y)),
      std::abs(value.z-std::round(value.z))});
}

class FixedChartCudaStepper {
 public:
  FixedChartCudaStepper(ConnectedMooreBlockState initial,
                        ConnectedMooreBlockOptions input_options,
                        double interaction_scale,Vec3 fixed_center)
      : state_(std::move(initial)),options_(std::move(input_options)),
        interaction_scale_(interaction_scale),fixed_center_(fixed_center),
        pipeline_(state_.electric.L),prepared_magnetic_(state_.electric.L),
        prepared_electric_(state_.electric.L) {
    options_.defer_volume_diagnostics=true;
    options_.measure_final_root_regularity=false;
    valid_=pipeline_.valid()
        &&pipeline_.upload(state_.electric,state_.magnetic_half);
  }

  bool valid() const { return valid_; }
  const ConnectedMooreBlockState& state() const { return state_; }

  FixedChartStepDiagnostics advance() {
    FixedChartStepDiagnostics record;
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
    const std::vector<int> radii(
        kFixedChartRadii.begin(),kFixedChartRadii.end());
    const auto profile=pipeline_.observe_deterministic(
        lambda,fixed_center_,radii,kFixedChartGate);
    record.observer_valid=profile.valid;
    if(!profile.valid) {
      record.failure_stage=4;
      valid_=false;
      return record;
    }
    const auto diagnostics=pipeline_.diagnose_common_action(
        step.segments,options_.polarity_scale,interaction_scale_,
        options_.wave_speed,options_.dt,kFixedChartGate);
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
      record.failure_stage=5;
      valid_=false;
    }
    valid_=valid_&&record.valid&&record.common;
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

struct FixedChartRow {
  int tick=0;
  bool has_step=false;
  Vec3 midpoint_before{};
  double midpoint_integer_residual=0.0;
  bool moving_center_api_admissible=true;
  FixedChartStepDiagnostics step{};
  ftd::eft::SupportInvariantMatterPredicate core{};
};

struct FixedChartResult {
  int volume=0;
  std::string arm;
  std::string direction;
  Vec3 fixed_center{};
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
  int first_fractional_midpoint_tick=-1;
  std::vector<FixedChartRow> rows;
};

std::string fixed_chart_arm(const std::string& direction) {
  if(direction=="0_0_1") return "face";
  if(direction=="0_1_-1") return "edge";
  return "body";
}

int expected_fractional_tick(const std::string& arm) {
  if(arm=="face") return 57;
  if(arm=="edge") return 30;
  return 122;
}

FixedChartResult run_fixed_chart(int L,const Direction& direction,
    const ConnectedMooreBlockOptions& options,double interaction_scale,
    int tick_limit) {
  FixedChartResult result;
  result.volume=L;
  result.direction=direction.label;
  result.arm=fixed_chart_arm(result.direction);
  const double c=static_cast<double>(L/2);
  result.fixed_center={c,c,c};
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
  FixedChartRow initial;
  initial.tick=0;
  initial.midpoint_before=pair_midpoint(preparation.state);
  initial.midpoint_integer_residual=
      integer_chart_residual(initial.midpoint_before);
  initial.moving_center_api_admissible=
      initial.midpoint_integer_residual==0.0;
  initial.core=ftd::eft::observe_support_invariant_matter(
      preparation.state,options);
  result.rows.push_back(initial);
  if(!preparation.valid||!preparation.density_contained
      ||!preparation.compact_support||!preparation.zero_boundary_crossing)
    return result;

  FixedChartCudaStepper stepper(std::move(preparation.state),options,
      interaction_scale,result.fixed_center);
  result.replay_started=stepper.valid();
  if(!result.replay_started) {
    result.first_failure_tick=0;
    result.first_failure_stage=-1;
    return result;
  }
  for(int tick=1;tick<=tick_limit;++tick) {
    FixedChartRow row;
    row.tick=tick;
    row.has_step=true;
    row.midpoint_before=pair_midpoint(stepper.state());
    row.midpoint_integer_residual=integer_chart_residual(row.midpoint_before);
    row.moving_center_api_admissible=row.midpoint_integer_residual==0.0;
    if(!row.moving_center_api_admissible
        &&result.first_fractional_midpoint_tick<0)
      result.first_fractional_midpoint_tick=tick;
    row.step=stepper.advance();
    row.core=ftd::eft::observe_support_invariant_matter(
        stepper.state(),options);
    result.rows.push_back(row);
    if(!row.step.valid||!row.step.common||!stepper.valid()) {
      result.first_failure_tick=tick;
      result.first_failure_stage=row.step.failure_stage;
      break;
    }
  }
  const auto& final=result.rows.back();
  result.reached_tick_160=final.tick==kFixedChartTickLimit
      &&final.step.valid&&final.step.common&&final.step.observer_valid
      &&final.core.sector_valid&&final.core.member
      &&final.core.graph_margin>=1e-6&&final.core.energy_margin>=1e-6;
  return result;
}

void write_fixed_chart(const FixedChartResult& result) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0757";
  std::filesystem::create_directories(directory);
  const auto stem="ftd_0757_m3_fixed_chart_parent_v1_"+result.arm+"_L"
      +std::to_string(result.volume);
  std::ofstream csv(directory/(stem+".csv"));
  csv<<"volume,arm,direction,tick,has_step,fixed_center_x,fixed_center_y,"
      "fixed_center_z,midpoint_x,midpoint_y,midpoint_z,"
      "midpoint_integer_residual,moving_center_api_admissible,step_valid,"
      "common,observer_valid,failure_stage,max_residual,energy_residual,"
      "recoil_defect,speed_excess,sector_valid,member,separation_squared,"
      "separation,graph_margin,energy_margin,pair_energy\n"
      <<std::setprecision(17);
  for(const auto& row:result.rows) {
    csv<<result.volume<<','<<result.arm<<','<<result.direction<<','<<row.tick
       <<','<<row.has_step<<','<<result.fixed_center.x<<','
       <<result.fixed_center.y<<','<<result.fixed_center.z<<','
       <<row.midpoint_before.x<<','<<row.midpoint_before.y<<','
       <<row.midpoint_before.z<<','<<row.midpoint_integer_residual<<','
       <<row.moving_center_api_admissible<<','
       <<(row.has_step?row.step.valid:false)<<','
       <<(row.has_step?row.step.common:false)<<','
       <<(row.has_step?row.step.observer_valid:false)<<','
       <<(row.has_step?row.step.failure_stage:0)<<','
       <<(row.has_step?row.step.maximum_residual:0.0)<<','
       <<(row.has_step?row.step.energy_residual:0.0)<<','
       <<(row.has_step?row.step.recoil_defect:0.0)<<','
       <<(row.has_step?row.step.speed_excess:0.0)<<','
       <<row.core.sector_valid<<','<<row.core.member<<','
       <<row.core.separation_squared<<','
       <<std::sqrt(row.core.separation_squared)<<','<<row.core.graph_margin
       <<','<<row.core.energy_margin<<','<<row.core.pair_energy<<'\n';
  }
  const auto& final=result.rows.back().core;
  std::ofstream json(directory/(stem+".json"));
  json<<std::setprecision(17)
      <<"{\n  \"ftd_id\": \"FTD-0757\",\n"
      <<"  \"protocol_sha256\": \""<<kFixedChartProtocolSha256<<"\",\n"
      <<"  \"volume\": "<<result.volume<<",\n"
      <<"  \"arm\": \""<<result.arm<<"\",\n"
      <<"  \"direction\": \""<<result.direction<<"\",\n"
      <<"  \"fixed_center\": ["<<result.fixed_center.x<<','
      <<result.fixed_center.y<<','<<result.fixed_center.z<<"],\n"
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
      <<"  \"first_fractional_midpoint_tick\": "
      <<result.first_fractional_midpoint_tick<<",\n"
      <<"  \"expected_fractional_midpoint_tick\": "
      <<expected_fractional_tick(result.arm)<<",\n"
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
    std::cout<<"FTD-0757: --qualify face|edge|body L 1; "
        "--run face|edge|body 321|385\n";
    return 2;
  }
  Direction direction;
  const std::string arm=argv[2];
  if(!select_horizon_direction(arm,direction)) return 2;
  const int L=std::stoi(argv[3]);
  const int ticks=qualification?std::stoi(argv[4]):kFixedChartTickLimit;
  if(qualification&&(L!=321||arm!="face"||ticks!=1)) return 2;
  if(registered&&(L!=321&&L!=385)) return 2;
  if(registered&&std::string(kFixedChartProtocolSha256)=="UNLOCKED") {
    std::cerr<<"FTD-0757 registered replay refused before protocol lock\n";
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
  if(!normalization.valid) return 1;
  const auto start=std::chrono::steady_clock::now();
  const auto result=run_fixed_chart(L,direction,options,
      normalization.mapped_field_work_coefficient,ticks);
  const double seconds=std::chrono::duration<double>(
      std::chrono::steady_clock::now()-start).count();
  if(registered) write_fixed_chart(result);
  const bool midpoint_match=qualification
      ||result.first_fractional_midpoint_tick==expected_fractional_tick(arm);
  const bool pass=result.preparation_valid&&result.replay_started
      &&(qualification
          ?result.rows.size()==2&&result.rows.back().step.valid
              &&result.rows.back().step.common
              &&result.rows.back().step.observer_valid
          :result.reached_tick_160&&midpoint_match);
  std::cout<<std::setprecision(17)<<"FTD-0757 "<<arm<<" L="<<L
      <<" pass="<<pass<<" rows="<<result.rows.size()
      <<" first_fractional="<<result.first_fractional_midpoint_tick
      <<" first_failure="<<result.first_failure_tick
      <<" stage="<<result.first_failure_stage<<" seconds="<<seconds
      <<" protocol="<<kFixedChartProtocolSha256<<'\n';
  return pass?0:1;
}
