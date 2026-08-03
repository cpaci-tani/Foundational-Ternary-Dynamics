#pragma once
/**
 * @file connected_moore_tangent_codec.h
 * @brief Test-only constrained tangent chart used by the locked FTD-0774 run.
 *
 * This header deliberately contains no production engine hook.  It is included
 * after the FTD-0638/0639 test helpers by test_l17_complete_tangent_candidate.
 */

#include "ftd/eft/connected_moore_block_action.h"
#include "ftd/eft/matched_face_energy_transaction.h"
#include "ftd/eft/production_hop_kinematics.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <limits>
#include <numeric>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace ftd0774 {

constexpr int kL = 17;
constexpr int kMatterCount = 16;
constexpr int kMatterDimension = 3 * kMatterCount;
constexpr std::size_t kVolume = static_cast<std::size_t>(kL) * kL * kL;
constexpr std::size_t kFieldDimension = 3 * kVolume;
constexpr std::size_t kRawChartDimension =
    2 * kMatterDimension + 2 * kFieldDimension;
constexpr std::size_t kIndependentChartDimension =
    5 * kVolume + 6 * kMatterCount + 1;

using ftd::Vec3;
using ftd::eft::ConnectedMooreBlockState;
using ftd::eft::MatchedEdgeField;
using ftd::eft::MatchedFaceFlux;
using SmallMatrix = std::vector<std::vector<double>>;

inline double finite_or(double value, double fallback = INFINITY) {
  return std::isfinite(value) ? value : fallback;
}

inline double component(const Vec3& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
}

inline void set_component(Vec3& value, int axis, double entry) {
  if (axis == 0) value.x = entry;
  else if (axis == 1) value.y = entry;
  else value.z = entry;
}

inline bool finite_face(const MatchedFaceFlux& field) {
  if (field.L != kL || field.x.size() != kVolume
      || field.y.size() != kVolume || field.z.size() != kVolume) return false;
  const auto finite = [](double x) { return std::isfinite(x); };
  return std::all_of(field.x.begin(), field.x.end(), finite)
      && std::all_of(field.y.begin(), field.y.end(), finite)
      && std::all_of(field.z.begin(), field.z.end(), finite);
}

inline bool finite_edge(const MatchedEdgeField& field) {
  if (field.L != kL || field.x.size() != kVolume
      || field.y.size() != kVolume || field.z.size() != kVolume) return false;
  const auto finite = [](double x) { return std::isfinite(x); };
  return std::all_of(field.x.begin(), field.x.end(), finite)
      && std::all_of(field.y.begin(), field.y.end(), finite)
      && std::all_of(field.z.begin(), field.z.end(), finite);
}

inline double face_dot(const MatchedFaceFlux& left,
                       const MatchedFaceFlux& right) {
  return static_cast<double>(ftd::eft::matched_face_dot(left, right));
}

inline double edge_dot(const MatchedEdgeField& left,
                       const MatchedEdgeField& right) {
  return static_cast<double>(ftd::eft::matched_edge_dot(left, right));
}

inline double face_l2(const MatchedFaceFlux& field) {
  return std::sqrt(std::max(0.0, face_dot(field, field)));
}

inline double edge_l2(const MatchedEdgeField& field) {
  return std::sqrt(std::max(0.0, edge_dot(field, field)));
}

inline double face_max_abs(const MatchedFaceFlux& field) {
  double result = 0.0;
  for (std::size_t i = 0; i < field.x.size(); ++i)
    result = std::max({result,std::abs(field.x[i]),
                       std::abs(field.y[i]),std::abs(field.z[i])});
  return result;
}

inline double edge_max_abs(const MatchedEdgeField& field) {
  double result = 0.0;
  for (std::size_t i = 0; i < field.x.size(); ++i)
    result = std::max({result,std::abs(field.x[i]),
                       std::abs(field.y[i]),std::abs(field.z[i])});
  return result;
}

inline void add_scaled(MatchedFaceFlux& target,
                       const MatchedFaceFlux& source, double scale) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale * source.x[i];
    target.y[i] += scale * source.y[i];
    target.z[i] += scale * source.z[i];
  }
}

inline void add_scaled(MatchedEdgeField& target,
                       const MatchedEdgeField& source, double scale) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale * source.x[i];
    target.y[i] += scale * source.y[i];
    target.z[i] += scale * source.z[i];
  }
}

inline MatchedFaceFlux face_difference(const MatchedFaceFlux& left,
                                       const MatchedFaceFlux& right,
                                       double scale = 1.0) {
  MatchedFaceFlux result(left.L);
  for (std::size_t i = 0; i < result.x.size(); ++i) {
    result.x[i] = scale * (left.x[i] - right.x[i]);
    result.y[i] = scale * (left.y[i] - right.y[i]);
    result.z[i] = scale * (left.z[i] - right.z[i]);
  }
  return result;
}

inline MatchedEdgeField edge_difference(const MatchedEdgeField& left,
                                        const MatchedEdgeField& right,
                                        double scale = 1.0) {
  MatchedEdgeField result(left.L);
  for (std::size_t i = 0; i < result.x.size(); ++i) {
    result.x[i] = scale * (left.x[i] - right.x[i]);
    result.y[i] = scale * (left.y[i] - right.y[i]);
    result.z[i] = scale * (left.z[i] - right.z[i]);
  }
  return result;
}

inline std::array<double, 3> face_means(const MatchedFaceFlux& field) {
  std::array<long double, 3> sums{};
  for (std::size_t i = 0; i < kVolume; ++i) {
    sums[0] += field.x[i];
    sums[1] += field.y[i];
    sums[2] += field.z[i];
  }
  return {{static_cast<double>(sums[0] / kVolume),
           static_cast<double>(sums[1] / kVolume),
           static_cast<double>(sums[2] / kVolume)}};
}

inline std::array<double, 3> edge_means(const MatchedEdgeField& field) {
  std::array<long double, 3> sums{};
  for (std::size_t i = 0; i < kVolume; ++i) {
    sums[0] += field.x[i];
    sums[1] += field.y[i];
    sums[2] += field.z[i];
  }
  return {{static_cast<double>(sums[0] / kVolume),
           static_cast<double>(sums[1] / kVolume),
           static_cast<double>(sums[2] / kVolume)}};
}

inline std::vector<double> divergence(const MatchedFaceFlux& field) {
  std::vector<double> result(kVolume, 0.0);
  for (int x = 0; x < kL; ++x)
    for (int y = 0; y < kL; ++y)
      for (int z = 0; z < kL; ++z) {
        const auto i = static_cast<std::size_t>(field.index(x, y, z));
        result[i] = ftd::eft::divergence_at(field, x, y, z);
      }
  return result;
}

inline void enforce_zero_face_means(MatchedFaceFlux& field) {
  // Every periodic discrete gradient has exactly zero uniform coefficient.
  // Repeatedly remove only the floating-point summation residue so the Hodge
  // correction cannot consume a retained harmonic coordinate.
  for (int pass = 0; pass < 4; ++pass) {
    const auto means = face_means(field);
    for (std::size_t i = 0; i < kVolume; ++i) {
      field.x[i] -= means[0];
      field.y[i] -= means[1];
      field.z[i] -= means[2];
    }
  }
}

inline double max_abs(const std::vector<double>& values) {
  double result = 0.0;
  for (double value : values) result = std::max(result, std::abs(value));
  return result;
}

inline long double vector_dot(const std::vector<double>& left,
                              const std::vector<double>& right) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < left.size(); ++i)
    result += static_cast<long double>(left[i]) * right[i];
  return result;
}

inline int wrap(int value) {
  const int result = value % kL;
  return result < 0 ? result + kL : result;
}

inline std::size_t site_index(int x, int y, int z) {
  return static_cast<std::size_t>((wrap(x) * kL + wrap(y)) * kL + wrap(z));
}

