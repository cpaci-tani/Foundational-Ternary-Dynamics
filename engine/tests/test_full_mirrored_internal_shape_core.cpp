/** FTD-0605: full mirrored internal-shape matter-core discriminator. */

#include "ftd/eft/closed_neutral_trimer_pair.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr int NPARAM = 6;
constexpr double gate = 1e-12;
constexpr double basin = 0.20;
constexpr double direct_tolerance = 1e-15;
constexpr const char* protocol_sha256 =
    "388926B3947F0C0A378FC3B52BD99E3C94D8F9BBB0A4D325E26CE1252B79C70F";

using ftd::Vec3;
using ftd::eft::ClosedNeutralPairOptions;
using ftd::eft::ClosedNeutralTrimerPairState;
using ftd::eft::ClosedNeutralTrimerPairStepResult;
using Shape = std::array<double, NPARAM>;

int wrap(int value) {
  const int remainder = value % L;
  return remainder < 0 ? remainder + L : remainder;
}

int index(int x, int y, int z) {
  return (wrap(x) * L + wrap(y)) * L + wrap(z);
}

Vec3 effective_position(const ftd::eft::MatchedMatterPoint& point) {
  return {point.anchor.x + point.remainder.x,
          point.anchor.y + point.remainder.y,
          point.anchor.z + point.remainder.z};
}

ftd::eft::MatchedMatterPoint point_at(const Vec3& position) {
  ftd::eft::MatchedMatterPoint point;
  const long long ax = std::llround(position.x);
  const long long ay = std::llround(position.y);
  const long long az = std::llround(position.z);
  point.anchor = {wrap(static_cast<int>(ax)), wrap(static_cast<int>(ay)),
                  wrap(static_cast<int>(az))};
  point.remainder = {position.x - ax, position.y - ay, position.z - az};
  point.momentum = {};
  return point;
}

long double dot(const std::vector<double>& lhs,
                const std::vector<double>& rhs) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result += static_cast<long double>(lhs[i]) * rhs[i];
  return result;
}

void apply_ddt(const std::vector<double>& scalar,
               std::vector<double>& result) {
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int i = index(x, y, z);
        result[static_cast<std::size_t>(i)] =
            6.0 * scalar[static_cast<std::size_t>(i)]
            - scalar[static_cast<std::size_t>(index(x + 1, y, z))]
            - scalar[static_cast<std::size_t>(index(x - 1, y, z))]
            - scalar[static_cast<std::size_t>(index(x, y + 1, z))]
            - scalar[static_cast<std::size_t>(index(x, y - 1, z))]
            - scalar[static_cast<std::size_t>(index(x, y, z + 1))]
            - scalar[static_cast<std::size_t>(index(x, y, z - 1))];
      }
}

struct PotentialSolve {
  bool valid = false;
  double residual = INFINITY;
  std::vector<double> phi{};
};

PotentialSolve solve_potential(const std::vector<double>& source) {
  PotentialSolve result;
  const std::size_t count = static_cast<std::size_t>(L) * L * L;
  if (source.size() != count) return result;
  long double mean = 0.0L;
  for (double value : source) mean += value;
  if (std::abs(static_cast<double>(mean)) > 1e-12) return result;
  result.phi.assign(count, 0.0);
  std::vector<double> residual = source;
  std::vector<double> direction = source;
  std::vector<double> image(count, 0.0);
  long double rr = dot(residual, residual);
  result.residual = 0.0;
  for (double value : residual)
    result.residual = std::max(result.residual, std::abs(value));
  for (int iteration = 1;
       iteration <= 40 * L && result.residual > direct_tolerance;
       ++iteration) {
    apply_ddt(direction, image);
    const long double p_ap = dot(direction, image);
    if (!(p_ap > 0.0L)) break;
    const long double alpha = rr / p_ap;
    for (std::size_t i = 0; i < count; ++i) {
      result.phi[i] += static_cast<double>(alpha * direction[i]);
      residual[i] -= static_cast<double>(alpha * image[i]);
    }
    result.residual = 0.0;
    for (double value : residual)
      result.residual = std::max(result.residual, std::abs(value));
    if (result.residual <= direct_tolerance) break;
    const long double next = dot(residual, residual);
    const long double beta = next / rr;
    for (std::size_t i = 0; i < count; ++i)
      direction[i] = residual[i] + static_cast<double>(beta * direction[i]);
    rr = next;
  }
  result.valid = result.residual <= direct_tolerance;
  return result;
}

