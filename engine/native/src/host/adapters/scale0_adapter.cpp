// host/adapters/scale0_adapter.cpp — Scale 0 (RenderBridge) behind the seam.
//
// Re-homes NativeEngineSession::boot()/capture()/interop + the Scale-0 command
// dispatch. Mutations/observations/snapshots travel through the existing free
// functions (apply_mutation_on_bridge / observe_on_bridge / build_snapshot) so
// behavior is identical to the session this replaces. The one intentional
// behavior CHANGE is the W9 fix in boot() (see below).

#include "native/host/adapters/scale0_adapter.h"
#include "native/host/adapters/scale1_adapter.h"
#include "native/host/adapters/streamlines.h"
#include "native/scale0_overlays.h"

#include "ftd/constants.h"       // DELTA_SQUARED (DUAL_DELTA), canonical chain
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
#include <array>
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

// ── Colour ramps (one per OverlayRamp) ────────────────────────────────────

// Cool→hot magnitude ramp (blue → green → red) for force arrows.
void ramp_cool_hot(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    r = std::clamp(1.5f * t - 0.2f, 0.0f, 1.0f);
    g = std::clamp(1.0f - std::abs(2.0f * t - 1.0f) * 0.9f, 0.0f, 1.0f);
    b = std::clamp(1.2f - 1.8f * t, 0.0f, 1.0f);
}

// Sign-aware diverging ramp: v<0 → cool (blue), v≥0 → warm (red); t in [0,1]
// sets intensity. For signed scalars (divergence, Gauss residual).
void ramp_diverging(float v, float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    if (v >= 0.0f) {
        r = 0.35f + 0.65f * t;
        g = 0.35f - 0.15f * t;
        b = 0.30f - 0.25f * t;
    } else {
        r = 0.30f - 0.25f * t;
        g = 0.45f + 0.20f * t;
        b = 0.55f + 0.45f * t;
    }
    r = std::clamp(r, 0.0f, 1.0f);
    g = std::clamp(g, 0.0f, 1.0f);
    b = std::clamp(b, 0.0f, 1.0f);
}

// State field: s=+1 saturated red, s=-1 saturated blue (void already dropped by
// the threshold). Intensity is fixed (the state is ternary, not a magnitude).
void ramp_state(float v, float& r, float& g, float& b) {
    if (v >= 0.0f) { r = 0.97f; g = 0.44f; b = 0.44f; }
    else           { r = 0.36f; g = 0.55f; b = 0.98f; }
}

// Latency L: blue (low) → red (high).
void ramp_latency(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    r = 0.20f + 0.80f * t;
    g = std::clamp(0.35f - std::abs(2.0f * t - 1.0f) * 0.25f, 0.0f, 1.0f);
    b = 0.95f - 0.80f * t;
}

// Horizon shell (L≥threshold): dim ember red so the shell reads as a dark rim.
void ramp_horizon(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    r = 0.35f + 0.45f * t;
    g = 0.06f + 0.05f * t;
    b = 0.05f;
}

// Poynting energy-flux arrows: yellow → orange with magnitude.
void ramp_poynting(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    r = 1.0f;
    g = std::clamp(0.90f - 0.55f * t, 0.0f, 1.0f);
    b = 0.10f;
}

// Weak (∇×J pseudovector) arrows: violet.
void ramp_weak(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    r = 0.55f + 0.35f * t;
    g = 0.20f + 0.10f * t;
    b = 0.85f;
}

void overlay_point_color(OverlayRamp ramp, float v, float t, float& r, float& g, float& b) {
    switch (ramp) {
        case OverlayRamp::StateSign: ramp_state(v, r, g, b); break;
        case OverlayRamp::Latency:   ramp_latency(t, r, g, b); break;
        case OverlayRamp::Horizon:   ramp_horizon(t, r, g, b); break;
        case OverlayRamp::Diverging:
        default:                     ramp_diverging(v, t, r, g, b); break;
    }
}

void overlay_arrow_color(OverlayRamp ramp, float t, float& r, float& g, float& b) {
    switch (ramp) {
        case OverlayRamp::Poynting: ramp_poynting(t, r, g, b); break;
        case OverlayRamp::Weak:     ramp_weak(t, r, g, b); break;
        case OverlayRamp::CoolHot:
        default:                    ramp_cool_hot(t, r, g, b); break;
    }
}

// True while `t`/`v` clear the descriptor's keep test: an absolute lower bound
// on the raw value (select_min ≥ 0, used by the Horizon shell) OR a relative
// magnitude-fraction threshold.
bool overlay_keep(const OverlayDescriptor& d, float v, float t) {
    if (d.select_min >= 0.0f) return v >= d.select_min;
    return t >= d.threshold;
}

