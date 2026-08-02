// FTD-0623: repeated rest/boost dynamics of the connected w=2 integer object.

#include "ftd/eft/connected_moore_block_action.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

using ftd::Vec3;
constexpr char protocol_sha256[] =
    "7AA42C401938C48F134A1BF95C70FD8C6026B24B0FE2979173BBEF598800A3F7";
constexpr char parent_sha256[] =
    "6ED5287FB9AD84BACED79885E24E2352FE05CA82FA77636DD968297D6DF73396";
constexpr int L = 17, width = 2, tick_count = 16;
constexpr double launch_p = 0.12, exact_gate = 1e-10;

struct ArmSpec {
  std::string label;
  int orientation = 0;
  Vec3 momentum{}, direction{};
  bool rest = false;
};

struct TickRecord {
  int tick = 0, hops = 0;
  Vec3 center{}, momentum{}, local_defect{}, spline_defect{};
  double kinetic = 0.0, binding = 0.0, field = 0.0, total = 0.0;
  double energy_drift = 0.0, shape = 0.0, strain = 0.0;
  double common_residual = 0.0, cumulative_D = 0.0;
};

struct ArmResult {
  ArmSpec spec{};
  bool initialization_pass = false, forward_pass = false;
  bool reverse_pass = false, coherence_pass = false;
  bool rest_pass = true, transport_pass = true;
  int total_hops = 0;
  double free_speed = 0.0, free_displacement = 0.0;
  Vec3 displacement{}, final_momentum{}, final_velocity{};
  double projected_displacement = 0.0, transverse_displacement = 0.0;
  double maximum_shape = INFINITY, maximum_strain = INFINITY;
  double maximum_common = INFINITY, maximum_energy_drift = INFINITY;
  double recovery = INFINITY, cumulative_D = INFINITY;
  std::vector<TickRecord> history;
};

struct Summary {
  bool parent_pass = false, coverage_pass = false, action_pass = false;
  bool rest_pass = false, mobility_pass = false;
  bool mirror_pass = false, covariance_pass = false;
  double beta = 0.0, mirror_residual = INFINITY;
  double covariance_residual = INFINITY, worst_common = INFINITY;
  double worst_energy_drift = INFINITY, worst_recovery = INFINITY;
  std::string verdict;
  std::vector<ArmResult> arms;
};

double max_component(const Vec3& v) {
  return std::max({std::abs(v.x),std::abs(v.y),std::abs(v.z)});
}
Vec3 cycle(const Vec3& v) { return {v.z,v.x,v.y}; }
double relative(double a, double b) {
  return std::abs(a-b)/std::max({1e-300,std::abs(a),std::abs(b)});
}
Vec3 position(const ftd::eft::MatchedMatterPoint& p) {
  return {static_cast<double>(p.anchor.x)+p.remainder.x,
          static_cast<double>(p.anchor.y)+p.remainder.y,
          static_cast<double>(p.anchor.z)+p.remainder.z};
}

double maximum_residual(
    const ftd::eft::ConnectedMooreBlockStepResult& s) {
  return std::max({s.root_residual,s.continuity_residual,
      s.gauss_before_residual,s.gauss_after_residual,s.force_residual,
      s.kinematic_residual,s.kinetic_discrete_gradient_residual,
      s.electric_adjoint_residual,s.magnetic_work_residual,
      s.binding_work_residual,s.binding_impulse_sum_residual,
      s.matter_work_residual,s.field_work_residual,s.total_energy_residual,
      s.causal_speed_excess});
}