struct MinimumField {
  bool valid = false;
  double solver_residual = INFINITY;
  double gauss_residual = INFINITY;
  double curl_residual = INFINITY;
  double raw_energy = INFINITY;
  ftd::eft::MatchedFaceFlux electric{L};
};

MinimumField initialize_minimum_energy(const std::vector<double>& density) {
  MinimumField result;
  const auto solve = solve_potential(density);
  result.solver_residual = solve.residual;
  if (!solve.valid) return result;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int i = index(x, y, z);
        result.electric.x[static_cast<std::size_t>(i)] =
            solve.phi[static_cast<std::size_t>(i)]
            - solve.phi[static_cast<std::size_t>(index(x + 1, y, z))];
        result.electric.y[static_cast<std::size_t>(i)] =
            solve.phi[static_cast<std::size_t>(i)]
            - solve.phi[static_cast<std::size_t>(index(x, y + 1, z))];
        result.electric.z[static_cast<std::size_t>(i)] =
            solve.phi[static_cast<std::size_t>(i)]
            - solve.phi[static_cast<std::size_t>(index(x, y, z + 1))];
      }
  result.gauss_residual = ftd::eft::max_fractional_gauss_residual(
      result.electric, density);
  result.curl_residual = ftd::eft::max_curl_adjoint(result.electric);
  result.raw_energy = ftd::eft::quadratic_energy(result.electric);
  result.valid = result.gauss_residual <= 1e-11
      && result.curl_residual <= 1e-11;
  return result;
}

struct GreenKernel {
  bool valid = false;
  double residual = INFINITY;
  std::vector<double> values{};
};

GreenKernel make_green_kernel() {
  GreenKernel result;
  const std::size_t count = static_cast<std::size_t>(L) * L * L;
  std::vector<double> source(count, -1.0 / static_cast<double>(count));
  source[static_cast<std::size_t>(index(0, 0, 0))] += 1.0;
  const auto solve = solve_potential(source);
  result.valid = solve.valid;
  result.residual = solve.residual;
  result.values = solve.phi;
  return result;
}

const std::array<Vec3, 3> reference_offsets{{
    {-2.0 / 3.0, -1.0 / 3.0, -1.0 / 3.0},
    { 1.0 / 3.0,  2.0 / 3.0, -1.0 / 3.0},
    { 1.0 / 3.0, -1.0 / 3.0,  2.0 / 3.0}}};
const Vec3 center_a{4.839666666666667, 7.114333333333334,
                    7.620333333333333};
const Vec3 center_b{11.196333333333333, 8.857666666666667,
                    8.433666666666667};

std::array<Vec3, 3> offsets_of(const Shape& shape) {
  std::array<Vec3, 3> offsets{};
  offsets[0] = reference_offsets[0]
      + Vec3{shape[0], shape[1], shape[2]};
  offsets[1] = reference_offsets[1]
      + Vec3{shape[3], shape[4], shape[5]};
  offsets[2] = (offsets[0] + offsets[1]) * -1.0;
  return offsets;
}

ClosedNeutralTrimerPairState make_state(double phase, const Shape& shape) {
  ClosedNeutralTrimerPairState state(L);
  const Vec3 shift{phase, 0.0, 0.0};
  const auto offsets = offsets_of(shape);
  for (std::size_t a = 0; a < 3; ++a) {
    state.constituents[a] = point_at(center_a + shift + offsets[a]);
    state.constituents[a + 3] = point_at(center_b + shift - offsets[a]);
  }
  return state;
}

std::vector<double> density_of(const ClosedNeutralTrimerPairState& state) {
  std::vector<double> density(static_cast<std::size_t>(L) * L * L, 0.0);
  for (std::size_t a = 0; a < state.constituents.size(); ++a) {
    const auto coat = ftd::eft::make_quadratic_polarity_coat(
        effective_position(state.constituents[a]), state.charges[a]);
    if (!coat.valid) return {};
    for (std::size_t item = 0; item < coat.weight_count; ++item) {
      const auto& weight = coat.weights[item];
      density[static_cast<std::size_t>(index(
          weight.site.x, weight.site.y, weight.site.z))] += weight.weight;
    }
  }
  return density;
}

struct SparseEntry {
  int x = 0, y = 0, z = 0;
  double weight = 0.0;
};

std::vector<SparseEntry> sparse_density(const std::vector<double>& density) {
  std::vector<SparseEntry> result;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const double value = density[static_cast<std::size_t>(index(x, y, z))];
        if (value != 0.0) result.push_back({x, y, z, value});
      }
  return result;
}

