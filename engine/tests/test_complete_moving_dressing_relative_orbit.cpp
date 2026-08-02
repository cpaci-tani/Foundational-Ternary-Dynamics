// FTD-0706: test whether the selected static dressing plus a uniform v=1/2
// constituent boost is already a complete relative-periodic moving state.

#define FTD_0704_EMBEDDED
#include "test_connected_dressed_matter_high_speed_preflight.cpp"
#undef FTD_0704_EMBEDDED

namespace {

constexpr char orbit_protocol_sha256[] =
    "D07F8CE10B43D209A3C2EAA6AA9A316B12192CE2CF072612935E0F8451FE8FA7";
constexpr char orbit_parent_protocol_sha256[] =
    "A60CF2A5E5EE0DFA6903B185D07CACEBDCD8F1D1E57AAC619D1AD6E49B6F18DE";
constexpr int orbit_ticks = 2;

struct OrbitRun {
  bool initialized = false;
  bool forward = false;
  bool reverse = false;
  int total_hops = 0;
  double maximum_energy_drift = 0.0;
  double maximum_common_residual = 0.0;
  double inverse_residual = INFINITY;
  ftd::eft::ConnectedMooreBlockState initial;
  ftd::eft::ConnectedMooreBlockState final;
};

struct OrbitSummary {
  bool parent = false;
  bool normalization = false;
  bool execution = false;
  bool inverse = false;
  bool rest = false;
  bool covariance = false;
  double position_residual = INFINITY;
  double momentum_residual = INFINITY;
  double electric_residual = INFINITY;
  double magnetic_residual = INFINITY;
  double complete_residual = INFINITY;
  double rest_residual = INFINITY;
  double covariance_residual = INFINITY;
  OrbitRun moving;
  std::string verdict = "MOVING_DRESSING_RELATIVE_ORBIT_EXECUTION_INVALID";
};

bool orbit_parent_fingerprint() {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0705/ftd_0705_moving_dressed_matter_transverse_field_growth_v1.json";
  std::ifstream input(path, std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)), {});
  return bytes.find(orbit_parent_protocol_sha256) != std::string::npos
      && bytes.find(
          "MOVING_DRESSED_MATTER_DYNAMIC_TRANSVERSE_NO_THRESHOLD_SEPARATION")
          != std::string::npos;
}

int orbit_wrap(int value, int L) {
  const int wrapped = value % L;
  return wrapped < 0 ? wrapped + L : wrapped;
}

ftd::eft::ConnectedMooreBlockState translate_state(
    const ftd::eft::ConnectedMooreBlockState& source,
    const ftd::Coord& shift) {
  auto translated = source;
  const int L = source.electric.L;
  if (L <= 0 || source.magnetic_half.L != L) {
    return ftd::eft::ConnectedMooreBlockState{};
  }
  for (auto& point : translated.constituents) {
    point.anchor.x = orbit_wrap(point.anchor.x + shift.x, L);
    point.anchor.y = orbit_wrap(point.anchor.y + shift.y, L);
    point.anchor.z = orbit_wrap(point.anchor.z + shift.z, L);
  }
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int source_index = source.electric.index(x, y, z);
        const int target_index = translated.electric.index(
            x + shift.x, y + shift.y, z + shift.z);
        translated.electric.x[target_index] = source.electric.x[source_index];
        translated.electric.y[target_index] = source.electric.y[source_index];
        translated.electric.z[target_index] = source.electric.z[source_index];
        translated.magnetic_half.x[target_index] =
            source.magnetic_half.x[source_index];
        translated.magnetic_half.y[target_index] =
            source.magnetic_half.y[source_index];
        translated.magnetic_half.z[target_index] =
            source.magnetic_half.z[source_index];
      }
    }
  }
  return translated;
}

double orbit_maximum_component(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y), std::abs(value.z)});
}

double periodic_position_difference(
    const ftd::eft::ConnectedMooreBlockState& lhs,
    const ftd::eft::ConnectedMooreBlockState& rhs) {
  if (lhs.electric.L <= 0 || lhs.electric.L != rhs.electric.L
      || lhs.constituents.size() != rhs.constituents.size()) return INFINITY;
  const double L = lhs.electric.L;
  double residual = 0.0;
  for (std::size_t i = 0; i < lhs.constituents.size(); ++i) {
    Vec3 delta = position(lhs.constituents[i]) - position(rhs.constituents[i]);
    delta.x -= std::round(delta.x / L) * L;
    delta.y -= std::round(delta.y / L) * L;
    delta.z -= std::round(delta.z / L) * L;
    residual = std::max(residual, orbit_maximum_component(delta));
  }
  return residual;
}

double momentum_difference(
    const ftd::eft::ConnectedMooreBlockState& lhs,
    const ftd::eft::ConnectedMooreBlockState& rhs) {
  if (lhs.constituents.size() != rhs.constituents.size()) return INFINITY;
  double residual = 0.0;
  for (std::size_t i = 0; i < lhs.constituents.size(); ++i) {
    residual = std::max(residual, orbit_maximum_component(
        lhs.constituents[i].momentum - rhs.constituents[i].momentum));
  }
  return residual;
}