Vec3 center(const ftd::eft::ConnectedMooreBlockState& s) {
  Vec3 r{};
  for (const auto& p : s.constituents) r += position(p);
  return r*(1.0/static_cast<double>(s.constituents.size()));
}
Vec3 momentum(const ftd::eft::ConnectedMooreBlockState& s) {
  Vec3 r{};
  for (const auto& p : s.constituents) r += p.momentum;
  return r;
}
Vec3 mean_velocity(const ftd::eft::ConnectedMooreBlockState& s) {
  Vec3 r{};
  for (const auto& p : s.constituents)
    r += ftd::eft::production_flat_velocity_from_momentum(p.momentum);
  return r*(1.0/static_cast<double>(s.constituents.size()));
}
double kinetic(const ftd::eft::ConnectedMooreBlockState& s) {
  long double r = 0.0L;
  for (const auto& p : s.constituents)
    r += ftd::eft::production_flat_energy_from_momentum(p.momentum);
  return static_cast<double>(r);
}
double field(const ftd::eft::ConnectedMooreBlockState& s, double beta) {
  return beta*ftd::eft::matched_modified_energy(
      s.electric,s.magnetic_half,ftd::C_SPEED);
}
double total(const ftd::eft::ConnectedMooreBlockState& s,
             const ftd::eft::ConnectedMooreBlockOptions& o, double beta) {
  return kinetic(s)+ftd::eft::connected_moore_block_binding_energy(s,o)
      +field(s,beta);
}
double shape_error(const ftd::eft::ConnectedMooreBlockState& initial,
                   const ftd::eft::ConnectedMooreBlockState& state) {
  const Vec3 c0 = center(initial), c1 = center(state);
  long double sum = 0.0L;
  for (std::size_t i = 0; i < state.constituents.size(); ++i) {
    const Vec3 d = (position(state.constituents[i])-c1)
        -(position(initial.constituents[i])-c0);
    sum += d.dot(d);
  }
  return std::sqrt(static_cast<double>(
      sum/static_cast<long double>(state.constituents.size())));
}

bool parent_fingerprint() {
  const auto p = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0622/ftd_0622_connected_moore_block_action_v1.json";
  std::ifstream in(p,std::ios::binary);
  const std::string b((std::istreambuf_iterator<char>(in)),{});
  return b.find("\"ftd_id\": \"FTD-0622\"") != std::string::npos
      && b.find("CONNECTED_MOORE_BLOCK_ACTION_CONSTRUCTIVE_IR_TREND_POSITIVE")
          != std::string::npos;
}

ArmResult run_arm(const ArmSpec& spec,
                  const ftd::eft::ConnectedMooreBlockOptions& options,
                  double beta) {
  ArmResult r;
  r.spec = spec;
  auto init = ftd::eft::initialize_connected_moore_block(
      L,width,spec.orientation,spec.orientation,0.0);
  r.initialization_pass = init.valid && init.state.constituents.size() == 16
      && init.state.edges.size() == 72 && init.poisson_residual <= 1e-11
      && init.gauss_residual <= 1e-11
      && init.curl_adjoint_residual <= 1e-11;
  if (!r.initialization_pass) return r;
  for (auto& p : init.state.constituents) p.momentum = spec.momentum;
  const auto initial = init.state;
  auto state = initial;
  const Vec3 c0 = center(initial);
  const double e0 = total(initial,options,beta);
  r.free_speed = ftd::eft::production_flat_velocity_from_momentum(
      spec.momentum).mag();
  r.free_displacement = tick_count*r.free_speed;
  r.maximum_shape = r.maximum_strain = r.maximum_common = 0.0;
  r.maximum_energy_drift = r.cumulative_D = 0.0;
  Vec3 cumulative_spline{};
  r.forward_pass = true;
  for (int tick = 1; tick <= tick_count; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state,options);
    const double residual = maximum_residual(step);
    r.maximum_common = std::max(r.maximum_common,residual);
    if (!step.common_action_gates_pass || residual > exact_gate) {
      r.forward_pass = false;
      break;
    }
    state = step.later;
    r.total_hops += step.site_hops;
    cumulative_spline += step.spline_total_defect;
    const double ke = kinetic(state);
    const double be = ftd::eft::connected_moore_block_binding_energy(
        state,options);
    const double fe = field(state,beta), te = ke+be+fe;
    const double drift = std::abs(te-e0);
    const double shape = shape_error(initial,state);
    r.maximum_energy_drift = std::max(r.maximum_energy_drift,drift);
    r.maximum_shape = std::max(r.maximum_shape,shape);
    r.maximum_strain = std::max(r.maximum_strain,step.maximum_edge_strain);
    const double D = ftd::C_SPEED*cumulative_spline.mag()/field(initial,beta);
    r.history.push_back({tick,step.site_hops,center(state),momentum(state),
        step.local_total_defect,step.spline_total_defect,ke,be,fe,te,drift,
        shape,step.maximum_edge_strain,residual,D});
  }
  r.forward_pass = r.forward_pass && r.history.size() == tick_count
      && r.maximum_energy_drift <= 1e-9;
  r.coherence_pass = r.forward_pass && r.maximum_shape <= 0.25
      && r.maximum_strain <= 0.25;
  r.displacement = center(state)-c0;
  r.final_momentum = momentum(state);
  r.final_velocity = mean_velocity(state);
  if (!r.history.empty()) r.cumulative_D = r.history.back().cumulative_D;
  if (spec.rest) {
    r.rest_pass = r.coherence_pass && r.displacement.mag() <= 1e-8
        && r.final_momentum.mag() <= 1e-8 && r.total_hops == 0;
  } else {
    r.projected_displacement = r.displacement.dot(spec.direction);
    r.transverse_displacement =
        (r.displacement-spec.direction*r.projected_displacement).mag();
    r.transport_pass = r.coherence_pass
        && r.projected_displacement >= 0.75
        && r.displacement.mag() <= 1.5*r.free_displacement
        && r.transverse_displacement <= 0.10 && r.total_hops >= 16
        && r.final_velocity.dot(spec.direction) > 0.0;
  }
  r.reverse_pass = r.forward_pass;
  for (int tick = tick_count; r.reverse_pass && tick >= 1; --tick) {
    const auto step = ftd::eft::solve_connected_moore_block_reverse(
        state,options);
    const double residual = maximum_residual(step);
    r.maximum_common = std::max(r.maximum_common,residual);
    if (!step.common_action_gates_pass || residual > exact_gate)
      r.reverse_pass = false;
    else state = step.earlier;
  }
  if (r.reverse_pass)
    r.recovery = ftd::eft::connected_moore_block_state_max_difference(
        initial,state);
  r.reverse_pass = r.reverse_pass && r.recovery <= 1e-8;
  return r;
}

