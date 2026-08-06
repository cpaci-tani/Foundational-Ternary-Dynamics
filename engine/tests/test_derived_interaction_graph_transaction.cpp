// FTD-0721: derived interaction graph and closed-pair capture discriminator.

#include "ftd/eft/derived_interaction_graph.h"

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
using ftd::eft::DerivedInteractionGraphOptions;
using ftd::eft::RelationalPairState;

constexpr char protocol_sha256[] =
    "FFCAC54E3368A3DE9FE466908A8BAFF2831D58B0F07AF83BA045BA4315AB6807";
constexpr int tick_count = 256;

struct Direction {
  int x = 0, y = 0, z = 0;
  std::string label;
};

struct ArmResult {
  std::string family;
  std::string direction;
  std::string polarity;
  std::string translation;
  bool pass = false;
  int graph_transitions = 0;
  int active_ticks = 0;
  double initial_internal_energy = 0.0;
  double final_internal_energy = 0.0;
  double recovery = INFINITY;
  double maximum_root = 0.0;
  double maximum_energy = 0.0;
  double maximum_momentum = 0.0;
  double maximum_impulse = 0.0;
  double maximum_kinematic = 0.0;
  double maximum_causal = 0.0;
  std::vector<double> separations;
  std::vector<double> momenta;
};

std::vector<Direction> directions() {
  std::vector<Direction> result;
  for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y)
      for (int z = -1; z <= 1; ++z) {
        if (x == 0 && y == 0 && z == 0) continue;
        const int first = x != 0 ? x : (y != 0 ? y : z);
        if (first < 0) continue;
        result.push_back({x,y,z,std::to_string(x)+"_"
              +std::to_string(y)+"_"+std::to_string(z)});
      }
  return result;
}

double internal_energy(const RelationalPairState& state,
                       const DerivedInteractionGraphOptions& options) {
  const Vec3 delta = state.second_position-state.first_position;
  const double p = state.first_momentum.mag();
  const double h = std::sqrt(options.rest_energy*options.rest_energy
      +options.speed*options.speed*p*p);
  return 2.0*(h-options.rest_energy)
      +ftd::eft::derived_interaction_potential(delta.mag2(),options);
}