inline void negative_laplacian(const std::vector<double>& input,
                               std::vector<double>& output) {
  for (int x = 0; x < kL; ++x)
    for (int y = 0; y < kL; ++y)
      for (int z = 0; z < kL; ++z) {
        const auto i = site_index(x, y, z);
        output[i] = 6.0 * input[i]
            - input[site_index(x + 1, y, z)]
            - input[site_index(x - 1, y, z)]
            - input[site_index(x, y + 1, z)]
            - input[site_index(x, y - 1, z)]
            - input[site_index(x, y, z + 1)]
            - input[site_index(x, y, z - 1)];
      }
}

struct LongitudinalSolve {
  bool valid = false;
  int iterations = 0;
  double source_mean = INFINITY;
  double source_total = INFINITY;
  double source_scale = INFINITY;
  double compatibility_absolute = INFINITY;
  double compatibility_relative = INFINITY;
  double relative_residual = INFINITY;
  double absolute_residual = INFINITY;
  std::vector<double> potential;
  MatchedFaceFlux field{kL};
};

inline LongitudinalSolve solve_longitudinal(
    const std::vector<double>& source, double tolerance = 1e-13,
    int max_iterations = 4096) {
  LongitudinalSolve result;
  if (source.size() != kVolume || !(tolerance > 0.0)
      || max_iterations <= 0) return result;
  std::vector<double> rhs = source;
  const long double total = std::accumulate(rhs.begin(), rhs.end(), 0.0L);
  const long double mean = total / static_cast<long double>(kVolume);
  result.source_total = static_cast<double>(total);
  result.source_mean = static_cast<double>(mean);
  result.source_scale = max_abs(rhs);
  result.compatibility_absolute = std::abs(result.source_mean);
  result.compatibility_relative = result.source_scale > 0.0
      ? result.compatibility_absolute / result.source_scale
      : (result.compatibility_absolute == 0.0 ? 0.0 : INFINITY);
  if (!std::isfinite(result.compatibility_relative)
      || result.compatibility_relative > tolerance) return result;
  for (double& value : rhs) value -= result.source_mean;
  const double scale = max_abs(rhs);
  result.potential.assign(kVolume, 0.0);
  if (scale == 0.0) {
    result.valid = true;
    result.relative_residual = 0.0;
    result.absolute_residual = 0.0;
    return result;
  }
  for (double& value : rhs) value /= scale;
  std::vector<double> residual = rhs;
  std::vector<double> direction = rhs;
  std::vector<double> image(kVolume, 0.0);
  long double rr = vector_dot(residual, residual);
  for (int iteration = 1; iteration <= max_iterations; ++iteration) {
    const double recursive = max_abs(residual);
    if (recursive <= tolerance) {
      result.iterations = iteration - 1;
      break;
    }
    negative_laplacian(direction, image);
    const long double denominator = vector_dot(direction, image);
    if (!(denominator > 0.0L)) break;
    const long double alpha = rr / denominator;
    for (std::size_t i = 0; i < kVolume; ++i) {
      result.potential[i] += static_cast<double>(alpha * direction[i]);
      residual[i] -= static_cast<double>(alpha * image[i]);
    }
    const long double next = vector_dot(residual, residual);
    if (!(next >= 0.0L)) break;
    const long double ratio = rr > 0.0L ? next / rr : 0.0L;
    for (std::size_t i = 0; i < kVolume; ++i)
      direction[i] = residual[i] + static_cast<double>(ratio * direction[i]);
    rr = next;
    result.iterations = iteration;
  }
  std::vector<double> check(kVolume, 0.0);
  negative_laplacian(result.potential, check);
  result.relative_residual = 0.0;
  for (std::size_t i = 0; i < kVolume; ++i)
    result.relative_residual = std::max(
        result.relative_residual, std::abs(check[i] - rhs[i]));
  for (double& value : result.potential) value *= scale;
  result.absolute_residual = result.relative_residual * scale;
  for (int x = 0; x < kL; ++x)
    for (int y = 0; y < kL; ++y)
      for (int z = 0; z < kL; ++z) {
        const auto i = site_index(x, y, z);
        result.field.x[i] = result.potential[i]
            - result.potential[site_index(x + 1, y, z)];
        result.field.y[i] = result.potential[i]
            - result.potential[site_index(x, y + 1, z)];
        result.field.z[i] = result.potential[i]
            - result.potential[site_index(x, y, z + 1)];
      }
  result.valid = result.relative_residual <= tolerance
      && finite_face(result.field);
  return result;
}

struct ChartVector {
  std::vector<double> dx;
  std::vector<double> dp;
  MatchedFaceFlux e;
  MatchedEdgeField b;

  ChartVector()
      : dx(kMatterDimension, 0.0), dp(kMatterDimension, 0.0),
        e(kL), b(kL) {}
};

inline bool finite_chart(const ChartVector& value) {
  const auto finite = [](double x) { return std::isfinite(x); };
  return value.dx.size() == kMatterDimension
      && value.dp.size() == kMatterDimension
      && std::all_of(value.dx.begin(), value.dx.end(), finite)
      && std::all_of(value.dp.begin(), value.dp.end(), finite)
      && finite_face(value.e) && finite_edge(value.b);
}

inline ChartVector scaled(const ChartVector& value, double scale) {
  ChartVector result;
  for (int i = 0; i < kMatterDimension; ++i) {
    result.dx[i] = scale * value.dx[i];
    result.dp[i] = scale * value.dp[i];
  }
  for (std::size_t i = 0; i < kVolume; ++i) {
    result.e.x[i] = scale * value.e.x[i];
    result.e.y[i] = scale * value.e.y[i];
    result.e.z[i] = scale * value.e.z[i];
    result.b.x[i] = scale * value.b.x[i];
    result.b.y[i] = scale * value.b.y[i];
    result.b.z[i] = scale * value.b.z[i];
  }
  return result;
}

inline void axpy(ChartVector& target, const ChartVector& source, double alpha) {
  for (int i = 0; i < kMatterDimension; ++i) {
    target.dx[i] += alpha * source.dx[i];
    target.dp[i] += alpha * source.dp[i];
  }
  add_scaled(target.e, source.e, alpha);
  add_scaled(target.b, source.b, alpha);
}

inline ChartVector difference(const ChartVector& left,
                              const ChartVector& right) {
  ChartVector result = left;
  axpy(result, right, -1.0);
  return result;
}

inline std::vector<double> flatten(const ChartVector& value) {
  std::vector<double> result;
  result.reserve(kRawChartDimension);
  result.insert(result.end(), value.dx.begin(), value.dx.end());
  result.insert(result.end(), value.dp.begin(), value.dp.end());
  result.insert(result.end(), value.e.x.begin(), value.e.x.end());
  result.insert(result.end(), value.e.y.begin(), value.e.y.end());
  result.insert(result.end(), value.e.z.begin(), value.e.z.end());
  result.insert(result.end(), value.b.x.begin(), value.b.x.end());
  result.insert(result.end(), value.b.y.begin(), value.b.y.end());
  result.insert(result.end(), value.b.z.begin(), value.b.z.end());
  return result;
}

struct TangentMetric {
  SmallMatrix hessian;
  double beta = NAN;
  double lambda = NAN;
};

inline double inner(const TangentMetric& metric, const ChartVector& left,
                    const ChartVector& right) {
  long double result = 0.0L;
  for (int i = 0; i < kMatterDimension; ++i)
    result += static_cast<long double>(left.dp[i]) * right.dp[i]
        / ftd::M_INERTIAL;
  for (int i = 0; i < kMatterDimension; ++i)
    for (int j = 0; j < kMatterDimension; ++j)
      result += static_cast<long double>(left.dx[i])
          * metric.hessian[i][j] * right.dx[j];
  const MatchedEdgeField curl_left = ftd::eft::matched_curl_adjoint(left.e);
  const MatchedEdgeField curl_right = ftd::eft::matched_curl_adjoint(right.e);
  const long double field = ftd::eft::matched_face_dot(left.e, right.e)
      + ftd::eft::matched_edge_dot(left.b, right.b)
      - 0.5L * metric.lambda
          * (ftd::eft::matched_edge_dot(left.b, curl_right)
             + ftd::eft::matched_edge_dot(right.b, curl_left));
  result += metric.beta * field;
  return static_cast<double>(result);
}

