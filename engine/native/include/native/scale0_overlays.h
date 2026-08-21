#pragma once
//
// scale0_overlays.h — the data-driven Scale-0 overlay registry.
//
// The web dashboard composites MANY Scale-0 field/phenomena overlays at once,
// grouped into 7 menu columns (Volume · Fields · Forces · Quantum · Topology ·
// Stress-Energy · Phenomena). This registry is the native mirror of that menu:
// one immutable descriptor per overlay, keyed by a stable OverlayId, carrying
// everything both consumers need —
//   • the Scale-0 adapter's capture(): how to turn the overlay into scene
//     geometry (which VisualFieldKind to sample, points-vs-arrows, colour ramp,
//     threshold, per-overlay quirks like ×DUAL_DELTA or a latency-horizon cut);
//   • the app's FIELDS panel + CLI: the stable name, human label, and column.
//
// Adding an overlay in a later tranche is a single row here (plus its ramp/logic
// in the adapter if genuinely new) — the panel groups by `column` automatically
// and the CLI resolves it by `name`, so no panel/CLI wiring changes are needed.
//
// This header names only ftd::VisualFieldKind (a visualization enum, freely
// named across the native UI). It does NOT name RenderBridge/TermToggles/any
// Scale-0 command or snapshot struct, so it stays outside the adapter's
// concrete-type boundary and can be shared by the app layer.
//
#include "ftd/visual_field_sample.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace ftd::native {

// The 7 web overlay-menu columns, in menu order. `Count` bounds iteration.
enum class OverlayColumn : std::uint32_t {
    Volume = 0,
    Fields,
    Forces,
    Quantum,
    Topology,
    StressEnergy,
    Phenomena,
    Count,
};

// How an overlay's samples become scene geometry through the presenter's two
// existing primitives (sprite point cloud + line list).
enum class OverlayRender : std::uint32_t {
    Sprite,      // ambient |J| flux cloud (FluxVector magnitude) → sprite points
    Points,      // scalar field → magnitude/sign-coloured sprite points
    Arrows,      // 3-vector field → line-segment arrows (dim base → bright tip)
    Streamline,  // 3-vector field → RK4-traced field lines (LINE PSO polylines)
    // ── Tranche 2 (EXTEND): reuse the SAME two primitives (sprite points +
    //    line list), no new PSO. The formula/band each needs is named by the
    //    descriptor's `derive` selector where several overlays share a path.
    DerivedPoints,  // per-voxel DERIVED scalar → sprite points (|ψ|² · ℒ · Entropy · Chirality)
    DualPoints,     // amplitude split J_L=J(1+δ)/2, J_R=J(1−δ)/2 → two coloured point sets
    DenseBand,      // dense stride-1 |J| magnitude, band-select → sprite points (DM Halo · Genesis)
    FluxSlice,      // |J| sprite points on the 3 lattice mid-planes (xy@z=L/2, xz@y=L/2, yz@x=L/2)
    PhaseNeedles,   // per-voxel short oriented needle along Ĵ → line list (cyclic-HSL by arg)
    DampingBoxes,   // 12 wireframe-box edges (3-voxel cube) around each particle → line list
    PairLinks,      // qualifying particle-pair link segments (1<r<√120) → line list
    Recolor,        // recolour existing particle sprites in place (emits no new geometry)
    // ── Tranche 5 (NEW): rubber-sheet surfaces (triangle-mesh vertex-colour PSO)
    //    — the app's first non-billboard surface. A deformed ~40×40 grid whose
    //    Y is displaced by a per-voxel scalar (scattered → box-blurred → sampled)
    //    and whose vertices are ramp-coloured. Emits into NativeFrame.field_sheets.
    Sheet,          // deformable rubber sheet (Φ · EM energy · Charge ρ · Vorticity · P_E · P_B)
    // ── Knot Zones (the 33rd overlay) ──
    //    Traces E and B streamlines internally, clusters where field lines bunch
    //    (density grid → 26-neighbour flood fill, the web field-line-knots.js
    //    default gate), and emits one wireframe box per knot (E-family and
    //    B-family, per-knot hue) through the LINE PSO. Depends on the streamline
    //    integrator. Emits into NativeFrame.field_lines like the other line overlays.
    KnotZones,
};

