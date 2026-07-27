/** FTD-0606: global SO(3) x local-strain compact matter-core discriminator. */

// Reuse the locked FTD-0605 observer utilities without changing its runner.
#define main ftd0605_embedded_main
#include "test_full_mirrored_internal_shape_core.cpp"
#undef main

namespace {

constexpr const char* global_protocol_sha256 =
    "EC0CECED1CCF40187BCE0C4B38DA34039B5CAD94069AFD05F16420D25D99494A";
constexpr int global_starts = 24;
constexpr int global_max_evaluations = 1500;
constexpr double strain_basin = 0.20;

using Matrix3 = std::array<std::array<double, 3>, 3>;
using Strain = std::array<double, 3>;
using Parameters = std::array<double, 6>;

Matrix3 identity_matrix() {
  return {{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}};
}

Matrix3 multiply(const Matrix3& lhs, const Matrix3& rhs) {
  Matrix3 result{};
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      for (int k = 0; k < 3; ++k)
        result[i][j] += lhs[i][k] * rhs[k][j];
  return result;
}

Vec3 apply(const Matrix3& matrix, const Vec3& vector) {
  return {
      matrix[0][0] * vector.x + matrix[0][1] * vector.y
          + matrix[0][2] * vector.z,
      matrix[1][0] * vector.x + matrix[1][1] * vector.y
          + matrix[1][2] * vector.z,
      matrix[2][0] * vector.x + matrix[2][1] * vector.y
          + matrix[2][2] * vector.z};
}

double determinant(const Matrix3& matrix) {
  return matrix[0][0] * (matrix[1][1] * matrix[2][2]
                         - matrix[1][2] * matrix[2][1])
      - matrix[0][1] * (matrix[1][0] * matrix[2][2]
                        - matrix[1][2] * matrix[2][0])
      + matrix[0][2] * (matrix[1][0] * matrix[2][1]
                        - matrix[1][1] * matrix[2][0]);
}

double orthogonality_residual(const Matrix3& matrix) {
  double residual = std::abs(determinant(matrix) - 1.0);
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j) {
      double value = 0.0;
      for (int k = 0; k < 3; ++k) value += matrix[k][i] * matrix[k][j];
      residual = std::max(residual,
          std::abs(value - (i == j ? 1.0 : 0.0)));
    }
  return residual;
}

Matrix3 rotation_exponential(const Vec3& omega) {
  const double theta = omega.mag();
  if (theta < 1e-14) {
    Matrix3 result = identity_matrix();
    result[0][1] -= omega.z;
    result[0][2] += omega.y;
    result[1][0] += omega.z;
    result[1][2] -= omega.x;
    result[2][0] -= omega.y;
    result[2][1] += omega.x;
    return result;
  }
  const Vec3 axis = omega * (1.0 / theta);
  const double c = std::cos(theta);
  const double s = std::sin(theta);
  const double t = 1.0 - c;
  return {{{
      c + axis.x * axis.x * t,
      axis.x * axis.y * t - axis.z * s,
      axis.x * axis.z * t + axis.y * s}, {
      axis.y * axis.x * t + axis.z * s,
      c + axis.y * axis.y * t,
      axis.y * axis.z * t - axis.x * s}, {
      axis.z * axis.x * t - axis.y * s,
      axis.z * axis.y * t + axis.x * s,
      c + axis.z * axis.z * t}}};
}

std::array<Matrix3, global_starts> cubic_rotations() {
  std::array<Matrix3, global_starts> result{};
  std::array<int, 3> permutation{{0, 1, 2}};
  int count = 0;
  do {
    for (int sx : {-1, 1})
      for (int sy : {-1, 1})
        for (int sz : {-1, 1}) {
          const std::array<int, 3> sign{{sx, sy, sz}};
          Matrix3 matrix{};
          for (int column = 0; column < 3; ++column)
            matrix[permutation[column]][column] = sign[column];
          if (determinant(matrix) > 0.5)
            result[static_cast<std::size_t>(count++)] = matrix;
        }
  } while (std::next_permutation(permutation.begin(), permutation.end()));
  if (count != global_starts) throw std::runtime_error("cubic rotation count");
  return result;
}

