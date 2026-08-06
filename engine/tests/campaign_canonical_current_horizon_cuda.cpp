/** FTD-0748 candidate: canonical net-current CUDA horizon successor. */

#define main ftd_0747_frozen_main
#include "campaign_causal_horizon_environmental_persistence_cuda.cpp"
#undef main
#ifdef FTD_0748_MAIN_NAME
#define main FTD_0748_MAIN_NAME
#endif

#include <chrono>

namespace {

constexpr char kCanonicalProtocolSha256[]=
    "D01039341BCA3098C9F837549A26199CCE5BB6660C84A7C86C5037D17A2B0C46";
constexpr double kCanonicalMomentGate=1e-12;

struct CanonicalSupportRow {
  int tick=0;
  bool valid=false;
  std::size_t raw_contributions=0,net_support=0;
  int source_radius=0;
  double raw_l1=0.0,net_l1=0.0,cancelled_l1=0.0;
  double discarded_l1=0.0,moment_residual=INFINITY;
};

struct CanonicalHorizonArm {
  HorizonArm horizon;
  std::vector<CanonicalSupportRow> support;
  bool aggregation_pass=false;
  double maximum_discarded_l1=0.0;
  double maximum_moment_residual=0.0;
  std::size_t maximum_net_support=0;
};

int canonical_source_radius(
    const ftd::eft::QuadraticCoatAggregatedCurrent& current,
    const Vec3& center) {
  int maximum=0;
  const int cx=static_cast<int>(std::llround(center.x));
  const int cy=static_cast<int>(std::llround(center.y));
  const int cz=static_cast<int>(std::llround(center.z));
  for(const auto& entry:current.entries) {
    const int radius=1+std::max({
        horizon_periodic_abs(entry.face.x,cx,current.L),
        horizon_periodic_abs(entry.face.y,cy,current.L),
        horizon_periodic_abs(entry.face.z,cz,current.L)});
    maximum=std::max(maximum,radius);
  }
  return maximum;
}

CanonicalSupportRow make_canonical_support_row(
    int tick,const std::vector<ftd::eft::QuadraticCoatFaceCurrent>& segments,
    double polarity_scale,const Vec3& center) {
  CanonicalSupportRow row;
  row.tick=tick;
  const auto current=ftd::eft::aggregate_quadratic_coat_face_current(
      segments,polarity_scale,kHorizonGate);
  row.valid=current.valid;
  row.raw_contributions=current.raw_contributions;
  row.net_support=current.entries.size();
  row.source_radius=canonical_source_radius(current,center);
  row.raw_l1=current.raw_l1;
  row.net_l1=current.net_l1;
  row.cancelled_l1=current.cancelled_l1;
  row.discarded_l1=current.discarded_l1;
  row.moment_residual=current.aggregation_moment_residual;
  return row;
}

CanonicalHorizonArm run_canonical_horizon_cuda_arm(
    const std::string& slug,const Direction& direction,
    const ConnectedMooreBlockOptions& input_options,double interaction_scale,
    int tick_limit) {
  CanonicalHorizonArm result;
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
  auto initial=make_horizon_initial(
      state,input_options,interaction_scale,center);
  arm.initial_pass=!initial.graph_inside&&initial.pair_energy>1e-6
      &&initial.outside[5]<=1e-12;
  arm.rows.push_back(std::move(initial));
  result.support.push_back({0,true,0,0,0,0.0,0.0,0.0,0.0,0.0});

  auto options=input_options;
  options.defer_volume_diagnostics=true;
  ftd::eft::CudaMatchedFieldPipeline pipeline(kHorizonL);
  if(!pipeline.valid()||!pipeline.upload(state.electric,state.magnetic_half))
    return result;
  const double lambda=options.wave_speed*options.dt;
  const std::vector<int> radii(kHorizonRadii.begin(),kHorizonRadii.end());
  ftd::eft::MatchedEdgeField prepared_magnetic(kHorizonL);
  ftd::eft::MatchedFaceFlux prepared_electric(kHorizonL);
  bool valid=true,exact=true,aggregation=true;
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
        ||!pipeline.apply_sparse_current(step.segments,options.polarity_scale)) {
      valid=false; break;
    }
    const auto profile=pipeline.observe(lambda,center,radii,kHorizonGate);
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
    result.maximum_net_support=std::max(
        result.maximum_net_support,support.net_support);
    aggregation=aggregation&&support.valid
        &&support.discarded_l1<=kHorizonGate
        &&support.moment_residual<=kCanonicalMomentGate
        &&support.source_radius<=3;
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
    result.support.push_back(support);
    if(!pipeline.advance()) { valid=false; break; }
  }
  arm.forward_executed=valid
      &&arm.rows.size()==static_cast<std::size_t>(tick_limit+1)
      &&result.support.size()==arm.rows.size();
  result.aggregation_pass=arm.forward_executed&&aggregation;
  if(!arm.forward_executed||tick_limit!=kHorizonTicks) return result;

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
  auto prefix_arm=arm;
  if(baseline_valid) for(int tick=0;tick<=kHorizonPrefixTicks;++tick)
    prefix_arm.rows[static_cast<std::size_t>(tick)].source_entries=
        baseline.at(tick).source_entries;
  arm.prefix_scalar_difference=baseline_valid
      ?horizon_prefix_difference(
          prefix_arm,baseline,arm.prefix_discrete_pass):INFINITY;
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
  return result;
}

