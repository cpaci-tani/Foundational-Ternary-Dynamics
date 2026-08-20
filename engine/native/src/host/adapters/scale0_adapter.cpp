// host/adapters/scale0_adapter.cpp — Scale 0 (RenderBridge) behind the seam.
//
// Re-homes NativeEngineSession::boot()/capture()/interop + the Scale-0 command
// dispatch. Mutations/observations/snapshots travel through the existing free
// functions (apply_mutation_on_bridge / observe_on_bridge / build_snapshot) so
// behavior is identical to the session this replaces. The one intentional
// behavior CHANGE is the W9 fix in boot() (see below).

#include "native/host/adapters/scale0_adapter.h"

#include "ftd/render_bridge.h"
#include "ftd/scenario_meta.h"
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
#include <variant>
#include <vector>

namespace ftd::native {
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

bool scenario_known(const std::string& id) {
    for (const auto name : ftd::scale0_scenario_ids()) {
        if (name == id) return true;
    }
    return false;
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

// Convert a Scale-0 payload alternative into the flat UiCommand the existing
// applier/observer free functions consume. Every Scale0Cmd alternative is also a
// UiCommand alternative, so this is a straight widening.
UiCommand to_ui_command(const Scale0Cmd& cmd) {
    return std::visit([](const auto& c) -> UiCommand { return c; }, cmd);
}

}  // namespace

Scale0Adapter::Scale0Adapter() = default;
Scale0Adapter::~Scale0Adapter() = default;

void Scale0Adapter::apply_boundary() {
    int mode = flux_boundary_;
    if (mode < 0 || mode > 2) mode = 2;
    flux_boundary_ = mode;
    bridge_->toggles.flux_boundary = static_cast<ftd::FluxBoundaryMode>(mode);
}

void Scale0Adapter::boot(const ftd::ScenarioMeta& meta, const RunConfig& cfg,
                         BootReport& out) {
    cfg_ = cfg;
    const std::string id = meta.id ? meta.id : "";

    if (cfg.lattice_size < 4 || cfg.lattice_size > 256) {
        // Preserve the old NativeEngineSession invalid-argument contract.
        throw std::invalid_argument("lattice_size must be in [4, 256]");
    }
    if (cfg.force_cpu) request_cpu_backend();

    flux_boundary_ = std::clamp(cfg.flux_boundary, 0, 2);
    interop_enabled_ = false;

    // NOTE: dt / sor_iterations from RunConfig are intentionally NOT forced at
    // boot — the legacy NativeEngineSession::boot() did not, and forcing them
    // would change the tree's initial physics profile. They are set at runtime
    // via the SetDt / SetSorIterations Scale-0 commands instead.
    auto make_bridge = [&]() {
        auto rb = std::make_unique<RenderBridge>(cfg.lattice_size);
        if (cfg.force_cpu) {
            rb->force_cpu();
        } else {
            rb->set_interactive_gpu_mode(true);
        }
        rb->toggles.flux_boundary = static_cast<ftd::FluxBoundaryMode>(flux_boundary_);
        return rb;
    };

    bridge_ = make_bridge();

    const bool known = scenario_known(id);
    if (!known) {
        // W9 case 1 — UNKNOWN id: dispatch_scenario() bailed before touching the
        // bridge, so the fresh bridge is clean. Seed a visible demo pair on it,
        // matching the legacy fallback (no half-mutation hazard here).
        seed_visible_pair(*bridge_);
        scenario_ = "demo-pair";
        status_ = "unknown scenario '" + id + "'; seeded a +/- pair";
        out.status = ReloadStatus::UnknownScenario;
        out.message = "unknown scenario: " + id;
    } else if (!dispatch_scenario(*bridge_, id)) {
        // W9 case 2/3 — VALIDATION-REJECT (or handler miss) of a REGISTERED id:
        // the scenario setup already mutated toggles/state, so the bridge is
        // half-mutated. The legacy code seeded a demo pair OVER that corrupted
        // state. Instead, discard the bridge and re-boot a fresh one into a
        // known-good scenario.
        bridge_ = make_bridge();
        constexpr const char* kKnownGood = "empty";
        dispatch_scenario(*bridge_, kKnownGood);
        scenario_ = kKnownGood;
        status_ = "scenario '" + id + "' rejected; rebooted to '" + kKnownGood + "'";
        out.status = ReloadStatus::ValidationRejected;
        out.message = "scenario rejected: " + id;
    } else {
        scenario_ = id;
        status_ = id;
        out.status = ReloadStatus::Success;
    }
    apply_boundary();

    // NOTE: the legacy NativeEngineSession never called
    // scheduler_.on_source_replaced() across a reload, and with the native
    // telemetry demand mask held at 0 the scheduler is inert (latest() returns an
    // empty snapshot). Matching that exactly keeps behavior identical; wiring
    // on_source_replaced() belongs with the first non-zero telemetry demand.

    out.scenario = scenario_;
    out.status_line = status_;
}

void Scale0Adapter::bind_sim_thread() {
    bridge_->bind_sim_thread();
}

void Scale0Adapter::tick() {
    bridge_->bind_sim_thread();
    bridge_->tick();
}

int Scale0Adapter::current_tick() const { return bridge_->current_tick(); }

bool Scale0Adapter::is_observation(const ScalePayload& payload) const {
    const Scale0Cmd* s0 = std::get_if<Scale0Cmd>(&payload);
    if (!s0) return false;
    return is_observation_command(to_ui_command(*s0));
}

bool Scale0Adapter::is_host_write(const ScalePayload& payload) const {
    const Scale0Cmd* s0 = std::get_if<Scale0Cmd>(&payload);
    if (!s0) return false;
    return is_harness_command(to_ui_command(*s0));
}

ApplyResult Scale0Adapter::apply(const ScalePayload& payload, ParameterJournal& journal,
                                 int apply_tick, LoopControl& loop) {
    ApplyResult result;
    const Scale0Cmd* s0 = std::get_if<Scale0Cmd>(&payload);
    if (!s0) {
        result.ok = false;
        result.error_code = 1;
        result.message = "Scale 0 received a non-Scale-0 payload";
        return result;
    }
    // The session used to persist the flux-boundary choice across reloads; keep
    // that here so a subsequent reboot re-applies it.
    if (const SetBoundary* sb = std::get_if<SetBoundary>(s0)) {
        flux_boundary_ = static_cast<int>(sb->mode);
    }
    QueuedCommand item;
    item.command = to_ui_command(*s0);
    // session == nullptr: the reload-shaped alternatives (LoadScenario /
    // ApplyReboot / SetLatticeSize) are core commands the HOST handles and never
    // reach here, so no session back-reference is needed.
    return apply_mutation_on_bridge(*bridge_, /*session=*/nullptr, item, journal,
                                    apply_tick, loop);
}

void Scale0Adapter::flush_writes() {
    bridge_->backend().flush_host_mutations();
}

void Scale0Adapter::begin_boundary() {
    boundary_snapshot_ = Scale0Snapshot{};
}

bool Scale0Adapter::observe(const ScalePayload& payload) {
    const Scale0Cmd* s0 = std::get_if<Scale0Cmd>(&payload);
    if (!s0) return false;
    const UiCommand ui = to_ui_command(*s0);
    const ObservationResult r =
        observe_on_bridge(*bridge_, ui, boundary_snapshot_, obs_state_);
    return r.status == ObservationStatus::Ready;
}

void Scale0Adapter::on_tick_complete() {
    scheduler_.on_tick_complete(*bridge_);
    (void)scheduler_.pump(*bridge_);
}

void Scale0Adapter::build_snapshot(const DataNeeds& needs) {
    obs_state_.demand = needs;
    const ftd::NativeTelemetryScheduler::CachedView cached = scheduler_.latest();
    ftd::native::build_snapshot(*bridge_, &cached, needs, boundary_snapshot_);
    boundary_snapshot_.frame.scenario = scenario_;
    boundary_snapshot_.frame.backend = backend_name();
    boundary_snapshot_.frame.status = status_;
    boundary_snapshot_.frame.tick = bridge_->current_tick();
    boundary_snapshot_.frame.lattice_size = bridge_->lattice().size();
    boundary_snapshot_.frame.total_manifested = last_total_manifested_;
}

ScaleSnapshot Scale0Adapter::take_scale_snapshot() {
    ScaleSnapshot out = std::move(boundary_snapshot_);
    boundary_snapshot_ = Scale0Snapshot{};
    return out;
}

NativeFrame Scale0Adapter::capture() {
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
    frame.tick = snapshot.meta.tick != 0 ? snapshot.meta.tick : bridge_->current_tick();
    frame.lattice_size =
        snapshot.meta.lattice_size != 0 ? snapshot.meta.lattice_size : bridge_->lattice().size();
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

    frame.scenario = scenario_;
    frame.backend = backend_name();
    frame.status = status_;
    frame.flux_boundary = flux_boundary_;
    last_total_manifested_ = frame.total_manifested;
    return frame;
}

const char* Scale0Adapter::backend_name() const {
    return bridge_->backend_kind() == Backend::Kind::Gpu ? "cuda" : "cpu";
}

int Scale0Adapter::lattice_size() const { return bridge_->lattice().size(); }

bool Scale0Adapter::try_enable_interop(void* buf, std::uint64_t bytes, void* fence) {
#ifdef FTD_ENABLE_CUDA
    if (bridge_->backend_kind() != Backend::Kind::Gpu) return false;
    ftd::gpu::GpuEngine* engine = bridge_->gpu_engine_ptr();
    if (!engine) return false;
    if (!engine->import_d3d12_particle_buffer(buf, bytes)) return false;
    if (!engine->import_d3d12_fence(fence)) return false;
    interop_enabled_ = true;
    return true;
#else
    (void)buf; (void)bytes; (void)fence;
    return false;
#endif
}

bool Scale0Adapter::request_interop_gather(std::uint64_t fence_value) {
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

int Scale0Adapter::poll_interop_particle_count() {
#ifdef FTD_ENABLE_CUDA
    if (!interop_enabled_) return -1;
    ftd::gpu::GpuEngine* engine = bridge_->gpu_engine_ptr();
    if (!engine || !engine->interop_gather_ready()) return -1;
    return static_cast<int>(engine->interop_particle_count());
#else
    return -1;
#endif
}

// ── The one place that names concrete adapter types. Everything else generic. ──
std::unique_ptr<ScaleAdapter> make_scale_adapter(int scale_level) {
    switch (scale_level) {
        case 0:
            return std::make_unique<Scale0Adapter>();
        default:
            // Scale 1+ adapters land in later steps (ParticleEngine, AtomEngine,
            // CosmicEngine — all `: ScaleEngine`). Until then, fall back to
            // Scale 0 so the host always has a live adapter.
            return std::make_unique<Scale0Adapter>();
    }
}

}  // namespace ftd::native
