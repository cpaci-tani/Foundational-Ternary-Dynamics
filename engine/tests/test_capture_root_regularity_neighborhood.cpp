/** FTD-0735: implicit-root regularity on captured matter histories. */

// Reuse the hash-locked parent/history helpers.  The FTD-0734 corner algebra
// is reproduced below because that runner itself embeds FTD-0732 and cannot be
// nested without colliding main-macro scopes.
#define main ftd_0732_embedded_main_for_0735
#include "test_captured_state_perturbation_survival.cpp"
#undef main

#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>

namespace {

constexpr char kRegularityProtocolSha256[] =
    "C8439AD7BCE95CF1EE530B28F741F9C1A11A7933478FD775FE4316C214C2A668";
constexpr double kMinimumSigma = 1e-3;
constexpr double kMaximumCondition = 1e4;
constexpr double kMaximumScaleDifference = 1e-5;
constexpr double kCornerImpulse0735 = 0.0006;

struct RegularityCornerSpec {
  int sigma_r = 0;
  int sigma_1 = 0;
  int sigma_2 = 0;
  int radial_side = 0;
  double field_scale = 1.0;
  std::string name;
};

double selected_potential0735(
    double d, const ConnectedMooreBlockOptions& options) {
  if (d >= options.compact_pair_cutoff_distance_squared) return 0.0;
  return -16.0 * options.compact_pair_well_depth
      * (d - 1.5) * (d - 1.5) * (d - 0.75);
}

double kinetic_above_rest0735(const ConnectedMooreBlockState& state) {
  long double kinetic = 0.0L;
  for (const auto& point : state.constituents) {
    const double total = std::sqrt(
        ftd::E_REST * ftd::E_REST
        + ftd::C_SPEED * ftd::C_SPEED * point.momentum.mag2());
    kinetic += total - ftd::E_REST;
  }
  return static_cast<double>(kinetic);
}

std::pair<double, double> isolate_selected_root0735(
    double kinetic, double left, double right,
    const ConnectedMooreBlockOptions& options) {
  auto value = [&](double d) {
    return kinetic + selected_potential0735(d, options);
  };
  double f_left = value(left);
  const double f_right = value(right);
  if (!(std::isfinite(f_left) && std::isfinite(f_right)
        && f_left * f_right < 0.0))
    return {NAN, INFINITY};
  for (int iteration = 0; iteration < 160; ++iteration) {
    const double midpoint = 0.5 * (left + right);
    const double f_midpoint = value(midpoint);
    if (f_left * f_midpoint <= 0.0) {
      right = midpoint;
    } else {
      left = midpoint;
      f_left = f_midpoint;
    }
  }
  const double root = 0.5 * (left + right);
  return {root, std::abs(value(root))};
}

std::string corner_sign_name0735(int sign) {
  return sign < 0 ? "m" : "p";
}

std::vector<RegularityCornerSpec> regularity_corner_specs() {
  std::vector<RegularityCornerSpec> result;
  result.reserve(32);
  for (int sigma_r : {-1, 1})
    for (int sigma_1 : {-1, 1})
      for (int sigma_2 : {-1, 1})
        for (int radial_side : {-1, 1})
          for (double field_scale : {0.95, 1.05}) {
            RegularityCornerSpec spec;
            spec.sigma_r = sigma_r;
            spec.sigma_1 = sigma_1;
            spec.sigma_2 = sigma_2;
            spec.radial_side = radial_side;
            spec.field_scale = field_scale;
            spec.name = "sr" + corner_sign_name0735(sigma_r)
                + "_s1" + corner_sign_name0735(sigma_1)
                + "_s2" + corner_sign_name0735(sigma_2)
                + (radial_side < 0 ? "_rin" : "_rout")
                + (field_scale < 1.0 ? "_fminus" : "_fplus");
            result.push_back(spec);
          }
  return result;
}

const RegularityCornerSpec* find_regularity_spec(
    const std::vector<RegularityCornerSpec>& specs,
    const std::string& name) {
  const auto found = std::find_if(
      specs.begin(), specs.end(), [&](const RegularityCornerSpec& spec) {
        return spec.name == name;
      });
  return found == specs.end() ? nullptr : &*found;
}

void add_scaled_face_residual0735(
    MatchedFaceFlux& output, const MatchedFaceFlux& parent,
    const MatchedFaceFlux& static_parent, double scale) {
  for (std::size_t i = 0; i < output.x.size(); ++i) {
    output.x[i] += scale * (parent.x[i] - static_parent.x[i]);
    output.y[i] += scale * (parent.y[i] - static_parent.y[i]);
    output.z[i] += scale * (parent.z[i] - static_parent.z[i]);
  }
}

void add_scaled_edge_residual0735(
    MatchedEdgeField& output, const MatchedEdgeField& parent,
    const MatchedEdgeField& static_parent, double scale) {
  for (std::size_t i = 0; i < output.x.size(); ++i) {
    output.x[i] += scale * (parent.x[i] - static_parent.x[i]);
    output.y[i] += scale * (parent.y[i] - static_parent.y[i]);
    output.z[i] += scale * (parent.z[i] - static_parent.z[i]);
  }
}

VariantState make_regularity_corner_state(
    const ParentState& parent, const Direction& direction,
    const RegularityCornerSpec& spec,
    const ConnectedMooreBlockOptions& options) {
  const int L = parent.state.electric.L;
  VariantState result(L);
  if (!parent.valid) return result;
  const auto static_parent = ftd::eft::redress_derived_compact_pair(
      parent.state, options, 1e-13, 4096);
  if (!static_parent.valid) return result;

  const Vec3 x0 = effective_position(parent.state.constituents[0]);
  const Vec3 x1 = effective_position(parent.state.constituents[1]);
  const Vec3 center = (x0 + x1) * 0.5;
  const Vec3 relative = x1 - x0;
  const double parent_d = relative.mag2();
  const Vec3 radial = relative * (1.0 / std::sqrt(parent_d));
  const Vec3 impulse =
      (radial * static_cast<double>(spec.sigma_r)
       + direction.tangent1 * static_cast<double>(spec.sigma_1)
       + direction.tangent2 * static_cast<double>(spec.sigma_2))
      * (kCornerImpulse0735 / std::sqrt(3.0));

  ConnectedMooreBlockState geometry = parent.state;
  geometry.constituents[0].momentum -= impulse;
  geometry.constituents[1].momentum += impulse;
  const double kinetic = kinetic_above_rest0735(geometry);
  if (!(kinetic > 0.0 && kinetic < options.compact_pair_well_depth))
    return result;
  const auto inner = isolate_selected_root0735(
      kinetic, 0.75, 1.0, options);
  const auto outer = isolate_selected_root0735(
      kinetic, 1.0, 1.5, options);
  if (!(std::isfinite(inner.first) && std::isfinite(outer.first)
        && inner.first < parent_d && parent_d < outer.first
        && std::max(inner.second, outer.second) <= 1e-12))
    return result;
  const double margin = std::min(
      parent_d - inner.first, outer.first - parent_d);
  const double target_d = parent_d
      + static_cast<double>(spec.radial_side) * 0.5 * margin;
  const double position_scale = std::sqrt(target_d / parent_d);
  geometry.constituents[0] = point_at(
      center - relative * (0.5 * position_scale),
      geometry.constituents[0].momentum, L);
  geometry.constituents[1] = point_at(
      center + relative * (0.5 * position_scale),
      geometry.constituents[1].momentum, L);

  const auto static_perturbed = ftd::eft::redress_derived_compact_pair(
      geometry, options, 1e-13, 4096);
  if (!static_perturbed.valid) return result;
  result.state = static_perturbed.state;
  add_scaled_face_residual0735(
      result.state.electric, parent.state.electric,
      static_parent.state.electric, spec.field_scale);
  add_scaled_edge_residual0735(
      result.state.magnetic_half, parent.state.magnetic_half,
      static_parent.state.magnetic_half, spec.field_scale);
  const auto density = fractional_density(result.state);
  if (density.empty()) return result;
  result.gauss_residual = ftd::eft::max_fractional_gauss_residual(
      result.state.electric, density);
  result.momentum_preservation =
      (total_momentum(result.state) - total_momentum(parent.state)).mag();
  result.maximum_speed = maximum_speed(result.state);
  result.separation = pair_separation(result.state);
  result.pair_energy = pair_internal_energy(result.state, options);
  result.valid = std::isfinite(result.gauss_residual)
      && result.gauss_residual <= 1e-12
      && result.momentum_preservation <= 1e-15
      && result.maximum_speed <= ftd::C_SPEED + 1e-12
      && graph_inside(result.separation, options)
      && result.pair_energy < -1e-6;
  return result;
}

struct LockedSelectorNames {
  const char* direction;
  const char* energy;
  const char* graph;
};

constexpr std::array<LockedSelectorNames, 3> kLockedSelectors{{
    {"0_0_1", "srp_s1p_s2m_rin_fminus",
                "srp_s1m_s2m_rin_fminus"},
    {"0_1_-1", "srp_s1m_s2m_rin_fminus",
                 "srp_s1m_s2p_rin_fminus"},
    {"1_1_1", "srp_s1m_s2m_rin_fminus",
                "srp_s1p_s2m_rin_fminus"}}};

struct RootRecord {
  std::string direction;
  std::string polarity;
  std::string variant;
  std::string phase;
  int tick = 0;
  bool step_valid = false;
  bool gates_pass = false;
  bool measured = false;
  int evaluations = 0;
  double sigma_min = 0.0;
  double sigma_max = 0.0;
  double condition = INFINITY;
  double scale_difference = INFINITY;
  double common_residual = INFINITY;
  double pair_energy = INFINITY;
  double graph_margin = -INFINITY;
};

struct HistoryRecord {
  std::string direction;
  std::string polarity;
  std::string variant;
  bool initialized = false;
  bool executed = false;
  bool capture_pass = false;
  bool regularity_pass = false;
  bool inverse_pass = false;
  bool survives = false;
  double minimum_sigma = INFINITY;
  double maximum_condition = 0.0;
  double maximum_scale_difference = 0.0;
  double minimum_energy_margin = INFINITY;
  double minimum_graph_margin = INFINITY;
  double inverse_recovery = INFINITY;
  std::vector<RootRecord> roots;
};

const LockedSelectorNames* locked_selector(const std::string& direction) {
  const auto found = std::find_if(
      kLockedSelectors.begin(), kLockedSelectors.end(),
      [&](const LockedSelectorNames& item) {
        return direction == item.direction;
      });
  return found == kLockedSelectors.end() ? nullptr : &*found;
}

VariantState selected_initial(
    const ParentState& parent, const Direction& direction,
    const std::string& variant, const ConnectedMooreBlockOptions& options) {
  if (variant == "center")
    return make_variant(parent, direction, "center", options);
  const auto specs = regularity_corner_specs();
  const auto spec = find_regularity_spec(specs, variant);
  if (spec == nullptr) return VariantState(parent.state.electric.L);
  return make_regularity_corner_state(parent, direction, *spec, options);
}

bool root_gate(const RootRecord& root) {
  return root.step_valid && root.gates_pass && root.measured
      && root.evaluations == 24 && root.sigma_min >= kMinimumSigma
      && root.condition <= kMaximumCondition
      && root.scale_difference <= kMaximumScaleDifference
      && root.common_residual <= kGate;
}

RootRecord root_record(
    const Direction& direction, bool conjugate, const std::string& variant,
    const std::string& phase, int tick,
    const ftd::eft::ConnectedMooreBlockStepResult& step,
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& options) {
  RootRecord result;
  result.direction = direction.label;
  result.polarity = conjugate ? "minus_plus" : "plus_minus";
  result.variant = variant;
  result.phase = phase;
  result.tick = tick;
  result.step_valid = step.valid;
  result.gates_pass = step.common_action_gates_pass;
  result.measured = step.solve.final_root_regularity_measured;
  result.evaluations = step.solve.regularity_residual_evaluations;
  result.sigma_min = step.solve.final_minimum_singular_value;
  result.sigma_max = step.solve.final_maximum_singular_value;
  result.condition = step.solve.final_condition_number;
  result.scale_difference =
      step.solve.regularity_scale_relative_difference;
  result.common_residual = maximum_step_residual(step);
  result.pair_energy = pair_internal_energy(state, options);
  result.graph_margin =
      std::sqrt(options.compact_pair_cutoff_distance_squared)
      - pair_separation(state);
  return result;
}

HistoryRecord run_history(
    const Direction& direction, bool conjugate, const std::string& variant,
    const ParentState& parent, ConnectedMooreBlockOptions options,
    double interaction_scale) {
  HistoryRecord result;
  result.direction = direction.label;
  result.polarity = conjugate ? "minus_plus" : "plus_minus";
  result.variant = variant;
  const auto initial = selected_initial(parent, direction, variant, options);
  result.initialized = initial.valid;
  if (!initial.valid) return result;
  options.measure_final_root_regularity = true;
  ConnectedMooreBlockState state = initial.state;
  const ConnectedMooreBlockState original = state;
  bool roots_valid = true;
  bool captured = graph_inside(pair_separation(state), options)
      && pair_internal_energy(state, options) < -1e-6
      && field_energy(state, options, interaction_scale) >= -1e-12;
  ConnectedMooreBlockSolveCache forward_cache;
  result.roots.reserve(2*kContinuationTicks);

  for (int tick = 1; tick <= kContinuationTicks; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state, options, &forward_cache);
    if (!step.valid) {
      result.roots.push_back(root_record(
          direction, conjugate, variant, "forward", tick, step, state,
          options));
      roots_valid = false;
      break;
    }
    state = step.later;
    auto record = root_record(
        direction, conjugate, variant, "forward", tick, step, state,
        options);
    roots_valid = roots_valid && root_gate(record);
    result.minimum_sigma = std::min(result.minimum_sigma, record.sigma_min);
    result.maximum_condition = std::max(
        result.maximum_condition, record.condition);
    result.maximum_scale_difference = std::max(
        result.maximum_scale_difference, record.scale_difference);
    result.minimum_energy_margin = std::min(
        result.minimum_energy_margin, -record.pair_energy / 0.01);
    result.minimum_graph_margin = std::min(
        result.minimum_graph_margin, record.graph_margin);
    captured = captured && record.pair_energy < -1e-6
        && record.graph_margin > 0.0
        && field_energy(state, options, interaction_scale) >= -1e-12;
    result.roots.push_back(std::move(record));
  }

