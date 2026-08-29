#pragma once
/**
 * @file backend.h
 * @brief Backend abstraction — collapses #ifdef FTD_ENABLE_CUDA proliferation.
 *
 * ARCH-2 (CHECKLIST_ENGINE.md): RenderBridge currently has 14 #ifdef blocks
 * in render_bridge.cpp + 6 in render_bridge.h that branch between CPU and GPU
 * paths. This file introduces an abstract Backend interface so those branches
 * can be replaced with virtual dispatch.
 *
 * Migration is INCREMENTAL — the interface is added in parallel to the
 * existing if-use_gpu_ pattern. Each phase migrates a set of operations from
 * the ifdef pattern to the interface, with regression at each step.
 *
 * Design notes:
 * - The interface is intentionally minimal at first (set_dt, sync, tick).
 *   Methods will be added as migration phases land.
 * - GPU-default policy: when CUDA is available the engine constructs a
 *   GpuBackend; CpuBackend is the fallback. force_cpu() swaps the backend
 *   to CpuBackend without rebuilding.
 * - Virtual dispatch overhead is one function-pointer indirection per phase
 *   per tick (~6 calls/tick at L=64) — negligible compared to a 3 MB PCIe
 *   transfer or a 262144-voxel kernel launch.
 */

#include <memory>
#include <cstdint>
#include <vector>
#include "ftd/dynamical_state_digest.h"
#include "ftd/telemetry_snapshot.h"
#include "ftd/visual_snapshot.h"

namespace ftd {

class RenderBridge;
enum class VisualFieldKind : std::uint32_t;
struct VisualFieldSample;
struct Diagnostics;
struct EnergyAudit;
struct GravityMetricAgg;
struct LagrangianDiag;
struct VoxelInspection;
struct ForceDiag;

namespace gpu { class GpuEngine; }
namespace eft { struct DualCellContinuity; }

/// Abstract execution backend for RenderBridge. Implementations:
///   - CpuBackend: invokes the CPU phase methods on the bridge directly.
///   - GpuBackend: forwards to the GpuEngine and manages host/device sync.
class Backend {
public:
    virtual ~Backend() = default;

    /// One simulation step. Implementations call the bridge's phase methods
    /// (CPU) or the GpuEngine's tick (GPU).
    virtual void tick() = 0;

    /// Time-step setter. CPU is a no-op (RenderBridge owns dt_); GPU pushes
    /// the value to the GpuEngine.
    virtual void set_dt(double dt) = 0;

    /// Ensure the host-side voxel array is up-to-date. CPU is a no-op; GPU
    /// downloads device buffers when dirty.
    virtual void sync_to_host() = 0;

    /// Mark host-side state as dirty so it is uploaded before the next tick.
    /// CPU is a no-op; GPU sets a flag the next tick will pick up.
    virtual void mark_host_dirty() = 0;

    /// Unconditional upload of host-side voxels to device. CPU no-op; GPU
    /// pushes the full voxel array up and clears the dirty flag.
    virtual void push_to_device() = 0;

    /// Upload host mutations only if `mark_host_dirty()` was called since
    /// the last upload. CPU no-op; GPU flushes and clears the flag.
    virtual void flush_host_mutations() = 0;

    /// Mirror the GPU's phi_latency buffer into RenderBridge::phi_latency_
    /// so external `phi_latency()` callers get a stable host reference.
    /// CPU no-op (the SOR solver already writes phi_latency_ directly).
    virtual void mirror_phi_latency() = 0;

    /// Flag that the device has newer state than the host (the inverse of
    /// `mark_host_dirty`). Used after a GPU-side write — the next access
    /// through `voxels()` will trigger `sync_to_host`. CPU no-op.
    virtual void mark_gpu_dirty() = 0;

    /// Selective visualization readback. GPU implementations copy only the
    /// compact fields required by the renderer and leave the canonical host
    /// mirror dirty; CPU returns false so RenderBridge uses its resident AoS.
    virtual bool copy_visual_states(std::vector<std::int8_t>& /*out*/) { return false; }
    virtual bool copy_visual_flux_magnitude(std::vector<float>& /*out*/) { return false; }
    virtual bool copy_visual_flux_magnitude_plane(
        int /*axis*/, int /*index*/, std::vector<float>& /*out*/) { return false; }
    virtual bool copy_visual_field_sample(VisualFieldKind /*kind*/, int /*stride*/,
                                          VisualFieldSample& /*out*/) { return false; }
    // Five floats per selected manifested site: remainder xyz, spin, color.
    // The renderer combines the remainder with the cell centre without ever
    // materializing the full GPU voxel mirror.
    virtual bool copy_visual_particle_attributes(
        const std::vector<int>& /*indices*/, std::vector<float>& /*out*/) {
        return false;
    }

