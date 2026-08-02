/** FTD-0734: energy-adapted mixed capture corners. */

// FTD-0732 is hash-locked.  Include its observer-only reconstruction and
// continuation helpers without editing that source-of-record runner.
#define main ftd_0732_embedded_main
#include "test_captured_state_perturbation_survival.cpp"
#undef main

#include <array>
#include <future>
#include <tuple>

namespace {

constexpr char kMixedProtocolSha256[] =
    "E2F4F92894526CBDE66B919D13AA22B739268739E3DF783977F08F8D7D2251C3";
constexpr double kCornerImpulse = 0.0006;

struct CornerSpec {
  int sigma_r = 0;
  int sigma_1 = 0;
  int sigma_2 = 0;
  int radial_side = 0;
  double field_scale = 1.0;
  std::string name;
};

struct CornerMetadata {
  bool registered = false;
  int sigma_r = 0;
  int sigma_1 = 0;
  int sigma_2 = 0;
  int radial_side = 0;
  double field_scale = 1.0;
  double kinetic = 0.0;
  double inner_d = 0.0;
  double outer_d = 0.0;
  double nearest_margin = 0.0;
  double target_d = 0.0;
  double root_residual = 0.0;
};

struct CornerState {
  VariantState initial;
  CornerMetadata metadata;
  explicit CornerState(int L) : initial(L) {}
};

struct CornerArm {
  ArmResult arm;
  CornerMetadata metadata;
};

double selected_potential(double d,
                          const ConnectedMooreBlockOptions& options) {
  if (d >= options.compact_pair_cutoff_distance_squared) return 0.0;
  return -16.0 * options.compact_pair_well_depth
      * (d - 1.5) * (d - 1.5) * (d - 0.75);
}

double kinetic_above_rest(const ConnectedMooreBlockState& state) {
  long double kinetic = 0.0L;
  for (const auto& point : state.constituents) {
    const double total = std::sqrt(
        ftd::E_REST * ftd::E_REST
        + ftd::C_SPEED * ftd::C_SPEED * point.momentum.mag2());
    kinetic += total - ftd::E_REST;
  }
  return static_cast<double>(kinetic);
}

std::pair<double, double> isolate_selected_root(
    double kinetic, double left, double right,
    const ConnectedMooreBlockOptions& options) {
  auto value = [&](double d) {
    return kinetic + selected_potential(d, options);
  };
  double f_left = value(left);
  double f_right = value(right);
  if (!(std::isfinite(f_left) && std::isfinite(f_right)
        && f_left * f_right < 0.0))
    return {NAN, INFINITY};
  for (int iteration = 0; iteration < 160; ++iteration) {
    const double midpoint = 0.5 * (left + right);
    const double f_midpoint = value(midpoint);
    if (f_left * f_midpoint <= 0.0) {
      right = midpoint;
      f_right = f_midpoint;
    } else {
      left = midpoint;
      f_left = f_midpoint;
    }
  }
  const double root = 0.5 * (left + right);
  return {root, std::abs(value(root))};
}

std::string sign_name(int sign) { return sign < 0 ? "m" : "p"; }

std::vector<CornerSpec> corner_specs() {
  std::vector<CornerSpec> result;
  result.reserve(32);
  for (int sigma_r : {-1, 1})
    for (int sigma_1 : {-1, 1})
      for (int sigma_2 : {-1, 1})
        for (int radial_side : {-1, 1})
          for (double field_scale : {0.95, 1.05}) {
            CornerSpec spec;
            spec.sigma_r = sigma_r;
            spec.sigma_1 = sigma_1;
            spec.sigma_2 = sigma_2;
            spec.radial_side = radial_side;
            spec.field_scale = field_scale;
            spec.name = "sr" + sign_name(sigma_r)
                + "_s1" + sign_name(sigma_1)
                + "_s2" + sign_name(sigma_2)
                + (radial_side < 0 ? "_rin" : "_rout")
                + (field_scale < 1.0 ? "_fminus" : "_fplus");
            result.push_back(spec);
          }
  return result;
}

void add_scaled_face_residual(MatchedFaceFlux& output,
                              const MatchedFaceFlux& parent,
                              const MatchedFaceFlux& static_parent,
                              double scale) {
  for (std::size_t i = 0; i < output.x.size(); ++i) {
    output.x[i] += scale * (parent.x[i] - static_parent.x[i]);
    output.y[i] += scale * (parent.y[i] - static_parent.y[i]);
    output.z[i] += scale * (parent.z[i] - static_parent.z[i]);
  }
}

void add_scaled_edge_residual(MatchedEdgeField& output,
                              const MatchedEdgeField& parent,
                              const MatchedEdgeField& static_parent,
                              double scale) {
  for (std::size_t i = 0; i < output.x.size(); ++i) {
    output.x[i] += scale * (parent.x[i] - static_parent.x[i]);
    output.y[i] += scale * (parent.y[i] - static_parent.y[i]);
    output.z[i] += scale * (parent.z[i] - static_parent.z[i]);
  }
}

CornerState make_corner_state(
    const ParentState& parent, const Direction& direction,
    const CornerSpec& spec, const ConnectedMooreBlockOptions& options) {
  const int L = parent.state.electric.L;
  CornerState result(L);
  result.metadata.registered = true;
  result.metadata.sigma_r = spec.sigma_r;
  result.metadata.sigma_1 = spec.sigma_1;
  result.metadata.sigma_2 = spec.sigma_2;
  result.metadata.radial_side = spec.radial_side;
  result.metadata.field_scale = spec.field_scale;
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
      * (kCornerImpulse / std::sqrt(3.0));

  ConnectedMooreBlockState geometry = parent.state;
  geometry.constituents[0].momentum -= impulse;
  geometry.constituents[1].momentum += impulse;
  const double kinetic = kinetic_above_rest(geometry);
  result.metadata.kinetic = kinetic;
  if (!(kinetic > 0.0 && kinetic < options.compact_pair_well_depth))
    return result;

  const auto inner = isolate_selected_root(kinetic, 0.75, 1.0, options);
  const auto outer = isolate_selected_root(kinetic, 1.0, 1.5, options);
  result.metadata.inner_d = inner.first;
  result.metadata.outer_d = outer.first;
  result.metadata.root_residual = std::max(inner.second, outer.second);
  if (!(std::isfinite(inner.first) && std::isfinite(outer.first)
        && inner.first < parent_d && parent_d < outer.first
        && result.metadata.root_residual <= 1e-12))
    return result;

  const double margin = std::min(
      parent_d - inner.first, outer.first - parent_d);
  const double target_d = parent_d
      + static_cast<double>(spec.radial_side) * 0.5 * margin;
  result.metadata.nearest_margin = margin;
  result.metadata.target_d = target_d;
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
  result.initial.state = static_perturbed.state;
  add_scaled_face_residual(
      result.initial.state.electric, parent.state.electric,
      static_parent.state.electric, spec.field_scale);
  add_scaled_edge_residual(
      result.initial.state.magnetic_half, parent.state.magnetic_half,
      static_parent.state.magnetic_half, spec.field_scale);

  const auto density = fractional_density(result.initial.state);
  if (density.empty()) return result;
  result.initial.gauss_residual = ftd::eft::max_fractional_gauss_residual(
      result.initial.state.electric, density);
  result.initial.momentum_preservation =
      (total_momentum(result.initial.state) - total_momentum(parent.state)).mag();
  result.initial.maximum_speed = maximum_speed(result.initial.state);
  result.initial.separation = pair_separation(result.initial.state);
  result.initial.pair_energy = pair_internal_energy(
      result.initial.state, options);
  result.initial.valid = std::isfinite(result.initial.gauss_residual)
      && result.initial.gauss_residual <= 1e-12
      && result.initial.momentum_preservation <= 1e-15
      && result.initial.maximum_speed <= ftd::C_SPEED + 1e-12
      && graph_inside(result.initial.separation, options)
      && result.initial.pair_energy < -1e-6;
  return result;
}

CornerArm run_corner(
    int L, const std::string& stage, const Direction& direction,
    bool conjugate, const CornerSpec& spec, const std::string& selector,
    const ParentState& parent, const ConnectedMooreBlockOptions& options,
    double interaction_scale) {
  CornerArm result;
  const auto corner = make_corner_state(parent, direction, spec, options);
  result.metadata = corner.metadata;
  ParentState staged(L);
  staged.valid = corner.initial.valid;
  staged.reproduction_pass = parent.reproduction_pass;
  staged.separation = corner.initial.separation;
  staged.pair_energy = corner.initial.pair_energy;
  staged.state = corner.initial.state;
  result.arm = run_continuation(
      L, stage, direction, conjugate, "center", selector,
      staged, options, interaction_scale);
  result.arm.variant = spec.name;
  result.arm.parent_valid = parent.valid;
  result.arm.parent_reproduction_pass = parent.reproduction_pass;
  if (!corner.initial.valid) {
    result.arm.initialized = false;
    result.arm.initial_gauss_residual = corner.initial.gauss_residual;
    result.arm.initial_momentum_preservation =
        corner.initial.momentum_preservation;
    result.arm.initial_maximum_speed = corner.initial.maximum_speed;
    result.arm.initial_pair_energy = corner.initial.pair_energy;
    result.arm.separation_history = {corner.initial.separation};
    result.arm.internal_history = {corner.initial.pair_energy};
  }
  return result;
}

CornerArm run_center(
    int L, const std::string& stage, const Direction& direction,
    bool conjugate, const ParentState& parent,
    const ConnectedMooreBlockOptions& options, double interaction_scale) {
  CornerArm result;
  result.arm = run_continuation(
      L, stage, direction, conjugate, "center", "center",
      parent, options, interaction_scale);
  return result;
}

const CornerArm* find_record(
    const std::vector<CornerArm>& records, int volume,
    const std::string& direction, const std::string& polarity,
    const std::string& variant) {
  const auto found = std::find_if(
      records.begin(), records.end(), [&](const CornerArm& record) {
        const auto& arm = record.arm;
        return arm.volume == volume && arm.direction == direction
            && arm.polarity == polarity && arm.variant == variant;
      });
  return found == records.end() ? nullptr : &*found;
}

std::vector<Selector> select_corner_stress(
    const std::vector<CornerArm>& records) {
  std::vector<Selector> selectors;
  for (const auto& direction : kDirections)
    for (const std::string polarity : {"plus_minus", "minus_plus"}) {
      std::vector<const ArmResult*> candidates;
      for (const auto& record : records) {
        const auto& arm = record.arm;
        if (arm.volume == 33 && arm.direction == direction.label
            && arm.polarity == polarity && arm.variant != "center")
          candidates.push_back(&arm);
      }
      std::sort(candidates.begin(), candidates.end(),
          [](const ArmResult* lhs, const ArmResult* rhs) {
            if (lhs->minimum_energy_margin != rhs->minimum_energy_margin)
              return lhs->minimum_energy_margin < rhs->minimum_energy_margin;
            return lhs->variant < rhs->variant;
          });
      const std::string energy_variant = candidates.front()->variant;
      std::sort(candidates.begin(), candidates.end(),
          [](const ArmResult* lhs, const ArmResult* rhs) {
            if (lhs->minimum_graph_margin != rhs->minimum_graph_margin)
              return lhs->minimum_graph_margin < rhs->minimum_graph_margin;
            return lhs->variant < rhs->variant;
          });
      const auto graph = std::find_if(
          candidates.begin(), candidates.end(), [&](const ArmResult* arm) {
            return arm->variant != energy_variant;
          });
      selectors.push_back({direction.label, polarity, energy_variant,
                           (*graph)->variant});
    }
  return selectors;
}

const CornerSpec* find_spec(const std::vector<CornerSpec>& specs,
                            const std::string& name) {
  const auto found = std::find_if(
      specs.begin(), specs.end(), [&](const CornerSpec& spec) {
        return spec.name == name;
      });
  return found == specs.end() ? nullptr : &*found;
}

void write_corner_records(
    const std::vector<CornerArm>& records,
    const std::vector<Selector>& selectors, const std::string& verdict,
    int polarity_mismatches, int volume_mismatches,
    int stage_a_survives, int stage_b_survives, int center_survives,
    double maximum_common, double maximum_recoil,
    double maximum_inverse, double maximum_balance,
    double minimum_shell_margin) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results" / "ftd_0734";
  std::filesystem::create_directories(directory);
  std::ofstream csv(directory /
      "ftd_0734_energy_adapted_mixed_capture_corners_v1.csv");
  csv << "volume,stage,direction,polarity,variant,selector,parent_valid,"
         "parent_reproduction_pass,initialized,executed,identity_pass,"
         "recoil_pass,inverse_pass,positive_field_energy,survives,"
         "final_class,graph_transitions,transition_ticks,initial_gauss_residual,"
         "initial_momentum_preservation,initial_maximum_speed,"
         "initial_pair_energy,final_pair_energy,energy_export,"
         "pair_field_balance,minimum_energy_margin,minimum_graph_margin,"
         "max_common_residual,max_recoil_defect,inverse_recovery,"
         "registered_corner,sigma_r,sigma_1,sigma_2,radial_side,field_scale,"
         "kinetic,inner_d,outer_d,nearest_margin,target_d,root_residual,"
         "separation_history,internal_history,field_history\n"
      << std::setprecision(17);
  for (const auto& record : records) {
    const auto& arm = record.arm;
    const auto& meta = record.metadata;
    csv << arm.volume << ',' << arm.stage << ',' << arm.direction << ','
        << arm.polarity << ',' << arm.variant << ',' << arm.selector << ','
        << arm.parent_valid << ',' << arm.parent_reproduction_pass << ','
        << arm.initialized << ',' << arm.executed << ',' << arm.identity_pass
        << ',' << arm.recoil_pass << ',' << arm.inverse_pass << ','
        << arm.positive_field_energy << ',' << arm.survives << ','
        << arm.final_class << ',' << arm.graph_transitions << ','
        << join_ticks(arm.graph_transition_ticks) << ','
        << arm.initial_gauss_residual << ','
        << arm.initial_momentum_preservation << ','
        << arm.initial_maximum_speed << ',' << arm.initial_pair_energy << ','
        << arm.final_pair_energy << ','
        << arm.final_field_energy - arm.initial_field_energy << ','
        << arm.pair_field_balance << ',' << arm.minimum_energy_margin << ','
        << arm.minimum_graph_margin << ',' << arm.maximum_common_residual
        << ',' << arm.maximum_recoil_defect << ',' << arm.inverse_recovery
        << ',' << meta.registered << ',' << meta.sigma_r << ','
        << meta.sigma_1 << ',' << meta.sigma_2 << ',' << meta.radial_side
        << ',' << meta.field_scale << ',' << meta.kinetic << ','
        << meta.inner_d << ',' << meta.outer_d << ','
        << meta.nearest_margin << ',' << meta.target_d << ','
        << meta.root_residual << ',' << join_values(arm.separation_history)
        << ',' << join_values(arm.internal_history) << ','
        << join_values(arm.field_history) << '\n';
  }