inline double norm(const TangentMetric& metric, const ChartVector& value) {
  const double square = inner(metric, value, value);
  if (!std::isfinite(square) || square < 0.0) return NAN;
  return std::sqrt(square);
}

inline bool normalize(const TangentMetric& metric, ChartVector& value,
                      double minimum = 0.0) {
  const double value_norm = norm(metric, value);
  if (!(value_norm > minimum) || !std::isfinite(value_norm)) return false;
  value = scaled(value, 1.0 / value_norm);
  return true;
}

inline bool k_mgs_two_pass(const TangentMetric& metric, ChartVector& value,
                           const std::vector<ChartVector>& basis,
                           double minimum = 1e-12) {
  for (int pass = 0; pass < 2; ++pass)
    for (const auto& column : basis)
      axpy(value, column, -inner(metric, column, value));
  return normalize(metric, value, minimum);
}

template <class DepositType>
inline LongitudinalSolve longitudinal_from_dx(
    const DepositType& density_jet, const std::vector<double>& dx) {
  if (!density_jet.valid
      || !std::isfinite(density_jet.derivative_charge_residual)
      || density_jet.derivative_charge_residual > 1e-12)
    return LongitudinalSolve{};
  std::vector<double> source(kVolume, 0.0);
  for (int coordinate = 0; coordinate < kMatterDimension; ++coordinate)
    for (std::size_t site = 0; site < kVolume; ++site)
      source[site] += dx[coordinate] * density_jet.first[coordinate][site];
  return solve_longitudinal(source, 1e-13, 4096);
}

inline bool same_edges(const ConnectedMooreBlockState& left,
                       const ConnectedMooreBlockState& right) {
  if (left.edges.size() != right.edges.size()) return false;
  for (std::size_t i = 0; i < left.edges.size(); ++i) {
    const auto& a = left.edges[i];
    const auto& b = right.edges[i];
    if (a.first != b.first || a.second != b.second
        || a.reference_delta.x != b.reference_delta.x
        || a.reference_delta.y != b.reference_delta.y
        || a.reference_delta.z != b.reference_delta.z
        || a.rest_length_squared != b.rest_length_squared) return false;
  }
  return true;
}

inline bool same_metadata(const ConnectedMooreBlockState& left,
                          const ConnectedMooreBlockState& right) {
  if (left.width != right.width
      || left.orientation_axis != right.orientation_axis
      || left.charges != right.charges || !same_edges(left, right)
      || left.constituents.size() != right.constituents.size()) return false;
  for (std::size_t i = 0; i < left.constituents.size(); ++i) {
    const auto& a = left.constituents[i].anchor;
    const auto& b = right.constituents[i].anchor;
    if (a.x != b.x || a.y != b.y || a.z != b.z) return false;
  }
  return true;
}

template <class DepositFunction, class PositionFunction,
          class PointFunction, class SectorFunction>
struct TangentChart {
  using DepositResult = std::invoke_result_t<
      DepositFunction, const ConnectedMooreBlockState&>;
  const ConnectedMooreBlockState& reference;
  DepositFunction deposit_fn;
  PositionFunction position_fn;
  PointFunction point_fn;
  SectorFunction sector_fn;
  DepositResult density_jet;

  TangentChart(const ConnectedMooreBlockState& state, DepositFunction deposit,
               PositionFunction position, PointFunction point,
               SectorFunction sector)
      : reference(state), deposit_fn(deposit), position_fn(position),
        point_fn(point), sector_fn(sector), density_jet(deposit_fn(reference)) {}
};

struct RetractionResult {
  bool valid = false;
  bool metadata = false;
  bool sector = false;
  bool finite = false;
  int poisson_iterations = 0;
  double poisson_absolute_residual = INFINITY;
  double poisson_relative_residual = INFINITY;
  double gauss_residual = INFINITY;
  ConnectedMooreBlockState longitudinal{kL};
  ConnectedMooreBlockState state{kL};
};

template <class Chart>
inline RetractionResult retract(const Chart& chart, const ChartVector& tangent,
                                double amount) {
  RetractionResult result;
  if (!finite_chart(tangent) || !std::isfinite(amount)) return result;
  auto geometry = chart.reference;
  for (int particle = 0; particle < kMatterCount; ++particle) {
    Vec3 x = chart.position_fn(chart.reference.constituents[particle]);
    for (int axis = 0; axis < 3; ++axis)
      set_component(x, axis, component(x, axis)
          + amount * tangent.dx[3 * particle + axis]);
    geometry.constituents[particle] = chart.point_fn(x);
  }
  const auto dressed = ftd::eft::redress_connected_moore_block_with_fibre_limit(
      geometry, 8, 1e-13, 4096);
  if (!dressed.valid) return result;
  result.longitudinal = dressed.state;
  result.state = dressed.state;
  for (int particle = 0; particle < kMatterCount; ++particle) {
    Vec3 momentum = chart.reference.constituents[particle].momentum;
    for (int axis = 0; axis < 3; ++axis)
      set_component(momentum, axis, component(momentum, axis)
          + amount * tangent.dp[3 * particle + axis]);
    result.state.constituents[particle].momentum = momentum;
  }
  add_scaled(result.state.electric, tangent.e, amount);
  result.state.magnetic_half = chart.reference.magnetic_half;
  add_scaled(result.state.magnetic_half, tangent.b, amount);
  const auto deposited = chart.deposit_fn(result.state);
  result.gauss_residual = INFINITY;
  if (deposited.valid) {
    result.gauss_residual = 0.0;
    const auto div = divergence(result.state.electric);
    for (std::size_t i = 0; i < kVolume; ++i)
      result.gauss_residual = std::max(
          result.gauss_residual, std::abs(div[i] - deposited.rho[i]));
  }
  result.poisson_iterations = dressed.poisson_iterations;
  result.poisson_absolute_residual = dressed.poisson_residual;
  double source_scale = deposited.valid ? max_abs(deposited.rho) : 0.0;
  result.poisson_relative_residual = dressed.poisson_residual
      / std::max(source_scale, 1e-30);
  result.metadata = same_metadata(chart.reference, result.state);
  result.sector = chart.sector_fn(chart.reference) == chart.sector_fn(result.state);
  result.finite = finite_face(result.state.electric)
      && finite_edge(result.state.magnetic_half)
      && std::all_of(result.state.constituents.begin(),
          result.state.constituents.end(), [](const auto& point) {
            return std::isfinite(point.remainder.x)
                && std::isfinite(point.remainder.y)
                && std::isfinite(point.remainder.z)
                && std::isfinite(point.momentum.x)
                && std::isfinite(point.momentum.y)
                && std::isfinite(point.momentum.z);
          });
  result.valid = result.metadata && result.sector && result.finite
      && result.gauss_residual <= 1e-10
      && result.poisson_absolute_residual <= 1e-13;
  return result;
}

struct CodecResult {
  bool valid = false;
  double preclean_divergence = INFINITY;
  double cleaned_divergence = INFINITY;
  double hodge_correction = INFINITY;
  double reconstruction = INFINITY;
  double face_harmonic = INFINITY;
  double edge_harmonic = INFINITY;
  std::array<double,3> face_raw{{NAN,NAN,NAN}};
  std::array<double,3> face_rebuilt{{NAN,NAN,NAN}};
  std::array<double,3> edge_raw{{NAN,NAN,NAN}};
  std::array<double,3> edge_rebuilt{{NAN,NAN,NAN}};
  double tangent_source_mean_abs = INFINITY;
  double tangent_source_mean_rel = INFINITY;
  double hodge_source_mean_abs = INFINITY;
  double hodge_source_mean_rel = INFINITY;
  double tangent_poisson_relative_residual = INFINITY;
  double hodge_poisson_relative_residual = INFINITY;
  ChartVector value;
};

