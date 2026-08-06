#pragma once
/**
 * @file cuda_matched_field_pipeline.h
 * @brief CUDA accelerator for the selected matched face/edge field step.
 *
 * The class owns only an instrumentation mirror of the matched field state.
 * It does not alter RenderBridge or production defaults.  Matter endpoints
 * and the implicit common-action root remain host-side; the O(L^3) curl,
 * sparse-current application, and regional-energy reductions execute on the
 * device.
 */

#include "ftd/eft/batched_regional_energy_profile.h"
#include "ftd/eft/connected_moore_block_action.h"
#include "ftd/eft/quadratic_coat_face_current.h"

#include <memory>
#include <vector>

namespace ftd::eft {

struct CudaMatchedFieldTimings {
  double upload_ms = 0.0;
  double prepare_ms = 0.0;
  double current_ms = 0.0;
  double observe_ms = 0.0;
  double download_ms = 0.0;
};

/**
 * Borrowed read-only view of one device-resident matched vector field.
 *
 * The pointers remain valid only while the originating
 * CudaMatchedFieldPipeline is alive and has not been moved.  This is a
 * research/instrumentation interface for CUDA-local observers and implicit
 * root gathers; it is not an ontological state record or a RenderBridge API.
 */
struct CudaMatchedFieldDeviceView {
  int L = 0;
  const double* x = nullptr;
  const double* y = nullptr;
  const double* z = nullptr;

  bool valid() const {
    return L > 0 && x != nullptr && y != nullptr && z != nullptr;
  }
};

struct CudaMatchedFieldResidentViews {
  CudaMatchedFieldDeviceView electric_before{};
  CudaMatchedFieldDeviceView magnetic_before{};
  CudaMatchedFieldDeviceView magnetic_prepared{};
  CudaMatchedFieldDeviceView electric_pre_current{};
  CudaMatchedFieldDeviceView electric_after{};
  bool prepared = false;
  bool current_applied = false;
};

class CudaMatchedFieldPipeline {
 public:
  explicit CudaMatchedFieldPipeline(int L);
  ~CudaMatchedFieldPipeline();

  CudaMatchedFieldPipeline(const CudaMatchedFieldPipeline&) = delete;
  CudaMatchedFieldPipeline& operator=(const CudaMatchedFieldPipeline&) = delete;
  CudaMatchedFieldPipeline(CudaMatchedFieldPipeline&&) noexcept;
  CudaMatchedFieldPipeline& operator=(CudaMatchedFieldPipeline&&) noexcept;

  bool valid() const;
  int size() const;
  const char* error() const;
  const CudaMatchedFieldTimings& timings() const;

  /// Replace the device-resident state with one host matched field state.
  bool upload(const MatchedFaceFlux& electric,
              const MatchedEdgeField& magnetic_half);

  /// Execute B1=B0-lambda*C^T E0 and E*=E0+lambda*C B1.
  bool prepare_forward(double lambda);

  /// Download B1 and E* for the unchanged host-side local implicit root.
  bool download_prepared(MatchedEdgeField& magnetic_after,
                         MatchedFaceFlux& electric_pre_current);

  /// Apply E1=E*-polarity_scale*sum(current) on the resident device field.
  bool apply_sparse_current(
      const std::vector<QuadraticCoatFaceCurrent>& segments,
      double polarity_scale);

  /// Research-only deterministic deposition.  Aggregate the complete ungated
  /// current to unique periodic oriented faces on the host, then perform one
  /// non-atomic device update per face.  This leaves apply_sparse_current()
  /// and every production caller unchanged.
  bool apply_canonical_sparse_current(
      const std::vector<QuadraticCoatFaceCurrent>& segments,
      double polarity_scale);

  /// Research-only CPU-parity deposition.  Group raw current contributions by
  /// periodic oriented face while preserving their original per-face order,
  /// then let one device thread reproduce the host's sequential additions.
  /// No production caller or legacy deposition path is changed.
  bool apply_ordered_sparse_current(
      const std::vector<QuadraticCoatFaceCurrent>& segments,
      double polarity_scale);

  /// Observe the before/pre-current/after fields without downloading volumes.
  BatchedRegionalEnergyProfile observe(
      double lambda,
      const Vec3& integer_center,
      const std::vector<int>& chebyshev_radii,
      double tolerance = 1e-10);

  /// Research-only deterministic selected-radius observer.  It replaces the
  /// histogram atomics used by observe() with a fixed block-reduction tree.
  /// At most six preregistered radii are accepted.
  BatchedRegionalEnergyProfile observe_deterministic(
      double lambda,
      const Vec3& integer_center,
      const std::vector<int>& chebyshev_radii,
      double tolerance = 1e-10);

  /// Complete the Gauss, energy, local-momentum, and spline-Poynting volume
  /// diagnostics for the resident before/after transaction.  observe() must
  /// have succeeded first so the exact modified energies are reused.
  ConnectedMooreBlockVolumeDiagnostics diagnose_common_action(
      const std::vector<QuadraticCoatFaceCurrent>& segments,
      double polarity_scale,
      double interaction_scale,
      double wave_speed,
      double dt,
      double tolerance = 1e-10);

  /// Download the accepted E1,B1 state for the host matter/root representation.
  bool download_after(MatchedFaceFlux& electric_after,
                      MatchedEdgeField& magnetic_after);

  /// Promote E1,B1 to E0,B0 without a host round trip.
  bool advance();

  /// Borrow read-only device views without downloading any complete field.
  CudaMatchedFieldResidentViews resident_views() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

bool cuda_matched_field_pipeline_available();

}  // namespace ftd::eft
