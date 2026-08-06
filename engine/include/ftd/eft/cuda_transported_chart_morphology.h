#pragma once
/** CUDA reduction counterpart of the FTD-0764 morphology observer. */

#include "ftd/eft/transported_chart_morphology.h"

#include <cstddef>
#include <string>

namespace ftd::eft {

struct CudaTransportedChartMorphologyTelemetry {
  bool valid = false;
  std::size_t host_to_device_bytes = 0;
  std::size_t device_to_host_bytes = 0;
  std::size_t complete_field_downloads = 0;
  double allocation_ms = 0.0;
  double upload_ms = 0.0;
  double kernel_ms = 0.0;
  std::string error;
};

TransportedChartMorphologyObservation
observe_transported_chart_morphology_cuda(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& action_options,
    const TransportedChartMorphologyOptions& observer_options,
    CudaTransportedChartMorphologyTelemetry* telemetry = nullptr);

}  // namespace ftd::eft
