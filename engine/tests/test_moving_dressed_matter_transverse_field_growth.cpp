// FTD-0705: finite-horizon transverse-field growth from coherently moving
// selected dressed matter. Disabled run-of-record campaign.

#define FTD_0704_EMBEDDED
#include "test_connected_dressed_matter_high_speed_preflight.cpp"
#undef FTD_0704_EMBEDDED

#include "ftd/eft/component_aware_radial_field_profile.h"
#include "ftd/eft/matched_face_current_spectrum.h"
#include "ftd/eft/matched_symmetry_ray_spectrum.h"

#include <complex>

namespace {

constexpr char growth_protocol_sha256[] =
    "A60CF2A5E5EE0DFA6903B185D07CACEBDCD8F1D1E57AAC619D1AD6E49B6F18DE";
constexpr char growth_parent_protocol_sha256[] =
    "E70EC4DA01504CA929A710482ACE6CCAEAB09075951505EF7F0ECD1D6B374E5E";
constexpr int growth_volume = 65;
constexpr int growth_ticks = 24;
constexpr int growth_late_first = 9;

struct GrowthMode {
  std::string label;
  std::array<int, 3> mode{};
};

const std::array<GrowthMode, 4> growth_modes{{
    {"R45", {31, 9, 0}},
    {"R50", {24, 5, 0}},
    {"C45", {26, 0, 0}},
    {"C50", {22, 0, 0}}}};

struct GrowthModeTick {
  std::string label;
  std::array<int, 3> mode{};
  double omega = 0.0;
  double detuning = 0.0;
  std::complex<double> electric{};
  std::complex<double> current{};
  double field_transverse_power = 0.0;
  double current_transverse_power = 0.0;
  double current_transverse_fraction = 0.0;
  double field_projection_residual = 0.0;
  double current_projection_residual = 0.0;
  bool valid = false;
};

struct GrowthTick {
  int tick = 0;
  int hops = 0;
  int multiplicity = 0;
  double displacement = 0.0;
  double increment = 0.0;
  double transverse = 0.0;
  double mean_velocity = 0.0;
  double shape = 0.0;
  double strain = 0.0;
  double energy_drift = 0.0;
  double common = 0.0;
  double separation = INFINITY;
  double magnetic_far_fraction_r6 = NAN;
  bool radial_valid = true;
  std::vector<GrowthModeTick> modes;
};

struct GrowthFit {
  double speed = 0.0;
  std::string label;
  double slope = 0.0;
  double r_squared = -INFINITY;
  double amplitude_ratio = 0.0;
  double mean_current = 0.0;
  double response = 0.0;
};

struct GrowthArm {
  double target_speed = 0.0;
  bool initialized = false;
  bool forward = false;
  bool reverse = false;
  bool coherent = false;
  bool source_quality = false;
  bool observer = false;
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
  std::vector<GrowthTick> ticks;
  std::vector<GrowthFit> fits;
};

struct GrowthSummary {
  bool parent = false;
  bool normalization = false;
  bool coverage = false;
  bool execution = false;
  bool coherence = false;
  bool source_quality = false;
  bool observer = false;
  bool coupling = false;
  bool collinear = false;
  bool growth45 = false;
  bool growth50 = false;
  double beta = 0.0;
  double q45 = 0.0;
  double q50 = 0.0;
  double b45 = 0.0;
  double b50 = 0.0;
  double k45 = 0.0;
  double k50 = 0.0;
  std::string verdict = "MOVING_DRESSED_MATTER_FIELD_EXECUTION_INVALID";
  std::vector<GrowthArm> arms;
};

bool growth_parent_fingerprint() {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0704/ftd_0704_connected_dressed_matter_high_speed_preflight_v1.json";
  std::ifstream input(path, std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)), {});
  return bytes.find(growth_parent_protocol_sha256) != std::string::npos
      && bytes.find("DRESSED_MATTER_HIGH_SPEED_PREFLIGHT_CONSTRUCTIVE")
          != std::string::npos;
}

