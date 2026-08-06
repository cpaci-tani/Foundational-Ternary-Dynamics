// FTD-0704: short reversible high-speed preflight for the selected dressed
// connected matter candidate. This is a disabled run-of-record campaign.

#define FTD_0639_EMBEDDED
#include "test_connected_block_analytic_dynamical_rest.cpp"
#undef FTD_0639_EMBEDDED

#include <numeric>

namespace {

constexpr char preflight_protocol_sha256[] =
    "E70EC4DA01504CA929A710482ACE6CCAEAB09075951505EF7F0ECD1D6B374E5E";
constexpr char preflight_source_sha256[] =
    "8A717BC9DFE3A43FB21A6B46EF723BD2649D5F1F5BC2174BBA6027D25550214F";
constexpr int preflight_volume = 33;
constexpr int preflight_ticks = 8;

struct PreflightSpec {
  double target_speed = 0.0;
  int sign = 1;
  std::string label;
};

struct PreflightTick {
  int tick = 0;
  int hops = 0;
  int multiplicity = 0;
  double axial_displacement = 0.0;
  double axial_increment = 0.0;
  double transverse_displacement = 0.0;
  double mean_axial_velocity = 0.0;
  double axial_momentum = 0.0;
  double shape = 0.0;
  double strain = 0.0;
  double field_energy = 0.0;
  double energy_drift = 0.0;
  double common = 0.0;
  double same_anchor_separation = INFINITY;
};

struct PreflightArm {
  PreflightSpec spec;
  bool initialized = false;
  bool forward = false;
  bool reverse = false;
  bool coherent = false;
  bool source_quality = false;
  int total_hops = 0;
  int maximum_multiplicity = 0;
  double minimum_separation = INFINITY;
  double maximum_shape = 0.0;
  double maximum_strain = 0.0;
  double maximum_transverse = 0.0;
  double maximum_energy_drift = 0.0;
  double maximum_common = 0.0;
  double recovery = INFINITY;
  double mean_speed = 0.0;
  double increment_cv = INFINITY;
  std::vector<PreflightTick> ticks;
};

struct PreflightSummary {
  bool parent = false;
  bool normalization = false;
  bool coverage = false;
  bool execution = false;
  bool coherence = false;
  bool source_quality = false;
  bool mirror = false;
  double beta = 0.0;
  double mirror_residual = INFINITY;
  std::string verdict = "DRESSED_MATTER_HIGH_SPEED_EXECUTION_INVALID";
  std::vector<PreflightArm> arms;
};

int preflight_wrap(int value, int size) {
  const int result = value % size;
  return result < 0 ? result + size : result;
}

ftd::eft::MatchedMatterPoint preflight_point_at(const Vec3& x, int size) {
  ftd::eft::MatchedMatterPoint point;
  const long long ax = std::llround(x.x);
  const long long ay = std::llround(x.y);
  const long long az = std::llround(x.z);
  point.anchor = {preflight_wrap(static_cast<int>(ax), size),
                  preflight_wrap(static_cast<int>(ay), size),
                  preflight_wrap(static_cast<int>(az), size)};
  point.remainder = {x.x - ax, x.y - ay, x.z - az};
  return point;
}

bool preflight_parent_fingerprint() {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0638/ftd_0638_connected_block_analytic_static_refinement_v1.json";
  std::ifstream input(path, std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)), {});
  return bytes.find(refinement_protocol_sha256) != std::string::npos
      && bytes.find("CONNECTED_BLOCK_ANALYTIC_STATIC_BASIN_CONSTRUCTIVE")
          != std::string::npos;
}

ftd::eft::ConnectedMooreBlockState preflight_reference() {
  const auto base = load_refined_state(0);
  const auto initialized = ftd::eft::initialize_connected_moore_block(
      preflight_volume, 2, 0, 0, 0.5, 1e-13, 4096);
  if (base.electric.L != L || !initialized.valid
      || base.constituents.size() != count) {
    return ftd::eft::ConnectedMooreBlockState{};
  }
  auto geometry = initialized.state;
  const Vec3 base_center = center(base);
  const Vec3 target_center{16.0, 16.0, 16.0};
  for (int particle = 0; particle < count; ++particle) {
    const Vec3 x = target_center
        + (position(base.constituents[particle]) - base_center);
    geometry.constituents[particle] = preflight_point_at(x, preflight_volume);
    geometry.constituents[particle].momentum = {};
  }
  geometry.charges = base.charges;
  const auto dressed = ftd::eft::redress_connected_moore_block_with_fibre_limit(
      geometry, 8, 1e-13, 4096);
  return dressed.valid ? dressed.state
                       : ftd::eft::ConnectedMooreBlockState{};
}