// Force render-style — a SINGLE global setting that applies to all four Force
// overlays (EM · Gravity · Strong · ∇×J), mirroring the web force-style
// selector. Only the Forces-column overlays honour it; every other overlay is
// unaffected. Arrows is the default (== the descriptor's OverlayRender::Arrows).
enum class ForceStyle : std::uint32_t {
    Arrows = 0,  // base→tip line-segment arrows (the existing force path)
    Heatmap,     // gaussian additive sprite points, size ∝ log(|force|), per-force palette
    Flow,        // animated dashed RK4 streamlines seeded ∝ |force|
    Glyphs,      // instanced oriented cones (per-force palette, magnitude-scaled)
    Count,
};

inline const char* force_style_name(ForceStyle s) {
    switch (s) {
        case ForceStyle::Arrows:  return "Arrows";
        case ForceStyle::Heatmap: return "Heatmap";
        case ForceStyle::Flow:    return "Flow";
        case ForceStyle::Glyphs:  return "Glyphs";
        default:                  return "";
    }
}

// Colour ramp applied to the per-sample magnitude/sign.
enum class OverlayRamp : std::uint32_t {
    FluxCloud,   // ambient flux cloud (cool blue, brightening with |J|)
    Diverging,   // signed red(+)/blue(-) — divergence, Gauss residual
    StateSign,   // s=+1 red / s=-1 blue, saturated (void invisible)
    Latency,     // blue(low) → red(high)
    Horizon,     // dim ember red (dark, threshold-selected shell)
    CoolHot,     // magnitude cool→hot (force arrows)
    Poynting,    // yellow → orange (energy-flux arrows)
    Weak,        // violet (parity-odd ∇×J pseudovector arrows)
    // Tranche-2 point ramps (derived-scalar sprite points).
    Viridis,     // |ψ|²: approximate viridis (purple → teal → yellow), t∈[0,1]
    RdBu,        // ℒ Lagrangian: signed diverging red(+) / blue(−), t∈[-1,1]
    Grayscale,   // Entropy s: straight grayscale, t∈[0,1]
    Chirality,   // Chirality: signed red (L-dominant) / blue (R-dominant)
    CyclicHSL,   // Phase φ needles: full hue cycle by arg (S=1, L=0.5)
    // Streamline ramps: applied per-vertex inside the RK4 integrator (streamlines
    // .cpp), keyed off the overlay, not consulted by the point/arrow colour
    // switches. Named here so each streamline descriptor documents its own look.
    FluxLine,    // Flux Lines: flux colormap by LOCAL |J| (dark-blue→cyan→red)
    CyanFade,    // E Field: cyan (0.30,0.82,0.88) faded along the line
    GreenFade,   // B Field: green (0.40,0.73,0.42) faded along the line
    // Tranche-5 rubber-sheet ramps (per-vertex surface colour; ports of the
    // engine/web/js/viewport/color-ramps.js sheet ramps). Signed sheets pass the
    // signed height t∈[-1,1] (grav-well uses |t|); unsigned pass t∈[0,1].
    GravWell,    // Φ potential: peak yellow → deep blue well (coloured by |t|)
    EmEnergy,    // EM energy u: teal → warm orange, t∈[0,1]
    Charge,      // Charge ρ: diverging blue(sink) ↔ red(source), t∈[-1,1]
    Vorticity,   // Vorticity ω: magma near-black → violet → gold, t∈[0,1]
    EPressure,   // P_E: pale yellow → saturated red, t∈[0,1]
    BPressure,   // P_B: pale cyan → deep teal, t∈[0,1]
};