double green_energy(const std::vector<SparseEntry>& density,
                    const GreenKernel& green) {
  long double value = 0.0L;
  for (const auto& a : density)
    for (const auto& b : density)
      value += 0.5L * static_cast<long double>(a.weight) * b.weight
          * green.values[static_cast<std::size_t>(index(
              a.x - b.x, a.y - b.y, a.z - b.z))];
  return static_cast<double>(value);
}

std::pair<double, double> internal_distance_range(const Shape& shape) {
  const auto offsets = offsets_of(shape);
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

double max_abs(const Shape& shape) {
  double result = 0.0;
  for (double value : shape) result = std::max(result, std::abs(value));
  return result;
}

struct StaticEvaluation {
  bool valid = false;
  Shape shape{};
  double binding_energy = INFINITY;
  double field_energy = INFINITY;
  double total_energy = INFINITY;
  double minimum_distance = 0.0;
  double maximum_distance = INFINITY;
  ClosedNeutralTrimerPairState state{L};
};

StaticEvaluation evaluate_fast(double phase, const Shape& shape,
                               const ClosedNeutralPairOptions& options,
                               const GreenKernel& green, double beta) {
  StaticEvaluation result;
  result.shape = shape;
  if (max_abs(shape) > basin) return result;
  const auto distances = internal_distance_range(shape);
  result.minimum_distance = distances.first;
  result.maximum_distance = distances.second;
  if (result.minimum_distance < 0.5 || result.maximum_distance > 2.0)
    return result;
  result.state = make_state(phase, shape);
  const auto dense = density_of(result.state);
  if (dense.empty()) return result;
  result.binding_energy = ftd::eft::closed_neutral_pair_binding_energy(
      result.state, options);
  result.field_energy = beta * green_energy(sparse_density(dense), green);
  result.total_energy = result.binding_energy + result.field_energy;
  result.valid = std::isfinite(result.total_energy);
  return result;
}

struct Vertex {
  Shape point{};
  StaticEvaluation evaluation{};
};

struct RelaxationResult {
  bool valid = false;
  bool terminated = false;
  int evaluations = 0;
  double diameter = INFINITY;
  double energy_spread = INFINITY;
  StaticEvaluation minimum{};
};

Shape affine(const Shape& origin, const Shape& other, double factor) {
  Shape result{};
  for (int d = 0; d < NPARAM; ++d)
    result[d] = origin[d] + factor * (other[d] - origin[d]);
  return result;
}

RelaxationResult relax_shape(double phase,
                             const ClosedNeutralPairOptions& options,
                             const GreenKernel& green, double beta) {
  RelaxationResult result;
  std::array<Vertex, NPARAM + 1> simplex{};
  const auto evaluate = [&](const Shape& point) {
    Vertex vertex;
    vertex.point = point;
    if (result.evaluations >= 900) return vertex;
    vertex.evaluation = evaluate_fast(phase, point, options, green, beta);
    ++result.evaluations;
    return vertex;
  };
  Shape zero{};
  simplex[0] = evaluate(zero);
  for (int d = 0; d < NPARAM; ++d) {
    Shape point{};
    point[d] = 0.01;
    simplex[static_cast<std::size_t>(d + 1)] = evaluate(point);
  }
  const auto score = [](const Vertex& vertex) {
    return vertex.evaluation.valid
        ? vertex.evaluation.total_energy : 1e100;
  };
  while (result.evaluations < 900) {
    std::sort(simplex.begin(), simplex.end(), [&](const Vertex& a,
                                                   const Vertex& b) {
      return score(a) < score(b);
    });
    result.diameter = 0.0;
    for (std::size_t i = 1; i < simplex.size(); ++i)
      for (int d = 0; d < NPARAM; ++d)
        result.diameter = std::max(result.diameter,
            std::abs(simplex[i].point[d] - simplex[0].point[d]));
    result.energy_spread = std::abs(
        score(simplex.back()) - score(simplex.front()));
    if (result.diameter <= 2e-8 && result.energy_spread <= 1e-14) {
      result.terminated = true;
      break;
    }
    Shape centroid{};
    for (int i = 0; i < NPARAM; ++i)
      for (int d = 0; d < NPARAM; ++d)
        centroid[d] += simplex[static_cast<std::size_t>(i)].point[d]
            / static_cast<double>(NPARAM);
    const Vertex reflected = evaluate(affine(centroid,
        simplex.back().point, -1.0));
    if (score(reflected) < score(simplex.front())) {
      const Vertex expanded = evaluate(affine(centroid,
          reflected.point, 2.0));
      simplex.back() = score(expanded) < score(reflected)
          ? expanded : reflected;
    } else if (score(reflected) < score(simplex[NPARAM - 1])) {
      simplex.back() = reflected;
    } else {
      const bool outside = score(reflected) < score(simplex.back());
      const Shape target = outside ? reflected.point : simplex.back().point;
      const Vertex contracted = evaluate(affine(centroid, target, 0.5));
      if (score(contracted) < (outside ? score(reflected)
                                      : score(simplex.back()))) {
        simplex.back() = contracted;
      } else {
        for (std::size_t i = 1;
             i < simplex.size() && result.evaluations < 900; ++i) {
          const Shape shrunk = affine(simplex[0].point,
                                      simplex[i].point, 0.5);
          simplex[i] = evaluate(shrunk);
        }
      }
    }
  }
  std::sort(simplex.begin(), simplex.end(), [&](const Vertex& a,
                                                 const Vertex& b) {
    return score(a) < score(b);
  });
  result.minimum = simplex.front().evaluation;
  result.valid = result.terminated && result.minimum.valid
      && result.evaluations <= 900;
  return result;
}

struct DifferentialResult {
  bool valid = false;
  double gradient_inf = 0.0;
  std::array<double, NPARAM> eigenvalues{};
  double minimum_eigenvalue = -INFINITY;
  int positive_modes = 0;
};

std::array<double, NPARAM> jacobi_eigenvalues(
    std::array<std::array<double, NPARAM>, NPARAM> matrix) {
  for (int sweep = 0; sweep < 200; ++sweep) {
    int p = 0, q = 1;
    double largest = std::abs(matrix[0][1]);
    for (int i = 0; i < NPARAM; ++i)
      for (int j = i + 1; j < NPARAM; ++j)
        if (std::abs(matrix[i][j]) > largest) {
          largest = std::abs(matrix[i][j]);
          p = i;
          q = j;
        }
    if (largest <= 1e-12) break;
    const double angle = 0.5 * std::atan2(
        2.0 * matrix[p][q], matrix[q][q] - matrix[p][p]);
    const double c = std::cos(angle);
    const double s = std::sin(angle);
    const double app = matrix[p][p];
    const double aqq = matrix[q][q];
    const double apq = matrix[p][q];
    for (int k = 0; k < NPARAM; ++k) {
      if (k == p || k == q) continue;
      const double akp = matrix[k][p];
      const double akq = matrix[k][q];
      matrix[k][p] = matrix[p][k] = c * akp - s * akq;
      matrix[k][q] = matrix[q][k] = s * akp + c * akq;
    }
    matrix[p][p] = c * c * app - 2.0 * s * c * apq + s * s * aqq;
    matrix[q][q] = s * s * app + 2.0 * s * c * apq + c * c * aqq;
    matrix[p][q] = matrix[q][p] = 0.0;
  }
  std::array<double, NPARAM> result{};
  for (int i = 0; i < NPARAM; ++i) result[i] = matrix[i][i];
  std::sort(result.begin(), result.end());
  return result;
}

DifferentialResult differentiate(double phase,
                                 const StaticEvaluation& minimum,
                                 const ClosedNeutralPairOptions& options,
                                 const GreenKernel& green, double beta) {
  DifferentialResult result;
  constexpr double hg = 1e-4;
  constexpr double hh = 2e-3;
  std::array<std::array<double, NPARAM>, NPARAM> hessian{};
  for (int i = 0; i < NPARAM; ++i) {
    Shape plus = minimum.shape, minus = minimum.shape;
    plus[i] += hg;
    minus[i] -= hg;
    const auto fp = evaluate_fast(phase, plus, options, green, beta);
    const auto fm = evaluate_fast(phase, minus, options, green, beta);
    if (!fp.valid || !fm.valid) return result;
    result.gradient_inf = std::max(result.gradient_inf,
        std::abs(fp.total_energy - fm.total_energy) / (2.0 * hg));

    plus = minimum.shape;
    minus = minimum.shape;
    plus[i] += hh;
    minus[i] -= hh;
    const auto hp = evaluate_fast(phase, plus, options, green, beta);
    const auto hm = evaluate_fast(phase, minus, options, green, beta);
    if (!hp.valid || !hm.valid) return result;
    hessian[i][i] = (hp.total_energy - 2.0 * minimum.total_energy
        + hm.total_energy) / (hh * hh);
  }
  for (int i = 0; i < NPARAM; ++i)
    for (int j = i + 1; j < NPARAM; ++j) {
      Shape pp = minimum.shape, pm = minimum.shape;
      Shape mp = minimum.shape, mm = minimum.shape;
      pp[i] += hh; pp[j] += hh;
      pm[i] += hh; pm[j] -= hh;
      mp[i] -= hh; mp[j] += hh;
      mm[i] -= hh; mm[j] -= hh;
      const auto fpp = evaluate_fast(phase, pp, options, green, beta);
      const auto fpm = evaluate_fast(phase, pm, options, green, beta);
      const auto fmp = evaluate_fast(phase, mp, options, green, beta);
      const auto fmm = evaluate_fast(phase, mm, options, green, beta);
      if (!fpp.valid || !fpm.valid || !fmp.valid || !fmm.valid) return result;
      hessian[i][j] = hessian[j][i] =
          (fpp.total_energy - fpm.total_energy - fmp.total_energy
           + fmm.total_energy) / (4.0 * hh * hh);
    }
  result.eigenvalues = jacobi_eigenvalues(hessian);
  result.minimum_eigenvalue = result.eigenvalues.front();
  result.positive_modes = static_cast<int>(std::count_if(
      result.eigenvalues.begin(), result.eigenvalues.end(),
      [](double value) { return value > 1e-3; }));
  result.valid = true;
  return result;
}

double maximum_common_gate(const ClosedNeutralTrimerPairStepResult& result) {
  return std::max({result.root_residual, result.continuity_residual,
      result.gauss_before_residual, result.gauss_after_residual,
      result.force_residual, result.kinematic_residual,
      result.kinetic_discrete_gradient_residual,
      result.electric_adjoint_residual, result.magnetic_work_residual,
      result.binding_work_residual, result.binding_impulse_sum_residual,
      result.matter_work_residual, result.field_work_residual,
      result.total_energy_residual, result.causal_speed_excess});
}

ClosedNeutralTrimerPairState translate_x(
    const ClosedNeutralTrimerPairState& source, int amount) {
  ClosedNeutralTrimerPairState target(L);
  target.charges = source.charges;
  for (std::size_t a = 0; a < source.constituents.size(); ++a) {
    target.constituents[a] = source.constituents[a];
    target.constituents[a].anchor.x = wrap(
        target.constituents[a].anchor.x + amount);
  }
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int from = source.electric.index(x, y, z);
        const int to = target.electric.index(x + amount, y, z);
        target.electric.x[to] = source.electric.x[from];
        target.electric.y[to] = source.electric.y[from];
        target.electric.z[to] = source.electric.z[from];
        target.magnetic_half.x[to] = source.magnetic_half.x[from];
        target.magnetic_half.y[to] = source.magnetic_half.y[from];
        target.magnetic_half.z[to] = source.magnetic_half.z[from];
      }
  return target;
}