Vec3 preflight_mean_velocity(
    const ftd::eft::ConnectedMooreBlockState& state) {
  Vec3 result{};
  for (const auto& point : state.constituents) {
    result += ftd::eft::production_flat_velocity_from_momentum(point.momentum);
  }
  return state.constituents.empty()
      ? result : result * (1.0 / state.constituents.size());
}

Vec3 preflight_total_momentum(
    const ftd::eft::ConnectedMooreBlockState& state) {
  Vec3 result{};
  for (const auto& point : state.constituents) result += point.momentum;
  return result;
}

double preflight_shape(const ftd::eft::ConnectedMooreBlockState& initial,
                       const ftd::eft::ConnectedMooreBlockState& state) {
  const Vec3 c0 = center(initial);
  const Vec3 c1 = center(state);
  long double squared = 0.0L;
  for (std::size_t i = 0; i < initial.constituents.size(); ++i) {
    const Vec3 delta = (position(state.constituents[i]) - c1)
        - (position(initial.constituents[i]) - c0);
    squared += static_cast<long double>(delta.dot(delta));
  }
  return initial.constituents.empty() ? INFINITY
      : std::sqrt(static_cast<double>(squared / initial.constituents.size()));
}

std::pair<int, double> preflight_fibre(
    const ftd::eft::ConnectedMooreBlockState& state) {
  std::map<std::tuple<int, int, int>, std::vector<std::size_t>> fibres;
  for (std::size_t i = 0; i < state.constituents.size(); ++i) {
    const auto& a = state.constituents[i].anchor;
    fibres[{a.x, a.y, a.z}].push_back(i);
  }
  int maximum = 0;
  double separation = INFINITY;
  for (const auto& entry : fibres) {
    const auto& indices = entry.second;
    maximum = std::max(maximum, static_cast<int>(indices.size()));
    for (std::size_t a = 0; a < indices.size(); ++a) {
      for (std::size_t b = a + 1; b < indices.size(); ++b) {
        separation = std::min(separation,
            (position(state.constituents[indices[a]])
             - position(state.constituents[indices[b]])).mag());
      }
    }
  }
  return {maximum, separation};
}

double preflight_energy(const ftd::eft::ConnectedMooreBlockState& state,
                        double beta,
                        const ftd::eft::ConnectedMooreBlockOptions& options) {
  long double kinetic = 0.0L;
  for (const auto& point : state.constituents) {
    kinetic += ftd::eft::production_flat_energy_from_momentum(point.momentum);
  }
  return static_cast<double>(kinetic)
      + ftd::eft::connected_moore_block_binding_energy(state, options)
      + beta * ftd::eft::matched_modified_energy(
          state.electric, state.magnetic_half, options.wave_speed);
}

double preflight_field_energy(
    const ftd::eft::ConnectedMooreBlockState& state, double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  return beta * ftd::eft::matched_modified_energy(
      state.electric, state.magnetic_half, options.wave_speed);
}

