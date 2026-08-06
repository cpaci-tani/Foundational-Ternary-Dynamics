/** FTD-0746: causal-horizon radius-48 environmental persistence. */

#ifndef main
#define main ftd_0731_source_of_record_main
#define FTD_CAUSAL_HORIZON_OWNS_MAIN_RENAME
#endif
#include "test_multipass_formation_persistence.cpp"
#ifdef FTD_CAUSAL_HORIZON_OWNS_MAIN_RENAME
#undef main
#undef FTD_CAUSAL_HORIZON_OWNS_MAIN_RENAME
#endif

#include "ftd/eft/batched_regional_energy_profile.h"
#include "ftd/eft/matched_regional_energy_transport.h"

#include <array>
#include <map>
#include <sstream>

namespace {

constexpr char kHorizonProtocolSha256[] =
    "B98DB9B18050D1799814ABD0B6C70936BF631AEF258CF969FC8D15E7B8DCA9A0";
constexpr char kHorizonBaselineSha256[] =
    "58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C";
constexpr int kHorizonL = 321;
constexpr int kHorizonTicks = 312;
constexpr int kHorizonSupportRadius = 4;
constexpr int kHorizonContactTick = kHorizonL-2*kHorizonSupportRadius;
constexpr int kHorizonPrefixTicks = 184;
constexpr std::array<int,6> kHorizonRadii{8,12,16,24,32,48};
constexpr int kHorizonLateBegin = 281;
constexpr int kHorizonArrivalDeadline = 300;
constexpr int kHorizonPostArrivalBegin = 301;
constexpr double kHorizonGate = 1e-10;
constexpr double kHorizonTailThreshold = 1e-8;
constexpr double kHorizonTailFinalThreshold = 1e-9;
constexpr double kHorizonNearMinimum = 5e-4;
constexpr double kHorizonNearDynamicRange = 4.0;

int horizon_periodic_abs(int value, int center, int L) {
  const int direct=std::abs(value-center);
  return std::min(direct,L-direct);
}

int horizon_source_radius(
    const std::vector<ftd::eft::QuadraticCoatFaceCurrent>& segments,
    const Vec3& center, int& entries) {
  int maximum=0;
  entries=0;
  const int cx=static_cast<int>(std::llround(center.x));
  const int cy=static_cast<int>(std::llround(center.y));
  const int cz=static_cast<int>(std::llround(center.z));
  for(const auto& segment:segments) for(const auto& entry:segment.sparse_current) {
    if(entry.value==0.0) continue;
    ++entries;
    const int radius=1+std::max({
        horizon_periodic_abs(entry.face.x,cx,segment.L),
        horizon_periodic_abs(entry.face.y,cy,segment.L),
        horizon_periodic_abs(entry.face.z,cz,segment.L)});
    maximum=std::max(maximum,radius);
  }
  return maximum;
}

ftd::eft::MatchedFaceFlux horizon_pre_current_field(
    const ftd::eft::ConnectedMooreBlockStepResult& step, double lambda) {
  auto result=step.earlier.electric;
  const auto curl=ftd::eft::matched_curl(step.later.magnetic_half);
  for(std::size_t i=0;i<result.x.size();++i) {
    result.x[i]+=lambda*curl.x[i];
    result.y[i]+=lambda*curl.y[i];
    result.z[i]+=lambda*curl.z[i];
  }
  return result;
}

double horizon_regional_residual(
    const ftd::eft::MatchedRegionalEnergyTransportResult& value) {
  return std::max({value.magnetic_update_residual,
      value.electric_pre_update_residual,value.global_source_free_residual,
      value.partition_residual,value.regional_ledger_residual});
}

struct HorizonRow {
  int tick=0;
  bool valid=false,common=false,regional_valid=false,graph_inside=false;
  double maximum_residual=INFINITY,total_energy_residual=INFINITY;
  double recoil_defect=INFINITY,speed_excess=INFINITY;
  double regional_residual=INFINITY,outside_source_residual=INFINITY;
  int source_radius=0,source_entries=0;
  double separation=INFINITY,pair_energy=INFINITY,field_energy=INFINITY;
  std::array<double,6> inside{},outside{},transport_into{};
  std::array<double,6> source_exchange{},cumulative_outward{};
};

struct HorizonArm {
  std::string slug,direction;
  bool initialized=false,preparation_pass=false,initial_pass=false;
  bool forward_executed=false,exact_pass=false,support_pass=false;
  bool prefix_discrete_pass=false,prefix_pass=false;
  bool core_pass=false,near_field_pass=false,arrival_pass=false;
  bool post_arrival_pass=false;
  int energetic_onset_tick=-1;
  std::array<int,6> first_tail_tick{-1,-1,-1,-1,-1,-1};
  std::array<double,6> maximum_outside{},final_outside{};
  std::array<double,6> minimum_outward_increment{};
  int maximum_source_radius=0;
  double maximum_common_residual=0.0,maximum_energy_residual=0.0;
  double maximum_recoil_defect=0.0,maximum_speed_excess=0.0;
  double maximum_regional_residual=0.0,maximum_outside_source=0.0;
  double pair_field_balance=INFINITY,prefix_scalar_difference=INFINITY;
  double late_inside_8_minimum=INFINITY,late_inside_8_maximum=0.0;
  double post_arrival_48_minimum=INFINITY;
  std::vector<HorizonRow> rows;
};

HorizonRow make_horizon_initial(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& options,double interaction_scale,
    const Vec3& center) {
  HorizonRow row;
  row.valid=row.common=row.regional_valid=true;
  row.maximum_residual=row.total_energy_residual=0.0;
  row.recoil_defect=row.speed_excess=row.outside_source_residual=0.0;
  row.separation=pair_separation(state);
  row.pair_energy=pair_internal_energy(state,options);
  row.field_energy=field_energy(state,options,interaction_scale);
  row.graph_inside=graph_inside(row.separation,options);
  row.regional_residual=0.0;
  const double lambda=options.wave_speed*options.dt;
  for(std::size_t i=0;i<kHorizonRadii.size();++i) {
    const auto snapshot=ftd::eft::measure_matched_regional_energy(
        state.electric,state.magnetic_half,lambda,center,kHorizonRadii[i],
        kHorizonGate);
    row.regional_valid=row.regional_valid&&snapshot.valid;
    row.regional_residual=std::max(
        row.regional_residual,snapshot.partition_residual);
    row.inside[i]=interaction_scale*snapshot.inside_energy;
    row.outside[i]=interaction_scale*snapshot.outside_energy;
  }
  return row;
}

HorizonRow make_horizon_record(
    int tick,const ftd::eft::ConnectedMooreBlockStepResult& step,
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& options,double interaction_scale,
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
  row.field_energy=field_energy(state,options,interaction_scale);
  row.graph_inside=graph_inside(row.separation,options);

  const double lambda=options.wave_speed*options.dt;
  const auto pre=horizon_pre_current_field(step,lambda);
  const std::vector<int> radii(kHorizonRadii.begin(),kHorizonRadii.end());
  const auto profile=ftd::eft::evaluate_batched_regional_energy_profile(
      step.earlier.electric,step.earlier.magnetic_half,pre,
      step.later.magnetic_half,step.later.electric,lambda,center,radii,
      kHorizonGate);
  row.regional_valid=profile.valid&&profile.regions.size()==radii.size();
  row.regional_residual=profile.maximum_scalar_equivalence_residual;
  row.outside_source_residual=0.0;
  if(!row.regional_valid) return row;
  const double total_after=ftd::eft::matched_modified_energy(
      step.later.electric,step.later.magnetic_half,lambda);
  const double total_pre=ftd::eft::matched_modified_energy(
      pre,step.later.magnetic_half,lambda);
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

int horizon_negative_onset(const HorizonArm& arm,
                           const ConnectedMooreBlockOptions& options) {
  for(int tick=0;tick<=kHorizonTicks;++tick) {
    bool tail=true;
    for(int later=tick;later<=kHorizonTicks;++later) {
      const auto& row=arm.rows[static_cast<std::size_t>(later)];
      tail=tail&&row.pair_energy<-1e-6&&graph_inside(row.separation,options);
      if(!tail) break;
    }
    if(tail) return tick;
  }
  return -1;
}

struct HorizonBaselineRow {
  bool valid=false,common=false,regional_valid=false,graph_inside=false;
  int source_radius=0,source_entries=0;
  std::array<double,39> scalar{};
};

using HorizonBaseline=std::map<int,HorizonBaselineRow>;

std::vector<std::string> horizon_split_csv(const std::string& line) {
  std::vector<std::string> result;
  std::stringstream stream(line);
  std::string field;
  while(std::getline(stream,field,',')) {
    // std::getline removes LF but retains a Windows CR on the final field
    // when this baseline is read by the WSL campaign executable.
    if(!field.empty()&&field.back()=='\r') field.pop_back();
    result.push_back(field);
  }
  return result;
}

HorizonBaseline load_horizon_baseline(
    const std::string& direction,bool& valid) {
  valid=false;
  const auto path=std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results"/"ftd_0745"
      /"ftd_0745_finite_support_environmental_closure_v1.csv";
  std::ifstream input(path);
  std::string line;
  if(!std::getline(input,line)) return {};
  const auto header=horizon_split_csv(line);
  std::map<std::string,std::size_t> column;
  for(std::size_t i=0;i<header.size();++i) column[header[i]]=i;
  const std::array<std::string,14> required{
      "family","direction","polarity","tick","valid","common",
      "regional_valid","source_radius","source_entries","graph_inside",
      "max_residual","total_energy_residual","recoil_defect","speed_excess"};
  for(const auto& name:required) if(!column.count(name)) return {};
  HorizonBaseline result;
  while(std::getline(input,line)) {
    const auto value=horizon_split_csv(line);
    if(value.size()!=header.size()||value[column["family"]]!="unbound"
        ||value[column["direction"]]!=direction
        ||value[column["polarity"]]!="plus_minus") continue;
    const int tick=std::stoi(value[column["tick"]]);
    if(tick>kHorizonPrefixTicks) continue;
    HorizonBaselineRow row;
    row.valid=std::stoi(value[column["valid"]])!=0;
    row.common=std::stoi(value[column["common"]])!=0;
    row.regional_valid=std::stoi(value[column["regional_valid"]])!=0;
    row.source_radius=std::stoi(value[column["source_radius"]]);
    row.source_entries=std::stoi(value[column["source_entries"]]);
    row.graph_inside=std::stoi(value[column["graph_inside"]])!=0;
    const std::array<std::string,9> base_names{
        "max_residual","total_energy_residual","recoil_defect","speed_excess",
        "regional_residual","outside_source_residual","separation",
        "pair_energy","field_energy"};
    std::size_t index=0;
    for(const auto& name:base_names) {
      if(!column.count(name)) return {};
      row.scalar[index++]=std::stod(value[column[name]]);
    }
    for(int radius:kHorizonRadii) for(const auto* prefix:{"inside_","outside_",
        "transport_into_","source_exchange_","cumulative_outward_"}) {
      const auto name=std::string(prefix)+std::to_string(radius);
      if(!column.count(name)) return {};
      row.scalar[index++]=std::stod(value[column[name]]);
    }
    result.emplace(tick,row);
  }
  valid=result.size()==static_cast<std::size_t>(kHorizonPrefixTicks+1);
  return result;
}

double horizon_prefix_difference(
    const HorizonArm& arm,const HorizonBaseline& baseline,bool& discrete_pass) {
  discrete_pass=true;
  double maximum=0.0;
  for(int tick=0;tick<=kHorizonPrefixTicks;++tick) {
    const auto found=baseline.find(tick);
    if(found==baseline.end()) { discrete_pass=false; return INFINITY; }
    const auto& now=arm.rows[static_cast<std::size_t>(tick)];
    const auto& old=found->second;
    discrete_pass=discrete_pass&&now.valid==old.valid&&now.common==old.common
        &&now.regional_valid==old.regional_valid
        &&now.source_radius==old.source_radius
        &&now.source_entries==old.source_entries
        &&now.graph_inside==old.graph_inside;
    std::array<double,39> scalar{};
    std::size_t index=0;
    for(double value:{now.maximum_residual,now.total_energy_residual,
        now.recoil_defect,now.speed_excess,now.regional_residual,
        now.outside_source_residual,now.separation,now.pair_energy,
        now.field_energy}) scalar[index++]=value;
    for(std::size_t i=0;i<kHorizonRadii.size();++i)
      for(double value:{now.inside[i],now.outside[i],now.transport_into[i],
          now.source_exchange[i],now.cumulative_outward[i]})
        scalar[index++]=value;
    for(std::size_t i=0;i<scalar.size();++i)
      maximum=std::max(maximum,std::abs(scalar[i]-old.scalar[i]));
  }
  return maximum;
}

HorizonArm run_horizon_arm(
    const std::string& slug,const Direction& direction,
    const ConnectedMooreBlockOptions& options,double interaction_scale) {
  HorizonArm arm;
  arm.slug=slug; arm.direction=direction.label;
  arm.minimum_outward_increment.fill(INFINITY);
  arm.rows.reserve(static_cast<std::size_t>(kHorizonTicks+1));
  const Vec3 center{static_cast<double>(kHorizonL/2),
                    static_cast<double>(kHorizonL/2),
                    static_cast<double>(kHorizonL/2)};
  const auto prep=ftd::eft::prepare_finite_support_derived_compact_pair(
      make_geometry(kHorizonL,direction,false,1.30,0.0120),options,
      kHorizonSupportRadius,1e-13,4096);
  arm.initialized=prep.valid;
  arm.preparation_pass=prep.valid&&prep.density_contained&&prep.compact_support
      &&prep.zero_boundary_crossing&&prep.poisson_residual<=1e-13
      &&prep.gauss_residual<=1e-12&&prep.outside_maximum==0.0
      &&prep.boundary_crossing_maximum==0.0;
  if(!arm.preparation_pass) return arm;

  ConnectedMooreBlockState state=prep.state;
  auto initial=make_horizon_initial(state,options,interaction_scale,center);
  arm.initial_pass=!initial.graph_inside&&initial.pair_energy>1e-6
      &&initial.outside[5]<=1e-12;
  arm.rows.push_back(std::move(initial));
  bool valid=true,exact=true;
  std::array<double,6> cumulative_outward{};
  ConnectedMooreBlockSolveCache cache;
  for(int tick=1;tick<=kHorizonTicks;++tick) {
    const auto step=ftd::eft::solve_connected_moore_block_forward(
        state,options,&cache);
    valid=valid&&step.valid;
    if(!step.valid) break;
    state=step.later;
    auto row=make_horizon_record(tick,step,state,options,interaction_scale,
        center,cumulative_outward);
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
  }
  arm.forward_executed=valid
      &&arm.rows.size()==static_cast<std::size_t>(kHorizonTicks+1);
  if(!arm.forward_executed) return arm;

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

  const std::size_t r48=5;
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

void horizon_json_number(std::ostream& output,double value) {
  if(std::isfinite(value)) output<<value;
  else output<<"null";
}

std::string horizon_verdict(const HorizonArm& arm) {
  const bool infrastructure=arm.initialized&&arm.preparation_pass
      &&arm.initial_pass&&arm.forward_executed&&arm.exact_pass&&arm.support_pass;
  if(!infrastructure) return "CAUSAL_HORIZON_EXECUTION_INVALID";
  if(!arm.prefix_pass) return "CAUSAL_HORIZON_PREFIX_DRIFT";
  if(!arm.core_pass) return "CAUSAL_HORIZON_CORE_NOT_PERSISTENT";
  if(!arm.near_field_pass) return "CAUSAL_HORIZON_NEAR_FIELD_NOT_STABLE";
  if(!arm.arrival_pass) return "CAUSAL_HORIZON_R48_ARRIVAL_FAIL";
  if(!arm.post_arrival_pass)
    return "CAUSAL_HORIZON_POST_ARRIVAL_NOT_PERSISTENT";
  return "CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE";
}

void write_horizon_records(
    const HorizonArm& arm,const std::string& verdict,
    const std::string& ftd_id="FTD-0746",
    const std::string& protocol_sha256=kHorizonProtocolSha256,
    const std::string& result_folder="ftd_0746",
    const std::string& stem_prefix=
        "ftd_0746_causal_horizon_environmental_persistence_v1",
    const std::string& backend="wsl2_cpu") {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/result_folder;
  std::filesystem::create_directories(directory);
  const auto stem=stem_prefix+"_"+arm.slug;
  std::ofstream csv(directory/(stem+".csv"));
  csv<<"arm,direction,polarity,tick,valid,common,regional_valid,max_residual,"
        "total_energy_residual,recoil_defect,speed_excess,regional_residual,"
        "outside_source_residual,source_radius,source_entries,separation,"
        "pair_energy,field_energy,graph_inside";
  for(int radius:kHorizonRadii)
    csv<<",inside_"<<radius<<",outside_"<<radius<<",transport_into_"<<radius
       <<",source_exchange_"<<radius<<",cumulative_outward_"<<radius;
  csv<<'\n'<<std::setprecision(17);
  for(const auto& row:arm.rows) {
    csv<<arm.slug<<','<<arm.direction<<",plus_minus,"<<row.tick<<','
       <<row.valid<<','<<row.common<<','<<row.regional_valid<<','
       <<row.maximum_residual<<','<<row.total_energy_residual<<','
       <<row.recoil_defect<<','<<row.speed_excess<<','<<row.regional_residual<<','
       <<row.outside_source_residual<<','<<row.source_radius<<','
       <<row.source_entries<<','<<row.separation<<','<<row.pair_energy<<','
       <<row.field_energy<<','<<row.graph_inside;
    for(std::size_t i=0;i<kHorizonRadii.size();++i)
      csv<<','<<row.inside[i]<<','<<row.outside[i]<<','<<row.transport_into[i]
         <<','<<row.source_exchange[i]<<','<<row.cumulative_outward[i];
    csv<<'\n';
  }

  std::ofstream json(directory/(stem+".json"));
  json<<std::setprecision(17)<<"{\n"
      <<"  \"ftd_id\": \""<<ftd_id<<"\",\n"
      <<"  \"protocol_sha256\": \""<<protocol_sha256<<"\",\n"
      <<"  \"baseline_csv_sha256\": \""<<kHorizonBaselineSha256<<"\",\n"
      <<"  \"backend\": \""<<backend<<"\",\n"
      <<"  \"arm\": \""<<arm.slug<<"\",\n"
      <<"  \"direction\": \""<<arm.direction<<"\",\n"
      <<"  \"polarity\": \"plus_minus\",\n"
      <<"  \"verdict\": \""<<verdict<<"\",\n"
      <<"  \"volume\": "<<kHorizonL<<",\n"
      <<"  \"horizon\": "<<kHorizonTicks<<",\n"
      <<"  \"contact_tick\": "<<kHorizonContactTick<<",\n"
      <<"  \"radii\": [8,12,16,24,32,48],\n"
      <<"  \"tail_threshold\": "<<kHorizonTailThreshold<<",\n"
      <<"  \"tail_final_threshold\": "<<kHorizonTailFinalThreshold<<",\n"
      <<"  \"arrival_deadline\": "<<kHorizonArrivalDeadline<<",\n"
      <<"  \"post_arrival_begin\": "<<kHorizonPostArrivalBegin<<",\n"
      <<"  \"inverse_tested\": false,\n"
      <<"  \"initialized\": "<<arm.initialized<<",\n"
      <<"  \"preparation_pass\": "<<arm.preparation_pass<<",\n"
      <<"  \"initial_pass\": "<<arm.initial_pass<<",\n"
      <<"  \"forward_executed\": "<<arm.forward_executed<<",\n"
      <<"  \"exact_pass\": "<<arm.exact_pass<<",\n"
      <<"  \"support_pass\": "<<arm.support_pass<<",\n"
      <<"  \"prefix_discrete_pass\": "<<arm.prefix_discrete_pass<<",\n"
      <<"  \"prefix_pass\": "<<arm.prefix_pass<<",\n"
      <<"  \"core_pass\": "<<arm.core_pass<<",\n"
      <<"  \"near_field_pass\": "<<arm.near_field_pass<<",\n"
      <<"  \"arrival_pass\": "<<arm.arrival_pass<<",\n"
      <<"  \"post_arrival_pass\": "<<arm.post_arrival_pass<<",\n"
      <<"  \"energetic_onset_tick\": "<<arm.energetic_onset_tick<<",\n"
      <<"  \"maximum_source_radius\": "<<arm.maximum_source_radius<<",\n"
      <<"  \"maximum_common_residual\": ";
  horizon_json_number(json,arm.maximum_common_residual);
  json<<",\n  \"maximum_energy_residual\": ";
  horizon_json_number(json,arm.maximum_energy_residual);
  json<<",\n  \"maximum_recoil_defect\": ";
  horizon_json_number(json,arm.maximum_recoil_defect);
  json<<",\n  \"maximum_speed_excess\": ";
  horizon_json_number(json,arm.maximum_speed_excess);
  json<<",\n  \"maximum_regional_residual\": ";
  horizon_json_number(json,arm.maximum_regional_residual);
  json<<",\n  \"maximum_outside_source\": ";
  horizon_json_number(json,arm.maximum_outside_source);
  json<<",\n  \"pair_field_balance\": ";
  horizon_json_number(json,arm.pair_field_balance);
  json<<",\n  \"prefix_scalar_difference\": ";
  horizon_json_number(json,arm.prefix_scalar_difference);
  json<<",\n  \"late_inside_8_minimum\": ";
  horizon_json_number(json,arm.late_inside_8_minimum);
  json<<",\n  \"late_inside_8_maximum\": ";
  horizon_json_number(json,arm.late_inside_8_maximum);
  json<<",\n  \"post_arrival_48_minimum\": ";
  horizon_json_number(json,arm.post_arrival_48_minimum);
  json<<",\n  \"first_tail_ticks\": [";
  for(std::size_t i=0;i<kHorizonRadii.size();++i)
    json<<(i==0?"":",")<<arm.first_tail_tick[i];
  json<<"],\n  \"maximum_outside\": [";
  for(std::size_t i=0;i<kHorizonRadii.size();++i) {
    if(i!=0) json<<',';
    horizon_json_number(json,arm.maximum_outside[i]);
  }
  json<<"],\n  \"final_outside\": [";
  for(std::size_t i=0;i<kHorizonRadii.size();++i) {
    if(i!=0) json<<',';
    horizon_json_number(json,arm.final_outside[i]);
  }
  json<<"],\n  \"minimum_outward_increment\": [";
  for(std::size_t i=0;i<kHorizonRadii.size();++i) {
    if(i!=0) json<<',';
    horizon_json_number(json,arm.minimum_outward_increment[i]);
  }
  json<<"]\n}\n";
}

bool select_horizon_direction(
    const std::string& slug,Direction& direction) {
  if(slug=="face") { direction=kDirections[0]; return true; }
  if(slug=="edge") { direction=kDirections[1]; return true; }
  if(slug=="body") { direction=kDirections[2]; return true; }
  return false;
}

}  // namespace

#ifndef FTD_CAUSAL_HORIZON_MAIN
#define FTD_CAUSAL_HORIZON_MAIN main
#endif

int FTD_CAUSAL_HORIZON_MAIN(int argc,char** argv) {
  if(argc==1) {
    std::cout<<"FTD-0746 smoke: invoke exactly once per face|edge|body after lock\n";
    return 0;
  }
  if(argc!=2) return 2;
  const std::string slug=argv[1];
  Direction direction;
  if(!select_horizon_direction(slug,direction)) return 2;

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
  const double interaction_scale=normalization.mapped_field_work_coefficient;
  auto arm=run_horizon_arm(slug,direction,options,interaction_scale);
  if(!normalization.valid) arm.exact_pass=false;
  const auto verdict=horizon_verdict(arm);
  write_horizon_records(arm,verdict);
  std::cout<<"FTD-0746 "<<slug<<' '<<verdict
           <<" prefix="<<std::setprecision(8)<<arm.prefix_scalar_difference
           <<" r48_tick="<<arm.first_tail_tick[5]<<'\n';
  return verdict=="CAUSAL_HORIZON_EXECUTION_INVALID"?1:0;
}
