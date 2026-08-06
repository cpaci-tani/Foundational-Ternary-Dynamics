/** FTD-0745: held-out environmental closure after FTD-0739 formation. */

#define main ftd_0731_source_of_record_main
#include "test_multipass_formation_persistence.cpp"
#undef main

#include "ftd/eft/batched_regional_energy_profile.h"
#include "ftd/eft/matched_regional_energy_transport.h"

#include <array>
#include <future>
#include <map>
#include <sstream>

namespace {

constexpr char kEnvironmentProtocolSha256[] =
    "D5FB9923FCBF69E2DFD75300FEE4C381AE28EAA10843BF0D52B2D60FCE456888";
constexpr char kBaselineCsvSha256[] =
    "E9B9B2FCE0FDA1350DBD6195AE039E99004141C86CB8A3F195ACE5CF24ADC622";
constexpr int kEnvironmentL = 193;
constexpr int kEnvironmentTicks = 184;
constexpr int kEnvironmentSupportRadius = 4;
constexpr int kEnvironmentContactTick = kEnvironmentL-2*kEnvironmentSupportRadius;
constexpr std::array<int,6> kEnvironmentRadii{8,12,16,24,32,48};
constexpr int kBaselineTicks = 136;
constexpr int kLateWindow = 32;
constexpr double kEnvironmentGate = 1e-10;
constexpr double kTailThreshold = 1e-8;
constexpr double kTailFinalThreshold = 1e-9;
constexpr double kLateNearMinimum = 5e-4;
constexpr double kLateNearDynamicRange = 4.0;

int environment_periodic_abs(int value, int center, int L) {
  const int direct=std::abs(value-center);
  return std::min(direct,L-direct);
}

int environment_source_radius(
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
        environment_periodic_abs(entry.face.x,cx,segment.L),
        environment_periodic_abs(entry.face.y,cy,segment.L),
        environment_periodic_abs(entry.face.z,cz,segment.L)});
    maximum=std::max(maximum,radius);
  }
  return maximum;
}