PreflightArm run_preflight(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const PreflightSpec& spec, double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  PreflightArm arm;
  arm.spec = spec;
  auto initial = reference;
  const Vec3 assigned_velocity{spec.sign * spec.target_speed, 0.0, 0.0};
  const Vec3 assigned_momentum =
      ftd::eft::production_flat_momentum(assigned_velocity);
  for (auto& point : initial.constituents) point.momentum = assigned_momentum;
  arm.initialized = initial.electric.L == preflight_volume
      && initial.constituents.size() == count
      && std::accumulate(initial.charges.begin(), initial.charges.end(), 0) == 0
      && std::abs(ftd::eft::production_flat_velocity_from_momentum(
              assigned_momentum).x - assigned_velocity.x) <= 1e-14;
  if (!arm.initialized) return arm;

  const double energy0 = preflight_energy(initial, beta, options);
  const Vec3 center0 = center(initial);
  Vec3 prior_center = center0;
  auto state = initial;
  arm.forward = true;
  ftd::eft::ConnectedMooreBlockSolveCache forward_cache;
  for (int tick = 1; tick <= preflight_ticks && arm.forward; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state, options, &forward_cache);
    const double residual = common_residual(step);
    if (!step.valid || !step.common_action_gates_pass || residual > 1e-10) {
      arm.forward = false;
      break;
    }
    state = step.later;
    const Vec3 current_center = center(state);
    const Vec3 displacement = current_center - center0;
    const Vec3 increment = current_center - prior_center;
    prior_center = current_center;
    PreflightTick row;
    row.tick = tick;
    row.hops = step.site_hops;
    row.axial_displacement = spec.sign * displacement.x;
    row.axial_increment = spec.sign * increment.x;
    row.transverse_displacement = std::hypot(displacement.y, displacement.z);
    row.mean_axial_velocity = spec.sign * preflight_mean_velocity(state).x;
    row.axial_momentum = spec.sign * preflight_total_momentum(state).x;
    row.shape = preflight_shape(initial, state);
    row.strain = step.maximum_edge_strain;
    row.field_energy = preflight_field_energy(state, beta, options);
    row.energy_drift = std::abs(preflight_energy(state, beta, options) - energy0);
    row.common = residual;
    std::tie(row.multiplicity, row.same_anchor_separation) =
        preflight_fibre(state);
    arm.total_hops += row.hops;
    arm.maximum_multiplicity = std::max(arm.maximum_multiplicity,
                                        row.multiplicity);
    if (std::isfinite(row.same_anchor_separation)) {
      arm.minimum_separation = std::min(arm.minimum_separation,
                                        row.same_anchor_separation);
    }
    arm.maximum_shape = std::max(arm.maximum_shape, row.shape);
    arm.maximum_strain = std::max(arm.maximum_strain, row.strain);
    arm.maximum_transverse = std::max(arm.maximum_transverse,
                                      row.transverse_displacement);
    arm.maximum_energy_drift = std::max(arm.maximum_energy_drift,
                                        row.energy_drift);
    arm.maximum_common = std::max(arm.maximum_common, row.common);
    arm.ticks.push_back(row);
  }
  arm.forward = arm.forward && arm.ticks.size() == preflight_ticks;

  arm.reverse = arm.forward;
  ftd::eft::ConnectedMooreBlockSolveCache reverse_cache;
  for (int tick = preflight_ticks; tick >= 1 && arm.reverse; --tick) {
    const auto step = ftd::eft::solve_connected_moore_block_reverse(
        state, options, &reverse_cache);
    const double residual = common_residual(step);
    if (!step.valid || !step.common_action_gates_pass || residual > 1e-10) {
      arm.reverse = false;
      break;
    }
    state = step.earlier;
    arm.maximum_common = std::max(arm.maximum_common, residual);
    arm.maximum_energy_drift = std::max(
        arm.maximum_energy_drift,
        std::abs(preflight_energy(state, beta, options) - energy0));
  }
  if (arm.reverse) {
    arm.recovery = ftd::eft::connected_moore_block_state_max_difference(
        initial, state);
  }

  if (!arm.ticks.empty()) {
    arm.mean_speed = arm.ticks.back().axial_displacement / preflight_ticks;
    double mean_increment = 0.0;
    for (const auto& row : arm.ticks) mean_increment += row.axial_increment;
    mean_increment /= arm.ticks.size();
    double variance = 0.0;
    for (const auto& row : arm.ticks) {
      const double delta = row.axial_increment - mean_increment;
      variance += delta * delta;
    }
    variance /= arm.ticks.size();
    arm.increment_cv = mean_increment > 0.0
        ? std::sqrt(variance) / mean_increment : INFINITY;
  }

  const bool finite_separation = std::isfinite(arm.minimum_separation);
  arm.coherent = arm.initialized && arm.forward && arm.reverse
      && arm.maximum_multiplicity <= 8
      && (!finite_separation || arm.minimum_separation >= 0.9)
      && arm.maximum_shape <= 0.05
      && arm.maximum_strain <= 0.05
      && arm.maximum_transverse <= 1e-8
      && arm.maximum_energy_drift <= 1e-10
      && arm.maximum_common <= 1e-10
      && arm.recovery <= 1e-9;
  const bool increments_positive = std::all_of(
      arm.ticks.begin(), arm.ticks.end(),
      [](const PreflightTick& row) { return row.axial_increment > 0.0; });
  arm.source_quality = arm.coherent && arm.total_hops >= 16
      && increments_positive
      && std::abs(arm.mean_speed - spec.target_speed) <= 0.05
      && arm.increment_cv <= 0.15;
  return arm;
}