// Which derivation/band a DerivedPoints / DenseBand overlay runs. `None` for
// every other render mode (and value-initialised for the Tranche-1/4 rows that
// predate this field). The formula for each is documented at its adapter helper.
enum class OverlayDerive : std::uint32_t {
    None = 0,
    PsiSquared,   // |J|² (max-normalised)
    Lagrangian,   // ½|E|² − ½(∇·J)² (signed)
    Entropy,      // 4p(1−p), p=|J|/|J|max
    Chirality,    // |J|·δ (δ = DUAL_DELTA, canonical)
    DmHalo,       // dense |J| band 0.003 < |J| < K_GENESIS
    Genesis,      // dense |J| shell |J| ≈ K_GENESIS ± K_GENESIS·0.15
    // ── Tranche 5: rubber-sheet scalars. Each names the field(s) it derives its
    //    per-voxel height from; the builder pairs multi-field cases by voxel.
    SheetGravPotential,  // −|J|² from FluxVector (signed, wells dip)
    SheetEmEnergy,       // ½|E|² + (c²/2)|B|² from Electric+Magnetic (unsigned)
    SheetCharge,         // ∇·J from Divergence (signed, red hills / blue wells)
    SheetVorticity,      // |∇×J| from the Vorticity scalar field (unsigned)
    SheetEPressure,      // ½|E|² from Electric (unsigned)
    SheetBPressure,      // (c²/2)|B|² from Magnetic (unsigned)
};

// Stable overlay identifiers. The numeric value is the wire id carried by the
// SetOverlay command; NEVER renumber an existing entry (later tranches append).
enum class OverlayId : std::uint32_t {
    // ── Tranche 1: the 11 COVERED overlays ──
    FluxVolume = 0,
    Divergence,
    State,
    Poynting,
    EmForce,
    GravityForce,
    StrongForce,
    WeakCurl,
    Latency,
    GaussResidual,
    Horizon,
    // ── Tranche 4: the 3 STREAMLINE overlays (RK4 field-line integration) ──
    FluxLines,
    EField,
    BField,
    // ── Tranche 2: the EXTEND overlays that reuse sprite points + line list ──
    PsiSquared,      // Quantum  — |ψ|² viridis point cloud
    Phase,           // Quantum  — φ oriented needles
    Lagrangian,      // Quantum  — ℒ signed diverging points
    Entropy,         // Quantum  — s grayscale jitter points
    FluxSliceMid,    // Volume   — |J| points on the 3 lattice mid-planes
    Chirality,       // Phenomena — |J|·δ signed points
    DualFlux,        // Phenomena — J_L / J_R two-set points
    DarkMatterHalo,  // Phenomena — sub-threshold |J| band cloud
    Genesis,         // Phenomena — genesis-frontier |J| shell band
    ColorCharge,     // Phenomena — recolour particles by genesis colour axis
    Damping,         // Phenomena — wireframe boxes around particles
    Confinement,     // Phenomena — particle-pair link segments
    // ── Tranche 5: the 6 rubber-sheet overlays (triangle-mesh vertex-colour) ──
    GravPotential,   // Topology     — Φ = −|J|² rubber sheet (wells dip)
    EmEnergy,        // Topology     — u = ½|E|²+(c²/2)|B|² rubber sheet
    ChargeDensity,   // Topology     — ρ = ∇·J signed rubber sheet
    Vorticity,       // Topology     — ω = |∇×J| rubber sheet (thin band)
    EPressure,       // Stress-Energy — P_E = ½|E|² rubber sheet
    BPressure,       // Stress-Energy — P_B = (c²/2)|B|² rubber sheet
    // ── The 33rd overlay: field-line Knot Zones (Phenomena column) ──
    KnotZones,       // Phenomena     — wireframe boxes around E/B streamline knots
    // Future tranches append here (EXTEND / NEW overlays) — do not reorder.
};

// The 3-stop force palette (low/mid/high RGB) for one Force overlay — ports of
// the web FORCE_PALETTES (color-ramps.js), keyed by OverlayId. Used by the
// Heatmap/Flow/Glyphs styles (Arrows uses its own cool→hot ramp). EM=cyan,
// Gravity=amber, Strong=red, ∇×J("weak")=violet.
struct ForcePalette { float low[3], mid[3], high[3]; };

