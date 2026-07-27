#include "ftd/eft/ten_source_orbit_coherence.h"

#include "ftd/constants.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {

TenSourceOrbitCoherenceResult analyze_ten_source_orbit_coherence() {
  TenSourceOrbitCoherenceResult result;
  const RemovalTimeOrbitCoherenceResult parent =
      analyze_removal_time_orbit_coherence();
  result.parent_orbit_analysis_valid = parent.valid
      && parent.cubic_orbit_bound_derived
      && parent.all_orbit_partitions_exact
      && parent.all_direct_character_checks_pass
      && !parent.production_changed;
  result.volumes.reserve(parent.volumes.size());

  bool all_finite = true;
  bool all_closed = true;
  for (const RemovalTimeOrbitCoherenceVolume& spectral : parent.volumes) {
    TenSourceOrbitCoherenceVolume volume;
    volume.spectral = spectral;
    long double maximum_bound =
        -std::numeric_limits<long double>::infinity();
    int maximizing_removed = -1;
    bool finite = true;

    for (int removed = 0; removed <= 10; ++removed) {
      const long double remaining =
          static_cast<long double>(spectral.common_step_coefficient)
          * std::sqrt(static_cast<long double>(10 - removed));
      const long double removed_factor = static_cast<long double>(removed)
          + static_cast<long double>(spectral.maximum_orbit_coherence)
              * removed * (removed - 1);
      const long double pulse =
          static_cast<long double>(spectral.pulse_operator_coefficient)
          * std::sqrt(std::max(0.0L, removed_factor));
      const long double bound = remaining + pulse;
      volume.removal_partition_bounds[static_cast<std::size_t>(removed)] =
          static_cast<double>(bound);
      finite = finite && std::isfinite(static_cast<double>(bound));
      if (bound > maximum_bound) {
        maximum_bound = bound;
        maximizing_removed = removed;
      }
    }

    volume.maximizing_removed_count = maximizing_removed;
    volume.ten_source_orbit_bound = static_cast<double>(maximum_bound);
    volume.ten_source_margin = K_GENESIS - volume.ten_source_orbit_bound;
    volume.all_partition_bounds_finite = finite;
    volume.ten_source_closed = maximum_bound
        < static_cast<long double>(K_GENESIS);
    volume.valid = spectral.valid && finite
        && maximizing_removed >= 0 && maximizing_removed <= 10
        && std::isfinite(volume.ten_source_margin);
    all_finite = all_finite && finite;
    all_closed = all_closed && volume.ten_source_closed;
    result.volumes.push_back(volume);
    ++result.spectral_volume_count;
  }

  result.all_partition_bounds_finite = all_finite;
  result.arbitrary_removal_n_le_ten_closed =
      result.parent_orbit_analysis_valid && all_finite && all_closed;
  result.ten_source_bound_inconclusive =
      result.parent_orbit_analysis_valid && all_finite && !all_closed;
  result.valid = result.parent_orbit_analysis_valid
      && result.spectral_volume_count == 4
      && result.volumes.size() == 4
      && all_finite
      && (result.arbitrary_removal_n_le_ten_closed
          || result.ten_source_bound_inconclusive);
  return result;
}

}  // namespace ftd::eft