void evaluate_preflight(PreflightSummary& summary) {
  summary.coverage = summary.arms.size() == 6;
  summary.execution = summary.coverage && std::all_of(
      summary.arms.begin(), summary.arms.end(),
      [](const PreflightArm& arm) {
        return arm.initialized && arm.forward && arm.reverse;
      });
  summary.coherence = summary.execution && std::all_of(
      summary.arms.begin(), summary.arms.end(),
      [](const PreflightArm& arm) { return arm.coherent; });
  summary.source_quality = summary.coherence && std::all_of(
      summary.arms.begin(), summary.arms.end(),
      [](const PreflightArm& arm) { return arm.source_quality; });

  summary.mirror = summary.coherence;
  summary.mirror_residual = 0.0;
  for (double speed : {0.35, 0.45, 0.50}) {
    const PreflightArm* positive = nullptr;
    const PreflightArm* negative = nullptr;
    for (const auto& arm : summary.arms) {
      if (std::abs(arm.spec.target_speed - speed) > 1e-15) continue;
      if (arm.spec.sign > 0) positive = &arm;
      else negative = &arm;
    }
    if (!positive || !negative || positive->ticks.size() != preflight_ticks
        || negative->ticks.size() != preflight_ticks) {
      summary.mirror = false;
      continue;
    }
    for (int tick = 0; tick < preflight_ticks; ++tick) {
      const auto& p = positive->ticks[tick];
      const auto& n = negative->ticks[tick];
      summary.mirror_residual = std::max({
          summary.mirror_residual,
          std::abs(p.axial_displacement - n.axial_displacement),
          std::abs(p.axial_increment - n.axial_increment),
          std::abs(p.mean_axial_velocity - n.mean_axial_velocity),
          std::abs(p.axial_momentum - n.axial_momentum),
          std::abs(p.shape - n.shape),
          std::abs(p.strain - n.strain),
          std::abs(p.field_energy - n.field_energy)});
    }
  }
  summary.mirror = summary.mirror && summary.mirror_residual <= 1e-6;

  if (!summary.parent || !summary.normalization || !summary.execution) {
    summary.verdict = "DRESSED_MATTER_HIGH_SPEED_EXECUTION_INVALID";
  } else if (!summary.coherence) {
    summary.verdict = "DRESSED_MATTER_HIGH_SPEED_COHERENCE_CLOSED";
  } else if (!summary.source_quality || !summary.mirror) {
    summary.verdict = "DRESSED_MATTER_HIGH_SPEED_SOURCE_UNSTEADY";
  } else {
    summary.verdict = "DRESSED_MATTER_HIGH_SPEED_PREFLIGHT_CONSTRUCTIVE";
  }
}