template <class Chart>
inline CodecResult encode_centered(const Chart& chart,
                                   const ConnectedMooreBlockState& plus,
                                   const ConnectedMooreBlockState& minus,
                                   double h) {
  CodecResult result;
  if (!(h > 0.0) || !same_metadata(plus, minus)
      || chart.sector_fn(plus) != chart.sector_fn(minus)) return result;
  const double scale = 1.0 / (2.0 * h);
  for (int particle = 0; particle < kMatterCount; ++particle) {
    const Vec3 xp = chart.position_fn(plus.constituents[particle]);
    const Vec3 xm = chart.position_fn(minus.constituents[particle]);
    for (int axis = 0; axis < 3; ++axis) {
      result.value.dx[3 * particle + axis] =
          scale * (component(xp, axis) - component(xm, axis));
      result.value.dp[3 * particle + axis] = scale * (
          component(plus.constituents[particle].momentum, axis)
          - component(minus.constituents[particle].momentum, axis));
    }
  }
  const MatchedFaceFlux delta_e = face_difference(
      plus.electric, minus.electric, scale);
  result.value.b = edge_difference(
      plus.magnetic_half, minus.magnetic_half, scale);
  result.face_raw = face_means(delta_e);
  result.edge_raw = edge_means(result.value.b);
  const auto longitudinal = longitudinal_from_dx(
      chart.density_jet, result.value.dx);
  result.tangent_poisson_relative_residual = longitudinal.relative_residual;
  result.tangent_source_mean_abs = longitudinal.compatibility_absolute;
  result.tangent_source_mean_rel = longitudinal.compatibility_relative;
  if (!longitudinal.valid) return result;
  MatchedFaceFlux longitudinal_field = longitudinal.field;
  enforce_zero_face_means(longitudinal_field);
  MatchedFaceFlux residue = face_difference(delta_e, longitudinal_field);
  // Work in the zero-harmonic summand while cleaning.  The three raw
  // coefficients are carried separately and reinserted exactly below.
  for (std::size_t i = 0; i < kVolume; ++i) {
    residue.x[i] -= result.face_raw[0];
    residue.y[i] -= result.face_raw[1];
    residue.z[i] -= result.face_raw[2];
  }
  enforce_zero_face_means(residue);
  result.preclean_divergence = ftd::eft::max_divergence(residue);
  const auto hodge = solve_longitudinal(divergence(residue), 1e-13, 4096);
  result.hodge_poisson_relative_residual = hodge.relative_residual;
  result.hodge_source_mean_abs = hodge.compatibility_absolute;
  result.hodge_source_mean_rel = hodge.compatibility_relative;
  if (!hodge.valid) return result;
  MatchedFaceFlux hodge_field = hodge.field;
  enforce_zero_face_means(hodge_field);
  result.value.e = face_difference(residue, hodge_field);
  enforce_zero_face_means(result.value.e);
  for (std::size_t i = 0; i < kVolume; ++i) {
    result.value.e.x[i] += result.face_raw[0];
    result.value.e.y[i] += result.face_raw[1];
    result.value.e.z[i] += result.face_raw[2];
  }
  // Measure the retained harmonic coordinates from the completed chart
  // fields. These are replay primitives, not copied bookkeeping values.
  result.face_rebuilt = face_means(result.value.e);
  result.edge_rebuilt = edge_means(result.value.b);
  result.cleaned_divergence = ftd::eft::max_divergence(result.value.e);
  const double denominator = std::max(face_l2(delta_e), 1e-30);
  result.hodge_correction = face_l2(hodge_field) / denominator;
  MatchedFaceFlux rebuilt = longitudinal_field;
  add_scaled(rebuilt, result.value.e, 1.0);
  result.reconstruction = face_l2(face_difference(delta_e, rebuilt))
      / denominator;
  double face_scale = 1e-30, edge_scale = 1e-30;
  result.face_harmonic = 0.0;
  result.edge_harmonic = 0.0;
  for (int axis = 0; axis < 3; ++axis) {
    face_scale = std::max(face_scale, std::abs(result.face_raw[axis]));
    edge_scale = std::max(edge_scale, std::abs(result.edge_raw[axis]));
    result.face_harmonic = std::max(
        result.face_harmonic,
        std::abs(result.face_raw[axis] - result.face_rebuilt[axis]));
    result.edge_harmonic = std::max(
        result.edge_harmonic,
        std::abs(result.edge_raw[axis] - result.edge_rebuilt[axis]));
  }
  result.face_harmonic /= face_scale;
  result.edge_harmonic /= edge_scale;
  result.valid = finite_chart(result.value)
      && result.preclean_divergence <= 2e-7
      && result.cleaned_divergence <= 1e-10
      && result.hodge_correction <= 2e-4
      && result.reconstruction <= 2e-4
      && result.face_harmonic <= 1e-12
      && result.edge_harmonic <= 1e-12
      && result.tangent_source_mean_rel <= 1e-13
      && result.hodge_source_mean_rel <= 1e-13;
  return result;
}

template <class Chart>
inline double stable_energy_increment(
    const Chart& chart, const RetractionResult& retracted,
    const ChartVector& tangent, double amount, double beta, double lambda,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  if (!retracted.valid) return NAN;
  long double kinetic = 0.0L;
  for (int particle = 0; particle < kMatterCount; ++particle) {
    const Vec3 p0 = chart.reference.constituents[particle].momentum;
    const Vec3 p1 = retracted.state.constituents[particle].momentum;
    const double e0 = ftd::eft::production_flat_energy_from_momentum(p0);
    const double e1 = ftd::eft::production_flat_energy_from_momentum(p1);
    kinetic += ftd::C_SPEED * ftd::C_SPEED * (p1.mag2() - p0.mag2())
        / (e1 + e0);
  }
  long double binding = 0.0L;
  for (const auto& edge : chart.reference.edges) {
    const Vec3 d0 = chart.position_fn(chart.reference.constituents[edge.first])
        - chart.position_fn(chart.reference.constituents[edge.second]);
    const Vec3 d1 = chart.position_fn(
        retracted.state.constituents[edge.first])
        - chart.position_fn(retracted.state.constituents[edge.second]);
    const long double u0 = static_cast<long double>(d0.dot(d0))
        - edge.rest_length_squared;
    const long double u1 = static_cast<long double>(d1.dot(d1))
        - edge.rest_length_squared;
    binding += 0.25L * options.binding_stiffness * (u1 - u0) * (u1 + u0);
  }
  const auto rho0 = chart.deposit_fn(chart.reference);
  const auto rho1 = chart.deposit_fn(retracted.longitudinal);
  if (!rho0.valid || !rho1.valid) return NAN;
  const auto phi0 = solve_longitudinal(rho0.rho, 1e-13, 4096);
  std::vector<double> delta_rho(kVolume, 0.0);
  for (std::size_t i = 0; i < kVolume; ++i)
    delta_rho[i] = rho1.rho[i] - rho0.rho[i];
  const auto delta_phi = solve_longitudinal(delta_rho, 1e-13, 4096);
  if (!phi0.valid || !delta_phi.valid) return NAN;
  const long double longitudinal = vector_dot(delta_rho, phi0.potential)
      + 0.5L * vector_dot(delta_rho, delta_phi.potential);

  const MatchedFaceFlux& l1 = retracted.longitudinal.electric;
  const MatchedFaceFlux& l0 = chart.reference.electric;
  const MatchedEdgeField& b0 = chart.reference.magnetic_half;
  const MatchedEdgeField curl_l1 = ftd::eft::matched_curl_adjoint(l1);
  const MatchedEdgeField curl_l0 = ftd::eft::matched_curl_adjoint(l0);
  const MatchedEdgeField curl_e = ftd::eft::matched_curl_adjoint(tangent.e);
  const long double transverse =
      amount * ftd::eft::matched_face_dot(l1, tangent.e)
      + 0.5L * amount * amount
          * ftd::eft::matched_face_dot(tangent.e, tangent.e)
      + amount * ftd::eft::matched_edge_dot(b0, tangent.b)
      + 0.5L * amount * amount
          * ftd::eft::matched_edge_dot(tangent.b, tangent.b)
      - 0.5L * lambda * (
          ftd::eft::matched_edge_dot(b0, curl_l1)
          - ftd::eft::matched_edge_dot(b0, curl_l0)
          + amount * ftd::eft::matched_edge_dot(b0, curl_e)
          + amount * ftd::eft::matched_edge_dot(tangent.b, curl_l1)
          + amount * amount
              * ftd::eft::matched_edge_dot(tangent.b, curl_e));
  return static_cast<double>(kinetic + binding
      + static_cast<long double>(beta) * (longitudinal + transverse));
}