ArmResult run_arm(const std::string& family, const Direction& direction,
                  bool conjugate, bool translated,
                  const DerivedInteractionGraphOptions& options) {
  ArmResult result;
  result.family = family;
  result.direction = direction.label;
  result.polarity = conjugate ? "minus_plus" : "plus_minus";
  result.translation = translated ? "shifted" : "origin";
  const Vec3 center = translated ? Vec3{4.0,-3.0,2.0} : Vec3{};
  const Vec3 ray{static_cast<double>(direction.x),
                 static_cast<double>(direction.y),
                 static_cast<double>(direction.z)};
  const bool scattering = family == "scattering";
  const double separation = scattering ? 1.30 : 1.0;
  const double momentum = scattering ? 0.07 : 0.015;
  const int first_polarity = conjugate ? -1 : +1;
  RelationalPairState state = ftd::eft::make_relational_pair_state(
      center,ray,separation,momentum,first_polarity,-first_polarity);
  const RelationalPairState initial = state;
  result.initial_internal_energy = internal_energy(state,options);
  bool all_steps = true;
  bool active = ftd::eft::derived_interaction_edge(state,options);
  result.separations.push_back(
      (state.second_position-state.first_position).mag());
  result.momenta.push_back(state.first_momentum.mag()
      *(state.first_momentum.dot(ray) >= 0.0 ? 1.0 : -1.0));
  for (int tick = 0; tick < tick_count; ++tick) {
    if (active) ++result.active_ticks;
    const auto step = ftd::eft::solve_derived_interaction_graph_step(
        state,options);
    all_steps = all_steps && step.gates_pass;
    result.maximum_root = std::max(result.maximum_root,step.root_residual);
    result.maximum_energy = std::max(
        result.maximum_energy,step.energy_residual);
    result.maximum_momentum = std::max(
        result.maximum_momentum,step.momentum_residual);
    result.maximum_impulse = std::max(
        result.maximum_impulse,step.impulse_balance_residual);
    result.maximum_kinematic = std::max(
        result.maximum_kinematic,step.kinematic_residual);
    result.maximum_causal = std::max(
        result.maximum_causal,step.causal_speed_excess);
    if (!step.gates_pass) break;
    if (step.graph_changed) ++result.graph_transitions;
    state = step.later;
    active = step.edge_after;
    result.separations.push_back(
        (state.second_position-state.first_position).mag());
    const Vec3 unit = ray*(1.0/ray.mag());
    result.momenta.push_back(state.first_momentum.dot(unit));
  }
  result.final_internal_energy = internal_energy(state,options);

  auto inverse_options = options;
  inverse_options.dt = -options.dt;
  RelationalPairState recovered = state;
  bool inverse_steps = all_steps;
  for (int tick = 0; tick < tick_count && inverse_steps; ++tick) {
    const auto inverse = ftd::eft::solve_derived_interaction_graph_step(
        recovered,inverse_options);
    inverse_steps = inverse.gates_pass;
    if (inverse_steps) recovered = inverse.later;
  }
  result.recovery = inverse_steps
      ? ftd::eft::relational_pair_state_max_difference(initial,recovered)
      : INFINITY;
  const bool sign_margin = scattering
      ? result.initial_internal_energy > 1e-6
          && result.final_internal_energy > 1e-6
      : result.initial_internal_energy < -1e-6
          && result.final_internal_energy < -1e-6;
  const bool topology = scattering
      ? !ftd::eft::derived_interaction_edge(initial,options)
          && !ftd::eft::derived_interaction_edge(state,options)
          && result.graph_transitions == 2 && result.active_ticks > 0
      : ftd::eft::derived_interaction_edge(initial,options)
          && ftd::eft::derived_interaction_edge(state,options)
          && result.graph_transitions == 0
          && result.active_ticks == tick_count;
  result.pass = all_steps && inverse_steps && result.recovery < 1e-10
      && sign_margin && topology;
  return result;
}

double scalar_history_spread(const std::vector<ArmResult>& arms) {
  if (arms.empty()) return INFINITY;
  double result = 0.0;
  for (const auto& family : {std::string("scattering"),std::string("bound")}) {
    const auto reference = std::find_if(arms.begin(),arms.end(),
        [&](const ArmResult& arm) { return arm.family == family; });
    if (reference == arms.end()) return INFINITY;
    for (const auto& arm : arms) {
      if (arm.family != family) continue;
      if (arm.separations.size() != reference->separations.size()
          || arm.momenta.size() != reference->momenta.size()) return INFINITY;
      for (std::size_t i = 0; i < arm.separations.size(); ++i) {
        result = std::max(result,
            std::abs(arm.separations[i]-reference->separations[i]));
        result = std::max(result,
            std::abs(arm.momenta[i]-reference->momenta[i]));
      }
    }
  }
  return result;
}

