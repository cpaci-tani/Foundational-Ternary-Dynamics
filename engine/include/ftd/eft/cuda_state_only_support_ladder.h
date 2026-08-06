#pragma once
/**
 * @file cuda_state_only_support_ladder.h
 * @brief Device reduction for the FTD-0754 state-only support ladder.
 *
 * This research-only observer preserves the CPU construction of every compact
 * Gauss dressing and moves only the O(L^3) quadratic accounting to CUDA.  It
 * does not alter dynamics, the matter predicate, or production defaults.
 */

#include "ftd/eft/cuda_matched_field_pipeline.h"
#include "ftd/eft/state_only_matter_field_observer.h"

#include <cstddef>
#include <string>
#include <vector>

namespace ftd::eft {

struct CudaStateOnlySupportLadderTelemetry {
  bool valid = false;
  std::size_t host_to_device_bytes = 0;
  std::size_t device_to_host_bytes = 0;
  std::size_t complete_field_downloads = 0;
  double allocation_ms = 0.0;
  double upload_ms = 0.0;
  double kernel_ms = 0.0;
  double download_ms = 0.0;
  std::string error;
};

/**
 * CUDA-equivalent implementation of observe_state_only_support_ladder().
 *
 * Compact dressing construction remains on the CPU and is therefore exactly
 * the same selected Poisson problem as the reference observer.  The actual,
 * residual, interference, and nested-support quadratic reductions execute on
 * the GPU.  Only fixed-size block partials return to the host.
 */
StateOnlySupportLadderObservation observe_state_only_support_ladder_cuda(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& action_options,
    const std::vector<int>& support_half_widths,
    double poisson_tolerance = 1e-13,
    int poisson_max_iterations = 4096,
    double gate_tolerance = 1e-12,
    CudaStateOnlySupportLadderTelemetry* telemetry = nullptr,
    bool allow_fractional_center = false);

/** Resident-field counterpart; matter_only carries L and constituent data. */
StateOnlySupportLadderObservation
observe_state_only_support_ladder_cuda_resident(
    const ConnectedMooreBlockState& matter_only,
    const ConnectedMooreBlockOptions& action_options,
    const CudaMatchedFieldDeviceView& actual_electric,
    const std::vector<int>& support_half_widths,
    double poisson_tolerance = 1e-13,
    int poisson_max_iterations = 4096,
    double gate_tolerance = 1e-12,
    CudaStateOnlySupportLadderTelemetry* telemetry = nullptr,
    bool allow_fractional_center = false);

/** CUDA reduction counterpart of observe_state_only_matter_field(). */
StateOnlyMatterFieldObservation observe_state_only_matter_field_cuda(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& action_options,
    const StateOnlyMatterFieldObserverOptions& observer_options = {},
    CudaStateOnlySupportLadderTelemetry* telemetry = nullptr);

/** Resident-field counterpart; downloads only reduction scalars. */
StateOnlyMatterFieldObservation observe_state_only_matter_field_cuda_resident(
    const ConnectedMooreBlockState& matter_only,
    const ConnectedMooreBlockOptions& action_options,
    const CudaMatchedFieldDeviceView& actual_electric,
    const CudaMatchedFieldDeviceView& actual_magnetic,
    const StateOnlyMatterFieldObserverOptions& observer_options = {},
    CudaStateOnlySupportLadderTelemetry* telemetry = nullptr);

}  // namespace ftd::eft