// Vector-field overlay → line-segment arrows (dim base → bright tip), autoscaled
// so the longest vector spans ~0.9 of a sample cell. Appends into the shared
// frame.field_lines group; multiple active vector overlays coexist.
void append_overlay_arrows(RenderBridge& rb, NativeFrame& frame,
                           const OverlayDescriptor& d, VisualFieldKind kind) {
    const int L = rb.lattice().size();
    const int stride = d.force_stride1 ? 1 : std::max(1, (L + 31) / 32);
    VisualFieldSample sample;
    rb.copy_visual_field_sample(kind, stride, sample);
    if (sample.components != 3u || sample.count() == 0) return;

    // Canonical parity-odd scaling for ∇×J (never hardcoded; DELTA_SQUARED from
    // the ontic chain via constants.h). Uniform, so it cancels under the
    // per-overlay autoscale, but the geometry is built from the canonical value.
    const float vmul =
        d.scale_by_dual_delta ? static_cast<float>(std::sqrt(ftd::DELTA_SQUARED)) : 1.0f;

    const std::size_t n = sample.count();
    std::vector<float> mag(n, 0.0f);
    float max_mag = 1.0e-6f;
    for (std::size_t i = 0; i < n; ++i) {
        const float dx = sample.data[i * 3u] * vmul;
        const float dy = sample.data[i * 3u + 1u] * vmul;
        const float dz = sample.data[i * 3u + 2u] * vmul;
        mag[i] = std::sqrt(dx * dx + dy * dy + dz * dz);
        max_mag = std::max(max_mag, mag[i]);
    }
    const float target = 0.9f * static_cast<float>(std::max(1, sample.effective_stride));
    const float vscale = (target / max_mag) * vmul;

    frame.field_lines.reserve(frame.field_lines.size() + n);
    for (std::size_t i = 0; i < n; ++i) {
        const float t = mag[i] / max_mag;
        if (!overlay_keep(d, mag[i], t)) continue;
        const float bx = sample.positions[i * 3u];
        const float by = sample.positions[i * 3u + 1u];
        const float bz = sample.positions[i * 3u + 2u];
        float r, g, b;
        overlay_arrow_color(d.ramp, t, r, g, b);
        NativeLine line;
        line.x0 = bx;
        line.y0 = by;
        line.z0 = bz;
        line.r0 = r * 0.35f;  // dim base
        line.g0 = g * 0.35f;
        line.b0 = b * 0.35f;
        line.x1 = bx + sample.data[i * 3u] * vscale;
        line.y1 = by + sample.data[i * 3u + 1u] * vscale;
        line.z1 = bz + sample.data[i * 3u + 2u] * vscale;
        line.r1 = r;  // bright tip
        line.g1 = g;
        line.b1 = b;
        frame.field_lines.push_back(line);
    }
}

// Scalar-field overlay → magnitude/sign-coloured points through the sprite path.
// Appends into the shared frame.flux group; multiple active point overlays (and
// the sprite Flux-Volume cloud) coexist.
void append_overlay_points(RenderBridge& rb, NativeFrame& frame,
                           const OverlayDescriptor& d, VisualFieldKind kind) {
    const int L = rb.lattice().size();
    const int stride = d.force_stride1 ? 1 : std::max(1, (L + 31) / 32);
    VisualFieldSample sample;
    rb.copy_visual_field_sample(kind, stride, sample);
    if (sample.components != 1u || sample.count() == 0) return;

    const std::size_t n = sample.count();
    float max_mag = 1.0e-6f;
    for (std::size_t i = 0; i < n; ++i)
        max_mag = std::max(max_mag, std::abs(sample.data[i]));

    frame.flux.reserve(frame.flux.size() + n);
    for (std::size_t i = 0; i < n; ++i) {
        const float v = sample.data[i];
        const float t = std::abs(v) / max_mag;
        if (!overlay_keep(d, v, t)) continue;
        NativeParticle p;
        p.x = sample.positions[i * 3u];
        p.y = sample.positions[i * 3u + 1u];
        p.z = sample.positions[i * 3u + 2u];
        overlay_point_color(d.ramp, v, t, p.r, p.g, p.b);
        p.size = 0.20f + 0.55f * t;
        frame.flux.push_back(p);
    }
}

// Map a streamline overlay's field kind → the integrator's overlay profile
// (seed strategy + colour ramp). Only the three streamline kinds reach here.
streamlines::Overlay streamline_overlay_for(VisualFieldKind kind) {
    switch (kind) {
        case VisualFieldKind::Electric: return streamlines::Overlay::Electric;
        case VisualFieldKind::Magnetic: return streamlines::Overlay::Magnetic;
        case VisualFieldKind::FluxVector:
        default:                        return streamlines::Overlay::Flux;
    }
}