void write_records(const std::vector<ArmResult>& arms,
                   const std::string& verdict, double history_spread) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0721";
  std::filesystem::create_directories(directory);
  double maximum_root = 0.0, maximum_energy = 0.0,
         maximum_momentum = 0.0, maximum_impulse = 0.0,
         maximum_kinematic = 0.0, maximum_causal = 0.0,
         maximum_recovery = 0.0;
  int passed = 0;
  for (const auto& arm : arms) {
    passed += arm.pass ? 1 : 0;
    maximum_root = std::max(maximum_root,arm.maximum_root);
    maximum_energy = std::max(maximum_energy,arm.maximum_energy);
    maximum_momentum = std::max(maximum_momentum,arm.maximum_momentum);
    maximum_impulse = std::max(maximum_impulse,arm.maximum_impulse);
    maximum_kinematic = std::max(maximum_kinematic,arm.maximum_kinematic);
    maximum_causal = std::max(maximum_causal,arm.maximum_causal);
    maximum_recovery = std::max(maximum_recovery,arm.recovery);
  }
  std::ofstream csv(directory/
      "ftd_0721_derived_interaction_graph_transaction_v1.csv");
  csv << "family,direction,polarity,translation,pass,graph_transitions,"
         "active_ticks,initial_internal_energy,final_internal_energy,"
         "recovery,max_root,max_energy,max_momentum,max_impulse,"
         "max_kinematic,max_causal\n" << std::setprecision(17);
  for (const auto& arm : arms)
    csv << arm.family << ',' << arm.direction << ',' << arm.polarity << ','
        << arm.translation << ',' << (arm.pass ? 1 : 0) << ','
        << arm.graph_transitions << ',' << arm.active_ticks << ','
        << arm.initial_internal_energy << ',' << arm.final_internal_energy
        << ',' << arm.recovery << ',' << arm.maximum_root << ','
        << arm.maximum_energy << ',' << arm.maximum_momentum << ','
        << arm.maximum_impulse << ',' << arm.maximum_kinematic << ','
        << arm.maximum_causal << '\n';
  std::ofstream json(directory/
      "ftd_0721_derived_interaction_graph_transaction_v1.json");
  json << std::setprecision(17)
       << "{\n  \"identifier\": \"FTD-0721\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"arm_count\": " << arms.size() << ",\n"
       << "  \"passed_arms\": " << passed << ",\n"
       << "  \"maximum_root_residual\": " << maximum_root << ",\n"
       << "  \"maximum_energy_residual\": " << maximum_energy << ",\n"
       << "  \"maximum_momentum_residual\": " << maximum_momentum << ",\n"
       << "  \"maximum_impulse_balance_residual\": " << maximum_impulse << ",\n"
       << "  \"maximum_kinematic_residual\": " << maximum_kinematic << ",\n"
       << "  \"maximum_causal_speed_excess\": " << maximum_causal << ",\n"
       << "  \"maximum_inverse_recovery\": " << maximum_recovery << ",\n"
       << "  \"maximum_scalar_history_spread\": " << history_spread << "\n}\n";
}

}  // namespace

int main() {
  DerivedInteractionGraphOptions options;
  std::vector<ArmResult> arms;
  const auto rays = directions();
  for (const auto& family : {std::string("scattering"),std::string("bound")})
    for (const auto& direction : rays)
      for (bool conjugate : {false,true})
        for (bool translated : {false,true})
          arms.push_back(run_arm(
              family,direction,conjugate,translated,options));
  const double history_spread = scalar_history_spread(arms);
  const bool analytic = rays.size() == 13
      && std::abs(ftd::eft::derived_interaction_potential(1.0,options)+0.01)
          < 1e-15
      && std::abs(ftd::eft::derived_interaction_potential(0.0,options)-0.27)
          < 1e-15
      && ftd::eft::derived_interaction_potential(1.5,options) == 0.0
      && ftd::eft::derived_interaction_potential_derivative(1.5,options)
          == 0.0;
  const bool pass = analytic && arms.size() == 104
      && std::all_of(arms.begin(),arms.end(),
          [](const ArmResult& arm) { return arm.pass; })
      && history_spread < 1e-12;
  const std::string verdict = pass
      ? "DERIVED_INTERACTION_GRAPH_REVERSIBLE_CAPTURE_REQUIRES_RESERVOIR"
      : "DERIVED_GRAPH_TRANSACTION_NUMERICALLY_UNRESOLVED";
  write_records(arms,verdict,history_spread);
  std::cout << "FTD-0721 " << verdict << " arms=" << arms.size()
            << " history_spread=" << std::setprecision(17)
            << history_spread << '\n';
  return pass ? 0 : 1;
}
