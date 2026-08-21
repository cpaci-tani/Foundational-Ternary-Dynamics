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
#include "native/scenario_catalog.h"  // ftd::native::ScenarioMeta (self-contained; not the untracked ftd/scenario_meta.h)
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
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <thread>
#include <unordered_map>
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

// Ambient flux-cloud colour (cool blue brightening with |J|), factored out of
// append_flux so the Flux-Slice mid-plane points share the exact same look.
void flux_cloud_color(float t, float& r, float& g, float& b) {
    r = 0.12f + 0.25f * t;
    g = 0.40f + 0.45f * t;
    b = 0.85f + 0.15f * t;
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
        flux_cloud_color(t, p.r, p.g, p.b);
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

// ── Tranche-2 ramps (ported from engine/web/js/viewport/color-ramps.js) ─────

// Approximate viridis (purple → teal → yellow), t∈[0,1]. |ψ|².
void ramp_viridis(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    if (t < 0.5f) {
        const float u = t * 2.0f;
        r = 0.267f * (1.0f - u) + 0.130f * u;
        g = 0.004f * (1.0f - u) + 0.566f * u;
        b = 0.329f * (1.0f - u) + 0.551f * u;
    } else {
        const float u = (t - 0.5f) * 2.0f;
        r = 0.130f * (1.0f - u) + 0.993f * u;
        g = 0.566f * (1.0f - u) + 0.906f * u;
        b = 0.551f * (1.0f - u) + 0.144f * u;
    }
}

// Diverging red(+)/blue(−), signed v∈[-1,1]. ℒ Lagrangian density.
void ramp_rdbu(float v, float& r, float& g, float& b) {
    v = std::clamp(v, -1.0f, 1.0f);
    if (v >= 0.0f) {
        const float u = v;
        r = 0.969f * (1.0f - u) + 0.698f * u;
        g = 0.969f * (1.0f - u) + 0.094f * u;
        b = 0.969f * (1.0f - u) + 0.169f * u;
    } else {
        const float u = -v;
        r = 0.969f * (1.0f - u) + 0.129f * u;
        g = 0.969f * (1.0f - u) + 0.400f * u;
        b = 0.969f * (1.0f - u) + 0.675f * u;
    }
}

// Straight grayscale, t∈[0,1]. Entropy s.
void ramp_grayscale(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    r = t; g = t; b = t;
}

// Chirality: signed red (v≥0, L-dominant) / blue (v<0, R-dominant); t=|v|/max.
void ramp_chirality(float v, float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    if (v >= 0.0f) { r = 0.90f * t; g = 0.25f * t; b = 0.15f * t; }
    else           { r = 0.15f * t; g = 0.35f * t; b = 0.90f * t; }
}

// Cyclic HSL over hue∈[0,1) at S=1, L=0.5 (port of rampCyclicHSL). Phase φ.
void ramp_cyclic_hsl(float hue01, float& r, float& g, float& b) {
    hue01 -= std::floor(hue01);              // wrap to [0,1)
    const float h6 = hue01 * 6.0f;
    const float c = 1.0f;
    const float x = c * (1.0f - std::abs(std::fmod(h6, 2.0f) - 1.0f));
    if (h6 < 1.0f)      { r = c; g = x; b = 0.0f; }
    else if (h6 < 2.0f) { r = x; g = c; b = 0.0f; }
    else if (h6 < 3.0f) { r = 0.0f; g = c; b = x; }
    else if (h6 < 4.0f) { r = 0.0f; g = x; b = c; }
    else if (h6 < 5.0f) { r = x; g = 0.0f; b = c; }
    else                { r = c; g = 0.0f; b = x; }
}

// ── Tranche-5 rubber-sheet ramps (ports of the sheet ramps in
//    engine/web/js/viewport/color-ramps.js). Signed ramps take t∈[-1,1];
//    unsigned take t∈[0,1]. GravWell is coloured by |t| (the caller passes it). ─

// Φ potential: peak yellow (t→0) → deep blue well (t→1). t is |height| here.
void ramp_grav_well(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    if (t > 0.5f) {
        const float u = (t - 0.5f) * 2.0f;
        r = 0.0f;
        g = 0.4f * (1.0f - u);
        b = 0.8f * (1.0f - u) + 0.2f * u;
    } else {
        const float u = t * 2.0f;
        r = 1.0f * (1.0f - u);
        g = 1.0f * (1.0f - u) + 0.4f * u;
        b = 0.8f * u;
    }
}

// EM energy u: teal → warm orange, t∈[0,1].
void ramp_em_energy(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    r = 0.05f * (1.0f - t) + 0.98f * t;
    g = 0.55f * (1.0f - t) + 0.62f * t;
    b = 0.55f * (1.0f - t) + 0.14f * t;
}

// Charge ρ = ∇·J: diverging blue(sink) ↔ red(source), t∈[-1,1].
void ramp_charge(float t, float& r, float& g, float& b) {
    t = std::clamp(t, -1.0f, 1.0f);
    if (t >= 0.0f) {
        const float u = t;
        r = 0.95f * (1.0f - u) + 0.90f * u;
        g = 0.95f * (1.0f - u) + 0.10f * u;
        b = 0.95f * (1.0f - u) + 0.20f * u;
    } else {
        const float u = -t;
        r = 0.95f * (1.0f - u) + 0.13f * u;
        g = 0.95f * (1.0f - u) + 0.35f * u;
        b = 0.95f * (1.0f - u) + 0.85f * u;
    }
}

// Vorticity ω: magma near-black → violet → gold, t∈[0,1].
void ramp_vorticity(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    if (t < 0.5f) {
        const float u = t * 2.0f;
        r = 0.02f * (1.0f - u) + 0.48f * u;
        g = 0.02f * (1.0f - u) + 0.05f * u;
        b = 0.08f * (1.0f - u) + 0.53f * u;
    } else {
        const float u = (t - 0.5f) * 2.0f;
        r = 0.48f * (1.0f - u) + 1.00f * u;
        g = 0.05f * (1.0f - u) + 0.85f * u;
        b = 0.53f * (1.0f - u) + 0.20f * u;
    }
}

// P_E = ½|E|²: pale yellow → saturated red, t∈[0,1].
void ramp_e_pressure(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    r = 0.95f;
    g = 0.95f * (1.0f - t) + 0.25f * t;
    b = 0.65f * (1.0f - t) + 0.15f * t;
}

// P_B = (c²/2)|B|²: pale cyan → deep teal, t∈[0,1].
void ramp_b_pressure(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    r = 0.75f * (1.0f - t);
    g = 0.95f * (1.0f - t) + 0.55f * t;
    b = 0.95f * (1.0f - t) + 0.70f * t;
}

// Sheet per-vertex colour: dispatch on the sheet ramp. `t_signed` is the
// signed normalised height (∈[-1,1] for signed sheets, ∈[0,1] otherwise); the
// grav-well ramp keys off |t| (mirrors the web `rampGravWell(|t|)`), the
// signed charge ramp keeps the sign, and the unsigned ramps clamp to [0,1].
void sheet_vertex_color(OverlayRamp ramp, float t_signed, float& r, float& g, float& b) {
    switch (ramp) {
        case OverlayRamp::GravWell:  ramp_grav_well(std::abs(t_signed), r, g, b); break;
        case OverlayRamp::EmEnergy:  ramp_em_energy(t_signed, r, g, b); break;
        case OverlayRamp::Charge:    ramp_charge(t_signed, r, g, b); break;
        case OverlayRamp::Vorticity: ramp_vorticity(t_signed, r, g, b); break;
        case OverlayRamp::EPressure: ramp_e_pressure(t_signed, r, g, b); break;
        case OverlayRamp::BPressure: ramp_b_pressure(t_signed, r, g, b); break;
        default:                     r = g = b = 0.7f; break;
    }
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

// ── Tranche-2 EXTEND helpers ───────────────────────────────────────────────
// All emit through the SAME two primitives (frame.flux sprite points and
// frame.field_lines line segments) — no new PSO. Ported from the web renderers
// (field-quantum-renderer.js, field-topology-renderer.js, overlay-frames.js).

// Bounded visualization stride for the derived point overlays — the ambient
// cloud's stride, so a derived overlay has the same point density as |J|.
int overlay_stride(int L) { return std::max(1, (L + 31) / 32); }

// Derived-scalar overlays → sprite points. Which scalar is set by d.derive:
//   PsiSquared: |J|² (viridis, max-normalised, threshold 0.02);
//   Lagrangian: ½|E|² − ½(∇·J)² (signed RdBu, threshold 0.10) — E and ∇·J are
//               sampled independently and PAIRED BY VOXEL POSITION (the two
//               samplers compact away sub-floor voxels, so raw index does not
//               address the same voxel — port of computeLagrangianDensityFrame);
//   Entropy:    4p(1−p), p=|J|/|J|max (grayscale + per-point jitter, thr 0.04);
//   Chirality:  |J|·δ, δ = √DELTA_SQUARED = DUAL_DELTA canonical (signed
//               red/blue, threshold 0.02).
void append_overlay_derived_points(RenderBridge& rb, NativeFrame& frame,
                                   const OverlayDescriptor& d) {
    const int L = rb.lattice().size();
    const int stride = overlay_stride(L);

    if (d.derive == OverlayDerive::Lagrangian) {
        VisualFieldSample eF, dF;
        rb.copy_visual_field_sample(VisualFieldKind::Electric, stride, eF);
        rb.copy_visual_field_sample(VisualFieldKind::Divergence, stride, dF);
        if (eF.components != 3u || eF.count() == 0) return;
        auto vox_key = [L](float px, float py, float pz) -> std::int64_t {
            const std::int64_t ix = static_cast<std::int64_t>(std::floor(px));
            const std::int64_t iy = static_cast<std::int64_t>(std::floor(py));
            const std::int64_t iz = static_cast<std::int64_t>(std::floor(pz));
            return (ix * L + iy) * L + iz;
        };
        std::unordered_map<std::int64_t, float> div_at;
        if (dF.components == 1u) {
            div_at.reserve(dF.count());
            for (std::size_t i = 0; i < dF.count(); ++i)
                div_at[vox_key(dF.positions[i * 3u], dF.positions[i * 3u + 1u],
                               dF.positions[i * 3u + 2u])] = dF.data[i];
        }
        const std::size_t n = eF.count();
        std::vector<float> lval(n, 0.0f);
        float max_abs = 1.0e-9f;
        for (std::size_t i = 0; i < n; ++i) {
            const float ex = eF.data[i * 3u], ey = eF.data[i * 3u + 1u], ez = eF.data[i * 3u + 2u];
            const float kinetic = 0.5f * (ex * ex + ey * ey + ez * ez);
            float grad = 0.0f;
            const auto it = div_at.find(vox_key(eF.positions[i * 3u], eF.positions[i * 3u + 1u],
                                                eF.positions[i * 3u + 2u]));
            if (it != div_at.end()) grad = 0.5f * it->second * it->second;
            lval[i] = kinetic - grad;
            max_abs = std::max(max_abs, std::abs(lval[i]));
        }
        frame.flux.reserve(frame.flux.size() + n);
        for (std::size_t i = 0; i < n; ++i) {
            const float t = lval[i] / max_abs;  // signed [-1,1]
            if (std::abs(t) < d.threshold) continue;
            NativeParticle p;
            p.x = eF.positions[i * 3u];
            p.y = eF.positions[i * 3u + 1u];
            p.z = eF.positions[i * 3u + 2u];
            ramp_rdbu(t, p.r, p.g, p.b);
            p.size = 0.20f + 0.55f * std::abs(t);
            frame.flux.push_back(p);
        }
        return;
    }

    // PsiSquared / Entropy / Chirality all derive from |J| alone.
    VisualFieldSample s;
    rb.copy_visual_field_sample(VisualFieldKind::FluxVector, stride, s);
    if (s.components != 3u || s.count() == 0) return;
    const std::size_t n = s.count();
    std::vector<float> mag(n, 0.0f);
    float max_mag = 1.0e-9f;
    for (std::size_t i = 0; i < n; ++i) {
        const float x = s.data[i * 3u], y = s.data[i * 3u + 1u], z = s.data[i * 3u + 2u];
        mag[i] = std::sqrt(x * x + y * y + z * z);
        max_mag = std::max(max_mag, mag[i]);
    }
    // Canonical DUAL_DELTA from the ontic chain (never hardcoded).
    const float delta = static_cast<float>(std::sqrt(ftd::DELTA_SQUARED));
    frame.flux.reserve(frame.flux.size() + n);
    for (std::size_t i = 0; i < n; ++i) {
        NativeParticle p;
        p.x = s.positions[i * 3u];
        p.y = s.positions[i * 3u + 1u];
        p.z = s.positions[i * 3u + 2u];
        if (d.derive == OverlayDerive::PsiSquared) {
            const float t = (mag[i] * mag[i]) / (max_mag * max_mag);
            if (t < d.threshold) continue;
            ramp_viridis(t, p.r, p.g, p.b);
            p.size = 0.20f + 0.55f * t;
        } else if (d.derive == OverlayDerive::Entropy) {
            const float pfrac = mag[i] / max_mag;
            const float sval = 4.0f * pfrac * (1.0f - pfrac);  // 0→1→0 impurity
            if (sval < d.threshold) continue;                  // absolute (s∈[0,1])
            // Deterministic per-index sparkle jitter (port of the web offset;
            // fixed seed so captures are reproducible).
            const std::uint32_t seed = static_cast<std::uint32_t>(i) * 9301u + 1u;
            const float r1 = static_cast<float>((seed * 49297u) % 233280u) / 233280.0f - 0.5f;
            const float r2 = static_cast<float>((seed * 2147u) % 233280u) / 233280.0f - 0.5f;
            const float r3 = static_cast<float>((seed * 8191u) % 233280u) / 233280.0f - 0.5f;
            const float off = sval * 0.8f;
            p.x += r1 * off; p.y += r2 * off; p.z += r3 * off;
            ramp_grayscale(sval, p.r, p.g, p.b);
            p.size = 0.20f + 0.55f * sval;
        } else {  // Chirality: |J|·δ (non-negative under the scalar proxy → warm)
            const float t = mag[i] / max_mag;  // = (|J|·δ)/(|J|max·δ)
            if (t < d.threshold) continue;
            ramp_chirality(mag[i] * delta, t, p.r, p.g, p.b);
            p.size = 0.20f + 0.55f * t;
        }
        frame.flux.push_back(p);
    }
}

// Dual J: amplitude split J_L=J(1+δ)/2 (warm) + J_R=J(1−δ)/2 (cool) from
// FluxVector, δ = √DELTA_SQUARED (canonical). Two coloured point sets, both
// normalised by the shared max amplitude, threshold 2% of that max. Under the
// scalar (1±δ)/2 proxy J_L and J_R are collinear with J, so this is an
// amplitude-asymmetry demonstration, not a true chirality projection.
void append_dual_flux(RenderBridge& rb, NativeFrame& frame, const OverlayDescriptor& d) {
    const int L = rb.lattice().size();
    const int stride = overlay_stride(L);
    VisualFieldSample s;
    rb.copy_visual_field_sample(VisualFieldKind::FluxVector, stride, s);
    if (s.components != 3u || s.count() == 0) return;
    const float delta = static_cast<float>(std::sqrt(ftd::DELTA_SQUARED));
    const float lf = (1.0f + delta) * 0.5f;
    const float rf = std::abs((1.0f - delta) * 0.5f);
    const std::size_t n = s.count();
    std::vector<float> mag_l(n), mag_r(n);
    float max_val = 1.0e-20f;
    for (std::size_t i = 0; i < n; ++i) {
        const float x = s.data[i * 3u], y = s.data[i * 3u + 1u], z = s.data[i * 3u + 2u];
        const float m = std::sqrt(x * x + y * y + z * z);
        mag_l[i] = m * lf;
        mag_r[i] = m * rf;
        max_val = std::max(max_val, std::max(mag_l[i], mag_r[i]));
    }
    const float thr = max_val * d.threshold;
    frame.flux.reserve(frame.flux.size() + 2u * n);
    for (std::size_t i = 0; i < n; ++i) {  // L (warm)
        if (mag_l[i] < thr) continue;
        const float t = mag_l[i] / max_val;
        NativeParticle p;
        p.x = s.positions[i * 3u]; p.y = s.positions[i * 3u + 1u]; p.z = s.positions[i * 3u + 2u];
        p.r = 0.90f * t; p.g = 0.40f * t; p.b = 0.15f * t;
        p.size = 0.20f + 0.55f * t;
        frame.flux.push_back(p);
    }
    for (std::size_t i = 0; i < n; ++i) {  // R (cool)
        if (mag_r[i] < thr) continue;
        const float t = mag_r[i] / max_val;
        NativeParticle p;
        p.x = s.positions[i * 3u]; p.y = s.positions[i * 3u + 1u]; p.z = s.positions[i * 3u + 2u];
        p.r = 0.30f * t; p.g = 0.20f * t; p.b = 0.90f * t;
        p.size = 0.20f + 0.55f * t;
        frame.flux.push_back(p);
    }
}

// Dense |J| band overlays → sprite points. Samples FluxVector at stride 1 (the
// compacted, near-zero-free per-voxel magnitude buffer — reused for both bands)
// and keeps voxels inside the overlay's absolute |J| band:
//   DmHalo:  0.003 < |J| < K_GENESIS   (sub-threshold flux envelope);
//   Genesis: |J| ≈ K_GENESIS ± K_GENESIS·0.15  (genesis-frontier shell).
// K_GENESIS from the ontic chain (never hardcoded).
void append_dense_band_points(RenderBridge& rb, NativeFrame& frame, const OverlayDescriptor& d) {
    VisualFieldSample s;
    rb.copy_visual_field_sample(VisualFieldKind::FluxVector, /*stride=*/1, s);
    if (s.components != 3u || s.count() == 0) return;
    const float k_gen = static_cast<float>(ftd::K_GENESIS);
    const float band = k_gen * 0.15f;
    const std::size_t n = s.count();
    constexpr std::size_t kMaxBandPoints = 20000;
    std::size_t emitted = 0;
    frame.flux.reserve(frame.flux.size() + std::min(n, kMaxBandPoints));
    for (std::size_t i = 0; i < n && emitted < kMaxBandPoints; ++i) {
        const float x = s.data[i * 3u], y = s.data[i * 3u + 1u], z = s.data[i * 3u + 2u];
        const float m = std::sqrt(x * x + y * y + z * z);
        NativeParticle p;
        bool keep = false;
        float t = 0.0f;
        if (d.derive == OverlayDerive::DmHalo) {
            if (m > 0.003f && m < k_gen) {
                keep = true; t = m / k_gen;
                p.r = 0.30f + 0.40f * t; p.g = 0.10f + 0.15f * t; p.b = 0.50f + 0.40f * t;
            }
        } else {  // Genesis shell
            const float dist = std::abs(m - k_gen);
            if (dist < band && m > 0.01f) {
                keep = true; t = 1.0f - dist / band;
                p.r = 0.15f + 0.15f * t; p.g = 0.70f + 0.30f * t; p.b = 0.20f + 0.15f * t;
            }
        }
        if (!keep) continue;
        p.x = s.positions[i * 3u]; p.y = s.positions[i * 3u + 1u]; p.z = s.positions[i * 3u + 2u];
        p.size = 0.25f + 0.60f * t;
        frame.flux.push_back(p);
        ++emitted;
    }
}

// Flux Slice: |J| sprite points on the three lattice mid-planes (xy@z=L/2,
// xz@y=L/2, yz@x=L/2). Samples FluxVector at stride 1 so the exact mid-plane
// indices are represented, keeps voxels on any mid-plane, and colours them with
// the shared flux-cloud ramp at the shared Flux-Volume threshold.
void append_flux_slice(RenderBridge& rb, NativeFrame& frame, const OverlayDescriptor& d) {
    const int L = rb.lattice().size();
    VisualFieldSample s;
    rb.copy_visual_field_sample(VisualFieldKind::FluxVector, /*stride=*/1, s);
    if (s.components != 3u || s.count() == 0) return;
    const int mid = L / 2;
    const std::size_t n = s.count();
    std::vector<float> mag(n);
    float max_mag = 1.0e-6f;
    for (std::size_t i = 0; i < n; ++i) {
        const float x = s.data[i * 3u], y = s.data[i * 3u + 1u], z = s.data[i * 3u + 2u];
        mag[i] = std::sqrt(x * x + y * y + z * z);
        max_mag = std::max(max_mag, mag[i]);
    }
    frame.flux.reserve(frame.flux.size() + n);
    for (std::size_t i = 0; i < n; ++i) {
        const int ix = static_cast<int>(std::floor(s.positions[i * 3u]));
        const int iy = static_cast<int>(std::floor(s.positions[i * 3u + 1u]));
        const int iz = static_cast<int>(std::floor(s.positions[i * 3u + 2u]));
        if (ix != mid && iy != mid && iz != mid) continue;
        const float t = mag[i] / max_mag;
        if (t < d.threshold) continue;
        NativeParticle p;
        p.x = s.positions[i * 3u]; p.y = s.positions[i * 3u + 1u]; p.z = s.positions[i * 3u + 2u];
        flux_cloud_color(t, p.r, p.g, p.b);
        p.size = 0.18f + 0.55f * t;
        frame.flux.push_back(p);
    }
}

// Phase φ: one short oriented needle per voxel, along Ĵ (length ≈ 1.2),
// cyclic-HSL coloured by the flux azimuth arg = atan2(Jy, Jx).
//
// ⚠ [PROXY LIMITATION] The FTD phase is arg(J_L + i·J_R). Under the scalar
// (1±δ)/2 dual proxy J_L and J_R are COLLINEAR with J, so that phase is
// spatially CONSTANT and carries no per-voxel structure — it is meaningful only
// with a true (pseudovector) dual split (PLAN §4; the web computePhaseFrame
// returns 0 without dual). We therefore colour by the geometric flux azimuth —
// the only per-voxel angle the proxy exposes — so the overlay is faithful to
// the "oriented needle, cyclic-HSL by arg" spec while not pretending to show
// the (unavailable) true chiral phase.
void append_phase_needles(RenderBridge& rb, NativeFrame& frame, const OverlayDescriptor& d) {
    const int L = rb.lattice().size();
    const int stride = overlay_stride(L);
    VisualFieldSample s;
    rb.copy_visual_field_sample(VisualFieldKind::FluxVector, stride, s);
    if (s.components != 3u || s.count() == 0) return;
    const std::size_t n = s.count();
    std::vector<float> mag(n);
    float max_mag = 1.0e-9f;
    for (std::size_t i = 0; i < n; ++i) {
        const float x = s.data[i * 3u], y = s.data[i * 3u + 1u], z = s.data[i * 3u + 2u];
        mag[i] = std::sqrt(x * x + y * y + z * z);
        max_mag = std::max(max_mag, mag[i]);
    }
    constexpr float kHalfLen = 0.6f;  // total needle length ≈ 1.2 voxels
    constexpr float kPi = 3.14159265358979323846f;
    frame.field_lines.reserve(frame.field_lines.size() + n);
    for (std::size_t i = 0; i < n; ++i) {
        const float t = mag[i] / max_mag;
        if (t < d.threshold) continue;
        const float jx = s.data[i * 3u], jy = s.data[i * 3u + 1u], jz = s.data[i * 3u + 2u];
        const float inv = 1.0f / mag[i];
        const float hx = jx * inv, hy = jy * inv, hz = jz * inv;
        float r, g, b;
        ramp_cyclic_hsl((std::atan2(jy, jx) + kPi) / (2.0f * kPi), r, g, b);
        const float bx = s.positions[i * 3u], by = s.positions[i * 3u + 1u], bz = s.positions[i * 3u + 2u];
        NativeLine line;
        line.x0 = bx - hx * kHalfLen; line.y0 = by - hy * kHalfLen; line.z0 = bz - hz * kHalfLen;
        line.x1 = bx + hx * kHalfLen; line.y1 = by + hy * kHalfLen; line.z1 = bz + hz * kHalfLen;
        line.r0 = r; line.g0 = g; line.b0 = b;
        line.r1 = r; line.g1 = g; line.b1 = b;
        frame.field_lines.push_back(line);
    }
}

// Damping zones: 12 wireframe-box edges (a 3-voxel cube, half-extent 1.5)
// around each manifested particle, red, capped at ~1200 segments.
void append_damping_boxes(NativeFrame& frame) {
    static const int edges[12][6] = {
        {0, 0, 0, 1, 0, 0}, {0, 1, 0, 1, 1, 0}, {0, 0, 1, 1, 0, 1}, {0, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1, 0}, {1, 0, 0, 1, 1, 0}, {0, 0, 1, 0, 1, 1}, {1, 0, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 1}, {1, 0, 0, 1, 0, 1}, {0, 1, 0, 0, 1, 1}, {1, 1, 0, 1, 1, 1},
    };
    constexpr std::size_t kMaxSegments = 1200;
    std::size_t seg = 0;
    for (const NativeParticle& part : frame.particles) {
        if (seg + 12u > kMaxSegments) break;
        const float cx = part.x, cy = part.y, cz = part.z;  // voxel-centre already
        for (const auto& e : edges) {
            NativeLine line;
            line.x0 = cx - 1.5f + e[0] * 3.0f;
            line.y0 = cy - 1.5f + e[1] * 3.0f;
            line.z0 = cz - 1.5f + e[2] * 3.0f;
            line.x1 = cx - 1.5f + e[3] * 3.0f;
            line.y1 = cy - 1.5f + e[4] * 3.0f;
            line.z1 = cz - 1.5f + e[5] * 3.0f;
            line.r0 = 0.80f; line.g0 = 0.20f; line.b0 = 0.20f;
            line.r1 = 0.80f; line.g1 = 0.20f; line.b1 = 0.20f;
            frame.field_lines.push_back(line);
            ++seg;
        }
    }
}

// Confinement strings: a link segment between each particle pair with
// 1 < r < √120 (r² ∈ (1, CONFINEMENT_PAIR_DIST2)), coloured by separation
// direction (port of updateConfinementStrings). Pair scan is spatial-hashed
// (cell = √threshold) so each particle only tests its 27-cell neighbourhood.
void append_confinement_links(NativeFrame& frame) {
    const std::size_t count = frame.particles.size();
    if (count < 2) return;
    constexpr float kDist2 = 120.0f;  // CONFINEMENT_PAIR_DIST2 (web constant)
    constexpr std::size_t kMaxSegments = 1500;
    const float cell = std::sqrt(kDist2);
    // Cell coords are non-negative and bounded (L ≤ 256 ⇒ coord < 24 < 1024),
    // so pack them collision-free into one key (exact, no hash aliasing).
    auto cell_key = [](int cx, int cy, int cz) -> std::int64_t {
        return (static_cast<std::int64_t>(cx) * 1024 + cy) * 1024 + cz;
    };
    std::unordered_map<std::int64_t, std::vector<int>> buckets;
    buckets.reserve(count);
    for (int p = 0; p < static_cast<int>(count); ++p) {
        const NativeParticle& pt = frame.particles[static_cast<std::size_t>(p)];
        buckets[cell_key(static_cast<int>(std::floor(pt.x / cell)),
                         static_cast<int>(std::floor(pt.y / cell)),
                         static_cast<int>(std::floor(pt.z / cell)))].push_back(p);
    }
    std::size_t seg = 0;
    for (int i = 0; i < static_cast<int>(count) && seg < kMaxSegments; ++i) {
        const NativeParticle& pi = frame.particles[static_cast<std::size_t>(i)];
        const float xi = pi.x, yi = pi.y, zi = pi.z;
        const int cix = static_cast<int>(std::floor(xi / cell));
        const int ciy = static_cast<int>(std::floor(yi / cell));
        const int ciz = static_cast<int>(std::floor(zi / cell));
        for (int ax = -1; ax <= 1 && seg < kMaxSegments; ++ax)
        for (int ay = -1; ay <= 1 && seg < kMaxSegments; ++ay)
        for (int az = -1; az <= 1 && seg < kMaxSegments; ++az) {
            const auto it = buckets.find(cell_key(cix + ax, ciy + ay, ciz + az));
            if (it == buckets.end()) continue;
            for (int j : it->second) {
                if (j <= i) continue;  // each unordered pair emitted once
                const NativeParticle& pj = frame.particles[static_cast<std::size_t>(j)];
                const float dx = pj.x - xi, dy = pj.y - yi, dz = pj.z - zi;
                const float r2 = dx * dx + dy * dy + dz * dz;
                if (r2 <= 1.0f || r2 >= kDist2) continue;
                if (seg >= kMaxSegments) break;
                const float t = r2 / kDist2;
                const float alpha = 1.0f - t * 0.4f;
                const float inv_r = 1.0f / std::sqrt(r2);
                const float cr = std::abs(dx) * inv_r * alpha + 0.2f;
                const float cg = std::abs(dy) * inv_r * alpha + 0.2f;
                const float cb = std::abs(dz) * inv_r * alpha + 0.2f;
                NativeLine line;
                line.x0 = xi; line.y0 = yi; line.z0 = zi;
                line.x1 = pj.x; line.y1 = pj.y; line.z1 = pj.z;
                line.r0 = cr; line.g0 = cg; line.b0 = cb;
                line.r1 = cr; line.g1 = cg; line.b1 = cb;
                frame.field_lines.push_back(line);
                ++seg;
            }
        }
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

// ── Tranche-5 rubber-sheet builder ──────────────────────────────────────────
//
// One shared CPU pipeline for all six sheet overlays (Φ potential · EM energy ·
// Charge ρ · Vorticity ω · P_E · P_B). Ports the web _scatterHeights pipeline
// (topology-sheet-renderer.js): derive the per-voxel scalar → bilinear-splat it
// onto a small 2D grid over the XZ plane → 2-pass separable box-blur → build a
// ~40×40 PlaneGeometry at the overlay's y_frac height → deform Y by the (signed)
// blurred scalar × depth_frac·N → colour each vertex by the overlay's ramp.
// Emits one NativeSheet into frame.field_sheets (drawn by the presenter's sheet
// PSO + wireframe). O(count + gridN² + verts) — cheap even with several sheets.

// The per-voxel scalar the sheet's height is built from: sample positions
// (xyz voxel centres), values (one per position), the height normalizer (t =
// value/normalizer), and whether the value is signed (deform + colour keep the
// sign) or non-negative (clamped to [0,1]).
struct SheetScalar {
    std::vector<float> positions;
    std::vector<float> values;
    float              normalizer = 1.0f;
    bool               is_signed = false;
    bool               ok = false;
    // The field sample's effective Y spacing — sets the slice-slab half-width so
    // the two sampled Y-layers bracketing the slice plane interpolate linearly.
    int                effective_stride = 1;
};

// Derive the sheet's per-voxel scalar for one overlay's OverlayDerive selector.
SheetScalar derive_sheet_scalar(RenderBridge& rb, const OverlayDescriptor& d) {
    SheetScalar out;
    const int L = rb.lattice().size();
    const int stride = std::max(1, (L + 31) / 32);
    // c = C_SPEED from the ontic constant chain (never hardcoded); the magnetic
    // channel carries c² (matches diagnostics_compute.cpp's Hamiltonian units).
    const float c2 = static_cast<float>(ftd::C_SPEED * ftd::C_SPEED);

    // EM energy density u = ½|E|² + (c²/2)|B|². E and B are compacted
    // INDEPENDENTLY by the sampler (each drops sub-floor voxels), so pair them
    // by voxel position — not raw index — over the union of both fields' sites.
    if (d.derive == OverlayDerive::SheetEmEnergy) {
        VisualFieldSample eF, bF;
        rb.copy_visual_field_sample(VisualFieldKind::Electric, stride, eF);
        rb.copy_visual_field_sample(VisualFieldKind::Magnetic, stride, bF);
        const bool haveE = eF.components == 3u && eF.count() > 0;
        const bool haveB = bF.components == 3u && bF.count() > 0;
        if (!haveE && !haveB) return out;
        auto vox_key = [L](float px, float py, float pz) -> std::int64_t {
            const std::int64_t ix = static_cast<std::int64_t>(std::floor(px));
            const std::int64_t iy = static_cast<std::int64_t>(std::floor(py));
            const std::int64_t iz = static_cast<std::int64_t>(std::floor(pz));
            return (ix * L + iy) * L + iz;
        };
        std::unordered_map<std::int64_t, float> b2_at;
        if (haveB) {
            b2_at.reserve(bF.count());
            for (std::size_t i = 0; i < bF.count(); ++i) {
                const float x = bF.data[i * 3u], y = bF.data[i * 3u + 1u], z = bF.data[i * 3u + 2u];
                b2_at[vox_key(bF.positions[i * 3u], bF.positions[i * 3u + 1u],
                              bF.positions[i * 3u + 2u])] = x * x + y * y + z * z;
            }
        }
        float max_u = 1.0e-9f;
        std::unordered_map<std::int64_t, bool> seen;
        auto emit = [&](const VisualFieldSample& f, bool is_e) {
            for (std::size_t i = 0; i < f.count(); ++i) {
                const std::int64_t key = vox_key(f.positions[i * 3u], f.positions[i * 3u + 1u],
                                                 f.positions[i * 3u + 2u]);
                if (seen.count(key)) continue;
                seen[key] = true;
                float e2 = 0.0f, b2 = 0.0f;
                if (is_e) {
                    const float x = f.data[i * 3u], y = f.data[i * 3u + 1u], z = f.data[i * 3u + 2u];
                    e2 = x * x + y * y + z * z;
                    const auto it = b2_at.find(key);
                    if (it != b2_at.end()) b2 = it->second;
                } else {
                    const float x = f.data[i * 3u], y = f.data[i * 3u + 1u], z = f.data[i * 3u + 2u];
                    b2 = x * x + y * y + z * z;  // E-only voxels already emitted
                }
                const float u = 0.5f * e2 + 0.5f * c2 * b2;
                out.positions.push_back(f.positions[i * 3u]);
                out.positions.push_back(f.positions[i * 3u + 1u]);
                out.positions.push_back(f.positions[i * 3u + 2u]);
                out.values.push_back(u);
                max_u = std::max(max_u, u);
            }
        };
        if (haveE) emit(eF, true);
        if (haveB) emit(bF, false);
        out.normalizer = max_u;
        out.is_signed = false;
        out.effective_stride = std::max(haveE ? eF.effective_stride : 1,
                                        haveB ? bF.effective_stride : 1);
        out.ok = !out.values.empty();
        return out;
    }

    // Charge ρ = ∇·J (signed) and Vorticity |∇×J| (unsigned) are 1-component
    // scalar fields sampled directly.
    if (d.derive == OverlayDerive::SheetCharge || d.derive == OverlayDerive::SheetVorticity) {
        VisualFieldSample s;
        rb.copy_visual_field_sample(d.kind, stride, s);
        if (s.components != 1u || s.count() == 0) return out;
        const bool sgn = (d.derive == OverlayDerive::SheetCharge);
        float max_abs = 1.0e-9f;
        out.positions = s.positions;
        out.values.resize(s.count());
        for (std::size_t i = 0; i < s.count(); ++i) {
            out.values[i] = s.data[i];
            max_abs = std::max(max_abs, std::abs(s.data[i]));
        }
        out.normalizer = max_abs;
        out.is_signed = sgn;
        out.effective_stride = s.effective_stride;
        out.ok = true;
        return out;
    }

    // The remaining sheets derive from a single 3-vector field:
    //   SheetGravPotential: −|J|² from FluxVector (signed, wells dip);
    //   SheetEPressure:     ½|E|²  from Electric  (unsigned);
    //   SheetBPressure:     (c²/2)|B|² from Magnetic (unsigned).
    VisualFieldSample s;
    rb.copy_visual_field_sample(d.kind, stride, s);
    if (s.components != 3u || s.count() == 0) return out;
    const std::size_t n = s.count();
    out.positions = s.positions;
    out.values.resize(n);
    float max_abs = 1.0e-9f;
    for (std::size_t i = 0; i < n; ++i) {
        const float x = s.data[i * 3u], y = s.data[i * 3u + 1u], z = s.data[i * 3u + 2u];
        const float m2 = x * x + y * y + z * z;
        float v = 0.0f;
        switch (d.derive) {
            case OverlayDerive::SheetGravPotential: v = -m2; break;          // signed
            case OverlayDerive::SheetEPressure:     v = 0.5f * m2; break;     // ½|E|²
            case OverlayDerive::SheetBPressure:     v = 0.5f * c2 * m2; break; // (c²/2)|B|²
            default:                                v = m2; break;
        }
        out.values[i] = v;
        max_abs = std::max(max_abs, std::abs(v));
    }
    out.normalizer = max_abs;
    out.is_signed = (d.derive == OverlayDerive::SheetGravPotential);
    out.effective_stride = s.effective_stride;
    out.ok = true;
    return out;
}

// Build one rubber sheet for overlay `d`, sliced at `height` (fraction of the
// lattice box), and append it to frame.field_sheets. The sheet is a 2-D slice
// of the field on the horizontal y = height·L plane: the base plane sits at that
// height AND the per-(x,z) scalar comes from that y-level (linearly interpolated
// in y across the sampled layers), so sweeping `height` re-slices the field AND
// repositions the surface. `normalizer` is the GLOBAL (all-y) field max, so a
// quiet slice reads flat and a high-energy slice deforms strongly — that is how
// the surface reveals the energy at successive levels.
void build_sheet(RenderBridge& rb, NativeFrame& frame, const OverlayDescriptor& d,
                 float height) {
    const SheetScalar sc = derive_sheet_scalar(rb, d);
    if (!sc.ok) return;
    const int N = rb.lattice().size();
    if (N < 2) return;
    const float denom = std::max(sc.normalizer, 1.0e-9f);
    const float inv_denom = 1.0f / denom;

    // The slice plane in world-y, and the y-slab half-width. Samples within
    // ±slab_half of the plane contribute, weighted linearly by y-proximity — a
    // triangular kernel of half-width = the sample's y spacing, so the two
    // sampled y-layers bracketing the plane interpolate linearly (trilinear-in-y
    // at the plane; bilinear-in-(x,z) from the splat below).
    const float h = std::clamp(height, 0.0f, 0.999f);
    const float target_y = static_cast<float>(N) * h;
    const float slab_half = std::max(1.0f, static_cast<float>(sc.effective_stride));

    // 1. Slice + scatter: bilinear-splat only the near-plane samples onto a small
    //    2D (x,z) grid, y-weighted, then normalise by the accumulated weight
    //    (unsampled cells stay 0 and are filled by the blur). gridN ~ 1 voxel/cell.
    const int gridN = std::max(16, std::min(N, 48));
    const int G2 = gridN * gridN;
    std::vector<float> grid(static_cast<std::size_t>(G2), 0.0f);
    std::vector<float> weight(static_cast<std::size_t>(G2), 0.0f);
    std::vector<float> tmp(static_cast<std::size_t>(G2), 0.0f);
    const float scale = static_cast<float>(gridN - 1) / static_cast<float>(N);
    const std::size_t count = sc.values.size();
    for (std::size_t i = 0; i < count; ++i) {
        const float py = sc.positions[i * 3u + 1u];
        const float wy = std::abs(py - target_y);
        if (wy >= slab_half) continue;            // not on this y-slice
        const float yw = 1.0f - wy / slab_half;   // linear y kernel (peak on the plane)
        const float sx = sc.positions[i * 3u] * scale;
        const float sz = sc.positions[i * 3u + 2u] * scale;
        if (sx < 0.0f || sx >= gridN - 1 || sz < 0.0f || sz >= gridN - 1) continue;
        const int xi = static_cast<int>(sx), zi = static_cast<int>(sz);
        const float xf = sx - xi, zf = sz - zi;
        const float v = sc.values[i];
        const float w00 = (1 - xf) * (1 - zf) * yw, w01 = xf * (1 - zf) * yw;
        const float w10 = (1 - xf) * zf * yw, w11 = xf * zf * yw;
        const int row0 = zi * gridN + xi;
        const int row1 = row0 + gridN;
        grid[row0]     += v * w00; weight[row0]     += w00;
        grid[row0 + 1] += v * w01; weight[row0 + 1] += w01;
        grid[row1]     += v * w10; weight[row1]     += w10;
        grid[row1 + 1] += v * w11; weight[row1 + 1] += w11;
    }
    for (int i = 0; i < G2; ++i)
        if (weight[i] > 1.0e-9f) grid[i] /= weight[i];

    // 2. Separable 3-tap box-blur, 2 passes (interior-only; edges pin boundaries).
    for (int p = 0; p < 2; ++p) {
        for (int z = 0; z < gridN; ++z) {
            const int rowBase = z * gridN;
            tmp[rowBase] = grid[rowBase];
            tmp[rowBase + gridN - 1] = grid[rowBase + gridN - 1];
            for (int x = 1; x < gridN - 1; ++x)
                tmp[rowBase + x] = (grid[rowBase + x - 1] + grid[rowBase + x]
                                    + grid[rowBase + x + 1]) * (1.0f / 3.0f);
        }
        for (int x = 0; x < gridN; ++x) {
            grid[x] = tmp[x];
            grid[(gridN - 1) * gridN + x] = tmp[(gridN - 1) * gridN + x];
        }
        for (int z = 1; z < gridN - 1; ++z) {
            const int rowPrev = (z - 1) * gridN, rowCurr = z * gridN, rowNext = (z + 1) * gridN;
            for (int x = 0; x < gridN; ++x)
                grid[rowCurr + x] = (tmp[rowPrev + x] + tmp[rowCurr + x]
                                     + tmp[rowNext + x]) * (1.0f / 3.0f);
        }
    }

    // 3. Build the PlaneGeometry (segments+1)² grid at the SLICE height target_y,
    //    spanning N·0.95 centred at (N/2, ·, N/2). Deform Y by t·DEPTH, colour by
    //    the ramp. The base plane rides at target_y, so moving the height both
    //    re-slices (step 1) and repositions the surface.
    const int segments = std::max(24, std::min(N, 40));
    const int side = segments + 1;
    const float span = static_cast<float>(N) * 0.95f;
    const float half_span = span * 0.5f;
    const float centre = static_cast<float>(N) * 0.5f;
    const float y_world = target_y;
    const float depth = static_cast<float>(N) * d.depth_frac;
    const float grid_max = static_cast<float>(gridN - 1);
    // Constant translucent-sheet opacity (matches the web solid sheet's 0.45–0.55).
    constexpr float kSheetAlpha = 0.55f;

    NativeSheet sheet;
    sheet.vertices.reserve(static_cast<std::size_t>(side) * side);
    for (int j = 0; j < side; ++j) {
        const float fz = static_cast<float>(j) / static_cast<float>(segments);   // 0..1
        const float world_z = centre - half_span + fz * span;
        for (int i = 0; i < side; ++i) {
            const float fx = static_cast<float>(i) / static_cast<float>(segments);
            const float world_x = centre - half_span + fx * span;
            // Bilinear lookup into the blurred grid at this vertex's world XZ.
            float gx = world_x * scale, gz = world_z * scale;
            gx = std::clamp(gx, 0.0f, grid_max);
            gz = std::clamp(gz, 0.0f, grid_max);
            const int xi = static_cast<int>(gx), zi = static_cast<int>(gz);
            const float xf = gx - xi, zf = gz - zi;
            const int xi1 = xi < gridN - 1 ? xi + 1 : xi;
            const int zi1 = zi < gridN - 1 ? zi + 1 : zi;
            const float v00 = grid[zi * gridN + xi],  v01 = grid[zi * gridN + xi1];
            const float v10 = grid[zi1 * gridN + xi], v11 = grid[zi1 * gridN + xi1];
            const float blended = (1 - xf) * (1 - zf) * v00 + xf * (1 - zf) * v01
                                + (1 - xf) * zf * v10 + xf * zf * v11;
            float t = blended * inv_denom;
            t = sc.is_signed ? std::clamp(t, -1.0f, 1.0f) : std::clamp(t, 0.0f, 1.0f);
            NativeSheetVertex vtx;
            vtx.x = world_x;
            vtx.y = y_world + t * depth;
            vtx.z = world_z;
            sheet_vertex_color(d.ramp, t, vtx.r, vtx.g, vtx.b);
            vtx.a = kSheetAlpha;
            sheet.vertices.push_back(vtx);
        }
    }
    // Two triangles per quad, CCW; the PSO culls nothing so winding is cosmetic.
    sheet.indices.reserve(static_cast<std::size_t>(segments) * segments * 6u);
    for (int j = 0; j < segments; ++j) {
        for (int i = 0; i < segments; ++i) {
            const std::uint32_t a = static_cast<std::uint32_t>(j * side + i);
            const std::uint32_t b = a + 1u;
            const std::uint32_t c = a + static_cast<std::uint32_t>(side);
            const std::uint32_t dd = c + 1u;
            sheet.indices.push_back(a); sheet.indices.push_back(c); sheet.indices.push_back(b);
            sheet.indices.push_back(b); sheet.indices.push_back(c); sheet.indices.push_back(dd);
        }
    }
    frame.field_sheets.push_back(std::move(sheet));
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

void Scale0Adapter::boot(const ScenarioMeta& meta, const RunConfig& cfg,
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
    // Harness injections AND the view-state overlay/sheet-height commands all
    // change what the next frame should draw, so all three flag "refresh the
    // frame" — this is what makes toggling an overlay or sweeping a sheet's
    // height update live even while the sim is PAUSED (the host re-captures on a
    // host write). They never mutate the RenderBridge (apply() short-circuits
    // SetOverlay/SetSheetHeight), so this only drives the re-capture gate.
    if (std::holds_alternative<SetOverlay>(*s0)
        || std::holds_alternative<SetSheetHeight>(*s0))
        return true;
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
    // SetSheetHeight is adapter view-state only (the slice height of one active
    // sheet); like SetOverlay it never mutates the RenderBridge, so update the
    // height and short-circuit.
    if (const SetSheetHeight* sh = std::get_if<SetSheetHeight>(s0)) {
        set_sheet_height(static_cast<OverlayId>(sh->overlay_id), sh->height);
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

    // Color-charge overlay recolours the particle sprites in place by the
    // genesis-assigned colour axis carried per particle (VisualParticleRecord::
    // color ∈ {0 colorless, 1 red, 2 green, 3 blue} — argmax|J_axis| at genesis).
    // No engine change and no extra snapshot field: the colour is already in the
    // record. It does NOT suppress the ambient flux cloud (web special-cases
    // showColorCharge outside anyFieldActive — see the compositing gate below).
    const bool color_charge = overlay_active(OverlayId::ColorCharge);
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
        if (color_charge && rec.color != 0) {
            switch (rec.color) {
                case 1: p.r = 0.90f; p.g = 0.30f; p.b = 0.30f; break;  // red
                case 2: p.r = 0.30f; p.g = 0.85f; p.b = 0.35f; break;  // green
                case 3: p.r = 0.35f; p.g = 0.45f; p.b = 0.95f; break;  // blue
                default: break;                                        // colorless: keep sign
            }
        }
        p.size = 0.55f;
        frame.particles.push_back(p);
    }
    // Overlay compositing. The ambient flux cloud shows when no GEOMETRY-emitting
    // overlay is active — i.e. the active set is empty OR holds only the Recolor
    // overlay (Color charge), which recolours particles without adding geometry
    // (mirrors the web `anyFieldActive` gate, where showColorCharge is special-
    // cased outside it). Otherwise composite EVERY active overlay into this frame
    // — each sampled once (O(active overlays)) — appending arrows/lines into
    // field_lines and points into flux. Groups coexist.
    bool any_geometry = false;
    for (const OverlayId id : active_overlays_) {
        const OverlayDescriptor* d = overlay_by_id(static_cast<std::uint32_t>(id));
        if (d && d->render != OverlayRender::Recolor) { any_geometry = true; break; }
    }
    if (!any_geometry) {
        append_flux(*bridge_, frame);
    }
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
            case OverlayRender::DerivedPoints:
                append_overlay_derived_points(*bridge_, frame, *d);
                break;
            case OverlayRender::DualPoints:
                append_dual_flux(*bridge_, frame, *d);
                break;
            case OverlayRender::DenseBand:
                append_dense_band_points(*bridge_, frame, *d);
                break;
            case OverlayRender::FluxSlice:
                append_flux_slice(*bridge_, frame, *d);
                break;
            case OverlayRender::PhaseNeedles:
                append_phase_needles(*bridge_, frame, *d);
                break;
            case OverlayRender::DampingBoxes:
                append_damping_boxes(frame);
                break;
            case OverlayRender::PairLinks:
                append_confinement_links(frame);
                break;
            case OverlayRender::Sheet:
                build_sheet(*bridge_, frame, *d, sheet_height_frac(id, *d));
                break;
            case OverlayRender::Recolor:
                break;  // handled in the particle loop above
        }
    }

    // Expose each active sheet's current slice height so the panel reflects a
    // CLI-set / runtime-adjusted height (adapter-authoritative).
    for (const OverlayId id : active_overlays_) {
        const OverlayDescriptor* d = overlay_by_id(static_cast<std::uint32_t>(id));
        if (!d || d->render != OverlayRender::Sheet) continue;
        frame.sheet_heights.push_back(
            NativeSheetHeight{static_cast<std::uint32_t>(id), sheet_height_frac(id, *d)});
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
        // Seed a Sheet overlay's slice height to its registry y_frac default on
        // first activation (emplace never overwrites a value a prior
        // SetSheetHeight already set — e.g. a CLI --sheet-height stamped first).
        const OverlayDescriptor* d = overlay_by_id(static_cast<std::uint32_t>(id));
        if (d && d->render == OverlayRender::Sheet)
            sheet_height_.emplace(static_cast<std::uint32_t>(id), d->y_frac);
    } else if (it != active_overlays_.end()) {
        active_overlays_.erase(it);
        sheet_height_.erase(static_cast<std::uint32_t>(id));  // forget on toggle-off
    }
}

void Scale0Adapter::set_sheet_height(OverlayId id, float height) {
    sheet_height_[static_cast<std::uint32_t>(id)] = std::clamp(height, 0.0f, 0.999f);
}

float Scale0Adapter::sheet_height_frac(OverlayId id, const OverlayDescriptor& d) const {
    const auto it = sheet_height_.find(static_cast<std::uint32_t>(id));
    return it != sheet_height_.end() ? it->second : d.y_frac;
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