// Streamline overlay → RK4-traced field lines appended into the shared
// frame.field_lines group. Samples the field DENSELY (stride 1) so the
// integrator can trace through a voxel-indexed lookup, and seeds E/B from the
// frame's already-gathered particle centres. Multiple streamline overlays (and
// the arrow/point overlays) coexist in field_lines.
void append_overlay_streamlines(RenderBridge& rb, NativeFrame& frame,
                                const OverlayDescriptor& d, VisualFieldKind kind) {
    VisualFieldSample sample;
    rb.copy_visual_field_sample(kind, /*stride=*/1, sample);
    if (sample.components != 3u || sample.count() == 0) return;

    // Particle-anchored E/B seeds read the frame's manifested particle centres
    // (voxel-centre coords, already built into frame.particles this capture).
    std::vector<std::array<float, 3>> particles;
    particles.reserve(frame.particles.size());
    for (const NativeParticle& p : frame.particles)
        particles.push_back(std::array<float, 3>{p.x, p.y, p.z});

    streamlines::append(sample, particles, rb.lattice().size(),
                        streamline_overlay_for(kind), frame.field_lines);
    (void)d;
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
    // SetOverlay is adapter view-state only (which overlays capture() renders);
    // it never mutates the RenderBridge, so update the active set here and
    // short-circuit — apply_mutation_on_bridge has no case for it.
    if (const SetOverlay* so = std::get_if<SetOverlay>(s0)) {
        set_overlay(static_cast<OverlayId>(so->overlay_id), so->on);
        ApplyResult ok;
        ok.ok = true;
        return ok;
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
    // Overlay compositing. Empty set (default): the ambient flux cloud, as
    // before. Otherwise composite EVERY active overlay into this frame — each
    // sampled once (O(active overlays)) — appending arrows into field_lines and
    // points into flux. Groups coexist; the ambient cloud shows only when empty
    // (mirrors the web `anyFieldActive` gate).
    if (active_overlays_.empty()) {
        append_flux(*bridge_, frame);
    } else {
        for (const OverlayId id : active_overlays_) {
            const OverlayDescriptor* d = overlay_by_id(static_cast<std::uint32_t>(id));
            if (!d) continue;
            const VisualFieldKind kind = resolve_overlay_kind(*d);
            switch (d->render) {
                case OverlayRender::Sprite:
                    append_flux(*bridge_, frame);  // Flux Volume == ambient cloud
                    break;
                case OverlayRender::Arrows:
                    append_overlay_arrows(*bridge_, frame, *d, kind);
                    break;
                case OverlayRender::Points:
                    append_overlay_points(*bridge_, frame, *d, kind);
                    break;
                case OverlayRender::Streamline:
                    append_overlay_streamlines(*bridge_, frame, *d, kind);
                    break;
            }
        }
    }

    frame.scenario = scenario_;
    frame.backend = backend_name();
    frame.status = status_;
    frame.flux_boundary = flux_boundary_;
    last_total_manifested_ = frame.total_manifested;
    return frame;
}

void Scale0Adapter::set_overlay(OverlayId id, bool on) {
    const auto it = std::find(active_overlays_.begin(), active_overlays_.end(), id);
    if (on) {
        if (it == active_overlays_.end()) active_overlays_.push_back(id);
    } else if (it != active_overlays_.end()) {
        active_overlays_.erase(it);
    }
}

bool Scale0Adapter::overlay_active(OverlayId id) const {
    return std::find(active_overlays_.begin(), active_overlays_.end(), id)
           != active_overlays_.end();
}

ftd::VisualFieldKind Scale0Adapter::resolve_overlay_kind(const OverlayDescriptor& d) const {
    // Only the |J|²-proxy Latency slot (kind 8) is overridden; every other kind
    // passes through. The single native mass-gravity scenario is the seed that
    // drives a real Poisson latency field (mirrors SCALE0_MASS_GRAVITY_SCENARIOS).
    if (d.kind == VisualFieldKind::Latency && scenario_ == "s0-seed-massive-body")
        return VisualFieldKind::PoissonLatency;
    return d.kind;
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
        case 1:
            // R1 validation: ParticleEngine (a real ScaleEngine) behind the seam.
            return std::make_unique<Scale1Adapter>();
        default:
            // Scale 2/5 adapters land in later steps (AtomEngine, CosmicEngine —
            // all `: ScaleEngine`). Until then, fall back to Scale 0 so the host
            // always has a live adapter.
            return std::make_unique<Scale0Adapter>();
    }
}

}  // namespace ftd::native
