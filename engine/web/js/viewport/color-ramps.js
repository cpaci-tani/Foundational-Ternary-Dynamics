/**
 * Color ramps for Scale 0 viewport overlays.
 *
 * Each ramp writes `(r, g, b)` into a pre-allocated destination array at
 * offset `i`, so callers avoid per-call heap allocation:
 *
 *     rampViridis(0.5, colorAttr.array, vertexIndex * 3);
 *
 * Ramps accept `t ∈ [0, 1]` unless documented otherwise — signed ramps
 * (diverging palettes) accept `t ∈ [-1, 1]` with negative = one endpoint,
 * zero = neutral, positive = the other endpoint. The input is clamped at
 * the boundary of its expected range so callers don't need to worry
 * about over-ranged values leaking into the output.
 *
 * Along with the ramps this module also exports:
 *   - FORCE_PALETTES  — the 3-stop palettes used by per-force overlays
 *   - lerpPalette     — low/mid/high palette interpolator used by the
 *                       force glyph + heatmap overlays
 *
 * Extracted from viewport.js as part of the large-file refactor (Wave 1
 * ticket 1 in docs/SPEC_REFACTOR_LARGE_FILES.md). The extraction is
 * mechanical — these functions had zero `this` access and zero side
 * effects, so moving them to module scope is a pure rename from
 * `this._rampX(...)` to `rampX(...)` at every call site.
 */

// ── Diverging / scalar ramps ──────────────────────────────────────────

/**
 * Approximate Viridis: purple → teal → yellow across `t ∈ [0, 1]`.
 * @param {number} t
 * @param {number[]|Float32Array} out
 * @param {number} i  destination offset (writes out[i], out[i+1], out[i+2])
 */
export function rampViridis(t, out, i) {
    t = Math.max(0, Math.min(1, t));
    if (t < 0.5) {
        const u = t * 2;
        out[i]     = 0.267 * (1 - u) + 0.13  * u;
        out[i + 1] = 0.004 * (1 - u) + 0.566 * u;
        out[i + 2] = 0.329 * (1 - u) + 0.551 * u;
    } else {
        const u = (t - 0.5) * 2;
        out[i]     = 0.13  * (1 - u) + 0.993 * u;
        out[i + 1] = 0.566 * (1 - u) + 0.906 * u;
        out[i + 2] = 0.551 * (1 - u) + 0.144 * u;
    }
}

/**
 * Cyclic HSL ramp over a phase in `[0, π/2]` (the range produced by
 * `atan2(|J_R|, |J_L|)`). Maps the full hue cycle to the phase.
 * @param {number} phase  phase in [0, π/2]
 * @param {number[]|Float32Array} out
 * @param {number} i
 */
export function rampCyclicHSL(phase, out, i) {
    const hue = (phase / (Math.PI / 2)) % 1;
    // HSL → RGB with S=1, L=0.5
    const h6 = hue * 6;
    const c = 1;  // saturation * (1 - |2L-1|) = 1 * 1 = 1
    const x = c * (1 - Math.abs((h6 % 2) - 1));
    let r, g, b;
    if (h6 < 1)      { r = c; g = x; b = 0; }
    else if (h6 < 2) { r = x; g = c; b = 0; }
    else if (h6 < 3) { r = 0; g = c; b = x; }
    else if (h6 < 4) { r = 0; g = x; b = c; }
    else if (h6 < 5) { r = x; g = 0; b = c; }
    else             { r = c; g = 0; b = x; }
    out[i] = r; out[i + 1] = g; out[i + 2] = b;
}

/**
 * Diverging red-blue: `t ∈ [-1, 1]`; negative = blue, zero = white,
 * positive = red.
 */
export function rampDivergingRdBu(t, out, i) {
    t = Math.max(-1, Math.min(1, t));
    if (t >= 0) {
        const u = t;
        out[i]     = 0.969 * (1 - u) + 0.698 * u;
        out[i + 1] = 0.969 * (1 - u) + 0.094 * u;
        out[i + 2] = 0.969 * (1 - u) + 0.169 * u;
    } else {
        const u = -t;
        out[i]     = 0.969 * (1 - u) + 0.129 * u;
        out[i + 1] = 0.969 * (1 - u) + 0.400 * u;
        out[i + 2] = 0.969 * (1 - u) + 0.675 * u;
    }
}

/** Straight grayscale: `out = (t, t, t)` on `t ∈ [0, 1]`. */
export function rampGrayscale(t, out, i) {
    t = Math.max(0, Math.min(1, t));
    out[i] = t; out[i + 1] = t; out[i + 2] = t;
}

/**
 * Gravitational-well ramp: `t ∈ [0, 1]`. Deeper well (higher t) = deep
 * blue; peak = yellow.
 */
