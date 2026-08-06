/**
 * Scale 0 — Flux slice panel pure helpers/constants.
 * Extracted from flux-slice-panel.js (registry, slice rasterizers, axis map).
 */

import {
    rampViridis,
    rampEmEnergy,
    rampVorticity,
    rampCharge,
    rampGrayscale,
    rampGravWell,
    rampCyclicHSL,
    rampDivergingRdBu,
    rampEPressure,
    rampBPressure,
    FORCE_PALETTES,
    lerpPalette,
} from '../../../../viewport/color-ramps.js';
import {
    computePsiSquaredFrame,
    computePhaseFrame,
    computeLagrangianDensityFrame,
    computeEntropyDensityFrame,
    computeGravPotentialFrame,
    computeEmEnergyFrame,
    computeEPressureFrame,
    computeBPressureFrame,
    computeHorizonFrame,
} from '../../runtime/overlay-frames.js';
import { DUAL_DELTA } from '../../../../constants.js';

export const DEFAULT_CANVAS_PX = 220;
export const DENSE_CANVAS_PX = 160; // shrink when >2 active rows are visible
export const FLOOR_FRAC = 1e-6;
export const DENSE_THRESHOLD = 2;

// ── Slot → canonical sampler kind ────────────────────────────────────
//
// Keyed exactly like field-sample-cache.js's KIND_BY_SLOT so a driver's
// `slot` / `requiredSampledKeys` line up with the same vocabulary the 3D
// overlay runtime already uses. Only kinds reachable through
// bridge.getSamplerOr (SCALE0_SAMPLER_METHODS in bridge-contract.js)
// belong here — force fields are a separate method family (see
// `forceType` on the force-field drivers below), handled directly in
// _buildFrameSampleCache.
export const SLOT_TO_KIND = {
    fluxVector:    'fluxVector',
    poynting:      'poynting',
    eField:        'e',
    bField:        'b',
    divergence:    'divJ',
    vorticity:     'vorticity',
    helicity:      'helicity',
    kretschmann:   'kretschmann',
    latency:       'latency',
    fisher:        'fisher',
    coherence:     'coherence',
    curlJ:         'curlJ',
    state:         'state',
    gaussResidual: 'gaussResidual',
};

// Slots that must always sample at stride 1 regardless of the panel's own
// coarse-stride choice (mirrors STRIDE_ONE_SLOTS in field-sample-cache.js —
// genuinely sparse/threshold quantities the 3D overlay system also always
// samples at full resolution; not exported there, so duplicated here rather
// than reimplemented differently).
export const STRIDE_ONE_SLOTS = new Set(['state', 'gaussResidual']);

