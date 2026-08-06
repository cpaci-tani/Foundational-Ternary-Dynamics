// FTD-0612: deterministic refinement of the FTD-0611 positive basin.
#define FTD0611_NO_MAIN
#include "test_uniform_neutralized_single_core_static.cpp"

namespace {

constexpr char refine_protocol_sha256[] =
    "B0C93907D5EEB6BE96ED9BA485E2BC452E6180FE619533052A2D870C73B52002";
constexpr double locked_ftd0611_energy = 0.0015517955076684736;
using StaticMatrix =
    std::array<std::array<double, static_dof>, static_dof>;

struct RefineDerivatives {
  bool valid = false;
  StaticParameters gradient{};
  StaticMatrix hessian{};
  std::array<double, static_dof> eigenvalues{};
  double gradient_inf = INFINITY;
  double minimum_eigenvalue = -INFINITY;
  int positive_modes = 0;
};

RefineDerivatives refinement_derivatives(
    const StaticCoreEvaluation& state,
    const ftd::eft::ChargedTrimerOptions& options,
    const GreenKernel& green, double beta) {
  RefineDerivatives result;
  constexpr double hg = 2e-4;
  constexpr double hh = 5e-4;
  result.gradient_inf = 0.0;
  for (int i = 0; i < static_dof; ++i) {
    StaticParameters p2{}, p1{}, m1{}, m2{};
    p2[i] = 2.0 * hg; p1[i] = hg;
    m1[i] = -hg; m2[i] = -2.0 * hg;
    const auto fp2 = evaluate_static_tangent(state, p2, options, green, beta);
    const auto fp1 = evaluate_static_tangent(state, p1, options, green, beta);
    const auto fm1 = evaluate_static_tangent(state, m1, options, green, beta);
    const auto fm2 = evaluate_static_tangent(state, m2, options, green, beta);
    if (!fp2.valid || !fp1.valid || !fm1.valid || !fm2.valid) return result;
    result.gradient[i] = (-fp2.energy + 8.0 * fp1.energy
        - 8.0 * fm1.energy + fm2.energy) / (12.0 * hg);
    result.gradient_inf = std::max(
        result.gradient_inf, std::abs(result.gradient[i]));

    StaticParameters plus{}, minus{};
    plus[i] = hh; minus[i] = -hh;
    const auto hp = evaluate_static_tangent(state, plus, options, green, beta);
    const auto hm = evaluate_static_tangent(state, minus, options, green, beta);
    if (!hp.valid || !hm.valid) return result;
    result.hessian[i][i] = (hp.energy - 2.0 * state.energy + hm.energy)
        / (hh * hh);
  }
  for (int i = 0; i < static_dof; ++i)
    for (int j = i + 1; j < static_dof; ++j) {
      StaticParameters pp{}, pm{}, mp{}, mm{};
      pp[i] = hh; pp[j] = hh;
      pm[i] = hh; pm[j] = -hh;
      mp[i] = -hh; mp[j] = hh;
      mm[i] = -hh; mm[j] = -hh;
      const auto fpp = evaluate_static_tangent(state, pp, options, green, beta);
      const auto fpm = evaluate_static_tangent(state, pm, options, green, beta);
      const auto fmp = evaluate_static_tangent(state, mp, options, green, beta);
      const auto fmm = evaluate_static_tangent(state, mm, options, green, beta);
      if (!fpp.valid || !fpm.valid || !fmp.valid || !fmm.valid) return result;
      result.hessian[i][j] = result.hessian[j][i] =
          (fpp.energy - fpm.energy - fmp.energy + fmm.energy)
          / (4.0 * hh * hh);
    }
  result.eigenvalues = static_jacobi_eigenvalues(result.hessian);
  result.minimum_eigenvalue = result.eigenvalues.front();
  result.positive_modes = static_cast<int>(std::count_if(
      result.eigenvalues.begin(), result.eigenvalues.end(),
      [](double value) { return value > 1e-6; }));
  result.valid = true;
  return result;
}

bool solve_newton_system(StaticMatrix matrix,
                         const StaticParameters& gradient,
                         StaticParameters& step,
                         double& minimum_pivot) {
  StaticMatrix augmented = matrix;
  StaticParameters rhs{};
  for (int i = 0; i < static_dof; ++i) rhs[i] = -gradient[i];
  minimum_pivot = INFINITY;
  for (int column = 0; column < static_dof; ++column) {
    int pivot = column;
    for (int row = column + 1; row < static_dof; ++row)
      if (std::abs(augmented[row][column])
          > std::abs(augmented[pivot][column])) pivot = row;
    const double pivot_value = std::abs(augmented[pivot][column]);
    minimum_pivot = std::min(minimum_pivot, pivot_value);
    if (pivot_value <= 1e-8 || !std::isfinite(pivot_value)) return false;
    if (pivot != column) {
      std::swap(augmented[pivot], augmented[column]);
      std::swap(rhs[pivot], rhs[column]);
    }
    for (int row = column + 1; row < static_dof; ++row) {
      const double factor = augmented[row][column]
          / augmented[column][column];
      for (int k = column; k < static_dof; ++k)
        augmented[row][k] -= factor * augmented[column][k];
      rhs[row] -= factor * rhs[column];
    }
  }
  for (int row = static_dof - 1; row >= 0; --row) {
    double value = rhs[row];
    for (int k = row + 1; k < static_dof; ++k)
      value -= augmented[row][k] * step[k];
    step[row] = value / augmented[row][row];
  }
  return std::all_of(step.begin(), step.end(),
      [](double value) { return std::isfinite(value); });
}

double parameter_inf(const StaticParameters& values) {
  double result = 0.0;
  for (double value : values) result = std::max(result, std::abs(value));
  return result;
}

struct RefineIteration {
  int iteration = 0;
  double energy_before = INFINITY;
  double energy_after = INFINITY;
  double gradient = INFINITY;
  double step = INFINITY;
  double damping = 0.0;
  double minimum_pivot = INFINITY;
};

struct RefineResult {
  bool coverage = false;
  bool converged = false;
  StaticCoreEvaluation state{};
  RefineDerivatives derivatives{};
  std::vector<RefineIteration> iterations{};
};

RefineResult refine_static_state(
    const StaticCoreEvaluation& initial,
    const ftd::eft::ChargedTrimerOptions& options,
    const GreenKernel& green, double beta) {
  RefineResult result;
  result.state = initial;
  result.coverage = initial.valid;
  for (int iteration = 0; iteration < 8 && result.coverage; ++iteration) {
    const auto derivatives = refinement_derivatives(
        result.state, options, green, beta);
    if (!derivatives.valid) { result.coverage = false; break; }
    if (derivatives.gradient_inf <= 1e-11) {
      result.converged = true;
      result.derivatives = derivatives;
      break;
    }
    StaticParameters newton{};
    double minimum_pivot = INFINITY;
    if (!solve_newton_system(
        derivatives.hessian, derivatives.gradient,
        newton, minimum_pivot)) {
      result.coverage = false;
      break;
    }
    const double raw_step = parameter_inf(newton);
    RefineIteration record;
    record.iteration = iteration;
    record.energy_before = result.state.energy;
    record.gradient = derivatives.gradient_inf;
    record.step = raw_step;
    record.minimum_pivot = minimum_pivot;
    if (raw_step <= 1e-12) {
      record.energy_after = result.state.energy;
      result.iterations.push_back(record);
      result.converged = true;
      result.derivatives = derivatives;
      break;
    }
    bool accepted = false;
    for (int power = 0; power <= 7; ++power) {
      const double damping = std::ldexp(1.0, -power);
      StaticParameters trial{};
      for (int d = 0; d < static_dof; ++d)
        trial[d] = damping * newton[d];
      const auto candidate = evaluate_static_tangent(
          result.state, trial, options, green, beta);
      if (candidate.valid && candidate.energy < result.state.energy) {
        record.damping = damping;
        record.energy_after = candidate.energy;
        result.state = candidate;
        accepted = true;
        break;
      }
    }
    result.iterations.push_back(record);
    if (!accepted) { result.coverage = false; break; }
  }
  if (result.coverage) {
    result.derivatives = refinement_derivatives(
        result.state, options, green, beta);
    result.coverage = result.derivatives.valid;
    result.converged = result.coverage
        && result.derivatives.gradient_inf <= 1e-10;
  }
  return result;
}

struct RefinementSummary {
  bool fingerprint = false;
  bool search_coverage = false;
  bool refinement_coverage = false;
  bool direct_coverage = false;
  bool covariance_coverage = false;
  bool transaction_coverage = false;
  double initial_energy = INFINITY;
  double refined_energy = INFINITY;
  double total_charge = INFINITY;
  double field_gate = INFINITY;
  double covariance_energy = INFINITY;
  double covariance_state = INFINITY;
  int increasing_perturbations = 0;
  RefineResult refinement{};
  SingleArm rest{};
  std::string verdict;
};

void write_refinement_record(const RefinementSummary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0612";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0612_uniform_single_core_refinement_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0612\",\n"
       << "  \"protocol_sha256\": \"" << refine_protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"fingerprint_pass\": "
       << (summary.fingerprint ? "true" : "false") << ",\n"
       << "  \"search_coverage\": "
       << (summary.search_coverage ? "true" : "false") << ",\n"
       << "  \"refinement_coverage\": "
       << (summary.refinement_coverage ? "true" : "false") << ",\n"
       << "  \"direct_coverage\": "
       << (summary.direct_coverage ? "true" : "false") << ",\n"
       << "  \"covariance_coverage\": "
       << (summary.covariance_coverage ? "true" : "false") << ",\n"
       << "  \"transaction_coverage\": "
       << (summary.transaction_coverage ? "true" : "false") << ",\n"
       << "  \"initial_energy\": " << json_number(summary.initial_energy)
       << ",\n  \"refined_energy\": " << json_number(summary.refined_energy)
       << ",\n  \"gradient_inf\": "
       << json_number(summary.refinement.derivatives.gradient_inf)
       << ",\n  \"minimum_eigenvalue\": "
       << json_number(summary.refinement.derivatives.minimum_eigenvalue)
       << ",\n  \"positive_modes\": "
       << summary.refinement.derivatives.positive_modes
       << ",\n  \"increasing_perturbations\": "
       << summary.increasing_perturbations
       << ",\n  \"eigenvalues\": [";
  for (int i = 0; i < static_dof; ++i)
    json << (i ? "," : "")
         << summary.refinement.derivatives.eigenvalues[i];
  json << "],\n  \"total_charge\": " << json_number(summary.total_charge)
       << ",\n  \"field_gate\": " << json_number(summary.field_gate)
       << ",\n  \"covariance_energy_residual\": "
       << json_number(summary.covariance_energy)
       << ",\n  \"covariance_state_residual\": "
       << json_number(summary.covariance_state)
       << ",\n  \"iterations\": " << summary.refinement.iterations.size()
       << ",\n  \"rest\": {\"forward_ticks\": " << summary.rest.forward
       << ", \"reverse_ticks\": " << summary.rest.reverse
       << ", \"execution_complete\": "
       << (summary.rest.complete ? "true" : "false")
       << ", \"longitudinal_displacement\": "
       << json_number(summary.rest.longitudinal)
       << ", \"transverse_drift\": " << json_number(summary.rest.transverse)
       << ", \"center_momentum_change\": "
       << json_number(summary.rest.momentum_change)
       << ", \"worst_common_gate\": " << summary.rest.worst_gate
       << ", \"maximum_energy_drift\": " << summary.rest.energy_drift
       << ", \"maximum_anchor_multiplicity\": "
       << summary.rest.max_multiplicity
       << ", \"minimum_pair_distance\": "
       << json_number(summary.rest.min_distance)
       << ", \"maximum_pair_distance\": "
       << json_number(summary.rest.max_distance)
       << ", \"reverse_recovery\": " << json_number(summary.rest.recovery)
       << ", \"maximum_pseudomomentum_defect\": "
       << summary.rest.momentum_defect << "}\n}\n";

  std::ofstream csv(dir / "ftd_0612_uniform_single_core_iterations_v1.csv");
  csv << "ftd_id,iteration,energy_before,energy_after,gradient_inf,"
         "step_inf,damping,minimum_pivot\n";
  for (const auto& row : summary.refinement.iterations)
    csv << std::setprecision(17) << "FTD-0612," << row.iteration << ','
        << row.energy_before << ',' << row.energy_after << ',' << row.gradient
        << ',' << row.step << ',' << row.damping << ',' << row.minimum_pivot
        << '\n';
}

}  // namespace

