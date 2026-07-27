#pragma once
/**
 * @file free_flux_localization.h
 * @brief Analytic observer for the isolated free-flux localization boundary
 *        (FTD-0557).
 */

#include "ftd/lattice.h"

#include <vector>

namespace ftd::eft {

struct FreeFluxPacketDiagnostics {
  bool valid = false;
  Coord direction{};
  int spectral_mode_count = 0;
  double spectral_norm = 0.0;
  double minimum_symbol = 0.0;
  double maximum_symbol = 0.0;
  double minimum_phase = 0.0;
  double maximum_phase = 0.0;
  double mean_coordinate_velocity = 0.0;
  double coordinate_velocity_variance = 0.0;
};

struct FreeFluxLocalizationResult {
  bool valid = false;
  bool finite_range_symbol_is_real_analytic = false;
  bool transfer_trace_is_nonconstant = false;
  bool native_band_is_not_flat = false;
  bool no_nonzero_l2_point_spectrum = false;
  bool no_nonzero_finite_time_rigid_l2_translate = false;
  bool exact_branch_second_moment_identity = false;
  bool unchirped_localized_packet_must_broaden = false;
  int L = 0;
  double c2 = 0.0;
  int first_mode = 0;
  int last_mode = 0;
  double center_mode = 0.0;
  double width_modes = 0.0;
  double minimum_velocity_variance = 0.0;
  std::vector<FreeFluxPacketDiagnostics> packets;
};

FreeFluxLocalizationResult analyze_free_flux_localization(
    int L,
    int first_mode,
    int last_mode,
    double center_mode,
    double width_modes,
    const std::vector<Coord>& directions,
    double c2);

}  // namespace ftd::eft