struct SymmetricEigen {
  bool valid = false;
  double residual = INFINITY;
  double orthogonality = INFINITY;
  std::vector<double> values;
  SmallMatrix vectors;  // vectors[row][column]
};

inline SymmetricEigen symmetric_eigen(const SmallMatrix& input,
                                      double offdiag_gate = 1e-13) {
  SymmetricEigen result;
  const int n = static_cast<int>(input.size());
  if (n <= 0) return result;
  for (const auto& row : input) if (static_cast<int>(row.size()) != n) return result;
  SmallMatrix a = input;
  SmallMatrix vectors(n, std::vector<double>(n, 0.0));
  for (int i = 0; i < n; ++i) vectors[i][i] = 1.0;
  const int limit = std::max(1000, 80 * n * n);
  for (int iteration = 0; iteration < limit; ++iteration) {
    int p = 0, q = n > 1 ? 1 : 0;
    double largest = 0.0;
    for (int i = 0; i < n; ++i)
      for (int j = i + 1; j < n; ++j)
        if (std::abs(a[i][j]) > largest) {
          largest = std::abs(a[i][j]); p = i; q = j;
        }
    if (largest <= offdiag_gate || n == 1) break;
    const double angle = 0.5 * std::atan2(
        2.0 * a[p][q], a[q][q] - a[p][p]);
    const double c = std::cos(angle), s = std::sin(angle);
    for (int k = 0; k < n; ++k) if (k != p && k != q) {
      const double kp = a[k][p], kq = a[k][q];
      a[k][p] = a[p][k] = c * kp - s * kq;
      a[k][q] = a[q][k] = s * kp + c * kq;
    }
    const double pp = a[p][p], qq = a[q][q], pq = a[p][q];
    a[p][p] = c * c * pp - 2.0 * c * s * pq + s * s * qq;
    a[q][q] = s * s * pp + 2.0 * c * s * pq + c * c * qq;
    a[p][q] = a[q][p] = 0.0;
    for (int k = 0; k < n; ++k) {
      const double kp = vectors[k][p], kq = vectors[k][q];
      vectors[k][p] = c * kp - s * kq;
      vectors[k][q] = s * kp + c * kq;
    }
  }
  std::vector<int> order(n);
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&](int left, int right) {
    return a[left][left] < a[right][right];
  });
  result.values.resize(n);
  result.vectors.assign(n, std::vector<double>(n, 0.0));
  for (int column = 0; column < n; ++column) {
    const int source = order[column];
    result.values[column] = a[source][source];
    for (int row = 0; row < n; ++row)
      result.vectors[row][column] = vectors[row][source];
  }
  result.residual = 0.0;
  result.orthogonality = 0.0;
  for (int column = 0; column < n; ++column) {
    for (int row = 0; row < n; ++row) {
      long double image = 0.0L;
      for (int j = 0; j < n; ++j)
        image += static_cast<long double>(input[row][j])
            * result.vectors[j][column];
      result.residual = std::max(result.residual,
          std::abs(static_cast<double>(image)
              - result.values[column] * result.vectors[row][column]));
    }
    for (int other = 0; other < n; ++other) {
      long double product = 0.0L;
      for (int row = 0; row < n; ++row)
        product += static_cast<long double>(result.vectors[row][column])
            * result.vectors[row][other];
      result.orthogonality = std::max(result.orthogonality,
          std::abs(static_cast<double>(product) - (column == other ? 1.0 : 0.0)));
    }
  }
  result.valid = std::all_of(result.values.begin(), result.values.end(),
      [](double value) { return std::isfinite(value); });
  return result;
}

inline SmallMatrix identity_matrix(int n) {
  SmallMatrix result(n, std::vector<double>(n, 0.0));
  for (int i = 0; i < n; ++i) result[i][i] = 1.0;
  return result;
}

inline SmallMatrix transpose(const SmallMatrix& input) {
  if (input.empty()) return {};
  SmallMatrix result(input.front().size(),
                     std::vector<double>(input.size(), 0.0));
  for (std::size_t i = 0; i < input.size(); ++i)
    for (std::size_t j = 0; j < input[i].size(); ++j)
      result[j][i] = input[i][j];
  return result;
}

inline SmallMatrix multiply(const SmallMatrix& left,
                            const SmallMatrix& right) {
  if (left.empty() || right.empty() || left.front().size() != right.size())
    return {};
  SmallMatrix result(left.size(),
                     std::vector<double>(right.front().size(), 0.0));
  for (std::size_t i = 0; i < left.size(); ++i)
    for (std::size_t k = 0; k < right.size(); ++k)
      for (std::size_t j = 0; j < right[k].size(); ++j)
        result[i][j] += left[i][k] * right[k][j];
  return result;
}

inline double frobenius(const SmallMatrix& matrix) {
  long double value = 0.0L;
  for (const auto& row : matrix)
    for (double entry : row) value += static_cast<long double>(entry) * entry;
  return std::sqrt(static_cast<double>(value));
}

inline double relative_frobenius(const SmallMatrix& left,
                                 const SmallMatrix& right) {
  if (left.size() != right.size()) return INFINITY;
  SmallMatrix difference = left;
  for (std::size_t i = 0; i < left.size(); ++i) {
    if (left[i].size() != right[i].size()) return INFINITY;
    for (std::size_t j = 0; j < left[i].size(); ++j)
      difference[i][j] -= right[i][j];
  }
  return frobenius(difference) / std::max(frobenius(right), 1e-30);
}

struct GeneralReal4Eigen {
  bool valid = false;
  double residual = INFINITY;
  double polynomial_residual = INFINITY;
  double determinant_residual = INFINITY;
  double coefficient_residual = INFINITY;
  std::array<std::complex<double>, 4> values{{
      std::complex<double>{NAN, NAN}, std::complex<double>{NAN, NAN},
      std::complex<double>{NAN, NAN}, std::complex<double>{NAN, NAN}}};
};

