#pragma once
/**
 * @file nine_source_orbit_coherence.h
 * @brief Locked N=9 evaluation of the FTD-0590 orbit bound (FTD-0592).
 *
 * Observer-only.  The module recomputes the parent spectral orbit data and
 * evaluates all ten removal partitions without changing production state.
 */

#include "ftd/eft/removal_time_orbit_coherence.h"

#include <array>
#include <vector>

namespace ftd::eft {

struct NineSourceOrbitCoherenceVolume {
  RemovalTimeOrbitCoherenceVolume spectral{};
  std::array<double, 10> removal_partition_bounds{};
  int maximizing_removed_count = -1;
  double nine_source_orbit_bound = 0.0;
  double nine_source_margin = 0.0;
  bool all_partition_bounds_finite = false;
  bool nine_source_closed = false;
  bool valid = false;
};

struct NineSourceOrbitCoherenceResult {
  std::vector<NineSourceOrbitCoherenceVolume> volumes;
  int registered_source_count = 9;
  int spectral_volume_count = 0;
  bool parent_orbit_analysis_valid = false;
  bool all_partition_bounds_finite = false;
  bool arbitrary_removal_n_le_nine_closed = false;
  bool nine_source_bound_inconclusive = false;
  bool geometry_search_performed = false;
  bool removal_schedule_search_performed = false;
  bool production_changed = false;
  bool valid = false;
};

NineSourceOrbitCoherenceResult analyze_nine_source_orbit_coherence();

}  // namespace ftd::eft