ftd::eft::ConnectedMooreBlockState growth_reference() {
  const auto base = load_refined_state(0);
  const auto initialized = ftd::eft::initialize_connected_moore_block(
      growth_volume, 2, 0, 0, 0.5, 1e-13, 4096);
  if (base.electric.L != L || !initialized.valid
      || base.constituents.size() != count) {
    return ftd::eft::ConnectedMooreBlockState{};
  }
  auto geometry = initialized.state;
  const Vec3 base_center = center(base);
  const Vec3 target_center{32.0, 32.0, 32.0};
  for (int particle = 0; particle < count; ++particle) {
    const Vec3 x = target_center
        + (position(base.constituents[particle]) - base_center);
    geometry.constituents[particle] = preflight_point_at(x, growth_volume);
    geometry.constituents[particle].momentum = {};
  }
  geometry.charges = base.charges;
  const auto dressed = ftd::eft::redress_connected_moore_block_with_fibre_limit(
      geometry, 8, 1e-13, 4096);
  return dressed.valid ? dressed.state
                       : ftd::eft::ConnectedMooreBlockState{};
}

double growth_omega(const std::array<double, 3>& lattice_wavevector) {
  double sum = 0.0;
  for (double component : lattice_wavevector) sum += component * component;
  return 2.0 * std::asin(0.5 * ftd::C_SPEED * std::sqrt(sum));
}

std::vector<ftd::eft::QuadraticCoatFaceEntry> growth_entries(
    const ftd::eft::ConnectedMooreBlockStepResult& step) {
  std::size_t count_entries = 0;
  for (const auto& segment : step.segments)
    count_entries += segment.sparse_current.size();
  std::vector<ftd::eft::QuadraticCoatFaceEntry> entries;
  entries.reserve(count_entries);
  for (const auto& segment : step.segments) {
    entries.insert(entries.end(), segment.sparse_current.begin(),
                   segment.sparse_current.end());
  }
  return entries;
}

template <typename ComplexVector>
std::complex<double> growth_polarize(
    const ComplexVector& value,
    const std::array<double, 3>& lattice_wavevector) {
  const double norm = std::hypot(lattice_wavevector[0],
                                 lattice_wavevector[1]);
  if (!(norm > 0.0)) return {};
  return lattice_wavevector[1] * value[0] / norm
      - lattice_wavevector[0] * value[1] / norm;
}

GrowthModeTick growth_observe_mode(
    const GrowthMode& requested, double target_speed, double displacement,
    const ftd::eft::ConnectedMooreBlockState& initial,
    const ftd::eft::ConnectedMooreBlockState& state,
    const std::vector<ftd::eft::QuadraticCoatFaceEntry>& entries) {
  GrowthModeTick row;
  row.label = requested.label;
  row.mode = requested.mode;
  const auto field = ftd::eft::observe_matched_wavevector_spectrum(
      initial.electric, initial.magnetic_half,
      state.electric, state.magnetic_half,
      requested.mode, ftd::C_SPEED);
  const std::array<double, 3> k{{
      2.0 * ftd::PI * requested.mode[0] / growth_volume,
      2.0 * ftd::PI * requested.mode[1] / growth_volume,
      2.0 * ftd::PI * requested.mode[2] / growth_volume}};
  const double volume = static_cast<double>(growth_volume) * growth_volume
      * growth_volume;
  const auto current = ftd::eft::observe_sparse_face_current_spectrum(
      growth_volume, entries, k, volume);
  row.omega = growth_omega(field.lattice_wavevector);
  row.detuning = std::abs(row.omega - target_speed * k[0]);
  const auto comoving = std::polar(1.0, k[0] * displacement);
  row.electric = growth_polarize(field.electric_transverse,
                                 field.lattice_wavevector) * comoving;
  row.current = growth_polarize(current.transverse,
                                current.lattice_wavevector) * comoving;
  row.field_transverse_power = field.transverse_power;
  row.current_transverse_power = current.transverse_power;
  row.current_transverse_fraction = current.transverse_fraction;
  row.field_projection_residual = std::max(
      field.electric_projection_residual,
      field.magnetic_projection_residual);
  row.current_projection_residual = std::max(
      current.projection_residual, current.power_partition_residual);
  row.valid = field.valid && current.valid
      && row.field_projection_residual <= 1e-12
      && row.current_projection_residual <= 1e-12;
  return row;
}