namespace real4_detail {

using LongComplex = std::complex<long double>;
using LongMatrix = std::array<std::array<long double, 4>, 4>;
using ComplexMatrix = std::array<std::array<LongComplex, 4>, 4>;
using Polynomial = std::array<long double, 5>;
using Roots = std::array<LongComplex, 4>;

struct SchurRoots {
  bool converged = false;
  Roots values{};
};

inline bool finite(const LongComplex& value) {
  return std::isfinite(value.real()) && std::isfinite(value.imag());
}

inline LongComplex polynomial_value(const Polynomial& coefficients,
                                    const LongComplex& argument) {
  LongComplex value = coefficients[0];
  for (int i = 1; i < 5; ++i)
    value = value * argument + coefficients[i];
  return value;
}

inline LongComplex polynomial_derivative(const Polynomial& coefficients,
                                         const LongComplex& argument) {
  LongComplex value = 4.0L * coefficients[0];
  for (int i = 1; i < 4; ++i)
    value = value * argument + (4 - i) * coefficients[i];
  return value;
}

inline long double polynomial_scale(const Polynomial& coefficients,
                                    const LongComplex& argument) {
  const long double magnitude = std::abs(argument);
  long double scale = std::abs(coefficients[0]);
  for (int i = 1; i < 5; ++i)
    scale = scale * magnitude + std::abs(coefficients[i]);
  return std::max(scale, 1.0L);
}

inline long double polynomial_residual(const Polynomial& coefficients,
                                       const Roots& roots) {
  long double residual = 0.0L;
  for (const auto& root : roots) {
    if (!finite(root)) return INFINITY;
    residual = std::max(residual,
        std::abs(polynomial_value(coefficients, root))
            / polynomial_scale(coefficients, root));
  }
  return residual;
}

inline long double coefficient_residual(const Polynomial& coefficients,
                                        const Roots& roots) {
  std::array<LongComplex, 5> reconstructed{};
  reconstructed[0] = 1.0L;
  int degree = 0;
  for (const auto& root : roots) {
    std::array<LongComplex, 5> next{};
    for (int i = 0; i <= degree; ++i) {
      next[i] += reconstructed[i];
      next[i + 1] -= root * reconstructed[i];
    }
    reconstructed = next;
    ++degree;
  }
  long double residual = 0.0L;
  for (int i = 0; i < 5; ++i) {
    if (!finite(reconstructed[i])) return INFINITY;
    residual = std::max(residual,
        std::abs(reconstructed[i] - LongComplex(coefficients[i], 0.0L))
            / std::max(1.0L, std::abs(coefficients[i])));
  }
  return residual;
}

inline long double root_set_residual(const Polynomial& coefficients,
                                     const Roots& roots) {
  return std::max(polynomial_residual(coefficients, roots),
                  coefficient_residual(coefficients, roots));
}

inline LongComplex shifted_determinant(const LongMatrix& matrix,
                                       const LongComplex& argument) {
  std::array<std::array<LongComplex, 4>, 4> work{};
  for (int row = 0; row < 4; ++row)
    for (int column = 0; column < 4; ++column)
      work[row][column] = (row == column ? argument : LongComplex{})
          - matrix[row][column];
  LongComplex determinant = 1.0L;
  int sign = 1;
  for (int column = 0; column < 4; ++column) {
    int pivot = column;
    long double pivot_magnitude = std::abs(work[pivot][column]);
    for (int row = column + 1; row < 4; ++row) {
      const long double candidate = std::abs(work[row][column]);
      if (candidate > pivot_magnitude) {
        pivot = row;
        pivot_magnitude = candidate;
      }
    }
    if (pivot_magnitude == 0.0L) return LongComplex{};
    if (pivot != column) {
      std::swap(work[pivot], work[column]);
      sign = -sign;
    }
    const LongComplex diagonal = work[column][column];
    determinant *= diagonal;
    for (int row = column + 1; row < 4; ++row) {
      const LongComplex factor = work[row][column] / diagonal;
      for (int j = column + 1; j < 4; ++j)
        work[row][j] -= factor * work[column][j];
    }
  }
  return static_cast<long double>(sign) * determinant;
}

inline long double determinant_residual(const LongMatrix& matrix,
                                        const Roots& roots) {
  long double residual = 0.0L;
  for (const auto& root : roots) {
    long double hadamard_scale = 1.0L;
    for (int row = 0; row < 4; ++row) {
      long double squared_norm = 0.0L;
      for (int column = 0; column < 4; ++column) {
        const LongComplex entry =
            (row == column ? root : LongComplex{}) - matrix[row][column];
        squared_norm += std::norm(entry);
      }
      hadamard_scale *= std::max(1.0L, std::sqrt(squared_norm));
    }
    const LongComplex determinant = shifted_determinant(matrix, root);
    if (!finite(determinant) || !std::isfinite(hadamard_scale)) return INFINITY;
    residual = std::max(residual, std::abs(determinant) / hadamard_scale);
  }
  return residual;
}

inline Polynomial characteristic_polynomial(const LongMatrix& matrix) {
  Polynomial coefficients{{1.0L, 0.0L, 0.0L, 0.0L, 0.0L}};
  LongMatrix leverrier{};
  for (int i = 0; i < 4; ++i) leverrier[i][i] = 1.0L;
  for (int order = 1; order <= 4; ++order) {
    LongMatrix product{};
    for (int row = 0; row < 4; ++row)
      for (int inner = 0; inner < 4; ++inner)
        for (int column = 0; column < 4; ++column)
          product[row][column] +=
              matrix[row][inner] * leverrier[inner][column];
    long double trace = 0.0L;
    for (int i = 0; i < 4; ++i) trace += product[i][i];
    coefficients[order] = -trace / order;
    for (int i = 0; i < 4; ++i)
      product[i][i] += coefficients[order];
    leverrier = product;
  }
  return coefficients;
}

struct Householder {
  std::array<LongComplex, 4> vector{};
  int size = 0;
  long double beta = 0.0L;
};

inline Householder householder(
    const std::array<LongComplex, 4>& input, int size) {
  Householder result;
  result.vector = input;
  result.size = size;
  long double squared_norm = 0.0L;
  for (int i = 0; i < size; ++i) squared_norm += std::norm(input[i]);
  const long double norm = std::sqrt(squared_norm);
  if (norm == 0.0L) return result;
  const LongComplex phase = std::abs(input[0]) == 0.0L
      ? LongComplex{1.0L, 0.0L} : input[0] / std::abs(input[0]);
  result.vector[0] += phase * norm;
  long double denominator = 0.0L;
  for (int i = 0; i < size; ++i)
    denominator += std::norm(result.vector[i]);
  if (denominator != 0.0L) result.beta = 2.0L / denominator;
  return result;
}

inline ComplexMatrix upper_hessenberg(const LongMatrix& input) {
  ComplexMatrix matrix{};
  for (int row = 0; row < 4; ++row)
    for (int column = 0; column < 4; ++column)
      matrix[row][column] = input[row][column];
  for (int column = 0; column < 2; ++column) {
    const int offset = column + 1;
    const int size = 4 - offset;
    std::array<LongComplex, 4> entries{};
    for (int i = 0; i < size; ++i)
      entries[i] = matrix[offset + i][column];
    const Householder reflection = householder(entries, size);
    if (reflection.beta == 0.0L) continue;
    for (int j = column; j < 4; ++j) {
      LongComplex product{};
      for (int i = 0; i < size; ++i)
        product += std::conj(reflection.vector[i])
            * matrix[offset + i][j];
      for (int i = 0; i < size; ++i)
        matrix[offset + i][j] -= reflection.beta
            * reflection.vector[i] * product;
    }
    for (int i = 0; i < 4; ++i) {
      LongComplex product{};
      for (int j = 0; j < size; ++j)
        product += matrix[i][offset + j] * reflection.vector[j];
      for (int j = 0; j < size; ++j)
        matrix[i][offset + j] -= reflection.beta * product
            * std::conj(reflection.vector[j]);
    }
    for (int row = column + 2; row < 4; ++row)
      matrix[row][column] = LongComplex{};
  }
  return matrix;
}

inline LongComplex wilkinson_shift(const ComplexMatrix& matrix, int high) {
  const LongComplex a = matrix[high - 1][high - 1];
  const LongComplex b = matrix[high - 1][high];
  const LongComplex c = matrix[high][high - 1];
  const LongComplex d = matrix[high][high];
  const LongComplex half_difference = 0.5L * (a - d);
  const LongComplex discriminant =
      std::sqrt(half_difference * half_difference + b * c);
  const LongComplex center = 0.5L * (a + d);
  const LongComplex first = center + discriminant;
  const LongComplex second = center - discriminant;
  return std::abs(first - d) <= std::abs(second - d) ? first : second;
}

inline void shifted_qr_step(ComplexMatrix& matrix, int low, int high,
                            const LongComplex& shift) {
  const int size = high - low + 1;
  ComplexMatrix triangular{};
  ComplexMatrix unitary{};
  for (int i = 0; i < size; ++i) {
    unitary[i][i] = 1.0L;
    for (int j = 0; j < size; ++j)
      triangular[i][j] = matrix[low + i][low + j]
          - (i == j ? shift : LongComplex{});
  }
  for (int column = 0; column < size; ++column) {
    const int tail = size - column;
    std::array<LongComplex, 4> entries{};
    for (int i = 0; i < tail; ++i)
      entries[i] = triangular[column + i][column];
    const Householder reflection = householder(entries, tail);
    if (reflection.beta == 0.0L) continue;
    for (int j = column; j < size; ++j) {
      LongComplex product{};
      for (int i = 0; i < tail; ++i)
        product += std::conj(reflection.vector[i])
            * triangular[column + i][j];
      for (int i = 0; i < tail; ++i)
        triangular[column + i][j] -= reflection.beta
            * reflection.vector[i] * product;
    }
    for (int row = 0; row < size; ++row) {
      LongComplex product{};
      for (int i = 0; i < tail; ++i)
        product += unitary[row][column + i] * reflection.vector[i];
      for (int i = 0; i < tail; ++i)
        unitary[row][column + i] -= reflection.beta * product
            * std::conj(reflection.vector[i]);
    }
  }
  ComplexMatrix next{};
  for (int row = 0; row < size; ++row)
    for (int inner = 0; inner < size; ++inner)
      for (int column = 0; column < size; ++column)
        next[row][column] +=
            triangular[row][inner] * unitary[inner][column];
  for (int row = 0; row < size; ++row)
    for (int column = 0; column < size; ++column)
      matrix[low + row][low + column] = next[row][column]
          + (row == column ? shift : LongComplex{});
  // Exact Hessenberg structure follows from a QR step on a Hessenberg block.
  // Reasserting those structural zeros prevents roundoff fill from accumulating.
  for (int row = low + 2; row <= high; ++row)
    for (int column = low; column < row - 1; ++column)
      matrix[row][column] = LongComplex{};
}

inline SchurRoots complex_schur_roots(const LongMatrix& input) {
  SchurRoots result;
  ComplexMatrix matrix = upper_hessenberg(input);
  const long double epsilon = std::numeric_limits<double>::epsilon();
  int high = 3;
  int iterations = 0;
  int iterations_since_deflation = 0;
  while (high >= 0 && iterations < 8192) {
    int low = high;
    while (low > 0) {
      const long double diagonal_scale = std::max(1.0L,
          std::abs(matrix[low - 1][low - 1])
              + std::abs(matrix[low][low]));
      if (std::abs(matrix[low][low - 1])
          <= 128.0L * epsilon * diagonal_scale) {
        matrix[low][low - 1] = LongComplex{};
        break;
      }
      --low;
    }
    if (low == high) {
      result.values[high] = matrix[high][high];
      --high;
      iterations_since_deflation = 0;
      continue;
    }
    if (high - low == 1) {
      const LongComplex a = matrix[low][low];
      const LongComplex b = matrix[low][high];
      const LongComplex c = matrix[high][low];
      const LongComplex d = matrix[high][high];
      const LongComplex half_difference = 0.5L * (a - d);
      const LongComplex discriminant =
          std::sqrt(half_difference * half_difference + b * c);
      const LongComplex center = 0.5L * (a + d);
      result.values[low] = center + discriminant;
      result.values[high] = center - discriminant;
      high -= 2;
      iterations_since_deflation = 0;
      continue;
    }
    LongComplex shift = wilkinson_shift(matrix, high);
    if (iterations_since_deflation > 0
        && iterations_since_deflation % 64 == 0) {
      shift = matrix[high][high]
          + LongComplex{0.75L, 0.25L}
              * std::abs(matrix[high][high - 1]);
    }
    shifted_qr_step(matrix, low, high, shift);
    ++iterations;
    ++iterations_since_deflation;
  }
  result.converged = high < 0
      && std::all_of(result.values.begin(), result.values.end(), finite);
  return result;
}

inline Roots aberth_roots(const Polynomial& coefficients) {
  Roots roots{};
  long double radius = 1.0L;
  for (int i = 1; i < 5; ++i)
    radius = std::max(radius, 1.0L + std::abs(coefficients[i]));
  const long double pi = std::acos(-1.0L);
  for (int i = 0; i < 4; ++i)
    roots[i] = std::polar(radius, 2.0L * pi * (i + 0.5L) / 4.0L);

  Roots best = roots;
  long double best_residual = root_set_residual(coefficients, roots);
  const long double epsilon = std::numeric_limits<long double>::epsilon();
  for (int iteration = 0; iteration < 2048; ++iteration) {
    Roots next = roots;
    long double maximum_correction = 0.0L;
    long double maximum_root = 0.0L;
    for (int i = 0; i < 4; ++i) {
      const LongComplex value = polynomial_value(coefficients, roots[i]);
      const LongComplex derivative =
          polynomial_derivative(coefficients, roots[i]);
      const long double magnitude = std::abs(roots[i]);
      const long double derivative_scale = std::max(1.0L,
          4.0L * magnitude * magnitude * magnitude
          + 3.0L * std::abs(coefficients[1]) * magnitude * magnitude
          + 2.0L * std::abs(coefficients[2]) * magnitude
          + std::abs(coefficients[3]));
      LongComplex correction{};
      bool have_correction = false;
      if (std::abs(derivative) > 64.0L * epsilon * derivative_scale) {
        const LongComplex newton = value / derivative;
        LongComplex repulsion{};
        bool separated = true;
        for (int other = 0; other < 4; ++other) if (other != i) {
          const LongComplex difference = roots[i] - roots[other];
          if (std::abs(difference)
              <= std::numeric_limits<long double>::min()) {
            separated = false;
            break;
          }
          repulsion += 1.0L / difference;
        }
        const LongComplex denominator = 1.0L - newton * repulsion;
        if (separated && std::abs(denominator) > 64.0L * epsilon) {
          correction = newton / denominator;
          have_correction = finite(correction);
        }
        if (!have_correction && finite(newton)) {
          correction = newton;
          have_correction = true;
        }
      } else {
        LongComplex denominator = 1.0L;
        bool separated = true;
        for (int other = 0; other < 4; ++other) if (other != i) {
          const LongComplex difference = roots[i] - roots[other];
          if (std::abs(difference)
              <= std::numeric_limits<long double>::min()) {
            separated = false;
            break;
          }
          denominator *= difference;
        }
        if (separated
            && std::abs(denominator) > std::numeric_limits<long double>::min()) {
          correction = value / denominator;
          have_correction = finite(correction);
        }
      }
      if (have_correction) next[i] = roots[i] - correction;
      maximum_correction = std::max(maximum_correction,
          have_correction ? std::abs(correction) : 0.0L);
      maximum_root = std::max(maximum_root, std::abs(next[i]));
    }
    roots = next;
    const long double candidate_residual =
        root_set_residual(coefficients, roots);
    if (candidate_residual < best_residual) {
      best = roots;
      best_residual = candidate_residual;
    }
    if (best_residual <= 4096.0L * epsilon
        && maximum_correction
            <= 64.0L * epsilon * std::max(1.0L, maximum_root)) break;
  }

  Roots polished = best;
  for (auto& root : polished) {
    long double root_residual = std::abs(polynomial_value(coefficients, root))
        / polynomial_scale(coefficients, root);
    for (int iteration = 0; iteration < 32; ++iteration) {
      const LongComplex derivative = polynomial_derivative(coefficients, root);
      if (std::abs(derivative)
          <= 64.0L * epsilon * polynomial_scale(coefficients, root)) break;
      const LongComplex candidate =
          root - polynomial_value(coefficients, root) / derivative;
      if (!finite(candidate)) break;
      const long double candidate_residual =
          std::abs(polynomial_value(coefficients, candidate))
              / polynomial_scale(coefficients, candidate);
      if (candidate_residual > root_residual) break;
      root = candidate;
      root_residual = candidate_residual;
    }
  }
  if (root_set_residual(coefficients, polished) < best_residual)
    best = polished;
  return best;
}

}  // namespace real4_detail