// ── Per-field driver registry ───────────────────────────────────────
//
// One entry per supported field. The slice panel iterates this list,
// renders one row per active driver, and dispatches to the driver's
// sample/ramp pair to paint each tile.
//
// `vizFlagKey` matches the keys in store.js::fieldFlags
// (FIELD_TOGGLE_KEYS, lines 3–42 of state/store.js).
//
// `sample(bridge, axis, mid, N)` MUST return a Float64Array(N*N)
// scoped to a single plane. Implementations live below.
//
// `signed` controls the autoscale + ramp input: false → t = v / vmax in
// [0, 1]; true → t = clamp(v / vmax, -1, +1) for diverging ramps.
//
// `ramp` is one of the named ramps from viewport/color-ramps.js.
export const FIELD_DRIVERS = [
    // ── Flagship dense |J| volume slice (unchanged) ──────────────────────
    {
        key: 'fluxJ',
        label: '|J|',
        vizFlagKey: 'showFluxLines',
        signed: false,
        ramp: rampViridis,
        // Per-frame source: returns whatever the driver needs to slice;
        // for fluxJ the source is unused because getFluxSlice slices directly.
        source: (/* bridge */) => null,
        sample: (bridge, axis, mid, N /*, source */) => {
            // getFluxSlice reuses an internal _sliceBuf. The
            // transpose+Y-flip combo rewrites the bridge's layout into
            // the panel's (col=first-named-axis, +second-named-axis up)
            // convention in a single pass, snapshotting in the process
            // so the next bridge call can clobber _sliceBuf safely.
            const s = bridge.getFluxSlice?.(axis, mid);
            if (!s || s.length !== N * N) return s ? s.slice() : null;
            return transposeAndFlipNN(s, N);
        },
    },

    // ── Raw sampler-kind rows — routed through the shared per-frame
    // cache built by _buildFrameSampleCache. `slot` matches
    // field-sample-cache.js's KIND_BY_SLOT vocabulary. ───────────────────
    {
        key: 'eField',
        label: '|E|',
        vizFlagKey: 'showEField',
        signed: false,
        ramp: rampEmEnergy,
        slot: 'eField',
        source: (bridge, sampled) => sampled.eField ?? null,
        sample: (bridge, axis, mid, N, source) => sliceVectorMag(source, axis, mid, N),
    },
    {
        key: 'bField',
        label: '|B|',
        vizFlagKey: 'showBField',
        signed: false,
        ramp: rampVorticity,
        slot: 'bField',
        source: (bridge, sampled) => sampled.bField ?? null,
        sample: (bridge, axis, mid, N, source) => sliceVectorMag(source, axis, mid, N),
    },
    {
        key: 'poynting',
        label: '|S|',
        vizFlagKey: 'showPoynting',
        signed: false,
        ramp: rampEmEnergy,
        slot: 'poynting',
        source: (bridge, sampled) => sampled.poynting ?? null,
        sample: (bridge, axis, mid, N, source) => sliceVectorMag(source, axis, mid, N),
    },
    {
        key: 'divJ',
        label: '∇·J',
        vizFlagKey: 'showDivField',
        signed: true,
        ramp: rampCharge,
        slot: 'divergence',
        source: (bridge, sampled) => sampled.divergence ?? null,
        sample: (bridge, axis, mid, N, source) => sliceScalarSigned(source, axis, mid, N),
    },
    {
        key: 'fluxVector',
        label: '|J| (sparse)',
        vizFlagKey: 'showFluxLines',
        signed: false,
        ramp: rampViridis,
        slot: 'fluxVector',
        source: (bridge, sampled) => sampled.fluxVector ?? null,
        sample: (bridge, axis, mid, N, source) => sliceVectorMag(source, axis, mid, N),
    },
    {
        key: 'vorticity',
        label: '|ω|',
        vizFlagKey: 'showVorticity',
        signed: false,
        ramp: rampVorticity,
        slot: 'vorticity',
        source: (bridge, sampled) => sampled.vorticity ?? null,
        sample: (bridge, axis, mid, N, source) => sliceScalarSigned(source, axis, mid, N),
    },
    {
        key: 'helicity',
        label: 'H',
        vizFlagKey: '',
        signed: true,
        ramp: rampDivergingRdBu,
        slot: 'helicity',
        source: (bridge, sampled) => sampled.helicity ?? null,
        sample: (bridge, axis, mid, N, source) => sliceScalarSigned(source, axis, mid, N),
    },
    {
        key: 'kretschmann',
        label: 'K',
        vizFlagKey: '',
        signed: false,
        ramp: rampGrayscale,
        slot: 'kretschmann',
        source: (bridge, sampled) => sampled.kretschmann ?? null,
        sample: (bridge, axis, mid, N, source) => sliceScalarSigned(source, axis, mid, N),
    },
    {
        key: 'latency',
        label: 'L',
        vizFlagKey: 'showLatency',
        signed: false,
        ramp: rampGravWell,
        slot: 'latency',
        source: (bridge, sampled) => sampled.latency ?? null,
        sample: (bridge, axis, mid, N, source) => sliceScalarSigned(source, axis, mid, N),
    },
    {
        key: 'fisher',
        label: 'F',
        vizFlagKey: '',
        signed: false,
        ramp: rampViridis,
        slot: 'fisher',
        source: (bridge, sampled) => sampled.fisher ?? null,
        sample: (bridge, axis, mid, N, source) => sliceScalarSigned(source, axis, mid, N),
    },
    {
        key: 'coherence',
        label: 'C',
        vizFlagKey: '',
        signed: true,
        ramp: rampDivergingRdBu,
        slot: 'coherence',
        source: (bridge, sampled) => sampled.coherence ?? null,
        sample: (bridge, axis, mid, N, source) => sliceScalarSigned(source, axis, mid, N),
    },
    {
        key: 'curlJ',
        label: '|∇×J|',
        vizFlagKey: 'showForceWeak',
        signed: false,
        ramp: rampGrayscale,
        slot: 'curlJ',
        source: (bridge, sampled) => sampled.curlJ ?? null,
        sample: (bridge, axis, mid, N, source) => sliceVectorMag(source, axis, mid, N),
    },
    {
        key: 'state',
        label: 's',
        vizFlagKey: 'showStateField',
        signed: true,
        ramp: rampCharge,
        slot: 'state',
        source: (bridge, sampled) => sampled.state ?? null,
        sample: (bridge, axis, mid, N, source) => sliceScalarSigned(source, axis, mid, N),
    },
    {
        key: 'gaussResidual',
        label: 'r',
        vizFlagKey: 'showGaussResidual',
        signed: true,
        ramp: rampCharge,
        slot: 'gaussResidual',
        source: (bridge, sampled) => sampled.gaussResidual ?? null,
        sample: (bridge, axis, mid, N, source) => sliceScalarSigned(source, axis, mid, N),
    },

    // ── Force-field rows — bridge.get{EM,Gravity,Strong}ForceField, same
    // sparse {positions,vectors,count} shape as e/b/poynting. Ramps reuse
    // the exact FORCE_PALETTES used by the 3D Forces column. ────────────
    {
        key: 'forceEm',
        label: 'EM',
        vizFlagKey: 'showForceEM',
        signed: false,
        ramp: forcePaletteRamp('em'),
        forceType: 'em',
        slot: 'forceEm',
        source: (bridge, sampled) => sampled.forceEm ?? null,
        sample: (bridge, axis, mid, N, source) => sliceVectorMag(source, axis, mid, N),
    },
    {
        key: 'forceGravity',
        // getGravityForceField has no C++/WASM binding in this build (only
        // getEMForceField and getStrongForceField exist in ftd_wasm.cpp) —
        // the worker's postFrame() silently `continue`s past it every frame
        // (typeof mod[method] !== 'function'), so this row is cheap but will
        // render as a permanently-empty tile. Labeled honestly rather than
        // pretending it works; remove this row entirely once/if the binding
        // is added.
        label: 'Gravity (N/A)',
        vizFlagKey: 'showForceGravity',
        signed: false,
        ramp: forcePaletteRamp('gravity'),
        forceType: 'gravity',
        slot: 'forceGravity',
        source: (bridge, sampled) => sampled.forceGravity ?? null,
        sample: (bridge, axis, mid, N, source) => sliceVectorMag(source, axis, mid, N),
    },
    {
        key: 'forceStrong',
        label: 'Strong',
        vizFlagKey: 'showForceStrong',
        signed: false,
        ramp: forcePaletteRamp('strong'),
        forceType: 'strong',
        slot: 'forceStrong',
        source: (bridge, sampled) => sampled.forceStrong ?? null,
        sample: (bridge, axis, mid, N, source) => sliceVectorMag(source, axis, mid, N),
    },

    // ── Derived Tier-1 overlay rows — reuse the exact overlay-frames.js
    // compute*Frame functions against the shared sample cache + a
    // per-driver, panel-owned scratch object (this._scratch[drv.key]).
    // computeChargeDensityFrame is deliberately OMITTED (confirmed true
    // duplicate of the divJ row's own sampled.divergence buffer — same
    // positions/values/signed convention, its own doc comment says "we
    // just forward the buffer"). computeVorticityFrame / computeLatencyFrame
    // / computeGaussResidualFrame / computeStateFieldFrame are also
    // omitted as separate rows (confirmed pure pass-throughs of raw kinds
    // already covered above, modulo a `normalizer` this panel doesn't
    // consume — it runs its own independent per-row rolling autoscale via
    // _fieldGlobalMax) — only computeHorizonFrame earns its own row
    // (genuinely different: thresholded/filtered data, not a pass-through). ─
    {
        key: 'psiSquared',
        label: '|ψ|²',
        vizFlagKey: 'showPsiSquared',
        signed: false,
        ramp: rampViridis,
        requiredSampledKeys: ['fluxVector'],
        source: (bridge, sampled, scratch) => computePsiSquaredFrame(sampled, scratch, false),
        sample: (bridge, axis, mid, N, source) => sliceDerivedFrame(source, axis, mid, N),
    },
    {
        key: 'phase',
        label: 'Phase φ',
        vizFlagKey: 'showPhase',
        signed: true,
        ramp: rampCyclicHSL,
        // rampCyclicHSL wants a raw radian angle, not a per-frame
        // normalized [-1,1] ratio — see the rawRamp branch in _paintSlice.
        rawRamp: true,
        requiredSampledKeys: ['fluxVector'],
        source: (bridge, sampled, scratch) => {
            const fv = sampled.fluxVector;
            if (!fv || !fv.count) return null;
            // Reproduce the real controller's fixed (1±δ)/2 dual-substrate
            // scalar split locally (field-overlays.js) — a pure rescale of
            // the already-fetched fluxVector sample, so no dependency on
            // the live showDualSubstrate toggle or controller state. Under
            // this fixed split, φ=atan2(rightFactor,leftFactor) is a
            // SPATIAL CONSTANT wherever |J|>0 — this row is expected to
            // render as one flat tile, not a wiring bug.
            const vecLen = fv.vectors.length;
            if (!scratch.dualLVecs || scratch.dualLVecs.length < vecLen) {
                scratch.dualLVecs = new Float32Array(vecLen);
                scratch.dualRVecs = new Float32Array(vecLen);
            }
            const leftFactor = (1 + DUAL_DELTA) / 2;
            const rightFactor = (1 - DUAL_DELTA) / 2;
            for (let i = 0; i < vecLen; i++) {
                scratch.dualLVecs[i] = fv.vectors[i] * leftFactor;
                scratch.dualRVecs[i] = fv.vectors[i] * rightFactor;
            }
            return computePhaseFrame(sampled, scratch, scratch.dualLVecs, scratch.dualRVecs);
        },
        sample: (bridge, axis, mid, N, source) => sliceDerivedFrame(source, axis, mid, N),
    },
    {
        key: 'lagrangianDensity',
        label: 'ℒ(x)',
        vizFlagKey: 'showLagrangianDensity',
        signed: true,
        ramp: rampDivergingRdBu,
        // NOT field-sample-cache.js's SCALAR_SAMPLE_DEPS list (which also
        // names fluxVector/poynting) — the actual function body only reads
        // sampled.eField and sampled.divergence.
        requiredSampledKeys: ['eField', 'divergence'],
        source: (bridge, sampled, scratch) => computeLagrangianDensityFrame(sampled, scratch),
        sample: (bridge, axis, mid, N, source) => sliceDerivedFrame(source, axis, mid, N),
    },
    {
        key: 'entropyDensity',
        label: 'Entropy s',
        vizFlagKey: 'showEntropyDensity',
        signed: false,
        ramp: rampGrayscale,
        requiredSampledKeys: ['fluxVector'],
        source: (bridge, sampled, scratch) => computeEntropyDensityFrame(sampled, scratch),
        sample: (bridge, axis, mid, N, source) => sliceDerivedFrame(source, axis, mid, N),
    },
    {
        key: 'gravPotential',
        label: 'Φ potential',
        vizFlagKey: 'showGravPotential',
        signed: true,
        ramp: absThenRamp(rampGravWell),
        requiredSampledKeys: ['fluxVector'],
        // ctx={bridge} reproduces getActiveScale0Bridge(ctx,scratch)'s
        // ctx.bridge fallback exactly (scratch is a fresh {} lacking
        // useFluxMock/fluxMock) — getGravPotentialSamples is implemented on
        // zero bridges repo-wide today, so this always falls through to
        // the |J|² JS proxy, matching production.
        source: (bridge, sampled, scratch) => computeGravPotentialFrame({ bridge }, sampled, scratch),
        sample: (bridge, axis, mid, N, source) => sliceDerivedFrame(source, axis, mid, N),
    },
    {
        key: 'emEnergy',
        label: 'EM energy u',
        vizFlagKey: 'showEmEnergy',
        signed: false,
        ramp: rampEmEnergy,
        requiredSampledKeys: ['eField', 'bField'],
        source: (bridge, sampled, scratch) => computeEmEnergyFrame(sampled, scratch),
        sample: (bridge, axis, mid, N, source) => sliceDerivedFrame(source, axis, mid, N),
    },
    {
        key: 'ePressure',
        label: 'P_E (electric)',
        vizFlagKey: 'showEPressure',
        signed: false,
        ramp: rampEPressure,
        requiredSampledKeys: ['eField'],
        source: (bridge, sampled, scratch) => computeEPressureFrame(sampled, scratch),
        sample: (bridge, axis, mid, N, source) => sliceDerivedFrame(source, axis, mid, N),
    },
    {
        key: 'bPressure',
        label: 'P_B (magnetic)',
        vizFlagKey: 'showBPressure',
        signed: false,
        ramp: rampBPressure,
        requiredSampledKeys: ['bField'],
        source: (bridge, sampled, scratch) => computeBPressureFrame(sampled, scratch),
        sample: (bridge, axis, mid, N, source) => sliceDerivedFrame(source, axis, mid, N),
    },
    {
        key: 'horizon',
        label: 'Horizon',
        vizFlagKey: 'showHorizon',
        signed: false,
        ramp: rampGrayscale,
        requiredSampledKeys: ['latency'],
        source: (bridge, sampled, scratch) => computeHorizonFrame(sampled, scratch),
        sample: (bridge, axis, mid, N, source) => sliceDerivedFrame(source, axis, mid, N),
    },
];

