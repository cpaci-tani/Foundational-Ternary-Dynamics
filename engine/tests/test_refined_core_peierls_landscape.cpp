// FTD-0614: selected compact-core Peierls landscape and proper covariance.
#define FTD0612_NO_MAIN
#include "test_uniform_single_core_stationary_refinement.cpp"

namespace {

constexpr char landscape_protocol_sha256[] =
    "D409501414737F70D884A553CA05E86200EA42876854FCFD834BE04581493D82";
constexpr double locked_landscape_rest_energy = 0.0015517955076684577;
constexpr int landscape_intervals = 64;
constexpr int internal_dof = 6;
constexpr int internal_max_evaluations = 1500;
using InternalParameters = std::array<double, internal_dof>;

struct InternalVertex {
  InternalParameters point{};
  StaticCoreEvaluation evaluation{};
};

struct InternalSearch {
  bool admissible = false;
  bool terminated = false;
  int evaluations = 0;
  double diameter = INFINITY;
  double spread = INFINITY;
  StaticCoreEvaluation minimum{};
};

InternalParameters internal_affine(const InternalParameters& origin,
                                   const InternalParameters& other,
                                   double factor) {
  InternalParameters result{};
  for (int d = 0; d < internal_dof; ++d)
    result[d] = origin[d] + factor * (other[d] - origin[d]);
  return result;
}

StaticCoreEvaluation evaluate_internal_parameters(
    const Vec3& translation, const StaticCoreEvaluation& seed,
    const InternalParameters& point,
    const ftd::eft::ChargedTrimerOptions& options,
    const GreenKernel& green, double beta) {
  const Matrix3 orientation = multiply(rotation_exponential(
      {point[0], point[1], point[2]}), seed.orientation);
  Strain strain = seed.strain;
  for (int i = 0; i < 3; ++i) strain[i] += point[i + 3];
  return evaluate_static_core(
      translation, orientation, strain, options, green, beta);
}

InternalSearch search_internal_shape(
    const Vec3& translation, const StaticCoreEvaluation& seed,
    const ftd::eft::ChargedTrimerOptions& options,
    const GreenKernel& green, double beta) {
  InternalSearch result;
  std::array<InternalVertex, internal_dof + 1> simplex{};
  const auto evaluate = [&](const InternalParameters& point) {
    InternalVertex vertex;
    vertex.point = point;
    if (result.evaluations >= internal_max_evaluations) return vertex;
    vertex.evaluation = evaluate_internal_parameters(
        translation, seed, point, options, green, beta);
    ++result.evaluations;
    return vertex;
  };
  InternalParameters zero{};
  simplex[0] = evaluate(zero);
  result.admissible = simplex[0].evaluation.valid;
  for (int d = 0; d < internal_dof; ++d) {
    InternalParameters point{};
    point[d] = d < 3 ? 0.02 : 0.01;
    simplex[static_cast<std::size_t>(d + 1)] = evaluate(point);
  }
  const auto score = [](const InternalVertex& vertex) {
    return vertex.evaluation.valid ? vertex.evaluation.energy : 1e100;
  };
  while (result.evaluations < internal_max_evaluations) {
    std::sort(simplex.begin(), simplex.end(), [&](const InternalVertex& a,
                                                   const InternalVertex& b) {
      return score(a) < score(b);
    });
    result.diameter = 0.0;
    for (std::size_t i = 1; i < simplex.size(); ++i)
      for (int d = 0; d < internal_dof; ++d)
        result.diameter = std::max(result.diameter,
            std::abs(simplex[i].point[d] - simplex[0].point[d]));
    result.spread = std::abs(score(simplex.back()) - score(simplex.front()));
    if (result.diameter <= 1e-8 && result.spread <= 1e-14) {
      result.terminated = true;
      break;
    }
    InternalParameters centroid{};
    for (int i = 0; i < internal_dof; ++i)
      for (int d = 0; d < internal_dof; ++d)
        centroid[d] += simplex[static_cast<std::size_t>(i)].point[d]
            / static_cast<double>(internal_dof);
    const auto reflected = evaluate(internal_affine(
        centroid, simplex.back().point, -1.0));
    if (score(reflected) < score(simplex.front())) {
      const auto expanded = evaluate(internal_affine(
          centroid, reflected.point, 2.0));
      simplex.back() = score(expanded) < score(reflected)
          ? expanded : reflected;
    } else if (score(reflected) < score(simplex[internal_dof - 1])) {
      simplex.back() = reflected;
    } else {
      const bool outside = score(reflected) < score(simplex.back());
      const auto target = outside ? reflected.point : simplex.back().point;
      const auto contracted = evaluate(internal_affine(
          centroid, target, 0.5));
      if (score(contracted) < (outside ? score(reflected)
                                      : score(simplex.back()))) {
        simplex.back() = contracted;
      } else {
        for (std::size_t i = 1; i < simplex.size()
             && result.evaluations < internal_max_evaluations; ++i)
          simplex[i] = evaluate(internal_affine(
              simplex[0].point, simplex[i].point, 0.5));
      }
    }
  }
  std::sort(simplex.begin(), simplex.end(), [&](const InternalVertex& a,
                                                 const InternalVertex& b) {
    return score(a) < score(b);
  });
  result.minimum = simplex.front().evaluation;
  return result;
}

StaticCoreEvaluation lower_internal_minimum(
    const Vec3& translation, const StaticCoreEvaluation& base,
    const StaticCoreEvaluation& continuation,
    const ftd::eft::ChargedTrimerOptions& options,
    const GreenKernel& green, double beta, bool& complete,
    int& evaluations) {
  const auto from_base = search_internal_shape(
      translation, base, options, green, beta);
  const auto from_continuation = search_internal_shape(
      translation, continuation, options, green, beta);
  evaluations += from_base.evaluations + from_continuation.evaluations;
  complete = complete && from_base.terminated && from_base.minimum.valid
      && from_continuation.terminated && from_continuation.minimum.valid;
  if (!from_base.minimum.valid) return from_continuation.minimum;
  if (!from_continuation.minimum.valid) return from_base.minimum;
  return from_base.minimum.energy <= from_continuation.minimum.energy
      ? from_base.minimum : from_continuation.minimum;
}

Vec3 cycle_vec_0614(const Vec3& value, int turns = 1) {
  Vec3 result = value;
  for (int i = 0; i < turns; ++i) result = {result.y, result.z, result.x};
  return result;
}

Matrix3 cycle_matrix_0614(const Matrix3& value, int turns) {
  Matrix3 result = value;
  for (int turn = 0; turn < turns; ++turn) {
    Matrix3 next{};
    for (int column = 0; column < 3; ++column) {
      next[0][column] = result[1][column];
      next[1][column] = result[2][column];
      next[2][column] = result[0][column];
    }
    result = next;
  }
  return result;
}

StaticCoreEvaluation cycle_static_core_0614(
    const StaticCoreEvaluation& source, int turns,
    const ftd::eft::ChargedTrimerOptions& options,
    const GreenKernel& green, double beta) {
  const Vec3 center = center_b + source.translation;
  const Vec3 translation = cycle_vec_0614(center, turns) - center_b;
  return evaluate_static_core(translation,
      cycle_matrix_0614(source.orientation, turns), source.strain,
      options, green, beta);
}

struct LandscapePath {
  bool complete = false;
  int family = 0;
  int axis = 0;
  int sign = 1;
  int rotation = 0;
  int evaluations = 0;
  double rigid_barrier = INFINITY;
  double relaxed_barrier = INFINITY;
  double rigid_endpoint_residual = INFINITY;
  double relaxed_endpoint_residual = INFINITY;
  double hysteresis = INFINITY;
  double relaxation_excess = INFINITY;
  double threshold_momentum = INFINITY;
  double threshold_speed = INFINITY;
  double threshold_energy_residual = INFINITY;
  std::vector<double> rigid;
  std::vector<double> relaxed_forward;
  std::vector<double> relaxed_backward;
};

LandscapePath evaluate_landscape_path(
    int family, int axis, int sign, int rotation,
    const StaticCoreEvaluation& base,
    const ftd::eft::ChargedTrimerOptions& options,
    const GreenKernel& green, double beta) {
  LandscapePath path;
  path.family = family;
  path.axis = axis;
  path.sign = sign;
  path.rotation = rotation;
  path.rigid.resize(landscape_intervals + 1, INFINITY);
  path.relaxed_forward.resize(landscape_intervals + 1, INFINITY);
  path.relaxed_backward.resize(landscape_intervals + 1, INFINITY);
  const Vec3 direction = axis == 0 ? Vec3{static_cast<double>(sign),0,0}
      : (axis == 1 ? Vec3{0,static_cast<double>(sign),0}
                   : Vec3{0,0,static_cast<double>(sign)});
  bool complete = base.valid;
  StaticCoreEvaluation continuation = base;
  for (int j = 0; j <= landscape_intervals; ++j) {
    const double q = static_cast<double>(j) / landscape_intervals;
    const Vec3 translation = base.translation + direction * q;
    const auto rigid = evaluate_static_core(translation, base.orientation,
        base.strain, options, green, beta);
    complete = complete && rigid.valid;
    path.rigid[static_cast<std::size_t>(j)] = rigid.energy;
    if (j == 0) {
      path.relaxed_forward[0] = base.energy;
      continuation = base;
    } else {
      continuation = lower_internal_minimum(translation, base, continuation,
          options, green, beta, complete, path.evaluations);
      path.relaxed_forward[static_cast<std::size_t>(j)] = continuation.energy;
    }
  }
  continuation = evaluate_static_core(base.translation + direction,
      base.orientation, base.strain, options, green, beta);
  complete = complete && continuation.valid;
  path.relaxed_backward.back() = continuation.energy;
  for (int j = landscape_intervals - 1; j >= 0; --j) {
    const double q = static_cast<double>(j) / landscape_intervals;
    const Vec3 translation = base.translation + direction * q;
    continuation = lower_internal_minimum(translation, base, continuation,
        options, green, beta, complete, path.evaluations);
    path.relaxed_backward[static_cast<std::size_t>(j)] = continuation.energy;
  }
  const auto rigid_peak = std::max_element(path.rigid.begin(), path.rigid.end());
  const auto relaxed_peak = std::max_element(
      path.relaxed_forward.begin(), path.relaxed_forward.end());
  path.rigid_barrier = *rigid_peak - path.rigid.front();
  path.relaxed_barrier = *relaxed_peak - path.relaxed_forward.front();
  path.rigid_endpoint_residual = std::abs(
      path.rigid.back() - path.rigid.front());
  path.relaxed_endpoint_residual = std::abs(
      path.relaxed_forward.back() - path.relaxed_forward.front());
  path.hysteresis = 0.0;
  path.relaxation_excess = 0.0;
  for (int j = 0; j <= landscape_intervals; ++j) {
    path.hysteresis = std::max(path.hysteresis,
        std::abs(path.relaxed_forward[static_cast<std::size_t>(j)]
                 - path.relaxed_backward[static_cast<std::size_t>(j)]));
    path.relaxation_excess = std::max(path.relaxation_excess,
        path.relaxed_forward[static_cast<std::size_t>(j)]
        - path.rigid[static_cast<std::size_t>(j)]);
  }
  const double delta = path.relaxed_barrier / 3.0;
  if (delta >= 0.0 && std::isfinite(delta)) {
    path.threshold_momentum = std::sqrt(
        2.0 * ftd::E_REST * delta + delta * delta) / ftd::C_SPEED;
    path.threshold_speed = ftd::C_SPEED * ftd::C_SPEED
        * path.threshold_momentum / (ftd::E_REST + delta);
    const Vec3 p{path.threshold_momentum, 0.0, 0.0};
    path.threshold_energy_residual = std::abs(
        3.0 * (ftd::eft::production_flat_energy_from_momentum(p)
               - ftd::E_REST) - path.relaxed_barrier);
  }
  path.complete = complete && path.evaluations > 0
      && path.rigid_endpoint_residual <= 1e-12
      && path.relaxed_endpoint_residual <= 1e-12
      && path.hysteresis <= 1e-9
      && path.relaxation_excess <= 1e-12
      && path.threshold_energy_residual <= 1e-12
      && path.rigid_barrier >= 0.0 && path.relaxed_barrier >= 0.0;
  return path;
}

ChargedTrimerState cycle_state_once_0614(const ChargedTrimerState& source) {
  ChargedTrimerState target(L);
  target.charges = source.charges;
  for (std::size_t a = 0; a < source.constituents.size(); ++a) {
    target.constituents[a] = point_at(cycle_vec_0614(
        effective_position(source.constituents[a])));
    target.constituents[a].momentum = cycle_vec_0614(
        source.constituents[a].momentum);
  }
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int from = index(x, y, z);
        const int to = index(y, z, x);
        target.electric.x[static_cast<std::size_t>(to)] =
            source.electric.y[static_cast<std::size_t>(from)];
        target.electric.y[static_cast<std::size_t>(to)] =
            source.electric.z[static_cast<std::size_t>(from)];
        target.electric.z[static_cast<std::size_t>(to)] =
            source.electric.x[static_cast<std::size_t>(from)];
        target.magnetic_half.x[static_cast<std::size_t>(to)] =
            source.magnetic_half.y[static_cast<std::size_t>(from)];
        target.magnetic_half.y[static_cast<std::size_t>(to)] =
            source.magnetic_half.z[static_cast<std::size_t>(from)];
        target.magnetic_half.z[static_cast<std::size_t>(to)] =
            source.magnetic_half.x[static_cast<std::size_t>(from)];
      }
  return target;
}

