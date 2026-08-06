// FTD-0652: checkpointed cached-solver execution of the FTD-0650 physics gate.

#include <chrono>
#include <stdexcept>

#define main ftd_0650_archived_main
#include "test_cell_measure_long_horizon_transport.cpp"
#undef main

namespace {

constexpr char protocol_sha256_v2[] =
    "1F6AB75BC11FD05D93E450029D020CDCA94B76CA7E1186A8197CC110AFFC829D";

struct SolverMetrics {
  long long residual_evaluations = 0;
  long long jacobian_refreshes = 0;
  long long jacobian_reuses = 0;
  double wall_seconds = 0.0;
};

struct ArmV2 {
  Arm arm;
  SolverMetrics solver;
};

template <typename Function>
ConnectedMooreBlockStepResult timed_cached_step(Function&& function,
                                                SolverMetrics& metrics) {
  const auto start = std::chrono::steady_clock::now();
  auto step = function();
  metrics.wall_seconds += std::chrono::duration<double>(
      std::chrono::steady_clock::now()-start).count();
  metrics.residual_evaluations += step.solve.residual_evaluations;
  metrics.jacobian_refreshes += step.solve.jacobian_refreshes;
  metrics.jacobian_reuses += step.solve.jacobian_reuses;
  return step;
}

ArmV2 run_arm_v2(const Spec& spec) {
  ArmV2 result;
  Arm& arm = result.arm;
  arm.spec = spec;
  arm.a = 2.0/spec.width;
  arm.ticks = 16*spec.width;
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
  const Vec3 launch_velocity = spec.direction*spec.speed;
  const Vec3 launch_momentum =
      ftd::eft::production_flat_momentum(launch_velocity)*arm.mass_scale;
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
    const auto step = timed_cached_step([&]() {
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
    arm.total_krylov_matvecs += row.krylov_matvecs;
    const bool pass = row.valid && step.common_action_gates_pass
        && row.graph_connected && row.graph_local
        && row.constituent_count == arm.constituent_count
        && row.anchor_multiplicity <= fibre_limit
        && row.action <= 1e-9 && row.causal <= 1e-12;
    if (!pass) {
      arm.forward = false;
      break;
    }
    state = step.later;
    forward_centers.push_back(row.center);
  }
  arm.forward = arm.forward
      && forward_centers.size() == static_cast<std::size_t>(arm.ticks);
  if (!arm.forward) return result;

  arm.final_state = state;
  arm.final_hash = state_hash(state);
  arm.final_center = center(state);
  const auto& last_forward = arm.records.back();
  arm.final_matter_momentum = last_forward.matter_momentum;
  arm.final_spline_momentum = last_forward.spline_field_momentum;
  arm.final_energy = last_forward.total;
  const Vec3 displacement = arm.final_center-arm.initial_center;
  const double projected_lattice = displacement.dot(spec.direction);
  const Vec3 transverse = displacement-spec.direction*projected_lattice;
  arm.parallel_displacement = arm.a*projected_lattice;
  arm.transverse_displacement = arm.a*transverse.mag();
  if (spec.speed != 0.0)
    arm.mobility = arm.parallel_displacement
        /(spec.speed*physical_horizon);
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
          /(std::abs(spec.speed)*physical_horizon) <= 0.10;

  arm.reverse = true;
  for (int tick = arm.ticks; tick >= 1; --tick) {
    const auto step = timed_cached_step([&]() {
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
    arm.total_krylov_matvecs += row.krylov_matvecs;
    const bool pass = row.valid && step.common_action_gates_pass
        && row.graph_connected && row.graph_local
        && row.constituent_count == arm.constituent_count
        && row.anchor_multiplicity <= fibre_limit
        && row.action <= 1e-9 && row.causal <= 1e-12;
    if (!pass) {
      arm.reverse = false;
      break;
    }
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

std::filesystem::path output_dir_v2() {
  return std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results/ftd_0652";
}

void replace_checkpoint(const std::filesystem::path& temporary,
                        const std::filesystem::path& final) {
  std::error_code error;
  std::filesystem::remove(final,error);
  error.clear();
  std::filesystem::rename(temporary,final,error);
  if (error) throw std::runtime_error("checkpoint rename failed: "+error.message());
}

void write_arm_checkpoint(const ArmV2& result) {
  const auto directory = output_dir_v2()/"checkpoints";
  std::filesystem::create_directories(directory);
  const auto json_final = directory/(result.arm.spec.label+".json");
  const auto json_temp = directory/(result.arm.spec.label+".json.tmp");
  {
    std::ofstream out(json_temp);
    out << std::boolalpha << std::setprecision(17)
        << "{\n  \"ftd_id\": \"FTD-0652\",\n"
        << "  \"protocol_sha256\": \"" << protocol_sha256_v2 << "\",\n"
        << "  \"label\": \"" << result.arm.spec.label << "\",\n"
        << "  \"record_count\": " << result.arm.records.size() << ",\n"
        << "  \"initialized\": " << result.arm.initialized << ",\n"
        << "  \"forward\": " << result.arm.forward << ",\n"
        << "  \"reverse\": " << result.arm.reverse << ",\n"
        << "  \"exact\": " << result.arm.exact << ",\n"
        << "  \"coherent\": " << result.arm.coherent << ",\n"
        << "  \"residual_evaluations\": "
        << result.solver.residual_evaluations << ",\n"
        << "  \"jacobian_refreshes\": "
        << result.solver.jacobian_refreshes << ",\n"
        << "  \"jacobian_reuses\": " << result.solver.jacobian_reuses << ",\n"
        << "  \"wall_seconds\": " << result.solver.wall_seconds << "\n}\n";
  }
  replace_checkpoint(json_temp,json_final);

  const auto csv_final = directory/(result.arm.spec.label+".csv");
  const auto csv_temp = directory/(result.arm.spec.label+".csv.tmp");
  {
    std::ofstream out(csv_temp);
    out << "ftd_id,label,phase,tick,state_hash,valid,graph_connected,graph_local,"
           "constituent_count,anchor_multiplicity,site_hops,solve_iterations,"
           "center_x,center_y,center_z,matter_px,matter_py,matter_pz,"
           "spline_px,spline_py,spline_pz,total,energy_drift,relative_edge_strain,"
           "action,root,force,continuity,gauss_before,gauss_after,total_energy,causal\n";
    for (const auto& row : result.arm.records)
      out << std::boolalpha << std::setprecision(17) << "FTD-0652,"
          << result.arm.spec.label << ',' << row.phase << ',' << row.tick << ','
          << row.state_hash << ',' << row.valid << ',' << row.graph_connected
          << ',' << row.graph_local << ',' << row.constituent_count << ','
          << row.anchor_multiplicity << ',' << row.site_hops << ','
          << row.solve_iterations << ',' << row.center.x << ',' << row.center.y
          << ',' << row.center.z << ',' << row.matter_momentum.x << ','
          << row.matter_momentum.y << ',' << row.matter_momentum.z << ','
          << row.spline_field_momentum.x << ',' << row.spline_field_momentum.y
          << ',' << row.spline_field_momentum.z << ',' << row.total << ','
          << row.energy_drift << ',' << row.relative_edge_strain << ','
          << row.action << ',' << row.root << ',' << row.force << ','
          << row.continuity << ',' << row.gauss_before << ',' << row.gauss_after
          << ',' << row.total_energy << ',' << row.causal << '\n';
  }
  replace_checkpoint(csv_temp,csv_final);
}

void write_final_v2(const Summary& summary,
                    const std::vector<ArmV2>& results) {
  const auto directory = output_dir_v2();
  std::filesystem::create_directories(directory);
  long long evaluations = 0,refreshes = 0,reuses = 0;
  double seconds = 0.0;
  for (const auto& result : results) {
    evaluations += result.solver.residual_evaluations;
    refreshes += result.solver.jacobian_refreshes;
    reuses += result.solver.jacobian_reuses;
    seconds += result.solver.wall_seconds;
  }
  std::ofstream json(directory/"ftd_0652_cell_measure_long_horizon_v2.json");
  json << std::boolalpha << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0652\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256_v2 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"arm_count\": " << summary.arms.size() << ",\n"
       << "  \"coverage_pass\": " << summary.coverage << ",\n"
       << "  \"execution_pass\": " << summary.execution << ",\n"
       << "  \"exact_pass\": " << summary.exact << ",\n"
       << "  \"coherence_pass\": " << summary.coherence << ",\n"
       << "  \"zero_pass\": " << summary.zero << ",\n"
       << "  \"mirror_pass\": " << summary.mirror << ",\n"
       << "  \"cubic_pass\": " << summary.cubic << ",\n"
       << "  \"transport_pass\": " << summary.transport << ",\n"
       << "  \"mobility_trend_pass\": " << summary.mobility_trend << ",\n"
       << "  \"anisotropy_trend_pass\": " << summary.anisotropy_trend << ",\n"
       << "  \"defect_trend_pass\": " << summary.defect_trend << ",\n"
       << "  \"resolution_pass\": " << summary.resolution << ",\n"
       << "  \"high_persistent_count\": " << summary.high_persistent << ",\n"
       << "  \"low_persistent_count\": " << summary.low_persistent << ",\n"
       << "  \"worst_action_residual\": " << summary.worst_action << ",\n"
       << "  \"worst_causal_excess\": " << summary.worst_causal << ",\n"
       << "  \"worst_relative_edge_strain\": " << summary.worst_strain << ",\n"
       << "  \"worst_recovery\": " << summary.worst_recovery << ",\n"
       << "  \"worst_zero_displacement\": " << summary.worst_zero << ",\n"
       << "  \"mirror_residual\": " << summary.mirror_residual << ",\n"
       << "  \"cubic_residual\": " << summary.cubic_residual << ",\n"
       << "  \"residual_evaluations\": " << evaluations << ",\n"
       << "  \"jacobian_refreshes\": " << refreshes << ",\n"
       << "  \"jacobian_reuses\": " << reuses << ",\n"
       << "  \"solver_wall_seconds_sum\": " << seconds << ",\n"
       << "  \"minimum_high_mobility\": {\"2\": "
       << summary.minimum_high_mobility.at(2) << ", \"3\": "
       << summary.minimum_high_mobility.at(3) << ", \"4\": "
       << summary.minimum_high_mobility.at(4) << "},\n"
       << "  \"high_mobility_span\": {\"2\": "
       << summary.high_mobility_span.at(2) << ", \"3\": "
       << summary.high_mobility_span.at(3) << ", \"4\": "
       << summary.high_mobility_span.at(4) << "},\n"
       << "  \"maximum_high_defect\": {\"2\": "
       << summary.maximum_high_defect.at(2) << ", \"3\": "
       << summary.maximum_high_defect.at(3) << ", \"4\": "
       << summary.maximum_high_defect.at(4) << "}\n}\n";

  std::ofstream arms(directory/"ftd_0652_cell_measure_long_horizon_arms_v2.csv");
  arms << "ftd_id,label,kind,family,width,orientation,speed,initialized,forward,"
          "reverse,exact,coherent,persistent,ticks,total_hops,max_strain,recovery,"
          "normalized_spline_defect,parallel_displacement,transverse_displacement,"
          "mobility,window1,window2,window3,window4,residual_evaluations,"
          "jacobian_refreshes,jacobian_reuses,wall_seconds\n";
  for (std::size_t i = 0; i < results.size(); ++i) {
    const auto& arm = results[i].arm;
    const auto& solver = results[i].solver;
    arms << std::boolalpha << std::setprecision(17) << "FTD-0652,"
         << arm.spec.label << ',' << arm.spec.kind << ',' << arm.spec.family
         << ',' << arm.spec.width << ',' << arm.spec.orientation << ','
         << arm.spec.speed << ',' << arm.initialized << ',' << arm.forward << ','
         << arm.reverse << ',' << arm.exact << ',' << arm.coherent << ','
         << arm.persistent << ',' << arm.ticks << ',' << arm.total_hops << ','
         << arm.maximum_relative_edge_strain << ',' << arm.recovery << ','
         << arm.normalized_spline_defect << ',' << arm.parallel_displacement
         << ',' << arm.transverse_displacement << ',' << arm.mobility << ','
         << arm.window_advance[0] << ',' << arm.window_advance[1] << ','
         << arm.window_advance[2] << ',' << arm.window_advance[3] << ','
         << solver.residual_evaluations << ',' << solver.jacobian_refreshes
         << ',' << solver.jacobian_reuses << ',' << solver.wall_seconds << '\n';
  }
}

}  // namespace

int main() {
  const auto all_specs = specs();
  std::vector<ArmV2> results;
  constexpr std::size_t batch = 6;
  for (std::size_t start = 0; start < all_specs.size(); start += batch) {
    const std::size_t end = std::min(start+batch,all_specs.size());
    std::vector<std::future<ArmV2>> futures;
    for (std::size_t i = start; i < end; ++i)
      futures.push_back(std::async(std::launch::async,
          [spec=all_specs[i]]() { return run_arm_v2(spec); }));
    for (std::size_t i = start; i < end; ++i) {
      results.push_back(futures[i-start].get());
      write_arm_checkpoint(results.back());
      std::cout << "checkpointed " << all_specs[i].label << std::endl;
    }
  }
  Summary summary;
  for (const auto& result : results) summary.arms.push_back(result.arm);
  evaluate(summary);
  write_final_v2(summary,results);
  std::cout << std::boolalpha << std::setprecision(17)
            << "protocol_sha256=" << protocol_sha256_v2 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "coverage=" << summary.coverage
            << " execution=" << summary.execution
            << " exact=" << summary.exact
            << " coherence=" << summary.coherence
            << " transport=" << summary.transport
            << " resolution=" << summary.resolution << '\n'
            << "action=" << summary.worst_action
            << " recovery=" << summary.worst_recovery
            << " mirror=" << summary.mirror_residual
            << " cubic=" << summary.cubic_residual << '\n';
  return summary.verdict == "CELL_MEASURE_LONG_HORIZON_EXECUTION_INVALID"
      ? 1 : 0;
}