export const DRIVER_BY_KEY = Object.fromEntries(FIELD_DRIVERS.map(d => [d.key, d]));

// Default per-field override on construction / scenario change. Every field
// defaults to 'on' (always visible) EXCEPT the 3 force-field rows, which
// default to null (mirror the 3D viz panel's own toggle, itself default-off
// — see createFieldFlags() in state/store.js). Force-field sampling is
// O(sample-points × manifested-voxel-count) for EM and O(sample-points ×
// manifested-voxel-count²) for Strong (it materializes every particle PAIR)
// — for a scenario with a few thousand manifested voxels (e.g. two locked
// marker planes) that is tens of millions to tens of BILLIONS of iterations
// per frame, run inside the worker's tick loop. Defaulting these 3 rows on
// unconditionally (as every other row safely can) silently saddled every
// scenario with that cost the moment the panel was opened, stalling the
// worker's own physics loop. A user who explicitly wants a force-field
// slice can still turn one on via its chip or the 3D panel's own toggle.
export const DEFAULT_FIELD_OVERRIDE = Object.fromEntries(
    FIELD_DRIVERS.map(d => [d.key, d.forceType ? null : 'on']));

// ── File-local helpers ───────────────────────────────────────────────

/**
 * Rasterize a sparse sampled vector field (positions = voxel centers,
 * vectors = 3-tuples per sample) onto an N×N grid covering the chosen
 * mid-plane. Cells without a matching sample stay zero.
 *
 * Output layout (consumed by ImageData(buf,N,N) where pixel (px,py) reads
 * buf[(py*N + px)*4..]):
 *
 *   - "xy" panel (axis=2, z=mid):  panel X = lattice x →,  panel Y = lattice y ↑
 *   - "xz" panel (axis=1, y=mid):  panel X = lattice x →,  panel Y = lattice z ↑
 *   - "yz" panel (axis=0, x=mid):  panel X = lattice y →,  panel Y = lattice z ↑
 *
 * The Y axis is FLIPPED on the canvas (row = N-1-axis_value) so each
 * panel matches the Three.js viewport's right-handed Y-up orientation:
 * "+y goes up" in the xy tile mirrors "+y goes up" in the 3D scene.
 * Without the flip, ImageData's natural y-down convention would make
 * the panels appear as their mirror image of the on-screen lattice.
 *
 * Concretely we store `out[row*N + col]` where:
 *   - `col` (canvas X) is the FIRST-named axis value (rightward).
 *   - `row` (canvas Y) is `N - 1 - second_named_axis_value` (upward
 *     in screen space, since canvas Y is technically downward).
 *
 * @param {{positions:Float32Array, vectors:Float32Array, count:number}|null|undefined} sample
 * @param {0|1|2} axis  0 → x=mid (yz plane); 1 → y=mid (xz); 2 → z=mid (xy)
 * @param {number} mid  integer voxel index of the slice plane
 * @param {number} N    lattice size
 * @returns {Float64Array}  N*N scalar magnitudes
 */