struct BodyFrame {
  Vec3 e0{};
  Vec3 e1{};
  std::array<std::array<double, 2>, 3> coordinates{};
  double reconstruction_residual = INFINITY;
};

BodyFrame make_body_frame() {
  BodyFrame frame;
  frame.e0 = reference_offsets[0]
      * (1.0 / reference_offsets[0].mag());
  const Vec3 residual = reference_offsets[1]
      - frame.e0 * reference_offsets[1].dot(frame.e0);
  frame.e1 = residual * (1.0 / residual.mag());
  frame.reconstruction_residual = 0.0;
  for (std::size_t a = 0; a < 3; ++a) {
    frame.coordinates[a] = {{reference_offsets[a].dot(frame.e0),
                             reference_offsets[a].dot(frame.e1)}};
    const Vec3 rebuilt = frame.e0 * frame.coordinates[a][0]
        + frame.e1 * frame.coordinates[a][1];
    frame.reconstruction_residual = std::max(
        frame.reconstruction_residual,
        (rebuilt - reference_offsets[a]).mag());
  }
  return frame;
}

const BodyFrame body_frame = make_body_frame();

double strain_max_abs(const Strain& strain) {
  return std::max({std::abs(strain[0]), std::abs(strain[1]),
                   std::abs(strain[2])});
}

double strain_minimum_eigenvalue(const Strain& strain) {
  const double a = 1.0 + strain[0];
  const double d = 1.0 + strain[2];
  const double radius = std::sqrt(
      0.25 * (a - d) * (a - d) + strain[1] * strain[1]);
  return 0.5 * (a + d) - radius;
}

std::array<Vec3, 3> global_offsets(const Matrix3& orientation,
                                   const Strain& strain) {
  std::array<Vec3, 3> offsets{};
  const double a00 = 1.0 + strain[0];
  const double a01 = strain[1];
  const double a11 = 1.0 + strain[2];
  for (std::size_t particle = 0; particle < 3; ++particle) {
    const double x = body_frame.coordinates[particle][0];
    const double y = body_frame.coordinates[particle][1];
    const Vec3 strained = body_frame.e0 * (a00 * x + a01 * y)
        + body_frame.e1 * (a01 * x + a11 * y);
    offsets[particle] = apply(orientation, strained);
  }
  return offsets;
}

ClosedNeutralTrimerPairState make_global_state(
    double phase, const Matrix3& orientation, const Strain& strain) {
  ClosedNeutralTrimerPairState state(L);
  const Vec3 shift{phase, 0.0, 0.0};
  const auto offsets = global_offsets(orientation, strain);
  for (std::size_t a = 0; a < 3; ++a) {
    state.constituents[a] = point_at(center_a + shift + offsets[a]);
    state.constituents[a + 3] = point_at(center_b + shift - offsets[a]);
  }
  return state;
}

std::pair<double, double> global_distance_range(
    const Matrix3& orientation, const Strain& strain) {
  const auto offsets = global_offsets(orientation, strain);
  double minimum = INFINITY;
  double maximum = 0.0;
  for (std::size_t a = 0; a < 3; ++a)
    for (std::size_t b = a + 1; b < 3; ++b) {
      const double distance = (offsets[a] - offsets[b]).mag();
      minimum = std::min(minimum, distance);
      maximum = std::max(maximum, distance);
    }
  return {minimum, maximum};
}

struct GlobalEvaluation {
  bool valid = false;
  Matrix3 orientation = identity_matrix();
  Strain strain{};
  double binding_energy = INFINITY;
  double field_energy = INFINITY;
  double total_energy = INFINITY;
  double minimum_distance = 0.0;
  double maximum_distance = INFINITY;
  ClosedNeutralTrimerPairState state{L};
};