ChargedTrimerState cycle_state_0614(
    const ChargedTrimerState& source, int turns) {
  ChargedTrimerState result = source;
  for (int i = 0; i < turns; ++i) result = cycle_state_once_0614(result);
  return result;
}

struct CovarianceArm {
  bool complete = false;
  int speed_index = 0;
  int sign = 1;
  int rotation = 0;
  double speed = 0.0;
  double worst_gate = INFINITY;
  double later_residual = INFINITY;
  double recovery = INFINITY;
};

CovarianceArm run_covariance_arm(
    int speed_index, int sign, int rotation, double speed,
    const ChargedTrimerState& rest,
    const std::vector<double>& uniform,
    const ftd::eft::ChargedTrimerOptions& options,
    const ChargedTrimerStepResult* base_forward) {
  CovarianceArm arm;
  arm.speed_index = speed_index;
  arm.sign = sign;
  arm.rotation = rotation;
  arm.speed = speed;
  ChargedTrimerState initial = cycle_state_0614(rest, rotation);
  const Vec3 direction = cycle_vec_0614(
      {static_cast<double>(sign), 0.0, 0.0}, rotation);
  const Vec3 momentum = ftd::eft::production_flat_momentum(direction * speed);
  for (auto& point : initial.constituents) point.momentum = momentum;
  const auto forward = ftd::eft::solve_charged_trimer_forward(
      initial, uniform, options);
  if (!forward.valid) return arm;
  const auto reverse = ftd::eft::solve_charged_trimer_reverse(
      forward.later, uniform, options);
  if (!reverse.valid) return arm;
  arm.worst_gate = std::max(single_maximum_gate(forward),
                            single_maximum_gate(reverse));
  arm.recovery = ftd::eft::charged_trimer_state_max_difference(
      initial, reverse.earlier);
  if (rotation == 0) arm.later_residual = 0.0;
  else if (base_forward)
    arm.later_residual = ftd::eft::charged_trimer_state_max_difference(
        cycle_state_0614(base_forward->later, rotation), forward.later);
  arm.complete = arm.worst_gate <= gate && arm.recovery <= 1e-10
      && arm.later_residual <= 1e-10;
  return arm;
}