struct PhaseRecord {
  int phase_index = 0;
  double phase = 0.0;
  int evaluations = 0;
  Shape shape{};
  double reference_energy = INFINITY;
  double relaxed_energy = INFINITY;
  double gradient_inf = INFINITY;
  double minimum_eigenvalue = -INFINITY;
  int positive_modes = 0;
  double minimum_distance = 0.0;
  double maximum_distance = INFINITY;
  double direct_energy_residual = INFINITY;
  double field_gate = INFINITY;
  double common_gate = INFINITY;
  double inverse = INFINITY;
  double inward_impulse = NAN;
  double separation_decrease = NAN;
  double pseudomomentum_defect = INFINITY;
  bool attractive = false;
};

struct Summary {
  int phase_arms = 0;
  int forward_arms = 0;
  int reverse_arms = 0;
  int attractive_phases = 0;
  bool green_pass = false;
  bool optimizer_pass = true;
  bool interior_pass = true;
  bool distance_pass = true;
  bool stability_pass = true;
  bool energy_pass = true;
  bool field_pass = true;
  bool common_pass = true;
  bool inverse_pass = true;
  bool periodicity_pass = false;
  bool attraction_robust = true;
  double green_residual = INFINITY;
  double maximum_shape_displacement = 0.0;
  double worst_gradient = 0.0;
  double minimum_hessian_eigenvalue = INFINITY;
  int minimum_positive_modes = NPARAM;
  double minimum_internal_distance = INFINITY;
  double maximum_internal_distance = 0.0;
  double worst_direct_energy_residual = 0.0;
  double worst_field_gate = 0.0;
  double worst_common_gate = 0.0;
  double worst_inverse = 0.0;
  double minimum_inward_impulse = INFINITY;
  double minimum_separation_decrease = INFINITY;
  double maximum_pseudomomentum_defect = 0.0;
  double reference_barrier = NAN;
  double relaxed_barrier = NAN;
  double barrier_ratio = NAN;
  double periodicity_energy_residual = INFINITY;
  double periodicity_state_residual = INFINITY;
  std::vector<PhaseRecord> phases{};
  std::string verdict;
};

