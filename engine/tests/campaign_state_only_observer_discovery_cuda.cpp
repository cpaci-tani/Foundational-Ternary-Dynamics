/** FTD-0754: discovery-only replay for the state-only field observer. */

#include "ftd/eft/cuda_matched_field_pipeline.h"

#define apply_sparse_current apply_ordered_sparse_current
#define observe observe_deterministic
#define FTD_0748_MAIN_NAME ftd_0754_discovery_parent_main
#include "campaign_canonical_current_horizon_cuda.cpp"
#undef FTD_0748_MAIN_NAME
#undef observe
#undef apply_sparse_current

#include "ftd/eft/state_only_matter_field_observer.h"

#include <set>

namespace {

constexpr char kObserverDiscoveryProtocolSha256[]=
    "D0861537AE33953169AD220E2E3416DF4D6B0BABFBDFF82CC553B85139879EC0";
constexpr char kSupportLadderProtocolSha256[]=
    "F1E8A18631D923040607128D34CCC6C2FF17D6D9D0BA594CBF57C7A9157BD48A";
constexpr std::array<int,8> kObserverTicks{{0,80,96,115,160,240,297,312}};
constexpr std::array<int,3> kSupportLadder{{4,6,8}};

struct ObserverDiscoveryRow {
  int tick=0;
  ftd::eft::StateOnlyMatterFieldObservation observation;
};

struct SupportLadderRow {
  int tick=0;
  ftd::eft::StateOnlySupportLadderObservation observation;
};

struct ObserverDiscoveryArm {
  HorizonArm horizon;
  std::vector<CanonicalSupportRow> support;
  std::vector<ObserverDiscoveryRow> observations;
  std::vector<SupportLadderRow> support_ladders;
  bool execution_pass=false;
  bool aggregation_pass=false;
  bool observer_pass=false;
  bool scalar_replay_exact=false;
  int scalar_rows_compared=0;
  double maximum_discarded_l1=0.0;
  double maximum_moment_residual=0.0;
};

bool observer_tick(int tick) {
  return std::find(kObserverTicks.begin(),kObserverTicks.end(),tick)
      !=kObserverTicks.end();
}

std::string format_horizon_row(const HorizonArm& arm,const HorizonRow& row) {
  std::ostringstream csv;
  csv<<std::setprecision(17)<<arm.slug<<','<<arm.direction<<",plus_minus,"
     <<row.tick<<','<<row.valid<<','<<row.common<<','<<row.regional_valid<<','
     <<row.maximum_residual<<','<<row.total_energy_residual<<','
     <<row.recoil_defect<<','<<row.speed_excess<<','<<row.regional_residual<<','
     <<row.outside_source_residual<<','<<row.source_radius<<','
     <<row.source_entries<<','<<row.separation<<','<<row.pair_energy<<','
     <<row.field_energy<<','<<row.graph_inside;
  for(std::size_t i=0;i<kHorizonRadii.size();++i)
    csv<<','<<row.inside[i]<<','<<row.outside[i]<<','<<row.transport_into[i]
       <<','<<row.source_exchange[i]<<','<<row.cumulative_outward[i];
  return csv.str();
}

bool compare_registered_scalar_rows(const HorizonArm& arm,int tick_limit,
                                    int& compared) {
  const auto path=std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results"/"ftd_0753"/
      ("ftd_0753_explicit_rounding_causal_horizon_m2_v1_"+arm.slug+".csv");
  std::ifstream input(path);
  std::string line;
  if(!std::getline(input,line)) return false;
  compared=0;
  for(int tick=0;tick<=tick_limit;++tick) {
    if(!std::getline(input,line)
        ||static_cast<std::size_t>(tick)>=arm.rows.size()
        ||line!=format_horizon_row(arm,arm.rows[static_cast<std::size_t>(tick)]))
      return false;
    ++compared;
  }
  return true;
}

bool append_observation(ObserverDiscoveryArm& result,int tick,
                        const ConnectedMooreBlockState& state,
                        const ConnectedMooreBlockOptions& options,
                        bool support_ladder) {
  if(!observer_tick(tick)) return true;
  ftd::eft::StateOnlyMatterFieldObserverOptions observer;
  observer.support_half_width=kHorizonSupportRadius;
  observer.shell_radii.assign(kHorizonRadii.begin(),kHorizonRadii.end());
  observer.wave_speed=options.wave_speed;
  observer.dt=options.dt;
  observer.poisson_tolerance=1e-13;
  observer.poisson_max_iterations=4096;
  observer.gate_tolerance=1e-12;
  auto value=ftd::eft::observe_state_only_matter_field(
      state,options,observer);
  const bool valid=value.valid;
  result.observations.push_back({tick,std::move(value)});
  if(!support_ladder) return valid;
  const std::vector<int> supports(
      kSupportLadder.begin(),kSupportLadder.end());
  auto ladder=ftd::eft::observe_state_only_support_ladder(
      state,options,supports,1e-13,4096,1e-12);
  const bool ladder_valid=ladder.valid;
  result.support_ladders.push_back({tick,std::move(ladder)});
  return valid&&ladder_valid;
}

ObserverDiscoveryArm run_observer_discovery_arm(
    const std::string& slug,const Direction& direction,
    const ConnectedMooreBlockOptions& input_options,double interaction_scale,
    int tick_limit,bool support_ladder=false) {
  ObserverDiscoveryArm result;
  auto& arm=result.horizon;
  arm.slug=slug; arm.direction=direction.label;
  arm.minimum_outward_increment.fill(INFINITY);
  arm.rows.reserve(static_cast<std::size_t>(tick_limit+1));
  result.support.reserve(static_cast<std::size_t>(tick_limit+1));
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
  if(!arm.preparation_pass) return result;

  ConnectedMooreBlockState state=prep.state;
  auto initial=make_horizon_initial(state,input_options,interaction_scale,center);
  arm.initial_pass=!initial.graph_inside&&initial.pair_energy>1e-6
      &&initial.outside[5]<=1e-12;
  arm.rows.push_back(std::move(initial));
  result.support.push_back({0,true,0,0,0,0.0,0.0,0.0,0.0,0.0});
  bool observer_valid=append_observation(
      result,0,state,input_options,support_ladder);

  auto options=input_options;
  options.defer_volume_diagnostics=true;
  ftd::eft::CudaMatchedFieldPipeline pipeline(kHorizonL);
  if(!pipeline.valid()||!pipeline.upload(state.electric,state.magnetic_half))
    return result;
  const double lambda=options.wave_speed*options.dt;
  const std::vector<int> radii(kHorizonRadii.begin(),kHorizonRadii.end());
  ftd::eft::MatchedEdgeField prepared_magnetic(kHorizonL);
  ftd::eft::MatchedFaceFlux prepared_electric(kHorizonL);
  bool valid=true,aggregation=true;
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
    const auto support=make_canonical_support_row(
        tick,step.segments,options.polarity_scale,center);
    if(!step.volume_diagnostics_pending||!support.valid
        ||!pipeline.apply_ordered_sparse_current(
            step.segments,options.polarity_scale)) {
      valid=false; break;
    }
    const auto profile=pipeline.observe_deterministic(
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
    row.source_entries=static_cast<int>(support.net_support);
    row.source_radius=support.source_radius;
    result.maximum_discarded_l1=std::max(
        result.maximum_discarded_l1,support.discarded_l1);
    result.maximum_moment_residual=std::max(
        result.maximum_moment_residual,support.moment_residual);
    aggregation=aggregation&&support.valid
        &&support.discarded_l1<=kHorizonGate
        &&support.moment_residual<=kCanonicalMomentGate
        &&support.source_radius<=3;
    arm.rows.push_back(std::move(row));
    result.support.push_back(support);
    observer_valid=append_observation(
        result,tick,state,options,support_ladder)&&observer_valid;
    if(!pipeline.advance()) { valid=false; break; }
  }
  result.execution_pass=valid
      &&arm.rows.size()==static_cast<std::size_t>(tick_limit+1)
      &&result.support.size()==arm.rows.size();
  result.aggregation_pass=result.execution_pass&&aggregation;
  result.observer_pass=observer_valid;
  result.scalar_replay_exact=result.execution_pass
      &&compare_registered_scalar_rows(
          arm,tick_limit,result.scalar_rows_compared);
  return result;
}

void write_observer_discovery(const ObserverDiscoveryArm& result) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0754";
  std::filesystem::create_directories(directory);
  const auto stem="ftd_0754_state_only_observer_discovery_v1_"
      +result.horizon.slug;
  std::ofstream csv(directory/(stem+".csv"));
  csv<<"arm,direction,tick,valid,scalar_replay_exact,center_x,center_y,"
      "center_z,kinetic_energy,pair_internal_energy,bound_energy,"
      "residual_energy,outgoing_energy,incoming_energy,radial_energy,"
      "background_energy,bound_residual_interference,signed_poynting,"
      "outward_power,inward_power,reconstruction_residual,gauss_residual,"
      "energy_partition_residual,characteristic_flux_residual,"
      "bound_poisson_residual,bound_gauss_residual,bound_outside_maximum,"
      "bound_boundary_maximum";
  for(int radius:kHorizonRadii)
    csv<<",shell_"<<radius<<"_out,shell_"<<radius<<"_in,"
       <<"shell_"<<radius<<"_radial,shell_"<<radius<<"_background,"
       <<"shell_"<<radius<<"_signed";
  csv<<'\n'<<std::setprecision(17);
  for(const auto& row:result.observations) {
    const auto& o=row.observation;
    csv<<result.horizon.slug<<','<<result.horizon.direction<<','<<row.tick<<','
       <<o.valid<<','<<result.scalar_replay_exact<<','
       <<o.center.x<<','<<o.center.y<<','<<o.center.z<<','
       <<o.constituent_kinetic_energy<<','<<o.pair_internal_energy<<','
       <<o.bound_energy<<','<<o.residual_energy<<','<<o.outgoing_energy<<','
       <<o.incoming_energy<<','<<o.radial_energy<<','<<o.background_energy<<','
       <<o.bound_residual_interference<<','<<o.signed_radial_poynting<<','
       <<o.outward_characteristic_power<<','
       <<o.inward_characteristic_power<<','
       <<o.maximum_reconstruction_residual<<','
       <<o.actual_gauss_compatibility_residual<<','
       <<o.energy_partition_residual<<','<<o.characteristic_flux_residual<<','
       <<o.bound_poisson_residual<<','<<o.bound_gauss_residual<<','
       <<o.bound_outside_maximum<<','<<o.bound_boundary_crossing_maximum;
    for(const auto& shell:o.shells)
      csv<<','<<shell.outgoing_energy<<','<<shell.incoming_energy<<','
         <<shell.radial_energy<<','<<shell.background_energy<<','
         <<shell.signed_radial_poynting;
    csv<<'\n';
  }
  std::ofstream json(directory/(stem+".json"));
  json<<std::setprecision(17)
      <<"{\n  \"ftd_id\": \"FTD-0754\",\n"
      <<"  \"protocol_sha256\": \""<<kObserverDiscoveryProtocolSha256
      <<"\",\n  \"scope\": \"discovery_replay_not_validation\",\n"
      <<"  \"backend\": \"wsl2_cuda_explicit_rounding_ordered\",\n"
      <<"  \"arm\": \""<<result.horizon.slug<<"\",\n"
      <<"  \"scalar_replay_exact\": "<<result.scalar_replay_exact<<",\n"
      <<"  \"scalar_rows_compared\": "<<result.scalar_rows_compared<<",\n"
      <<"  \"execution_pass\": "<<result.execution_pass<<",\n"
      <<"  \"aggregation_pass\": "<<result.aggregation_pass<<",\n"
      <<"  \"observer_pass\": "<<result.observer_pass<<",\n"
      <<"  \"observer_ticks\": [0,80,96,115,160,240,297,312],\n"
      <<"  \"readout\": \"odd_volume_centered_maxwell_characteristic\",\n"
      <<"  \"primitive_cochain_uniqueness_claimed\": false\n}\n";
}

