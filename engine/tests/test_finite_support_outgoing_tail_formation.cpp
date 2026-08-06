/** FTD-0739: finite-support outgoing-tail matter formation. */

#define main ftd_0731_source_of_record_main
#include "test_multipass_formation_persistence.cpp"
#undef main

#include "ftd/eft/matched_regional_energy_transport.h"

#include <future>
#include <utility>

namespace {

constexpr char kFiniteProtocolSha256[] =
    "9AA9B806877F07F9567291E73B58E6157CFBDAE425DE843B85D3753CECA7868E";
constexpr int kFiniteL = 145;
constexpr int kFiniteTicks = 136;
constexpr int kFiniteSupportRadius = 4;
constexpr int kFiniteContactTick = kFiniteL-2*kFiniteSupportRadius;
constexpr double kInnerShell = 8.0;
constexpr double kOuterShell = 12.0;
constexpr double kFiniteGate = 1e-10;

int finite_periodic_abs(int value, int center, int L) {
  const int direct=std::abs(value-center);
  return std::min(direct,L-direct);
}

int current_source_radius(
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
        finite_periodic_abs(entry.face.x,cx,segment.L),
        finite_periodic_abs(entry.face.y,cy,segment.L),
        finite_periodic_abs(entry.face.z,cz,segment.L)});
    maximum=std::max(maximum,radius);
  }
  return maximum;
}

struct FiniteStepRecord {
  std::string family, direction, polarity, phase;
  int tick = 0;
  bool valid = false, common = false, regional_valid = false;
  double maximum_residual = INFINITY, total_energy_residual = INFINITY;
  double recoil_defect = INFINITY, speed_excess = INFINITY;
  double regional_residual = INFINITY;
  int source_radius = 0, source_entries = 0;
  double separation = INFINITY, pair_energy = INFINITY;
  double field_energy = INFINITY;
  bool graph_inside = false;
  double inside_energy_8 = 0.0, outside_energy_8 = 0.0;
  double boundary_transport_into_8 = 0.0, source_exchange_8 = 0.0;
  double inside_energy_12 = 0.0, outside_energy_12 = 0.0;
  double boundary_transport_into_12 = 0.0, source_exchange_12 = 0.0;
  double cumulative_outward_12 = 0.0;
};

struct FiniteArm {
  std::string family, direction, polarity;
  bool initialized = false, preparation_pass = false;
  bool forward_executed = false, reverse_executed = false;
  bool identity_pass = false, regional_pass = false;
  bool recoil_pass = false, speed_pass = false, inverse_pass = false;
  bool support_pass = false, initial_pass = false, core_pass = false;
  bool first_passage_pass = false, tail_pass = false;
  bool bound_control_pass = false;
  int maximum_source_radius = 0, source_entries = 0;
  int energetic_onset_tick = -1, final_entry_tick = -1;
  int predicted_onset_tick = -1, first_tail_tick = -1;
  std::vector<int> transition_ticks;
  std::vector<double> separation_history, pair_history, field_history;
  std::vector<double> outside12_history;
  double preparation_poisson_residual = INFINITY;
  double preparation_gauss_residual = INFINITY;
  double preparation_outside_maximum = INFINITY;
  double preparation_boundary_maximum = INFINITY;
  double preparation_curl_adjoint = INFINITY;
  double maximum_common_residual = 0.0, maximum_energy_residual = 0.0;
  double maximum_recoil_defect = 0.0, maximum_speed_excess = 0.0;
  double maximum_regional_residual = 0.0, maximum_outside_12 = 0.0;
  double final_outside_12 = 0.0, maximum_cumulative_outward_12 = 0.0;
  double maximum_first_passage_residual = 0.0;
  double pair_field_balance = INFINITY, inverse_recovery = INFINITY;
  std::vector<FiniteStepRecord> rows;
};

ftd::eft::MatchedFaceFlux pre_current_field(
    const ftd::eft::ConnectedMooreBlockStepResult& step, double lambda) {
  auto result = step.earlier.electric;
  const auto curl = ftd::eft::matched_curl(step.later.magnetic_half);
  for (std::size_t i = 0; i < result.x.size(); ++i) {
    result.x[i] += lambda*curl.x[i];
    result.y[i] += lambda*curl.y[i];
    result.z[i] += lambda*curl.z[i];
  }
  return result;
}

double regional_residual(
    const ftd::eft::MatchedRegionalEnergyTransportResult& value) {
  return std::max({value.magnetic_update_residual,
      value.electric_pre_update_residual,value.global_source_free_residual,
      value.partition_residual,value.regional_ledger_residual});
}

