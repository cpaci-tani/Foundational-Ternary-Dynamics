#pragma once
/** CUDA reductions for the FTD-0768 paired response and regional ledger. */

#include "ftd/eft/cuda_matched_field_pipeline.h"
#include "ftd/eft/paired_field_response.h"

#include <cstddef>
#include <string>

namespace ftd::eft {

struct CudaPairedFieldResponseTelemetry {
  bool valid = false;
  std::size_t host_to_device_bytes = 0;
  std::size_t device_to_host_bytes = 0;
  std::size_t complete_field_downloads = 0;
  double allocation_ms = 0.0;
  double upload_ms = 0.0;
  double kernel_ms = 0.0;
  std::string error;
};

PairedFieldResponseObservation observe_paired_field_response_cuda(
    const ConnectedMooreBlockState& moving,
    const ConnectedMooreBlockState& rest,
    const ConnectedMooreBlockOptions& action_options,
    const PairedFieldResponseOptions& observer_options,
    CudaPairedFieldResponseTelemetry* telemetry = nullptr);

RegionalModifiedEnergyTransportObservation
observe_regional_modified_energy_transport_cuda(
    const CudaMatchedFieldResidentViews& views,
    double lambda,
    const FieldResponseRegionSpec& region,
    double tolerance = 1e-12,
    CudaPairedFieldResponseTelemetry* telemetry = nullptr);

}  // namespace ftd::eft