void write_boundary_accounting(const ObserverDiscoveryArm& result) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0754_boundary_accounting";
  std::filesystem::create_directories(directory);
  const auto stem="ftd_0754b_boundary_accounting_v1_"
      +result.horizon.slug;
  std::ofstream csv(directory/(stem+".csv"));
  csv<<"arm,direction,tick,valid,scalar_replay_exact,boundary_ledger_valid,"
      "bound_energy,total_interference,primitive_face_interference,"
      "induced_boundary_interference,centering_metric_interference,"
      "centered_electric_interference,centered_magnetic_interference,"
      "boundary_flux_sum,primitive_boundary_identity_residual,"
      "readout_interference_reconstruction_residual\n"
      <<std::setprecision(17);
  for(const auto& row:result.observations) {
    const auto& o=row.observation;
    csv<<result.horizon.slug<<','<<result.horizon.direction<<','<<row.tick<<','
       <<o.valid<<','<<result.scalar_replay_exact<<','
       <<o.boundary_energy_ledger_valid<<','<<o.bound_energy<<','
       <<o.bound_residual_interference<<','
       <<o.primitive_face_interference<<','
       <<o.induced_boundary_interference<<','
       <<o.centering_metric_interference<<','
       <<o.centered_electric_interference<<','
       <<o.centered_magnetic_interference<<','
       <<o.boundary_flux_sum<<','
       <<o.primitive_boundary_identity_residual<<','
       <<o.readout_interference_reconstruction_residual<<'\n';
  }
  std::ofstream json(directory/(stem+".json"));
  json<<std::setprecision(17)
      <<"{\n  \"source_ftd_id\": \"FTD-0754\",\n"
      <<"  \"source_protocol_sha256\": \""
      <<kObserverDiscoveryProtocolSha256<<"\",\n"
      <<"  \"scope\": \"posthoc_existing_discovery_corpus_no_validation\",\n"
      <<"  \"arm\": \""<<result.horizon.slug<<"\",\n"
      <<"  \"scalar_replay_exact\": "<<result.scalar_replay_exact<<",\n"
      <<"  \"scalar_rows_compared\": "<<result.scalar_rows_compared<<",\n"
      <<"  \"observer_rows\": "<<result.observations.size()<<",\n"
      <<"  \"dynamics_changed\": false,\n"
      <<"  \"held_out_validation_consumed\": false\n}\n";
}