GlobalEvaluation evaluate_global(double phase, const Matrix3& orientation,
                                 const Strain& strain,
                                 const ClosedNeutralPairOptions& options,
                                 const GreenKernel& green, double beta) {
  GlobalEvaluation result;
  result.orientation = orientation;
  result.strain = strain;
  if (strain_max_abs(strain) > strain_basin
      || strain_minimum_eigenvalue(strain) < 0.70
      || orthogonality_residual(orientation) > 1e-11) return result;
  const auto distances = global_distance_range(orientation, strain);
  result.minimum_distance = distances.first;
  result.maximum_distance = distances.second;
  if (result.minimum_distance < 0.5 || result.maximum_distance > 2.0)
    return result;
  result.state = make_global_state(phase, orientation, strain);
  const auto dense = density_of(result.state);
  if (dense.empty()) return result;
  result.binding_energy = ftd::eft::closed_neutral_pair_binding_energy(
      result.state, options);
  result.field_energy = beta * green_energy(sparse_density(dense), green);
  result.total_energy = result.binding_energy + result.field_energy;
  result.valid = std::isfinite(result.total_energy);
  return result;
}

GlobalEvaluation evaluate_parameters(
    double phase, const Matrix3& start, const Parameters& parameters,
    const ClosedNeutralPairOptions& options, const GreenKernel& green,
    double beta) {
  const Matrix3 orientation = multiply(rotation_exponential(
      {parameters[0], parameters[1], parameters[2]}), start);
  const Strain strain{{parameters[3], parameters[4], parameters[5]}};
  return evaluate_global(phase, orientation, strain, options, green, beta);
}

struct GlobalVertex {
  Parameters point{};
  GlobalEvaluation evaluation{};
};

struct GlobalSearchResult {
  bool terminated = false;
  int evaluations = 0;
  double diameter = INFINITY;
  double energy_spread = INFINITY;
  GlobalEvaluation minimum{};
};

Parameters global_affine(const Parameters& origin, const Parameters& other,
                         double factor) {
  Parameters result{};
  for (int d = 0; d < 6; ++d)
    result[d] = origin[d] + factor * (other[d] - origin[d]);
  return result;
}