inline ForcePalette force_palette_for(OverlayId id) {
    switch (id) {
        case OverlayId::GravityForce:
            return {{0.4f, 0.2f, 0.0f}, {1.0f, 0.67f, 0.0f}, {1.0f, 1.0f, 0.6f}};
        case OverlayId::StrongForce:
            return {{0.4f, 0.0f, 0.05f}, {1.0f, 0.09f, 0.27f}, {1.0f, 0.7f, 0.7f}};
        case OverlayId::WeakCurl:
            return {{0.2f, 0.0f, 0.4f}, {0.67f, 0.0f, 1.0f}, {0.9f, 0.6f, 1.0f}};
        case OverlayId::EmForce:
        default:
            return {{0.0f, 0.2f, 0.4f}, {0.0f, 0.9f, 1.0f}, {0.7f, 1.0f, 1.0f}};
    }
}

// low/mid/high palette interpolator (port of lerpPalette, color-ramps.js): the
// first half blends low→mid, the second half mid→high.
inline void force_palette_lerp(const ForcePalette& p, float t, float& r, float& g, float& b) {
    t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    if (t < 0.5f) {
        const float u = t * 2.0f;
        r = p.low[0] + (p.mid[0] - p.low[0]) * u;
        g = p.low[1] + (p.mid[1] - p.low[1]) * u;
        b = p.low[2] + (p.mid[2] - p.low[2]) * u;
    } else {
        const float u = (t - 0.5f) * 2.0f;
        r = p.mid[0] + (p.high[0] - p.mid[0]) * u;
        g = p.mid[1] + (p.high[1] - p.mid[1]) * u;
        b = p.mid[2] + (p.high[2] - p.mid[2]) * u;
    }
}

struct OverlayDescriptor {
    OverlayId            id;
    const char*          name;    // stable id (panel + --overlays CLI)
    const char*          label;   // human label shown in the panel
    OverlayColumn        column;
    OverlayRender        render;
    ftd::VisualFieldKind kind;    // field sampled from the bridge
    OverlayRamp          ramp;
    // Keep a sample when its magnitude fraction of the frame max ≥ threshold,
    // UNLESS select_min ≥ 0, in which case keep it when the RAW value ≥
    // select_min (absolute — used by the Horizon L≥0.95 shell).
    float                threshold;
    float                select_min;
    bool                 scale_by_dual_delta;  // ∇×J "weak": ×DUAL_DELTA (canonical)
    bool                 force_stride1;         // State / Gauss sample every voxel
    // Which formula/band a DerivedPoints / DenseBand / Sheet overlay runs; None
    // (the value-initialised default) for every other render mode.
    OverlayDerive        derive = OverlayDerive::None;
    // Rubber-sheet (OverlayRender::Sheet) placement, mirroring the web
    // TOPOLOGY_CONFIGS: `y_frac` is the sheet's rest-plane height as a fraction
    // of the lattice box (world y = y_frac·N); `depth_frac` is the Y-deform
    // amplitude as a fraction of N (world Δy = t · depth_frac·N). 0 for every
    // non-sheet row (the value-initialised default).
    float                y_frac = 0.0f;
    float                depth_frac = 0.0f;
};