GrowthFit growth_fit(const GrowthArm& arm, const std::string& label) {
  GrowthFit fit;
  fit.speed = arm.target_speed;
  fit.label = label;
  std::vector<std::pair<double, std::complex<double>>> samples;
  double current_sum = 0.0;
  for (const auto& tick : arm.ticks) {
    if (tick.tick < growth_late_first) continue;
    const auto it = std::find_if(tick.modes.begin(), tick.modes.end(),
        [&](const GrowthModeTick& row) { return row.label == label; });
    if (it == tick.modes.end()) continue;
    samples.push_back({static_cast<double>(tick.tick), it->electric});
    current_sum += std::abs(it->current);
  }
  if (samples.size() != growth_ticks - growth_late_first + 1) return fit;
  double mean_t = 0.0;
  std::complex<double> mean_z{};
  for (const auto& sample : samples) {
    mean_t += sample.first;
    mean_z += sample.second;
  }
  mean_t /= samples.size();
  mean_z /= static_cast<double>(samples.size());
  double denominator = 0.0;
  std::complex<double> slope{};
  for (const auto& sample : samples) {
    const double dt = sample.first - mean_t;
    denominator += dt * dt;
    slope += dt * (sample.second - mean_z);
  }
  slope /= denominator;
  const std::complex<double> intercept = mean_z - slope * mean_t;
  double residual = 0.0;
  double total = 0.0;
  for (const auto& sample : samples) {
    residual += std::norm(sample.second
                          - intercept - slope * sample.first);
    total += std::norm(sample.second - mean_z);
  }
  fit.slope = std::abs(slope);
  fit.r_squared = total > 0.0 ? 1.0 - residual / total : 0.0;
  fit.amplitude_ratio = std::abs(samples.back().second)
      / std::max(1e-300, std::abs(samples.front().second));
  fit.mean_current = current_sum / samples.size();
  fit.response = fit.mean_current > 0.0
      ? fit.slope / fit.mean_current : 0.0;
  return fit;
}

GrowthArm run_growth(
    const ftd::eft::ConnectedMooreBlockState& reference,
    double target_speed, double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  GrowthArm arm;
  arm.target_speed = target_speed;
  auto initial = reference;
  const Vec3 assigned_momentum = ftd::eft::production_flat_momentum(
      {target_speed, 0.0, 0.0});
  for (auto& point : initial.constituents) point.momentum = assigned_momentum;
  arm.initialized = initial.electric.L == growth_volume
      && initial.constituents.size() == count
      && std::accumulate(initial.charges.begin(), initial.charges.end(), 0) == 0;
  if (!arm.initialized) return arm;

  const Vec3 center0 = center(initial);
  Vec3 prior_center = center0;
  const double energy0 = preflight_energy(initial, beta, options);
  auto state = initial;
  arm.forward = true;
  arm.observer = true;
  ftd::eft::ConnectedMooreBlockSolveCache forward_cache;
  for (int tick = 1; tick <= growth_ticks && arm.forward; ++tick) {
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
    GrowthTick row;
    row.tick = tick;
    row.hops = step.site_hops;
    row.displacement = displacement.x;
    row.increment = increment.x;
    row.transverse = std::hypot(displacement.y, displacement.z);
    row.mean_velocity = preflight_mean_velocity(state).x;
    row.shape = preflight_shape(initial, state);
    row.strain = step.maximum_edge_strain;
    row.energy_drift = std::abs(preflight_energy(state, beta, options) - energy0);
    row.common = residual;
    std::tie(row.multiplicity, row.separation) = preflight_fibre(state);
    const auto entries = growth_entries(step);
    for (const auto& mode : growth_modes) {
      row.modes.push_back(growth_observe_mode(
          mode, target_speed, displacement.x, initial, state, entries));
      arm.observer = arm.observer && row.modes.back().valid;
    }
    if (tick == 8 || tick == 16 || tick == 24) {
      const Vec3 origin{std::round(current_center.x),
                        std::round(current_center.y),
                        std::round(current_center.z)};
      const auto profile =
          ftd::eft::observe_component_aware_radial_field_profile(
              state.electric, initial.magnetic_half,
              state.electric, state.magnetic_half,
              origin, beta, options.wave_speed, 1e-12);
      row.radial_valid = profile.valid && profile.total_norm > 0.0
          && profile.cumulative_norm_by_doubled_radius.size() > 12;
      if (row.radial_valid) {
        row.magnetic_far_fraction_r6 = 1.0
            - profile.cumulative_norm_by_doubled_radius[12]
                / profile.total_norm;
      }
      arm.observer = arm.observer && row.radial_valid;
    }
    arm.total_hops += row.hops;
    arm.maximum_multiplicity = std::max(arm.maximum_multiplicity,
                                        row.multiplicity);
    if (std::isfinite(row.separation)) {
      arm.minimum_separation = std::min(arm.minimum_separation,
                                        row.separation);
    }
    arm.maximum_shape = std::max(arm.maximum_shape, row.shape);
    arm.maximum_strain = std::max(arm.maximum_strain, row.strain);
    arm.maximum_transverse = std::max(arm.maximum_transverse, row.transverse);
    arm.maximum_energy_drift = std::max(arm.maximum_energy_drift,
                                        row.energy_drift);
    arm.maximum_common = std::max(arm.maximum_common, row.common);
    arm.ticks.push_back(std::move(row));
  }
  arm.forward = arm.forward && arm.ticks.size() == growth_ticks;

  arm.reverse = arm.forward;
  ftd::eft::ConnectedMooreBlockSolveCache reverse_cache;
  for (int tick = growth_ticks; tick >= 1 && arm.reverse; --tick) {
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
    arm.mean_speed = arm.ticks.back().displacement / growth_ticks;
    double mean = 0.0;
    for (const auto& row : arm.ticks) mean += row.increment;
    mean /= arm.ticks.size();
    double variance = 0.0;
    for (const auto& row : arm.ticks) {
      const double delta = row.increment - mean;
      variance += delta * delta;
    }
    variance /= arm.ticks.size();
    arm.increment_cv = mean > 0.0 ? std::sqrt(variance) / mean : INFINITY;
  }
  arm.coherent = arm.initialized && arm.forward && arm.reverse
      && arm.maximum_multiplicity <= 8
      && (!std::isfinite(arm.minimum_separation)
          || arm.minimum_separation >= 0.9)
      && arm.maximum_shape <= 0.05
      && arm.maximum_strain <= 0.05
      && arm.maximum_transverse <= 1e-8
      && arm.maximum_energy_drift <= 1e-10
      && arm.maximum_common <= 1e-10
      && arm.recovery <= 1e-9;
  const bool positive = std::all_of(
      arm.ticks.begin(), arm.ticks.end(),
      [](const GrowthTick& row) { return row.increment > 0.0; });
  arm.source_quality = arm.coherent && positive
      && std::abs(arm.mean_speed - target_speed) <= 0.05
      && arm.increment_cv <= 0.15;
  for (const auto& mode : growth_modes)
    arm.fits.push_back(growth_fit(arm, mode.label));
  return arm;
}

