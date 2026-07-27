#pragma once
/**
 * @file ternary_collision_vertex.h
 * @brief Observer-only ternary collision-capacity and identical-worldline
 *        quotient analysis (FTD-0504).
 */

#include "ftd/eft/multibody_shape_observability.h"

#include <vector>

namespace ftd::eft {

struct TernaryCapacityResult {
  bool valid = false;
  int multiplicity = 0;
  int sign = 0;
  int required_charge = 0;
  int best_ternary_state = 0;
  int minimum_charge_defect = 0;
};

/// Exact capacity defect for m coincident same-sign unit carriers.
TernaryCapacityResult analyze_ternary_same_sign_capacity(
    int multiplicity, int sign);

struct CarrierIntrinsicAttributes {
  int polarity = 0;
  int spin_twice = 0;
  int color = 0;
  int flavor = 0;
  std::vector<int> additional_physical_tags;
};

/// Identity is physical-attribute identity. A bookkeeping particle id is
/// deliberately absent from this record.
bool physically_identical(const CarrierIntrinsicAttributes& lhs,
                          const CarrierIntrinsicAttributes& rhs);

struct PiecewiseWorldline {
  int charge = 0;
  std::vector<Vec3> vertices;
};

struct PiecewiseCurrentSignature {
  bool valid = false;
  int L = 0;
  int carrier_count = 0;
  int total_charge = 0;
  std::vector<double> rho_before;
  std::vector<double> rho_after;
  std::vector<double> current_x;
  std::vector<double> current_y;
  std::vector<double> current_z;
  double continuity_residual = 0.0;
  double current_l1 = 0.0;

  int index(int x, int y, int z) const;
};

/// Sum exact straight face-current segments over each registered polyline.
PiecewiseCurrentSignature make_piecewise_current_signature(
    int L, const std::vector<PiecewiseWorldline>& worldlines);

struct IdenticalCrossingResult {
  bool valid = false;
  bool boundary_overload = false;
  bool attributes_identical = false;
  bool label_quotient_equivalent = false;
  double collision_time = 0.0;
  double remaining_time = 0.0;
  double phase_space_multiset_residual = 0.0;
  double current_signature_residual = 0.0;
  double energy_residual = 0.0;
  double momentum_residual = 0.0;
  double charge_residual = 0.0;
  double causal_residual = 0.0;
  double continuity_residual = 0.0;
  double time_reversal_residual = 0.0;
  TernaryCapacityResult endpoint_capacity{};
  PiecewiseCurrentSignature pass_through{};
  PiecewiseCurrentSignature elastic_bounce{};
};

/** Analyze a symmetric equal-mass crossing.
 *
 * The initial points are center +/- half_separation*direction. At an interior
 * crossing the observer compares pass-through against equal-mass momentum
 * exchange after quotienting physically identical labels. A crossing exactly
 * at dt returns boundary_overload and does not invent an event rule.
 */
IdenticalCrossingResult analyze_identical_crossing(
    int L,
    const Vec3& center,
    const Vec3& direction,
    double half_separation,
    double speed,
    double dt,
    int charge,
    double rest_energy,
    double c_speed,
    const CarrierIntrinsicAttributes& lhs,
    const CarrierIntrinsicAttributes& rhs,
    double tolerance = 1e-12);

struct ElasticScatteringCounterfamily {
  bool valid = false;
  int output_count = 0;
  double momentum_magnitude = 0.0;
  double maximum_total_momentum_residual = 0.0;
  double maximum_total_energy_residual = 0.0;
  double minimum_direction_separation = 0.0;
};

/// Register five distinct COM-frame elastic output axes with the same total
/// momentum and relativistic energy.
ElasticScatteringCounterfamily analyze_elastic_scattering_counterfamily(
    double speed, double rest_energy, double c_speed);

}  // namespace ftd::eft
