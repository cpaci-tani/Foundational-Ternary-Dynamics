#include "native_desktop/engine_session.h"
#include "native_desktop/command_applier.h"

#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"
#include "ftd/term_toggles.h"
#include "ftd/visual_field_sample.h"
#include "ftd/visual_snapshot.h"
#ifdef FTD_ENABLE_CUDA
#include "ftd/gpu_engine.h"
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <thread>
#include <utility>

namespace ftd::native_desktop {
namespace {

void request_cpu_backend() {
#ifdef _WIN32
    _putenv_s("FTD_FORCE_CPU", "1");
#else
    setenv("FTD_FORCE_CPU", "1", 1);
#endif
}

void seed_visible_pair(RenderBridge& rb) {
    const int c = rb.lattice().size() / 2;
    rb.inject_particle(c - 2, c, c, 1, Vec3{0.25, 0.0, 0.0});
    rb.inject_particle(c + 2, c, c, -1, Vec3{-0.25, 0.0, 0.0});
}

bool wait_visual_snapshot(RenderBridge& rb, VisualSnapshot& out) {
    for (int i = 0; i < 5000; ++i) {
        if (rb.poll_visual_snapshot(out)) return true;
        std::this_thread::sleep_for(std::chrono::microseconds(200));
    }
    return false;
}

void append_flux(RenderBridge& rb, NativeFrame& frame) {
    const int L = rb.lattice().size();
    const int stride = std::max(1, (L + 31) / 32);
    VisualFieldSample sample;
    rb.copy_visual_field_sample(VisualFieldKind::FluxVector, stride, sample);
    if (sample.components != 3u || sample.count() == 0) return;

    float max_mag = 1.0e-6f;
    std::vector<float> mag(sample.count(), 0.0f);
    for (std::size_t i = 0; i < sample.count(); ++i) {
        const float jx = sample.data[i * 3u];
        const float jy = sample.data[i * 3u + 1u];
        const float jz = sample.data[i * 3u + 2u];
        mag[i] = std::sqrt(jx * jx + jy * jy + jz * jz);
        max_mag = std::max(max_mag, mag[i]);
    }

    frame.flux.reserve(sample.count());
    for (std::size_t i = 0; i < sample.count(); ++i) {
        const float t = mag[i] / max_mag;
        if (t < 0.04f) continue;
        NativeParticle p;
        p.x = sample.positions[i * 3u];
        p.y = sample.positions[i * 3u + 1u];
        p.z = sample.positions[i * 3u + 2u];
        p.r = 0.12f + 0.25f * t;
        p.g = 0.40f + 0.45f * t;
        p.b = 0.85f + 0.15f * t;
        p.size = 0.18f + 0.55f * t;
        frame.flux.push_back(p);
    }
}

}  // namespace

NativeEngineSession::NativeEngineSession(NativeEngineOptions options)
    : options_(std::move(options)) {
    staged_lattice_size_ = options_.lattice_size;
    ui_.publisher = &publisher_;
    ui_.journal = &journal_;
    ui_.scheduler = &scheduler_;
    boot();
}

void NativeEngineSession::boot() {
    if (options_.lattice_size < 4 || options_.lattice_size > 256) {
        throw std::invalid_argument("lattice_size must be in [4, 256]");
    }

    if (options_.force_cpu) {
        request_cpu_backend();
    }

    // A (re-)boot tears down bridge_ and, with it, whatever GpuEngine
    // try_enable_interop() last imported the shared D3D12 buffer/fence
    // into. try_enable_interop() is invoked at startup and, since Interop
    // Task 12, again on every reload (main.cpp's do_reload branch) -- but
    // each such call always lands strictly AFTER the fresh bridge_/
    // GpuEngine constructed below already exists, never before any import
    // call has been made against that fresh instance -- so interop_enabled_
    // must be cleared here, or callers keep observing interop_enabled() ==
    // true (and request_interop_gather()/poll_interop_particle_count() keep
    // being invoked) against an engine that was never re-imported and can
    // never succeed.
    interop_enabled_ = false;
    staged_lattice_size_ = options_.lattice_size;

    bridge_.reset();
    bridge_ = std::make_unique<RenderBridge>(options_.lattice_size);
    if (options_.force_cpu) {
        bridge_->force_cpu();
    } else {
        bridge_->set_interactive_gpu_mode(true);
    }
    apply_boundary();

    if (!dispatch_scenario(*bridge_, options_.scenario)) {
        options_.scenario = "demo-pair";
        seed_visible_pair(*bridge_);
        status_ = "unknown scenario; seeded a +/- pair";
    } else {
        status_ = options_.scenario;
    }
    apply_boundary();
}

void NativeEngineSession::apply_boundary() {
    int mode = options_.flux_boundary;
    if (mode < 0 || mode > 2) mode = 2;
    options_.flux_boundary = mode;
    bridge_->toggles.flux_boundary = static_cast<ftd::FluxBoundaryMode>(mode);
}

void NativeEngineSession::apply_options(NativeEngineOptions options) {
    options_ = std::move(options);
    boot();
}

void NativeEngineSession::load_scenario(std::string name) {
    options_.scenario = std::move(name);
    boot();
}

void NativeEngineSession::set_lattice_size(int lattice_size) {
    options_.lattice_size = lattice_size;
    boot();
}

void NativeEngineSession::set_flux_boundary(int flux_boundary) {
    options_.flux_boundary = flux_boundary;
    apply_boundary();
}

void NativeEngineSession::reset_current() { boot(); }

void NativeEngineSession::fill_frame_meta(NativeFrame& frame) const {
    frame.scenario = options_.scenario;
    frame.backend = backend_name();
    frame.status = status_;
    frame.flux_boundary = options_.flux_boundary;
}

NativeEngineSession::~NativeEngineSession() = default;

void NativeEngineSession::tick() {
    bridge_->bind_sim_thread();
    bridge_->tick();
}

TickResult NativeEngineSession::tick_once() {
    TickResult result;
    try {
        tick();
        ui_.did_tick = true;
        ui_.last_tick = result;
    } catch (const std::exception& ex) {
        result.ok = false;
        result.message = ex.what();
        ui_.last_tick = result;
        ui_.did_tick = false;
    }
    return result;
}

void NativeEngineSession::consume_pending_step() {
    if (ui_.loop.pending_steps > 0) --ui_.loop.pending_steps;
}

TickResult NativeEngineSession::process_ui_boundary(CommandQueue& queue) {
    ui_.publisher = &publisher_;
    ui_.journal = &journal_;
    ui_.scheduler = &scheduler_;
    ftd::native_desktop::process_ui_boundary(*bridge_, this, queue, ui_);
    ui_.did_tick = false;
    return ui_.last_tick;
}

NativeFrame NativeEngineSession::capture() {
    VisualSnapshotRequest request;
    request.kind = VisualCaptureKind::Particles;
    request.max_particles = kMaxVisualParticleCapture;
    if (!bridge_->begin_visual_snapshot(request)) {
        throw std::runtime_error("visual snapshot could not begin");
    }

    VisualSnapshot snapshot;
    if (!wait_visual_snapshot(*bridge_, snapshot)) {
        throw std::runtime_error("visual snapshot timed out");
    }

    NativeFrame frame;
    frame.tick = snapshot.meta.tick != 0 ? snapshot.meta.tick
                                         : bridge_->current_tick();
    frame.lattice_size = snapshot.meta.lattice_size != 0
                             ? snapshot.meta.lattice_size
                             : bridge_->lattice().size();
    frame.total_manifested = snapshot.particles.total_manifested;
    frame.particles.reserve(snapshot.particles.records.size());

    const Lattice& lattice = bridge_->lattice();
    for (const VisualParticleRecord& rec : snapshot.particles.records) {
        if (rec.index < 0) continue;
        const Coord c = lattice.coord(rec.index);
        NativeParticle p;
        p.x = static_cast<float>(c.x) + 0.5f + rec.remainder_x;
        p.y = static_cast<float>(c.y) + 0.5f + rec.remainder_y;
        p.z = static_cast<float>(c.z) + 0.5f + rec.remainder_z;
        if (rec.state >= 0) {
            p.r = 0.29f;
            p.g = 0.87f;
            p.b = 0.50f;
        } else {
            p.r = 0.97f;
            p.g = 0.44f;
            p.b = 0.44f;
        }
        p.size = 0.55f;
        frame.particles.push_back(p);
    }
    append_flux(*bridge_, frame);
    fill_frame_meta(frame);
    return frame;
}

int NativeEngineSession::lattice_size() const {
    return bridge_->lattice().size();
}

int NativeEngineSession::current_tick() const { return bridge_->current_tick(); }

const char* NativeEngineSession::backend_name() const {
    return bridge_->backend_kind() == Backend::Kind::Gpu ? "cuda" : "cpu";
}

bool NativeEngineSession::try_enable_interop(void* shared_buffer_handle,
                                             std::uint64_t buffer_bytes,
                                             void* shared_fence_handle) {
#ifdef FTD_ENABLE_CUDA
    if (bridge_->backend_kind() != Backend::Kind::Gpu) return false;
    ftd::gpu::GpuEngine* engine = bridge_->gpu_engine_ptr();
    if (!engine) return false;
    if (!engine->import_d3d12_particle_buffer(shared_buffer_handle, buffer_bytes)) {
        return false;
    }
    if (!engine->import_d3d12_fence(shared_fence_handle)) return false;
    interop_enabled_ = true;
    return true;
#else
    (void)shared_buffer_handle; (void)buffer_bytes; (void)shared_fence_handle;
    return false;
#endif
}

bool NativeEngineSession::request_interop_gather(std::uint64_t fence_value) {
#ifdef FTD_ENABLE_CUDA
    if (!interop_enabled_) return false;
    ftd::gpu::GpuEngine* engine = bridge_->gpu_engine_ptr();
    if (!engine) return false;
    return engine->interop_gather_particles(kMaxVisualParticleCapture, fence_value);
#else
    (void)fence_value;
    return false;
#endif
}

int NativeEngineSession::poll_interop_particle_count() {
#ifdef FTD_ENABLE_CUDA
    if (!interop_enabled_) return -1;
    ftd::gpu::GpuEngine* engine = bridge_->gpu_engine_ptr();
    if (!engine || !engine->interop_gather_ready()) return -1;
    return static_cast<int>(engine->interop_particle_count());
#else
    return -1;
#endif
}

#ifdef FTD_ENABLE_CUDA
ftd::gpu::GpuEngine* NativeEngineSession::debug_gpu_engine() {
    return bridge_->gpu_engine_ptr();
}
#endif

InteropReloadOutcome reimport_interop_after_reload(
    NativeEngineSession& session, void* shared_buffer_handle,
    std::uint64_t buffer_bytes, void* shared_fence_handle, bool was_active) {
    InteropReloadOutcome outcome;
    bool reimported = false;
    if (shared_buffer_handle && shared_fence_handle) {
        reimported = session.try_enable_interop(shared_buffer_handle, buffer_bytes,
                                                 shared_fence_handle);
    }
    outcome.interop_active = reimported;
    if (reimported) {
        outcome.log_enabled = !was_active;
    } else if (was_active) {
        outcome.log_lost = true;
    }
    return outcome;
}

}  // namespace ftd::native_desktop