const GrowthArm* growth_arm(const GrowthSummary& summary, double speed) {
  for (const auto& arm : summary.arms)
    if (std::abs(arm.target_speed - speed) <= 1e-15) return &arm;
  return nullptr;
}

const GrowthFit* growth_find_fit(const GrowthArm* arm,
                                 const std::string& label) {
  if (!arm) return nullptr;
  for (const auto& fit : arm->fits) if (fit.label == label) return &fit;
  return nullptr;
}

const GrowthModeTick* growth_find_mode(const GrowthArm* arm, int tick,
                                       const std::string& label) {
  if (!arm || tick < 1 || tick > static_cast<int>(arm->ticks.size()))
    return nullptr;
  for (const auto& mode : arm->ticks[tick - 1].modes)
    if (mode.label == label) return &mode;
  return nullptr;
}

void evaluate_growth(GrowthSummary& summary) {
  summary.coverage = summary.arms.size() == 3;
  summary.execution = summary.coverage && std::all_of(
      summary.arms.begin(), summary.arms.end(),
      [](const GrowthArm& arm) {
        return arm.initialized && arm.forward && arm.reverse;
      });
  summary.coherence = summary.execution && std::all_of(
      summary.arms.begin(), summary.arms.end(),
      [](const GrowthArm& arm) { return arm.coherent; });
  summary.source_quality = summary.coherence && std::all_of(
      summary.arms.begin(), summary.arms.end(),
      [](const GrowthArm& arm) { return arm.source_quality; });
  summary.observer = summary.source_quality && std::all_of(
      summary.arms.begin(), summary.arms.end(),
      [](const GrowthArm& arm) { return arm.observer; });

  const auto* a35 = growth_arm(summary, 0.35);
  const auto* a45 = growth_arm(summary, 0.45);
  const auto* a50 = growth_arm(summary, 0.50);
  const auto* r45 = growth_find_fit(a45, "R45");
  const auto* r50 = growth_find_fit(a50, "R50");
  const auto* b45 = growth_find_fit(a35, "R45");
  const auto* b50 = growth_find_fit(a35, "R50");
  const auto* c45 = growth_find_fit(a45, "C45");
  const auto* c50 = growth_find_fit(a50, "C50");
  const auto* c45_current = growth_find_mode(a45, growth_ticks, "C45");
  const auto* c50_current = growth_find_mode(a50, growth_ticks, "C50");

  summary.coupling = r45 && r50 && r45->mean_current > 1e-12
      && r50->mean_current > 1e-12;
  summary.collinear = c45_current && c50_current
      && c45_current->current_transverse_fraction <= 1e-20
      && c50_current->current_transverse_fraction <= 1e-20;
  if (r45 && r50 && b45 && b50 && c45 && c50 && summary.coupling) {
    summary.q45 = r45->response;
    summary.q50 = r50->response;
    summary.b45 = b45->response;
    summary.b50 = b50->response;
    summary.k45 = c45->slope / r45->mean_current;
    summary.k50 = c50->slope / r50->mean_current;
    summary.growth45 = r45->r_squared >= 0.80
        && r45->amplitude_ratio >= 2.0
        && summary.q45 >= 5.0 * std::max(summary.b45, summary.k45);
    summary.growth50 = r50->r_squared >= 0.80
        && r50->amplitude_ratio >= 2.0
        && summary.q50 >= 5.0 * std::max(summary.b50, summary.k50);
  }

  if (!summary.parent || !summary.normalization || !summary.observer
      || !summary.collinear) {
    summary.verdict = "MOVING_DRESSED_MATTER_FIELD_EXECUTION_INVALID";
  } else if (!summary.coupling || !r45 || !r50
      || r45->slope <= 0.0 || r50->slope <= 0.0) {
    summary.verdict = "MOVING_DRESSED_MATTER_NO_TRANSVERSE_RESPONSE";
  } else if (summary.growth45 && summary.growth50) {
    summary.verdict = "MOVING_DRESSED_MATTER_RESONANT_TRANSVERSE_GROWTH";
  } else {
    summary.verdict =
        "MOVING_DRESSED_MATTER_DYNAMIC_TRANSVERSE_NO_THRESHOLD_SEPARATION";
  }
}