std::string json_number(double value) {
  if (!std::isfinite(value)) return "null";
  std::ostringstream stream;
  stream << std::setprecision(17) << value;
  return stream.str();
}

void write_record(const Summary& s) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0605";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0605_full_mirrored_shape_core_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0605\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << s.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"phase_arms\": " << s.phase_arms << ",\n"
       << "  \"forward_arms\": " << s.forward_arms << ",\n"
       << "  \"reverse_arms\": " << s.reverse_arms << ",\n"
       << "  \"attractive_phases\": " << s.attractive_phases << ",\n"
       << "  \"green_pass\": " << (s.green_pass ? "true" : "false") << ",\n"
       << "  \"optimizer_pass\": " << (s.optimizer_pass ? "true" : "false") << ",\n"
       << "  \"interior_pass\": " << (s.interior_pass ? "true" : "false") << ",\n"
       << "  \"distance_pass\": " << (s.distance_pass ? "true" : "false") << ",\n"
       << "  \"stability_pass\": " << (s.stability_pass ? "true" : "false") << ",\n"
       << "  \"energy_pass\": " << (s.energy_pass ? "true" : "false") << ",\n"
       << "  \"field_pass\": " << (s.field_pass ? "true" : "false") << ",\n"
       << "  \"common_pass\": " << (s.common_pass ? "true" : "false") << ",\n"
       << "  \"inverse_pass\": " << (s.inverse_pass ? "true" : "false") << ",\n"
       << "  \"periodicity_pass\": " << (s.periodicity_pass ? "true" : "false") << ",\n"
       << "  \"attraction_robust\": " << (s.attraction_robust ? "true" : "false") << ",\n"
       << "  \"green_residual\": " << s.green_residual << ",\n"
       << "  \"maximum_shape_displacement\": " << s.maximum_shape_displacement << ",\n"
       << "  \"worst_gradient\": " << s.worst_gradient << ",\n"
       << "  \"minimum_hessian_eigenvalue\": "
       << json_number(s.minimum_hessian_eigenvalue) << ",\n"
       << "  \"minimum_positive_modes\": " << s.minimum_positive_modes << ",\n"
       << "  \"minimum_internal_distance\": " << s.minimum_internal_distance << ",\n"
       << "  \"maximum_internal_distance\": " << s.maximum_internal_distance << ",\n"
       << "  \"worst_direct_energy_residual\": " << s.worst_direct_energy_residual << ",\n"
       << "  \"worst_field_gate\": " << s.worst_field_gate << ",\n"
       << "  \"worst_common_gate\": " << s.worst_common_gate << ",\n"
       << "  \"worst_inverse\": " << s.worst_inverse << ",\n"
       << "  \"minimum_inward_impulse\": " << s.minimum_inward_impulse << ",\n"
       << "  \"minimum_separation_decrease\": " << s.minimum_separation_decrease << ",\n"
       << "  \"maximum_pseudomomentum_defect\": "
       << s.maximum_pseudomomentum_defect << ",\n"
       << "  \"reference_barrier\": "
       << json_number(s.reference_barrier) << ",\n"
       << "  \"relaxed_barrier\": "
       << json_number(s.relaxed_barrier) << ",\n"
       << "  \"barrier_ratio\": " << json_number(s.barrier_ratio) << ",\n"
       << "  \"periodicity_energy_residual\": "
       << json_number(s.periodicity_energy_residual) << ",\n"
       << "  \"periodicity_state_residual\": "
       << json_number(s.periodicity_state_residual) << "\n}\n";
  std::ofstream csv(dir / "ftd_0605_full_mirrored_shape_core_samples_v1.csv");
  csv << "ftd_id,phase_index,phase,evaluations,u0,u1,u2,u3,u4,u5,"
         "reference_energy,relaxed_energy,gradient_inf,minimum_eigenvalue,"
         "positive_modes,minimum_distance,maximum_distance,"
         "direct_energy_residual,field_gate,common_gate,inverse,"
         "inward_impulse,separation_decrease,pseudomomentum_defect,attractive\n";
  for (const auto& p : s.phases) {
    csv << std::setprecision(17) << "FTD-0605," << p.phase_index << ','
        << p.phase << ',' << p.evaluations;
    for (double value : p.shape) csv << ',' << value;
    csv << ',' << p.reference_energy << ',' << p.relaxed_energy << ','
        << p.gradient_inf << ',' << p.minimum_eigenvalue << ','
        << p.positive_modes << ',' << p.minimum_distance << ','
        << p.maximum_distance << ',' << p.direct_energy_residual << ','
        << p.field_gate << ',' << p.common_gate << ',' << p.inverse << ','
        << p.inward_impulse << ',' << p.separation_decrease << ','
        << p.pseudomomentum_defect << ',' << p.attractive << '\n';
  }
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  ClosedNeutralPairOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-13;
  options.max_iterations = 64;
  Summary summary;
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const auto green = make_green_kernel();
  summary.green_residual = green.residual;
  summary.green_pass = normalization.valid && green.valid
      && green.residual <= direct_tolerance;
  if (!summary.green_pass) {
    summary.verdict = "FULL_MIRRORED_SHAPE_STATIC_BRANCH_CLOSED_NEGATIVE";
    write_record(summary);
    return 1;
  }
  const double beta = normalization.mapped_field_work_coefficient;
  Shape zero{};
  double reference_minimum = INFINITY, reference_maximum = -INFINITY;
  double relaxed_minimum = INFINITY, relaxed_maximum = -INFINITY;
  StaticEvaluation phase_zero_minimum;
  ClosedNeutralTrimerPairStepResult phase_zero_step(L);

  for (int j = 0; j < 32; ++j) {
    PhaseRecord record;
    record.phase_index = j;
    record.phase = static_cast<double>(j) / 32.0;
    const auto reference = evaluate_fast(
        record.phase, zero, options, green, beta);
    const auto relaxed = relax_shape(
        record.phase, options, green, beta);
    ++summary.phase_arms;
    record.evaluations = relaxed.evaluations;
    summary.optimizer_pass = summary.optimizer_pass
        && reference.valid && relaxed.valid;
    if (!reference.valid || !relaxed.valid) {
      summary.field_pass = false;
      summary.common_pass = false;
      summary.inverse_pass = false;
      summary.attraction_robust = false;
      summary.phases.push_back(record);
      continue;
    }
    record.shape = relaxed.minimum.shape;
    record.reference_energy = reference.total_energy;
    record.relaxed_energy = relaxed.minimum.total_energy;
    record.minimum_distance = relaxed.minimum.minimum_distance;
    record.maximum_distance = relaxed.minimum.maximum_distance;
    const auto differential = differentiate(record.phase, relaxed.minimum,
        options, green, beta);
    record.gradient_inf = differential.gradient_inf;
    record.minimum_eigenvalue = differential.minimum_eigenvalue;
    record.positive_modes = differential.positive_modes;
    summary.stability_pass = summary.stability_pass && differential.valid
        && record.gradient_inf <= 5e-7
        && record.minimum_eigenvalue >= -5e-6
        && record.positive_modes >= 3;
    summary.worst_gradient = std::max(
        summary.worst_gradient, record.gradient_inf);
    summary.minimum_hessian_eigenvalue = std::min(
        summary.minimum_hessian_eigenvalue, record.minimum_eigenvalue);
    summary.minimum_positive_modes = std::min(
        summary.minimum_positive_modes, record.positive_modes);
    summary.maximum_shape_displacement = std::max(
        summary.maximum_shape_displacement, max_abs(record.shape));
    summary.interior_pass = summary.interior_pass
        && max_abs(record.shape) <= basin - 1e-4;
    summary.distance_pass = summary.distance_pass
        && record.minimum_distance >= 0.5
        && record.maximum_distance <= 2.0;
    summary.minimum_internal_distance = std::min(
        summary.minimum_internal_distance, record.minimum_distance);
    summary.maximum_internal_distance = std::max(
        summary.maximum_internal_distance, record.maximum_distance);
    summary.energy_pass = summary.energy_pass
        && record.relaxed_energy <= record.reference_energy + 1e-12;
    reference_minimum = std::min(reference_minimum, record.reference_energy);
    reference_maximum = std::max(reference_maximum, record.reference_energy);
    relaxed_minimum = std::min(relaxed_minimum, record.relaxed_energy);
    relaxed_maximum = std::max(relaxed_maximum, record.relaxed_energy);

    auto final_state = relaxed.minimum.state;
    const auto dense = density_of(final_state);
    const auto direct = initialize_minimum_energy(dense);
    record.direct_energy_residual = direct.valid
        ? std::abs(relaxed.minimum.field_energy - beta * direct.raw_energy)
        : INFINITY;
    record.field_gate = std::max({direct.solver_residual,
        direct.gauss_residual, direct.curl_residual,
        record.direct_energy_residual});
    summary.worst_direct_energy_residual = std::max(
        summary.worst_direct_energy_residual, record.direct_energy_residual);
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
      record.pseudomomentum_defect = forward.pseudomomentum_defect_norm;
      record.attractive = record.inward_impulse > 1e-10
          && record.separation_decrease > 0.0;
      if (record.attractive) ++summary.attractive_phases;
      summary.worst_inverse = std::max(summary.worst_inverse, record.inverse);
      summary.inverse_pass = summary.inverse_pass
          && reverse.common_action_gates_pass && record.inverse <= 1e-10;
      summary.minimum_inward_impulse = std::min(
          summary.minimum_inward_impulse, record.inward_impulse);
      summary.minimum_separation_decrease = std::min(
          summary.minimum_separation_decrease, record.separation_decrease);
      summary.maximum_pseudomomentum_defect = std::max(
          summary.maximum_pseudomomentum_defect,
          record.pseudomomentum_defect);
      summary.attraction_robust = summary.attraction_robust
          && record.attractive;
      if (j == 0) {
        phase_zero_minimum = relaxed.minimum;
        phase_zero_minimum.state = final_state;
        phase_zero_step = forward;
      }
    } else {
      summary.common_pass = false;
      summary.inverse_pass = false;
      summary.attraction_robust = false;
    }
    summary.phases.push_back(record);
  }

  if (summary.optimizer_pass) {
    summary.reference_barrier = reference_maximum - reference_minimum;
    summary.relaxed_barrier = relaxed_maximum - relaxed_minimum;
    summary.barrier_ratio = summary.reference_barrier > 0.0
        ? summary.relaxed_barrier / summary.reference_barrier : NAN;
  }
  if (phase_zero_minimum.valid && phase_zero_step.valid) {
    const auto phase_one = evaluate_fast(
        1.0, phase_zero_minimum.shape, options, green, beta);
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

  const bool static_gates = summary.green_pass && summary.optimizer_pass
      && summary.interior_pass && summary.distance_pass
      && summary.stability_pass && summary.energy_pass && summary.field_pass;
  if (!static_gates || !summary.common_pass || !summary.inverse_pass
      || !summary.periodicity_pass) {
    summary.verdict = "FULL_MIRRORED_SHAPE_STATIC_BRANCH_CLOSED_NEGATIVE";
  } else if (summary.attraction_robust) {
    summary.verdict = "FULL_MIRRORED_SHAPE_PHASE_ROBUST_CONSTRUCTIVE";
  } else if (!summary.attraction_robust) {
    summary.verdict = "FULL_MIRRORED_SHAPE_RELAXES_BUT_FORCE_SIGN_FAILS";
  } else {
    summary.verdict = "FULL_MIRRORED_SHAPE_UNRESOLVED";
  }

  int failures = 0;
  const auto check = [&](bool condition, const std::string& label) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << label << '\n';
    if (!condition) ++failures;
  };
  check(summary.phase_arms == 32, "all 32 locked phase arms attempted");
  check(summary.phases.size() == 32, "all 32 locked phase arms recorded");
  check(!summary.verdict.empty(), "campaign produced a locked verdict");
  write_record(summary);
  std::cout << "protocol_sha256=" << protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "attractive_phases=" << summary.attractive_phases << "/32\n"
            << "optimizer_pass=" << summary.optimizer_pass << '\n'
            << "maximum_shape_displacement="
            << summary.maximum_shape_displacement << '\n'
            << "worst_gradient=" << summary.worst_gradient << '\n'
            << "minimum_hessian_eigenvalue="
            << summary.minimum_hessian_eigenvalue << '\n'
            << "minimum_positive_modes=" << summary.minimum_positive_modes
            << '\n'
            << "reference_barrier=" << summary.reference_barrier << '\n'
            << "relaxed_barrier=" << summary.relaxed_barrier << '\n'
            << "barrier_ratio=" << summary.barrier_ratio << '\n'
            << "minimum_inward_impulse=" << summary.minimum_inward_impulse
            << '\n'
            << "minimum_separation_decrease="
            << summary.minimum_separation_decrease << '\n'
            << "periodicity_state_residual="
            << summary.periodicity_state_residual << '\n'
            << "failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}