// The registry, in menu/column order. Defined once here (header-inline) so both
// the adapter and the app see the same table without a link dependency.
inline constexpr OverlayDescriptor kOverlayRegistry[] = {
    // ── Volume ──
    {OverlayId::FluxVolume,   "fluxVolume", "Flux Volume", OverlayColumn::Volume,
     OverlayRender::Sprite, ftd::VisualFieldKind::FluxVector,   OverlayRamp::FluxCloud, 0.04f, -1.0f, false, false},
    {OverlayId::FluxLines,    "fluxLines",  "Flux Lines",  OverlayColumn::Volume,
     OverlayRender::Streamline, ftd::VisualFieldKind::FluxVector, OverlayRamp::FluxLine, 0.0f, -1.0f, false, true},
    // Label ASCII-ized ("div J", not "∇·J"): the vendored Inter shell
    // font lacks U+2207 NABLA, so the glyph rendered as an empty box. No symbol
    // fallback face is vendored in the repo (do-not-download policy), so the
    // three ∇/ℒ overlay labels spell the operator in ASCII instead.
    {OverlayId::Divergence,   "divJ",       "div J", OverlayColumn::Volume,
     OverlayRender::Points, ftd::VisualFieldKind::Divergence,   OverlayRamp::Diverging, 0.01f, -1.0f, false, false},
    {OverlayId::State,        "state",      "State s",     OverlayColumn::Volume,
     OverlayRender::Points, ftd::VisualFieldKind::State,        OverlayRamp::StateSign, 0.50f, -1.0f, false, true},
    // ── Fields ──
    {OverlayId::EField,       "eField",     "E Field",     OverlayColumn::Fields,
     OverlayRender::Streamline, ftd::VisualFieldKind::Electric, OverlayRamp::CyanFade,  0.0f, -1.0f, false, true},
    {OverlayId::BField,       "bField",     "B Field",     OverlayColumn::Fields,
     OverlayRender::Streamline, ftd::VisualFieldKind::Magnetic, OverlayRamp::GreenFade, 0.0f, -1.0f, false, true},
    {OverlayId::Poynting,     "poynting",   "Poynting S",  OverlayColumn::Fields,
     OverlayRender::Arrows, ftd::VisualFieldKind::Poynting,     OverlayRamp::Poynting,  0.05f, -1.0f, false, false},
    // ── Forces ──
    {OverlayId::EmForce,      "emForce",    "EM",          OverlayColumn::Forces,
     OverlayRender::Arrows, ftd::VisualFieldKind::EmForce,      OverlayRamp::CoolHot,   0.04f, -1.0f, false, false},
    {OverlayId::GravityForce, "gravityForce","Gravity",    OverlayColumn::Forces,
     OverlayRender::Arrows, ftd::VisualFieldKind::GravityForce, OverlayRamp::CoolHot,   0.04f, -1.0f, false, false},
    {OverlayId::StrongForce,  "strongForce","Strong",      OverlayColumn::Forces,
     OverlayRender::Arrows, ftd::VisualFieldKind::StrongForce,  OverlayRamp::CoolHot,   0.04f, -1.0f, false, false},
    {OverlayId::WeakCurl,     "weakCurl",   "curl J", OverlayColumn::Forces,  // ASCII for ∇×J (Inter lacks ∇)
     OverlayRender::Arrows, ftd::VisualFieldKind::Curl,         OverlayRamp::Weak,      0.08f, -1.0f, true,  false},
    // ── Topology ──
    {OverlayId::Latency,      "latency",    "Latency L",   OverlayColumn::Topology,
     OverlayRender::Points, ftd::VisualFieldKind::Latency,      OverlayRamp::Latency,   0.02f, -1.0f, false, false},
    {OverlayId::GaussResidual,"gaussResidual","Gauss resid.", OverlayColumn::Topology,
     OverlayRender::Points, ftd::VisualFieldKind::GaussResidual,OverlayRamp::Diverging, 0.05f, -1.0f, false, true},
    // ── Phenomena ──
    {OverlayId::Horizon,      "horizon",    "Horizon",     OverlayColumn::Phenomena,
     OverlayRender::Points, ftd::VisualFieldKind::Latency,      OverlayRamp::Horizon,   0.02f, 0.95f, false, false},

    // ── Tranche 2 (EXTEND) — sprite points + line list, no new PSO ──
    // Quantum: derived scalars → sprite points.
    {OverlayId::PsiSquared,   "psiSquared", "|\xCF\x88|\xC2\xB2", OverlayColumn::Quantum,
     OverlayRender::DerivedPoints, ftd::VisualFieldKind::FluxVector, OverlayRamp::Viridis,
     0.02f, -1.0f, false, false, OverlayDerive::PsiSquared},
    {OverlayId::Phase,        "phase",      "Phase \xCF\x86",    OverlayColumn::Quantum,
     OverlayRender::PhaseNeedles,  ftd::VisualFieldKind::FluxVector, OverlayRamp::CyclicHSL,
     0.02f, -1.0f, false, false, OverlayDerive::None},
    {OverlayId::Lagrangian,   "lagrangian", "L(x)",      OverlayColumn::Quantum,  // ASCII for ℒ (Inter lacks U+2112)
     OverlayRender::DerivedPoints, ftd::VisualFieldKind::Electric,   OverlayRamp::RdBu,
     0.10f, -1.0f, false, false, OverlayDerive::Lagrangian},
    {OverlayId::Entropy,      "entropy",    "Entropy s",   OverlayColumn::Quantum,
     OverlayRender::DerivedPoints, ftd::VisualFieldKind::FluxVector, OverlayRamp::Grayscale,
     0.04f, -1.0f, false, false, OverlayDerive::Entropy},
    // Volume: |J| on the 3 mid-planes (shared threshold with Flux Volume).
    {OverlayId::FluxSliceMid, "fluxSlice",  "Flux Slice",  OverlayColumn::Volume,
     OverlayRender::FluxSlice,     ftd::VisualFieldKind::FluxVector, OverlayRamp::FluxCloud,
     0.04f, -1.0f, false, true,  OverlayDerive::None},
    // Phenomena: derived scalars / dense bands / particle-geometry.
    {OverlayId::Chirality,    "chirality",  "Chirality",   OverlayColumn::Phenomena,
     OverlayRender::DerivedPoints, ftd::VisualFieldKind::FluxVector, OverlayRamp::Chirality,
     0.02f, -1.0f, false, false, OverlayDerive::Chirality},
    {OverlayId::DualFlux,     "dualFlux",   "Dual J",      OverlayColumn::Phenomena,
     OverlayRender::DualPoints,    ftd::VisualFieldKind::FluxVector, OverlayRamp::FluxCloud,
     0.02f, -1.0f, false, false, OverlayDerive::None},
    {OverlayId::DarkMatterHalo,"dmHalo",    "DM Halo",     OverlayColumn::Phenomena,
     OverlayRender::DenseBand,     ftd::VisualFieldKind::FluxVector, OverlayRamp::FluxCloud,
     0.0f, -1.0f, false, true,  OverlayDerive::DmHalo},
    {OverlayId::Genesis,      "genesis",    "Genesis",     OverlayColumn::Phenomena,
     OverlayRender::DenseBand,     ftd::VisualFieldKind::FluxVector, OverlayRamp::FluxCloud,
     0.0f, -1.0f, false, true,  OverlayDerive::Genesis},
    {OverlayId::ColorCharge,  "colorCharge","Color charge",OverlayColumn::Phenomena,
     OverlayRender::Recolor,       ftd::VisualFieldKind::FluxVector, OverlayRamp::FluxCloud,
     0.0f, -1.0f, false, false, OverlayDerive::None},
    {OverlayId::Damping,      "damping",    "Damping",     OverlayColumn::Phenomena,
     OverlayRender::DampingBoxes,  ftd::VisualFieldKind::FluxVector, OverlayRamp::FluxCloud,
     0.0f, -1.0f, false, false, OverlayDerive::None},
    {OverlayId::Confinement,  "confinement","Confinement", OverlayColumn::Phenomena,
     OverlayRender::PairLinks,     ftd::VisualFieldKind::FluxVector, OverlayRamp::FluxCloud,
     0.0f, -1.0f, false, false, OverlayDerive::None},

    // ── Tranche 5 (NEW) — rubber-sheet surfaces (triangle-mesh vertex-colour) ──
    // Topology sheets. Each deforms a ~40×40 grid at world y = y_frac·N by its
    // (signed) blurred scalar × depth_frac·N, coloured by the per-sheet ramp.
    {OverlayId::GravPotential, "gravPotential", "\xCE\xA6 potential", OverlayColumn::Topology,
     OverlayRender::Sheet, ftd::VisualFieldKind::FluxVector, OverlayRamp::GravWell,
     0.0f, -1.0f, false, false, OverlayDerive::SheetGravPotential, 0.25f, 0.25f},
    {OverlayId::EmEnergy,      "emEnergy",      "EM energy u",   OverlayColumn::Topology,
     OverlayRender::Sheet, ftd::VisualFieldKind::Electric,   OverlayRamp::EmEnergy,
     0.0f, -1.0f, false, false, OverlayDerive::SheetEmEnergy, 0.05f, 0.08f},
    {OverlayId::ChargeDensity, "chargeDensity", "Charge \xCF\x81",  OverlayColumn::Topology,
     OverlayRender::Sheet, ftd::VisualFieldKind::Divergence, OverlayRamp::Charge,
     0.0f, -1.0f, false, false, OverlayDerive::SheetCharge, 0.87f, 0.08f},
    {OverlayId::Vorticity,     "vorticity",     "Vorticity \xCF\x89", OverlayColumn::Topology,
     OverlayRender::Sheet, ftd::VisualFieldKind::Vorticity,  OverlayRamp::Vorticity,
     0.0f, -1.0f, false, false, OverlayDerive::SheetVorticity, 0.97f, 0.03f},
    // Stress-Energy sheets (this column appears in the panel automatically).
    {OverlayId::EPressure,     "ePressure",     "P_E (electric)", OverlayColumn::StressEnergy,
     OverlayRender::Sheet, ftd::VisualFieldKind::Electric,   OverlayRamp::EPressure,
     0.0f, -1.0f, false, false, OverlayDerive::SheetEPressure, 0.35f, 0.08f},
    {OverlayId::BPressure,     "bPressure",     "P_B (magnetic)", OverlayColumn::StressEnergy,
     OverlayRender::Sheet, ftd::VisualFieldKind::Magnetic,   OverlayRamp::BPressure,
     0.0f, -1.0f, false, false, OverlayDerive::SheetBPressure, 0.45f, 0.08f},

    // ── Knot Zones (the 33rd overlay) — Phenomena ──────────────────────────────
    // The adapter's KnotZones handler traces E and B streamlines internally and
    // clusters them into wireframe boxes; the descriptor's `kind` is nominal
    // (Magnetic) and unused — the handler samples both Electric and Magnetic.
    {OverlayId::KnotZones,     "knotZones",     "Knot Zones",     OverlayColumn::Phenomena,
     OverlayRender::KnotZones, ftd::VisualFieldKind::Magnetic,  OverlayRamp::FluxCloud,
     0.0f, -1.0f, false, false, OverlayDerive::None},
};

inline constexpr std::size_t kOverlayCount =
    sizeof(kOverlayRegistry) / sizeof(kOverlayRegistry[0]);

// Human title for a column (menu order). Empty columns are simply not rendered.
inline const char* overlay_column_title(OverlayColumn c) {
    switch (c) {
        case OverlayColumn::Volume:       return "Volume";
        case OverlayColumn::Fields:       return "Fields";
        case OverlayColumn::Forces:       return "Forces";
        case OverlayColumn::Quantum:      return "Quantum";
        case OverlayColumn::Topology:     return "Topology";
        case OverlayColumn::StressEnergy: return "Stress-Energy";
        case OverlayColumn::Phenomena:    return "Phenomena";
        default:                          return "";
    }
}

inline const OverlayDescriptor* overlay_by_id(std::uint32_t id) {
    for (const OverlayDescriptor& d : kOverlayRegistry)
        if (static_cast<std::uint32_t>(d.id) == id) return &d;
    return nullptr;
}

inline const OverlayDescriptor* overlay_by_name(std::string_view name) {
    for (const OverlayDescriptor& d : kOverlayRegistry)
        if (name == d.name) return &d;
    return nullptr;
}

}  // namespace ftd::native