void write_support_ladder(const ObserverDiscoveryArm& result) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0754_support_ladder";
  std::filesystem::create_directories(directory);
  const auto stem="ftd_0754c_state_only_support_ladder_v1_"
      +result.horizon.slug;
  std::ofstream csv(directory/(stem+".csv"));
  csv<<"arm,direction,tick,valid,scalar_replay_exact,ladder_valid,"
      "support_half_width,actual_face_energy,bound_face_energy,"
      "residual_face_energy,primitive_interference,"
      "energy_reconstruction_residual,inner_half_width,outer_half_width,"
      "relaxation_energy,outer_difference_inner_product,"
      "pythagorean_residual,monotonicity_margin\n"
      <<std::setprecision(17);
  for(const auto& row:result.support_ladders) {
    const auto& ladder=row.observation;
    for(std::size_t i=0;i<ladder.scales.size();++i) {
      const auto& scale=ladder.scales[i];
      int inner=0,outer=0;
      double relaxation=0.0,projection=0.0,pythagorean=0.0,margin=0.0;
      if(i>0&&i-1<ladder.transitions.size()) {
        const auto& transition=ladder.transitions[i-1];
        inner=transition.inner_half_width;
        outer=transition.outer_half_width;
        relaxation=transition.relaxation_energy;
        projection=transition.outer_difference_inner_product;
        pythagorean=transition.pythagorean_residual;
        margin=transition.monotonicity_margin;
      }
      csv<<result.horizon.slug<<','<<result.horizon.direction<<','
         <<row.tick<<','<<scale.valid<<','<<result.scalar_replay_exact<<','
         <<ladder.valid<<','<<scale.support_half_width<<','
         <<scale.actual_face_energy<<','<<scale.bound_face_energy<<','
         <<scale.residual_face_energy<<','<<scale.primitive_interference<<','
         <<scale.energy_reconstruction_residual<<','<<inner<<','<<outer<<','
         <<relaxation<<','<<projection<<','<<pythagorean<<','<<margin<<'\n';
    }
  }
  std::ofstream json(directory/(stem+".json"));
  json<<std::setprecision(17)
      <<"{\n  \"source_ftd_id\": \"FTD-0754\",\n"
      <<"  \"analytic_addendum\": \"FTD-0754C\",\n"
      <<"  \"protocol_sha256\": \""<<kSupportLadderProtocolSha256
      <<"\",\n"
      <<"  \"scope\": \"posthoc_existing_discovery_corpus_no_validation\",\n"
      <<"  \"arm\": \""<<result.horizon.slug<<"\",\n"
      <<"  \"support_half_widths\": [4,6,8],\n"
      <<"  \"observer_rows\": "<<result.support_ladders.size()<<",\n"
      <<"  \"scalar_replay_exact\": "<<result.scalar_replay_exact<<",\n"
      <<"  \"dynamics_changed\": false,\n"
      <<"  \"held_out_validation_consumed\": false\n}\n";
}

}  // namespace

