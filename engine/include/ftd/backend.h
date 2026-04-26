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

namespace ftd {

class RenderBridge;

namespace gpu { class GpuEngine; }

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
    Kind kind() const override { return Kind::Cpu; }

private:
    RenderBridge& bridge_;
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
    Kind kind() const override { return Kind::Gpu; }

private:
    RenderBridge&    bridge_;
    gpu::GpuEngine*  engine_;  // Non-owning; RenderBridge owns the unique_ptr.
};
#endif

}  // namespace ftd
