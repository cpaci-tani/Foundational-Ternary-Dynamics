// FTD-0611: stationary compact core under a uniform periodic neutralizer.
#define FTD0610_NO_MAIN
#include "test_single_core_neutralizer_control.cpp"

namespace {

constexpr char static_protocol_sha256[] =
    "45FC3250CE24A236EBC231DAD9AA171CADFD754FA8289601892B73C107279B69";
constexpr int static_dof = 9;
constexpr int static_max_evaluations = 2500;
using StaticParameters = std::array<double, static_dof>;
using ftd::Coord;

struct StaticCoreEvaluation {
  bool valid = false;
  Vec3 translation{};
  Matrix3 orientation = identity_matrix();
  Strain strain{};
  double binding = INFINITY;
  double field = INFINITY;
  double energy = INFINITY;
  double min_distance = 0.0;
  double max_distance = INFINITY;
  ChargedTrimerState state{L};
};

StaticCoreEvaluation evaluate_static_core(
    const Vec3& translation, const Matrix3& orientation,
    const Strain& strain, const ftd::eft::ChargedTrimerOptions& options,
    const GreenKernel& green, double beta) {
  StaticCoreEvaluation result;
  result.translation = translation;
  result.orientation = orientation;
  result.strain = strain;
  if (strain_max_abs(strain) > strain_basin
      || strain_minimum_eigenvalue(strain) < 0.70
      || orthogonality_residual(orientation) > 1e-11) return result;
  const auto distances = global_distance_range(orientation, strain);
  result.min_distance = distances.first;
  result.max_distance = distances.second;
  if (result.min_distance < 0.5 || result.max_distance > 2.0) return result;
  result.state.charges = {{+1, +1, -1}};
  const auto offsets = global_offsets(orientation, strain);
  for (std::size_t a = 0; a < 3; ++a)
    result.state.constituents[a] = point_at(
        center_b + translation - offsets[a]);
  const auto density = coat_density(result.state);
  if (density.empty()) return result;
  result.binding = ftd::eft::charged_trimer_binding_energy(
      result.state, options);
  result.field = beta * green_energy(sparse_density(density), green);
  result.energy = result.binding + result.field;
  result.valid = std::isfinite(result.energy);
  return result;
}

StaticCoreEvaluation evaluate_static_parameters(
    const Vec3& translation_start, const Matrix3& orientation_start,
    const StaticParameters& point,
    const ftd::eft::ChargedTrimerOptions& options,
    const GreenKernel& green, double beta) {
  const Vec3 translation = translation_start
      + Vec3{point[0], point[1], point[2]};
  const Matrix3 orientation = multiply(rotation_exponential(
      {point[3], point[4], point[5]}), orientation_start);
  const Strain strain{{point[6], point[7], point[8]}};
  return evaluate_static_core(
      translation, orientation, strain, options, green, beta);
}

struct StaticVertex {
  StaticParameters point{};
  StaticCoreEvaluation evaluation{};
};

struct StaticSearch {
  bool admissible_start = false;
  bool terminated = false;
  int evaluations = 0;
  double diameter = INFINITY;
  double spread = INFINITY;
  Vec3 translation_start{};
  int orientation_start = 0;
  StaticCoreEvaluation minimum{};
};

StaticParameters static_affine(const StaticParameters& origin,
                               const StaticParameters& other,
                               double factor) {
  StaticParameters result{};
  for (int d = 0; d < static_dof; ++d)
    result[d] = origin[d] + factor * (other[d] - origin[d]);
  return result;
}

StaticSearch search_static_core(
    const Vec3& translation_start, const Matrix3& orientation_start,
    int orientation_index, const ftd::eft::ChargedTrimerOptions& options,
    const GreenKernel& green, double beta) {
  StaticSearch result;
  result.translation_start = translation_start;
  result.orientation_start = orientation_index;
  std::array<StaticVertex, static_dof + 1> simplex{};
  const auto evaluate = [&](const StaticParameters& point) {
    StaticVertex vertex;
    vertex.point = point;
    if (result.evaluations >= static_max_evaluations) return vertex;
    vertex.evaluation = evaluate_static_parameters(
        translation_start, orientation_start, point, options, green, beta);
    ++result.evaluations;
    return vertex;
  };
  StaticParameters zero{};
  simplex[0] = evaluate(zero);
  result.admissible_start = simplex[0].evaluation.valid;
  for (int d = 0; d < static_dof; ++d) {
    StaticParameters point{};
    point[d] = d < 6 ? 0.03 : 0.01;
    simplex[static_cast<std::size_t>(d + 1)] = evaluate(point);
  }
  const auto score = [](const StaticVertex& vertex) {
    return vertex.evaluation.valid ? vertex.evaluation.energy : 1e100;
  };
  while (result.evaluations < static_max_evaluations) {
    std::sort(simplex.begin(), simplex.end(), [&](const StaticVertex& a,
                                                   const StaticVertex& b) {
      return score(a) < score(b);
    });
    result.diameter = 0.0;
    for (std::size_t i = 1; i < simplex.size(); ++i)
      for (int d = 0; d < static_dof; ++d)
        result.diameter = std::max(result.diameter,
            std::abs(simplex[i].point[d] - simplex[0].point[d]));
    result.spread = std::abs(score(simplex.back()) - score(simplex.front()));
    if (result.diameter <= 1e-7 && result.spread <= 1e-14) {
      result.terminated = true;
      break;
    }
    StaticParameters centroid{};
    for (int i = 0; i < static_dof; ++i)
      for (int d = 0; d < static_dof; ++d)
        centroid[d] += simplex[static_cast<std::size_t>(i)].point[d]
            / static_cast<double>(static_dof);
    const auto reflected = evaluate(static_affine(
        centroid, simplex.back().point, -1.0));
    if (score(reflected) < score(simplex.front())) {
      const auto expanded = evaluate(static_affine(
          centroid, reflected.point, 2.0));
      simplex.back() = score(expanded) < score(reflected)
          ? expanded : reflected;
    } else if (score(reflected) < score(simplex[static_dof - 1])) {
      simplex.back() = reflected;
    } else {
      const bool outside = score(reflected) < score(simplex.back());
      const StaticParameters target = outside
          ? reflected.point : simplex.back().point;
      const auto contracted = evaluate(static_affine(
          centroid, target, 0.5));
      if (score(contracted) < (outside ? score(reflected)
                                      : score(simplex.back()))) {
        simplex.back() = contracted;
      } else {
        for (std::size_t i = 1;
             i < simplex.size()
             && result.evaluations < static_max_evaluations; ++i)
          simplex[i] = evaluate(static_affine(
              simplex[0].point, simplex[i].point, 0.5));
      }
    }
  }
  std::sort(simplex.begin(), simplex.end(), [&](const StaticVertex& a,
                                                 const StaticVertex& b) {
    return score(a) < score(b);
  });
  result.minimum = simplex.front().evaluation;
  return result;
}

StaticCoreEvaluation evaluate_static_tangent(
    const StaticCoreEvaluation& base, const StaticParameters& tangent,
    const ftd::eft::ChargedTrimerOptions& options,
    const GreenKernel& green, double beta) {
  const Vec3 translation = base.translation
      + Vec3{tangent[0], tangent[1], tangent[2]};
  const Matrix3 orientation = multiply(rotation_exponential(
      {tangent[3], tangent[4], tangent[5]}), base.orientation);
  Strain strain = base.strain;
  for (int i = 0; i < 3; ++i) strain[i] += tangent[i + 6];
  return evaluate_static_core(
      translation, orientation, strain, options, green, beta);
}

std::array<double, static_dof> static_jacobi_eigenvalues(
    std::array<std::array<double, static_dof>, static_dof> matrix) {
  for (int sweep = 0; sweep < 400; ++sweep) {
    int p = 0, q = 1;
    double largest = std::abs(matrix[0][1]);
    for (int i = 0; i < static_dof; ++i)
      for (int j = i + 1; j < static_dof; ++j)
        if (std::abs(matrix[i][j]) > largest) {
          largest = std::abs(matrix[i][j]); p = i; q = j;
        }
    if (largest <= 1e-12) break;
    const double angle = 0.5 * std::atan2(
        2.0 * matrix[p][q], matrix[q][q] - matrix[p][p]);
    const double c = std::cos(angle), s = std::sin(angle);
    const double app = matrix[p][p], aqq = matrix[q][q];
    const double apq = matrix[p][q];
    for (int k = 0; k < static_dof; ++k) {
      if (k == p || k == q) continue;
      const double akp = matrix[k][p], akq = matrix[k][q];
      matrix[k][p] = matrix[p][k] = c * akp - s * akq;
      matrix[k][q] = matrix[q][k] = s * akp + c * akq;
    }
    matrix[p][p] = c * c * app - 2.0 * s * c * apq + s * s * aqq;
    matrix[q][q] = s * s * app + 2.0 * s * c * apq + c * c * aqq;
    matrix[p][q] = matrix[q][p] = 0.0;
  }
  std::array<double, static_dof> result{};
  for (int i = 0; i < static_dof; ++i) result[i] = matrix[i][i];
  std::sort(result.begin(), result.end());
  return result;
}

struct StaticDifferential {
  bool valid = false;
  double gradient = INFINITY;
  std::array<double, static_dof> eigenvalues{};
  double min_eigenvalue = -INFINITY;
  int positive_modes = 0;
  int increasing_perturbations = 0;
};

StaticDifferential differentiate_static_core(
    const StaticCoreEvaluation& minimum,
    const ftd::eft::ChargedTrimerOptions& options,
    const GreenKernel& green, double beta) {
  StaticDifferential result;
  result.gradient = 0.0;
  std::array<std::array<double, static_dof>, static_dof> hessian{};
  constexpr double hg = 1e-4;
  constexpr double hh = 1e-3;
  for (int i = 0; i < static_dof; ++i) {
    StaticParameters plus{}, minus{};
    plus[i] = hg; minus[i] = -hg;
    const auto fp = evaluate_static_tangent(
        minimum, plus, options, green, beta);
    const auto fm = evaluate_static_tangent(
        minimum, minus, options, green, beta);
    if (!fp.valid || !fm.valid) return result;
    result.gradient = std::max(result.gradient,
        std::abs(fp.energy - fm.energy) / (2.0 * hg));
    plus = {}; minus = {};
    plus[i] = hh; minus[i] = -hh;
    const auto hp = evaluate_static_tangent(
        minimum, plus, options, green, beta);
    const auto hm = evaluate_static_tangent(
        minimum, minus, options, green, beta);
    if (!hp.valid || !hm.valid) return result;
    if (hp.energy > minimum.energy) ++result.increasing_perturbations;
    if (hm.energy > minimum.energy) ++result.increasing_perturbations;
    hessian[i][i] = (hp.energy - 2.0 * minimum.energy + hm.energy)
        / (hh * hh);
  }
  for (int i = 0; i < static_dof; ++i)
    for (int j = i + 1; j < static_dof; ++j) {
      StaticParameters pp{}, pm{}, mp{}, mm{};
      pp[i] = hh; pp[j] = hh;
      pm[i] = hh; pm[j] = -hh;
      mp[i] = -hh; mp[j] = hh;
      mm[i] = -hh; mm[j] = -hh;
      const auto fpp = evaluate_static_tangent(
          minimum, pp, options, green, beta);
      const auto fpm = evaluate_static_tangent(
          minimum, pm, options, green, beta);
      const auto fmp = evaluate_static_tangent(
          minimum, mp, options, green, beta);
      const auto fmm = evaluate_static_tangent(
          minimum, mm, options, green, beta);
      if (!fpp.valid || !fpm.valid || !fmp.valid || !fmm.valid) return result;
      hessian[i][j] = hessian[j][i] =
          (fpp.energy - fpm.energy - fmp.energy + fmm.energy)
          / (4.0 * hh * hh);
    }
  result.eigenvalues = static_jacobi_eigenvalues(hessian);
  result.min_eigenvalue = result.eigenvalues.front();
  result.positive_modes = static_cast<int>(std::count_if(
      result.eigenvalues.begin(), result.eigenvalues.end(),
      [](double value) { return value > 1e-6; }));
  result.valid = true;
  return result;
}

ChargedTrimerState translate_static_state(
    const ChargedTrimerState& source, const Coord& shift) {
  ChargedTrimerState target(L);
  target.charges = source.charges;
  for (std::size_t a = 0; a < source.constituents.size(); ++a) {
    target.constituents[a] = source.constituents[a];
    target.constituents[a].anchor = {
        (target.constituents[a].anchor.x + shift.x + L) % L,
        (target.constituents[a].anchor.y + shift.y + L) % L,
        (target.constituents[a].anchor.z + shift.z + L) % L};
  }
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int from = index(x, y, z);
        const int to = index(x + shift.x, y + shift.y, z + shift.z);
        target.electric.x[to] = source.electric.x[from];
        target.electric.y[to] = source.electric.y[from];
        target.electric.z[to] = source.electric.z[from];
        target.magnetic_half.x[to] = source.magnetic_half.x[from];
        target.magnetic_half.y[to] = source.magnetic_half.y[from];
        target.magnetic_half.z[to] = source.magnetic_half.z[from];
      }
  return target;
}