export function sliceVectorMag(sample, axis, mid, N) {
    const out = new Float64Array(N * N);
    if (!sample || !sample.count) return out;
    const pos = sample.positions;
    const vec = sample.vectors;
    if (!pos || !vec) return out;
    const M = N - 1;
    for (let s = 0, p = 0, v = 0; s < sample.count; s++, p += 3, v += 3) {
        // Voxel centers come in as (x + 0.5, y + 0.5, z + 0.5).
        // Floor the center to recover the integer voxel index.
        const ix = (pos[p]     - 0.5) | 0;
        const iy = (pos[p + 1] - 0.5) | 0;
        const iz = (pos[p + 2] - 0.5) | 0;
        let row, col;
        if (axis === 0) {
            // yz plane: panel X = y (col), panel Y = z, flipped so +z goes UP.
            if (ix !== mid) continue;
            col = iy; row = M - iz;
        } else if (axis === 1) {
            // xz plane: panel X = x (col), panel Y = z, flipped so +z goes UP.
            if (iy !== mid) continue;
            col = ix; row = M - iz;
        } else {
            // xy plane: panel X = x (col), panel Y = y, flipped so +y goes UP.
            if (iz !== mid) continue;
            col = ix; row = M - iy;
        }
        if (col < 0 || col >= N || row < 0 || row >= N) continue;
        const m = Math.hypot(vec[v], vec[v + 1], vec[v + 2]);
        out[row * N + col] = m;
    }
    return out;
}

