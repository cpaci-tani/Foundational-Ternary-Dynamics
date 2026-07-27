#pragma once
/**
 * @file quadratic_coat_neutral_pair_work.h
 * @brief Neutral self-consistent longitudinal coat transaction (FTD-0546).
 */

#include "ftd/eft/quadratic_coat_matter_work.h"

#include <array>
#include <vector>

namespace ftd::eft {

struct QuadraticCoatNeutralPairWorkResult {
  bool valid = false;
  bool poisson_converged = false;
  int L = 0;
  int poisson_iterations = 0;
  double temporal_scale = 0.0;
  double beta = 0.0;
  double poisson_residual = 0.0;
  double neutrality_residual = 0.0;
  double temporal_gauss_residual = 0.0;
  double split_continuity_residual = 0.0;
  double endpoint_gauss_residual = 0.0;
  double field_update_residual = 0.0;
  double midpoint_split_residual = 0.0;
  double temporal_endpoint_average_mismatch = 0.0;
  double action_residual = 0.0;
  double matter_energy_change = 0.0;
  double field_energy_change = 0.0;
  double field_work = 0.0;
  double field_work_residual = 0.0;
  double pair_matter_work_defect = 0.0;
  double total_energy_defect = 0.0;

  std::array<Vec3, 2> start_position{};
  std::array<Vec3, 2> end_position{};
  std::array<int, 2> charge{};
  std::array<QuadraticCoatSpacetimeCurrent, 2> current{};
  std::array<QuadraticCoatMatterWorkResult, 2> matter{};
  std::vector<double> rho_before;
  std::vector<double> rho_after;
  std::vector<double> temporal_charge;
  DualGaugePotentialSlab slab;
  MatchedFaceFlux current_start;
  MatchedFaceFlux current_end;
  MatchedFaceFlux current_total;
  MatchedFaceFlux electric_slab;
  MatchedFaceFlux electric_before;
  MatchedFaceFlux electric_after;
  MatchedFaceFlux electric_midpoint;

  explicit QuadraticCoatNeutralPairWorkResult(int size = 0,
                                               double time_scale = 1.0);
};

QuadraticCoatNeutralPairWorkResult
evaluate_quadratic_coat_neutral_pair_work(
    int L,
    const std::array<Vec3, 2>& start_position,
    const std::array<Vec3, 2>& end_position,
    const std::array<int, 2>& charge,
    double rest_energy,
    double c_speed,
    double beta,
    double poisson_tolerance = 1e-13,
    int poisson_max_iterations = 4096);

}  // namespace ftd::eft