    /// Fixed-size device reductions for high-frequency native diagnostics.
    /// False means the caller must use the canonical CPU snapshot path.
    virtual bool copy_compact_diagnostics(Diagnostics& /*out*/) { return false; }
    virtual bool copy_compact_energy_audit(EnergyAudit& /*out*/) { return false; }
    virtual bool copy_compact_gravity_metric(GravityMetricAgg& /*out*/) { return false; }
    virtual bool copy_compact_lagrangian(LagrangianDiag& /*out*/) { return false; }
    virtual bool copy_compact_voxel(int /*index*/, VoxelInspection& /*out*/) {
        return false;
    }
    virtual bool copy_compact_force(int /*index*/, ForceDiag& /*out*/) {
        return false;
    }

    /// Canonical schema-versioned Scale-0 state digest. CPU computes from its
    /// resident named fields; CUDA performs a device reduction and copies one
    /// fixed accumulator. No implementation may satisfy this via raw Voxel
    /// bytes or a hidden full GPU mirror.
    virtual bool capture_dynamical_state_digest(
        DynamicalStateDigest& /*out*/) { return false; }

    /// Stage one versioned telemetry observation. GPU implementations return
    /// immediately after enqueuing their reduction/D2H fence; CPU captures a
    /// coherent fallback snapshot and makes it immediately pollable. A false
    /// return means a prior snapshot is still pending or the backend cannot
    /// serve the request.
    virtual bool begin_telemetry_snapshot(
        const TelemetrySnapshotRequest& /*request*/) { return false; }
    virtual bool telemetry_snapshot_ready() const { return false; }
    virtual bool poll_telemetry_snapshot(TelemetrySnapshot& /*out*/) {
        return false;
    }

    /// Stage one bounded visual frame.  This follows the telemetry lifecycle
    /// but owns separate staging/fence resources: a slow visual consumer must
    /// never overwrite or wait on the scalar telemetry publisher.  GPU
    /// implementations enqueue their device gather + pinned D2H copy and
    /// return immediately; the CPU compatibility backend is immediately
    /// pollable.  Only one visual capture may be pending per backend.
    virtual bool begin_visual_snapshot(
        const VisualSnapshotRequest& /*request*/) { return false; }
    virtual bool visual_snapshot_ready() const { return false; }
    virtual bool poll_visual_snapshot(VisualSnapshot& /*out*/) {
        return false;
    }
    /// Destructive source replacement must wait until this returns true.  A
    /// CPU snapshot is immediately safe; CUDA returns false only while its
    /// D2H event is genuinely unfinished.  This is distinct from a capture
    /// result being unpolled: a completed event is safe to discard.
    virtual bool visual_snapshot_safe_to_replace() const { return true; }
    virtual bool visual_snapshot_in_flight() const { return false; }

    /// Seed the device-side RNG (cuRAND). Default no-op — the host RNG is
    /// owned by RenderBridge::rng_state_; GpuBackend forwards to GpuEngine.
    /// (Revision 3.1: retired the seed_rng() ifdef in render_bridge.cpp.)
    virtual void set_rng_seed(unsigned int /*seed*/) {}

    /// Device-side dual-cell continuity measurement. Returns false when the
    /// backend has no device implementation — the caller then uses the CPU
    /// default. Forward-declared parameter type keeps backend.h include-light
    /// (the GPU override lives in backend.cpp).
    /// (Revision 3.1: retired the continuity_step() ifdef in render_bridge.cpp;
    /// gpu_engine_ptr() already gated on the ACTIVE backend, so dispatching
    /// through the virtual is semantics-preserving under force_cpu().)
    virtual bool continuity_step(eft::DualCellContinuity& /*out*/) { return false; }

    /// Identification — useful for tests that want to assert which backend
    /// is actually executing (not just which was requested).
    enum class Kind { Cpu, Gpu };
    virtual Kind kind() const = 0;
};

/// CPU backend factory. Always available.
class CpuBackend : public Backend {
public:
    explicit CpuBackend(RenderBridge& bridge);

