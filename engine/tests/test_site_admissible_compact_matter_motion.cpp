/** FTD-0607: site-admissible compact matter autonomous-motion discriminator. */

#define FTD0606_NO_MAIN
#include "test_global_orientation_strain_core.cpp"
#undef FTD0606_NO_MAIN

namespace {

constexpr const char* site_motion_protocol_sha256 =
    "CA37FB9700A2416FE293B26A903A9DCA5233091C215E0AEB83D92BA802D871E9";
constexpr int site_motion_max_evaluations = 1500;
constexpr double reported_chart_margin = 5e-3;

double chart_margin(const ClosedNeutralTrimerPairState& state) {
  double margin = 0.5;
  for (const auto& point : state.constituents) {
    margin = std::min(margin, 0.5 - std::abs(point.remainder.x));
    margin = std::min(margin, 0.5 - std::abs(point.remainder.y));
    margin = std::min(margin, 0.5 - std::abs(point.remainder.z));
  }
  return margin;
}

bool site_admissible(const ClosedNeutralTrimerPairState& state) {
  return duplicate_anchor_pairs(state) == 0 && chart_margin(state) >= 0.0;
}

GlobalEvaluation evaluate_site_admissible(
    double phase, const Matrix3& orientation, const Strain& strain,
    const ClosedNeutralPairOptions& options, const GreenKernel& green,
    double beta) {
  auto result = evaluate_global(
      phase, orientation, strain, options, green, beta);
  if (!result.valid || !site_admissible(result.state))
    return GlobalEvaluation{};
  return result;
}

GlobalEvaluation evaluate_site_parameters(
    double phase, const Matrix3& start, const Parameters& parameters,
    const ClosedNeutralPairOptions& options, const GreenKernel& green,
    double beta) {
  const Matrix3 orientation = multiply(rotation_exponential(
      {parameters[0], parameters[1], parameters[2]}), start);
  const Strain strain{{parameters[3], parameters[4], parameters[5]}};
  return evaluate_site_admissible(
      phase, orientation, strain, options, green, beta);
}

struct SiteSearchResult {
  bool admissible_start = false;
  bool terminated = false;
  int evaluations = 0;
  double diameter = INFINITY;
  double energy_spread = INFINITY;
  GlobalEvaluation minimum{};
};

SiteSearchResult search_site_cell(
    double phase, const Matrix3& start,
    const ClosedNeutralPairOptions& options, const GreenKernel& green,
    double beta) {
  SiteSearchResult result;
  Parameters zero{};
  const auto initial = evaluate_site_parameters(
      phase, start, zero, options, green, beta);
  result.admissible_start = initial.valid;
  if (!result.admissible_start) return result;

  std::array<GlobalVertex, 7> simplex{};
  const auto evaluate = [&](const Parameters& point) {
    GlobalVertex vertex;
    vertex.point = point;
    if (result.evaluations >= site_motion_max_evaluations) return vertex;
    vertex.evaluation = evaluate_site_parameters(
        phase, start, point, options, green, beta);
    ++result.evaluations;
    return vertex;
  };
  simplex[0].point = zero;
  simplex[0].evaluation = initial;
  ++result.evaluations;
  for (int d = 0; d < 6; ++d) {
    Parameters point{};
    point[d] = d < 3 ? 0.03 : 0.01;
    simplex[static_cast<std::size_t>(d + 1)] = evaluate(point);
  }
  const auto score = [](const GlobalVertex& vertex) {
    return vertex.evaluation.valid
        ? vertex.evaluation.total_energy : 1e100;
  };
  while (result.evaluations < site_motion_max_evaluations) {
    std::sort(simplex.begin(), simplex.end(), [&](const GlobalVertex& a,
                                                   const GlobalVertex& b) {
      return score(a) < score(b);
    });
    result.diameter = 0.0;
    for (std::size_t i = 1; i < simplex.size(); ++i)
      for (int d = 0; d < 6; ++d)
        result.diameter = std::max(result.diameter,
            std::abs(simplex[i].point[d] - simplex[0].point[d]));
    result.energy_spread = std::abs(
        score(simplex.back()) - score(simplex.front()));
    if (result.diameter <= 1e-7 && result.energy_spread <= 1e-14) {
      result.terminated = true;
      break;
    }
    Parameters centroid{};
    for (int i = 0; i < 6; ++i)
      for (int d = 0; d < 6; ++d)
        centroid[d] += simplex[static_cast<std::size_t>(i)].point[d] / 6.0;
    const auto reflected = [&]() {
      GlobalVertex vertex;
      vertex.point = global_affine(centroid, simplex.back().point, -1.0);
      return evaluate(vertex.point);
    }();
    if (score(reflected) < score(simplex.front())) {
      const auto expanded = evaluate(global_affine(
          centroid, reflected.point, 2.0));
      simplex.back() = score(expanded) < score(reflected)
          ? expanded : reflected;
    } else if (score(reflected) < score(simplex[5])) {
      simplex.back() = reflected;
    } else {
      const bool outside = score(reflected) < score(simplex.back());
      const Parameters target = outside ? reflected.point
                                        : simplex.back().point;
      const auto contracted = evaluate(global_affine(
          centroid, target, 0.5));
      if (score(contracted) < (outside ? score(reflected)
                                      : score(simplex.back()))) {
        simplex.back() = contracted;
      } else {
        for (std::size_t i = 1;
             i < simplex.size()
             && result.evaluations < site_motion_max_evaluations; ++i)
          simplex[i] = evaluate(global_affine(
              simplex[0].point, simplex[i].point, 0.5));
      }
    }
  }
  std::sort(simplex.begin(), simplex.end(), [&](const GlobalVertex& a,
                                                 const GlobalVertex& b) {
    return score(a) < score(b);
  });
  result.minimum = simplex.front().evaluation;
  return result;
}

GlobalEvaluation evaluate_site_tangent(
    double phase, const GlobalEvaluation& base, const Parameters& tangent,
    const ClosedNeutralPairOptions& options, const GreenKernel& green,
    double beta) {
  const Matrix3 orientation = multiply(rotation_exponential(
      {tangent[0], tangent[1], tangent[2]}), base.orientation);
  Strain strain = base.strain;
  for (int i = 0; i < 3; ++i) strain[i] += tangent[i + 3];
  return evaluate_site_admissible(
      phase, orientation, strain, options, green, beta);
}

GlobalDifferential differentiate_site(
    double phase, const GlobalEvaluation& minimum,
    const ClosedNeutralPairOptions& options, const GreenKernel& green,
    double beta) {
  GlobalDifferential result;
  result.gradient_inf = 0.0;
  std::array<std::array<double, 6>, 6> hessian{};
  constexpr double hg = 1e-4;
  constexpr double hh = 2e-3;
  for (int i = 0; i < 6; ++i) {
    Parameters plus{}, minus{};
    plus[i] = hg;
    minus[i] = -hg;
    const auto fp = evaluate_site_tangent(
        phase, minimum, plus, options, green, beta);
    const auto fm = evaluate_site_tangent(
        phase, minimum, minus, options, green, beta);
    if (!fp.valid || !fm.valid) return result;
    result.gradient_inf = std::max(result.gradient_inf,
        std::abs(fp.total_energy - fm.total_energy) / (2.0 * hg));
    plus = {};
    minus = {};
    plus[i] = hh;
    minus[i] = -hh;
    const auto hp = evaluate_site_tangent(
        phase, minimum, plus, options, green, beta);
    const auto hm = evaluate_site_tangent(
        phase, minimum, minus, options, green, beta);
    if (!hp.valid || !hm.valid) return result;
    hessian[i][i] = (hp.total_energy - 2.0 * minimum.total_energy
                     + hm.total_energy) / (hh * hh);
  }
  for (int i = 0; i < 6; ++i)
    for (int j = i + 1; j < 6; ++j) {
      Parameters pp{}, pm{}, mp{}, mm{};
      pp[i] = hh; pp[j] = hh;
      pm[i] = hh; pm[j] = -hh;
      mp[i] = -hh; mp[j] = hh;
      mm[i] = -hh; mm[j] = -hh;
      const auto fpp = evaluate_site_tangent(
          phase, minimum, pp, options, green, beta);
      const auto fpm = evaluate_site_tangent(
          phase, minimum, pm, options, green, beta);
      const auto fmp = evaluate_site_tangent(
          phase, minimum, mp, options, green, beta);
      const auto fmm = evaluate_site_tangent(
          phase, minimum, mm, options, green, beta);
      if (!fpp.valid || !fpm.valid || !fmp.valid || !fmm.valid) return result;
      hessian[i][j] = hessian[j][i] =
          (fpp.total_energy - fpm.total_energy - fmp.total_energy
           + fmm.total_energy) / (4.0 * hh * hh);
    }
  result.eigenvalues = jacobi_eigenvalues(hessian);
  result.minimum_eigenvalue = result.eigenvalues.front();
  result.positive_modes = static_cast<int>(std::count_if(
      result.eigenvalues.begin(), result.eigenvalues.end(),
      [](double value) { return value > 1e-6; }));
  result.valid = true;
  return result;
}

struct SitePhaseRecord {
  int phase_index = 0;
  int admissible_starts = 0;
  int terminated_starts = 0;
  int clustered_starts = 0;
  int total_evaluations = 0;
  double phase = 0.0;
  double chart_margin = -INFINITY;
  double energy = INFINITY;
  double gradient_inf = INFINITY;
  double minimum_eigenvalue = -INFINITY;
  int positive_modes = 0;
  double field_gate = INFINITY;
  bool coverage = false;
  bool qualified = false;
  GlobalEvaluation best{};
};

Vec3 group_center(const ClosedNeutralTrimerPairState& state,
                  std::size_t group) {
  Vec3 result{};
  for (std::size_t i = 0; i < 3; ++i)
    result += effective_position(state.constituents[3 * group + i]);
  return result * (1.0 / 3.0);
}

std::pair<double, double> dynamic_distance_range(
    const ClosedNeutralTrimerPairState& state) {
  double minimum = INFINITY;
  double maximum = 0.0;
  for (std::size_t group = 0; group < 2; ++group)
    for (std::size_t a = 0; a < 3; ++a)
      for (std::size_t b = a + 1; b < 3; ++b) {
        const Vec3 xa = effective_position(
            state.constituents[3 * group + a]);
        const Vec3 xb = effective_position(
            state.constituents[3 * group + b]);
        const double distance = (xa - xb).mag();
        minimum = std::min(minimum, distance);
        maximum = std::max(maximum, distance);
      }
  return {minimum, maximum};
}

int anchor_changes(const ClosedNeutralTrimerPairState& before,
                   const ClosedNeutralTrimerPairState& after) {
  int result = 0;
  for (std::size_t i = 0; i < before.constituents.size(); ++i) {
    const auto& a = before.constituents[i].anchor;
    const auto& b = after.constituents[i].anchor;
    if (a.x != b.x || a.y != b.y || a.z != b.z) ++result;
  }
  return result;
}

struct MotionTickRecord {
  int arm = 0;
  int direction = 1;
  int tick = 0;
  double common_gate = INFINITY;
  double energy_drift = INFINITY;
  int duplicate_anchors = 0;
  double minimum_distance = 0.0;
  double maximum_distance = INFINITY;
  double separation = INFINITY;
};

struct MotionArmResult {
  bool valid = false;
  int ticks_requested = 0;
  int forward_ticks = 0;
  int reverse_ticks = 0;
  int site_hops = 0;
  double requested_velocity = 0.0;
  double nominal_displacement = 0.0;
  double longitudinal_displacement = 0.0;
  double transverse_drift = INFINITY;
  double maximum_separation_change = INFINITY;
  double minimum_internal_distance = INFINITY;
  double maximum_internal_distance = 0.0;
  double worst_common_gate = 0.0;
  double maximum_energy_drift = 0.0;
  double reverse_recovery = INFINITY;
  int maximum_duplicate_anchors = 0;
  std::vector<MotionTickRecord> records{};
};

MotionArmResult run_motion_arm(
    int arm_index, const ClosedNeutralTrimerPairState& static_state,
    double velocity, int ticks, const ClosedNeutralPairOptions& options) {
  MotionArmResult result;
  result.requested_velocity = velocity;
  result.ticks_requested = ticks;
  result.nominal_displacement = velocity * ticks;
  ClosedNeutralTrimerPairState initial = static_state;
  const Vec3 momentum = ftd::eft::production_flat_momentum(
      {velocity, 0.0, 0.0});
  for (auto& point : initial.constituents) point.momentum = momentum;
  ClosedNeutralTrimerPairState current = initial;
  const Vec3 center0 = (group_center(initial, 0)
                        + group_center(initial, 1)) * 0.5;
  const double separation0 = (group_center(initial, 1)
                              - group_center(initial, 0)).mag();
  double baseline_energy = NAN;
  bool forward_ok = true;
  result.maximum_separation_change = 0.0;

  for (int tick = 0; tick < ticks; ++tick) {
    const auto step = ftd::eft::solve_closed_neutral_pair_forward(
        current, options);
    MotionTickRecord record;
    record.arm = arm_index;
    record.direction = 1;
    record.tick = tick;
    record.common_gate = maximum_common_gate(step);
    if (!step.valid) {
      forward_ok = false;
      result.records.push_back(record);
      break;
    }
    const double total_before = step.kinetic_energy_before
        + step.binding_energy_before + step.field_energy_before;
    const double total_after = step.kinetic_energy_after
        + step.binding_energy_after + step.field_energy_after;
    if (!std::isfinite(baseline_energy)) baseline_energy = total_before;
    record.energy_drift = std::abs(total_after - baseline_energy);
    record.duplicate_anchors = duplicate_anchor_pairs(step.later);
    const auto distances = dynamic_distance_range(step.later);
    record.minimum_distance = distances.first;
    record.maximum_distance = distances.second;
    record.separation = (group_center(step.later, 1)
                         - group_center(step.later, 0)).mag();
    result.site_hops += anchor_changes(current, step.later);
    ++result.forward_ticks;
    result.worst_common_gate = std::max(
        result.worst_common_gate, record.common_gate);
    result.maximum_energy_drift = std::max(
        result.maximum_energy_drift, record.energy_drift);
    result.maximum_duplicate_anchors = std::max(
        result.maximum_duplicate_anchors, record.duplicate_anchors);
    result.minimum_internal_distance = std::min(
        result.minimum_internal_distance, record.minimum_distance);
    result.maximum_internal_distance = std::max(
        result.maximum_internal_distance, record.maximum_distance);
    result.maximum_separation_change = std::max(
        result.maximum_separation_change,
        std::abs(record.separation - separation0));
    result.records.push_back(record);
    forward_ok = forward_ok && step.common_action_gates_pass
        && record.common_gate <= gate
        && record.duplicate_anchors == 0;
    current = step.later;
    if (!forward_ok) break;
  }

  if (forward_ok && result.forward_ticks == ticks) {
    const Vec3 center1 = (group_center(current, 0)
                          + group_center(current, 1)) * 0.5;
    const Vec3 displacement = center1 - center0;
    result.longitudinal_displacement = displacement.x;
    result.transverse_drift = std::sqrt(
        displacement.y * displacement.y + displacement.z * displacement.z);
    for (int tick = 0; tick < ticks; ++tick) {
      const auto step = ftd::eft::solve_closed_neutral_pair_reverse(
          current, options);
      MotionTickRecord record;
      record.arm = arm_index;
      record.direction = -1;
      record.tick = tick;
      record.common_gate = maximum_common_gate(step);
      if (!step.valid) {
        result.records.push_back(record);
        break;
      }
      record.duplicate_anchors = duplicate_anchor_pairs(step.earlier);
      const auto distances = dynamic_distance_range(step.earlier);
      record.minimum_distance = distances.first;
      record.maximum_distance = distances.second;
      record.separation = (group_center(step.earlier, 1)
                           - group_center(step.earlier, 0)).mag();
      ++result.reverse_ticks;
      result.worst_common_gate = std::max(
          result.worst_common_gate, record.common_gate);
      result.maximum_duplicate_anchors = std::max(
          result.maximum_duplicate_anchors, record.duplicate_anchors);
      result.records.push_back(record);
      current = step.earlier;
      if (!step.common_action_gates_pass || record.common_gate > gate
          || record.duplicate_anchors != 0) break;
    }
  }
  if (result.reverse_ticks == ticks)
    result.reverse_recovery =
        ftd::eft::closed_neutral_pair_state_max_difference(initial, current);

  result.valid = forward_ok && result.forward_ticks == ticks
      && result.reverse_ticks == ticks
      && result.longitudinal_displacement
          >= 0.75 * result.nominal_displacement
      && result.transverse_drift <= 0.25
      && result.site_hops >= 6
      && result.maximum_separation_change <= 0.25
      && result.minimum_internal_distance >= 0.5
      && result.maximum_internal_distance <= 2.0
      && result.worst_common_gate <= gate
      && result.maximum_energy_drift <= 1e-10
      && result.reverse_recovery <= 1e-9
      && result.maximum_duplicate_anchors == 0;
  return result;
}

struct SiteMotionSummary {
  bool green_pass = false;
  bool static_coverage_pass = true;
  bool static_branch_pass = true;
  bool phase_zero_selected = false;
  bool integer_covariance_pass = false;
  int phase_arms = 0;
  int qualified_phases = 0;
  double green_residual = INFINITY;
  double worst_static_gradient = 0.0;
  double minimum_static_eigenvalue = INFINITY;
  double worst_static_field_gate = 0.0;
  double minimum_chart_margin = INFINITY;
  double integer_covariance_residual = INFINITY;
  std::vector<SitePhaseRecord> phases{};
  std::array<MotionArmResult, 2> motion{};
  std::string verdict;
};

void write_site_motion_record(const SiteMotionSummary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0607";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0607_site_admissible_motion_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0607\",\n"
       << "  \"protocol_sha256\": \"" << site_motion_protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"phase_arms\": " << summary.phase_arms << ",\n"
       << "  \"qualified_phases\": " << summary.qualified_phases << ",\n"
       << "  \"green_pass\": " << (summary.green_pass ? "true" : "false") << ",\n"
       << "  \"static_coverage_pass\": " << (summary.static_coverage_pass ? "true" : "false") << ",\n"
       << "  \"static_branch_pass\": " << (summary.static_branch_pass ? "true" : "false") << ",\n"
       << "  \"phase_zero_selected\": " << (summary.phase_zero_selected ? "true" : "false") << ",\n"
       << "  \"integer_covariance_pass\": " << (summary.integer_covariance_pass ? "true" : "false") << ",\n"
       << "  \"green_residual\": " << summary.green_residual << ",\n"
       << "  \"worst_static_gradient\": " << summary.worst_static_gradient << ",\n"
       << "  \"minimum_static_eigenvalue\": " << json_number(summary.minimum_static_eigenvalue) << ",\n"
       << "  \"worst_static_field_gate\": " << summary.worst_static_field_gate << ",\n"
       << "  \"minimum_chart_margin\": " << json_number(summary.minimum_chart_margin) << ",\n"
       << "  \"integer_covariance_residual\": " << json_number(summary.integer_covariance_residual) << ",\n"
       << "  \"motion_arms\": [\n";
  for (std::size_t i = 0; i < summary.motion.size(); ++i) {
    const auto& arm = summary.motion[i];
    json << "    {\"velocity\": " << arm.requested_velocity
         << ", \"ticks_requested\": " << arm.ticks_requested
         << ", \"forward_ticks\": " << arm.forward_ticks
         << ", \"reverse_ticks\": " << arm.reverse_ticks
         << ", \"site_hops\": " << arm.site_hops
         << ", \"longitudinal_displacement\": "
         << arm.longitudinal_displacement
         << ", \"nominal_displacement\": " << arm.nominal_displacement
         << ", \"transverse_drift\": " << json_number(arm.transverse_drift)
         << ", \"maximum_separation_change\": "
         << json_number(arm.maximum_separation_change)
         << ", \"minimum_internal_distance\": "
         << json_number(arm.minimum_internal_distance)
         << ", \"maximum_internal_distance\": "
         << arm.maximum_internal_distance
         << ", \"worst_common_gate\": " << arm.worst_common_gate
         << ", \"maximum_energy_drift\": "
         << arm.maximum_energy_drift
         << ", \"reverse_recovery\": "
         << json_number(arm.reverse_recovery)
         << ", \"maximum_duplicate_anchors\": "
         << arm.maximum_duplicate_anchors
         << ", \"valid\": " << (arm.valid ? "true" : "false") << "}"
         << (i + 1 == summary.motion.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";

  std::ofstream static_csv(
      dir / "ftd_0607_site_admissible_static_samples_v1.csv");
  static_csv << "ftd_id,phase_index,phase,admissible_starts,terminated_starts,"
                "clustered_starts,total_evaluations,chart_margin,energy,"
                "gradient_inf,minimum_eigenvalue,positive_modes,field_gate,"
                "coverage,qualified\n";
  for (const auto& phase : summary.phases)
    static_csv << std::setprecision(17) << "FTD-0607,"
        << phase.phase_index << ',' << phase.phase << ','
        << phase.admissible_starts << ',' << phase.terminated_starts << ','
        << phase.clustered_starts << ',' << phase.total_evaluations << ','
        << phase.chart_margin << ',' << phase.energy << ','
        << phase.gradient_inf << ',' << phase.minimum_eigenvalue << ','
        << phase.positive_modes << ',' << phase.field_gate << ','
        << phase.coverage << ',' << phase.qualified << '\n';

  std::ofstream tick_csv(dir / "ftd_0607_motion_ticks_v1.csv");
  tick_csv << "ftd_id,arm,direction,tick,common_gate,energy_drift,"
              "duplicate_anchors,minimum_distance,maximum_distance,separation\n";
  for (const auto& arm : summary.motion)
    for (const auto& record : arm.records)
      tick_csv << std::setprecision(17) << "FTD-0607," << record.arm << ','
          << record.direction << ',' << record.tick << ','
          << record.common_gate << ',' << record.energy_drift << ','
          << record.duplicate_anchors << ',' << record.minimum_distance << ','
          << record.maximum_distance << ',' << record.separation << '\n';
}

}  // namespace

#ifndef FTD0607_NO_MAIN
int main() {
  std::cout << std::setprecision(17);
  ClosedNeutralPairOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-13;
  options.max_iterations = 64;
  SiteMotionSummary summary;
  const auto rotations = cubic_rotations();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const auto green = make_green_kernel();
  const double beta = normalization.mapped_field_work_coefficient;
  summary.green_residual = green.residual;
  summary.green_pass = normalization.valid && green.valid
      && green.residual <= direct_tolerance;

  GlobalEvaluation selected_phase_zero;
  if (summary.green_pass) {
    for (int j = 0; j < 32; ++j) {
      SitePhaseRecord record;
      record.phase_index = j;
      record.phase = static_cast<double>(j) / 32.0;
      std::vector<SiteSearchResult> searches;
      searches.reserve(rotations.size());
      for (const auto& rotation : rotations) {
        auto search = search_site_cell(
            record.phase, rotation, options, green, beta);
        if (search.admissible_start) ++record.admissible_starts;
        if (search.terminated && search.minimum.valid)
          ++record.terminated_starts;
        record.total_evaluations += search.evaluations;
        searches.push_back(std::move(search));
      }
      ++summary.phase_arms;
      for (const auto& search : searches)
        if (search.terminated && search.minimum.valid
            && (!record.best.valid
                || search.minimum.total_energy < record.best.total_energy))
          record.best = search.minimum;
      if (record.best.valid)
        for (const auto& search : searches)
          if (search.terminated && search.minimum.valid
              && std::abs(search.minimum.total_energy
                          - record.best.total_energy) <= 1e-10)
            ++record.clustered_starts;
      const int required_terminated =
          (3 * record.admissible_starts + 3) / 4;
      record.coverage = record.admissible_starts >= 12
          && record.terminated_starts >= required_terminated
          && record.clustered_starts >= 2;
      summary.static_coverage_pass = summary.static_coverage_pass
          && record.coverage;

      if (record.best.valid) {
        record.chart_margin = ::chart_margin(record.best.state);
        record.energy = record.best.total_energy;
        const auto differential = differentiate_site(
            record.phase, record.best, options, green, beta);
        record.gradient_inf = differential.gradient_inf;
        record.minimum_eigenvalue = differential.minimum_eigenvalue;
        record.positive_modes = differential.positive_modes;
        const auto direct = initialize_minimum_energy(
            density_of(record.best.state));
        const double energy_residual = direct.valid
            ? std::abs(record.best.field_energy - beta * direct.raw_energy)
            : INFINITY;
        record.field_gate = std::max({direct.solver_residual,
            direct.gauss_residual, direct.curl_residual, energy_residual});
        record.qualified = record.coverage && differential.valid
            && record.chart_margin >= reported_chart_margin
            && strain_max_abs(record.best.strain)
                <= strain_basin - 1e-4
            && strain_minimum_eigenvalue(record.best.strain) >= 0.70
            && record.best.minimum_distance >= 0.5
            && record.best.maximum_distance <= 2.0
            && record.gradient_inf <= 5e-7
            && record.minimum_eigenvalue >= -5e-6
            && record.positive_modes == 6
            && direct.valid && record.field_gate <= 1e-11;
        summary.worst_static_gradient = std::max(
            summary.worst_static_gradient, record.gradient_inf);
        summary.minimum_static_eigenvalue = std::min(
            summary.minimum_static_eigenvalue,
            record.minimum_eigenvalue);
        summary.worst_static_field_gate = std::max(
            summary.worst_static_field_gate, record.field_gate);
        summary.minimum_chart_margin = std::min(
            summary.minimum_chart_margin, record.chart_margin);
        if (record.qualified) {
          ++summary.qualified_phases;
          if (j == 0) {
            selected_phase_zero = record.best;
            selected_phase_zero.state.electric = direct.electric;
            summary.phase_zero_selected = true;
          }
        }
      }
      summary.static_branch_pass = summary.static_branch_pass
          && record.qualified;
      std::cout << "phase=" << j << "/32 starts="
                << record.admissible_starts << " terminated="
                << record.terminated_starts << " cluster="
                << record.clustered_starts << " qualified="
                << record.qualified << '\n';
      summary.phases.push_back(std::move(record));
    }
  }

  if (summary.phase_zero_selected) {
    summary.motion[0] = run_motion_arm(
        0, selected_phase_zero.state, 1.0 / 64.0, 128, options);
    summary.motion[1] = run_motion_arm(
        1, selected_phase_zero.state, 1.0 / 32.0, 64, options);

    auto original = selected_phase_zero.state;
    const Vec3 momentum = ftd::eft::production_flat_momentum(
        {1.0 / 64.0, 0.0, 0.0});
    for (auto& point : original.constituents) point.momentum = momentum;
    const auto original_step = ftd::eft::solve_closed_neutral_pair_forward(
        original, options);
    const auto translated = translate_x(original, 1);
    const auto translated_step =
        ftd::eft::solve_closed_neutral_pair_forward(translated, options);
    if (original_step.valid && translated_step.valid)
      summary.integer_covariance_residual =
          ftd::eft::closed_neutral_pair_state_max_difference(
              translate_x(original_step.later, 1), translated_step.later);
    summary.integer_covariance_pass = original_step.common_action_gates_pass
        && translated_step.common_action_gates_pass
        && summary.integer_covariance_residual <= gate;
  }

  const bool motion_pass = summary.motion[0].valid
      && summary.motion[1].valid && summary.integer_covariance_pass;
  if (!summary.green_pass || !summary.static_coverage_pass
      || summary.phase_arms != 32) {
    summary.verdict =
        "SITE_ADMISSIBLE_COMPACT_MATTER_NUMERICALLY_UNRESOLVED";
  } else if (!summary.static_branch_pass) {
    summary.verdict =
        "SITE_ADMISSIBLE_STATIC_BRANCH_NOT_FOUND_IN_REGISTERED_SEARCH";
  } else if (motion_pass) {
    summary.verdict =
        "SITE_ADMISSIBLE_COMPACT_MATTER_MOBILE_CONSTRUCTIVE";
  } else {
    summary.verdict =
        "SITE_ADMISSIBLE_STATIC_CORE_DYNAMICS_CLOSED_NEGATIVE";
  }

  write_site_motion_record(summary);
  std::cout << "protocol_sha256=" << site_motion_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "qualified_phases=" << summary.qualified_phases << "/32\n"
            << "static_coverage_pass=" << summary.static_coverage_pass << '\n'
            << "static_branch_pass=" << summary.static_branch_pass << '\n'
            << "minimum_chart_margin=" << summary.minimum_chart_margin << '\n'
            << "arm0_valid=" << summary.motion[0].valid << '\n'
            << "arm1_valid=" << summary.motion[1].valid << '\n'
            << "integer_covariance_residual="
            << summary.integer_covariance_residual << '\n';
  return summary.phase_arms == 32 && summary.phases.size() == 32 ? 0 : 1;
}
#endif