void write_growth(const GrowthSummary& summary) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results/ftd_0705";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory /
      "ftd_0705_moving_dressed_matter_transverse_field_growth_v1.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0705\",\n"
       << "  \"protocol_sha256\": \"" << growth_protocol_sha256
       << "\",\n  \"parent_protocol_sha256\": \""
       << growth_parent_protocol_sha256
       << "\",\n  \"verdict\": \"" << summary.verdict
       << "\",\n  \"production_changed\": false,\n"
       << "  \"volume\": " << growth_volume
       << ",\n  \"ticks_each_direction\": " << growth_ticks
       << ",\n  \"execution_pass\": " << summary.execution
       << ",\n  \"coherence_pass\": " << summary.coherence
       << ",\n  \"source_quality_pass\": " << summary.source_quality
       << ",\n  \"observer_pass\": " << summary.observer
       << ",\n  \"coupling_pass\": " << summary.coupling
       << ",\n  \"collinear_pass\": " << summary.collinear
       << ",\n  \"growth45_pass\": " << summary.growth45
       << ",\n  \"growth50_pass\": " << summary.growth50
       << ",\n  \"Q45\": " << summary.q45
       << ",\n  \"B45\": " << summary.b45
       << ",\n  \"K45\": " << summary.k45
       << ",\n  \"Q50\": " << summary.q50
       << ",\n  \"B50\": " << summary.b50
       << ",\n  \"K50\": " << summary.k50 << "\n}\n";

  std::ofstream fits(directory /
      "ftd_0705_moving_dressed_matter_transverse_field_growth_fits_v1.csv");
  fits << "ftd_id,speed,label,slope,r_squared,amplitude_ratio,mean_current,response\n";
  for (const auto& arm : summary.arms) for (const auto& fit : arm.fits) {
    fits << std::setprecision(17) << "FTD-0705," << fit.speed << ','
         << fit.label << ',' << fit.slope << ',' << fit.r_squared << ','
         << fit.amplitude_ratio << ',' << fit.mean_current << ','
         << fit.response << '\n';
  }

  std::ofstream arms(directory /
      "ftd_0705_moving_dressed_matter_transverse_field_growth_arms_v1.csv");
  arms << "ftd_id,speed,initialized,forward,reverse,coherent,source_quality,"
          "observer,total_hops,max_multiplicity,min_separation,max_shape,"
          "max_strain,max_transverse,max_energy_drift,max_common,recovery,"
          "mean_speed,increment_cv\n";
  for (const auto& arm : summary.arms) {
    arms << std::setprecision(17) << "FTD-0705," << arm.target_speed << ','
         << arm.initialized << ',' << arm.forward << ',' << arm.reverse << ','
         << arm.coherent << ',' << arm.source_quality << ',' << arm.observer
         << ',' << arm.total_hops << ',' << arm.maximum_multiplicity << ','
         << arm.minimum_separation << ',' << arm.maximum_shape << ','
         << arm.maximum_strain << ',' << arm.maximum_transverse << ','
         << arm.maximum_energy_drift << ',' << arm.maximum_common << ','
         << arm.recovery << ',' << arm.mean_speed << ',' << arm.increment_cv
         << '\n';
  }

  std::ofstream ticks(directory /
      "ftd_0705_moving_dressed_matter_transverse_field_growth_ticks_v1.csv");
  ticks << "ftd_id,speed,tick,hops,multiplicity,displacement,increment,"
           "transverse,mean_velocity,shape,strain,energy_drift,common,"
           "separation,magnetic_far_fraction_r6,label,nx,ny,nz,omega,detuning,"
           "electric_real,electric_imag,current_real,current_imag,"
           "field_transverse_power,current_transverse_power,"
           "current_transverse_fraction,field_projection_residual,"
           "current_projection_residual\n";
  for (const auto& arm : summary.arms) for (const auto& tick : arm.ticks) {
    for (const auto& mode : tick.modes) {
      ticks << std::setprecision(17) << "FTD-0705," << arm.target_speed << ','
            << tick.tick << ',' << tick.hops << ',' << tick.multiplicity << ','
            << tick.displacement << ',' << tick.increment << ','
            << tick.transverse << ',' << tick.mean_velocity << ','
            << tick.shape << ',' << tick.strain << ',' << tick.energy_drift
            << ',' << tick.common << ',' << tick.separation << ','
            << tick.magnetic_far_fraction_r6 << ',' << mode.label << ','
            << mode.mode[0] << ',' << mode.mode[1] << ',' << mode.mode[2] << ','
            << mode.omega << ',' << mode.detuning << ','
            << mode.electric.real() << ',' << mode.electric.imag() << ','
            << mode.current.real() << ',' << mode.current.imag() << ','
            << mode.field_transverse_power << ','
            << mode.current_transverse_power << ','
            << mode.current_transverse_fraction << ','
            << mode.field_projection_residual << ','
            << mode.current_projection_residual << '\n';
    }
  }
}

}  // namespace

