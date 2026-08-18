#pragma once

#include "native_desktop/engine_session.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdint>
#include <memory>

namespace ftd::native_desktop {

struct Camera {
    float target_x = 0.0f;
    float target_y = 0.0f;
    float target_z = 0.0f;
    float yaw = 0.6f;
    float pitch = 0.4f;
    float distance = 48.0f;
    float fov_y = 0.9f;
};

struct NativeViewOptions {
    bool particles = true;
    bool flux = true;
    bool lattice_box = true;
};

class D3D12Presenter {
public:
    D3D12Presenter();
    ~D3D12Presenter();

    D3D12Presenter(const D3D12Presenter&) = delete;
    D3D12Presenter& operator=(const D3D12Presenter&) = delete;

    void initialize(HWND hwnd, std::uint32_t width, std::uint32_t height);
    void resize(std::uint32_t width, std::uint32_t height);
    void render(const NativeFrame& frame, const Camera& camera,
                const NativeViewOptions& opts = {},
                std::uint32_t interop_particle_count = 0);
    void wait_idle();

    std::uint32_t width() const { return width_; }
    std::uint32_t height() const { return height_; }

    // Enumerates DXGI adapters and picks the first non-software one (skips
    // WARP). Static + no side effects on `this` so it's testable without a
    // window or a live device. Returns false if only a software adapter is
    // available (rare on real hardware, common in some CI/VM environments).
    static bool select_hardware_adapter(LUID* out_luid, bool* out_is_hardware);

    LUID adapter_luid() const { return adapter_luid_; }
    bool has_adapter_luid() const { return has_adapter_luid_; }

    // Creates a D3D12_HEAP_FLAG_SHARED committed buffer sized for
    // `max_particles` InteropParticleRecord entries and exports an NT handle
    // CUDA can import via cudaImportExternalMemory. Should be called after
    // initialize() -- calling it before initialize() (no live device) is a
    // graceful nullptr return, not a crash. The returned handle is owned by
    // the caller -- close it with CloseHandle once CUDA has imported it
    // (cudaImportExternalMemory takes ownership semantics that make the
    // D3D12-side handle disposable immediately after the import call
    // returns, per the CUDA Runtime API docs for
    // cudaExternalMemoryHandleDesc). Returns nullptr on failure.
    //
    // Safe to call more than once (e.g. to resize the buffer): any
    // previously-created buffer is waited-on (so no in-flight D3D12/CUDA
    // work is still touching it) and released before the new one is
    // created. shared_particle_buffer_bytes() reflects the live buffer's
    // size and resets to 0 on any failure path, so it never reports a
    // stale size for a buffer that no longer exists.
    HANDLE create_shared_particle_buffer(std::uint32_t max_particles);
    std::uint64_t shared_particle_buffer_bytes() const {
        return shared_particle_buffer_bytes_;
    }

    // Creates the SRV describing the current shared particle buffer as a
    // StructuredBuffer<InteropParticleRecord> and writes it into the
    // shader-visible SRV heap the interop PSO reads from (register t0). Must
    // be called after create_shared_particle_buffer() has succeeded (a no-op
    // if the shared buffer does not exist yet) and again any time that
    // buffer is recreated at a different size, since the SRV's element count
    // is derived from shared_particle_buffer_bytes().
    void bind_interop_particle_srv();

    // Creates a D3D12_FENCE_FLAG_SHARED fence starting at value 0 and
    // exports its NT handle for CUDA to import via cudaImportExternalSemaphore.
    // Must be called after initialize(), independent of create_shared_particle_buffer.
    //
    // Safe to call more than once (e.g. to re-import after a presenter
    // reset): any previously-created shared fence is waited-on (so no
    // in-flight queue->Wait() against it is still outstanding) before being
    // released and replaced, mirroring create_shared_particle_buffer()'s
    // identical pattern for the shared particle buffer.
    HANDLE create_shared_fence();
    // Makes the render queue wait (GPU-side, not CPU-side --
    // ID3D12CommandQueue::Wait is a queue-timeline operation) until the
    // shared fence reaches `value` before executing any further work: Wait
    // blocks the queue's timeline from that point forward, not just a single
    // next command list. Call this before the draw call that reads the
    // interop buffer.
    void wait_shared_fence(std::uint64_t value);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;
    LUID adapter_luid_{};
    bool has_adapter_luid_ = false;
    std::uint64_t shared_particle_buffer_bytes_ = 0;
};

}  // namespace ftd::native_desktop
