#include "native_desktop/engine_session.h"

#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"
#include "ftd/term_toggles.h"
#include "ftd/visual_field_sample.h"
#include "ftd/visual_snapshot.h"

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
    boot();
}

void NativeEngineSession::boot() {
    if (options_.lattice_size < 4 || options_.lattice_size > 256) {
        throw std::invalid_argument("lattice_size must be in [4, 256]");
    }

    if (options_.force_cpu) {
        request_cpu_backend();
    }

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

void NativeEngineSession::tick() { bridge_->tick(); }

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

}  // namespace ftd::native_desktop