const ArmResult* find(const Summary& s, const std::string& label) {
  const auto it = std::find_if(s.arms.begin(),s.arms.end(),
      [&](const ArmResult& r) { return r.spec.label == label; });
  return it == s.arms.end() ? nullptr : &*it;
}
bool basic(const ArmResult& r) {
  return r.initialization_pass && r.forward_pass && r.reverse_pass
      && r.coherence_pass;
}

void evaluate(Summary& s) {
  s.coverage_pass = s.arms.size() == 5;
  s.action_pass = s.parent_pass && s.coverage_pass;
  s.mobility_pass = s.coverage_pass;
  s.worst_common = s.worst_energy_drift = s.worst_recovery = 0.0;
  for (const auto& r : s.arms) {
    s.action_pass = s.action_pass && basic(r);
    if (!r.spec.rest) s.mobility_pass = s.mobility_pass && r.transport_pass;
    if (std::isfinite(r.maximum_common))
      s.worst_common = std::max(s.worst_common,r.maximum_common);
    if (std::isfinite(r.maximum_energy_drift))
      s.worst_energy_drift = std::max(
          s.worst_energy_drift,r.maximum_energy_drift);
    if (std::isfinite(r.recovery))
      s.worst_recovery = std::max(s.worst_recovery,r.recovery);
  }
  const auto* rest = find(s,"rest");
  s.rest_pass = rest != nullptr && rest->rest_pass;
  const auto* pos = find(s,"parallel_positive");
  const auto* neg = find(s,"parallel_negative");
  s.mirror_pass = s.action_pass && pos && neg
      && pos->history.size() == neg->history.size();
  s.mirror_residual = 0.0;
  if (s.mirror_pass) {
    s.mirror_residual = max_component(pos->displacement+neg->displacement);
    for (std::size_t i = 0; i < pos->history.size(); ++i)
      s.mirror_residual = std::max({s.mirror_residual,
          std::abs(pos->history[i].field-neg->history[i].field),
          std::abs(pos->history[i].shape-neg->history[i].shape)});
  }
  s.mirror_pass = s.mirror_pass && s.mirror_residual <= 1e-8;
  const auto* rot = find(s,"cyclic_parallel");
  s.covariance_pass = s.action_pass && pos && rot
      && pos->history.size() == rot->history.size();
  s.covariance_residual = 0.0;
  if (s.covariance_pass) {
    s.covariance_residual = std::max({
        max_component(rot->displacement-cycle(pos->displacement)),
        max_component(rot->final_momentum-cycle(pos->final_momentum)),
        relative(rot->cumulative_D,pos->cumulative_D),
        std::abs(static_cast<double>(rot->total_hops-pos->total_hops))});
    for (std::size_t i = 0; i < pos->history.size(); ++i)
      s.covariance_residual = std::max({s.covariance_residual,
          max_component(rot->history[i].center-cycle(pos->history[i].center)),
          max_component(rot->history[i].momentum
                        -cycle(pos->history[i].momentum)),
          relative(rot->history[i].field,pos->history[i].field),
          relative(rot->history[i].shape,pos->history[i].shape),
          relative(rot->history[i].strain,pos->history[i].strain),
          std::abs(static_cast<double>(rot->history[i].hops
                                      -pos->history[i].hops))});
  }
  s.covariance_pass = s.covariance_pass && s.covariance_residual <= 1e-8;
  if (!s.action_pass || !s.rest_pass)
    s.verdict = "CONNECTED_INTEGER_OBJECT_REPEATED_DYNAMICS_INVALID";
  else if (s.mobility_pass && s.mirror_pass && s.covariance_pass)
    s.verdict = "CONNECTED_INTEGER_OBJECT_REPEATED_MOBILITY_CONSTRUCTIVE";
  else s.verdict = "CONNECTED_INTEGER_OBJECT_STABLE_BUT_MOBILITY_NEGATIVE";
}

