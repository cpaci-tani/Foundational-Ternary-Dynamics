#pragma once
/**
 * @file eight_source_orbit_coherence.h
 * @brief Locked N=8 evaluation of the FTD-0590 orbit bound (FTD-0591).
 *
 * Observer-only.  The module recomputes the parent spectral orbit data and
 * evaluates all nine removal partitions without changing production state.
 */

#include "ftd/eft/removal_time_orbit_coherence.h"

#include <array>
#include <vector>

namespace ftd::eft {

struct EightSourceOrbitCoherenceVolume {
  RemovalTimeOrbitCoherenceVolume spectral{};
  std::array<double, 9> removal_partition_bounds{};
  int maximizing_removed_count = -1;
  double eight_source_orbit_bound = 0.0;
  double eight_source_margin = 0.0;
  bool all_partition_bounds_finite = false;
  bool eight_source_closed = false;
  bool valid = false;
};

struct EightSourceOrbitCoherenceResult {
  std::vector<EightSourceOrbitCoherenceVolume> volumes;
  int registered_source_count = 8;
  int spectral_volume_count = 0;
  bool parent_orbit_analysis_valid = false;
  bool all_partition_bounds_finite = false;
  bool arbitrary_removal_n_le_eight_closed = false;
  bool eight_source_bound_inconclusive = false;
  bool geometry_search_performed = false;
  bool removal_schedule_search_performed = false;
  bool production_changed = false;
  bool valid = false;
};

EightSourceOrbitCoherenceResult analyze_eight_source_orbit_coherence();

}  // namespace ftd::eft