inline GeneralReal4Eigen general_real4_eigenvalues(
    const SmallMatrix& input, double residual_gate = 1e-10) {
  GeneralReal4Eigen result;
  if (input.size() != 4 || !(residual_gate > 0.0)
      || !std::isfinite(residual_gate)) return result;
  long double input_scale = 0.0L;
  for (const auto& row : input) {
    if (row.size() != 4) return result;
    for (double entry : row) {
      if (!std::isfinite(entry)) return result;
      input_scale = std::max(input_scale,
          std::abs(static_cast<long double>(entry)));
    }
  }
  if (input_scale == 0.0L) input_scale = 1.0L;
  real4_detail::LongMatrix scaled{};
  for (int row = 0; row < 4; ++row)
    for (int column = 0; column < 4; ++column)
      scaled[row][column] = input[row][column] / input_scale;

  const auto coefficients =
      real4_detail::characteristic_polynomial(scaled);
  if (!std::all_of(coefficients.begin(), coefficients.end(),
      [](long double value) { return std::isfinite(value); })) return result;
  const auto schur = real4_detail::complex_schur_roots(scaled);
  const auto roots = schur.converged
      ? schur.values : real4_detail::aberth_roots(coefficients);
  for (int i = 0; i < 4; ++i) {
    const real4_detail::LongComplex value = input_scale * roots[i];
    result.values[i] = {static_cast<double>(value.real()),
                        static_cast<double>(value.imag())};
  }
  const bool finite_values = std::all_of(
      result.values.begin(), result.values.end(), [](const auto& value) {
        return std::isfinite(value.real()) && std::isfinite(value.imag());
      });
  if (!finite_values) return result;
  std::sort(result.values.begin(), result.values.end(),
      [](const auto& left, const auto& right) {
        if (left.real() != right.real()) return left.real() < right.real();
        return left.imag() < right.imag();
      });
  real4_detail::Roots returned_roots{};
  for (int i = 0; i < 4; ++i)
    returned_roots[i] = real4_detail::LongComplex(
        result.values[i].real(), result.values[i].imag()) / input_scale;
  // The reported residual applies to the returned double-precision roots,
  // rather than to the extended-precision iterates from which they came.
  result.polynomial_residual = static_cast<double>(
      real4_detail::polynomial_residual(coefficients, returned_roots));
  result.coefficient_residual = static_cast<double>(
      real4_detail::coefficient_residual(coefficients, returned_roots));
  result.determinant_residual = static_cast<double>(
      real4_detail::determinant_residual(scaled, returned_roots));
  result.residual = std::max({result.polynomial_residual,
      result.coefficient_residual, result.determinant_residual});
  result.valid = std::isfinite(result.residual)
      && result.residual <= residual_gate;
  return result;
}