  result.executed = result.roots.size()
      == static_cast<std::size_t>(kContinuationTicks);
  ConnectedMooreBlockState recovered = state;
  ConnectedMooreBlockSolveCache reverse_cache;
  if (result.executed)
    for (int tick = 1; tick <= kContinuationTicks; ++tick) {
      const auto step = ftd::eft::solve_connected_moore_block_reverse(
          recovered, options, &reverse_cache);
      if (!step.valid) {
        result.roots.push_back(root_record(
            direction, conjugate, variant, "reverse", tick, step,
            recovered, options));
        roots_valid = false;
        break;
      }
      recovered = step.earlier;
      auto record = root_record(
          direction, conjugate, variant, "reverse", tick, step, recovered,
          options);
      roots_valid = roots_valid && root_gate(record);
      result.minimum_sigma = std::min(result.minimum_sigma, record.sigma_min);
      result.maximum_condition = std::max(
          result.maximum_condition, record.condition);
      result.maximum_scale_difference = std::max(
          result.maximum_scale_difference, record.scale_difference);
      result.roots.push_back(std::move(record));
    }
  result.executed = result.executed && result.roots.size()
      == static_cast<std::size_t>(2*kContinuationTicks);
  result.inverse_recovery = result.executed
      ? ftd::eft::connected_moore_block_state_max_difference(
          original, recovered) : INFINITY;
  result.capture_pass = result.executed && captured;
  result.regularity_pass = result.executed && roots_valid
      && result.minimum_sigma >= kMinimumSigma
      && result.maximum_condition <= kMaximumCondition
      && result.maximum_scale_difference <= kMaximumScaleDifference;
  result.inverse_pass = result.executed && result.inverse_recovery <= 1e-8;
  result.survives = result.capture_pass && result.regularity_pass
      && result.inverse_pass;
  return result;
}