std::string canonical_horizon_verdict(const CanonicalHorizonArm& value) {
  const auto& arm=value.horizon;
  const bool infrastructure=arm.initialized&&arm.preparation_pass
      &&arm.initial_pass&&arm.forward_executed&&arm.exact_pass&&arm.support_pass;
  if(!infrastructure) return "CANONICAL_HORIZON_EXECUTION_INVALID";
  if(!value.aggregation_pass) return "CANONICAL_HORIZON_CURRENT_AGGREGATION_INVALID";
  if(!arm.prefix_pass) return "CANONICAL_HORIZON_PREFIX_DRIFT";
  if(!arm.core_pass) return "CANONICAL_HORIZON_CORE_NOT_PERSISTENT";
  if(!arm.near_field_pass) return "CANONICAL_HORIZON_NEAR_FIELD_NOT_STABLE";
  if(!arm.arrival_pass) return "CANONICAL_HORIZON_R48_ARRIVAL_FAIL";
  if(!arm.post_arrival_pass)
    return "CANONICAL_HORIZON_POST_ARRIVAL_NOT_PERSISTENT";
  return "CANONICAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE";
}

void write_canonical_support_records(
    const CanonicalHorizonArm& value,const std::string& verdict) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0748";
  std::filesystem::create_directories(directory);
  const auto stem="ftd_0748_canonical_current_horizon_cuda_v1_"
      +value.horizon.slug+"_support";
  std::ofstream csv(directory/(stem+".csv"));
  csv<<"tick,valid,raw_contributions,net_support,source_radius,raw_l1,"
      "net_l1,cancelled_l1,discarded_l1,moment_residual\n"
      <<std::setprecision(17);
  for(const auto& row:value.support)
    csv<<row.tick<<','<<row.valid<<','<<row.raw_contributions<<','
       <<row.net_support<<','<<row.source_radius<<','<<row.raw_l1<<','
       <<row.net_l1<<','<<row.cancelled_l1<<','<<row.discarded_l1<<','
       <<row.moment_residual<<'\n';
  std::ofstream json(directory/(stem+".json"));
  json<<std::setprecision(17)
      <<"{\n  \"ftd_id\": \"FTD-0748\",\n"
      <<"  \"protocol_sha256\": \""<<kCanonicalProtocolSha256<<"\",\n"
      <<"  \"backend\": \"wsl2_cuda_canonical_net_current\",\n"
      <<"  \"arm\": \""<<value.horizon.slug<<"\",\n"
      <<"  \"verdict\": \""<<verdict<<"\",\n"
      <<"  \"aggregation_pass\": "<<value.aggregation_pass<<",\n"
      <<"  \"maximum_net_support\": "<<value.maximum_net_support<<",\n"
      <<"  \"maximum_discarded_l1\": "<<value.maximum_discarded_l1<<",\n"
      <<"  \"maximum_moment_residual\": "
      <<value.maximum_moment_residual<<"\n}\n";
}

}  // namespace

int main(int argc,char** argv) {
  const bool qualification=argc==4&&std::string(argv[1])=="--qualify";
  const bool held_out=argc==2;
  if(!qualification&&!held_out) {
    std::cout<<"FTD-0748 CUDA: held-out face|edge|body; qualification uses "
        "--qualify face|edge|body N\n";
    return argc==1?0:2;
  }
  Direction direction;
  const std::string slug=argv[qualification?2:1];
  if(!select_horizon_direction(slug,direction)) return 2;
  const int ticks=qualification?std::stoi(argv[3]):kHorizonTicks;
  if(qualification&&(ticks<1||ticks>8)) return 2;
  if(held_out&&std::string(kCanonicalProtocolSha256)=="UNLOCKED") {
    std::cerr<<"FTD-0748 held-out execution refused before protocol lock\n";
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
  auto result=run_canonical_horizon_cuda_arm(slug,direction,options,
      normalization.mapped_field_work_coefficient,ticks);
  const double seconds=std::chrono::duration<double>(
      std::chrono::steady_clock::now()-start).count();
  if(qualification) {
    std::cout<<std::setprecision(17)<<"FTD-0748 qualification "<<slug
      <<" ticks="<<ticks<<" rows="<<result.horizon.rows.size()
      <<" aggregation="<<result.aggregation_pass
      <<" max_support="<<result.maximum_net_support
      <<" max_discarded="<<result.maximum_discarded_l1
      <<" max_moment="<<result.maximum_moment_residual
      <<" seconds="<<seconds<<" protocol="<<kCanonicalProtocolSha256<<'\n';
    return normalization.valid&&result.horizon.initialized
        &&result.horizon.preparation_pass&&result.horizon.initial_pass
        &&result.horizon.forward_executed&&result.aggregation_pass?0:1;
  }
  if(!normalization.valid) result.horizon.exact_pass=false;
  const auto verdict=canonical_horizon_verdict(result);
  write_horizon_records(result.horizon,verdict,"FTD-0748",
      kCanonicalProtocolSha256,"ftd_0748",
      "ftd_0748_canonical_current_horizon_cuda_v1",
      "wsl2_cuda_canonical_net_current");
  write_canonical_support_records(result,verdict);
  std::cout<<"FTD-0748 "<<slug<<' '<<verdict
      <<" prefix="<<std::setprecision(8)
      <<result.horizon.prefix_scalar_difference
      <<" support="<<result.maximum_net_support
      <<" r48_tick="<<result.horizon.first_tail_tick[5]
      <<" seconds="<<seconds<<'\n';
  return verdict=="CANONICAL_HORIZON_EXECUTION_INVALID"?1:0;
}

#ifdef FTD_0748_MAIN_NAME
#undef main
#endif