void write_preflight(const PreflightSummary& summary) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results/ftd_0704";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory /
      "ftd_0704_connected_dressed_matter_high_speed_preflight_v1.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0704\",\n"
       << "  \"protocol_sha256\": \"" << preflight_protocol_sha256
       << "\",\n  \"source_sha256\": \"" << preflight_source_sha256
       << "\",\n  \"verdict\": \"" << summary.verdict
       << "\",\n  \"production_changed\": false,\n"
       << "  \"volume\": " << preflight_volume
       << ",\n  \"ticks_each_direction\": " << preflight_ticks
       << ",\n  \"coverage_pass\": " << summary.coverage
       << ",\n  \"execution_pass\": " << summary.execution
       << ",\n  \"coherence_pass\": " << summary.coherence
       << ",\n  \"source_quality_pass\": " << summary.source_quality
       << ",\n  \"mirror_pass\": " << summary.mirror
       << ",\n  \"mirror_residual\": " << summary.mirror_residual
       << "\n}\n";

  std::ofstream arms(directory /
      "ftd_0704_connected_dressed_matter_high_speed_preflight_arms_v1.csv");
  arms << "ftd_id,label,target_speed,sign,initialized,forward,reverse,coherent,"
          "source_quality,total_hops,max_multiplicity,min_separation,max_shape,"
          "max_strain,max_transverse,max_energy_drift,max_common,recovery,"
          "mean_speed,increment_cv\n";
  for (const auto& arm : summary.arms) {
    arms << std::setprecision(17) << "FTD-0704," << arm.spec.label << ','
         << arm.spec.target_speed << ',' << arm.spec.sign << ','
         << arm.initialized << ',' << arm.forward << ',' << arm.reverse << ','
         << arm.coherent << ',' << arm.source_quality << ',' << arm.total_hops
         << ',' << arm.maximum_multiplicity << ',' << arm.minimum_separation
         << ',' << arm.maximum_shape << ',' << arm.maximum_strain << ','
         << arm.maximum_transverse << ',' << arm.maximum_energy_drift << ','
         << arm.maximum_common << ',' << arm.recovery << ',' << arm.mean_speed
         << ',' << arm.increment_cv << '\n';
  }

  std::ofstream ticks(directory /
      "ftd_0704_connected_dressed_matter_high_speed_preflight_ticks_v1.csv");
  ticks << "ftd_id,label,tick,hops,multiplicity,axial_displacement,"
           "axial_increment,transverse_displacement,mean_axial_velocity,"
           "axial_momentum,shape,strain,field_energy,energy_drift,common,"
           "same_anchor_separation\n";
  for (const auto& arm : summary.arms) {
    for (const auto& row : arm.ticks) {
      ticks << std::setprecision(17) << "FTD-0704," << arm.spec.label << ','
            << row.tick << ',' << row.hops << ',' << row.multiplicity << ','
            << row.axial_displacement << ',' << row.axial_increment << ','
            << row.transverse_displacement << ',' << row.mean_axial_velocity
            << ',' << row.axial_momentum << ',' << row.shape << ','
            << row.strain << ',' << row.field_energy << ',' << row.energy_drift
            << ',' << row.common << ',' << row.same_anchor_separation << '\n';
    }
  }
}

}  // namespace

#ifdef FTD_0704_EMBEDDED
int ftd_0704_embedded_main() {
#else
int main() {
#endif
  PreflightSummary summary;
  summary.parent = preflight_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  summary.normalization = normalization.valid;
  summary.beta = normalization.mapped_field_work_coefficient;

  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  options.use_sparse_local_current = true;
  options.use_local_residual_evaluation = true;

  const auto reference = preflight_reference();
  if (summary.parent && summary.normalization
      && reference.electric.L == preflight_volume) {
    for (double speed : {0.35, 0.45, 0.50}) {
      for (int sign : {-1, 1}) {
        PreflightSpec spec;
        spec.target_speed = speed;
        spec.sign = sign;
        spec.label = std::string(sign > 0 ? "p" : "n")
            + std::to_string(speed);
        summary.arms.push_back(run_preflight(
            reference, spec, summary.beta, options));
        std::cout << "completed " << spec.label << std::endl;
      }
    }
  }
  evaluate_preflight(summary);
  write_preflight(summary);

  std::cout << std::setprecision(17)
            << "protocol_sha256=" << preflight_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "coverage=" << summary.coverage
            << " execution=" << summary.execution
            << " coherence=" << summary.coherence
            << " source=" << summary.source_quality
            << " mirror=" << summary.mirror
            << " mirror_residual=" << summary.mirror_residual << '\n';
  for (const auto& arm : summary.arms) {
    std::cout << arm.spec.label << " coherent=" << arm.coherent
              << " source=" << arm.source_quality
              << " mean_v=" << arm.mean_speed
              << " cv=" << arm.increment_cv
              << " hops=" << arm.total_hops
              << " shape=" << arm.maximum_shape
              << " strain=" << arm.maximum_strain
              << " energy=" << arm.maximum_energy_drift
              << " residual=" << arm.maximum_common
              << " recovery=" << arm.recovery << '\n';
  }
  return summary.verdict == "DRESSED_MATTER_HIGH_SPEED_EXECUTION_INVALID"
      ? 1 : 0;
}