    void tick() override;
    void set_dt(double /*dt*/) override {}      // RenderBridge::dt_ is the source of truth
    void sync_to_host() override {}             // CPU state always lives in voxels_
    void mark_host_dirty() override {}          // No device to invalidate
    void push_to_device() override {}           // Same — voxels_ is always authoritative
    void flush_host_mutations() override {}     // Same
    void mirror_phi_latency() override {}       // SOR writes phi_latency_ directly
    void mark_gpu_dirty() override {}            // No device to mark
    bool begin_telemetry_snapshot(
        const TelemetrySnapshotRequest& request) override;
    bool telemetry_snapshot_ready() const override;
    bool poll_telemetry_snapshot(TelemetrySnapshot& out) override;
    bool begin_visual_snapshot(
        const VisualSnapshotRequest& request) override;
    bool visual_snapshot_ready() const override;
    bool poll_visual_snapshot(VisualSnapshot& out) override;
    bool capture_dynamical_state_digest(
        DynamicalStateDigest& out) override;
    bool visual_snapshot_safe_to_replace() const override { return true; }
    bool visual_snapshot_in_flight() const override { return false; }
    Kind kind() const override { return Kind::Cpu; }

private:
    RenderBridge& bridge_;
    TelemetrySnapshot telemetry_snapshot_{};
    bool telemetry_snapshot_pending_ = false;
    VisualSnapshot visual_snapshot_{};
    bool visual_snapshot_pending_ = false;
};

#ifdef FTD_ENABLE_CUDA
/// GPU backend factory. Constructed only when CUDA is enabled at build time.
class GpuBackend : public Backend {
public:
    GpuBackend(RenderBridge& bridge, gpu::GpuEngine* engine);

    void tick() override;
    void set_dt(double dt) override;
    void sync_to_host() override;
    void mark_host_dirty() override;
    void push_to_device() override;
    void flush_host_mutations() override;
    void mirror_phi_latency() override;
    void mark_gpu_dirty() override;
    bool copy_visual_states(std::vector<std::int8_t>& out) override;
    bool copy_visual_flux_magnitude(std::vector<float>& out) override;
    bool copy_visual_flux_magnitude_plane(
        int axis, int index, std::vector<float>& out) override;
    bool copy_visual_field_sample(VisualFieldKind kind, int stride,
                                  VisualFieldSample& out) override;
    bool copy_visual_particle_attributes(
        const std::vector<int>& indices, std::vector<float>& out) override;
    bool copy_compact_diagnostics(Diagnostics& out) override;
    bool copy_compact_energy_audit(EnergyAudit& out) override;
    bool copy_compact_gravity_metric(GravityMetricAgg& out) override;
    bool copy_compact_lagrangian(LagrangianDiag& out) override;
    bool copy_compact_voxel(int index, VoxelInspection& out) override;
    bool copy_compact_force(int index, ForceDiag& out) override;
    bool capture_dynamical_state_digest(
        DynamicalStateDigest& out) override;
    bool begin_telemetry_snapshot(
        const TelemetrySnapshotRequest& request) override;
    bool telemetry_snapshot_ready() const override;
    bool poll_telemetry_snapshot(TelemetrySnapshot& out) override;
    bool begin_visual_snapshot(
        const VisualSnapshotRequest& request) override;
    bool visual_snapshot_ready() const override;
    bool poll_visual_snapshot(VisualSnapshot& out) override;
    bool visual_snapshot_safe_to_replace() const override;
    bool visual_snapshot_in_flight() const override;
    void set_rng_seed(unsigned int seed) override;
    bool continuity_step(eft::DualCellContinuity& out) override;
    Kind kind() const override { return Kind::Gpu; }

private:
    RenderBridge&    bridge_;
    gpu::GpuEngine*  engine_;  // Non-owning; RenderBridge owns the unique_ptr.
    // EnergyAudit::self_field_injection is maintained by RenderBridge rather
    // than the device reduction. Capture it at snapshot submission so a poll
    // after a later tick cannot splice a newer ledger value into old fields.
    double telemetry_snapshot_self_field_injection_ = 0.0;
    // Device identity counters change only at a GPU tick/injection or a host
    // upload.  Once reconciled, repeated clean voxels() reads must not issue
    // two synchronous scalar D2H copies per accessor call.
    bool identity_counters_dirty_ = false;
};
#endif

/// Construct the default backend for a fresh RenderBridge: GpuBackend (and
/// the owned GpuEngine) when CUDA is compiled in, CpuBackend otherwise.
/// Revision 3.1: this factory owns the LAST backend-selection ifdef — the
/// policy lives here and in backend.cpp, nowhere else.
std::unique_ptr<Backend> make_default_backend(RenderBridge& bridge, int lattice_size);

}  // namespace ftd