  std::ofstream json(directory /
      "ftd_0734_energy_adapted_mixed_capture_corners_v1.json");
  json << std::setprecision(17)
       << "{\n  \"identifier\": \"FTD-0734\",\n"
       << "  \"protocol_sha256\": \"" << kMixedProtocolSha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"arm_count\": " << records.size() << ",\n"
       << "  \"stage_a_survives\": " << stage_a_survives << ",\n"
       << "  \"stage_b_survives\": " << stage_b_survives << ",\n"
       << "  \"center_survives\": " << center_survives << ",\n"
       << "  \"polarity_mismatches\": " << polarity_mismatches << ",\n"
       << "  \"volume_mismatches\": " << volume_mismatches << ",\n"
       << "  \"maximum_common\": " << maximum_common << ",\n"
       << "  \"maximum_recoil\": " << maximum_recoil << ",\n"
       << "  \"maximum_inverse\": " << maximum_inverse << ",\n"
       << "  \"maximum_balance\": " << maximum_balance << ",\n"
       << "  \"minimum_shell_margin\": " << minimum_shell_margin << ",\n"
       << "  \"selectors\": [\n";
  for (std::size_t i = 0; i < selectors.size(); ++i) {
    const auto& selector = selectors[i];
    json << "    {\"direction\": \"" << selector.direction
         << "\", \"polarity\": \"" << selector.polarity
         << "\", \"energy_variant\": \"" << selector.energy_variant
         << "\", \"graph_variant\": \"" << selector.graph_variant
         << "\"}" << (i + 1 == selectors.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";
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
  const auto specs = corner_specs();

  std::vector<std::future<std::vector<CornerArm>>> stage_a_jobs;
  for (const auto& direction : kDirections)
    for (bool conjugate : {false, true}) {
      stage_a_jobs.push_back(std::async(std::launch::async,
          [=]() {
            std::vector<CornerArm> local;
            local.reserve(33);
            const auto parent = build_parent(
                33, direction, conjugate, options);
            local.push_back(run_center(
                33, "A", direction, conjugate, parent,
                options, interaction_scale));
            for (const auto& spec : specs) {
              local.push_back(run_corner(
                  33, "A", direction, conjugate, spec,
                  "full_mixed_corner", parent, options,
                  interaction_scale));
            }
            return local;
          }));
    }

  std::vector<CornerArm> records;
  records.reserve(216);
  std::size_t completed_stage_a_groups = 0;
  for (auto& job : stage_a_jobs) {
    auto local = job.get();
    records.insert(records.end(),
                   std::make_move_iterator(local.begin()),
                   std::make_move_iterator(local.end()));
    ++completed_stage_a_groups;
    std::cerr << "FTD-0734 stageA group " << completed_stage_a_groups
              << "/6 complete\n" << std::flush;
  }

  const auto selectors = select_corner_stress(records);
  std::cerr << "FTD-0734 selectors complete count=" << selectors.size()
            << '\n' << std::flush;
  std::vector<std::future<std::vector<CornerArm>>> stage_b_jobs;
  for (const auto& selector : selectors)
    stage_b_jobs.push_back(std::async(std::launch::async,
        [=]() {
          std::vector<CornerArm> local;
          local.reserve(3);
          const auto direction = std::find_if(
              kDirections.begin(), kDirections.end(),
              [&](const Direction& item) {
                return item.label == selector.direction;
              });
          const bool conjugate = selector.polarity == "minus_plus";
          const auto parent = build_parent(
              65, *direction, conjugate, options);
          local.push_back(run_center(
              65, "B", *direction, conjugate, parent,
              options, interaction_scale));
          const auto energy_spec = find_spec(specs, selector.energy_variant);
          const auto graph_spec = find_spec(specs, selector.graph_variant);
          if (energy_spec != nullptr)
            local.push_back(run_corner(
                65, "B", *direction, conjugate, *energy_spec,
                "energy_stress", parent, options, interaction_scale));
          if (graph_spec != nullptr)
            local.push_back(run_corner(
                65, "B", *direction, conjugate, *graph_spec,
                "graph_stress", parent, options, interaction_scale));
          return local;
        }));
  std::size_t completed_stage_b_groups = 0;
  for (auto& job : stage_b_jobs) {
    auto local = job.get();
    records.insert(records.end(),
                   std::make_move_iterator(local.begin()),
                   std::make_move_iterator(local.end()));
    ++completed_stage_b_groups;
    std::cerr << "FTD-0734 stageB group " << completed_stage_b_groups
              << "/6 complete\n" << std::flush;
  }

  std::sort(records.begin(), records.end(),
      [](const CornerArm& lhs, const CornerArm& rhs) {
        return std::tie(lhs.arm.volume, lhs.arm.stage, lhs.arm.direction,
                        lhs.arm.polarity, lhs.arm.variant)
            < std::tie(rhs.arm.volume, rhs.arm.stage, rhs.arm.direction,
                       rhs.arm.polarity, rhs.arm.variant);
      });

  const bool matrix = normalization.valid && records.size() == 216
      && std::count_if(records.begin(), records.end(),
          [](const CornerArm& record) {
            return record.arm.volume == 33;
          }) == 198
      && std::count_if(records.begin(), records.end(),
          [](const CornerArm& record) {
            return record.arm.volume == 65;
          }) == 18;
  const bool executed = matrix && std::all_of(
      records.begin(), records.end(), [](const CornerArm& record) {
        const auto& arm = record.arm;
        return arm.parent_valid && arm.parent_reproduction_pass
            && arm.initialized && arm.executed;
      });
  const bool algebra = executed && std::all_of(
      records.begin(), records.end(), [](const CornerArm& record) {
        const auto& arm = record.arm;
        return arm.identity_pass && arm.recoil_pass && arm.inverse_pass
            && arm.positive_field_energy
            && arm.pair_field_balance <= 1e-8;
      });

  int polarity_mismatches = 0;
  std::vector<std::string> variant_names{"center"};
  for (const auto& spec : specs) variant_names.push_back(spec.name);
  for (const auto& direction : kDirections)
    for (const auto& variant : variant_names) {
      const auto plus = find_record(
          records, 33, direction.label, "plus_minus", variant);
      const auto minus = find_record(
          records, 33, direction.label, "minus_plus", variant);
      polarity_mismatches += plus != nullptr && minus != nullptr
          && plus->arm.survives != minus->arm.survives ? 1 : 0;
    }

  int volume_mismatches = 0;
  for (const auto& record : records) {
    const auto& arm = record.arm;
    if (arm.volume != 65) continue;
    const auto smaller = find_record(
        records, 33, arm.direction, arm.polarity, arm.variant);
    bool match = smaller != nullptr
        && smaller->arm.survives == arm.survives
        && smaller->arm.graph_transition_ticks.size()
            == arm.graph_transition_ticks.size();
    if (match)
      for (std::size_t i = 0; i < arm.graph_transition_ticks.size(); ++i)
        match = match && std::abs(
            smaller->arm.graph_transition_ticks[i]
            - arm.graph_transition_ticks[i]) <= 2;
    volume_mismatches += match ? 0 : 1;
  }

  const int stage_a_survives = static_cast<int>(std::count_if(
      records.begin(), records.end(), [](const CornerArm& record) {
        return record.arm.volume == 33 && record.arm.survives;
      }));
  const int stage_b_survives = static_cast<int>(std::count_if(
      records.begin(), records.end(), [](const CornerArm& record) {
        return record.arm.volume == 65 && record.arm.survives;
      }));
  const int center_survives = static_cast<int>(std::count_if(
      records.begin(), records.end(), [](const CornerArm& record) {
        return record.arm.variant == "center" && record.arm.survives;
      }));
  double maximum_common = 0.0;
  double maximum_recoil = 0.0;
  double maximum_inverse = 0.0;
  double maximum_balance = 0.0;
  double minimum_shell_margin = INFINITY;
  for (const auto& record : records) {
    const auto& arm = record.arm;
    if (arm.executed) {
      maximum_common = std::max(
          maximum_common, arm.maximum_common_residual);
      maximum_recoil = std::max(
          maximum_recoil, arm.maximum_recoil_defect);
      maximum_inverse = std::max(
          maximum_inverse, arm.inverse_recovery);
      maximum_balance = std::max(
          maximum_balance, arm.pair_field_balance);
    }
    if (record.metadata.registered)
      minimum_shell_margin = std::min(
          minimum_shell_margin, record.metadata.nearest_margin);
  }

  std::string verdict;
  if (!algebra)
    verdict = "ENERGY_ADAPTED_MIXED_CAPTURE_TRANSACTION_UNRESOLVED";
  else if (center_survives != 12)
    verdict = "CAPTURE_CENTER_LONG_HORIZON_UNSTABLE";
  else if (polarity_mismatches != 0)
    verdict = "CAPTURE_MIXED_CORNERS_POLARITY_SENSITIVE";
  else if (volume_mismatches != 0)
    verdict = "CAPTURE_MIXED_CORNERS_VOLUME_SENSITIVE";
  else if (stage_a_survives == 198 && stage_b_survives == 18)
    verdict = "CAPTURE_ENERGY_ADAPTED_MIXED_CORNERS_SURVIVE";
  else
    verdict = "CAPTURE_MIXED_DYNAMICAL_BOUNDARY_WITNESSED";

  write_corner_records(
      records, selectors, verdict, polarity_mismatches, volume_mismatches,
      stage_a_survives, stage_b_survives, center_survives,
      maximum_common, maximum_recoil, maximum_inverse, maximum_balance,
      minimum_shell_margin);
  std::cout << "FTD-0734 " << verdict
            << " stageA=" << stage_a_survives << "/198"
            << " stageB=" << stage_b_survives << "/18"
            << " centers=" << center_survives << "/12"
            << " polarity_mismatch=" << polarity_mismatches
            << " volume_mismatch=" << volume_mismatches
            << " min_shell_margin=" << minimum_shell_margin << '\n';
  return executed ? 0 : 1;
}