FiniteStepRecord make_finite_record(
    const std::string& family, const Direction& direction, bool conjugate,
    const std::string& phase, int tick,
    const ftd::eft::ConnectedMooreBlockStepResult& step,
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& options, double interaction_scale,
    const Vec3& center, double* cumulative_outward) {
  FiniteStepRecord row;
  row.family = family; row.direction = direction.label;
  row.polarity = conjugate ? "minus_plus" : "plus_minus";
  row.phase = phase; row.tick = tick; row.valid = step.valid;
  row.common = step.common_action_gates_pass;
  row.maximum_residual = maximum_step_residual(step);
  row.total_energy_residual = step.total_energy_residual;
  row.recoil_defect = std::max({step.matter_momentum_before.mag(),
      step.matter_momentum_after.mag(),step.spline_defect_norm});
  row.speed_excess = step.causal_speed_excess;
  row.source_radius = current_source_radius(
      step.segments,center,row.source_entries);
  row.separation = pair_separation(state);
  row.pair_energy = pair_internal_energy(state,options);
  row.field_energy = field_energy(state,options,interaction_scale);
  row.graph_inside = graph_inside(row.separation,options);
  if (phase != "forward") { row.regional_residual = 0.0; return row; }

  const double lambda = options.wave_speed*options.dt;
  const auto pre = pre_current_field(step,lambda);
  const auto r8 = ftd::eft::evaluate_matched_regional_energy_transport(
      step.earlier.electric,step.earlier.magnetic_half,pre,
      step.later.magnetic_half,step.later.electric,lambda,center,kInnerShell,
      kFiniteGate);
  const auto r12 = ftd::eft::evaluate_matched_regional_energy_transport(
      step.earlier.electric,step.earlier.magnetic_half,pre,
      step.later.magnetic_half,step.later.electric,lambda,center,kOuterShell,
      kFiniteGate);
  const auto s8 = ftd::eft::measure_matched_regional_energy(
      state.electric,state.magnetic_half,lambda,center,kInnerShell,kFiniteGate);
  const auto s12 = ftd::eft::measure_matched_regional_energy(
      state.electric,state.magnetic_half,lambda,center,kOuterShell,kFiniteGate);
  row.regional_valid = r8.valid && r12.valid && s8.valid && s12.valid;
  row.regional_residual = std::max({regional_residual(r8),regional_residual(r12),
      s8.partition_residual,s12.partition_residual});
  row.inside_energy_8 = interaction_scale*s8.inside_energy;
  row.outside_energy_8 = interaction_scale*s8.outside_energy;
  row.boundary_transport_into_8 = interaction_scale*r8.boundary_transport_into;
  row.source_exchange_8 = interaction_scale*r8.source_exchange_into_field;
  row.inside_energy_12 = interaction_scale*s12.inside_energy;
  row.outside_energy_12 = interaction_scale*s12.outside_energy;
  row.boundary_transport_into_12 = interaction_scale*r12.boundary_transport_into;
  row.source_exchange_12 = interaction_scale*r12.source_exchange_into_field;
  *cumulative_outward -= row.boundary_transport_into_12;
  row.cumulative_outward_12 = *cumulative_outward;
  return row;
}

int continuous_negative_onset(const FiniteArm& arm,
                              const ConnectedMooreBlockOptions& options) {
  for (int tick = 0; tick <= kFiniteTicks; ++tick) {
    bool tail = true;
    for (int later = tick; later <= kFiniteTicks; ++later) {
      const auto i = static_cast<std::size_t>(later);
      tail = tail && arm.pair_history[i] < -1e-6
          && graph_inside(arm.separation_history[i],options);
      if (!tail) break;
    }
    if (tail) return tick;
  }
  return -1;
}