inline double principal_sine(const SmallMatrix& overlap) {
  if (overlap.empty() || overlap.size() != overlap.front().size()) return INFINITY;
  const SmallMatrix gram = multiply(transpose(overlap), overlap);
  const auto eig = symmetric_eigen(gram);
  if (!eig.valid || eig.values.empty()) return INFINITY;
  return std::sqrt(std::max(0.0, 1.0 - std::max(0.0, eig.values.front())));
}

// Compact, deterministic SHA-256 used only for pre-execution provenance and
// result-manifest fingerprints.
class Sha256 {
 public:
  Sha256() { reset(); }

  void update(const unsigned char* data, std::size_t length) {
    for (std::size_t i = 0; i < length; ++i) {
      buffer_[buffer_size_++] = data[i];
      if (buffer_size_ == 64) {
        transform();
        bit_count_ += 512;
        buffer_size_ = 0;
      }
    }
  }

  std::string finish() {
    const std::uint64_t total_bits = bit_count_ + 8 * buffer_size_;
    buffer_[buffer_size_++] = 0x80;
    if (buffer_size_ > 56) {
      while (buffer_size_ < 64) buffer_[buffer_size_++] = 0;
      transform();
      buffer_size_ = 0;
    }
    while (buffer_size_ < 56) buffer_[buffer_size_++] = 0;
    for (int i = 7; i >= 0; --i)
      buffer_[buffer_size_++] = static_cast<unsigned char>(total_bits >> (8 * i));
    transform();
    std::ostringstream out;
    out << std::uppercase << std::hex << std::setfill('0');
    for (std::uint32_t value : state_) out << std::setw(8) << value;
    return out.str();
  }

 private:
  static constexpr std::array<std::uint32_t, 64> k_{{
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
    0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
    0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
    0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
    0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
    0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u}};

  static std::uint32_t rotate(std::uint32_t value, int bits) {
    return (value >> bits) | (value << (32 - bits));
  }

  void reset() {
    state_ = {{0x6a09e667u,0xbb67ae85u,0x3c6ef372u,0xa54ff53au,
               0x510e527fu,0x9b05688cu,0x1f83d9abu,0x5be0cd19u}};
    buffer_size_ = 0;
    bit_count_ = 0;
  }

  void transform() {
    std::uint32_t w[64]{};
    for (int i = 0; i < 16; ++i)
      w[i] = (static_cast<std::uint32_t>(buffer_[4*i]) << 24)
          | (static_cast<std::uint32_t>(buffer_[4*i+1]) << 16)
          | (static_cast<std::uint32_t>(buffer_[4*i+2]) << 8)
          | static_cast<std::uint32_t>(buffer_[4*i+3]);
    for (int i = 16; i < 64; ++i) {
      const std::uint32_t s0 = rotate(w[i-15],7) ^ rotate(w[i-15],18)
          ^ (w[i-15] >> 3);
      const std::uint32_t s1 = rotate(w[i-2],17) ^ rotate(w[i-2],19)
          ^ (w[i-2] >> 10);
      w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    std::uint32_t a=state_[0],b=state_[1],c=state_[2],d=state_[3];
    std::uint32_t e=state_[4],f=state_[5],g=state_[6],h=state_[7];
    for (int i = 0; i < 64; ++i) {
      const std::uint32_t s1 = rotate(e,6) ^ rotate(e,11) ^ rotate(e,25);
      const std::uint32_t choice = (e & f) ^ (~e & g);
      const std::uint32_t temp1 = h + s1 + choice + k_[i] + w[i];
      const std::uint32_t s0 = rotate(a,2) ^ rotate(a,13) ^ rotate(a,22);
      const std::uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
      const std::uint32_t temp2 = s0 + majority;
      h=g; g=f; f=e; e=d+temp1; d=c; c=b; b=a; a=temp1+temp2;
    }
    state_[0]+=a;state_[1]+=b;state_[2]+=c;state_[3]+=d;
    state_[4]+=e;state_[5]+=f;state_[6]+=g;state_[7]+=h;
  }

  std::array<std::uint32_t, 8> state_{};
  std::array<unsigned char, 64> buffer_{};
  std::size_t buffer_size_ = 0;
  std::uint64_t bit_count_ = 0;
};

inline std::string sha256_file(const std::string& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input) return {};
  Sha256 hash;
  std::array<unsigned char, 1 << 15> buffer{};
  while (input) {
    input.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    const auto count = input.gcount();
    if (count > 0) hash.update(buffer.data(), static_cast<std::size_t>(count));
  }
  return hash.finish();
}

}  // namespace ftd0774
