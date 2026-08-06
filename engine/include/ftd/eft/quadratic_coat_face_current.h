#pragma once
/**
 * @file quadratic_coat_face_current.h
 * @brief Smooth positive coupling coat and exact straight face current (FTD-0541).
 */

#include "ftd/lattice.h"
#include "ftd/voxel.h"

#include <array>
#include <cstddef>
#include <vector>

namespace ftd::eft {

struct QuadraticCoatSiteWeight {
  Coord site{};
  double weight = 0.0;
};

struct QuadraticPolarityCoat {
  Vec3 effective_position{};
  int polarity = 0;
  std::array<QuadraticCoatSiteWeight, 27> weights{};
  std::size_t weight_count = 0;
  double partition_residual = 0.0;
  Vec3 first_moment_residual{};
  double minimum_unsigned_weight = 0.0;
  double locality_residual = 0.0;
  bool valid = false;
};

struct QuadraticCoatSparseCurrentEntry {
  Coord face{};
  int axis = 0;
  double value = 0.0;
};

/// A unique periodic oriented-face coefficient obtained after summing every
/// contributing segment.  Unlike sparse_current.size(), this representation
/// is invariant under splitting, duplication/cancellation, and periodic image
/// choice.
struct QuadraticCoatAggregatedCurrentEntry {
  Coord face{};
  int axis = 0;
  double value = 0.0;
};

struct QuadraticCoatAggregatedCurrent {
  int L = 0;
  std::vector<QuadraticCoatAggregatedCurrentEntry> entries;
  std::size_t raw_contributions = 0;
  double raw_l1 = 0.0;
  double net_l1 = 0.0;
  double cancelled_l1 = 0.0;
  double discarded_l1 = 0.0;
  double aggregation_moment_residual = 0.0;
  bool valid = false;
};

struct QuadraticCoatFaceCurrent {
  int L = 0;
  int charge = 0;
  Vec3 start_effective_position{};
  Vec3 end_effective_position{};
  QuadraticPolarityCoat start_coat{};
  QuadraticPolarityCoat end_coat{};
  std::vector<double> rho_before;
  std::vector<double> rho_after;
  std::vector<double> current_x;
  std::vector<double> current_y;
  std::vector<double> current_z;
  std::vector<QuadraticCoatSparseCurrentEntry> sparse_current;
  bool dense_materialized = true;
  int rho_support = 0;
  int current_support = 0;
  double partition_residual = 0.0;
  double first_moment_residual = 0.0;
  double continuity_residual = 0.0;
  double current_moment_residual = 0.0;
  double locality_residual = 0.0;
  double causal_excess = 0.0;
  bool valid = false;

  int index(int x, int y, int z) const;
};

/// Construct the tensor quadratic B-spline coupling coat at an unwrapped
/// effective position.  Manifestation remains site-valued outside this
/// observer.
QuadraticPolarityCoat make_quadratic_polarity_coat(
    const Vec3& effective_position, int polarity);

/// Deposit the exact current for the nearest periodic straight segment.
QuadraticCoatFaceCurrent make_quadratic_coat_face_current(
    int L,
    const Vec3& start_effective_position,
    const Vec3& end_effective_position,
    int charge,
    bool materialize_dense = true);

/// Sum a collection of sparse segment currents on unique periodic oriented
/// faces.  Coefficients with |value| <= zero_tolerance are reported through
/// discarded_l1 and omitted from entries.  This is an observer: it does not
/// alter the current applied to the field.
QuadraticCoatAggregatedCurrent aggregate_quadratic_coat_face_current(
    const std::vector<QuadraticCoatFaceCurrent>& segments,
    double scale = 1.0,
    double zero_tolerance = 0.0);

double quadratic_coat_current_divergence_at(
    const QuadraticCoatFaceCurrent& segment, int x, int y, int z);

double quadratic_coat_continuity_at(
    const QuadraticCoatFaceCurrent& segment, int x, int y, int z);

/// Pair the deposited current with a fixed oriented-face connection.
double quadratic_coat_connection_coupling(
    const QuadraticCoatFaceCurrent& segment,
    const std::vector<double>& potential_x,
    const std::vector<double>& potential_y,
    const std::vector<double>& potential_z);

}  // namespace ftd::eft