void write_records(const std::vector<HistoryRecord>& histories,
                   const std::string& verdict) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results" / "ftd_0735";
  std::filesystem::create_directories(directory);
  std::ofstream csv(directory /
      "ftd_0735_capture_root_regularity_neighborhood_v1.csv");
  csv << "direction,polarity,variant,phase,tick,step_valid,gates_pass,"
         "measured,evaluations,sigma_min,sigma_max,condition,"
         "scale_difference,common_residual,pair_energy,graph_margin\n"
      << std::setprecision(17);
  for (const auto& history : histories)
    for (const auto& root : history.roots)
      csv << root.direction << ',' << root.polarity << ',' << root.variant
          << ',' << root.phase << ',' << root.tick << ',' << root.step_valid
          << ',' << root.gates_pass << ',' << root.measured << ','
          << root.evaluations << ',' << root.sigma_min << ','
          << root.sigma_max << ',' << root.condition << ','
          << root.scale_difference << ',' << root.common_residual << ','
          << root.pair_energy << ',' << root.graph_margin << '\n';

  int survives = 0;
  double minimum_sigma = INFINITY;
  double maximum_condition = 0.0;
  double maximum_scale_difference = 0.0;
  double minimum_energy_margin = INFINITY;
  double minimum_graph_margin = INFINITY;
  double maximum_inverse = 0.0;
  for (const auto& history : histories) {
    survives += history.survives ? 1 : 0;
    minimum_sigma = std::min(minimum_sigma, history.minimum_sigma);
    maximum_condition = std::max(
        maximum_condition, history.maximum_condition);
    maximum_scale_difference = std::max(
        maximum_scale_difference, history.maximum_scale_difference);
    minimum_energy_margin = std::min(
        minimum_energy_margin, history.minimum_energy_margin);
    minimum_graph_margin = std::min(
        minimum_graph_margin, history.minimum_graph_margin);
    maximum_inverse = std::max(maximum_inverse, history.inverse_recovery);
  }
  std::ofstream json(directory /
      "ftd_0735_capture_root_regularity_neighborhood_v1.json");
  json << std::setprecision(17)
       << "{\n"
       << "  \"identifier\": \"FTD-0735\",\n"
       << "  \"protocol_sha256\": \"" << kRegularityProtocolSha256
       << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"history_count\": " << histories.size() << ",\n"
       << "  \"root_count\": "
       << 2*kContinuationTicks*histories.size() << ",\n"
       << "  \"survives\": " << survives << ",\n"
       << "  \"minimum_sigma\": " << minimum_sigma << ",\n"
       << "  \"maximum_condition\": " << maximum_condition << ",\n"
       << "  \"maximum_scale_difference\": "
       << maximum_scale_difference << ",\n"
       << "  \"minimum_energy_margin\": "
       << minimum_energy_margin << ",\n"
       << "  \"minimum_graph_margin\": "
       << minimum_graph_margin << ",\n"
       << "  \"maximum_inverse\": " << maximum_inverse << "\n"
       << "}\n";
}

}  // namespace

