#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ftd {
class RenderBridge;
}

namespace ftd::native_desktop {

struct NativeParticle {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float size = 0.45f;
};

struct NativeFrame {
    int tick = 0;
    int lattice_size = 0;
    int flux_boundary = 2;
    std::uint32_t total_manifested = 0;
    std::string scenario;
    std::string backend;
    std::string status;
    std::vector<NativeParticle> particles;
    std::vector<NativeParticle> flux;
};

struct NativeEngineOptions {
    int lattice_size = 32;
    std::string scenario = "s0-seed-hydrogen";
    bool force_cpu = true;
    // Matches the web Scale-0 toolbar default (Dispersal).
    int flux_boundary = 2;
};

class NativeEngineSession {
public:
    explicit NativeEngineSession(NativeEngineOptions options);
    ~NativeEngineSession();

    NativeEngineSession(const NativeEngineSession&) = delete;
    NativeEngineSession& operator=(const NativeEngineSession&) = delete;

    void tick();
    NativeFrame capture();
    NativeFrame capture_particles() { return capture(); }

    void apply_options(NativeEngineOptions options);
    void load_scenario(std::string name);
    void set_lattice_size(int lattice_size);
    void set_flux_boundary(int flux_boundary);
    void reset_current();

    int lattice_size() const;
    int current_tick() const;
    int flux_boundary() const { return options_.flux_boundary; }
    const char* backend_name() const;
    const std::string& scenario() const { return options_.scenario; }
    const std::string& status() const { return status_; }
    const NativeEngineOptions& options() const { return options_; }

    // Thread affinity (single-writer-then-handoff, not enforced by this
    // class -- callers must honor it): try_enable_interop() is safe to call
    // from whichever thread currently exclusively owns `bridge_` -- the
    // main/GUI thread before the sim thread is constructed (the current
    // native_desktop main.cpp's startup call), OR the sim thread itself
    // afterward, once a reload's boot() has finished rebuilding `bridge_`
    // (main.cpp's do_reload branch re-imports the same still-open D3D12
    // buffer/fence NT handles there to re-establish interop after every
    // reload -- see Interop Task 12). It must never be called from both
    // "sides" concurrently. interop_enabled(), request_interop_gather(),
    // and poll_interop_particle_count() are safe to call only from the sim
    // thread once it exists -- same as tick(), capture(), and
    // apply_options(), which carry this same implicit contract because they
    // all reach into `bridge_`, and boot() (invoked by apply_options()) can
    // reset `bridge_` to null mid-reconstruction with zero locking. A call
    // racing a reload from any other thread is a null-pointer dereference
    // or a torn read, not a defined error.
    //
    // Attempts to initialize the D3D12/CUDA interop path: imports the given
    // shared buffer and fence handles into the GPU backend. No-op (returns
    // false) on the CPU backend or if either import fails -- callers must
    // keep using the plain capture()/render() path in that case, exactly as
    // before this method existed. Safe to call more than once against the
    // SAME session, including after a reload (apply_options()/
    // load_scenario()/set_lattice_size()/reset_current() all clear
    // interop_enabled_ via boot() -- see boot()'s doc comment): boot()
    // fully destroys the old bridge_/GpuEngine and constructs a fresh one
    // before this can run again, so there is no dangling-import or
    // re-entrancy-leak concern the way there would be re-importing into a
    // GpuEngine instance that was never destroyed. The NT handles
    // themselves may be reused across such calls -- neither
    // import_d3d12_particle_buffer() nor import_d3d12_fence() take
    // ownership of them (see gpu_engine.h) -- but the D3D12-side shared
    // fence's completed value is NOT reset by any of this (the underlying
    // ID3D12Fence object, unlike bridge_/GpuEngine, is never recreated
    // across a reload if the caller keeps reusing the same fence handle):
    // whatever fence_value a caller signals after a reload must keep
    // increasing from whatever value that same fence last reached before
    // the reload, or the D3D12-side Wait() the caller pairs it with can
    // return immediately without actually waiting for the new gather to
    // finish (cudaSignalExternalSemaphoresAsync itself will also simply
    // fail for a non-monotonic value -- see interop_signal_fence()'s doc
    // comment in gpu_engine.h).
    bool try_enable_interop(void* shared_buffer_handle, std::uint64_t buffer_bytes,
                            void* shared_fence_handle);
    bool interop_enabled() const { return interop_enabled_; }
    // Runs the interop gather (device-side only, no host particle vector) and
    // returns the particle count once ready, or -1 if not yet ready this call
    // (poll again). fence_value must be a strictly increasing counter the
    // caller also passes to D3D12Presenter::wait_shared_fence with the same
    // value.
    int poll_interop_particle_count();
    void request_interop_gather(std::uint64_t fence_value);

private:
    void boot();
    void apply_boundary();
    void fill_frame_meta(NativeFrame& frame) const;

    NativeEngineOptions options_;
    std::unique_ptr<RenderBridge> bridge_;
    std::string status_;
    bool interop_enabled_ = false;
};

}  // namespace ftd::native_desktop