GlobalSearchResult search_from(double phase, const Matrix3& start,
                               const ClosedNeutralPairOptions& options,
                               const GreenKernel& green, double beta) {
  GlobalSearchResult result;
  std::array<GlobalVertex, 7> simplex{};
  const auto evaluate = [&](const Parameters& point) {
    GlobalVertex vertex;
    vertex.point = point;
    if (result.evaluations >= global_max_evaluations) return vertex;
    vertex.evaluation = evaluate_parameters(
        phase, start, point, options, green, beta);
    ++result.evaluations;
    return vertex;
  };
  Parameters zero{};
  simplex[0] = evaluate(zero);
  for (int d = 0; d < 6; ++d) {
    Parameters point{};
    point[d] = d < 3 ? 0.03 : 0.01;
    simplex[static_cast<std::size_t>(d + 1)] = evaluate(point);
  }
  const auto score = [](const GlobalVertex& vertex) {
    return vertex.evaluation.valid
        ? vertex.evaluation.total_energy : 1e100;
  };
  while (result.evaluations < global_max_evaluations) {
    std::sort(simplex.begin(), simplex.end(), [&](const GlobalVertex& a,
                                                   const GlobalVertex& b) {
      return score(a) < score(b);
    });
    result.diameter = 0.0;
    for (std::size_t i = 1; i < simplex.size(); ++i)
      for (int d = 0; d < 6; ++d)
        result.diameter = std::max(result.diameter,
            std::abs(simplex[i].point[d] - simplex[0].point[d]));
    result.energy_spread = std::abs(score(simplex.back())
                                    - score(simplex.front()));
    if (result.diameter <= 1e-7 && result.energy_spread <= 1e-14) {
      result.terminated = true;
      break;
    }
    Parameters centroid{};
    for (int i = 0; i < 6; ++i)
      for (int d = 0; d < 6; ++d)
        centroid[d] += simplex[static_cast<std::size_t>(i)].point[d] / 6.0;
    const GlobalVertex reflected = evaluate(global_affine(
        centroid, simplex.back().point, -1.0));
    if (score(reflected) < score(simplex.front())) {
      const GlobalVertex expanded = evaluate(global_affine(
          centroid, reflected.point, 2.0));
      simplex.back() = score(expanded) < score(reflected)
          ? expanded : reflected;
    } else if (score(reflected) < score(simplex[5])) {
      simplex.back() = reflected;
    } else {
      const bool outside = score(reflected) < score(simplex.back());
      const Parameters target = outside ? reflected.point
                                        : simplex.back().point;
      const GlobalVertex contracted = evaluate(global_affine(
          centroid, target, 0.5));
      if (score(contracted) < (outside ? score(reflected)
                                      : score(simplex.back()))) {
        simplex.back() = contracted;
      } else {
        for (std::size_t i = 1;
             i < simplex.size()
             && result.evaluations < global_max_evaluations; ++i)
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

struct GlobalDifferential {
  bool valid = false;
  double gradient_inf = INFINITY;
  std::array<double, 6> eigenvalues{};
  double minimum_eigenvalue = -INFINITY;
  int positive_modes = 0;
};

GlobalEvaluation evaluate_tangent(
    double phase, const GlobalEvaluation& base, const Parameters& tangent,
    const ClosedNeutralPairOptions& options, const GreenKernel& green,
    double beta) {
  const Matrix3 orientation = multiply(rotation_exponential(
      {tangent[0], tangent[1], tangent[2]}), base.orientation);
  Strain strain = base.strain;
  for (int i = 0; i < 3; ++i) strain[i] += tangent[i + 3];
  return evaluate_global(phase, orientation, strain, options, green, beta);
}

GlobalDifferential differentiate_global(
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
    const auto fp = evaluate_tangent(
        phase, minimum, plus, options, green, beta);
    const auto fm = evaluate_tangent(
        phase, minimum, minus, options, green, beta);
    if (!fp.valid || !fm.valid) return result;
    result.gradient_inf = std::max(result.gradient_inf,
        std::abs(fp.total_energy - fm.total_energy) / (2.0 * hg));
    plus = {};
    minus = {};
    plus[i] = hh;
    minus[i] = -hh;
    const auto hp = evaluate_tangent(
        phase, minimum, plus, options, green, beta);
    const auto hm = evaluate_tangent(
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
      const auto fpp = evaluate_tangent(
          phase, minimum, pp, options, green, beta);
      const auto fpm = evaluate_tangent(
          phase, minimum, pm, options, green, beta);
      const auto fmp = evaluate_tangent(
          phase, minimum, mp, options, green, beta);
      const auto fmm = evaluate_tangent(
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

struct GlobalPhaseRecord {
  int phase_index = 0;
  int terminated_starts = 0;
  int clustered_starts = 0;
  int total_evaluations = 0;
  int duplicate_anchor_pairs = 0;
  double phase = 0.0;
  Strain strain{};
  double reference_energy = INFINITY;
  double relaxed_energy = INFINITY;
  double gradient_inf = INFINITY;
  double minimum_eigenvalue = -INFINITY;
  int positive_modes = 0;
  double minimum_distance = 0.0;
  double maximum_distance = INFINITY;
  double field_gate = INFINITY;
  double common_gate = INFINITY;
  double inverse = INFINITY;
  double inward_impulse = NAN;
  double separation_decrease = NAN;
  bool coverage = false;
  bool stable = false;
  bool attractive = false;
};

struct GlobalSummary {
  bool algebra_pass = false;
  bool green_pass = false;
  bool coverage_pass = true;
  bool stationary_core_pass = true;
  bool field_pass = true;
  bool common_pass = true;
  bool inverse_pass = true;
  bool periodicity_pass = false;
  bool attraction_robust = true;
  int phase_arms = 0;
  int forward_arms = 0;
  int reverse_arms = 0;
  int attractive_phases = 0;
  int site_projection_collision_phases = 0;
  int maximum_duplicate_anchor_pairs = 0;
  double algebra_residual = INFINITY;
  double green_residual = INFINITY;
  double worst_gradient = 0.0;
  double minimum_hessian_eigenvalue = INFINITY;
  int minimum_positive_modes = 6;
  double maximum_strain = 0.0;
  double worst_field_gate = 0.0;
  double worst_common_gate = 0.0;
  double worst_inverse = 0.0;
  double minimum_inward_impulse = INFINITY;
  double minimum_separation_decrease = INFINITY;
  double reference_barrier = NAN;
  double relaxed_barrier = NAN;
  double periodicity_energy_residual = INFINITY;
  double periodicity_state_residual = INFINITY;
  std::vector<GlobalPhaseRecord> phases{};
  std::string verdict;
};

void write_global_record(const GlobalSummary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0606";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0606_global_orientation_strain_core_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0606\",\n"
       << "  \"protocol_sha256\": \"" << global_protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"phase_arms\": " << summary.phase_arms << ",\n"
       << "  \"forward_arms\": " << summary.forward_arms << ",\n"
       << "  \"reverse_arms\": " << summary.reverse_arms << ",\n"
       << "  \"attractive_phases\": " << summary.attractive_phases << ",\n"
       << "  \"site_projection_collision_phases\": "
       << summary.site_projection_collision_phases << ",\n"
       << "  \"maximum_duplicate_anchor_pairs\": "
       << summary.maximum_duplicate_anchor_pairs << ",\n"
       << "  \"algebra_pass\": " << (summary.algebra_pass ? "true" : "false") << ",\n"
       << "  \"green_pass\": " << (summary.green_pass ? "true" : "false") << ",\n"
       << "  \"coverage_pass\": " << (summary.coverage_pass ? "true" : "false") << ",\n"
       << "  \"stationary_core_pass\": " << (summary.stationary_core_pass ? "true" : "false") << ",\n"
       << "  \"field_pass\": " << (summary.field_pass ? "true" : "false") << ",\n"
       << "  \"common_pass\": " << (summary.common_pass ? "true" : "false") << ",\n"
       << "  \"inverse_pass\": " << (summary.inverse_pass ? "true" : "false") << ",\n"
       << "  \"periodicity_pass\": " << (summary.periodicity_pass ? "true" : "false") << ",\n"
       << "  \"attraction_robust\": " << (summary.attraction_robust ? "true" : "false") << ",\n"
       << "  \"algebra_residual\": " << summary.algebra_residual << ",\n"
       << "  \"green_residual\": " << summary.green_residual << ",\n"
       << "  \"worst_gradient\": " << summary.worst_gradient << ",\n"
       << "  \"minimum_hessian_eigenvalue\": " << json_number(summary.minimum_hessian_eigenvalue) << ",\n"
       << "  \"minimum_positive_modes\": " << summary.minimum_positive_modes << ",\n"
       << "  \"maximum_strain\": " << summary.maximum_strain << ",\n"
       << "  \"worst_field_gate\": " << summary.worst_field_gate << ",\n"
       << "  \"worst_common_gate\": " << summary.worst_common_gate << ",\n"
       << "  \"worst_inverse\": " << summary.worst_inverse << ",\n"
       << "  \"minimum_inward_impulse\": " << json_number(summary.minimum_inward_impulse) << ",\n"
       << "  \"minimum_separation_decrease\": " << json_number(summary.minimum_separation_decrease) << ",\n"
       << "  \"reference_barrier\": " << json_number(summary.reference_barrier) << ",\n"
       << "  \"relaxed_barrier\": " << json_number(summary.relaxed_barrier) << ",\n"
       << "  \"periodicity_energy_residual\": " << json_number(summary.periodicity_energy_residual) << ",\n"
       << "  \"periodicity_state_residual\": " << json_number(summary.periodicity_state_residual) << "\n}\n";
  std::ofstream csv(dir / "ftd_0606_global_orientation_strain_core_samples_v1.csv");
  csv << "ftd_id,phase_index,phase,terminated_starts,clustered_starts,total_evaluations,duplicate_anchor_pairs,"
         "h0,h1,h2,reference_energy,relaxed_energy,gradient_inf,minimum_eigenvalue,"
         "positive_modes,minimum_distance,maximum_distance,field_gate,common_gate,"
         "inverse,inward_impulse,separation_decrease,coverage,stable,attractive\n";
  for (const auto& phase : summary.phases) {
    csv << std::setprecision(17) << "FTD-0606," << phase.phase_index << ','
        << phase.phase << ',' << phase.terminated_starts << ','
        << phase.clustered_starts << ',' << phase.total_evaluations << ','
        << phase.duplicate_anchor_pairs << ','
        << phase.strain[0] << ',' << phase.strain[1] << ',' << phase.strain[2]
        << ',' << phase.reference_energy << ',' << phase.relaxed_energy << ','
        << phase.gradient_inf << ',' << phase.minimum_eigenvalue << ','
        << phase.positive_modes << ',' << phase.minimum_distance << ','
        << phase.maximum_distance << ',' << phase.field_gate << ','
        << phase.common_gate << ',' << phase.inverse << ','
        << phase.inward_impulse << ',' << phase.separation_decrease << ','
        << phase.coverage << ',' << phase.stable << ',' << phase.attractive
        << '\n';
  }
}

int duplicate_anchor_pairs(const ClosedNeutralTrimerPairState& state) {
  int duplicates = 0;
  for (std::size_t a = 0; a < state.constituents.size(); ++a)
    for (std::size_t b = a + 1; b < state.constituents.size(); ++b) {
      const auto& lhs = state.constituents[a].anchor;
      const auto& rhs = state.constituents[b].anchor;
      if (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z) ++duplicates;
    }
  return duplicates;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  ClosedNeutralPairOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-13;
  options.max_iterations = 64;
  GlobalSummary summary;
  const auto rotations = cubic_rotations();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const auto green = make_green_kernel();
  const double beta = normalization.mapped_field_work_coefficient;

  summary.green_residual = green.residual;
  summary.green_pass = normalization.valid && green.valid
      && green.residual <= direct_tolerance;
  Strain zero_strain{};
  const auto identity_state = evaluate_global(
      0.0, identity_matrix(), zero_strain, options, green, beta);
  summary.algebra_residual = body_frame.reconstruction_residual;
  if (identity_state.valid) {
    const auto identity_distances = global_distance_range(
        identity_matrix(), zero_strain);
    for (const auto& rotation : rotations) {
      summary.algebra_residual = std::max(
          summary.algebra_residual, orthogonality_residual(rotation));
      const auto rotated = evaluate_global(
          0.0, rotation, zero_strain, options, green, beta);
      const auto rotated_distances = global_distance_range(
          rotation, zero_strain);
      if (!rotated.valid) {
        summary.algebra_residual = INFINITY;
        break;
      }
      summary.algebra_residual = std::max({summary.algebra_residual,
          std::abs(rotated.binding_energy - identity_state.binding_energy),
          std::abs(rotated_distances.first - identity_distances.first),
          std::abs(rotated_distances.second - identity_distances.second)});
      Vec3 centroid{};
      for (const auto& offset : global_offsets(rotation, zero_strain))
        centroid += offset;
      summary.algebra_residual = std::max(
          summary.algebra_residual, centroid.mag());
    }
  }
  summary.algebra_pass = summary.algebra_residual <= 1e-11;

  double reference_minimum = INFINITY, reference_maximum = -INFINITY;
  double relaxed_minimum = INFINITY, relaxed_maximum = -INFINITY;
  GlobalEvaluation phase_zero_minimum;
  ClosedNeutralTrimerPairStepResult phase_zero_step(L);

  if (summary.algebra_pass && summary.green_pass) {
    for (int j = 0; j < 32; ++j) {
      GlobalPhaseRecord record;
      record.phase_index = j;
      record.phase = static_cast<double>(j) / 32.0;
      const auto reference = evaluate_global(record.phase, identity_matrix(),
          zero_strain, options, green, beta);
      std::vector<GlobalSearchResult> searches;
      searches.reserve(global_starts);
      for (const auto& rotation : rotations) {
        auto search = search_from(
            record.phase, rotation, options, green, beta);
        record.total_evaluations += search.evaluations;
        if (search.terminated && search.minimum.valid)
          ++record.terminated_starts;
        searches.push_back(std::move(search));
      }
      ++summary.phase_arms;
      GlobalEvaluation best;
      for (const auto& search : searches)
        if (search.terminated && search.minimum.valid
            && (!best.valid
                || search.minimum.total_energy < best.total_energy))
          best = search.minimum;
      if (best.valid)
        for (const auto& search : searches)
          if (search.terminated && search.minimum.valid
              && std::abs(search.minimum.total_energy - best.total_energy)
                  <= 1e-10)
            ++record.clustered_starts;
      record.coverage = best.valid && record.terminated_starts >= 18
          && record.clustered_starts >= 3;
      summary.coverage_pass = summary.coverage_pass && record.coverage;
      if (!reference.valid || !best.valid) {
        summary.stationary_core_pass = false;
        summary.field_pass = false;
        summary.common_pass = false;
        summary.inverse_pass = false;
        summary.attraction_robust = false;
        summary.phases.push_back(record);
        continue;
      }

      record.strain = best.strain;
      record.reference_energy = reference.total_energy;
      record.relaxed_energy = best.total_energy;
      record.minimum_distance = best.minimum_distance;
      record.maximum_distance = best.maximum_distance;
      const auto differential = differentiate_global(
          record.phase, best, options, green, beta);
      record.gradient_inf = differential.gradient_inf;
      record.minimum_eigenvalue = differential.minimum_eigenvalue;
      record.positive_modes = differential.positive_modes;
      const bool interior = strain_max_abs(best.strain)
          <= strain_basin - 1e-4;
      const bool distances = best.minimum_distance >= 0.5
          && best.maximum_distance <= 2.0;
      const bool energy = best.total_energy <= reference.total_energy + 1e-12;
      record.stable = differential.valid && interior && distances && energy
          && strain_minimum_eigenvalue(best.strain) >= 0.70
          && record.gradient_inf <= 5e-7
          && record.minimum_eigenvalue >= -5e-6
          && record.positive_modes == 6;
      summary.stationary_core_pass = summary.stationary_core_pass
          && record.stable;
      summary.worst_gradient = std::max(
          summary.worst_gradient, record.gradient_inf);
      summary.minimum_hessian_eigenvalue = std::min(
          summary.minimum_hessian_eigenvalue, record.minimum_eigenvalue);
      summary.minimum_positive_modes = std::min(
          summary.minimum_positive_modes, record.positive_modes);
      summary.maximum_strain = std::max(
          summary.maximum_strain, strain_max_abs(best.strain));
      reference_minimum = std::min(reference_minimum, reference.total_energy);
      reference_maximum = std::max(reference_maximum, reference.total_energy);
      relaxed_minimum = std::min(relaxed_minimum, best.total_energy);
      relaxed_maximum = std::max(relaxed_maximum, best.total_energy);

      auto final_state = best.state;
      record.duplicate_anchor_pairs = duplicate_anchor_pairs(final_state);
      if (record.duplicate_anchor_pairs > 0)
        ++summary.site_projection_collision_phases;
      summary.maximum_duplicate_anchor_pairs = std::max(
          summary.maximum_duplicate_anchor_pairs,
          record.duplicate_anchor_pairs);
      const auto direct = initialize_minimum_energy(density_of(final_state));
      const double direct_energy_residual = direct.valid
          ? std::abs(best.field_energy - beta * direct.raw_energy) : INFINITY;
      record.field_gate = std::max({direct.solver_residual,
          direct.gauss_residual, direct.curl_residual,
          direct_energy_residual});
      summary.worst_field_gate = std::max(
          summary.worst_field_gate, record.field_gate);
      summary.field_pass = summary.field_pass && direct.valid
          && record.field_gate <= 1e-11;
      if (!direct.valid) {
        summary.common_pass = false;
        summary.inverse_pass = false;
        summary.attraction_robust = false;
        summary.phases.push_back(record);
        continue;
      }
      final_state.electric = direct.electric;
      const auto forward = ftd::eft::solve_closed_neutral_pair_forward(
          final_state, options);
      ++summary.forward_arms;
      record.common_gate = maximum_common_gate(forward);
      summary.worst_common_gate = std::max(
          summary.worst_common_gate, record.common_gate);
      summary.common_pass = summary.common_pass
          && forward.common_action_gates_pass && record.common_gate <= gate;
      if (forward.valid) {
        const auto reverse = ftd::eft::solve_closed_neutral_pair_reverse(
            forward.later, options);
        ++summary.reverse_arms;
        record.inverse = reverse.valid
            ? ftd::eft::closed_neutral_pair_state_max_difference(
                final_state, reverse.earlier) : INFINITY;
        record.inward_impulse = forward.inward_impulse;
        record.separation_decrease = forward.center_separation_before
            - forward.center_separation_after;
        record.attractive = record.inward_impulse > 1e-10
            && record.separation_decrease > 0.0;
        if (record.attractive) ++summary.attractive_phases;
        summary.worst_inverse = std::max(summary.worst_inverse, record.inverse);
        summary.inverse_pass = summary.inverse_pass
            && reverse.common_action_gates_pass && record.inverse <= 1e-10;
        summary.minimum_inward_impulse = std::min(
            summary.minimum_inward_impulse, record.inward_impulse);
        summary.minimum_separation_decrease = std::min(
            summary.minimum_separation_decrease,
            record.separation_decrease);
        summary.attraction_robust = summary.attraction_robust
            && record.attractive;
        if (j == 0) {
          phase_zero_minimum = best;
          phase_zero_minimum.state = final_state;
          phase_zero_step = forward;
        }
      } else {
        summary.common_pass = false;
        summary.inverse_pass = false;
        summary.attraction_robust = false;
      }
      summary.phases.push_back(record);
      std::cout << "phase=" << j << "/32 terminated="
                << record.terminated_starts << "/24 cluster="
                << record.clustered_starts << " stable=" << record.stable
                << " duplicate_anchors=" << record.duplicate_anchor_pairs
                << " attractive=" << record.attractive << '\n';
    }
  }

  if (summary.coverage_pass && summary.phase_arms == 32) {
    summary.reference_barrier = reference_maximum - reference_minimum;
    summary.relaxed_barrier = relaxed_maximum - relaxed_minimum;
  }
  if (phase_zero_minimum.valid && phase_zero_step.valid) {
    const auto phase_one = evaluate_global(1.0,
        phase_zero_minimum.orientation, phase_zero_minimum.strain,
        options, green, beta);
    summary.periodicity_energy_residual = phase_one.valid
        ? std::abs(phase_one.total_energy - phase_zero_minimum.total_energy)
        : INFINITY;
    const auto translated = translate_x(phase_zero_minimum.state, 1);
    const auto phase_one_step = ftd::eft::solve_closed_neutral_pair_forward(
        translated, options);
    summary.periodicity_state_residual = phase_one_step.valid
        ? ftd::eft::closed_neutral_pair_state_max_difference(
            translate_x(phase_zero_step.later, 1), phase_one_step.later)
        : INFINITY;
    summary.periodicity_pass = summary.periodicity_energy_residual <= gate
        && summary.periodicity_state_residual <= gate;
  }

  if (!summary.algebra_pass || !summary.green_pass
      || !summary.coverage_pass || !summary.field_pass
      || !summary.common_pass || !summary.inverse_pass
      || !summary.periodicity_pass) {
    summary.verdict = "GLOBAL_ORIENTATION_STRAIN_NUMERICALLY_UNRESOLVED";
  } else if (!summary.stationary_core_pass) {
    summary.verdict = "GLOBAL_ORIENTATION_STRAIN_COMPACT_CORE_CLOSED_NEGATIVE";
  } else if (summary.attraction_robust) {
    summary.verdict = "GLOBAL_ORIENTATION_STRAIN_PHASE_ROBUST_CONSTRUCTIVE";
  } else {
    summary.verdict = "GLOBAL_ORIENTATION_STRAIN_STABLE_FORCE_SIGN_FAILS";
  }

  write_global_record(summary);
  std::cout << "protocol_sha256=" << global_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "algebra_residual=" << summary.algebra_residual << '\n'
            << "coverage_pass=" << summary.coverage_pass << '\n'
            << "stationary_core_pass=" << summary.stationary_core_pass << '\n'
            << "attractive_phases=" << summary.attractive_phases << "/32\n"
            << "site_projection_collision_phases="
            << summary.site_projection_collision_phases << "/32\n"
            << "worst_gradient=" << summary.worst_gradient << '\n'
            << "minimum_hessian_eigenvalue="
            << summary.minimum_hessian_eigenvalue << '\n'
            << "minimum_positive_modes=" << summary.minimum_positive_modes
            << '\n'
            << "maximum_strain=" << summary.maximum_strain << '\n'
            << "worst_common_gate=" << summary.worst_common_gate << '\n'
            << "worst_inverse=" << summary.worst_inverse << '\n';
  return summary.phase_arms == 32 && summary.phases.size() == 32 ? 0 : 1;
}