int main() {
  ConnectedMooreBlockOptions options;
  options.dt = 0.25;
  options.binding_law = ConnectedBindingLaw::DerivedCompactPair;
  options.compact_pair_well_depth = 0.01;
  options.compact_pair_cutoff_distance_squared = 1.5;
  options.allow_shared_anchor_chart = true;
  options.gate_tolerance = kGate;
  options.solve_tolerance = 2e-14;
  options.max_iterations = 384;
  options.use_sparse_local_current = true;
  options.use_local_residual_evaluation = true;
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const double interaction_scale =
      normalization.mapped_field_work_coefficient;

  std::vector<std::future<std::vector<HistoryRecord>>> jobs;
  for (const auto& direction : kDirections)
    for (bool conjugate : {false, true})
      jobs.push_back(std::async(std::launch::async, [=]() {
        std::vector<HistoryRecord> local;
        const auto names = locked_selector(direction.label);
        const auto parent = build_parent(33, direction, conjugate, options);
        if (names == nullptr || !parent.valid) {
          local.emplace_back();
          return local;
        }
        for (const std::string variant : {
                 std::string("center"), std::string(names->energy),
                 std::string(names->graph)})
          local.push_back(run_history(
              direction, conjugate, variant, parent, options,
              interaction_scale));
        return local;
      }));

  std::vector<HistoryRecord> histories;
  histories.reserve(18);
  std::size_t completed = 0;
  for (auto& job : jobs) {
    auto local = job.get();
    for (auto& history : local)
      histories.push_back(std::move(history));
    std::cerr << "FTD-0735 group " << ++completed << "/6 complete\n"
              << std::flush;
  }
  std::sort(histories.begin(), histories.end(),
      [](const HistoryRecord& lhs, const HistoryRecord& rhs) {
        return std::tie(lhs.direction, lhs.polarity, lhs.variant)
            < std::tie(rhs.direction, rhs.polarity, rhs.variant);
      });

  const bool infrastructure = normalization.valid && histories.size() == 18
      && std::all_of(histories.begin(), histories.end(),
          [](const HistoryRecord& history) {
            return history.initialized && history.executed
                && history.inverse_pass;
          });
  const bool capture = infrastructure && std::all_of(
      histories.begin(), histories.end(),
      [](const HistoryRecord& history) { return history.capture_pass; });
  const bool regularity = infrastructure && std::all_of(
      histories.begin(), histories.end(),
      [](const HistoryRecord& history) { return history.regularity_pass; });
  std::string verdict;
  if (!infrastructure)
    verdict = "CAPTURE_REGULARITY_TRANSACTION_UNRESOLVED";
  else if (!capture)
    verdict = "CAPTURE_FINITE_TIME_BOUNDARY_FOUND";
  else if (!regularity)
    verdict = "CAPTURE_ROOT_REGULARITY_NOT_ESTABLISHED";
  else
    verdict =
        "CAPTURE_FINITE_TIME_OPEN_NEIGHBORHOOD_NUMERICALLY_SUPPORTED";
  write_records(histories, verdict);

  double minimum_sigma = INFINITY;
  double maximum_condition = 0.0;
  double maximum_scale_difference = 0.0;
  double maximum_inverse = 0.0;
  int survives = 0;
  for (const auto& history : histories) {
    survives += history.survives ? 1 : 0;
    minimum_sigma = std::min(minimum_sigma, history.minimum_sigma);
    maximum_condition = std::max(
        maximum_condition, history.maximum_condition);
    maximum_scale_difference = std::max(
        maximum_scale_difference, history.maximum_scale_difference);
    maximum_inverse = std::max(maximum_inverse, history.inverse_recovery);
  }
  std::cout << "FTD-0735 " << verdict
            << " histories=" << survives << "/18"
            << " roots=" << 2*kContinuationTicks*histories.size()
            << " sigma_min=" << minimum_sigma
            << " condition_max=" << maximum_condition
            << " scale_difference_max=" << maximum_scale_difference
            << " inverse_max=" << maximum_inverse << '\n';
  return infrastructure ? 0 : 1;
}