#ifndef FTD_0754_MAIN_NAME
#define FTD_0754_MAIN_NAME main
#endif
int FTD_0754_MAIN_NAME(int argc,char** argv) {
  const bool qualification=argc==4&&std::string(argv[1])=="--qualify";
  const bool accounting=argc==3&&std::string(argv[1])=="--account";
  const bool ladder=argc==3&&std::string(argv[1])=="--ladder";
  const bool registered=argc==2;
  if(!qualification&&!accounting&&!ladder&&!registered) {
    std::cout<<"FTD-0754 discovery replay: registered face|edge|body; "
        "qualification uses --qualify face|edge|body N; post-hoc existing-"
        "corpus boundary ledger uses --account face|edge|body; nested support "
        "projection uses --ladder face|edge|body\n";
    return argc==1?0:2;
  }
  Direction direction;
  const std::string slug=argv[qualification||accounting||ladder?2:1];
  if(!select_horizon_direction(slug,direction)) return 2;
  const int ticks=qualification?std::stoi(argv[3]):kHorizonTicks;
  if(qualification&&(ticks<1||ticks>8)) return 2;
  if(registered&&std::string(kObserverDiscoveryProtocolSha256)=="UNLOCKED") {
    std::cerr<<"FTD-0754 registered replay refused before protocol lock\n";
    return 3;
  }
  if(ladder&&std::string(kSupportLadderProtocolSha256)=="UNLOCKED") {
    std::cerr<<"FTD-0754C support ladder refused before protocol lock\n";
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
  auto result=run_observer_discovery_arm(slug,direction,options,
      normalization.mapped_field_work_coefficient,ticks,ladder);
  const double seconds=std::chrono::duration<double>(
      std::chrono::steady_clock::now()-start).count();
  const bool pass=normalization.valid&&result.execution_pass
      &&result.aggregation_pass&&result.observer_pass
      &&result.scalar_replay_exact;
  if(registered) write_observer_discovery(result);
  if(accounting) write_boundary_accounting(result);
  if(ladder) write_support_ladder(result);
  std::cout<<std::setprecision(17)<<"FTD-0754 "<<slug
      <<(ladder?" support_ladder=":" discovery=")<<pass
      <<" replay="<<result.scalar_replay_exact
      <<" rows="<<result.scalar_rows_compared
      <<" observations="<<result.observations.size()
      <<" ladders="<<result.support_ladders.size()
      <<" seconds="<<seconds<<" protocol="
      <<kObserverDiscoveryProtocolSha256<<'\n';
  return pass?0:1;
}