void write_records(const Summary& s) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0623";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir/"ftd_0623_connected_moore_block_repeated_v1.json");
  json << std::setprecision(17) << "{\n  \"ftd_id\": \"FTD-0623\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"parent_result_sha256\": \"" << parent_sha256 << "\",\n"
       << "  \"verdict\": \"" << s.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"parent_pass\": " << s.parent_pass << ",\n"
       << "  \"coverage_pass\": " << s.coverage_pass << ",\n"
       << "  \"action_pass\": " << s.action_pass << ",\n"
       << "  \"rest_pass\": " << s.rest_pass << ",\n"
       << "  \"mobility_pass\": " << s.mobility_pass << ",\n"
       << "  \"sign_mirror_pass\": " << s.mirror_pass << ",\n"
       << "  \"covariance_pass\": " << s.covariance_pass << ",\n"
       << "  \"beta\": " << s.beta << ",\n"
       << "  \"worst_sign_mirror_residual\": " << s.mirror_residual << ",\n"
       << "  \"worst_covariance_residual\": " << s.covariance_residual << ",\n"
       << "  \"worst_common_residual\": " << s.worst_common << ",\n"
       << "  \"worst_energy_drift\": " << s.worst_energy_drift << ",\n"
       << "  \"worst_recovery\": " << s.worst_recovery << "\n}\n";
  std::ofstream arms(dir/"ftd_0623_connected_moore_block_repeated_arms_v1.csv");
  arms << "ftd_id,label,orientation,rest,init,forward,reverse,coherence,rest_pass,transport_pass,total_hops,free_speed,free_displacement,center_x,center_y,center_z,final_momentum_x,final_momentum_y,final_momentum_z,final_velocity_x,final_velocity_y,final_velocity_z,projected_displacement,transverse_displacement,maximum_shape_error,maximum_edge_strain,maximum_common_residual,maximum_energy_drift,recovery,cumulative_D\n";
  for (const auto& r : s.arms)
    arms << std::setprecision(17) << "FTD-0623," << r.spec.label << ','
         << r.spec.orientation << ',' << r.spec.rest << ','
         << r.initialization_pass << ',' << r.forward_pass << ','
         << r.reverse_pass << ',' << r.coherence_pass << ',' << r.rest_pass
         << ',' << r.transport_pass << ',' << r.total_hops << ','
         << r.free_speed << ',' << r.free_displacement << ','
         << r.displacement.x << ',' << r.displacement.y << ','
         << r.displacement.z << ',' << r.final_momentum.x << ','
         << r.final_momentum.y << ',' << r.final_momentum.z << ','
         << r.final_velocity.x << ',' << r.final_velocity.y << ','
         << r.final_velocity.z << ',' << r.projected_displacement << ','
         << r.transverse_displacement << ',' << r.maximum_shape << ','
         << r.maximum_strain << ',' << r.maximum_common << ','
         << r.maximum_energy_drift << ',' << r.recovery << ','
         << r.cumulative_D << '\n';
  std::ofstream rows(dir/"ftd_0623_connected_moore_block_repeated_ticks_v1.csv");
  rows << "ftd_id,label,tick,center_x,center_y,center_z,momentum_x,momentum_y,momentum_z,kinetic,binding,field,total,energy_drift,shape_error,edge_strain,site_hops,common_residual,local_x,local_y,local_z,spline_x,spline_y,spline_z,cumulative_D\n";
  for (const auto& r : s.arms) for (const auto& t : r.history)
    rows << std::setprecision(17) << "FTD-0623," << r.spec.label << ','
         << t.tick << ',' << t.center.x << ',' << t.center.y << ','
         << t.center.z << ',' << t.momentum.x << ',' << t.momentum.y << ','
         << t.momentum.z << ',' << t.kinetic << ',' << t.binding << ','
         << t.field << ',' << t.total << ',' << t.energy_drift << ','
         << t.shape << ',' << t.strain << ',' << t.hops << ','
         << t.common_residual << ',' << t.local_defect.x << ','
         << t.local_defect.y << ',' << t.local_defect.z << ','
         << t.spline_defect.x << ',' << t.spline_defect.y << ','
         << t.spline_defect.z << ',' << t.cumulative_D << '\n';
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  Summary s;
  s.parent_pass = parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  s.beta = normalization.mapped_field_work_coefficient;
  ftd::eft::ConnectedMooreBlockOptions options;
  options.gate_tolerance = exact_gate;
  options.solve_tolerance = 2e-11;
  options.max_iterations = 48;
  const std::vector<ArmSpec> specs{
      {"rest",0,{0,0,0},{0,0,0},true},
      {"parallel_positive",0,{launch_p,0,0},{1,0,0},false},
      {"parallel_negative",0,{-launch_p,0,0},{-1,0,0},false},
      {"transverse_positive",0,{0,launch_p,0},{0,1,0},false},
      {"cyclic_parallel",1,{0,launch_p,0},{0,1,0},false}};
  if (s.parent_pass && normalization.valid) for (const auto& spec : specs) {
    std::cout << "running " << spec.label << std::endl;
    s.arms.push_back(run_arm(spec,options,s.beta));
  }
  evaluate(s);
  write_records(s);
  std::cout << "protocol_sha256=" << protocol_sha256 << '\n'
            << "verdict=" << s.verdict << '\n'
            << "action=" << s.action_pass << " rest=" << s.rest_pass
            << " mobility=" << s.mobility_pass << " mirror=" << s.mirror_pass
            << " covariance=" << s.covariance_pass << '\n'
            << "worst_common=" << s.worst_common
            << " energy_drift=" << s.worst_energy_drift
            << " recovery=" << s.worst_recovery << '\n';
  for (const auto& r : s.arms)
    std::cout << r.spec.label << " pass=" << basic(r)
              << " transport=" << r.transport_pass << " d="
              << r.displacement.x << ',' << r.displacement.y << ','
              << r.displacement.z << " hops=" << r.total_hops
              << " shape=" << r.maximum_shape << " strain="
              << r.maximum_strain << " D=" << r.cumulative_D
              << " recovery=" << r.recovery << '\n';
  return s.verdict == "CONNECTED_INTEGER_OBJECT_REPEATED_DYNAMICS_INVALID"
      ? 1 : 0;
}