/**
 * Same as sliceVectorMag but for sparse scalar samples. Preserves sign
 * (no abs/hypot) so signed fields like ∇·J light up with diverging ramps.
 *
 * Layout matches sliceVectorMag — see that docstring for the panel-axis
 * + Y-up convention.
 *
 * @param {{positions:Float32Array, values:Float32Array, count:number}|null|undefined} sample
 */
export function sliceScalarSigned(sample, axis, mid, N) {
    const out = new Float64Array(N * N);
    if (!sample || !sample.count) return out;
    const pos = sample.positions;
    const val = sample.values;
    if (!pos || !val) return out;
    const M = N - 1;
    for (let s = 0, p = 0; s < sample.count; s++, p += 3) {
        const ix = (pos[p]     - 0.5) | 0;
        const iy = (pos[p + 1] - 0.5) | 0;
        const iz = (pos[p + 2] - 0.5) | 0;
        let row, col;
        if (axis === 0) {
            if (ix !== mid) continue;
            col = iy; row = M - iz;
        } else if (axis === 1) {
            if (iy !== mid) continue;
            col = ix; row = M - iz;
        } else {
            if (iz !== mid) continue;
            col = ix; row = M - iy;
        }
        if (col < 0 || col >= N || row < 0 || row >= N) continue;
        out[row * N + col] = val[s];
    }
    return out;
}