#ifndef FTD0612_NO_MAIN
int main() {
  std::cout << std::setprecision(17);
  RefinementSummary summary;
  ftd::eft::ChargedTrimerOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-13;
  options.max_iterations = 64;
  options.allow_shared_anchor_chart = true;
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const auto green = make_green_kernel();
  const double beta = normalization.mapped_field_work_coefficient;
  const Matrix3 cyclic_orientation{{{0.0, 1.0, 0.0},
                                    {0.0, 0.0, 1.0},
                                    {1.0, 0.0, 0.0}}};
  std::vector<StaticSearch> searches;
  if (normalization.valid && green.valid
      && green.residual <= direct_tolerance) {
    for (double tx : {0.0, 0.5})
      for (double ty : {0.0, 0.5})
        for (double tz : {0.0, 0.5})
          for (int orientation = 0; orientation < 2; ++orientation)
            searches.push_back(search_static_core({tx, ty, tz},
                orientation == 0 ? identity_matrix() : cyclic_orientation,
                orientation, options, green, beta));
  }
  StaticCoreEvaluation initial;
  int admissible = 0, terminated = 0, clustered = 0;
  for (const auto& search : searches) {
    if (search.admissible_start) ++admissible;
    if (search.terminated && search.minimum.valid) {
      ++terminated;
      if (!initial.valid || search.minimum.energy < initial.energy)
        initial = search.minimum;
    }
  }
  if (initial.valid)
    for (const auto& search : searches)
      if (search.terminated && search.minimum.valid
          && std::abs(search.minimum.energy - initial.energy) <= 1e-10)
        ++clustered;
  summary.search_coverage = searches.size() == 16 && admissible == 16
      && terminated >= 12 && clustered >= 2 && initial.valid;
  summary.initial_energy = initial.energy;
  summary.fingerprint = initial.valid
      && std::abs(initial.energy - locked_ftd0611_energy) <= 1e-15;
  if (summary.search_coverage && summary.fingerprint)
    summary.refinement = refine_static_state(initial, options, green, beta);
  summary.refinement_coverage = summary.refinement.coverage
      && summary.refinement.converged;
  summary.refined_energy = summary.refinement.state.energy;

  if (summary.refinement.state.valid) {
    constexpr double perturbation = 1e-3;
    for (int d = 0; d < static_dof; ++d) {
      StaticParameters plus{}, minus{};
      plus[d] = perturbation; minus[d] = -perturbation;
      const auto fp = evaluate_static_tangent(
          summary.refinement.state, plus, options, green, beta);
      const auto fm = evaluate_static_tangent(
          summary.refinement.state, minus, options, green, beta);
      if (fp.valid && fp.energy > summary.refinement.state.energy)
        ++summary.increasing_perturbations;
      if (fm.valid && fm.energy > summary.refinement.state.energy)
        ++summary.increasing_perturbations;
    }
    const std::size_t count = static_cast<std::size_t>(L) * L * L;
    std::vector<double> uniform(count, -1.0 / static_cast<double>(count));
    auto total = coat_density(summary.refinement.state.state);
    if (total.size() == count) {
      for (std::size_t i = 0; i < count; ++i) total[i] += uniform[i];
      summary.total_charge = density_sum(total);
      const auto direct = initialize_minimum_energy(total);
      summary.field_gate = direct.valid
          ? std::max({direct.solver_residual, direct.gauss_residual,
              direct.curl_residual,
              std::abs(summary.refinement.state.field
                       - beta * direct.raw_energy)})
          : INFINITY;
      summary.direct_coverage = direct.valid;
      if (direct.valid)
        summary.refinement.state.state.electric = direct.electric;

      bool covariance_valid = direct.valid;
      summary.covariance_energy = 0.0;
      summary.covariance_state = 0.0;
      const std::array<Coord, 3> shifts{{{1,0,0},{0,1,0},{0,0,1}}};
      for (const auto& shift : shifts) {
        const auto shifted_eval = evaluate_static_core(
            summary.refinement.state.translation
                + Vec3{static_cast<double>(shift.x),
                       static_cast<double>(shift.y),
                       static_cast<double>(shift.z)},
            summary.refinement.state.orientation,
            summary.refinement.state.strain, options, green, beta);
        if (!shifted_eval.valid) { covariance_valid = false; continue; }
        summary.covariance_energy = std::max(summary.covariance_energy,
            std::abs(shifted_eval.energy - summary.refinement.state.energy));
        auto shifted_total = coat_density(shifted_eval.state);
        if (shifted_total.size() != count) {
          covariance_valid = false; continue;
        }
        for (std::size_t i = 0; i < count; ++i)
          shifted_total[i] += uniform[i];
        const auto shifted_field = initialize_minimum_energy(shifted_total);
        if (!shifted_field.valid) { covariance_valid = false; continue; }
        auto shifted_state = shifted_eval.state;
        shifted_state.electric = shifted_field.electric;
        summary.covariance_state = std::max(summary.covariance_state,
            ftd::eft::charged_trimer_state_max_difference(
                translate_static_state(summary.refinement.state.state, shift),
                shifted_state));
      }
      summary.covariance_coverage = covariance_valid;
      if (direct.valid) {
        NeutralizerFixture fixture;
        fixture.name = "uniform_refined";
        fixture.state = summary.refinement.state.state;
        fixture.stationary = uniform;
        fixture.valid = true;
        summary.rest = run_arm(0, 0, fixture, 0.0, 64, options);
        summary.transaction_coverage = summary.rest.complete;
      }
    }
  }

  const bool coverage = summary.fingerprint && summary.search_coverage
      && summary.refinement_coverage && summary.direct_coverage
      && summary.covariance_coverage && summary.transaction_coverage;
  const auto& derivative = summary.refinement.derivatives;
  const bool static_pass = coverage && derivative.gradient_inf <= 1e-10
      && derivative.positive_modes == static_dof
      && derivative.minimum_eigenvalue > 1e-6
      && summary.increasing_perturbations == 2 * static_dof
      && std::abs(summary.total_charge) <= 1e-11
      && summary.field_gate <= 1e-11
      && summary.covariance_energy <= 1e-12
      && summary.covariance_state <= 1e-12
      && summary.rest.worst_gate <= gate
      && summary.rest.energy_drift <= 1e-10
      && summary.rest.max_multiplicity <= 2
      && summary.rest.min_distance >= 0.5
      && summary.rest.max_distance <= 2.0
      && std::abs(summary.rest.longitudinal) <= 1e-9
      && summary.rest.transverse <= 1e-9
      && summary.rest.momentum_change <= 1e-9
      && summary.rest.recovery <= 1e-9;
  if (!coverage)
    summary.verdict =
        "REFINED_UNIFORM_SINGLE_CORE_STATIC_NUMERICALLY_UNRESOLVED";
  else if (static_pass)
    summary.verdict = "REFINED_UNIFORM_SINGLE_CORE_STATIC_CONSTRUCTIVE";
  else
    summary.verdict = "REFINED_UNIFORM_SINGLE_CORE_STATIC_CLOSED_NEGATIVE";
  write_refinement_record(summary);
  std::cout << "protocol_sha256=" << refine_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "fingerprint=" << summary.fingerprint
            << " iterations=" << summary.refinement.iterations.size()
            << " gradient=" << derivative.gradient_inf << '\n'
            << "energy=" << summary.initial_energy << " -> "
            << summary.refined_energy << '\n'
            << "rest_displacement=" << summary.rest.longitudinal
            << " transverse=" << summary.rest.transverse
            << " momentum_change=" << summary.rest.momentum_change << '\n';
  return searches.size() == 16 ? 0 : 1;
}
#endif
