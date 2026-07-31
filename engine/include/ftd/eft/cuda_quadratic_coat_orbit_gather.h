#pragma once
/**
 * @file cuda_quadratic_coat_orbit_gather.h
 * @brief Device-resident quadratic-coat orbit gather (FTD-0759).
 *
 * This research-only observer evaluates the unchanged FTD-0550 sparse
 * midpoint gather against fields already resident in a
 * CudaMatchedFieldPipeline.  It returns only per-constituent records and never
 * downloads a complete field.
 */

#include "ftd/eft/cuda_matched_field_pipeline.h"
#include "ftd/eft/quadratic_coat_orbit_gather.h"

#include <cstddef>
#include <string>
#include <vector>

namespace ftd::eft {

struct CudaQuadraticCoatOrbitGatherTelemetry {
  bool valid = false;
  std::size_t host_to_device_bytes = 0;
  std::size_t device_to_host_bytes = 0;
  std::size_t complete_field_downloads = 0;
  double upload_ms = 0.0;
  double kernel_ms = 0.0;
  double download_ms = 0.0;
  std::string error;
};

std::vector<QuadraticCoatOrbitGatherResult>
evaluate_quadratic_coat_orbit_gather_sparse_midpoint_batch_cuda_resident(
    const std::vector<QuadraticCoatFaceCurrent>& segments,
    const CudaMatchedFieldDeviceView& fixed_electric,
    const CudaMatchedFieldDeviceView& electric_pre_current,
    double current_scale,
    const CudaMatchedFieldDeviceView& magnetic,
    const std::vector<Vec3>& discrete_gradient_velocities,
    double temporal_scale,
    double beta = 1.0,
    double polarity_scale = 1.0,
    CudaQuadraticCoatOrbitGatherTelemetry* telemetry = nullptr);

}  // namespace ftd::eft
