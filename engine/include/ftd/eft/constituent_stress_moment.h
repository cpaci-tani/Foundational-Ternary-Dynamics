#pragma once
/**
 * @file constituent_stress_moment.h
 * @brief Observer-only minimal kinetic-stress lift for the axial current
 *        kernel (FTD-0513).
 */

#include "ftd/eft/constituent_relative_collision.h"

#include <vector>

namespace ftd::eft {

struct SymmetricTensor3 {
  double xx = 0.0;
  double yy = 0.0;
  double zz = 0.0;
  double xy = 0.0;
  double xz = 0.0;
  double yz = 0.0;

  double trace() const { return xx + yy + zz; }
  double component(int row, int column) const;
};

SymmetricTensor3 operator+(const SymmetricTensor3& lhs,
                           const SymmetricTensor3& rhs);
SymmetricTensor3 operator-(const SymmetricTensor3& lhs,
                           const SymmetricTensor3& rhs);
SymmetricTensor3 operator*(const SymmetricTensor3& tensor,
                           double scale);

struct ConstituentStressMoment {
  bool valid = false;
  int carrier_count = 0;
  double rest_energy = 0.0;
  double c_speed = 0.0;
  Vec3 total_momentum{};
  double total_energy = 0.0;
  double kinetic_energy = 0.0;
  SymmetricTensor3 stress{};
  double stress_trace = 0.0;
  double minimum_principal_minor = 0.0;
  double psd_residual = 0.0;
};

/// Sum c^2 (p tensor p)/E over an explicit constituent momentum list.
ConstituentStressMoment make_constituent_stress_moment(
    const std::vector<Vec3>& momenta,
    double rest_energy,
    double c_speed,
    double tolerance = 1e-12);

/// Transform a symmetric tensor by a signed coordinate permutation R S R^T.
SymmetricTensor3 transform_symmetric_tensor(
    const SymmetricTensor3& tensor,
    const int permutation[3],
    const int sign[3]);

struct TwoStreamStressLiftResult {
  bool valid = false;
  bool vector_current_cancelled = false;
  bool stress_retains_relative_mode = false;
  int polarity = 0;
  Coord chart_direction{};
  Vec3 collision_position{};
  ConstituentRelativeCollisionResult collision{};
  ConstituentStressMoment moment{};
  SymmetricTensor3 expected_axis_projector{};
  SymmetricTensor3 recovered_axis_projector{};
  double rank_one_residual = 0.0;
  double axis_projector_residual = 0.0;
  double recovered_single_energy = 0.0;
  double recovered_momentum_magnitude = 0.0;
  double recovered_pair_kinetic_energy = 0.0;
  double energy_recovery_residual = 0.0;
  double momentum_recovery_residual = 0.0;
  double kinetic_recovery_residual = 0.0;
};

TwoStreamStressLiftResult analyze_two_stream_stress_lift(
    int L,
    const Vec3& collision_position,
    Coord chart_direction,
    int polarity,
    double speed,
    double rest_energy,
    double c_speed,
    double tolerance = 1e-12);

struct MultistreamStressCounterexample {
  bool valid = false;
  int carrier_count = 0;
  ConstituentStressMoment axial{};
  ConstituentStressMoment diagonal{};
  double momentum_multiset_separation = 0.0;
  double total_momentum_residual = 0.0;
  double total_energy_residual = 0.0;
  double stress_residual = 0.0;
  double fourth_xxxx_difference = 0.0;
  double fourth_yyyy_difference = 0.0;
  double fourth_xxyy_difference = 0.0;
  double fourth_moment_difference = 0.0;
};

/// Four axial streams and four rotated diagonal streams with identical
/// count, total momentum, total energy, and rank-2 stress but different
/// momentum multisets and fourth moments.
MultistreamStressCounterexample analyze_multistream_stress_counterexample(
    double momentum_magnitude,
    double rest_energy,
    double c_speed,
    double tolerance = 1e-12);

}  // namespace ftd::eft