struct StaticSummary {
  bool green = false;
  bool search_coverage = false;
  bool differential_coverage = false;
  bool direct_coverage = false;
  bool covariance_coverage = false;
  bool transaction_coverage = false;
  int admissible_starts = 0;
  int terminated_starts = 0;
  int clustered_starts = 0;
  double best_energy = INFINITY;
  StaticDifferential differential{};
  double total_charge = INFINITY;
  double field_gate = INFINITY;
  double covariance_energy = INFINITY;
  double covariance_state = INFINITY;
  StaticCoreEvaluation best{};
  SingleArm rest{};
  std::vector<StaticSearch> searches{};
  std::string verdict;
};

void write_static_record(const StaticSummary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0611";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0611_uniform_single_core_static_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0611\",\n"
       << "  \"protocol_sha256\": \"" << static_protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"green_valid\": " << (summary.green ? "true" : "false") << ",\n"
       << "  \"search_coverage\": "
       << (summary.search_coverage ? "true" : "false") << ",\n"
       << "  \"differential_coverage\": "
       << (summary.differential_coverage ? "true" : "false") << ",\n"
       << "  \"direct_coverage\": "
       << (summary.direct_coverage ? "true" : "false") << ",\n"
       << "  \"covariance_coverage\": "
       << (summary.covariance_coverage ? "true" : "false") << ",\n"
       << "  \"transaction_coverage\": "
       << (summary.transaction_coverage ? "true" : "false") << ",\n"
       << "  \"admissible_starts\": " << summary.admissible_starts << ",\n"
       << "  \"terminated_starts\": " << summary.terminated_starts << ",\n"
       << "  \"clustered_starts\": " << summary.clustered_starts << ",\n"
       << "  \"best_energy\": " << json_number(summary.best_energy) << ",\n"
       << "  \"best_translation\": [" << summary.best.translation.x << ','
       << summary.best.translation.y << ',' << summary.best.translation.z
       << "],\n  \"best_orientation\": [";
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      json << ((i || j) ? "," : "") << summary.best.orientation[i][j];
  json << "],\n  \"best_strain\": [" << summary.best.strain[0] << ','
       << summary.best.strain[1] << ',' << summary.best.strain[2] << "],\n"
       << "  \"gradient_inf\": " << json_number(summary.differential.gradient) << ",\n"
       << "  \"minimum_eigenvalue\": "
       << json_number(summary.differential.min_eigenvalue) << ",\n"
       << "  \"positive_modes\": " << summary.differential.positive_modes << ",\n"
       << "  \"increasing_perturbations\": "
       << summary.differential.increasing_perturbations << ",\n"
       << "  \"eigenvalues\": [";
  for (int i = 0; i < static_dof; ++i)
    json << (i ? "," : "") << summary.differential.eigenvalues[i];
  json << "],\n  \"total_charge\": " << json_number(summary.total_charge)
       << ",\n  \"field_gate\": " << json_number(summary.field_gate)
       << ",\n  \"covariance_energy_residual\": "
       << json_number(summary.covariance_energy)
       << ",\n  \"covariance_state_residual\": "
       << json_number(summary.covariance_state)
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

  std::ofstream csv(dir / "ftd_0611_uniform_single_core_starts_v1.csv");
  csv << "ftd_id,start,tx,ty,tz,orientation,admissible,terminated,evaluations,"
         "diameter,energy_spread,energy,min_distance,max_distance\n";
  for (std::size_t i = 0; i < summary.searches.size(); ++i) {
    const auto& search = summary.searches[i];
    csv << std::setprecision(17) << "FTD-0611," << i << ','
        << search.translation_start.x << ',' << search.translation_start.y
        << ',' << search.translation_start.z << ','
        << search.orientation_start << ','
        << (search.admissible_start ? 1 : 0) << ','
        << (search.terminated ? 1 : 0) << ',' << search.evaluations << ','
        << search.diameter << ',' << search.spread << ','
        << search.minimum.energy << ',' << search.minimum.min_distance << ','
        << search.minimum.max_distance << '\n';
  }
}

}  // namespace

