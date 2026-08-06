// FTD-0618: one closed, symmetry-paired six-constituent internal gait.
#define FTD0617_NO_MAIN
#include "test_internal_gait_angular_response.cpp"
#include "ftd/eft/closed_neutral_trimer_pair.h"

namespace {

constexpr char balanced_protocol_sha256[] =
    "C8D6D2550A38BA01FAA52CDDB37A152AA0EB6D258BFBA8C1AA092B1973387A73";
constexpr char balanced_parent_sha256[] =
    "DABFBE348F9714E8B1F5EAF78D1EB06744A3BAE22D2BA4C9FBB2D2C5099995C0";
constexpr int balanced_ticks = 128;
constexpr double balanced_axis_x = 15.0;
constexpr double balanced_axis_y = 9.0;

using ftd::eft::ClosedNeutralTrimerPairState;
using ftd::eft::ClosedNeutralTrimerPairStepResult;

Vec3 half_turn_0618(const Vec3& value) {
  return {-value.x, -value.y, value.z};
}

Vec3 half_turn_position_0618(const Vec3& value) {
  return {2.0 * balanced_axis_x - value.x,
          2.0 * balanced_axis_y - value.y, value.z};
}

double periodic_delta_0618(double value) {
  while (value > 0.5 * L) value -= L;
  while (value < -0.5 * L) value += L;
  return value;
}

Vec3 point_displacement_0618(
    const ftd::eft::MatchedMatterPoint& later,
    const ftd::eft::MatchedMatterPoint& earlier) {
  const Vec3 delta = effective_position(later) - effective_position(earlier);
  return {periodic_delta_0618(delta.x), periodic_delta_0618(delta.y),
          periodic_delta_0618(delta.z)};
}

Vec3 core_displacement_0618(const ClosedNeutralTrimerPairState& later,
                            const ClosedNeutralTrimerPairState& earlier,
                            std::size_t offset) {
  Vec3 result{};
  for (std::size_t a = 0; a < 3; ++a)
    result += point_displacement_0618(
        later.constituents[offset + a], earlier.constituents[offset + a]);
  return result * (1.0 / 3.0);
}

Vec3 core_momentum_0618(const ClosedNeutralTrimerPairState& state,
                        std::size_t offset) {
  Vec3 result{};
  for (std::size_t a = 0; a < 3; ++a)
    result += state.constituents[offset + a].momentum;
  return result;
}

int maximum_anchor_multiplicity_0618(
    const ClosedNeutralTrimerPairState& state) {
  std::vector<int> counts(static_cast<std::size_t>(L) * L * L, 0);
  int maximum = 0;
  for (const auto& point : state.constituents) {
    const int index = (point.anchor.x * L + point.anchor.y) * L
        + point.anchor.z;
    maximum = std::max(maximum,
        ++counts[static_cast<std::size_t>(index)]);
  }
  return maximum;
}

std::vector<double> pair_density_0618(
    const ClosedNeutralTrimerPairState& state) {
  const std::size_t count = static_cast<std::size_t>(L) * L * L;
  std::vector<double> density(count, 0.0);
  for (std::size_t a = 0; a < state.constituents.size(); ++a) {
    const auto coat = ftd::eft::make_quadratic_polarity_coat(
        effective_position(state.constituents[a]), state.charges[a]);
    if (!coat.valid) return {};
    for (std::size_t item = 0; item < coat.weight_count; ++item) {
      const auto& weight = coat.weights[item];
      const int index = state.electric.index(
          weight.site.x, weight.site.y, weight.site.z);
      density[static_cast<std::size_t>(index)] += weight.weight;
    }
  }
  return density;
}

double balanced_common_gate_0618(
    const ClosedNeutralTrimerPairStepResult& result) {
  return std::max({result.root_residual, result.continuity_residual,
      result.gauss_before_residual, result.gauss_after_residual,
      result.force_residual, result.kinematic_residual,
      result.kinetic_discrete_gradient_residual,
      result.electric_adjoint_residual, result.magnetic_work_residual,
      result.binding_work_residual, result.binding_impulse_sum_residual,
      result.matter_work_residual, result.field_work_residual,
      result.total_energy_residual, result.causal_speed_excess});
}

double balanced_energy_before_0618(
    const ClosedNeutralTrimerPairStepResult& result) {
  return result.kinetic_energy_before + result.binding_energy_before
      + result.field_energy_before;
}

double balanced_energy_after_0618(
    const ClosedNeutralTrimerPairStepResult& result) {
  return result.kinetic_energy_after + result.binding_energy_after
      + result.field_energy_after;
}

struct BalancedRestContext {
  bool valid = false;
  ChargedTrimerState state{L};
  InternalBasis basis{};
};

BalancedRestContext make_balanced_rest_context_0618(
    const ftd::eft::ChargedTrimerOptions& options) {
  BalancedRestContext result;
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const auto green = make_green_kernel();
  if (!normalization.valid || !green.valid
      || green.residual > direct_tolerance) return result;
  const double beta = normalization.mapped_field_work_coefficient;
  const Matrix3 cyclic_orientation{{{0.0, 1.0, 0.0},
                                    {0.0, 0.0, 1.0},
                                    {1.0, 0.0, 0.0}}};
  std::vector<StaticSearch> searches;
  for (double tx : {0.0, 0.5})
    for (double ty : {0.0, 0.5})
      for (double tz : {0.0, 0.5})
        for (int orientation = 0; orientation < 2; ++orientation)
          searches.push_back(search_static_core({tx, ty, tz},
              orientation == 0 ? identity_matrix() : cyclic_orientation,
              orientation, options, green, beta));
  StaticCoreEvaluation initial;
  for (const auto& search : searches)
    if (search.terminated && search.minimum.valid
        && (!initial.valid || search.minimum.energy < initial.energy))
      initial = search.minimum;
  const RefineResult refined = initial.valid
      ? refine_static_state(initial, options, green, beta) : RefineResult{};
  result.valid = searches.size() == 16 && refined.coverage && refined.converged
      && std::abs(refined.state.energy - locked_landscape_rest_energy) <= 1e-15
      && refined.derivatives.gradient_inf <= 1e-10
      && refined.derivatives.positive_modes == static_dof;
  if (!result.valid) return result;
  result.state = refined.state.state;
  result.basis = make_internal_basis_0615(refined.state);
  result.valid = result.basis.valid;
  return result;
}

struct PairFixture0618 {
  bool valid = false;
  bool zero_net_charge = false;
  bool no_stationary_density = true;
  double amplitude = 0.0;
  double excitation_residual = INFINITY;
  double initial_core_momentum = INFINITY;
  double initial_total_momentum = INFINITY;
  ClosedNeutralTrimerPairState state{L};
};

PairFixture0618 make_pair_fixture_0618(
    int sign, const BalancedRestContext& rest) {
  PairFixture0618 result;
  if (!rest.valid) return result;
  const InternalPattern pattern = angular_pattern_0617(rest.basis, 0);
  const auto excitation = solve_excitation_0615(
      pattern, 4.0 * internal_delta_ref);
  result.amplitude = sign == 0 ? 0.0 : excitation.amplitude;
  result.excitation_residual = excitation.residual;
  for (std::size_t a = 0; a < 3; ++a) {
    result.state.constituents[a] = rest.state.constituents[a];
    result.state.constituents[a].momentum = sign == 0 ? Vec3{}
        : pattern[a] * (sign * excitation.amplitude);
    result.state.constituents[a + 3] = point_at(
        half_turn_position_0618(
            effective_position(rest.state.constituents[a])));
    result.state.constituents[a + 3].momentum = half_turn_0618(
        result.state.constituents[a].momentum);
  }
  int charge_sum = 0;
  for (int charge : result.state.charges) charge_sum += charge;
  result.zero_net_charge = charge_sum == 0;
  result.initial_core_momentum = std::max(
      core_momentum_0618(result.state, 0).mag(),
      core_momentum_0618(result.state, 3).mag());
  result.initial_total_momentum =
      (core_momentum_0618(result.state, 0)
       + core_momentum_0618(result.state, 3)).mag();
  const auto density = pair_density_0618(result.state);
  if (density.empty()) return result;
  const auto direct = initialize_minimum_energy(density);
  if (!direct.valid) return result;
  result.state.electric = direct.electric;
  result.valid = result.zero_net_charge && excitation.valid
      && result.initial_core_momentum <= 1e-12
      && result.initial_total_momentum <= 1e-12
      && ftd::eft::max_fractional_gauss_residual(
          result.state.electric, density) <= 1e-12;
  return result;
}

struct BalancedTick0618 {
  int tick = 0;
  Vec3 core_a_displacement{};
  Vec3 core_b_displacement{};
  Vec3 pair_displacement{};
  Vec3 total_matter_momentum{};
  double half_turn_residual = 0.0;
  double gate = 0.0;
  double energy_drift = 0.0;
  double pseudomomentum_defect = 0.0;
  double cumulative_pseudomomentum_drift = 0.0;
};

struct BalancedArm0618 {
  int sign = 0;
  bool initialized = false;
  bool complete = false;
  bool algebraic_pass = false;
  int forward_ticks = 0;
  int reverse_ticks = 0;
  int anchor_changes = 0;
  int maximum_multiplicity = 1;
  double amplitude = 0.0;
  double excitation_residual = INFINITY;
  double initial_core_momentum = INFINITY;
  double initial_total_momentum = INFINITY;
  Vec3 displacement{};
  double maximum_transverse = 0.0;
  double maximum_half_turn_residual = 0.0;
  double minimum_internal_distance = INFINITY;
  double maximum_internal_distance = 0.0;
  double worst_gate = INFINITY;
  double maximum_energy_drift = INFINITY;
  double maximum_pseudomomentum_defect = INFINITY;
  double maximum_cumulative_pseudomomentum_drift = INFINITY;
  double recovery = INFINITY;
  std::vector<BalancedTick0618> samples;
};

BalancedArm0618 run_balanced_arm_0618(
    int sign, const BalancedRestContext& rest,
    const ftd::eft::ClosedNeutralPairOptions& options) {
  BalancedArm0618 arm;
  arm.sign = sign;
  const auto fixture = make_pair_fixture_0618(sign, rest);
  arm.initialized = fixture.valid;
  arm.amplitude = fixture.amplitude;
  arm.excitation_residual = fixture.excitation_residual;
  arm.initial_core_momentum = fixture.initial_core_momentum;
  arm.initial_total_momentum = fixture.initial_total_momentum;
  if (!fixture.valid) return arm;
  const ClosedNeutralTrimerPairState initial = fixture.state;
  ClosedNeutralTrimerPairState current = initial;
  arm.samples.reserve(balanced_ticks + 1);
  arm.samples.push_back({});
  arm.worst_gate = 0.0;
  arm.maximum_energy_drift = 0.0;
  arm.maximum_pseudomomentum_defect = 0.0;
  arm.maximum_cumulative_pseudomomentum_drift = 0.0;
  double baseline_energy = NAN;
  Vec3 baseline_pseudomomentum{};

  for (int tick = 0; tick < balanced_ticks; ++tick) {
    const auto step = ftd::eft::solve_closed_neutral_pair_forward(
        current, options);
    if (!step.valid) break;
    if (!std::isfinite(baseline_energy)) {
      baseline_energy = balanced_energy_before_0618(step);
      baseline_pseudomomentum = step.total_pseudomomentum_before;
    }
    ++arm.forward_ticks;
    for (std::size_t a = 0; a < current.constituents.size(); ++a) {
      const auto& lhs = current.constituents[a].anchor;
      const auto& rhs = step.later.constituents[a].anchor;
      if (lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z)
        ++arm.anchor_changes;
    }
    arm.maximum_multiplicity = std::max(arm.maximum_multiplicity,
        maximum_anchor_multiplicity_0618(step.later));
    arm.worst_gate = std::max(
        arm.worst_gate, balanced_common_gate_0618(step));
    arm.maximum_energy_drift = std::max(arm.maximum_energy_drift,
        std::max(std::abs(balanced_energy_before_0618(step) - baseline_energy),
                 std::abs(balanced_energy_after_0618(step) - baseline_energy)));
    arm.minimum_internal_distance = std::min(
        arm.minimum_internal_distance, step.minimum_internal_pair_distance);
    arm.maximum_internal_distance = std::max(
        arm.maximum_internal_distance, step.maximum_internal_pair_distance);
    arm.maximum_pseudomomentum_defect = std::max(
        arm.maximum_pseudomomentum_defect, step.pseudomomentum_defect_norm);
    const double cumulative = (step.total_pseudomomentum_after
        - baseline_pseudomomentum).mag();
    arm.maximum_cumulative_pseudomomentum_drift = std::max(
        arm.maximum_cumulative_pseudomomentum_drift, cumulative);
    current = step.later;
    const Vec3 da = core_displacement_0618(current, initial, 0);
    const Vec3 db = core_displacement_0618(current, initial, 3);
    const Vec3 pair = (da + db) * 0.5;
    const double half_turn = (db - half_turn_0618(da)).mag();
    arm.maximum_transverse = std::max(
        arm.maximum_transverse, std::hypot(pair.x, pair.y));
    arm.maximum_half_turn_residual = std::max(
        arm.maximum_half_turn_residual, half_turn);
    arm.samples.push_back({tick + 1, da, db, pair,
        core_momentum_0618(current, 0) + core_momentum_0618(current, 3),
        half_turn, balanced_common_gate_0618(step), arm.maximum_energy_drift,
        step.pseudomomentum_defect_norm, cumulative});
  }

  if (arm.forward_ticks == balanced_ticks) {
    arm.displacement = arm.samples.back().pair_displacement;
    for (int tick = 0; tick < balanced_ticks; ++tick) {
      const auto step = ftd::eft::solve_closed_neutral_pair_reverse(
          current, options);
      if (!step.valid) break;
      ++arm.reverse_ticks;
      arm.maximum_multiplicity = std::max(arm.maximum_multiplicity,
          maximum_anchor_multiplicity_0618(step.earlier));
      arm.worst_gate = std::max(
          arm.worst_gate, balanced_common_gate_0618(step));
      arm.maximum_energy_drift = std::max(arm.maximum_energy_drift,
          std::max(std::abs(balanced_energy_before_0618(step) - baseline_energy),
                   std::abs(balanced_energy_after_0618(step) - baseline_energy)));
      arm.minimum_internal_distance = std::min(
          arm.minimum_internal_distance, step.minimum_internal_pair_distance);
      arm.maximum_internal_distance = std::max(
          arm.maximum_internal_distance, step.maximum_internal_pair_distance);
      current = step.earlier;
    }
  }
  arm.complete = arm.forward_ticks == balanced_ticks
      && arm.reverse_ticks == balanced_ticks;
  if (arm.complete)
    arm.recovery = ftd::eft::closed_neutral_pair_state_max_difference(
        initial, current);
  arm.algebraic_pass = arm.complete && arm.worst_gate <= 1e-12
      && arm.maximum_energy_drift <= 1e-10 && arm.recovery <= 1e-8
      && arm.maximum_multiplicity <= 2
      && arm.minimum_internal_distance >= 0.5
      && arm.maximum_internal_distance <= 2.0;
  return arm;
}

struct BalancedSummary0618 {
  bool parent_fingerprint = false;
  bool rest_fingerprint = false;
  bool arm_coverage = false;
  bool rest_pass = false;
  bool transverse_pass = false;
  bool axial_pass = false;
  bool sign_pass = false;
  bool symmetry_pass = false;
  bool momentum_pass = false;
  double sign_axial_residual = INFINITY;
  double maximum_transverse = INFINITY;
  double minimum_active_axial = INFINITY;
  double maximum_symmetry_residual = INFINITY;
  double maximum_cumulative_pseudomomentum_drift = INFINITY;
  std::vector<BalancedArm0618> arms;
  std::string verdict;
};

void evaluate_balanced_summary_0618(BalancedSummary0618& summary) {
  const auto find = [&](int sign) {
    return std::find_if(summary.arms.begin(), summary.arms.end(),
        [&](const BalancedArm0618& arm) { return arm.sign == sign; });
  };
  const auto rest = find(0), plus = find(+1), minus = find(-1);
  summary.arm_coverage = summary.arms.size() == 3
      && rest != summary.arms.end() && plus != summary.arms.end()
      && minus != summary.arms.end()
      && std::all_of(summary.arms.begin(), summary.arms.end(),
          [](const BalancedArm0618& arm) { return arm.algebraic_pass; });
  if (!summary.arm_coverage) return;
  summary.rest_pass = rest->displacement.mag() <= 1e-8;
  summary.maximum_transverse = std::max(
      plus->maximum_transverse, minus->maximum_transverse);
  summary.transverse_pass = summary.maximum_transverse <= 1e-8;
  summary.minimum_active_axial = std::min(
      std::abs(plus->displacement.z), std::abs(minus->displacement.z));
  summary.axial_pass = summary.minimum_active_axial >= 0.5;
  summary.sign_axial_residual = std::abs(
      plus->displacement.z + minus->displacement.z);
  summary.sign_pass = summary.sign_axial_residual <= 1e-8;
  summary.maximum_symmetry_residual = 0.0;
  summary.maximum_cumulative_pseudomomentum_drift = 0.0;
  for (const auto& arm : summary.arms) {
    summary.maximum_symmetry_residual = std::max(
        summary.maximum_symmetry_residual, arm.maximum_half_turn_residual);
    summary.maximum_cumulative_pseudomomentum_drift = std::max(
        summary.maximum_cumulative_pseudomomentum_drift,
        arm.maximum_cumulative_pseudomomentum_drift);
  }
  summary.symmetry_pass = summary.maximum_symmetry_residual <= 1e-8;
  summary.momentum_pass =
      summary.maximum_cumulative_pseudomomentum_drift <= 1e-10;
}

void write_balanced_record_0618(const BalancedSummary0618& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0618";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0618_closed_symmetry_balanced_gait_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0618\",\n"
       << "  \"protocol_sha256\": \"" << balanced_protocol_sha256 << "\",\n"
       << "  \"parent_result_sha256\": \"" << balanced_parent_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"net_charge\": 0,\n"
       << "  \"stationary_density_present\": false,\n"
       << "  \"parent_fingerprint_pass\": "
       << (summary.parent_fingerprint ? "true" : "false") << ",\n"
       << "  \"rest_fingerprint_pass\": "
       << (summary.rest_fingerprint ? "true" : "false") << ",\n"
       << "  \"arm_coverage\": "
       << (summary.arm_coverage ? "true" : "false") << ",\n"
       << "  \"rest_pass\": " << (summary.rest_pass ? "true" : "false")
       << ",\n  \"transverse_pass\": "
       << (summary.transverse_pass ? "true" : "false")
       << ",\n  \"axial_pass\": " << (summary.axial_pass ? "true" : "false")
       << ",\n  \"sign_pass\": " << (summary.sign_pass ? "true" : "false")
       << ",\n  \"symmetry_pass\": "
       << (summary.symmetry_pass ? "true" : "false")
       << ",\n  \"momentum_pass\": "
       << (summary.momentum_pass ? "true" : "false")
       << ",\n  \"sign_axial_residual\": "
       << json_number(summary.sign_axial_residual)
       << ",\n  \"maximum_transverse\": "
       << json_number(summary.maximum_transverse)
       << ",\n  \"minimum_active_axial\": "
       << json_number(summary.minimum_active_axial)
       << ",\n  \"maximum_symmetry_residual\": "
       << json_number(summary.maximum_symmetry_residual)
       << ",\n  \"maximum_cumulative_pseudomomentum_drift\": "
       << json_number(summary.maximum_cumulative_pseudomomentum_drift)
       << ",\n  \"arms\": [\n";
  for (std::size_t i = 0; i < summary.arms.size(); ++i) {
    const auto& arm = summary.arms[i];
    json << "    {\"sign\": " << arm.sign
         << ", \"initialized\": " << (arm.initialized ? "true" : "false")
         << ", \"complete\": " << (arm.complete ? "true" : "false")
         << ", \"algebraic_pass\": "
         << (arm.algebraic_pass ? "true" : "false")
         << ", \"forward_ticks\": " << arm.forward_ticks
         << ", \"reverse_ticks\": " << arm.reverse_ticks
         << ", \"anchor_changes\": " << arm.anchor_changes
         << ", \"maximum_anchor_multiplicity\": "
         << arm.maximum_multiplicity
         << ", \"amplitude\": " << json_number(arm.amplitude)
         << ", \"excitation_residual\": "
         << json_number(arm.excitation_residual)
         << ", \"initial_core_momentum\": "
         << json_number(arm.initial_core_momentum)
         << ", \"initial_total_momentum\": "
         << json_number(arm.initial_total_momentum)
         << ", \"displacement\": [" << arm.displacement.x << ','
         << arm.displacement.y << ',' << arm.displacement.z << ']'
         << ", \"maximum_transverse\": "
         << json_number(arm.maximum_transverse)
         << ", \"maximum_half_turn_residual\": "
         << json_number(arm.maximum_half_turn_residual)
         << ", \"minimum_internal_distance\": "
         << json_number(arm.minimum_internal_distance)
         << ", \"maximum_internal_distance\": "
         << json_number(arm.maximum_internal_distance)
         << ", \"worst_common_gate\": " << json_number(arm.worst_gate)
         << ", \"maximum_energy_drift\": "
         << json_number(arm.maximum_energy_drift)
         << ", \"maximum_pseudomomentum_defect\": "
         << json_number(arm.maximum_pseudomomentum_defect)
         << ", \"maximum_cumulative_pseudomomentum_drift\": "
         << json_number(arm.maximum_cumulative_pseudomomentum_drift)
         << ", \"reverse_recovery\": " << json_number(arm.recovery) << '}'
         << (i + 1 == summary.arms.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";

  std::ofstream arms(dir / "ftd_0618_closed_symmetry_balanced_gait_arms_v1.csv");
  arms << "ftd_id,sign,initialized,complete,algebraic_pass,forward_ticks,"
          "reverse_ticks,anchor_changes,max_multiplicity,amplitude,"
          "excitation_residual,initial_core_momentum,initial_total_momentum,"
          "dx,dy,dz,max_transverse,max_half_turn,min_internal,max_internal,"
          "worst_gate,energy_drift,pseudomomentum_defect,cumulative_momentum,"
          "recovery\n";
  for (const auto& arm : summary.arms)
    arms << std::setprecision(17) << "FTD-0618," << arm.sign << ','
         << arm.initialized << ',' << arm.complete << ',' << arm.algebraic_pass
         << ',' << arm.forward_ticks << ',' << arm.reverse_ticks << ','
         << arm.anchor_changes << ',' << arm.maximum_multiplicity << ','
         << arm.amplitude << ',' << arm.excitation_residual << ','
         << arm.initial_core_momentum << ',' << arm.initial_total_momentum << ','
         << arm.displacement.x << ',' << arm.displacement.y << ','
         << arm.displacement.z << ',' << arm.maximum_transverse << ','
         << arm.maximum_half_turn_residual << ',' << arm.minimum_internal_distance
         << ',' << arm.maximum_internal_distance << ',' << arm.worst_gate << ','
         << arm.maximum_energy_drift << ',' << arm.maximum_pseudomomentum_defect
         << ',' << arm.maximum_cumulative_pseudomomentum_drift << ','
         << arm.recovery << '\n';

  std::ofstream ticks(dir / "ftd_0618_closed_symmetry_balanced_gait_ticks_v1.csv");
  ticks << "ftd_id,sign,tick,dax,day,daz,dbx,dby,dbz,dx,dy,dz,px,py,pz,"
           "half_turn_residual,gate,energy_drift,pseudomomentum_defect,"
           "cumulative_pseudomomentum_drift\n";
  for (const auto& arm : summary.arms)
    for (const auto& sample : arm.samples)
      ticks << std::setprecision(17) << "FTD-0618," << arm.sign << ','
            << sample.tick << ',' << sample.core_a_displacement.x << ','
            << sample.core_a_displacement.y << ','
            << sample.core_a_displacement.z << ','
            << sample.core_b_displacement.x << ','
            << sample.core_b_displacement.y << ','
            << sample.core_b_displacement.z << ','
            << sample.pair_displacement.x << ','
            << sample.pair_displacement.y << ','
            << sample.pair_displacement.z << ','
            << sample.total_matter_momentum.x << ','
            << sample.total_matter_momentum.y << ','
            << sample.total_matter_momentum.z << ','
            << sample.half_turn_residual << ',' << sample.gate << ','
            << sample.energy_drift << ',' << sample.pseudomomentum_defect << ','
            << sample.cumulative_pseudomomentum_drift << '\n';
}

}  // namespace

#ifndef FTD0618_NO_MAIN
int main() {
  std::cout << std::setprecision(17);
  BalancedSummary0618 summary;
  const auto parent_path = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results" / "ftd_0617"
      / "ftd_0617_internal_gait_angular_response_v1.json";
  std::ifstream parent(parent_path, std::ios::binary);
  std::string parent_bytes((std::istreambuf_iterator<char>(parent)),
                           std::istreambuf_iterator<char>());
  summary.parent_fingerprint =
      parent_bytes.find("\"ftd_id\": \"FTD-0617\"") != std::string::npos
      && parent_bytes.find("MIXED_PARITY_INTERNAL_GAIT_RESPONSE_RESOLVED")
          != std::string::npos
      && parent_bytes.find("\"dft_pass\": true") != std::string::npos;

  ftd::eft::ClosedNeutralPairOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-13;
  options.max_iterations = 64;
  options.allow_shared_anchor_chart = true;
  const BalancedRestContext rest = summary.parent_fingerprint
      ? make_balanced_rest_context_0618(options) : BalancedRestContext{};
  summary.rest_fingerprint = rest.valid;
  if (rest.valid)
    for (int sign : {0, +1, -1})
      summary.arms.push_back(run_balanced_arm_0618(sign, rest, options));
  evaluate_balanced_summary_0618(summary);

  const bool algebraic = summary.parent_fingerprint && summary.rest_fingerprint
      && summary.arm_coverage;
  const bool kinematic = algebraic && summary.rest_pass
      && summary.transverse_pass && summary.axial_pass && summary.sign_pass
      && summary.symmetry_pass;
  if (!algebraic)
    summary.verdict = "CLOSED_SYMMETRY_BALANCED_GAIT_NUMERICALLY_UNRESOLVED";
  else if (kinematic && summary.momentum_pass)
    summary.verdict = "CLOSED_SYMMETRY_BALANCED_GAIT_CONSTRUCTIVE";
  else if (kinematic)
    summary.verdict = "SYMMETRY_BALANCED_GAIT_KINEMATIC_MOMENTUM_OPEN";
  else
    summary.verdict = "SYMMETRY_BALANCED_GAIT_NOT_CONSTRUCTIVE";
  write_balanced_record_0618(summary);

  std::cout << "protocol_sha256=" << balanced_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "arms=" << summary.arms.size()
            << " rest=" << summary.rest_pass
            << " transverse=" << summary.maximum_transverse
            << " axial=" << summary.minimum_active_axial
            << " sign_residual=" << summary.sign_axial_residual
            << " symmetry=" << summary.maximum_symmetry_residual
            << " cumulative_momentum="
            << summary.maximum_cumulative_pseudomomentum_drift << '\n';
  for (const auto& arm : summary.arms)
    std::cout << "sign=" << arm.sign
              << " d=(" << arm.displacement.x << ',' << arm.displacement.y
              << ',' << arm.displacement.z << ')'
              << " complete=" << arm.complete
              << " algebraic=" << arm.algebraic_pass
              << " gate=" << arm.worst_gate
              << " recovery=" << arm.recovery << '\n';
  return summary.arms.size() == 3 ? 0 : 1;
}
#endif