export function rampGravWell(t, out, i) {
    t = Math.max(0, Math.min(1, t));
    if (t > 0.5) {
        const u = (t - 0.5) * 2;
        out[i]     = 0.0 + 0.0   * u;
        out[i + 1] = 0.4 * (1 - u);
        out[i + 2] = 0.8 * (1 - u) + 0.2 * u;
    } else {
        const u = t * 2;
        out[i]     = 1.0 * (1 - u) + 0.0 * u;
        out[i + 1] = 1.0 * (1 - u) + 0.4 * u;
        out[i + 2] = 0.0 * (1 - u) + 0.8 * u;
    }
}

// ── Topology-sheet ramps ──────────────────────────────────────────────
// Unsigned ramps accept t ∈ [0, 1]; signed accepts t ∈ [-1, 1].

/** EM energy `½(|E|² + |B|²)`: teal → warm orange on `t ∈ [0, 1]`. */
export function rampEmEnergy(t, out, i) {
    t = Math.max(0, Math.min(1, t));
    out[i]     = 0.05 * (1 - t) + 0.98 * t;
    out[i + 1] = 0.55 * (1 - t) + 0.62 * t;
    out[i + 2] = 0.55 * (1 - t) + 0.14 * t;
}

/**
 * Charge density `∇·J`: diverging blue ↔ red on `t ∈ [-1, 1]`.
 * Negatives = sinks (blue well), positives = sources (red peak).
 */
export function rampCharge(t, out, i) {
    t = Math.max(-1, Math.min(1, t));
    if (t >= 0) {
        const u = t;
        out[i]     = 0.95 * (1 - u) + 0.90 * u;
        out[i + 1] = 0.95 * (1 - u) + 0.10 * u;
        out[i + 2] = 0.95 * (1 - u) + 0.20 * u;
    } else {
        const u = -t;
        out[i]     = 0.95 * (1 - u) + 0.13 * u;
        out[i + 1] = 0.95 * (1 - u) + 0.35 * u;
        out[i + 2] = 0.95 * (1 - u) + 0.85 * u;
    }
}

/** Vorticity `|∇×J|`: magma-like, near-black → violet → gold on `t ∈ [0, 1]`. */
export function rampVorticity(t, out, i) {
    t = Math.max(0, Math.min(1, t));
    if (t < 0.5) {
        const u = t * 2;
        out[i]     = 0.02 * (1 - u) + 0.48 * u;
        out[i + 1] = 0.02 * (1 - u) + 0.05 * u;
        out[i + 2] = 0.08 * (1 - u) + 0.53 * u;
    } else {
        const u = (t - 0.5) * 2;
        out[i]     = 0.48 * (1 - u) + 1.00 * u;
        out[i + 1] = 0.05 * (1 - u) + 0.85 * u;
        out[i + 2] = 0.53 * (1 - u) + 0.20 * u;
    }
}

// ── Tier 1/2/3 ramps (added 2026-04-18) ───────────────────────────────

/**
 * Helicity `J·(∇×J)`: diverging cyan ↔ magenta on `t ∈ [-1, 1]`.
 * Captures left/right-handed field-line linking.
 */
export function rampHelicity(t, out, i) {
    t = Math.max(-1, Math.min(1, t));
    if (t >= 0) {
        const u = t;
        out[i]     = 0.85 * (1 - u) + 0.95 * u;
        out[i + 1] = 0.90 * (1 - u) + 0.15 * u;
        out[i + 2] = 0.95 * (1 - u) + 0.85 * u;
    } else {
        const u = -t;
        out[i]     = 0.85 * (1 - u) + 0.10 * u;
        out[i + 1] = 0.90 * (1 - u) + 0.85 * u;
        out[i + 2] = 0.95 * (1 - u) + 0.90 * u;
    }
}

/** Kretschmann curvature: deep-space blue → molten white on `t ∈ [0, 1]`. */
export function rampKretschmann(t, out, i) {
    t = Math.max(0, Math.min(1, t));
    out[i]     = 0.05 * (1 - t) + 1.00 * t;
    out[i + 1] = 0.10 * (1 - t) + 0.95 * t;
    out[i + 2] = 0.35 * (1 - t) + 0.80 * t;
}

/** Electric pressure `P_E = ½|E|²`: pale yellow → saturated red on `t ∈ [0, 1]`. */
export function rampEPressure(t, out, i) {
    t = Math.max(0, Math.min(1, t));
    out[i]     = 0.95 * (1 - t) + 0.95 * t;
    out[i + 1] = 0.95 * (1 - t) + 0.25 * t;
    out[i + 2] = 0.65 * (1 - t) + 0.15 * t;
}

