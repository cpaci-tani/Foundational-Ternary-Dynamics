// FTD-0654: out-of-sample mobility convergence at new speeds and horizon.

#include <chrono>
#include <stdexcept>

#define main ftd_0650_reference_main
#include "test_cell_measure_long_horizon_transport.cpp"
#undef main

namespace {

constexpr char protocol_sha256_v4[] =
    "10C77F2DF5DADA77E583145498ED4D33EF1E2F0A3EF31938BA5A883D301CBEA2";
constexpr double physical_horizon_v4 = 64.0;

struct SolverMetricsV4 {
  long long residual_evaluations = 0;
  long long jacobian_refreshes = 0;
  long long jacobian_reuses = 0;
  double wall_seconds = 0.0;
};

struct ArmV4 {
  Arm arm;
  SolverMetricsV4 solver;
};

struct EvaluationV4 {
  std::vector<ArmV4> results;
  bool coverage = false;
  bool execution = false;
  bool exact = false;
  bool coherence = false;
  bool zero = false;
  bool mirror = false;
  bool cubic = false;
  bool persistent = false;
  bool normalized = false;
  bool renormalized = false;
  double worst_action = 0.0;
  double worst_recovery = 0.0;
  double worst_strain = 0.0;
  double worst_zero = 0.0;
  double mirror_residual = 0.0;
  double cubic_residual = 0.0;
  std::map<std::string,std::map<int,double>> error;
  std::map<std::string,std::map<int,double>> span;
  std::map<std::string,std::map<int,double>> defect;
  double common_interval_low = NAN;
  double common_interval_high = NAN;
  std::string verdict;
};

std::vector<Spec> specs_v4() {
  const double inv_sqrt2 = 1.0/std::sqrt(2.0);
  const double inv_sqrt3 = 1.0/std::sqrt(3.0);
  const std::array<std::pair<std::string,Vec3>,3> directions{{
      {"100",{1,0,0}},
      {"110",{inv_sqrt2,inv_sqrt2,0}},
      {"111",{inv_sqrt3,inv_sqrt3,inv_sqrt3}}}};
  std::vector<Spec> result;
  for (int width : {2,3,4}) {
    for (double speed : {0.02,0.03})
      for (const auto& family : directions)
        result.push_back({"p_w"+std::to_string(width)+"_v"
            +(speed < 0.025 ? "02_" : "03_")+family.first,
            "primary",family.first,width,0,0,family.second,speed});
    result.push_back({"z_w"+std::to_string(width),"zero","zero",
        width,0,0,{1,0,0},0.0});
    result.push_back({"m_w"+std::to_string(width)+"_v03_100",
        "mirror","100",width,0,0,{1,0,0},-0.03});
    result.push_back({"c_w"+std::to_string(width)+"_o1",
        "cubic","100",width,1,1,{0,1,0},0.03});
    result.push_back({"c_w"+std::to_string(width)+"_o2",
        "cubic","100",width,2,2,{0,0,1},0.03});
  }
  return result;
}

template <typename Function>
ConnectedMooreBlockStepResult timed_step_v4(Function&& function,
                                            SolverMetricsV4& metrics) {
  const auto start = std::chrono::steady_clock::now();
  auto step = function();
  metrics.wall_seconds += std::chrono::duration<double>(
      std::chrono::steady_clock::now()-start).count();
  metrics.residual_evaluations += step.solve.residual_evaluations;
  metrics.jacobian_refreshes += step.solve.jacobian_refreshes;
  metrics.jacobian_reuses += step.solve.jacobian_reuses;
  return step;
}

ArmV4 run_arm_v4(const Spec& spec) {
  ArmV4 result;
  Arm& arm = result.arm;
  arm.spec = spec;
  arm.a = 2.0/spec.width;
  arm.ticks = 32*spec.width;
  arm.mass_scale = arm.a*arm.a*arm.a;
  arm.polarity_scale = arm.mass_scale;
  arm.binding_scale = arm.mass_scale;
  arm.field_scale = 1.0/arm.a;
  const int L = 8*spec.width+1;
  const auto initialized = ftd::eft::initialize_connected_moore_block(
      L,spec.width,spec.orientation,0,0.0,1e-13,16384);
  if (!initialized.valid) return result;
  auto initial = initialized.state;
  scale_field(initial,arm.polarity_scale);
  const Vec3 launch_momentum = ftd::eft::production_flat_momentum(
      spec.direction*spec.speed)*arm.mass_scale;
  for (auto& point : initial.constituents) point.momentum = launch_momentum;
  arm.constituent_count = static_cast<int>(initial.constituents.size());
  arm.rest_energy = arm.constituent_count*arm.mass_scale*ftd::E_REST;
  arm.inertial_mass = arm.constituent_count*arm.mass_scale*ftd::M_INERTIAL;
  arm.integrated_positive = 0.5*arm.constituent_count*arm.polarity_scale;
  arm.initialized = arm.constituent_count
          == 2*spec.width*spec.width*spec.width
      && std::abs(arm.rest_energy-16*ftd::E_REST) <= 1e-13
      && std::abs(arm.inertial_mass-16*ftd::M_INERTIAL) <= 1e-13
      && std::abs(arm.integrated_positive-8.0) <= 1e-13;
  if (!arm.initialized) return result;

  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  options.constituent_mass_scale = arm.mass_scale;
  options.polarity_scale = arm.polarity_scale;
  options.binding_stiffness = arm.binding_scale;
  options.field_energy_scale = arm.field_scale;
  ftd::eft::ConnectedMooreBlockSolveCache forward_cache,reverse_cache;

  auto state = initial;
  arm.initial_hash = state_hash(initial);
  arm.initial_center = center(initial);
  arm.initial_matter_momentum = momentum(initial);
  double initial_energy = NAN;
  std::vector<Vec3> forward_centers;
  forward_centers.reserve(static_cast<std::size_t>(arm.ticks));
  arm.forward = true;
  for (int tick = 1; tick <= arm.ticks; ++tick) {
    const auto step = timed_step_v4([&]() {
      return ftd::eft::solve_connected_moore_block_forward(
          state,options,&forward_cache);
    },result.solver);
    if (tick == 1) {
      initial_energy = total_energy(step,false);
      arm.initial_matter_momentum = step.matter_momentum_before;
      arm.initial_spline_momentum = step.spline_field_momentum_before;
    }
    const TickRecord row = tick_record(
        "forward",tick,step,step.later,true,initial_energy);
    arm.records.push_back(row);
    arm.maximum_action = std::max(arm.maximum_action,row.action);
    arm.maximum_causal = std::max(arm.maximum_causal,row.causal);
    arm.maximum_relative_edge_strain = std::max(
        arm.maximum_relative_edge_strain,row.relative_edge_strain);
    arm.maximum_energy_drift = std::max(
        arm.maximum_energy_drift,row.energy_drift);
    arm.maximum_anchor_multiplicity = std::max(
        arm.maximum_anchor_multiplicity,row.anchor_multiplicity);
    arm.total_hops += row.site_hops;
    arm.total_solve_iterations += row.solve_iterations;
    const bool pass = row.valid && step.common_action_gates_pass
        && row.graph_connected && row.graph_local
        && row.constituent_count == arm.constituent_count
        && row.anchor_multiplicity <= fibre_limit
        && row.action <= 1e-9 && row.causal <= 1e-12;
    if (!pass) { arm.forward = false; break; }
    state = step.later;
    forward_centers.push_back(row.center);
  }
  arm.forward = arm.forward
      && forward_centers.size() == static_cast<std::size_t>(arm.ticks);
  if (!arm.forward) return result;

  arm.final_state = state;
  arm.final_hash = state_hash(state);
  arm.final_center = center(state);
  const auto& last = arm.records.back();
  arm.final_matter_momentum = last.matter_momentum;
  arm.final_spline_momentum = last.spline_field_momentum;
  arm.final_energy = last.total;
  const Vec3 displacement = arm.final_center-arm.initial_center;
  const double projected = displacement.dot(spec.direction);
  const Vec3 transverse = displacement-spec.direction*projected;
  arm.parallel_displacement = arm.a*projected;
  arm.transverse_displacement = arm.a*transverse.mag();
  if (spec.speed != 0.0)
    arm.mobility = arm.parallel_displacement
        /(spec.speed*physical_horizon_v4);
  Vec3 previous = arm.initial_center;
  for (int window = 0; window < 4; ++window) {
    const int endpoint = (window+1)*arm.ticks/4;
    const Vec3 current = forward_centers[static_cast<std::size_t>(endpoint-1)];
    arm.window_advance[static_cast<std::size_t>(window)] =
        arm.a*(current-previous).dot(spec.direction);
    previous = current;
  }
  const Vec3 total_defect = arm.final_matter_momentum
      +arm.final_spline_momentum-arm.initial_matter_momentum
      -arm.initial_spline_momentum;
  arm.normalized_spline_defect = total_defect.mag()
      /std::max(arm.initial_matter_momentum.mag(),1e-15);
  arm.persistent = spec.speed > 0.0
      && std::all_of(arm.window_advance.begin(),arm.window_advance.end(),
          [](double value) { return value > 0.0; })
      && arm.mobility >= 0.50
      && arm.transverse_displacement
          /(std::abs(spec.speed)*physical_horizon_v4) <= 0.10;

  arm.reverse = true;
  for (int tick = arm.ticks; tick >= 1; --tick) {
    const auto step = timed_step_v4([&]() {
      return ftd::eft::solve_connected_moore_block_reverse(
          state,options,&reverse_cache);
    },result.solver);
    const TickRecord row = tick_record(
        "reverse",tick,step,step.earlier,false,initial_energy);
    arm.records.push_back(row);
    arm.maximum_action = std::max(arm.maximum_action,row.action);
    arm.maximum_causal = std::max(arm.maximum_causal,row.causal);
    arm.maximum_relative_edge_strain = std::max(
        arm.maximum_relative_edge_strain,row.relative_edge_strain);
    arm.maximum_energy_drift = std::max(
        arm.maximum_energy_drift,row.energy_drift);
    arm.maximum_anchor_multiplicity = std::max(
        arm.maximum_anchor_multiplicity,row.anchor_multiplicity);
    arm.total_hops += row.site_hops;
    arm.total_solve_iterations += row.solve_iterations;
    const bool pass = row.valid && step.common_action_gates_pass
        && row.graph_connected && row.graph_local
        && row.constituent_count == arm.constituent_count
        && row.anchor_multiplicity <= fibre_limit
        && row.action <= 1e-9 && row.causal <= 1e-12;
    if (!pass) { arm.reverse = false; break; }
    state = step.earlier;
  }
  arm.reverse = arm.reverse
      && arm.records.size() == static_cast<std::size_t>(2*arm.ticks);
  if (arm.reverse) {
    arm.recovery = ftd::eft::connected_moore_block_state_max_difference(
        initial,state);
    arm.recovered_hash = state_hash(state);
  }
  arm.exact = arm.forward && arm.reverse
      && arm.maximum_action <= 1e-9 && arm.maximum_causal <= 1e-12
      && arm.recovery <= 1e-7;
  arm.coherent = arm.exact
      && arm.maximum_relative_edge_strain <= 0.10
      && arm.maximum_anchor_multiplicity <= fibre_limit;
  return result;
}

const Arm* find_v4(const EvaluationV4& evaluation, int width,
                   const std::string& kind, const std::string& family,
                   double speed, int maps = 0) {
  for (const auto& result : evaluation.results) {
    const auto& spec = result.arm.spec;
    if (spec.width == width && spec.kind == kind && spec.family == family
        && std::abs(spec.speed-speed) <= 1e-14
        && spec.rotation_maps == maps) return &result.arm;
  }
  return nullptr;
}

void evaluate_v4(EvaluationV4& evaluation) {
  evaluation.coverage = evaluation.results.size() == 30;
  evaluation.execution = evaluation.coverage;
  evaluation.exact = evaluation.coverage;
  evaluation.coherence = evaluation.coverage;
  for (const auto& result : evaluation.results) {
    const auto& arm = result.arm;
    evaluation.execution = evaluation.execution && arm.initialized
        && arm.forward && arm.reverse
        && arm.records.size() == static_cast<std::size_t>(2*arm.ticks);
    evaluation.exact = evaluation.exact && arm.exact;
    evaluation.coherence = evaluation.coherence && arm.coherent;
    evaluation.worst_action = std::max(evaluation.worst_action,arm.maximum_action);
    evaluation.worst_strain = std::max(
        evaluation.worst_strain,arm.maximum_relative_edge_strain);
    if (std::isfinite(arm.recovery))
      evaluation.worst_recovery = std::max(evaluation.worst_recovery,arm.recovery);
  }

  evaluation.zero = evaluation.execution;
  evaluation.mirror = evaluation.execution;
  evaluation.cubic = evaluation.execution;
  for (int width : {2,3,4}) {
    const Arm* zero = find_v4(evaluation,width,"zero","zero",0.0);
    if (!zero) { evaluation.zero = false; continue; }
    evaluation.worst_zero = std::max(evaluation.worst_zero,
        zero->a*(zero->final_center-zero->initial_center).mag());
    const Arm* positive = find_v4(evaluation,width,"primary","100",0.03);
    const Arm* negative = find_v4(evaluation,width,"mirror","100",-0.03);
    if (!positive || !negative) { evaluation.mirror = false; continue; }
    const Vec3 dp = (positive->final_center-positive->initial_center)*positive->a;
    const Vec3 dn = (negative->final_center-negative->initial_center)*negative->a;
    evaluation.mirror_residual = std::max({evaluation.mirror_residual,
        (dp+dn).mag(),
        (positive->final_matter_momentum+negative->final_matter_momentum).mag(),
        std::abs(positive->final_energy-negative->final_energy),
        std::abs(positive->recovery-negative->recovery)});
    const auto base_rows = forward_records(*positive);
    for (int maps : {1,2}) {
      const Arm* rotated = find_v4(
          evaluation,width,"cubic","100",0.03,maps);
      if (!rotated) { evaluation.cubic = false; continue; }
      const auto rows = forward_records(*rotated);
      if (rows.size() != base_rows.size()) {
        evaluation.cubic = false;
        continue;
      }
      for (std::size_t tick = 0; tick < rows.size(); ++tick) {
        const auto& lhs = *base_rows[tick];
        const auto& rhs = *rows[tick];
        evaluation.cubic_residual = std::max({evaluation.cubic_residual,
            max_component(cycle(lhs.center-positive->initial_center,maps)
                -(rhs.center-rotated->initial_center)),
            max_component(cycle(lhs.matter_momentum,maps)-rhs.matter_momentum),
            max_component(cycle(lhs.spline_field_momentum,maps)
                -rhs.spline_field_momentum),
            std::abs(lhs.total-rhs.total),
            std::abs(lhs.relative_edge_strain-rhs.relative_edge_strain),
            std::abs(lhs.action-rhs.action)});
        if (lhs.site_hops != rhs.site_hops) evaluation.cubic = false;
      }
      evaluation.cubic_residual = std::max({evaluation.cubic_residual,
          rotated_state_residual(positive->final_state,rotated->final_state,maps),
          std::abs(positive->recovery-rotated->recovery)});
    }
  }
  evaluation.zero = evaluation.zero && evaluation.worst_zero <= 1e-6;
  evaluation.mirror = evaluation.mirror && evaluation.mirror_residual <= 1e-6;
  evaluation.cubic = evaluation.cubic && evaluation.cubic_residual <= 1e-6;

  evaluation.persistent = evaluation.execution;
  for (const std::string speed_name : {"02","03"}) {
    const double speed = speed_name == "02" ? 0.02 : 0.03;
    for (int width : {2,3,4}) {
      double minimum = INFINITY,maximum = -INFINITY,max_error = 0.0,max_defect = 0.0;
      for (const std::string family : {"100","110","111"}) {
        const Arm* arm = find_v4(evaluation,width,"primary",family,speed);
        if (!arm) { evaluation.persistent = false; continue; }
        evaluation.persistent = evaluation.persistent && arm->persistent;
        minimum = std::min(minimum,arm->mobility);
        maximum = std::max(maximum,arm->mobility);
        max_error = std::max(max_error,std::abs(arm->mobility-1.0));
        max_defect = std::max(max_defect,arm->normalized_spline_defect);
      }
      evaluation.error[speed_name][width] = max_error;
      evaluation.span[speed_name][width] = maximum-minimum;
      evaluation.defect[speed_name][width] = max_defect;
    }
  }

  bool target = evaluation.persistent;
  bool shrinking_span = evaluation.persistent;
  for (const std::string speed_name : {"02","03"}) {
    target = target
        && evaluation.error[speed_name][4] < evaluation.error[speed_name][3]
        && evaluation.error[speed_name][3] < evaluation.error[speed_name][2]
        && evaluation.span[speed_name][4] < evaluation.span[speed_name][3]
        && evaluation.span[speed_name][3] < evaluation.span[speed_name][2]
        && evaluation.defect[speed_name][4] < evaluation.defect[speed_name][2];
    shrinking_span = shrinking_span
        && evaluation.span[speed_name][4] < evaluation.span[speed_name][3]
        && evaluation.span[speed_name][3] < evaluation.span[speed_name][2];
  }
  evaluation.normalized = target;

  double low = -INFINITY,high = INFINITY;
  for (double speed : {0.02,0.03}) {
    double minimum = INFINITY,maximum = -INFINITY;
    for (const std::string family : {"100","110","111"}) {
      const Arm* arm = find_v4(evaluation,4,"primary",family,speed);
      if (!arm) continue;
      minimum = std::min(minimum,arm->mobility);
      maximum = std::max(maximum,arm->mobility);
    }
    low = std::max(low,minimum);
    high = std::min(high,maximum);
  }
  evaluation.common_interval_low = low;
  evaluation.common_interval_high = high;
  evaluation.renormalized = !evaluation.normalized && shrinking_span
      && std::isfinite(low) && std::isfinite(high) && low > 0.0 && low <= high;

  if (!evaluation.coverage || !evaluation.execution)
    evaluation.verdict = "OUT_OF_SAMPLE_MOBILITY_EXECUTION_INVALID";
  else if (!evaluation.exact || !evaluation.coherence || !evaluation.zero
           || !evaluation.mirror || !evaluation.cubic)
    evaluation.verdict = "OUT_OF_SAMPLE_MOBILITY_CLOSED";
  else if (evaluation.normalized)
    evaluation.verdict = "OUT_OF_SAMPLE_NORMALIZED_MOBILITY_CONSTRUCTIVE";
  else if (evaluation.renormalized)
    evaluation.verdict = "OUT_OF_SAMPLE_RENORMALIZED_MOBILITY_MIXED";
  else
    evaluation.verdict = "OUT_OF_SAMPLE_MOBILITY_MIXED";
}

std::filesystem::path output_dir_v4() {
  return std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results/ftd_0654";
}

void replace_v4(const std::filesystem::path& temporary,
                const std::filesystem::path& final) {
  std::error_code error;
  std::filesystem::remove(final,error);
  error.clear();
  std::filesystem::rename(temporary,final,error);
  if (error) throw std::runtime_error("checkpoint rename failed: "+error.message());
}

void checkpoint_v4(const ArmV4& result) {
  const auto directory = output_dir_v4()/"checkpoints";
  std::filesystem::create_directories(directory);
  const auto final = directory/(result.arm.spec.label+".csv");
  const auto temporary = directory/(result.arm.spec.label+".csv.tmp");
  {
    std::ofstream out(temporary);
    out << "ftd_id,protocol_sha256,label,phase,tick,state_hash,valid,action,"
           "causal,relative_edge_strain,center_x,center_y,center_z,matter_px,"
           "matter_py,matter_pz,spline_px,spline_py,spline_pz,total\n";
    for (const auto& row : result.arm.records)
      out << std::boolalpha << std::setprecision(17) << "FTD-0654,"
          << protocol_sha256_v4 << ',' << result.arm.spec.label << ','
          << row.phase << ',' << row.tick << ',' << row.state_hash << ','
          << row.valid << ',' << row.action << ',' << row.causal << ','
          << row.relative_edge_strain << ',' << row.center.x << ','
          << row.center.y << ',' << row.center.z << ',' << row.matter_momentum.x
          << ',' << row.matter_momentum.y << ',' << row.matter_momentum.z << ','
          << row.spline_field_momentum.x << ',' << row.spline_field_momentum.y
          << ',' << row.spline_field_momentum.z << ',' << row.total << '\n';
  }
  replace_v4(temporary,final);
}

void write_v4(const EvaluationV4& evaluation) {
  const auto directory = output_dir_v4();
  std::filesystem::create_directories(directory);
  std::ofstream json(directory/"ftd_0654_out_of_sample_mobility_v1.json");
  json << std::boolalpha << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0654\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256_v4 << "\",\n"
       << "  \"verdict\": \"" << evaluation.verdict << "\",\n"
       << "  \"arm_count\": " << evaluation.results.size() << ",\n"
       << "  \"coverage\": " << evaluation.coverage << ",\n"
       << "  \"execution\": " << evaluation.execution << ",\n"
       << "  \"exact\": " << evaluation.exact << ",\n"
       << "  \"coherence\": " << evaluation.coherence << ",\n"
       << "  \"zero\": " << evaluation.zero << ",\n"
       << "  \"mirror\": " << evaluation.mirror << ",\n"
       << "  \"cubic\": " << evaluation.cubic << ",\n"
       << "  \"all_primary_persistent\": " << evaluation.persistent << ",\n"
       << "  \"normalized_target\": " << evaluation.normalized << ",\n"
       << "  \"renormalized_common_candidate\": "
       << evaluation.renormalized << ",\n"
       << "  \"worst_action\": " << evaluation.worst_action << ",\n"
       << "  \"worst_recovery\": " << evaluation.worst_recovery << ",\n"
       << "  \"worst_strain\": " << evaluation.worst_strain << ",\n"
       << "  \"worst_zero\": " << evaluation.worst_zero << ",\n"
       << "  \"mirror_residual\": " << evaluation.mirror_residual << ",\n"
       << "  \"cubic_residual\": " << evaluation.cubic_residual << ",\n"
       << "  \"common_interval_low\": " << evaluation.common_interval_low << ",\n"
       << "  \"common_interval_high\": " << evaluation.common_interval_high << ",\n"
       << "  \"metrics\": {\n";
  for (const std::string speed : {"02","03"}) {
    json << "    \"" << speed << "\": {";
    for (int width : {2,3,4}) {
      if (width != 2) json << ',';
      json << "\"" << width << "\": {\"error\": "
           << evaluation.error.at(speed).at(width) << ", \"span\": "
           << evaluation.span.at(speed).at(width) << ", \"defect\": "
           << evaluation.defect.at(speed).at(width) << '}';
    }
    json << '}' << (speed == "02" ? ",\n" : "\n");
  }
  json << "  }\n}\n";

  std::ofstream arms(directory/"ftd_0654_out_of_sample_mobility_arms_v1.csv");
  arms << "ftd_id,label,kind,family,width,orientation,speed,initialized,forward,"
          "reverse,exact,coherent,persistent,ticks,mobility,parallel_displacement,"
          "transverse_displacement,normalized_spline_defect,max_action,max_strain,"
          "recovery,window1,window2,window3,window4,residual_evaluations,"
          "jacobian_refreshes,jacobian_reuses,wall_seconds\n";
  for (const auto& result : evaluation.results) {
    const auto& arm = result.arm;
    arms << std::boolalpha << std::setprecision(17) << "FTD-0654,"
         << arm.spec.label << ',' << arm.spec.kind << ',' << arm.spec.family
         << ',' << arm.spec.width << ',' << arm.spec.orientation << ','
         << arm.spec.speed << ',' << arm.initialized << ',' << arm.forward << ','
         << arm.reverse << ',' << arm.exact << ',' << arm.coherent << ','
         << arm.persistent << ',' << arm.ticks << ',' << arm.mobility << ','
         << arm.parallel_displacement << ',' << arm.transverse_displacement << ','
         << arm.normalized_spline_defect << ',' << arm.maximum_action << ','
         << arm.maximum_relative_edge_strain << ',' << arm.recovery << ','
         << arm.window_advance[0] << ',' << arm.window_advance[1] << ','
         << arm.window_advance[2] << ',' << arm.window_advance[3] << ','
         << result.solver.residual_evaluations << ','
         << result.solver.jacobian_refreshes << ','
         << result.solver.jacobian_reuses << ',' << result.solver.wall_seconds
         << '\n';
  }
}

}  // namespace