/**
 * Transpose + Y-flip an N×N Float64Array (row-major) coming from
 * bridge.getFluxSlice.
 *
 * The bridge's layout is `data[a*N + b]` with (a, b) = (lattice-fast-axis,
 * lattice-slow-axis), the OPPOSITE of the sparse-sample helpers' layout.
 * In addition the panels apply Y-up (canvas row = N-1-axis_value) so the
 * heatmap matches the Three.js viewport. Both rewrites happen here in
 * a single pass so the paint code is source-agnostic.
 *
 * Mapping (using the bridge's getFluxSlice convention):
 *   - axis 0 (yz, x fixed): bridge → buf[y*N + z];
 *     panel target col=y, row=N-1-z  ⇒ out[(N-1-c)*N + r] = buf[r*N + c]
 *   - axis 1 (xz, y fixed): bridge → buf[x*N + z];
 *     panel target col=x, row=N-1-z  ⇒ out[(N-1-c)*N + r] = buf[r*N + c]
 *   - axis 2 (xy, z fixed): bridge → buf[x*N + y];
 *     panel target col=x, row=N-1-y  ⇒ out[(N-1-c)*N + r] = buf[r*N + c]
 *
 * All three axes share the same rewrite: out[(N-1-c)*N + r] = buf[r*N + c].
 */