FiniteArm run_finite_arm(
    const std::string& family, const Direction& direction, bool conjugate,
    const ConnectedMooreBlockOptions& options, double interaction_scale) {
  FiniteArm arm; arm.family = family; arm.direction = direction.label;
  arm.polarity = conjugate ? "minus_plus" : "plus_minus";
  const bool unbound = family == "unbound";
  const double separation = unbound ? 1.30 : 1.00;
  const double momentum = unbound ? 0.0120 : kBoundMomentum;
  const Vec3 center{static_cast<double>(kFiniteL/2),
                    static_cast<double>(kFiniteL/2),
                    static_cast<double>(kFiniteL/2)};
  const auto prep = ftd::eft::prepare_finite_support_derived_compact_pair(
      make_geometry(kFiniteL,direction,conjugate,separation,momentum),options,
      kFiniteSupportRadius,1e-13,4096);
  arm.initialized = prep.valid;
  arm.preparation_poisson_residual = prep.poisson_residual;
  arm.preparation_gauss_residual = prep.gauss_residual;
  arm.preparation_outside_maximum = prep.outside_maximum;
  arm.preparation_boundary_maximum = prep.boundary_crossing_maximum;
  arm.preparation_curl_adjoint = prep.curl_adjoint_residual;
  arm.preparation_pass = prep.valid && prep.density_contained
      && prep.compact_support && prep.zero_boundary_crossing
      && prep.poisson_residual <= 1e-13 && prep.gauss_residual <= 1e-12
      && prep.outside_maximum == 0.0 && prep.boundary_crossing_maximum == 0.0;
  if (!arm.preparation_pass) return arm;

  ConnectedMooreBlockState state = prep.state;
  const ConnectedMooreBlockState original = state;
  const double lambda = options.wave_speed*options.dt;
  const auto s8 = ftd::eft::measure_matched_regional_energy(
      state.electric,state.magnetic_half,lambda,center,kInnerShell,kFiniteGate);
  const auto s12 = ftd::eft::measure_matched_regional_energy(
      state.electric,state.magnetic_half,lambda,center,kOuterShell,kFiniteGate);
  FiniteStepRecord initial;
  initial.family=family; initial.direction=direction.label;
  initial.polarity=arm.polarity; initial.phase="forward"; initial.tick=0;
  initial.valid=true; initial.common=true;
  initial.regional_valid=s8.valid&&s12.valid;
  initial.maximum_residual=initial.total_energy_residual=0.0;
  initial.recoil_defect=initial.speed_excess=0.0;
  initial.regional_residual=std::max(s8.partition_residual,s12.partition_residual);
  initial.separation=pair_separation(state);
  initial.pair_energy=pair_internal_energy(state,options);
  initial.field_energy=field_energy(state,options,interaction_scale);
  initial.graph_inside=graph_inside(initial.separation,options);
  initial.inside_energy_8=interaction_scale*s8.inside_energy;
  initial.outside_energy_8=interaction_scale*s8.outside_energy;
  initial.inside_energy_12=interaction_scale*s12.inside_energy;
  initial.outside_energy_12=interaction_scale*s12.outside_energy;
  arm.rows.push_back(initial);
  arm.separation_history.push_back(initial.separation);
  arm.pair_history.push_back(initial.pair_energy);
  arm.field_history.push_back(initial.field_energy);
  arm.outside12_history.push_back(initial.outside_energy_12);
  arm.initial_pass = unbound
      ? (!initial.graph_inside && initial.pair_energy>1e-6
         && initial.outside_energy_12<=1e-12)
      : (initial.graph_inside && initial.pair_energy<-1e-6);

  bool common=true,regional=initial.regional_valid,recoil=true,speed=true;
  bool forward_valid=true; double cumulative_outward=0.0;
  ConnectedMooreBlockSolveCache forward_cache;
  for (int tick=1; tick<=kFiniteTicks; ++tick) {
    const auto step=ftd::eft::solve_connected_moore_block_forward(
        state,options,&forward_cache);
    forward_valid=forward_valid&&step.valid; if(!step.valid) break;
    state=step.later;
    auto row=make_finite_record(family,direction,conjugate,"forward",tick,
        step,state,options,interaction_scale,center,&cumulative_outward);
    arm.maximum_source_radius=std::max(arm.maximum_source_radius,row.source_radius);
    arm.source_entries+=row.source_entries;
    arm.maximum_common_residual=std::max(arm.maximum_common_residual,row.maximum_residual);
    arm.maximum_energy_residual=std::max(arm.maximum_energy_residual,row.total_energy_residual);
    arm.maximum_recoil_defect=std::max(arm.maximum_recoil_defect,row.recoil_defect);
    arm.maximum_speed_excess=std::max(arm.maximum_speed_excess,row.speed_excess);
    arm.maximum_regional_residual=std::max(arm.maximum_regional_residual,row.regional_residual);
    arm.maximum_outside_12=std::max(arm.maximum_outside_12,row.outside_energy_12);
    arm.maximum_cumulative_outward_12=std::max(
        arm.maximum_cumulative_outward_12,row.cumulative_outward_12);
    if(arm.first_tail_tick<0&&row.outside_energy_12>1e-6) arm.first_tail_tick=tick;
    common=common&&row.common&&row.maximum_residual<=kFiniteGate;
    regional=regional&&row.regional_valid&&row.regional_residual<=kFiniteGate;
    recoil=recoil&&row.recoil_defect<=1e-9; speed=speed&&row.speed_excess<=1e-12;
    if(step.relational_graph_changed) arm.transition_ticks.push_back(tick);
    arm.separation_history.push_back(row.separation);
    arm.pair_history.push_back(row.pair_energy);
    arm.field_history.push_back(row.field_energy);
    arm.outside12_history.push_back(row.outside_energy_12);
    arm.rows.push_back(std::move(row));
  }
  arm.forward_executed=forward_valid
      &&arm.pair_history.size()==static_cast<std::size_t>(kFiniteTicks+1);
  if(!arm.forward_executed) return arm;
  arm.final_outside_12=arm.outside12_history.back();
  arm.pair_field_balance=std::abs(arm.pair_history.back()-arm.pair_history.front()
      +arm.field_history.back()-arm.field_history.front());
  arm.energetic_onset_tick=continuous_negative_onset(arm,options);
  if(unbound&&arm.energetic_onset_tick>=0) {
    for(int transition:arm.transition_ticks)
      if(transition<=arm.energetic_onset_tick
          &&graph_inside(arm.separation_history[static_cast<std::size_t>(transition)],options))
        arm.final_entry_tick=transition;
    if(arm.final_entry_tick>=0) {
      const auto entry=static_cast<std::size_t>(arm.final_entry_tick);
      for(int tick=arm.final_entry_tick;tick<=kFiniteTicks;++tick) {
        const auto i=static_cast<std::size_t>(tick);
        const double predicted=arm.pair_history[entry]
            -(arm.field_history[i]-arm.field_history[entry]);
        arm.maximum_first_passage_residual=std::max(
            arm.maximum_first_passage_residual,std::abs(predicted-arm.pair_history[i]));
        if(arm.predicted_onset_tick<0&&predicted<-1e-6) arm.predicted_onset_tick=tick;
      }
    }
    arm.core_pass=arm.initial_pass&&arm.energetic_onset_tick<=120
        &&kFiniteTicks-arm.energetic_onset_tick+1>=16;
    arm.first_passage_pass=arm.final_entry_tick>=0
        &&arm.predicted_onset_tick==arm.energetic_onset_tick
        &&arm.maximum_first_passage_residual<=1e-8;
  }
  if(unbound) arm.tail_pass=arm.rows.front().outside_energy_12<=1e-12
      &&arm.maximum_outside_12>1e-6&&arm.maximum_cumulative_outward_12>1e-6
      &&arm.final_outside_12>1e-7&&arm.first_tail_tick>=0
      &&arm.maximum_source_radius<=3;
  else {
    arm.bound_control_pass=arm.initial_pass&&arm.transition_ticks.empty();
    for(std::size_t tick=0;tick<arm.pair_history.size();++tick)
      arm.bound_control_pass=arm.bound_control_pass&&arm.pair_history[tick]<-1e-6
          &&graph_inside(arm.separation_history[tick],options);
  }

  ConnectedMooreBlockState recovered=state; bool reverse_valid=true;
  ConnectedMooreBlockSolveCache reverse_cache; double unused=0.0;
  for(int tick=1;tick<=kFiniteTicks;++tick) {
    const auto step=ftd::eft::solve_connected_moore_block_reverse(
        recovered,options,&reverse_cache);
    reverse_valid=reverse_valid&&step.valid;if(!step.valid)break;
    recovered=step.earlier;
    auto row=make_finite_record(family,direction,conjugate,"reverse",tick,
        step,recovered,options,interaction_scale,center,&unused);
    arm.maximum_source_radius=std::max(arm.maximum_source_radius,row.source_radius);
    arm.source_entries+=row.source_entries;
    arm.maximum_common_residual=std::max(arm.maximum_common_residual,row.maximum_residual);
    arm.maximum_energy_residual=std::max(arm.maximum_energy_residual,row.total_energy_residual);
    arm.maximum_recoil_defect=std::max(arm.maximum_recoil_defect,row.recoil_defect);
    arm.maximum_speed_excess=std::max(arm.maximum_speed_excess,row.speed_excess);
    common=common&&row.common&&row.maximum_residual<=kFiniteGate;
    recoil=recoil&&row.recoil_defect<=1e-9;speed=speed&&row.speed_excess<=1e-12;
    arm.rows.push_back(std::move(row));
  }
  arm.reverse_executed=reverse_valid
      &&arm.rows.size()==static_cast<std::size_t>(2*kFiniteTicks+1);
  arm.inverse_recovery=arm.reverse_executed
      ?ftd::eft::connected_moore_block_state_max_difference(original,recovered):INFINITY;
  arm.identity_pass=common;arm.regional_pass=regional;arm.recoil_pass=recoil;
  arm.speed_pass=speed;arm.inverse_pass=arm.inverse_recovery<=1e-8;
  arm.support_pass=arm.maximum_source_radius<=3&&kFiniteTicks<kFiniteContactTick;
  if(!unbound&&arm.bound_control_pass)
    for(const auto& row:arm.rows)
      arm.bound_control_pass=arm.bound_control_pass&&row.graph_inside
          &&row.pair_energy<-1e-6;
  return arm;
}