int main() {
  GrowthSummary summary;
  summary.parent = growth_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  summary.normalization = normalization.valid;
  summary.beta = normalization.mapped_field_work_coefficient;
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  options.use_sparse_local_current = true;
  options.use_local_residual_evaluation = true;
  const auto reference = growth_reference();
  if (summary.parent && summary.normalization
      && reference.electric.L == growth_volume) {
    for (double speed : {0.35, 0.45, 0.50}) {
      summary.arms.push_back(run_growth(
          reference, speed, summary.beta, options));
      std::cout << "completed v=" << speed << std::endl;
    }
  }
  evaluate_growth(summary);
  write_growth(summary);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << growth_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "execution=" << summary.execution
            << " coherence=" << summary.coherence
            << " source=" << summary.source_quality
            << " observer=" << summary.observer
            << " coupling=" << summary.coupling
            << " collinear=" << summary.collinear << '\n'
            << "Q45=" << summary.q45 << " B45=" << summary.b45
            << " K45=" << summary.k45 << " pass45=" << summary.growth45
            << '\n'
            << "Q50=" << summary.q50 << " B50=" << summary.b50
            << " K50=" << summary.k50 << " pass50=" << summary.growth50
            << '\n';
  for (const auto& arm : summary.arms) {
    std::cout << "v=" << arm.target_speed
              << " mean=" << arm.mean_speed
              << " coherent=" << arm.coherent
              << " observer=" << arm.observer
              << " shape=" << arm.maximum_shape
              << " strain=" << arm.maximum_strain
              << " recovery=" << arm.recovery << '\n';
    for (const auto& fit : arm.fits) {
      std::cout << "  " << fit.label << " slope=" << fit.slope
                << " r2=" << fit.r_squared
                << " amp=" << fit.amplitude_ratio
                << " j=" << fit.mean_current
                << " response=" << fit.response << '\n';
    }
  }
  return summary.verdict == "MOVING_DRESSED_MATTER_FIELD_EXECUTION_INVALID"
      ? 1 : 0;
}