int main() {
  const auto all_specs = specs_v4();
  EvaluationV4 evaluation;
  constexpr std::size_t batch = 6;
  for (std::size_t start = 0; start < all_specs.size(); start += batch) {
    const std::size_t end = std::min(start+batch,all_specs.size());
    std::vector<std::future<ArmV4>> futures;
    for (std::size_t i = start; i < end; ++i)
      futures.push_back(std::async(std::launch::async,
          [spec=all_specs[i]]() { return run_arm_v4(spec); }));
    for (std::size_t i = start; i < end; ++i) {
      evaluation.results.push_back(futures[i-start].get());
      checkpoint_v4(evaluation.results.back());
      std::cout << "checkpointed " << all_specs[i].label << std::endl;
    }
  }
  evaluate_v4(evaluation);
  write_v4(evaluation);
  std::cout << std::boolalpha << std::setprecision(17)
            << "verdict=" << evaluation.verdict << '\n'
            << "exact=" << evaluation.exact
            << " coherence=" << evaluation.coherence
            << " persistent=" << evaluation.persistent
            << " normalized=" << evaluation.normalized
            << " renormalized=" << evaluation.renormalized << '\n'
            << "action=" << evaluation.worst_action
            << " recovery=" << evaluation.worst_recovery
            << " cubic=" << evaluation.cubic_residual << '\n';
  return evaluation.verdict == "OUT_OF_SAMPLE_MOBILITY_EXECUTION_INVALID"
      ? 1 : 0;
}
