#pragma once
/**
 * @file cuda_momentum_transport_current.h
 * @brief Fused per-tick masked momentum-ledger reduction on CUDA.
 *
 * Sec 4 item 2 of PREREG_TOTAL_MOMENTUM_STRESS_LEDGER_v1: one pass over the
 * lattice per tick emits, for every component i, every radius R, and both
 * localizations, the per-tick contributions to Phi^(.), the sweep term, the
 * source term, and the regional content Pi_i(R,t).  Output per tick is a small
 * vector of doubles (block-reduced).  Reduction remains scalar-only -- no
 * complete field is ever downloaded, per the FTD-0768 CUDA-telemetry
 * discipline inherited unchanged.
 *
 * The kernel evaluates the site-mask form of Sec 2.3, in which the S^(i)
 * channel drops out identically.  That is the production form; the genuinely
 * per-component mask that exercises S^(i) is run by the Sec 6.4 L=11 exactness
 * pre-check on the host path in momentum_transport_current.h.
 *
 * Observer-only.  No production path, no RenderBridge, no existing test.
 */

#include "ftd/eft/cuda_matched_field_pipeline.h"
#include "ftd/eft/momentum_transport_current.h"

#include <cstddef>
#include <memory>
#include <string>

namespace ftd::eft {

/// Registered radius scan of Sec 5: {8,16,24,32,48}.
constexpr int kCudaMomentumMaximumRadii = 5;
/// Radius slots plus one whole-domain slot for the Sec 3 G_U cross-check.
constexpr int kCudaMomentumSlots = kCudaMomentumMaximumRadii + 1;
constexpr int kCudaMomentumWholeDomainSlot = kCudaMomentumMaximumRadii;

struct CudaMomentumTransportTelemetry {
  bool valid = false;
  std::size_t host_to_device_bytes = 0;
  std::size_t device_to_host_bytes = 0;
  std::size_t complete_field_downloads = 0;
  std::size_t kernel_launches = 0;
  double allocation_ms = 0.0;
  double kernel_ms = 0.0;
  std::string error;
};

struct CudaMomentumLedgerOptions {
  double lambda = 0.0;
  double interaction_scale = 1.0;
  /// c(t): integer mask centre before the step.
  int previous_center[3]{};
  /// c(t+1): integer mask centre after the step.
  int current_center[3]{};
  /// Chebyshev radii; a negative entry marks the slot unused.
  int radius[kCudaMomentumMaximumRadii]{-1, -1, -1, -1, -1};
};

struct CudaMomentumLedgerTick {
  bool valid = false;
  /// terms[localization][component][slot]; slot kCudaMomentumWholeDomainSlot
  /// is the whole-domain (chi == 1) reference.
  MomentumLedgerTickTerms terms[2][3][kCudaMomentumSlots]{};

  const MomentumLedgerTickTerms& at(MomentumLocalization localization,
                                    int component, int slot) const {
    return terms[localization == MomentumLocalization::ECarries ? 0 : 1]
                [component][slot];
  }
  MomentumLedgerTickTerms& at(MomentumLocalization localization,
                              int component, int slot) {
    return terms[localization == MomentumLocalization::ECarries ? 0 : 1]
                [component][slot];
  }
};

/**
 * Device-resident chord tables plus the fused per-tick reduction.  Tables are
 * built once (impulse-response extraction on a probe torus) and uploaded once;
 * only the small per-tick partial vector crosses the bus afterwards.
 */
class CudaMomentumTransportLedger {
 public:
  explicit CudaMomentumTransportLedger(int L);
  ~CudaMomentumTransportLedger();

  CudaMomentumTransportLedger(const CudaMomentumTransportLedger&) = delete;
  CudaMomentumTransportLedger& operator=(const CudaMomentumTransportLedger&)
      = delete;

  bool valid() const;
  const char* error() const;
  int size() const;

  /// N = D_i, used by Phi^(u) (L1) and Phi^(w) (L2).
  const MomentumTransportCurrentTable& plain_table(int component) const;
  /// N = D_i C C^T (L1, on E) or N = D_i C^T C (L2, on B').
  const MomentumTransportCurrentTable& binding_table(
      MomentumLocalization localization, int component) const;

  CudaMomentumLedgerTick observe(
      const CudaMatchedFieldResidentViews& views,
      const CudaMomentumLedgerOptions& options,
      CudaMomentumTransportTelemetry* telemetry = nullptr);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

bool cuda_momentum_transport_ledger_available();

}  // namespace ftd::eft