OrbitRun run_orbit(const ftd::eft::ConnectedMooreBlockState& supplied,
                   double beta,
                   const ftd::eft::ConnectedMooreBlockOptions& options) {
  OrbitRun run;
  run.initial = supplied;
  run.initialized = run.initial.electric.L == preflight_volume
      && run.initial.constituents.size() == count;
  if (!run.initialized) return run;
  const double energy0 = preflight_energy(run.initial, beta, options);
  auto state = run.initial;
  run.forward = true;
  ftd::eft::ConnectedMooreBlockSolveCache forward_cache;
  for (int tick = 0; tick < orbit_ticks && run.forward; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state, options, &forward_cache);
    const double common = common_residual(step);
    if (!step.valid || !step.common_action_gates_pass || common > 1e-10) {
      run.forward = false;
      break;
    }
    state = step.later;
    run.total_hops += step.site_hops;
    run.maximum_common_residual = std::max(
        run.maximum_common_residual, common);
    run.maximum_energy_drift = std::max(run.maximum_energy_drift,
        std::abs(preflight_energy(state, beta, options) - energy0));
  }
  run.final = state;
  run.reverse = run.forward;
  ftd::eft::ConnectedMooreBlockSolveCache reverse_cache;
  for (int tick = 0; tick < orbit_ticks && run.reverse; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_reverse(
        state, options, &reverse_cache);
    const double common = common_residual(step);
    if (!step.valid || !step.common_action_gates_pass || common > 1e-10) {
      run.reverse = false;
      break;
    }
    state = step.earlier;
    run.maximum_common_residual = std::max(
        run.maximum_common_residual, common);
    run.maximum_energy_drift = std::max(run.maximum_energy_drift,
        std::abs(preflight_energy(state, beta, options) - energy0));
  }
  if (run.reverse) {
    run.inverse_residual =
        ftd::eft::connected_moore_block_state_max_difference(run.initial, state);
  }
  return run;
}

void evaluate_orbit(OrbitSummary& summary,
                    const ftd::eft::ConnectedMooreBlockState& reference,
                    double beta,
                    const ftd::eft::ConnectedMooreBlockOptions& options) {
  auto moving = reference;
  const Vec3 momentum = ftd::eft::production_flat_momentum({0.5, 0.0, 0.0});
  for (auto& point : moving.constituents) point.momentum = momentum;
  summary.moving = run_orbit(moving, beta, options);

  if (summary.moving.forward) {
    const auto target = translate_state(moving, {1, 0, 0});
    summary.position_residual = periodic_position_difference(
        summary.moving.final, target);
    summary.momentum_residual = momentum_difference(summary.moving.final, target);
    summary.electric_residual = ftd::eft::matched_face_max_difference(
        summary.moving.final.electric, target.electric);
    summary.magnetic_residual = ftd::eft::matched_edge_max_difference(
        summary.moving.final.magnetic_half, target.magnetic_half);
    summary.complete_residual =
        ftd::eft::connected_moore_block_state_max_difference(
            summary.moving.final, target);
  }

  const auto rest_run = run_orbit(reference, beta, options);
  if (rest_run.forward && rest_run.reverse) {
    summary.rest_residual =
        ftd::eft::connected_moore_block_state_max_difference(
            reference, rest_run.final);
  }
  summary.rest = rest_run.forward && rest_run.reverse
      && rest_run.maximum_common_residual <= 1e-10
      && rest_run.maximum_energy_drift <= 1e-10
      && rest_run.inverse_residual <= 1e-9
      && summary.rest_residual <= 1e-9;

  const auto shifted_initial = translate_state(moving, {3, 0, 0});
  const auto shifted_run = run_orbit(shifted_initial, beta, options);
  if (summary.moving.forward && shifted_run.forward) {
    const auto shifted_final = translate_state(summary.moving.final, {3, 0, 0});
    summary.covariance_residual =
        ftd::eft::connected_moore_block_state_max_difference(
            shifted_run.final, shifted_final);
  }
  summary.covariance = shifted_run.forward && shifted_run.reverse
      && shifted_run.maximum_common_residual <= 1e-10
      && shifted_run.maximum_energy_drift <= 1e-10
      && shifted_run.inverse_residual <= 1e-9
      && summary.covariance_residual <= 1e-9;

  summary.inverse = summary.moving.reverse
      && summary.moving.inverse_residual <= 1e-9;
  summary.execution = summary.parent && summary.normalization
      && summary.moving.initialized && summary.moving.forward
      && summary.moving.reverse
      && summary.moving.maximum_common_residual <= 1e-10
      && summary.moving.maximum_energy_drift <= 1e-10
      && summary.inverse && summary.rest && summary.covariance;

  if (!summary.execution) {
    summary.verdict = "MOVING_DRESSING_RELATIVE_ORBIT_EXECUTION_INVALID";
  } else if (summary.complete_residual <= 1e-9
      && summary.position_residual <= 1e-9
      && summary.momentum_residual <= 1e-9
      && summary.electric_residual <= 1e-9
      && summary.magnetic_residual <= 1e-9) {
    summary.verdict = "COMPLETE_MOVING_DRESSING_RELATIVE_ORBIT_CANDIDATE";
  } else if (summary.position_residual <= 0.05
      && summary.momentum_residual <= 0.05
      && (summary.electric_residual > 1e-6
          || summary.magnetic_residual > 1e-6)) {
    summary.verdict = "CORE_TRANSLATES_WITHOUT_COMPLETE_MOVING_DRESSING";
  } else {
    summary.verdict = "NO_RELATIVE_ORBIT_FOR_STATIC_BOOST_PREPARATION";
  }
}