/** Magnetic pressure `P_B = ½|B|²`: pale cyan → deep teal on `t ∈ [0, 1]`. */
export function rampBPressure(t, out, i) {
    t = Math.max(0, Math.min(1, t));
    out[i]     = 0.75 * (1 - t) + 0.00 * t;
    out[i + 1] = 0.95 * (1 - t) + 0.55 * t;
    out[i + 2] = 0.95 * (1 - t) + 0.70 * t;
}

/** Kinetic energy density: olive → hot yellow on `t ∈ [0, 1]`. */
export function rampKineticEnergy(t, out, i) {
    t = Math.max(0, Math.min(1, t));
    out[i]     = 0.30 * (1 - t) + 1.00 * t;
    out[i + 1] = 0.45 * (1 - t) + 0.95 * t;
    out[i + 2] = 0.10 * (1 - t) + 0.20 * t;
}

/** Fisher information `|∇ρ|²/ρ`: indigo → bright lime on `t ∈ [0, 1]`. */
export function rampFisher(t, out, i) {
    t = Math.max(0, Math.min(1, t));
    out[i]     = 0.25 * (1 - t) + 0.75 * t;
    out[i + 1] = 0.15 * (1 - t) + 1.00 * t;
    out[i + 2] = 0.55 * (1 - t) + 0.30 * t;
}

/**
 * Dual-substrate coherence `J·(∇×J)/(|J|·|∇×J|) ∈ [-1, 1]`:
 * diverging orange ↔ violet for right/left-handed Beltrami flow.
 */
export function rampCoherence(t, out, i) {
    t = Math.max(-1, Math.min(1, t));
    if (t >= 0) {
        const u = t;
        out[i]     = 0.90 * (1 - u) + 1.00 * u;
        out[i + 1] = 0.90 * (1 - u) + 0.55 * u;
        out[i + 2] = 0.90 * (1 - u) + 0.10 * u;
    } else {
        const u = -t;
        out[i]     = 0.90 * (1 - u) + 0.45 * u;
        out[i + 1] = 0.90 * (1 - u) + 0.15 * u;
        out[i + 2] = 0.90 * (1 - u) + 0.85 * u;
    }
}

// ── Force palettes + palette interpolator ─────────────────────────────

/**
 * 3-stop low/mid/high color palettes for each force type. Consumed by
 * arrow-field, heatmap, and glyph overlays via `lerpPalette(pal, t)`.
 */
export const FORCE_PALETTES = {
    em:      { low: [0.0, 0.2, 0.4], mid: [0.0, 0.9, 1.0],  high: [0.7, 1.0, 1.0] },
    gravity: { low: [0.4, 0.2, 0.0], mid: [1.0, 0.67, 0.0], high: [1.0, 1.0, 0.6] },
    strong:  { low: [0.4, 0.0, 0.05], mid: [1.0, 0.09, 0.27], high: [1.0, 0.7, 0.7] },
    weak:    { low: [0.2, 0.0, 0.4], mid: [0.67, 0.0, 1.0], high: [0.9, 0.6, 1.0] },
};

/**
 * Interpolate a 3-stop color palette at parameter t in `[0, 1]`.
 * Returns a freshly-allocated 3-tuple `[r, g, b]`; callers that run per
 * frame should cache the result or switch to an `(out, i)` writer if GC
 * pressure becomes an issue.
 *
 * @param {object} pal - `{ low: [r,g,b], mid: [r,g,b], high: [r,g,b] }`
 * @param {number} t   - 0..1
 * @returns {[number, number, number]}
 */
export function lerpPalette(pal, t) {
    const tt = Math.max(0, Math.min(1, t));
    if (tt < 0.5) {
        const u = tt * 2;
        return [
            pal.low[0] + (pal.mid[0] - pal.low[0]) * u,
            pal.low[1] + (pal.mid[1] - pal.low[1]) * u,
            pal.low[2] + (pal.mid[2] - pal.low[2]) * u,
        ];
    }
    const u = (tt - 0.5) * 2;
    return [
        pal.mid[0] + (pal.high[0] - pal.mid[0]) * u,
        pal.mid[1] + (pal.high[1] - pal.mid[1]) * u,
        pal.mid[2] + (pal.high[2] - pal.mid[2]) * u,
    ];
}

// ── Ramp registry (name → function) ───────────────────────────────────
// Used by the topology-sheet config which carries ramp references by
// string name (matches the legacy `this[cfg.ramp](...)` dispatch pattern
// while keeping the viewport refactor non-invasive).
export const RAMP_BY_NAME = {
    rampViridis,
    rampCyclicHSL,
    rampDivergingRdBu,
    rampGrayscale,
    rampGravWell,
    rampEmEnergy,
    rampCharge,
    rampVorticity,
    rampHelicity,
    rampKretschmann,
    rampEPressure,
    rampBPressure,
    rampKineticEnergy,
    rampFisher,
    rampCoherence,
};