export function transposeAndFlipNN(buf, N) {
    if (!buf || buf.length !== N * N) return buf;
    const out = new Float64Array(N * N);
    const M = N - 1;
    for (let r = 0; r < N; r++) {
        for (let c = 0; c < N; c++) {
            out[(M - c) * N + r] = buf[r * N + c];
        }
    }
    return out;
}

/**
 * Rasterize a compute*Frame() return value ({positions, values, count} |
 * null) from overlay-frames.js onto the N×N mid-plane grid. Structurally
 * parallel to sliceScalarSigned (identical voxel-center-float decode, same
 * row/col/flip convention — see sliceVectorMag's docstring for the panel-
 * axis + Y-up convention) but kept as its own function because the INPUT
 * CONTRACT is different: a raw bridge sampler payload vs. a derived-overlay
 * frame object that may also carry `normalizer` / `signed` / `dualAvailable`
 * / `threshold` metadata this panel does not consume (the panel runs its
 * OWN independent per-row rolling autoscale — see _fieldGlobalMax — rather
 * than trusting a compute function's own `normalizer`).
 *
 * @param {{positions:Float32Array, values:Float32Array, count:number}|null} frame
 * @param {0|1|2} axis  0 → x=mid (yz plane); 1 → y=mid (xz); 2 → z=mid (xy)
 * @param {number} mid
 * @param {number} N
 * @returns {Float64Array} N*N scalar values (0 where the frame has no sample)
 */
export function sliceDerivedFrame(frame, axis, mid, N) {
    const out = new Float64Array(N * N);
    if (!frame || !frame.count) return out;
    const pos = frame.positions;
    const val = frame.values;
    if (!pos || !val) return out;
    const M = N - 1;
    for (let s = 0, p = 0; s < frame.count; s++, p += 3) {
        const ix = (pos[p]     - 0.5) | 0;
        const iy = (pos[p + 1] - 0.5) | 0;
        const iz = (pos[p + 2] - 0.5) | 0;
        let row, col;
        if (axis === 0) {
            if (ix !== mid) continue;
            col = iy; row = M - iz;
        } else if (axis === 1) {
            if (iy !== mid) continue;
            col = ix; row = M - iz;
        } else {
            if (iz !== mid) continue;
            col = ix; row = M - iy;
        }
        if (col < 0 || col >= N || row < 0 || row >= N) continue;
        out[row * N + col] = val[s];
    }
    return out;
}

/**
 * Wrap a 3-stop FORCE_PALETTES entry as a ramp(t,out,i) function via the
 * existing lerpPalette(pal,t) interpolator (which returns a fresh [r,g,b]
 * tuple rather than writing into out/i — this is glue around an existing
 * export, not a new color model). Keeps each force row visually matched to
 * its 3D Forces-column counterpart (same palette, same interpolation).
 */
export function forcePaletteRamp(paletteKey) {
    const pal = FORCE_PALETTES[paletteKey];
    return (t, out, i) => {
        const c = lerpPalette(pal, t);
        out[i] = c[0]; out[i + 1] = c[1]; out[i + 2] = c[2];
    };
}

/**
 * Wrap an unsigned ramp so a value that is always <= 0 (e.g. the
 * gravPotential JS-fallback "well" proxy) still spans the ramp's full
 * color range by magnitude. rampGravWell's own first line is
 * `t = Math.max(0, Math.min(1, t))` — it silently clamps any negative
 * input to its t=0 endpoint, so without this wrapper every well would
 * paint the SAME color regardless of depth. The 3D topology-sheet
 * renderer already guards against exactly this (it abs()-es before
 * calling rampGravWell) — this mirrors that existing convention rather
 * than inventing a new one.
 */
export function absThenRamp(baseRamp) {
    return (t, out, i) => baseRamp(Math.abs(t), out, i);
}

export function axisIndex(axis) {
    if (axis === 'yz') return 0;
    if (axis === 'xz') return 1;
    return 2; // xy
}