void write_orbit(const OrbitSummary& summary) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results/ftd_0706";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory /
      "ftd_0706_complete_moving_dressing_relative_orbit_v1.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0706\",\n"
       << "  \"protocol_sha256\": \"" << orbit_protocol_sha256 << "\",\n"
       << "  \"parent_protocol_sha256\": \""
       << orbit_parent_protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"volume\": " << preflight_volume << ",\n"
       << "  \"ticks\": " << orbit_ticks << ",\n"
       << "  \"target_translation_x\": 1,\n"
       << "  \"parent_pass\": " << summary.parent << ",\n"
       << "  \"normalization_pass\": " << summary.normalization << ",\n"
       << "  \"execution_pass\": " << summary.execution << ",\n"
       << "  \"inverse_pass\": " << summary.inverse << ",\n"
       << "  \"rest_pass\": " << summary.rest << ",\n"
       << "  \"covariance_pass\": " << summary.covariance << ",\n"
       << "  \"total_hops\": " << summary.moving.total_hops << ",\n"
       << "  \"position_residual\": " << summary.position_residual << ",\n"
       << "  \"momentum_residual\": " << summary.momentum_residual << ",\n"
       << "  \"electric_residual\": " << summary.electric_residual << ",\n"
       << "  \"magnetic_residual\": " << summary.magnetic_residual << ",\n"
       << "  \"complete_residual\": " << summary.complete_residual << ",\n"
       << "  \"maximum_energy_drift\": "
       << summary.moving.maximum_energy_drift << ",\n"
       << "  \"maximum_common_residual\": "
       << summary.moving.maximum_common_residual << ",\n"
       << "  \"inverse_residual\": " << summary.moving.inverse_residual << ",\n"
       << "  \"rest_residual\": " << summary.rest_residual << ",\n"
       << "  \"covariance_residual\": " << summary.covariance_residual
       << "\n}\n";

  std::ofstream csv(directory /
      "ftd_0706_complete_moving_dressing_relative_orbit_metrics_v1.csv");
  csv << "ftd_id,verdict,total_hops,position_residual,momentum_residual,"
         "electric_residual,magnetic_residual,complete_residual,"
         "max_energy_drift,max_common_residual,inverse_residual,"
         "rest_residual,covariance_residual\n";
  csv << std::setprecision(17) << "FTD-0706," << summary.verdict << ','
      << summary.moving.total_hops << ',' << summary.position_residual << ','
      << summary.momentum_residual << ',' << summary.electric_residual << ','
      << summary.magnetic_residual << ',' << summary.complete_residual << ','
      << summary.moving.maximum_energy_drift << ','
      << summary.moving.maximum_common_residual << ','
      << summary.moving.inverse_residual << ',' << summary.rest_residual << ','
      << summary.covariance_residual << '\n';
}

}  // namespace

int main() {
  OrbitSummary summary;
  summary.parent = orbit_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  summary.normalization = normalization.valid;

  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  options.use_sparse_local_current = true;
  options.use_local_residual_evaluation = true;

  const auto reference = preflight_reference();
  if (summary.parent && summary.normalization
      && reference.electric.L == preflight_volume) {
    evaluate_orbit(summary, reference,
        normalization.mapped_field_work_coefficient, options);
  }
  write_orbit(summary);

  std::cout << std::setprecision(17)
            << "protocol_sha256=" << orbit_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "execution=" << summary.execution
            << " inverse=" << summary.inverse
            << " rest=" << summary.rest
            << " covariance=" << summary.covariance << '\n'
            << "hops=" << summary.moving.total_hops
            << " position=" << summary.position_residual
            << " momentum=" << summary.momentum_residual
            << " electric=" << summary.electric_residual
            << " magnetic=" << summary.magnetic_residual
            << " complete=" << summary.complete_residual << '\n'
            << "energy=" << summary.moving.maximum_energy_drift
            << " common=" << summary.moving.maximum_common_residual
            << " inverse_residual=" << summary.moving.inverse_residual
            << " rest_residual=" << summary.rest_residual
            << " covariance_residual=" << summary.covariance_residual << '\n';
  return summary.verdict ==
      "MOVING_DRESSING_RELATIVE_ORBIT_EXECUTION_INVALID" ? 1 : 0;
}