const FiniteArm* find_finite_arm(
    const std::vector<FiniteArm>& arms, const std::string& family,
    const std::string& direction, const std::string& polarity) {
  const auto found=std::find_if(arms.begin(),arms.end(),[&](const FiniteArm& arm) {
    return arm.family==family&&arm.direction==direction&&arm.polarity==polarity;
  });
  return found==arms.end()?nullptr:&*found;
}

double finite_scalar_difference(const FiniteArm& first,
                                const FiniteArm& second) {
  if(first.separation_history.size()!=second.separation_history.size()
      ||first.pair_history.size()!=second.pair_history.size()
      ||first.field_history.size()!=second.field_history.size()
      ||first.outside12_history.size()!=second.outside12_history.size()
      ||first.rows.size()!=second.rows.size())
    return INFINITY;
  double result=0.0;
  for(std::size_t i=0;i<first.separation_history.size();++i) {
    result=std::max(result,std::abs(first.separation_history[i]
        -second.separation_history[i]));
    result=std::max(result,std::abs(first.pair_history[i]
        -second.pair_history[i]));
    result=std::max(result,std::abs(first.field_history[i]
        -second.field_history[i]));
    result=std::max(result,std::abs(first.outside12_history[i]
        -second.outside12_history[i]));
  }
  for(std::size_t i=0;i<first.rows.size();++i) {
    const auto& lhs=first.rows[i];
    const auto& rhs=second.rows[i];
    if(lhs.family!=rhs.family||lhs.direction!=rhs.direction
        ||lhs.phase!=rhs.phase||lhs.tick!=rhs.tick||lhs.valid!=rhs.valid
        ||lhs.common!=rhs.common||lhs.regional_valid!=rhs.regional_valid
        ||lhs.source_radius!=rhs.source_radius
        ||lhs.source_entries!=rhs.source_entries
        ||lhs.graph_inside!=rhs.graph_inside)
      return INFINITY;
    for(const auto& values:{
        std::pair{lhs.maximum_residual,rhs.maximum_residual},
        std::pair{lhs.total_energy_residual,rhs.total_energy_residual},
        std::pair{lhs.recoil_defect,rhs.recoil_defect},
        std::pair{lhs.speed_excess,rhs.speed_excess},
        std::pair{lhs.regional_residual,rhs.regional_residual},
        std::pair{lhs.separation,rhs.separation},
        std::pair{lhs.pair_energy,rhs.pair_energy},
        std::pair{lhs.field_energy,rhs.field_energy},
        std::pair{lhs.inside_energy_8,rhs.inside_energy_8},
        std::pair{lhs.outside_energy_8,rhs.outside_energy_8},
        std::pair{lhs.boundary_transport_into_8,rhs.boundary_transport_into_8},
        std::pair{lhs.source_exchange_8,rhs.source_exchange_8},
        std::pair{lhs.inside_energy_12,rhs.inside_energy_12},
        std::pair{lhs.outside_energy_12,rhs.outside_energy_12},
        std::pair{lhs.boundary_transport_into_12,rhs.boundary_transport_into_12},
        std::pair{lhs.source_exchange_12,rhs.source_exchange_12},
        std::pair{lhs.cumulative_outward_12,rhs.cumulative_outward_12}})
      result=std::max(result,std::abs(values.first-values.second));
  }
  return result;
}