ftd::eft::MatchedFaceFlux environment_pre_current_field(
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

struct EnvironmentStepRecord {
  std::string family, direction, polarity, phase;
  int tick = 0;
  bool valid = false, common = false, regional_valid = false;
  double maximum_residual = INFINITY, total_energy_residual = INFINITY;
  double recoil_defect = INFINITY, speed_excess = INFINITY;
  double regional_residual = INFINITY, outside_source_residual = INFINITY;
  int source_radius = 0, source_entries = 0;
  double separation = INFINITY, pair_energy = INFINITY;
  double field_energy = INFINITY;
  bool graph_inside = false;
  std::array<double,6> inside{}, outside{}, transport_into{};
  std::array<double,6> source_exchange{}, cumulative_outward{};
};

struct EnvironmentArm {
  std::string family, direction, polarity;
  bool initialized = false, preparation_pass = false;
  bool forward_executed = false, reverse_executed = false;
  bool exact_pass = false, inverse_pass = false, support_pass = false;
  bool initial_pass = false, core_pass = false, near_field_pass = false;
  bool arrival_pass = false, no_return_pass = false;
  bool bound_control_pass = false;
  int energetic_onset_tick = -1;
  std::vector<int> transition_ticks;
  std::array<int,6> first_tail_tick{-1,-1,-1,-1,-1,-1};
  std::array<double,6> maximum_outside{};
  std::array<double,6> final_outside{};
  std::array<double,6> minimum_outward_increment{};
  int maximum_source_radius = 0;
  double maximum_common_residual = 0.0;
  double maximum_energy_residual = 0.0;
  double maximum_recoil_defect = 0.0;
  double maximum_speed_excess = 0.0;
  double maximum_regional_residual = 0.0;
  double maximum_outside_source = 0.0;
  double pair_field_balance = INFINITY, inverse_recovery = INFINITY;
  double late_inside_8_minimum = INFINITY;
  double late_inside_8_maximum = 0.0;
  std::vector<EnvironmentStepRecord> rows;
};

double environment_regional_residual(
    const ftd::eft::MatchedRegionalEnergyTransportResult& value) {
  return std::max({value.magnetic_update_residual,
      value.electric_pre_update_residual,value.global_source_free_residual,
      value.partition_residual,value.regional_ledger_residual});
}

EnvironmentStepRecord make_environment_record(
    const std::string& family, const Direction& direction, bool conjugate,
    int tick, const ftd::eft::ConnectedMooreBlockStepResult& step,
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& options, double interaction_scale,
    const Vec3& center, std::array<double,6>& cumulative_outward) {
  EnvironmentStepRecord row;
  row.family=family; row.direction=direction.label;
  row.polarity=conjugate?"minus_plus":"plus_minus";
  row.phase="forward"; row.tick=tick; row.valid=step.valid;
  row.common=step.common_action_gates_pass;
  row.maximum_residual=maximum_step_residual(step);
  row.total_energy_residual=step.total_energy_residual;
  row.recoil_defect=std::max({step.matter_momentum_before.mag(),
      step.matter_momentum_after.mag(),step.spline_defect_norm});
  row.speed_excess=step.causal_speed_excess;
  row.source_radius=environment_source_radius(
      step.segments,center,row.source_entries);
  row.separation=pair_separation(state);
  row.pair_energy=pair_internal_energy(state,options);
  row.field_energy=field_energy(state,options,interaction_scale);
  row.graph_inside=graph_inside(row.separation,options);

  const double lambda=options.wave_speed*options.dt;
  const auto pre=environment_pre_current_field(step,lambda);
  const std::vector<int> radii(kEnvironmentRadii.begin(),kEnvironmentRadii.end());
  const auto profile=ftd::eft::evaluate_batched_regional_energy_profile(
      step.earlier.electric,step.earlier.magnetic_half,pre,
      step.later.magnetic_half,step.later.electric,lambda,center,radii,
      kEnvironmentGate);
  row.regional_valid=profile.valid&&profile.regions.size()==radii.size();
  const double total_after=ftd::eft::matched_modified_energy(
      step.later.electric,step.later.magnetic_half,lambda);
  const double total_pre=ftd::eft::matched_modified_energy(
      pre,step.later.magnetic_half,lambda);
  const double total_source=total_after-total_pre;
  row.regional_residual=profile.maximum_scalar_equivalence_residual;
  row.outside_source_residual=0.0;
  if(!row.regional_valid) return row;
  for(std::size_t i=0;i<radii.size();++i) {
    const auto& region=profile.regions[i];
    row.regional_residual=std::max(
        row.regional_residual,environment_regional_residual(region));
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

EnvironmentStepRecord make_environment_initial(
    const std::string& family, const Direction& direction, bool conjugate,
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& options, double interaction_scale,
    const Vec3& center) {
  EnvironmentStepRecord row;
  row.family=family; row.direction=direction.label;
  row.polarity=conjugate?"minus_plus":"plus_minus";
  row.phase="forward"; row.valid=row.common=true; row.regional_valid=true;
  row.maximum_residual=row.total_energy_residual=0.0;
  row.recoil_defect=row.speed_excess=row.outside_source_residual=0.0;
  row.separation=pair_separation(state);
  row.pair_energy=pair_internal_energy(state,options);
  row.field_energy=field_energy(state,options,interaction_scale);
  row.graph_inside=graph_inside(row.separation,options);
  const double lambda=options.wave_speed*options.dt;
  row.regional_residual=0.0;
  for(std::size_t i=0;i<kEnvironmentRadii.size();++i) {
    const auto snapshot=ftd::eft::measure_matched_regional_energy(
        state.electric,state.magnetic_half,lambda,center,
        kEnvironmentRadii[i],kEnvironmentGate);
    row.regional_valid=row.regional_valid&&snapshot.valid;
    row.regional_residual=std::max(
        row.regional_residual,snapshot.partition_residual);
    row.inside[i]=interaction_scale*snapshot.inside_energy;
    row.outside[i]=interaction_scale*snapshot.outside_energy;
  }
  return row;
}

int environment_negative_onset(const EnvironmentArm& arm,
                               const ConnectedMooreBlockOptions& options) {
  for(int tick=0;tick<=kEnvironmentTicks;++tick) {
    bool tail=true;
    for(int later=tick;later<=kEnvironmentTicks;++later) {
      const auto& row=arm.rows[static_cast<std::size_t>(later)];
      tail=tail&&row.pair_energy<-1e-6&&graph_inside(row.separation,options);
      if(!tail) break;
    }
    if(tail) return tick;
  }
  return -1;
}

EnvironmentArm run_environment_arm(
    const std::string& family, const Direction& direction, bool conjugate,
    const ConnectedMooreBlockOptions& options, double interaction_scale) {
  EnvironmentArm arm;
  arm.family=family; arm.direction=direction.label;
  arm.polarity=conjugate?"minus_plus":"plus_minus";
  arm.minimum_outward_increment.fill(INFINITY);
  const bool unbound=family=="unbound";
  const double separation=unbound?1.30:1.00;
  const double momentum=unbound?0.0120:kBoundMomentum;
  const Vec3 center{static_cast<double>(kEnvironmentL/2),
                    static_cast<double>(kEnvironmentL/2),
                    static_cast<double>(kEnvironmentL/2)};
  const auto prep=ftd::eft::prepare_finite_support_derived_compact_pair(
      make_geometry(kEnvironmentL,direction,conjugate,separation,momentum),
      options,kEnvironmentSupportRadius,1e-13,4096);
  arm.initialized=prep.valid;
  arm.preparation_pass=prep.valid&&prep.density_contained&&prep.compact_support
      &&prep.zero_boundary_crossing&&prep.poisson_residual<=1e-13
      &&prep.gauss_residual<=1e-12&&prep.outside_maximum==0.0
      &&prep.boundary_crossing_maximum==0.0;
  if(!arm.preparation_pass) return arm;

  ConnectedMooreBlockState state=prep.state;
  const ConnectedMooreBlockState original=state;
  auto initial=make_environment_initial(
      family,direction,conjugate,state,options,interaction_scale,center);
  arm.initial_pass=unbound
      ?(!initial.graph_inside&&initial.pair_energy>1e-6
        &&initial.outside[1]<=1e-12)
      :(initial.graph_inside&&initial.pair_energy<-1e-6);
  arm.rows.push_back(std::move(initial));

  bool valid=true,exact=true;
  std::array<double,6> cumulative_outward{};
  ConnectedMooreBlockSolveCache forward_cache;
  for(int tick=1;tick<=kEnvironmentTicks;++tick) {
    const auto step=ftd::eft::solve_connected_moore_block_forward(
        state,options,&forward_cache);
    valid=valid&&step.valid;
    if(!step.valid) break;
    state=step.later;
    auto row=make_environment_record(family,direction,conjugate,tick,step,
        state,options,interaction_scale,center,cumulative_outward);
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
        &&row.maximum_residual<=kEnvironmentGate
        &&row.total_energy_residual<=1e-8
        &&row.recoil_defect<=1e-9&&row.speed_excess<=1e-12
        &&row.regional_residual<=kEnvironmentGate;
    if(step.relational_graph_changed) arm.transition_ticks.push_back(tick);
    for(std::size_t i=0;i<kEnvironmentRadii.size();++i) {
      arm.maximum_outside[i]=std::max(arm.maximum_outside[i],row.outside[i]);
      if(arm.first_tail_tick[i]<0&&row.outside[i]>kTailThreshold)
        arm.first_tail_tick[i]=tick;
      if(arm.first_tail_tick[i]>=0)
        arm.minimum_outward_increment[i]=std::min(
            arm.minimum_outward_increment[i],-row.transport_into[i]);
    }
    arm.rows.push_back(std::move(row));
  }
  arm.forward_executed=valid
      &&arm.rows.size()==static_cast<std::size_t>(kEnvironmentTicks+1);
  if(!arm.forward_executed) return arm;

  arm.energetic_onset_tick=environment_negative_onset(arm,options);
  if(unbound) arm.core_pass=arm.initial_pass&&arm.energetic_onset_tick>=0
      &&kEnvironmentTicks-arm.energetic_onset_tick+1>=64;
  else {
    arm.bound_control_pass=arm.initial_pass&&arm.transition_ticks.empty();
    for(const auto& row:arm.rows)
      arm.bound_control_pass=arm.bound_control_pass&&row.graph_inside
          &&row.pair_energy<-1e-6;
  }
  for(std::size_t i=0;i<kEnvironmentRadii.size();++i)
    arm.final_outside[i]=arm.rows.back().outside[i];

  if(unbound) {
    const int begin=kEnvironmentTicks-kLateWindow+1;
    for(int tick=begin;tick<=kEnvironmentTicks;++tick) {
      const double inside8=arm.rows[static_cast<std::size_t>(tick)].inside[0];
      arm.late_inside_8_minimum=std::min(arm.late_inside_8_minimum,inside8);
      arm.late_inside_8_maximum=std::max(arm.late_inside_8_maximum,inside8);
    }
    arm.near_field_pass=arm.late_inside_8_minimum>=kLateNearMinimum
        &&arm.late_inside_8_maximum
            <=kLateNearDynamicRange*arm.late_inside_8_minimum;
    arm.arrival_pass=arm.maximum_outside_source<=kEnvironmentGate;
    arm.no_return_pass=true;
    for(std::size_t i=1;i<kEnvironmentRadii.size();++i) {
      arm.arrival_pass=arm.arrival_pass&&arm.rows.front().outside[i]<=1e-12
          &&arm.first_tail_tick[i]>=0
          &&arm.maximum_outside[i]>kTailThreshold
          &&arm.final_outside[i]>kTailFinalThreshold;
      if(i>1) arm.arrival_pass=arm.arrival_pass
          &&arm.first_tail_tick[i]>=arm.first_tail_tick[i-1];
      arm.no_return_pass=arm.no_return_pass
          &&arm.minimum_outward_increment[i]>=-kEnvironmentGate;
    }
  }

  arm.pair_field_balance=std::abs(
      arm.rows.back().pair_energy-arm.rows.front().pair_energy
      +arm.rows.back().field_energy-arm.rows.front().field_energy);
  ConnectedMooreBlockState recovered=state;
  bool reverse_valid=true;
  ConnectedMooreBlockSolveCache reverse_cache;
  for(int tick=1;tick<=kEnvironmentTicks;++tick) {
    const auto step=ftd::eft::solve_connected_moore_block_reverse(
        recovered,options,&reverse_cache);
    reverse_valid=reverse_valid&&step.valid;
    if(!step.valid) break;
    recovered=step.earlier;
    const double residual=maximum_step_residual(step);
    const double recoil=std::max({step.matter_momentum_before.mag(),
        step.matter_momentum_after.mag(),step.spline_defect_norm});
    arm.maximum_common_residual=std::max(arm.maximum_common_residual,residual);
    arm.maximum_energy_residual=std::max(
        arm.maximum_energy_residual,step.total_energy_residual);
    arm.maximum_recoil_defect=std::max(arm.maximum_recoil_defect,recoil);
    arm.maximum_speed_excess=std::max(
        arm.maximum_speed_excess,step.causal_speed_excess);
    exact=exact&&step.common_action_gates_pass&&residual<=kEnvironmentGate
        &&step.total_energy_residual<=1e-8&&recoil<=1e-9
        &&step.causal_speed_excess<=1e-12;
  }
  arm.reverse_executed=reverse_valid;
  arm.inverse_recovery=arm.reverse_executed
      ?ftd::eft::connected_moore_block_state_max_difference(original,recovered)
      :INFINITY;
  arm.exact_pass=exact&&arm.pair_field_balance<=1e-8;
  arm.inverse_pass=arm.inverse_recovery<=1e-8;
  arm.support_pass=arm.maximum_source_radius<=3
      &&kEnvironmentTicks<kEnvironmentContactTick;
  return arm;
}

std::vector<std::string> split_csv(const std::string& line) {
  std::vector<std::string> fields;
  std::stringstream stream(line);
  std::string field;
  while(std::getline(stream,field,',')) fields.push_back(field);
  return fields;
}

struct BaselineRow {
  int source_radius=0,source_entries=0;
  bool valid=false,common=false,regional_valid=false,graph_inside=false;
  std::array<double,17> scalar{};
};

using BaselineMap=std::map<std::string,BaselineRow>;

std::string row_key(const std::string& family,const std::string& direction,
                    const std::string& polarity,int tick) {
  return family+"|"+direction+"|"+polarity+"|"+std::to_string(tick);
}

BaselineMap load_baseline(bool& valid) {
  valid=false;
  const auto path=std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results"/"ftd_0739"
      /"ftd_0739_finite_support_outgoing_tail_formation_v1.csv";
  std::ifstream input(path);
  std::string line;
  if(!std::getline(input,line)) return {};
  const auto header=split_csv(line);
  std::map<std::string,std::size_t> column;
  for(std::size_t i=0;i<header.size();++i) column[header[i]]=i;
  const std::array<std::string,28> required{
      "family","direction","polarity","phase","tick","source_radius",
      "source_entries","valid","common","regional_valid","graph_inside",
      "max_residual","total_energy_residual","recoil_defect","speed_excess",
      "regional_residual","separation","pair_energy","field_energy",
      "inside_energy_8","outside_energy_8","boundary_transport_into_8",
      "source_exchange_8","inside_energy_12","outside_energy_12",
      "boundary_transport_into_12","source_exchange_12",
      "cumulative_outward_12"};
  for(const auto& name:required) if(!column.count(name)) return {};
  BaselineMap result;
  while(std::getline(input,line)) {
    const auto value=split_csv(line);
    if(value.size()!=header.size()||value[column["phase"]]!="forward") continue;
    BaselineRow row;
    row.source_radius=std::stoi(value[column["source_radius"]]);
    row.source_entries=std::stoi(value[column["source_entries"]]);
    row.valid=std::stoi(value[column["valid"]])!=0;
    row.common=std::stoi(value[column["common"]])!=0;
    row.regional_valid=std::stoi(value[column["regional_valid"]])!=0;
    row.graph_inside=std::stoi(value[column["graph_inside"]])!=0;
    const std::array<std::string,17> names{
        "max_residual","total_energy_residual","recoil_defect","speed_excess",
        "regional_residual","separation","pair_energy","field_energy",
        "inside_energy_8","outside_energy_8","boundary_transport_into_8",
        "source_exchange_8","inside_energy_12","outside_energy_12",
        "boundary_transport_into_12","source_exchange_12",
        "cumulative_outward_12"};
    for(std::size_t i=0;i<names.size();++i)
      row.scalar[i]=std::stod(value[column[names[i]]]);
    const auto key=row_key(value[column["family"]],value[column["direction"]],
        value[column["polarity"]],std::stoi(value[column["tick"]]));
    result.emplace(key,row);
  }
  valid=result.size()==5*static_cast<std::size_t>(kBaselineTicks+1);
  return result;
}

double baseline_prefix_difference(const std::vector<EnvironmentArm>& arms,
                                  const BaselineMap& baseline,
                                  bool& discrete_pass) {
  discrete_pass=true;
  double maximum=0.0;
  for(const auto& arm:arms) for(int tick=0;tick<=kBaselineTicks;++tick) {
    const auto found=baseline.find(row_key(
        arm.family,arm.direction,arm.polarity,tick));
    if(found==baseline.end()) { discrete_pass=false; return INFINITY; }
    const auto& row=arm.rows[static_cast<std::size_t>(tick)];
    const auto& old=found->second;
    discrete_pass=discrete_pass&&row.source_radius==old.source_radius
        &&row.source_entries==old.source_entries
        &&row.valid==old.valid&&row.common==old.common
        &&row.regional_valid==old.regional_valid
        &&row.graph_inside==old.graph_inside;
    const std::array<double,17> now{
        row.maximum_residual,row.total_energy_residual,row.recoil_defect,
        row.speed_excess,row.regional_residual,row.separation,row.pair_energy,
        row.field_energy,row.inside[0],row.outside[0],row.transport_into[0],
        row.source_exchange[0],row.inside[1],row.outside[1],
        row.transport_into[1],row.source_exchange[1],row.cumulative_outward[1]};
    for(std::size_t i=0;i<now.size();++i)
      maximum=std::max(maximum,std::abs(now[i]-old.scalar[i]));
  }
  return maximum;
}

double environment_polarity_difference(const EnvironmentArm& first,
                                       const EnvironmentArm& second) {
  if(first.rows.size()!=second.rows.size()) return INFINITY;
  double maximum=0.0;
  for(std::size_t i=0;i<first.rows.size();++i) {
    const auto& a=first.rows[i]; const auto& b=second.rows[i];
    if(a.family!=b.family||a.direction!=b.direction||a.phase!=b.phase
        ||a.tick!=b.tick||a.valid!=b.valid||a.common!=b.common
        ||a.regional_valid!=b.regional_valid
        ||a.source_radius!=b.source_radius
        ||a.source_entries!=b.source_entries||a.graph_inside!=b.graph_inside)
      return INFINITY;
    for(const auto& pair:{std::pair{a.separation,b.separation},
        std::pair{a.pair_energy,b.pair_energy},
        std::pair{a.field_energy,b.field_energy},
        std::pair{a.maximum_residual,b.maximum_residual},
        std::pair{a.total_energy_residual,b.total_energy_residual},
        std::pair{a.recoil_defect,b.recoil_defect},
        std::pair{a.speed_excess,b.speed_excess},
        std::pair{a.regional_residual,b.regional_residual},
        std::pair{a.outside_source_residual,b.outside_source_residual}})
      maximum=std::max(maximum,std::abs(pair.first-pair.second));
    for(std::size_t j=0;j<kEnvironmentRadii.size();++j) {
      maximum=std::max(maximum,std::abs(a.inside[j]-b.inside[j]));
      maximum=std::max(maximum,std::abs(a.outside[j]-b.outside[j]));
      maximum=std::max(maximum,
          std::abs(a.transport_into[j]-b.transport_into[j]));
      maximum=std::max(maximum,
          std::abs(a.source_exchange[j]-b.source_exchange[j]));
      maximum=std::max(maximum,
          std::abs(a.cumulative_outward[j]-b.cumulative_outward[j]));
    }
  }
  return maximum;
}

const EnvironmentArm* find_environment_arm(
    const std::vector<EnvironmentArm>& arms,const std::string& family,
    const std::string& direction,const std::string& polarity) {
  const auto found=std::find_if(arms.begin(),arms.end(),[&](const auto& arm) {
    return arm.family==family&&arm.direction==direction&&arm.polarity==polarity;
  });
  return found==arms.end()?nullptr:&*found;
}

void write_environment_records(const std::vector<EnvironmentArm>& arms,
                               const std::string& verdict,
                               double prefix_difference,
                               bool prefix_discrete_pass,
                               double polarity_difference) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0745";
  std::filesystem::create_directories(directory);
  std::ofstream csv(directory/
      "ftd_0745_finite_support_environmental_closure_v1.csv");
  csv<<"family,direction,polarity,phase,tick,valid,common,regional_valid,"
        "max_residual,total_energy_residual,recoil_defect,speed_excess,"
        "regional_residual,outside_source_residual,source_radius,source_entries,"
        "separation,pair_energy,field_energy,graph_inside";
  for(int radius:kEnvironmentRadii)
    csv<<",inside_"<<radius<<",outside_"<<radius<<",transport_into_"<<radius
       <<",source_exchange_"<<radius<<",cumulative_outward_"<<radius;
  csv<<'\n'<<std::setprecision(17);
  for(const auto& arm:arms) for(const auto& row:arm.rows) {
    csv<<row.family<<','<<row.direction<<','<<row.polarity<<','<<row.phase<<','
       <<row.tick<<','<<row.valid<<','<<row.common<<','<<row.regional_valid<<','
       <<row.maximum_residual<<','<<row.total_energy_residual<<','
       <<row.recoil_defect<<','<<row.speed_excess<<','<<row.regional_residual<<','
       <<row.outside_source_residual<<','<<row.source_radius<<','
       <<row.source_entries<<','<<row.separation<<','<<row.pair_energy<<','
       <<row.field_energy<<','<<row.graph_inside;
    for(std::size_t i=0;i<kEnvironmentRadii.size();++i)
      csv<<','<<row.inside[i]<<','<<row.outside[i]<<','<<row.transport_into[i]
         <<','<<row.source_exchange[i]<<','<<row.cumulative_outward[i];
    csv<<'\n';
  }
  std::ofstream json(directory/
      "ftd_0745_finite_support_environmental_closure_v1.json");
  json<<std::setprecision(17)<<"{\n"
      <<"  \"ftd_id\": \"FTD-0745\",\n"
      <<"  \"protocol_sha256\": \""<<kEnvironmentProtocolSha256<<"\",\n"
      <<"  \"baseline_csv_sha256\": \""<<kBaselineCsvSha256<<"\",\n"
      <<"  \"verdict\": \""<<verdict<<"\",\n"
      <<"  \"volume\": "<<kEnvironmentL<<",\n"
      <<"  \"horizon\": "<<kEnvironmentTicks<<",\n"
      <<"  \"contact_tick\": "<<kEnvironmentContactTick<<",\n"
      <<"  \"radii\": [8,12,16,24,32,48],\n"
      <<"  \"tail_threshold\": "<<kTailThreshold<<",\n"
      <<"  \"tail_final_threshold\": "<<kTailFinalThreshold<<",\n"
      <<"  \"late_near_minimum\": "<<kLateNearMinimum<<",\n"
      <<"  \"late_near_dynamic_range\": "<<kLateNearDynamicRange<<",\n"
      <<"  \"history_count\": "<<arms.size()<<",\n"
      <<"  \"prefix_scalar_difference\": "<<prefix_difference<<",\n"
      <<"  \"prefix_discrete_pass\": "<<prefix_discrete_pass<<",\n"
      <<"  \"polarity_scalar_difference\": "<<polarity_difference<<",\n"
      <<"  \"arms\": [\n";
  for(std::size_t i=0;i<arms.size();++i) {
    const auto& arm=arms[i];
    json<<"    {\"family\": \""<<arm.family<<"\", \"direction\": \""
        <<arm.direction<<"\", \"polarity\": \""<<arm.polarity
        <<"\", \"preparation_pass\": "<<arm.preparation_pass
        <<", \"forward_executed\": "<<arm.forward_executed
        <<", \"reverse_executed\": "<<arm.reverse_executed
        <<", \"exact_pass\": "<<arm.exact_pass
        <<", \"inverse_pass\": "<<arm.inverse_pass
        <<", \"support_pass\": "<<arm.support_pass
        <<", \"core_pass\": "<<arm.core_pass
        <<", \"near_field_pass\": "<<arm.near_field_pass
        <<", \"arrival_pass\": "<<arm.arrival_pass
        <<", \"no_return_pass\": "<<arm.no_return_pass
        <<", \"bound_control_pass\": "<<arm.bound_control_pass
        <<", \"energetic_onset_tick\": "<<arm.energetic_onset_tick
        <<", \"late_inside_8_minimum\": "<<arm.late_inside_8_minimum
        <<", \"late_inside_8_maximum\": "<<arm.late_inside_8_maximum
        <<", \"maximum_source_radius\": "<<arm.maximum_source_radius
        <<", \"maximum_regional_residual\": "
        <<arm.maximum_regional_residual
        <<", \"maximum_outside_source\": "<<arm.maximum_outside_source
        <<", \"pair_field_balance\": "<<arm.pair_field_balance
        <<", \"inverse_recovery\": "<<arm.inverse_recovery
        <<", \"first_tail_ticks\": [";
    for(std::size_t j=0;j<kEnvironmentRadii.size();++j)
      json<<(j==0?"":",")<<arm.first_tail_tick[j];
    json<<"], \"maximum_outside\": [";
    for(std::size_t j=0;j<kEnvironmentRadii.size();++j)
      json<<(j==0?"":",")<<arm.maximum_outside[j];
    json<<"], \"final_outside\": [";
    for(std::size_t j=0;j<kEnvironmentRadii.size();++j)
      json<<(j==0?"":",")<<arm.final_outside[j];
    json<<"], \"minimum_outward_increment\": [";
    for(std::size_t j=0;j<kEnvironmentRadii.size();++j) {
      json<<(j==0?"":",");
      if(std::isfinite(arm.minimum_outward_increment[j]))
        json<<arm.minimum_outward_increment[j];
      else
        json<<"null";
    }
    json<<"]}"<<(i+1==arms.size()?"\n":",\n");
  }
  json<<"  ]\n}\n";
}

}  // namespace

int main() {
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
  const Direction face=kDirections[0],edge=kDirections[1],body=kDirections[2];
  auto face_future=std::async(std::launch::async,[=]() {
    std::vector<EnvironmentArm> result;
    result.push_back(run_environment_arm("unbound",face,false,options,interaction_scale));
    result.push_back(run_environment_arm("bound",face,false,options,interaction_scale));
    return result;
  });
  auto edge_future=std::async(std::launch::async,[=]() {
    return std::vector<EnvironmentArm>{
        run_environment_arm("unbound",edge,false,options,interaction_scale)};
  });
  auto body_future=std::async(std::launch::async,[=]() {
    std::vector<EnvironmentArm> result;
    result.push_back(run_environment_arm("unbound",body,false,options,interaction_scale));
    result.push_back(run_environment_arm("unbound",body,true,options,interaction_scale));
    return result;
  });
  std::vector<EnvironmentArm> arms;
  auto face_arms=face_future.get(),edge_arms=edge_future.get();
  auto body_arms=body_future.get();
  for(auto* group:{&face_arms,&edge_arms,&body_arms})
    for(auto& arm:*group) arms.push_back(std::move(arm));
  std::sort(arms.begin(),arms.end(),[](const auto& a,const auto& b) {
    return std::tie(a.family,a.direction,a.polarity)
        <std::tie(b.family,b.direction,b.polarity);
  });

  bool baseline_valid=false,prefix_discrete_pass=false;
  const auto baseline=load_baseline(baseline_valid);
  const double prefix_difference=baseline_valid
      ?baseline_prefix_difference(arms,baseline,prefix_discrete_pass):INFINITY;
  const bool prefix_pass=baseline_valid&&prefix_discrete_pass
      &&prefix_difference<=kEnvironmentGate;
  const auto plus_body=find_environment_arm(
      arms,"unbound","1_1_1","plus_minus");
  const auto minus_body=find_environment_arm(
      arms,"unbound","1_1_1","minus_plus");
  const double polarity_difference=plus_body&&minus_body
      ?environment_polarity_difference(*plus_body,*minus_body):INFINITY;
  const bool polarity_pass=plus_body&&minus_body
      &&plus_body->transition_ticks==minus_body->transition_ticks
      &&plus_body->first_tail_tick==minus_body->first_tail_tick
      &&polarity_difference<=1e-9;
  const bool matrix=normalization.valid&&arms.size()==5;
  const bool infrastructure=matrix&&std::all_of(
      arms.begin(),arms.end(),[](const auto& arm) {
        return arm.initialized&&arm.preparation_pass&&arm.forward_executed
            &&arm.reverse_executed&&arm.exact_pass&&arm.inverse_pass
            &&arm.support_pass&&arm.initial_pass;
      });
  const bool control=std::all_of(arms.begin(),arms.end(),[](const auto& arm) {
    return arm.family!="bound"||arm.bound_control_pass;
  });
  const bool cores=std::all_of(arms.begin(),arms.end(),[](const auto& arm) {
    return arm.family!="unbound"||arm.core_pass;
  });
  const bool near=std::all_of(arms.begin(),arms.end(),[](const auto& arm) {
    return arm.family!="unbound"||arm.near_field_pass;
  });
  const bool arrivals=std::all_of(arms.begin(),arms.end(),[](const auto& arm) {
    return arm.family!="unbound"||arm.arrival_pass;
  });
  const bool no_return=std::all_of(arms.begin(),arms.end(),[](const auto& arm) {
    return arm.family!="unbound"||arm.no_return_pass;
  });

  std::string verdict;
  if(!infrastructure) verdict="ENVIRONMENTAL_CLOSURE_EXECUTION_INVALID";
  else if(!prefix_pass) verdict="ENVIRONMENTAL_CLOSURE_CAUSAL_PREFIX_DRIFT";
  else if(!control) verdict="ENVIRONMENTAL_CLOSURE_BOUND_CONTROL_UNSTABLE";
  else if(!polarity_pass) verdict="ENVIRONMENTAL_CLOSURE_POLARITY_SENSITIVE";
  else if(!cores) verdict="ENVIRONMENTAL_CLOSURE_CORE_NOT_PERSISTENT";
  else if(!near) verdict="ENVIRONMENTAL_CLOSURE_NEAR_FIELD_NOT_STABLE";
  else if(!arrivals) verdict="ENVIRONMENTAL_CLOSURE_ARRIVAL_LAW_FAIL";
  else if(!no_return) verdict="ENVIRONMENTAL_CLOSURE_OUTGOING_COMPONENT_RETURNS";
  else verdict="FINITE_LADDER_ENVIRONMENTAL_CLOSURE_CONSTRUCTIVE";
  write_environment_records(arms,verdict,prefix_difference,
      prefix_discrete_pass,polarity_difference);
  std::cout<<"FTD-0745 "<<verdict
           <<" prefix="<<std::setprecision(8)<<prefix_difference
           <<" polarity="<<polarity_difference<<'\n';
  return infrastructure?0:1;
}