#ifndef FTD0611_NO_MAIN
int main() {
  std::cout << std::setprecision(17);
  StaticSummary summary;
  ftd::eft::ChargedTrimerOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-13;
  options.max_iterations = 64;
  options.allow_shared_anchor_chart = true;
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const auto green = make_green_kernel();
  const double beta = normalization.mapped_field_work_coefficient;
  summary.green = normalization.valid && green.valid
      && green.residual <= direct_tolerance;
  const Matrix3 cyclic_orientation{{{0.0, 1.0, 0.0},
                                    {0.0, 0.0, 1.0},
                                    {1.0, 0.0, 0.0}}};
  if (summary.green) {
    for (double tx : {0.0, 0.5})
      for (double ty : {0.0, 0.5})
        for (double tz : {0.0, 0.5})
          for (int orientation = 0; orientation < 2; ++orientation) {
            auto search = search_static_core({tx, ty, tz},
                orientation == 0 ? identity_matrix() : cyclic_orientation,
                orientation, options, green, beta);
            if (search.admissible_start) ++summary.admissible_starts;
            if (search.terminated && search.minimum.valid)
              ++summary.terminated_starts;
            summary.searches.push_back(std::move(search));
          }
  }
  StaticCoreEvaluation best;
  for (const auto& search : summary.searches)
    if (search.terminated && search.minimum.valid
        && (!best.valid || search.minimum.energy < best.energy))
      best = search.minimum;
  if (best.valid) {
    summary.best_energy = best.energy;
    summary.best = best;
    for (const auto& search : summary.searches)
      if (search.terminated && search.minimum.valid
          && std::abs(search.minimum.energy - best.energy) <= 1e-10)
        ++summary.clustered_starts;
  }
  summary.search_coverage = summary.searches.size() == 16
      && summary.admissible_starts == 16
      && summary.terminated_starts >= 12
      && summary.clustered_starts >= 2 && best.valid;

  if (best.valid) {
    summary.differential = differentiate_static_core(
        best, options, green, beta);
    summary.differential_coverage = summary.differential.valid;
    const std::size_t count = static_cast<std::size_t>(L) * L * L;
    std::vector<double> uniform(count, -1.0 / static_cast<double>(count));
    auto total = coat_density(best.state);
    if (total.size() == count) {
      for (std::size_t i = 0; i < count; ++i) total[i] += uniform[i];
      summary.total_charge = density_sum(total);
      const auto direct = initialize_minimum_energy(total);
      summary.field_gate = direct.valid
          ? std::max({direct.solver_residual, direct.gauss_residual,
              direct.curl_residual,
              std::abs(best.field - beta * direct.raw_energy)})
          : INFINITY;
      summary.direct_coverage = direct.valid;
      if (direct.valid) best.state.electric = direct.electric;

      summary.covariance_energy = 0.0;
      summary.covariance_state = 0.0;
      bool covariance_valid = direct.valid;
      const std::array<Coord, 3> shifts{{{1,0,0},{0,1,0},{0,0,1}}};
      for (const auto& shift : shifts) {
        const auto shifted_eval = evaluate_static_core(
            best.translation + Vec3{static_cast<double>(shift.x),
                static_cast<double>(shift.y), static_cast<double>(shift.z)},
            best.orientation, best.strain, options, green, beta);
        if (!shifted_eval.valid) { covariance_valid = false; continue; }
        summary.covariance_energy = std::max(summary.covariance_energy,
            std::abs(shifted_eval.energy - best.energy));
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
                translate_static_state(best.state, shift), shifted_state));
      }
      summary.covariance_coverage = covariance_valid;

      if (direct.valid) {
        NeutralizerFixture fixture;
        fixture.name = "uniform_static";
        fixture.state = best.state;
        fixture.stationary = uniform;
        fixture.valid = true;
        summary.rest = run_arm(0, 0, fixture, 0.0, 16, options);
        summary.transaction_coverage = summary.rest.complete;
      }
    }
  }

  const bool coverage = summary.green && summary.search_coverage
      && summary.differential_coverage && summary.direct_coverage
      && summary.covariance_coverage && summary.transaction_coverage;
  const bool static_pass = coverage
      && summary.differential.gradient <= 1e-8
      && summary.differential.positive_modes == static_dof
      && summary.differential.min_eigenvalue > 1e-6
      && summary.differential.increasing_perturbations == 2 * static_dof
      && std::abs(summary.total_charge) <= 1e-11
      && summary.field_gate <= 1e-11
      && summary.covariance_energy <= 1e-12
      && summary.covariance_state <= 1e-12
      && summary.rest.worst_gate <= gate
      && summary.rest.energy_drift <= 1e-10
      && summary.rest.max_multiplicity <= 2
      && summary.rest.min_distance >= 0.5
      && summary.rest.max_distance <= 2.0
      && std::abs(summary.rest.longitudinal) <= 1e-8
      && summary.rest.transverse <= 1e-8
      && summary.rest.momentum_change <= 1e-8
      && summary.rest.recovery <= 1e-9;
  if (!coverage)
    summary.verdict =
        "UNIFORM_NEUTRALIZED_SINGLE_CORE_STATIC_NUMERICALLY_UNRESOLVED";
  else if (static_pass)
    summary.verdict = "UNIFORM_NEUTRALIZED_SINGLE_CORE_STATIC_CONSTRUCTIVE";
  else
    summary.verdict = "UNIFORM_NEUTRALIZED_COMPACT_STATIC_CLOSED_NEGATIVE";
  write_static_record(summary);
  std::cout << "protocol_sha256=" << static_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "starts=" << summary.admissible_starts << '/'
            << summary.terminated_starts << '/' << summary.clustered_starts
            << " energy=" << summary.best_energy << '\n'
            << "gradient=" << summary.differential.gradient
            << " min_eigenvalue=" << summary.differential.min_eigenvalue
            << " positive=" << summary.differential.positive_modes << '\n'
            << "rest_displacement=" << summary.rest.longitudinal
            << " rest_momentum_change=" << summary.rest.momentum_change
            << " rest_complete=" << summary.rest.complete << '\n';
  return summary.searches.size() == 16 ? 0 : 1;
}
#endif