void write_finite_records(const std::vector<FiniteArm>& arms,
                          const std::string& verdict,
                          double polarity_difference) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0739";
  std::filesystem::create_directories(directory);
  std::ofstream csv(directory/
      "ftd_0739_finite_support_outgoing_tail_formation_v1.csv");
  csv << "family,direction,polarity,phase,tick,valid,common,regional_valid,"
         "max_residual,total_energy_residual,recoil_defect,speed_excess,"
         "regional_residual,source_radius,source_entries,separation,"
         "pair_energy,field_energy,graph_inside,inside_energy_8,"
         "outside_energy_8,boundary_transport_into_8,source_exchange_8,"
         "inside_energy_12,outside_energy_12,boundary_transport_into_12,"
         "source_exchange_12,cumulative_outward_12\n"
      << std::setprecision(17);
  for(const auto& arm:arms) for(const auto& row:arm.rows)
    csv << row.family << ',' << row.direction << ',' << row.polarity << ','
        << row.phase << ',' << row.tick << ',' << row.valid << ','
        << row.common << ',' << row.regional_valid << ','
        << row.maximum_residual << ',' << row.total_energy_residual << ','
        << row.recoil_defect << ',' << row.speed_excess << ','
        << row.regional_residual << ',' << row.source_radius << ','
        << row.source_entries << ',' << row.separation << ','
        << row.pair_energy << ',' << row.field_energy << ','
        << row.graph_inside << ',' << row.inside_energy_8 << ','
        << row.outside_energy_8 << ',' << row.boundary_transport_into_8 << ','
        << row.source_exchange_8 << ',' << row.inside_energy_12 << ','
        << row.outside_energy_12 << ',' << row.boundary_transport_into_12 << ','
        << row.source_exchange_12 << ',' << row.cumulative_outward_12 << '\n';

  int cores=0,first_passages=0,tails=0,bound_controls=0;
  std::size_t rows=0;
  int maximum_source=0;
  double maximum_common=0.0,maximum_energy=0.0,maximum_recoil=0.0;
  double maximum_speed=0.0,maximum_regional=0.0,maximum_inverse=0.0;
  double maximum_balance=0.0,maximum_first_passage=0.0;
  double maximum_outside=0.0,maximum_outward=0.0;
  for(const auto& arm:arms) {
    rows+=arm.rows.size();
    if(arm.family=="unbound") {
      cores+=arm.core_pass?1:0;
      first_passages+=arm.first_passage_pass?1:0;
      tails+=arm.tail_pass?1:0;
    } else bound_controls+=arm.bound_control_pass?1:0;
    maximum_source=std::max(maximum_source,arm.maximum_source_radius);
    maximum_common=std::max(maximum_common,arm.maximum_common_residual);
    maximum_energy=std::max(maximum_energy,arm.maximum_energy_residual);
    maximum_recoil=std::max(maximum_recoil,arm.maximum_recoil_defect);
    maximum_speed=std::max(maximum_speed,arm.maximum_speed_excess);
    maximum_regional=std::max(maximum_regional,arm.maximum_regional_residual);
    maximum_inverse=std::max(maximum_inverse,arm.inverse_recovery);
    maximum_balance=std::max(maximum_balance,arm.pair_field_balance);
    maximum_first_passage=std::max(
        maximum_first_passage,arm.maximum_first_passage_residual);
    maximum_outside=std::max(maximum_outside,arm.maximum_outside_12);
    maximum_outward=std::max(
        maximum_outward,arm.maximum_cumulative_outward_12);
  }

  std::ofstream json(directory/
      "ftd_0739_finite_support_outgoing_tail_formation_v1.json");
  json << std::setprecision(17)
       << "{\n"
       << "  \"ftd_id\": \"FTD-0739\",\n"
       << "  \"protocol_sha256\": \"" << kFiniteProtocolSha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"volume\": " << kFiniteL << ",\n"
       << "  \"horizon\": " << kFiniteTicks << ",\n"
       << "  \"initial_support_radius\": " << kFiniteSupportRadius << ",\n"
       << "  \"locked_contact_tick\": " << kFiniteContactTick << ",\n"
       << "  \"history_count\": " << arms.size() << ",\n"
       << "  \"step_row_count\": " << rows << ",\n"
       << "  \"unbound_core_passes\": " << cores << ",\n"
       << "  \"unbound_first_passage_passes\": " << first_passages << ",\n"
       << "  \"unbound_tail_passes\": " << tails << ",\n"
       << "  \"bound_controls\": " << bound_controls << ",\n"
       << "  \"polarity_scalar_difference\": " << polarity_difference << ",\n"
       << "  \"maximum_source_radius\": " << maximum_source << ",\n"
       << "  \"maximum_common_residual\": " << maximum_common << ",\n"
       << "  \"maximum_total_energy_residual\": " << maximum_energy << ",\n"
       << "  \"maximum_recoil_defect\": " << maximum_recoil << ",\n"
       << "  \"maximum_causal_speed_excess\": " << maximum_speed << ",\n"
       << "  \"maximum_regional_residual\": " << maximum_regional << ",\n"
       << "  \"maximum_inverse_recovery\": " << maximum_inverse << ",\n"
       << "  \"maximum_pair_field_balance\": " << maximum_balance << ",\n"
       << "  \"maximum_first_passage_residual\": "
       << maximum_first_passage << ",\n"
       << "  \"maximum_outside_energy_12\": " << maximum_outside << ",\n"
       << "  \"maximum_cumulative_outward_12\": " << maximum_outward << ",\n"
       << "  \"arms\": [\n";
  for(std::size_t i=0;i<arms.size();++i) {
    const auto& arm=arms[i];
    std::ostringstream transitions;
    for(std::size_t j=0;j<arm.transition_ticks.size();++j) {
      if(j!=0) transitions << ';';
      transitions << arm.transition_ticks[j];
    }
    json << "    {\"family\": \"" << arm.family
         << "\", \"direction\": \"" << arm.direction
         << "\", \"polarity\": \"" << arm.polarity
         << "\", \"initialized\": " << arm.initialized
         << ", \"preparation_pass\": " << arm.preparation_pass
         << ", \"forward_executed\": " << arm.forward_executed
         << ", \"reverse_executed\": " << arm.reverse_executed
         << ", \"identity_pass\": " << arm.identity_pass
         << ", \"regional_pass\": " << arm.regional_pass
         << ", \"recoil_pass\": " << arm.recoil_pass
         << ", \"speed_pass\": " << arm.speed_pass
         << ", \"inverse_pass\": " << arm.inverse_pass
         << ", \"support_pass\": " << arm.support_pass
         << ", \"initial_pass\": " << arm.initial_pass
         << ", \"core_pass\": " << arm.core_pass
         << ", \"first_passage_pass\": " << arm.first_passage_pass
         << ", \"tail_pass\": " << arm.tail_pass
         << ", \"bound_control_pass\": " << arm.bound_control_pass
         << ", \"transition_ticks\": \"" << transitions.str()
         << "\", \"energetic_onset_tick\": " << arm.energetic_onset_tick
         << ", \"final_entry_tick\": " << arm.final_entry_tick
         << ", \"predicted_onset_tick\": " << arm.predicted_onset_tick
         << ", \"first_tail_tick\": " << arm.first_tail_tick
         << ", \"preparation_poisson_residual\": "
         << arm.preparation_poisson_residual
         << ", \"preparation_gauss_residual\": "
         << arm.preparation_gauss_residual
         << ", \"preparation_outside_maximum\": "
         << arm.preparation_outside_maximum
         << ", \"preparation_boundary_maximum\": "
         << arm.preparation_boundary_maximum
         << ", \"preparation_curl_adjoint\": "
         << arm.preparation_curl_adjoint
         << ", \"maximum_source_radius\": " << arm.maximum_source_radius
         << ", \"maximum_outside_12\": " << arm.maximum_outside_12
         << ", \"final_outside_12\": " << arm.final_outside_12
         << ", \"maximum_cumulative_outward_12\": "
         << arm.maximum_cumulative_outward_12
         << ", \"maximum_first_passage_residual\": "
         << arm.maximum_first_passage_residual
         << ", \"pair_field_balance\": " << arm.pair_field_balance
         << ", \"inverse_recovery\": " << arm.inverse_recovery << "}"
         << (i+1==arms.size()?"\n":",\n");
  }
  json << "  ]\n}\n";
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
    std::vector<FiniteArm> result;
    result.push_back(run_finite_arm("unbound",face,false,options,interaction_scale));
    result.push_back(run_finite_arm("bound",face,false,options,interaction_scale));
    return result;
  });
  auto edge_future=std::async(std::launch::async,[=]() {
    std::vector<FiniteArm> result;
    result.push_back(run_finite_arm("unbound",edge,false,options,interaction_scale));
    return result;
  });
  auto body_future=std::async(std::launch::async,[=]() {
    std::vector<FiniteArm> result;
    result.push_back(run_finite_arm("unbound",body,false,options,interaction_scale));
    result.push_back(run_finite_arm("unbound",body,true,options,interaction_scale));
    return result;
  });
  std::vector<FiniteArm> arms;
  auto face_arms=face_future.get(),edge_arms=edge_future.get();
  auto body_arms=body_future.get();
  for(auto* group:{&face_arms,&edge_arms,&body_arms})
    for(auto& arm:*group) arms.push_back(std::move(arm));
  std::sort(arms.begin(),arms.end(),[](const FiniteArm& a,const FiniteArm& b) {
    return std::tie(a.family,a.direction,a.polarity)
        <std::tie(b.family,b.direction,b.polarity);
  });

  const auto plus_body=find_finite_arm(arms,"unbound","1_1_1","plus_minus");
  const auto minus_body=find_finite_arm(arms,"unbound","1_1_1","minus_plus");
  const double polarity_difference=plus_body&&minus_body
      ?finite_scalar_difference(*plus_body,*minus_body):INFINITY;
  const bool polarity_pass=plus_body&&minus_body
      &&plus_body->transition_ticks==minus_body->transition_ticks
      &&plus_body->energetic_onset_tick==minus_body->energetic_onset_tick
      &&polarity_difference<=1e-9;
  const bool matrix=normalization.valid&&arms.size()==5
      &&std::count_if(arms.begin(),arms.end(),[](const FiniteArm& arm) {
        return arm.family=="unbound";
      })==4
      &&std::count_if(arms.begin(),arms.end(),[](const FiniteArm& arm) {
        return arm.family=="bound";
      })==1;
  const bool infrastructure=matrix&&std::all_of(
      arms.begin(),arms.end(),[](const FiniteArm& arm) {
        return arm.initialized&&arm.preparation_pass&&arm.forward_executed
            &&arm.reverse_executed&&arm.identity_pass&&arm.regional_pass
            &&arm.recoil_pass&&arm.speed_pass&&arm.inverse_pass
            &&arm.support_pass&&arm.initial_pass
            &&arm.maximum_energy_residual<=1e-8
            &&arm.pair_field_balance<=1e-8;
      });
  const bool control=std::all_of(arms.begin(),arms.end(),[](const FiniteArm& arm) {
    return arm.family!="bound"||arm.bound_control_pass;
  });
  const bool cores=std::all_of(arms.begin(),arms.end(),[](const FiniteArm& arm) {
    return arm.family!="unbound"||arm.core_pass;
  });
  const bool first_passages=std::all_of(
      arms.begin(),arms.end(),[](const FiniteArm& arm) {
        return arm.family!="unbound"||arm.first_passage_pass;
      });
  const bool tails=std::all_of(arms.begin(),arms.end(),[](const FiniteArm& arm) {
    return arm.family!="unbound"||arm.tail_pass;
  });

  std::string verdict;
  if(!infrastructure)
    verdict="FINITE_SUPPORT_FORMATION_EXECUTION_INVALID";
  else if(!control)
    verdict="FINITE_SUPPORT_BOUND_CONTROL_UNSTABLE";
  else if(!polarity_pass)
    verdict="FINITE_SUPPORT_FORMATION_POLARITY_SENSITIVE";
  else if(!cores)
    verdict="FINITE_SUPPORT_NO_DURABLE_NEGATIVE_CORE_ALL_RAYS";
  else if(!first_passages)
    verdict="FINITE_SUPPORT_CAPTURE_ENERGY_LEDGER_MISMATCH";
  else if(!tails)
    verdict="FINITE_SUPPORT_CORE_WITHOUT_OUTGOING_TAIL";
  else
    verdict="FINITE_SUPPORT_OUTGOING_TAIL_FORMATION_CONSTRUCTIVE";
  write_finite_records(arms,verdict,polarity_difference);
  int core_count=0,tail_count=0;
  double maximum_outside=0.0,maximum_inverse=0.0;
  for(const auto& arm:arms) {
    if(arm.family=="unbound") {
      core_count+=arm.core_pass?1:0;
      tail_count+=arm.tail_pass?1:0;
    }
    maximum_outside=std::max(maximum_outside,arm.maximum_outside_12);
    maximum_inverse=std::max(maximum_inverse,arm.inverse_recovery);
  }
  std::cout << "FTD-0739 " << verdict << " cores=" << core_count << "/4"
            << " tails=" << tail_count << "/4"
            << " outside12=" << std::setprecision(8) << maximum_outside
            << " inverse=" << maximum_inverse << '\n';
  return infrastructure?0:1;
}