struct LandscapeSummary {
  bool rest_fingerprint = false;
  bool rest_gate = false;
  bool path_coverage = false;
  bool landscape_covariance = false;
  bool dynamic_covariance = false;
  double maximum_landscape_covariance_residual = INFINITY;
  double maximum_dynamic_covariance_residual = INFINITY;
  double minimum_relaxed_barrier = INFINITY;
  double maximum_relaxed_barrier = 0.0;
  RefineResult refined{};
  SingleArm rest{};
  std::vector<LandscapePath> paths;
  std::vector<CovarianceArm> covariance_arms;
  std::string verdict;
};

void write_landscape_record(const LandscapeSummary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0614";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0614_refined_core_peierls_landscape_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0614\",\n"
       << "  \"protocol_sha256\": \"" << landscape_protocol_sha256
       << "\",\n  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"rest_fingerprint_pass\": "
       << (summary.rest_fingerprint ? "true" : "false") << ",\n"
       << "  \"rest_gate_pass\": "
       << (summary.rest_gate ? "true" : "false") << ",\n"
       << "  \"path_coverage\": "
       << (summary.path_coverage ? "true" : "false") << ",\n"
       << "  \"landscape_covariance_pass\": "
       << (summary.landscape_covariance ? "true" : "false") << ",\n"
       << "  \"dynamic_covariance_pass\": "
       << (summary.dynamic_covariance ? "true" : "false") << ",\n"
       << "  \"refined_energy\": " << summary.refined.state.energy << ",\n"
       << "  \"maximum_landscape_covariance_residual\": "
       << json_number(summary.maximum_landscape_covariance_residual) << ",\n"
       << "  \"maximum_dynamic_covariance_residual\": "
       << json_number(summary.maximum_dynamic_covariance_residual) << ",\n"
       << "  \"minimum_relaxed_barrier\": "
       << json_number(summary.minimum_relaxed_barrier) << ",\n"
       << "  \"maximum_relaxed_barrier\": "
       << json_number(summary.maximum_relaxed_barrier) << ",\n"
       << "  \"paths\": [\n";
  for (std::size_t i = 0; i < summary.paths.size(); ++i) {
    const auto& path = summary.paths[i];
    json << "    {\"family\": " << path.family
         << ", \"axis\": " << path.axis << ", \"sign\": " << path.sign
         << ", \"rotation\": " << path.rotation
         << ", \"complete\": " << (path.complete ? "true" : "false")
         << ", \"evaluations\": " << path.evaluations
         << ", \"rigid_barrier\": " << json_number(path.rigid_barrier)
         << ", \"relaxed_barrier\": " << json_number(path.relaxed_barrier)
         << ", \"rigid_endpoint_residual\": "
         << json_number(path.rigid_endpoint_residual)
         << ", \"relaxed_endpoint_residual\": "
         << json_number(path.relaxed_endpoint_residual)
         << ", \"hysteresis\": " << json_number(path.hysteresis)
         << ", \"relaxation_excess\": "
         << json_number(path.relaxation_excess)
         << ", \"threshold_momentum\": "
         << json_number(path.threshold_momentum)
         << ", \"threshold_speed\": "
         << json_number(path.threshold_speed)
         << ", \"threshold_energy_residual\": "
         << json_number(path.threshold_energy_residual) << "}"
         << (i + 1 == summary.paths.size() ? "\n" : ",\n");
  }
  json << "  ],\n  \"covariance_arms\": [\n";
  for (std::size_t i = 0; i < summary.covariance_arms.size(); ++i) {
    const auto& arm = summary.covariance_arms[i];
    json << "    {\"speed_index\": " << arm.speed_index
         << ", \"sign\": " << arm.sign << ", \"rotation\": "
         << arm.rotation << ", \"speed\": " << arm.speed
         << ", \"complete\": " << (arm.complete ? "true" : "false")
         << ", \"worst_gate\": " << json_number(arm.worst_gate)
         << ", \"later_residual\": " << json_number(arm.later_residual)
         << ", \"recovery\": " << json_number(arm.recovery) << "}"
         << (i + 1 == summary.covariance_arms.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";

  std::ofstream csv(dir / "ftd_0614_refined_core_peierls_samples_v1.csv");
  csv << "ftd_id,family,axis,sign,rotation,phase,rigid_energy,"
         "relaxed_forward_energy,relaxed_backward_energy\n";
  for (const auto& path : summary.paths)
    for (int j = 0; j <= landscape_intervals; ++j)
      csv << std::setprecision(17) << "FTD-0614," << path.family << ','
          << path.axis << ',' << path.sign << ',' << path.rotation << ','
          << static_cast<double>(j) / landscape_intervals << ','
          << path.rigid[static_cast<std::size_t>(j)] << ','
          << path.relaxed_forward[static_cast<std::size_t>(j)] << ','
          << path.relaxed_backward[static_cast<std::size_t>(j)] << '\n';
}

}  // namespace

#ifndef FTD0614_NO_MAIN
int main() {
  std::cout << std::setprecision(17);
  LandscapeSummary summary;
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
  for (const auto& search : searches)
    if (search.terminated && search.minimum.valid
        && (!initial.valid || search.minimum.energy < initial.energy))
      initial = search.minimum;
  summary.refined = initial.valid
      ? refine_static_state(initial, options, green, beta) : RefineResult{};
  summary.rest_fingerprint = searches.size() == 16
      && summary.refined.coverage && summary.refined.converged
      && std::abs(summary.refined.state.energy
                  - locked_landscape_rest_energy) <= 1e-15
      && summary.refined.derivatives.gradient_inf <= 1e-10
      && summary.refined.derivatives.positive_modes == static_dof;

  const std::size_t count = static_cast<std::size_t>(L) * L * L;
  std::vector<double> uniform(count, -1.0 / static_cast<double>(count));
  ChargedTrimerState rest_state = summary.refined.state.state;
  if (summary.rest_fingerprint) {
    auto total = coat_density(rest_state);
    if (total.size() == count) {
      for (std::size_t i = 0; i < count; ++i) total[i] += uniform[i];
      const auto direct = initialize_minimum_energy(total);
      if (direct.valid) rest_state.electric = direct.electric;
      NeutralizerFixture fixture;
      fixture.name = "landscape_rest";
      fixture.state = rest_state;
      fixture.stationary = uniform;
      fixture.valid = direct.valid;
      if (direct.valid)
        summary.rest = run_arm(0, 0, fixture, 0.0, 64, options);
      summary.rest_gate = direct.valid && summary.rest.complete
          && summary.rest.worst_gate <= gate
          && summary.rest.energy_drift <= 1e-10
          && std::abs(summary.rest.longitudinal) <= 1e-9
          && summary.rest.transverse <= 1e-9
          && summary.rest.momentum_change <= 1e-9
          && summary.rest.recovery <= 1e-9;
    }
  }

  if (summary.rest_gate) {
    for (int axis = 0; axis < 3; ++axis)
      for (int sign : {-1, +1})
        summary.paths.push_back(evaluate_landscape_path(
            0, axis, sign, 0, summary.refined.state,
            options, green, beta));
    for (int rotation : {1, 2}) {
      const auto rotated = cycle_static_core_0614(
          summary.refined.state, rotation, options, green, beta);
      const Vec3 direction = cycle_vec_0614({1,0,0}, rotation);
      const int axis = std::abs(direction.x) > 0.5 ? 0
          : (std::abs(direction.y) > 0.5 ? 1 : 2);
      for (int sign : {-1, +1})
        summary.paths.push_back(evaluate_landscape_path(
            1, axis, sign, rotation, rotated,
            options, green, beta));
    }
  }
  summary.path_coverage = summary.paths.size() == 10
      && std::all_of(summary.paths.begin(), summary.paths.end(),
          [](const LandscapePath& path) { return path.complete; });
  summary.minimum_relaxed_barrier = INFINITY;
  summary.maximum_relaxed_barrier = 0.0;
  for (const auto& path : summary.paths) {
    summary.minimum_relaxed_barrier = std::min(
        summary.minimum_relaxed_barrier, path.relaxed_barrier);
    summary.maximum_relaxed_barrier = std::max(
        summary.maximum_relaxed_barrier, path.relaxed_barrier);
  }

  summary.maximum_landscape_covariance_residual = 0.0;
  bool covariance_coverage = summary.path_coverage;
  for (int sign : {-1, +1}) {
    const auto base = std::find_if(summary.paths.begin(), summary.paths.end(),
        [&](const LandscapePath& path) {
          return path.family == 0 && path.axis == 0 && path.sign == sign;
        });
    if (base == summary.paths.end()) { covariance_coverage = false; continue; }
    for (int rotation : {1, 2}) {
      const auto rotated = std::find_if(summary.paths.begin(), summary.paths.end(),
          [&](const LandscapePath& path) {
            return path.family == 1 && path.rotation == rotation
                && path.sign == sign;
          });
      if (rotated == summary.paths.end()) {
        covariance_coverage = false;
        continue;
      }
      for (int j = 0; j <= landscape_intervals; ++j)
        summary.maximum_landscape_covariance_residual = std::max(
            summary.maximum_landscape_covariance_residual,
            std::max(std::abs(base->rigid[static_cast<std::size_t>(j)]
                              - rotated->rigid[static_cast<std::size_t>(j)]),
                     std::abs(base->relaxed_forward[static_cast<std::size_t>(j)]
                              - rotated->relaxed_forward[static_cast<std::size_t>(j)])));
    }
  }
  summary.landscape_covariance = covariance_coverage
      && summary.maximum_landscape_covariance_residual <= 1e-12;

  const std::array<double, 2> speeds{{1.0/64.0, 1.0/32.0}};
  if (summary.rest_gate) {
    for (int speed_index = 0; speed_index < 2; ++speed_index)
      for (int sign : {-1, +1}) {
        ChargedTrimerState base_initial = rest_state;
        const Vec3 launch = ftd::eft::production_flat_momentum(
            {sign * speeds[speed_index], 0.0, 0.0});
        for (auto& point : base_initial.constituents) point.momentum = launch;
        const auto base_forward = ftd::eft::solve_charged_trimer_forward(
            base_initial, uniform, options);
        for (int rotation = 0; rotation < 3; ++rotation)
          summary.covariance_arms.push_back(run_covariance_arm(
              speed_index, sign, rotation, speeds[speed_index], rest_state,
              uniform, options, base_forward.valid ? &base_forward : nullptr));
      }
  }
  summary.maximum_dynamic_covariance_residual = 0.0;
  for (const auto& arm : summary.covariance_arms)
    summary.maximum_dynamic_covariance_residual = std::max(
        summary.maximum_dynamic_covariance_residual, arm.later_residual);
  summary.dynamic_covariance = summary.covariance_arms.size() == 12
      && std::all_of(summary.covariance_arms.begin(),
          summary.covariance_arms.end(),
          [](const CovarianceArm& arm) { return arm.complete; });

  const bool coverage = summary.rest_fingerprint && summary.rest_gate
      && summary.path_coverage && summary.landscape_covariance
      && summary.dynamic_covariance;
  if (!coverage)
    summary.verdict =
        "REFINED_CORE_PEIERLS_LANDSCAPE_NUMERICALLY_UNRESOLVED";
  else if (summary.minimum_relaxed_barrier <= 1e-12)
    summary.verdict = "REFINED_CORE_REGISTERED_PASSIVE_PATH_GAPLESS";
  else
    summary.verdict =
        "REFINED_CORE_SELECTED_PATH_BARRIER_AND_COVARIANCE_RESOLVED";
  write_landscape_record(summary);
  std::cout << "protocol_sha256=" << landscape_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "rest=" << summary.rest_gate
            << " paths=" << summary.paths.size()
            << " path_coverage=" << summary.path_coverage << '\n'
            << "barrier_range=" << summary.minimum_relaxed_barrier << ','
            << summary.maximum_relaxed_barrier << '\n'
            << "landscape_covariance="
            << summary.maximum_landscape_covariance_residual
            << " dynamic_covariance="
            << summary.maximum_dynamic_covariance_residual << '\n';
  for (const auto& path : summary.paths)
    std::cout << "path family=" << path.family << " axis=" << path.axis
              << " sign=" << path.sign << " rotation=" << path.rotation
              << " rigid=" << path.rigid_barrier
              << " relaxed=" << path.relaxed_barrier
              << " v_threshold=" << path.threshold_speed
              << " hysteresis=" << path.hysteresis
              << " complete=" << path.complete << '\n';
  return summary.paths.size() == 10
      && summary.covariance_arms.size() == 12 ? 0 : 1;
}
#endif
