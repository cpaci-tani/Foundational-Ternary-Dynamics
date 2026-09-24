/**
 * Gravity Observatory — Scale-0 gravity-field instrument.
 *
 * Three sections:
 *   ① Gravity field slices — per-axis (yz/xz/xy) 2D heatmaps of a selected gravity
 *      quantity (normalized latency proxy L_p / curvature proxy K_p /
 *      slice-force proxy |F_p| / proxy lapse deficit L_p²).
 *      gravitational wave propagate across the planes.
 *   ② Gravity telemetry — L/K/|F| stats, peak time-dilation, horizon proximity,
 *      GW strain, G_N / α_G, + spatial histograms.
 *   ③ Live Δ-trace — sparklines of the gravity metrics over recent field updates,
 *      plus "Δ since last observation". Mutate a field (inject / toggle /
 *      seed) and watch the next accepted gravity observation respond.
 *
 * Phase 1 is the WEB **proxy** (|J|²-derived). The engine's Poisson-derived,
 * [IMPOSED] latency map is surfaced separately in Phase 2, tagged [ENGINE].
 * Proxy readouts retain their [M]/[D] labels and are never presented as the
 * engine latency record or as recovered spacetime geometry.
 * Modelled on spectrum-panel.js. See .claude/plans/let-s-plan-for-and-eager-tide.md.
 */

import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { cardStyle, titleStyle, heroStyle, tagBadge, formatExp, formatFixed } from './_card-helpers.js';
import { paintSliceToCanvas } from './slice-render.js';
import { prepareGravitySlabSlice, prepareGravityVolumeSlice } from './gravity-slice-view.js';
import { readGravityObservation } from './gravity-observation.js';
import { renderDelta, histogramPath, histogramAriaLabel } from './gravity-chart-view.js?v=1';
import {
    aggregateMetrics,
    forceMagnitudes,
    maxRhoOf,
} from '../../analysis/gravity-analysis.js?v=4';
import {
    getScale0State,
    isScale0AuthoritativeGenerationReady,
    resolveActiveScale0BridgeFromWindow,
    subscribeScale0Qualification,
} from '../../state/store.js';
import {
    isPanelLive,
    PANEL_VISIBILITY_CHANGE_EVENT,
} from '../../../../ui/panels/panel-visibility.js?v=2';
import { rampViridis, rampEmEnergy, rampVorticity } from '../../../../viewport/color-ramps.js';
import { TickHistoryControl } from '../../../../ui/charts/history-window.js';
import { projectHistoryIndices } from '../../../../ui/charts/history-index.js';
import { ScenarioApplicabilityBinding } from '../../../../ui/utils/scenario-applicability-binding.js';

const PANEL_ID = 'gravity-panel';
// Worker observations already own bounded slabs and scalar samples. Consume
// every fresh completed state at display cadence; a second wall-clock throttle
// visibly steps both the heatmaps and the four history traces.
const DISPLAY_HZ = 60;
const LEGACY_READ_INTERVAL_MS = 250;
const DIRECT_MAX_DUTY = 0.25;
const EMPTY_SCENARIO_ID = 'empty';
const MAX_DIRECT_DENSE_SLICE_L = 48;

// Load the panel stylesheet via a JS-injected (async, NON-render-blocking) link
// instead of a <head> <link>, and only on first show. A render-blocking <link>
// measurably delayed first paint enough to flake the tight scale-switch timing;
// this keeps the .css the source of truth while off the critical boot path.
function ensureGravityCss() {
    if (typeof document === 'undefined' || document.getElementById('gravity-panel-css')) return;
    const l = document.createElement('link');
    l.id = 'gravity-panel-css';
    l.rel = 'stylesheet';
    l.href = 'css/ui/components/gravity-panel.css?v=2';
    document.head.appendChild(l);
}
// Preserve dense sampling on small lattices, but keep the number of sampled
// records bounded as N grows. These samples drive summary statistics only;
// the three rendered mid-plane slices still use the authoritative volume at
// its own dense/native compact resolution. L=97 stride 6 reduces each scalar
// reduction from ~118K records to ~5K without changing the engine state.
function gravityTelemetryStride(latticeSize) {
    const L = Math.max(1, Math.trunc(Number(latticeSize) || 1));
    if (L >= 145) return 8;
    if (L >= 97) return 6;
    if (L >= 65) return 3;
    return 2;
}
const SPARK_MAX = 60;      // Δ-trace rolling-window length
const TILE_PX = 116;       // slice canvas size

export function gravitySliceMidIndex(latticeSize, axisCount, spacing = 1, origin = 0) {
    const count = Math.max(1, Math.trunc(Number(axisCount) || 1));
    const h = Math.max(1, Number(spacing) || 1);
    const start = Number.isFinite(Number(origin)) ? Number(origin) : 0;
    const center = Math.max(0, Math.trunc(Number(latticeSize) || 1) >> 1);
    return Math.max(0, Math.min(count - 1, Math.round((center - start) / h)));
}

const QUANTITIES = [
    { kind: 'latency',     label: 'Lₚ', name: 'Normalized latency proxy', ramp: rampViridis,
      help: 'Web proxy Lₚ = √(|J|²/max|J|²). It is a normalized field-shape observable, not the native Poisson-derived latency record.' },
    { kind: 'kretschmann', label: 'Kₚ', name: 'Curvature proxy', ramp: rampEmEnergy,
      help: 'Web proxy Kₚ = (∇²Lₚ)². It is not the full Riemann-tensor Kretschmann invariant.' },
    { kind: 'force',       label: '|Fₚ|', name: 'Slice proxy force', ramp: rampVorticity,
      help: 'Presentation-only slice proxy |Fₚ| = G_N·|∇₁|J|| on the displayed grid. It is distinct from the selected radius-2 engine force sampler.' },
    { kind: 'dilation',    label: '1−fₚ', name: 'Proxy lapse deficit', ramp: rampViridis,
      help: 'Derived proxy lapse deficit 1−fₚ = Lₚ². It is not a native clock measurement.' },
];
const AXES = [
    { axis: 0, tag: 'yz' },   // x = mid
    { axis: 1, tag: 'xz' },   // y = mid
    { axis: 2, tag: 'xy' },   // z = mid
];

const SECTION_HELP = {
    slices: 'Per-axis 2D slices through the lattice mid-planes (yz / xz / xy). Lₚ, Kₚ, |Fₚ|, and 1−fₚ are explicitly named web proxies derived from displayed |J|; they are not the native Poisson-derived latency record or a full curvature tensor.',
    telemetry: 'Sampled scalar gravity telemetry. L and K are |J|²-derived web proxies. The engine-force row identifies the exact selected radius-2 branch and whether both the forces umbrella and gravity channel enable it; the sampler evaluates regular support sites, while phase_forces applies the branch only at manifested sites. A disabled branch is explicitly counterfactual. Values belong to the stated regular-grid stride, not necessarily the full lattice. The bottom block is the engine Poisson-derived [IMPOSED] latency map (voxel.latency), shown when the engine runs it.',
    delta: 'How gravity RESPONDS as you mutate fields. Sparklines track L_max / K_max / |F|_mean / dilation% over accepted panel sample cycles; "Δ since last observation" compares the previous accepted observation with the current one. Fresh worker observations are displayed together; each plotted point retains its measured simulation tick.',
};

// ── compute ──────────────────────────────────────────────────────────────────

function readGravityMetrics(caps, stride) {
    const samplerKeys = [
        ['latency', stride],
        ['kretschmann', stride],
        ['gravity', stride],
    ];
    const latency = caps.getScale0FieldSamples?.({ kind: 'latency', stride }) || { values: [], count: 0 };
    const kret = caps.getScale0FieldSamples?.({ kind: 'kretschmann', stride }) || { values: [], count: 0 };
    const force = caps.getScale0ForceField?.('gravity', stride) || { vectors: [], count: 0 };
    const latencyCount = latency.count | 0;
    const kretCount = kret.count | 0;
    const forceCount = force.count | 0;
    const forceMags = forceMagnitudes(force.vectors || [], forceCount);
    return {
        // Transport readiness is explicit: count=0 may be a legitimate sampled
        // result and must not be conflated with a lazy cache miss.
        ready: samplerKeys.every(([kind, sampleStride]) => (
            caps.hasScale0SamplerSnapshot?.(kind, sampleStride) !== false
        )),
        snapshotVersions: samplerKeys.map(([kind, sampleStride]) => (
            caps.getScale0SamplerSnapshotVersion?.(kind, sampleStride) ?? null
        )),
        metrics: aggregateMetrics({
            latencyVals: latency.values, latencyCount,
            kretVals: kret.values, kretCount,
            forceMags, forceCount,
        }),
    };
}

function readDirectWasmGravityForce(caps, stride) {
    const force = caps.getScale0ForceField?.('gravity', stride) || { vectors: [], count: 0 };
    const forceCount = force.count | 0;
    return {
        ready: caps.hasScale0SamplerSnapshot?.('gravity', stride) !== false,
        snapshotVersion: caps.getScale0SamplerSnapshotVersion?.('gravity', stride) ?? null,
        forceMags: forceMagnitudes(force.vectors || [], forceCount),
        forceCount,
    };
}

function readDirectWasmGravityMetrics(caps, stride) {
    // Embind can reuse its result storage on the next call. Snapshot each
    // bounded sampler immediately; never retain a native view across calls.
    const readScalar = (kind) => {
        const sample = caps.getScale0FieldSamples?.({ kind, stride }) || { values: [], count: 0 };
        const count = Math.max(0, Math.min(sample.count | 0, sample.values?.length || 0));
        return { values: Float64Array.from(sample.values?.subarray?.(0, count) ?? []), count };
    };
    const latency = readScalar('latency');
    const kret = readScalar('kretschmann');
    const forceSample = readDirectWasmGravityForce(caps, stride);
    return {
        // Direct WASM reports every bounded sampler synchronously.
        ready: typeof caps.getScale0FieldSamples === 'function' && forceSample.ready,
        snapshotVersions: [null, null, forceSample.snapshotVersion],
        metrics: aggregateMetrics({
            latencyVals: latency.values,
            latencyCount: latency.count,
            kretVals: kret.values,
            kretCount: kret.count,
            forceMags: forceSample.forceMags,
            forceCount: forceSample.forceCount,
        }),
    };
}

export function samplerVersionsAdvanced(previous, next) {
    if (!Array.isArray(previous) || !Array.isArray(next)
        || previous.length !== next.length
        || !previous.every(Number.isFinite)
        || !next.every(Number.isFinite)) return false;
    return next.every((version, index) => version > previous[index]);
}

// ── small render helpers ──────────────────────────────────────────────────────

function setText(node, value) {
    const text = String(value);
    if (!node || node.textContent === text) return;
    const child = node.firstChild;
    if (node.childNodes.length === 1 && child?.nodeType === Node.TEXT_NODE) {
        child.data = text;
    } else {
        node.textContent = text;
    }
}

function setHidden(node, hidden) {
    if (node && node.hidden !== hidden) node.hidden = hidden;
}

function setAttr(node, name, value) {
    if (!node) return;
    const text = String(value);
    if (node.getAttribute(name) !== text) node.setAttribute(name, text);
}

function telemetryRowTemplate(key, label, tag, tip) {
    return `<div class="grav-row"><span class="grav-row-l" title="${tip}">${tagBadge(tag)}${label}</span><span class="grav-row-v" data-grav-value="${key}">—</span></div>`;
}

function forceLawPresentation(bridge) {
    const forcesEnabled = bridge?.getToggle?.('forces');
    const gravityEnabled = bridge?.getToggle?.('gravity');
    const geometric = bridge?.getToggle?.('geometric_gravity');
    if (forcesEnabled === false || gravityEnabled === false) {
        const disabled = [
            forcesEnabled === false ? 'forces umbrella' : null,
            gravityEnabled === false ? 'gravity channel' : null,
        ].filter(Boolean).join(' and ');
        return {
            label: 'Engine force sampler · not applied',
            title: `The ${disabled} toggle is OFF. The sampled radius-2 gradient is a counterfactual field readout and is not applied by phase_forces.`,
        };
    }
    if (forcesEnabled === true && gravityEnabled === true && geometric === true) return {
        label: 'Engine force branch active · Mᵢc²L·∇₂L',
        title: 'The sampler evaluates M_inertial·c²·L·∇₂L on regular support sites using the periodic radius-2 central-difference stencil; phase_forces applies this enabled branch only at manifested sites.',
    };
    if (forcesEnabled === true && gravityEnabled === true) return {
        label: 'Engine force branch active · G_N·∇₂|J|',
        title: 'The sampler evaluates G_N·∇₂|J| on regular support sites using the periodic radius-2 central-difference stencil; phase_forces applies this enabled branch only at manifested sites.',
    };
    return {
        label: 'Engine force sampler · state unavailable',
        title: 'The finite engine-force sample is available, but this bridge did not expose authoritative gravity-toggle state.',
    };
}

function latencyRequestState(bridge, aggregate) {
    if (typeof aggregate?.requested === 'boolean') return aggregate.requested;
    const latencyField = bridge?.getToggle?.('latency_field');
    const fieldEnergyGravity = bridge?.getToggle?.('field_energy_gravity');
    if (typeof latencyField === 'boolean' && typeof fieldEnergyGravity === 'boolean') {
        return latencyField || fieldEnergyGravity;
    }
    return null;
}

// ── section renderers ─────────────────────────────────────────────────────────

function renderTelemetry(
    container,
    m,
    agg,
    stride = 1,
    aggReady = false,
    snapshotVersions = [],
    forceLaw = forceLawPresentation(null),
    latencyRequested = null,
    sampleTick = null,
) {
    if (!container._gravityTelemetryView) {
        container.innerHTML = `
            <div data-grav-warning style="${heroStyle()};color:var(--negative-text);margin-bottom:6px;visibility:hidden;">⚠ horizon</div>
            ${telemetryRowTemplate('latency', `Normalized proxy Lₚ (sampled mean / max; h=${stride})`, 'M', `Regular-grid Lₚ=√(|J|²/max|J|²) sample at stride h=${stride}; an off-grid extremum is not observed.`)}
            ${telemetryRowTemplate('kretschmann', `Curvature proxy Kₚ (sampled mean / max; h=${stride})`, 'M', `Regular-grid Kₚ=(∇²Lₚ)² sample at stride h=${stride}; it is not the full Kretschmann invariant.`)}
            <div class="grav-row"><span class="grav-row-l" data-grav-force-label-wrap title="${forceLaw.title}">${tagBadge('ENGINE')}<span data-grav-force-label>${forceLaw.label}</span> <span> (support-site sample mean / max; h=${stride})</span></span><span class="grav-row-v" data-grav-value="force">—</span></div>
            ${telemetryRowTemplate('dilation', 'Proxy lapse slowdown (peak)', 'D', 'Derived proxy 1−√(1−Lₚ,max²); not a native clock measurement.')}
            ${telemetryRowTemplate('horizon', 'Proxy clamp proximity (Lₚ,max)', 'D', 'How close the normalized web proxy is to its imposed Lₚ→1 display clamp; not an event-horizon detection.')}
            ${telemetryRowTemplate('strain', 'Field-contrast proxy', 'M', 'Lₚ,max − Lₚ,mean; a normalized field-shape contrast, not tensor gravitational-wave strain.')}
            ${telemetryRowTemplate('gn', 'G_N lattice coupling', 'I', '[IMPOSED] Scale-0 lattice-toy coupling in engine units; it is not physical Newton G.')}
            ${telemetryRowTemplate('alpha-g', 'α_G physical reference', 'T', 'External gravitational fine-structure reference for scale context; it is not produced by this Scale-0 run.')}
            <div class="grav-hist-row">
                <span>L <svg viewBox="0 0 70 20" preserveAspectRatio="none" class="grav-mini-hist" role="img" aria-label="Sample distribution"><path data-grav-hist="L" fill="var(--accent)" opacity="0.7"/></svg><small data-grav-hist-range="L"></small></span>
                <span>K <svg viewBox="0 0 70 20" preserveAspectRatio="none" class="grav-mini-hist" role="img" aria-label="Sample distribution"><path data-grav-hist="K" fill="var(--accent)" opacity="0.7"/></svg><small data-grav-hist-range="K"></small></span>
                <span>|F| <svg viewBox="0 0 70 20" preserveAspectRatio="none" class="grav-mini-hist" role="img" aria-label="Sample distribution"><path data-grav-hist="F" fill="var(--accent)" opacity="0.7"/></svg><small data-grav-hist-range="F"></small></span>
            </div>
            <div data-grav-sampler-provenance style="font-size:16px;color:var(--text-muted);padding:4px 0;"></div>
            <div data-grav-cpp-heading style="margin:8px 0 4px;padding-top:6px;border-top:1px solid var(--border-subtle,rgba(255,255,255,.09));font-size:16px;font-weight:600;color:var(--text-secondary);" title="Engine path: ∇²φ_latency = 4πG_N(ρ−ρ̄), then L=√clamp(−φ_latency, 0, LATENCY_HORIZON_CLAMP). [IMPOSED] ρ contains M_GRAVITATIONAL|s| plus optional ½(|J|²+|wave_vel|²) and optional selected strong T00/c². This is an engine mapping, not a derivation of spacetime geometry, and is distinct from the normalized web-proxy rows above.">Engine latency map (Poisson-derived; [IMPOSED]) ⓘ</div>
            <div data-grav-cpp-inactive style="font-size:16px;color:var(--text-muted);padding:3px 0;">${tagBadge('ENGINE')}<span data-grav-cpp-status>awaiting current aggregate</span></div>
            <div data-grav-cpp-active hidden>
                ${telemetryRowTemplate('cpp-latency', 'Mapped L (mean / max)', 'ENGINE', 'Engine voxel.latency after the [IMPOSED] Poisson mapping; it is not φ_latency itself or recovered spacetime geometry. The active source can contain imposed manifested mass, optional field/wave energy, and optional selected strong T00/c².')}
                ${telemetryRowTemplate('cpp-lapse', 'Mapped lapse f_min', 'ENGINE', 'Minimum mapped lapse readout f = 1 − L_max² from the [IMPOSED] engine latency map; not an independently derived physical clock measurement.')}
                ${telemetryRowTemplate('cpp-dilation', 'Mapped lapse slowdown (peak)', 'ENGINE', '(1 − √f_min)·100 from the [IMPOSED] engine latency map; not a claim of recovered physical time dilation.')}
                ${telemetryRowTemplate('cpp-gamma', 'γ_ftd (max)', 'ENGINE', 'Maximum engine transport factor γ_ftd = 1/√(1−L²−|v|²/C_SPEED²) from the imposed causal-budget mapping.')}
                ${telemetryRowTemplate('cpp-voxels', 'Mapped cells', 'ENGINE', 'Cells carrying nonzero mapped engine latency.')}
            </div>`;
        const values = Object.fromEntries([...container.querySelectorAll('[data-grav-value]')]
            .map((node) => [node.dataset.gravValue, node]));
        container._gravityTelemetryView = {
            warning: container.querySelector('[data-grav-warning]'),
            values,
            histL: container.querySelector('[data-grav-hist="L"]'),
            histK: container.querySelector('[data-grav-hist="K"]'),
            histF: container.querySelector('[data-grav-hist="F"]'),
            samplerProvenance: container.querySelector('[data-grav-sampler-provenance]'),
            forceLabel: container.querySelector('[data-grav-force-label]'),
            forceLabelWrap: container.querySelector('[data-grav-force-label-wrap]'),
            cppInactive: container.querySelector('[data-grav-cpp-inactive]'),
            cppStatus: container.querySelector('[data-grav-cpp-status]'),
            cppActive: container.querySelector('[data-grav-cpp-active]'),
        };
    }
    const view = container._gravityTelemetryView;
    const horizonColor = m.horizon >= 0.95 ? 'var(--negative-text)' : m.horizon >= 0.5 ? 'var(--caution-text)' : 'var(--positive-text)';
    setText(view.warning, `⚠ proxy clamp — Lₚ,max ${formatFixed(m.horizon, 3)}`);
    const warningVisibility = m.horizon >= 0.95 ? 'visible' : 'hidden';
    if (view.warning.style.visibility !== warningVisibility) view.warning.style.visibility = warningVisibility;
    setText(view.values.latency, `${formatFixed(m.L.mean, 3)} / ${formatFixed(m.L.max, 3)}`);
    setText(view.values.kretschmann, `${formatExp(m.K.mean)} / ${formatExp(m.K.max)}`);
    setText(view.values.force, `${formatExp(m.F.mean)} / ${formatExp(m.F.max)}`);
    setText(view.forceLabel, forceLaw.label);
    setAttr(view.forceLabelWrap, 'title', forceLaw.title);
    setText(view.values.dilation, `${formatFixed(m.dilationPct, 3)} %`);
    setText(view.values.horizon, formatFixed(m.horizon, 3));
    if (view.values.horizon.style.color !== horizonColor) view.values.horizon.style.color = horizonColor;
    setText(view.values.strain, formatExp(m.strain));
    setText(view.values.gn, formatFixed(m.gnG, 3));
    setText(view.values['alpha-g'], m.alphaG.toExponential(2));
    setAttr(view.histL, 'd', histogramPath(m.histL));
    setAttr(view.histK, 'd', histogramPath(m.histK));
    setAttr(view.histF, 'd', histogramPath(m.histF));
    for (const key of ['L', 'K', 'F']) {
        const histogram = m[`hist${key}`];
        setAttr(view[`hist${key}`].parentNode, 'aria-label', histogramAriaLabel(key, histogram));
        setText(container.querySelector(`[data-grav-hist-range="${key}"]`),
            `${formatExp(histogram.min).trim()}–${formatExp(histogram.max).trim()}`);
    }

    const finiteVersions = snapshotVersions
        .filter((version) => version !== null && Number.isFinite(Number(version)))
        .map(Number);
    if (Number.isSafeInteger(sampleTick) && sampleTick >= 0) {
        setText(view.samplerProvenance, `Sample tick ${sampleTick} · shared observation · grid stride h=${stride}.`);
    } else if (!finiteVersions.length) {
        setText(view.samplerProvenance, `Sampler provenance: synchronous current snapshot; regular-grid stride h=${stride}.`);
    } else {
        const minVersion = Math.min(...finiteVersions);
        const maxVersion = Math.max(...finiteVersions);
        const revision = minVersion === maxVersion
            ? `transport revision ${minVersion}`
            : `asynchronous transport revisions ${minVersion}–${maxVersion}`;
        setText(view.samplerProvenance, `Sampler provenance: ${revision}; regular-grid stride h=${stride}.`);
    }

    const cppActive = aggReady && !!agg?.active;
    setHidden(view.cppInactive, cppActive);
    setHidden(view.cppActive, !cppActive);
    if (!cppActive) {
        let status = 'awaiting current aggregate — proxy telemetry remains available';
        if (aggReady && latencyRequested === true) {
            status = 'requested — no nonzero latency cells in this engine observation';
        } else if (aggReady && latencyRequested === false) {
            status = 'inactive — Poisson-latency operator not requested';
        } else if (aggReady) {
            status = 'inactive — Poisson-latency request state unavailable';
        }
        setText(view.cppStatus, status);
    }
    if (cppActive) {
        setText(view.values['cpp-latency'], `${formatExp(agg.latencyMean)} / ${formatExp(agg.latencyMax)}`);
        setText(view.values['cpp-lapse'], formatFixed(agg.fMin, 5));
        setText(view.values['cpp-dilation'], `${formatExp(agg.dilationMaxPct)} %`);
        setText(view.values['cpp-gamma'], formatFixed(agg.gammaMax, 5));
        setText(view.values['cpp-voxels'], String(agg.voxelCount));
    }
}

// ── panel shell ───────────────────────────────────────────────────────────────

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.className = 'scale0-only gravity-panel';
    root.dataset.applicability = 'applicable';
    const qbtns = QUANTITIES.map((q, i) =>
        `<button type="button" class="grav-qbtn${i === 0 ? ' active' : ''}" aria-pressed="${i === 0}" data-kind="${q.kind}" title="${q.help}">${q.label}</button>`).join('');
    const tiles = AXES.map((a) =>
        `<div class="grav-tile"><canvas id="${PANEL_ID}-tile-${a.axis}" width="${TILE_PX}" height="${TILE_PX}" role="img" aria-label="${a.tag} gravity slice awaiting observation"></canvas><div class="grav-tile-meta"><span>${a.tag}</span><span id="${PANEL_ID}-tick-${a.axis}" class="grav-tile-tick">tick —</span></div><span id="${PANEL_ID}-rd-${a.axis}" class="grav-tile-readout grav-tile-range">—</span></div>`).join('');
    root.innerHTML = `
        <div class="gravity-applicable-content">
            <header class="grav-header">
                <span class="grav-title">Gravity Observatory</span>
                <span class="grav-mode" id="${PANEL_ID}-mode" title="Web proxy gravity (|J|²-derived). The engine's Poisson-derived [IMPOSED] latency map is shown separately in Phase 2.">proxy</span>
            </header>
            <section style="${cardStyle(210)}">
                <div style="${titleStyle()}" title="${SECTION_HELP.slices}">Gravity field slices ⓘ</div>
                <div class="grav-qsel" role="group" aria-label="Gravity field quantity" id="${PANEL_ID}-qsel">${qbtns}</div>
                <div class="grav-observation-status" role="status">Waiting for a complete observation…</div>
                <div class="grav-slice-tiles">${tiles}</div>
                <div class="grav-slice-legend"><span class="grav-legend-label"></span><span class="grav-color-ramp" aria-hidden="true"></span><span class="grav-legend-range"></span></div>
            </section>
            <section style="${cardStyle(220)}">
                <div style="${titleStyle()}" title="${SECTION_HELP.telemetry}">Gravity telemetry ⓘ</div>
                <div id="${PANEL_ID}-telemetry"></div>
            </section>
            <section style="${cardStyle(170)}">
                <div style="${titleStyle()}" title="${SECTION_HELP.delta}">Live Δ-trace ⓘ</div>
                <div id="${PANEL_ID}-delta"></div>
            </section>
        </div>
        <section class="mode-unavailable gravity-inapplicable"
                 data-applicability="inapplicable" role="status" hidden>
            <strong>Not applicable — imposed null control</strong>
            <p>Scenario 1 · Empty does not define a gravity-source, metric, or
               gravity-proxy domain. No gravity field sampling, slice analysis,
               telemetry history, or gravitational inference is performed.</p>
            <p>This imposed null control is not a measurement of physical
               vacuum, inert vacuum, zero-point energy, or spacetime curvature.</p>
        </section>
        <section class="mode-unavailable gravity-pending"
                 data-applicability="unavailable" role="status" hidden>
            <strong>Gravity observation pending</strong>
            <p class="gravity-pending-detail">Waiting for the current scenario
               generation to cross its authoritative engine setup barrier.</p>
            <p>No proxy slices, aggregate, history, sampler demand, or gravity
               inference is published from a pending, failed, or superseded load.</p>
        </section>
    `;
    return root;
}

export function mountGravityPanel(host, getBridge) {
    if (!host) return null;
    document.getElementById(PANEL_ID)?.remove();
    const panel = buildPanel();
    host.appendChild(panel);

    const el = (id) => panel.querySelector(`#${PANEL_ID}-${id}`);
    const telBody = el('telemetry'), deltaBody = el('delta'), modeEl = el('mode');
    const tiles = AXES.map((a) => ({ axis: a.axis, tag: a.tag, canvas: el(`tile-${a.axis}`), readout: el(`rd-${a.axis}`), tickLabel: el(`tick-${a.axis}`) }));
    const applicableContent = panel.querySelector('.gravity-applicable-content');
    const observationStatus = panel.querySelector('.grav-observation-status');
    const legend = panel.querySelector('.grav-slice-legend');
    const inapplicableMessage = panel.querySelector('.gravity-inapplicable');
    const pendingMessage = panel.querySelector('.gravity-pending');
    const pendingDetail = panel.querySelector('.gravity-pending-detail');
    const quantityButtons = [...panel.querySelectorAll('.grav-qbtn')];
    const historyControl = new TickHistoryControl(applicableContent, {
        id: 'gravity-panel',
        defaultTicks: SPARK_MAX,
        onChange: () => {
            if (lastMetrics) {
                renderDelta(deltaBody, visibleHistory(), latched, lastMetrics);
            }
        },
    });

    let activeKind = 'latency';
    let lastMetrics = null;
    let lastAgg = null;
    let lastObservation = null;
    let lastObservationStamp = null;
    let lastSliceSnapshot = null;
    let sampleTick = null;
    let bridgeId = null;
    let history = [];     // [{ ver, Lmax, Kmax, Fmean, dil }]
    let historyGeneration = 0;
    const historyBuffers = ['Lmax', 'Kmax', 'Fmean', 'dil'].map(key => ({
        get count() { return history.length; },
        get total() { return history.length; },
        get generation() { return historyGeneration; },
        get: index => history[index]?.[key],
        getTick: index => history[index]?.tick,
    }));
    function visibleHistory() {
        const count = historyControl.visibleCount(historyBuffers[0]);
        const indices = projectHistoryIndices(historyBuffers, count, 116);
        return indices ? indices.map(index => history[index]) : history.slice(history.length - count);
    }
    let latched = null;   // metric vector at the previous accepted sample cycle
    let lastVer = -1;
    let lastComputedVer = -1;
    let lastSamplerVersions = null;
    let lastHadVolume = false;
    let inapplicable = false;
    let disposed = false;
    let armSub = null;
    let liveSub = null;
    let samplerWantSignature = '';
    let samplerBridge = null;
    let scenarioBinding = null;
    let unsubscribeQualification = null;
    let qualifiedLoadGeneration = null;

    function setKind(kind) {
        if (!QUANTITIES.some(quantity => quantity.kind === kind)
            || inapplicable || panel.dataset.applicability !== 'applicable') return;
        activeKind = kind;
        for (const button of quantityButtons) {
            button.classList.toggle('active', button.dataset.kind === kind);
            button.setAttribute('aria-pressed', String(button.dataset.kind === kind));
        }
        // A presentation change renders the retained complete observation;
        // it cannot pair later slice pixels with earlier telemetry/history.
        const caps = getCaps();
        if (caps && lastSliceSnapshot) paintSlices(caps, { snapshot: lastSliceSnapshot });
    }
    panel.querySelector(`#${PANEL_ID}-qsel`).addEventListener('click', event => {
        const button = event.target.closest('.grav-qbtn');
        if (button) setKind(button.dataset.kind);
    });

    function getCaps() {
        const b = getBridge?.();
        return b?.capabilities?.scale0 || null;
    }

    function releaseSamplerWants(force = false) {
        if (!force && !samplerWantSignature && !samplerBridge) return;
        const target = samplerBridge || getBridge?.();
        target?.replaceSamplerWants?.(PANEL_ID, []);
        samplerWantSignature = '';
        samplerBridge = null;
    }

    function setSamplerWants(bridge, keys) {
        const signature = keys.join('|');
        if (samplerBridge === bridge && samplerWantSignature === signature) return;
        if (samplerBridge && samplerBridge !== bridge) {
            samplerBridge.replaceSamplerWants?.(PANEL_ID, []);
        }
        bridge?.replaceSamplerWants?.(PANEL_ID, keys);
        samplerBridge = bridge || null;
        samplerWantSignature = signature;
    }

    function resetAnalysisState() {
        lastMetrics = null;
        lastAgg = null;
        lastObservation = null;
        lastObservationStamp = null;
        lastSliceSnapshot = null;
        sampleTick = null;
        delete panel.dataset.sampleTick;
        delete telBody.dataset.sampleTick;
        delete deltaBody.dataset.sampleTick;
        setText(observationStatus, 'Waiting for a complete observation…');
        legend.hidden = true;
        bridgeId = null;
        history = [];
        historyGeneration++;
        latched = null;
        lastVer = -1;
        lastComputedVer = -1;
        lastSamplerVersions = null;
        lastHadVolume = false;
        // Never reveal a previous scenario generation while the new transport
        // is still filling its lazy sampler caches. Neutralize every rendered
        // scientific surface at the same boundary that retires the JS values.
        panel.dataset.telemetryState = 'waiting-samplers';
        applicableContent.setAttribute('aria-busy', 'true');
        telBody.innerHTML = '<div class="grav-empty">Waiting for current-generation gravity samplers…</div>';
        telBody._gravityTelemetryView = null;
        deltaBody.innerHTML = '<div class="grav-empty">Waiting for current-generation gravity samplers…</div>';
        deltaBody._gravityDeltaView = null;
        setText(modeEl, 'proxy · waiting');
        modeEl.title = 'Waiting for current-generation sampler and support-grid provenance.';
        for (const tile of tiles) {
            const context = tile.canvas?.getContext?.('2d');
            context?.clearRect(0, 0, tile.canvas.width, tile.canvas.height);
            tile.readout.textContent = '—';
            delete tile.canvas.dataset.sampleTick;
            setText(tile.tickLabel, 'tick —');
        }
    }

    function paintSlices(caps, { preparedVolume = null, snapshot = null, tick = null } = {}) {
        const L = caps.latticeSize || 33;
        const quantity = QUANTITIES.find(item => item.kind === activeKind);
        let source = snapshot;
        if (!source && typeof caps.getScale0FluxSlabsWithMaxRho === 'function') {
            const mid = L >> 1;
            const batch = caps.getScale0FluxSlabsWithMaxRho(AXES.map(({ axis }) => ({ axis, index: mid })));
            if (batch) source = { ...batch, sampleTick: batch.metadata?.sampleTick ?? null };
        } else if (!source) {
            const volume = preparedVolume || caps.getScale0FluxVolume?.();
            const compact = volume && !ArrayBuffer.isView(volume) && ArrayBuffer.isView(volume.data);
            const gridN = compact ? Math.trunc(Number(volume.axisCount) || 0) : L;
            const spacing = compact ? Math.max(1, Number(volume.stride) || 1) : 1;
            const origin = compact && Number.isFinite(Number(volume.origin)) ? Number(volume.origin) : 0;
            const magnitude = compact ? volume.data : volume;
            if (ArrayBuffer.isView(magnitude) && gridN > 1 && magnitude.length >= gridN ** 3
                && (!compact || Number(volume.latticeSize) === L)) {
                // Retain an owned view so quantity changes cannot read a reused
                // native buffer or a later asynchronous visual publication.
                source = { magnitude: new Float64Array(magnitude), gridN, spacing, origin,
                    N: L, sampleTick: compact ? (volume.sampleTick ?? null) : tick };
            }
        }
        let prepared = null;
        if (source?.N === L && source.slabs?.length === 3 && Number.isFinite(source.maxRho)) {
            prepared = AXES.map(({ axis }) => {
                const slab = source.slabs[axis];
                if (slab?.axis !== axis || slab.N !== L || slab.index !== (L >> 1)) return null;
                return prepareGravitySlabSlice({ ...slab, maxRho: source.maxRho }, activeKind, L);
            });
        } else if (source?.magnitude) {
            const { magnitude, gridN, spacing, origin } = source;
            const mid = gravitySliceMidIndex(L, gridN, spacing, origin);
            const rho = maxRhoOf(magnitude, gridN ** 3);
            prepared = AXES.map(({ axis }) => prepareGravityVolumeSlice(magnitude, gridN, axis, mid, activeKind, rho, spacing));
        }
        // Stage all three before publishing any pixels. A missing plane cannot
        // leave two new tiles beside a retained tile from an older observation.
        if (!prepared?.every(plane => plane?.data?.length && Number.isFinite(plane.max))) {
            lastHadVolume = false;
            setText(observationStatus, lastSliceSnapshot ? 'Waiting for a complete observation · previous tick retained' : 'Waiting for a complete observation…');
            return false;
        }
        const gridN = source.gridN || L;
        const knownTick = Number.isSafeInteger(source.sampleTick) && source.sampleTick >= 0;
        const upper = (activeKind === 'latency' || activeKind === 'dilation') ? 1
            : Math.max(...prepared.map(plane => plane.max));
        const norm = upper > 1e-30 ? 1 / upper : 1;
        for (let i = 0; i < tiles.length; i++) {
            const tile = tiles[i], plane = prepared[i];
            if (knownTick) tile.canvas.dataset.sampleTick = String(source.sampleTick);
            else delete tile.canvas.dataset.sampleTick;
            tile.canvas.dataset.sourceEpoch = String(source.sourceEpoch ?? source.metadata?.sourceEpoch ?? '');
            setText(tile.tickLabel, knownTick ? `tick ${source.sampleTick}` : 'tick unavailable');
            tile.canvas.setAttribute('aria-label', `${quantity.name}, ${tile.tag} mid-plane, ${tile.tickLabel.textContent}, maximum ${formatExp(plane.max)}`);
            paintSliceToCanvas(tile.canvas, plane.data, gridN, { ramp: quantity.ramp, signed: false, norm });
            setText(tile.readout, `max ${formatExp(plane.max)}`);
        }
        lastSliceSnapshot = source;
        setText(modeEl, source.spacing > 1 ? `proxy · ${gridN}³ support grid` : `proxy · dense ${L}³ grid`);
        modeEl.title = 'All three mid-planes share one retained field observation and one color scale.';
        setText(observationStatus, knownTick ? `Sample tick ${source.sampleTick} · all three planes` : 'Same field snapshot · source tick unavailable');
        const colors = [], rgb = [0, 0, 0];
        for (let step = 0; step <= 8; step++) {
            quantity.ramp(step / 8, rgb, 0);
            colors.push(`rgb(${rgb.map(value => Math.round(value * 255)).join(',')}) ${step / 8 * 100}%`);
        }
        legend.style.setProperty('--gravity-ramp', `linear-gradient(to right, ${colors.join(',')})`);
        setText(legend.querySelector('.grav-legend-label'), quantity.name);
        setText(legend.querySelector('.grav-legend-range'), `0 — ${formatExp(upper)} · shared scale`);
        legend.hidden = false;
        return true;
    }

    function acceptWorkerObservation(caps, stride) {
        const raw = caps.getScale0GravityObservation(stride);
        if (raw === lastObservation) return;
        const accepted = readGravityObservation(raw, caps.latticeSize, stride);
        if (!accepted) {
            setText(observationStatus, lastObservation ? `Sample tick ${sampleTick} retained · waiting for complete observation` : 'Waiting for a complete observation…');
            return;
        }
        if (accepted.stamp === lastObservationStamp) return;
        if (lastObservation?.sourceEpoch === raw.sourceEpoch && accepted.tick < sampleTick) return;
        if (lastObservation && lastObservation.sourceEpoch !== raw.sourceEpoch) {
            history = []; latched = null; historyGeneration++;
        }
        if (!paintSlices(caps, { snapshot: raw })) return;
        lastObservation = raw;
        lastObservationStamp = accepted.stamp;
        sampleTick = accepted.tick;
        lastMetrics = accepted.metrics;
        lastAgg = accepted.aggregate;
        lastHadVolume = true;
        panel.dataset.telemetryState = 'ready';
        panel.dataset.sampleTick = String(sampleTick);
        applicableContent.setAttribute('aria-busy', 'false');
        const observedBridge = { getToggle: name => raw.engineToggles?.[name] };
        renderTelemetry(telBody, lastMetrics, lastAgg, stride, true, [],
            forceLawPresentation(observedBridge), latencyRequestState(observedBridge, lastAgg), sampleTick);
        const entry = { tick: sampleTick, ver: sampleTick, Lmax: lastMetrics.L.max,
            Kmax: lastMetrics.K.max, Fmean: lastMetrics.F.mean, dil: lastMetrics.dilationPct };
        if (history.at(-1)?.tick === sampleTick) history[history.length - 1] = entry;
        else { latched = history.at(-1) || null; history.push(entry); }
        renderDelta(deltaBody, visibleHistory(), latched, lastMetrics);
        telBody.dataset.sampleTick = String(sampleTick);
        deltaBody.dataset.sampleTick = String(sampleTick);
        setText(observationStatus, `Sample tick ${sampleTick} · slices, telemetry and traces`);
    }

    function update() {
        // Empty is an imposed null control, not a gravity or vacuum sample.
        // The scenario event normally removes both coordinators before this
        // can run; this guard keeps direct/manual calls scientifically inert.
        const scale0State = getScale0State();
        if (inapplicable || scale0State.currentScenarioId === EMPTY_SCENARIO_ID) {
            if (!inapplicable) setEmptyApplicability(true);
            return;
        }
        // Direct/manual calls must remain inert until the exact scenario-load
        // generation has crossed the canonical setup barrier. The normal
        // qualification subscription already stops the runtime synchronously;
        // this is the final fail-closed guard against a stale queued callback.
        if (panel.dataset.applicability !== 'applicable'
            || !isScale0AuthoritativeGenerationReady(scale0State)) {
            stopRuntime();
            return;
        }
        const b = getBridge?.();
        const caps = b?.capabilities?.scale0 || null;
        if (!caps) return;
        // reset trace if the bridge identity changed (scenario / scale switch)
        if (b !== bridgeId) {
            // A replacement owner can reuse the same epoch/tick numbers. Retire
            // the complete prior observation before comparing publication IDs.
            resetAnalysisState();
            bridgeId = b;
        }

        // Gate the heavy work (full-volume read + O(N³) maxRho + 3 slices +
        // samplers) on visibility — when the Gravity tab isn't shown, do nothing.
        // This keeps the panel from loading the main thread (which otherwise
        // slows scale switches) and is the established panel pattern (isPanelLive).
        if (!isPanelLive(host)) {
            releaseSamplerWants();
            return;
        }
        const stride = gravityTelemetryStride(caps.latticeSize);
        const directMainWasm = b?.isWasm === true && b?.isWorker !== true;
        setSamplerWants(b, [
            `latency@${stride}`, `kretschmann@${stride}`, `gravity@${stride}`, 'gravityMetricAgg@0',
        ]);

        if (b.isWorker && typeof caps.getScale0GravityObservation === 'function') {
            acceptWorkerObservation(caps, stride);
            return;
        }
        const currentTick = b.currentTick?.();
        const observationTick = Number.isSafeInteger(currentTick) && currentTick >= 0 && !b.isNativeGPU ? currentTick : null;
        const ver = (getScale0State()?.fieldDataVersion) | 0;
        // A native visual read is asynchronous. Do not recompute/paint a
        // stable field over and over, but keep polling while its first compact
        // volume is still in flight so a paused scenario can populate after
        // the binary response arrives.
        let sample = null;
        let repaintSlices = true;
        if (ver === lastComputedVer && lastHadVolume && lastMetrics) {
            // No asynchronous snapshot can advance behind a stable direct-WASM
            // fieldDataVersion. Re-render retained truth without repeating the
            // main-thread L^3 read/reductions while paused.
            if (directMainWasm) {
                renderTelemetry(
                    telBody,
                    lastMetrics,
                    lastAgg,
                    stride,
                    true,
                    lastSamplerVersions,
                    forceLawPresentation(b),
                    latencyRequestState(b, lastAgg),
                    observationTick,
                );
                return;
            }
            // A native/WebSocket getter returns its prior cache while enqueueing
            // the current visual revision. Poll the bounded primary samples as
            // well as the aggregate: a reply can advance all three snapshot
            // versions while fieldDataVersion remains stable (paused one-step).
            // Consume that coherent reply without rereading/repainting volume.
            sample = readGravityMetrics(caps, stride);
            if (!sample.ready
                || !samplerVersionsAdvanced(lastSamplerVersions, sample.snapshotVersions)) {
                const aggReady = caps.hasScale0SamplerSnapshot?.('gravityMetricAgg', 0) !== false;
                const agg = aggReady ? (caps.getScale0GravityMetricAgg?.() || null) : null;
                lastAgg = agg;
                renderTelemetry(
                    telBody,
                    lastMetrics,
                    agg,
                    stride,
                    aggReady,
                    lastSamplerVersions,
                    forceLawPresentation(b),
                    latencyRequestState(b, agg),
                );
                return;
            }
            repaintSlices = false;
        }

        const aggReady = caps.hasScale0SamplerSnapshot?.('gravityMetricAgg', 0) !== false;
        let agg = null;
        let preparedVolume = null;
        if (directMainWasm) {
            // Panel telemetry consumes bounded regular samples. Dense direct
            // readback is presentation-only and remains limited to small L;
            // large browser-WASM sessions must use the worker/slab contract.
            sample ||= readDirectWasmGravityMetrics(caps, stride);
            agg = aggReady ? (caps.getScale0GravityMetricAgg?.() || null) : null;
            if ((Number(caps.latticeSize) || 0) <= MAX_DIRECT_DENSE_SLICE_L) {
                preparedVolume = caps.getScale0FluxVolume?.();
            }
        } else {
            sample ||= readGravityMetrics(caps, stride);
            agg = aggReady ? (caps.getScale0GravityMetricAgg?.() || null) : null;
        }
        lastAgg = agg;
        if (repaintSlices) {
            if (directMainWasm && (Number(caps.latticeSize) || 0) > MAX_DIRECT_DENSE_SLICE_L) {
                for (const tile of tiles) {
                    const context = tile.canvas?.getContext?.('2d');
                    context?.clearRect(0, 0, tile.canvas.width, tile.canvas.height);
                    tile.readout.textContent = 'worker/slab required';
                }
                setText(modeEl, 'proxy slices · unavailable in large direct WASM');
                modeEl.title = 'Dense L³ readback is disabled above L=48 on the browser main thread. Use the worker-backed WASM owner for coherent bounded slabs.';
                lastHadVolume = false;
            } else {
                lastHadVolume = !!paintSlices(caps, { preparedVolume, tick: observationTick });
            }
        }
        if (!sample.ready) {
            // Worker/native samplers are lazy and may arrive after the compact
            // volume without advancing fieldDataVersion. Do not latch the
            // version, history, or a zero-valued placeholder; the visible callback
            // keeps polling until all three requested scientific samples exist.
            return;
        }
        const comparableVersions = lastSamplerVersions?.every(Number.isFinite)
            && sample.snapshotVersions.every(Number.isFinite);
        if (comparableVersions
            && !samplerVersionsAdvanced(lastSamplerVersions, sample.snapshotVersions)) {
            // A native transport may deliver the three asynchronous fields at
            // different visual revisions. Require every component to advance
            // beyond the last accepted cycle so history never mixes one new
            // field with two retained old fields.
            return;
        }
        const m = sample.metrics;
        lastMetrics = m;
        lastSamplerVersions = sample.snapshotVersions;
        panel.dataset.telemetryState = 'ready';
        applicableContent.setAttribute('aria-busy', 'false');
        renderTelemetry(
            telBody,
            m,
            agg,
            stride,
            aggReady,
            lastSamplerVersions,
            forceLawPresentation(b),
            latencyRequestState(b, agg),
            observationTick,
        );

        if (ver !== lastVer) {
            latched = history.length ? history[history.length - 1] : null;   // previous accepted sample = baseline
            lastVer = ver;
            const entry = { ver, tick: observationTick, Lmax: m.L.max, Kmax: m.K.max, Fmean: m.F.mean, dil: m.dilationPct };
            if (observationTick != null && history.at(-1)?.tick === observationTick) history[history.length - 1] = entry;
            else history.push(entry);
        }
        sampleTick = observationTick;
        if (sampleTick != null) {
            panel.dataset.sampleTick = String(sampleTick);
            telBody.dataset.sampleTick = String(sampleTick);
            deltaBody.dataset.sampleTick = String(sampleTick);
        }
        renderDelta(deltaBody, visibleHistory(), latched, m);
        lastComputedVer = ver;
    }

    function stopCoordinators() {
        armSub?.unsubscribe();
        armSub = null;
        liveSub?.unsubscribe();
        liveSub = null;
    }

    function subscribeLive() {
        if (liveSub || disposed || inapplicable
            || panel.dataset.applicability !== 'applicable') return;
        let nextLegacyReadAt = Number.NEGATIVE_INFINITY;
        liveSub = rafCoordinator.subscribe(PANEL_ID, { hz: DISPLAY_HZ, cb: () => {
            // Explicit dock/floating notifications normally take this path
            // synchronously. This guard is the fail-safe for any visibility
            // mutation outside those controllers.
            if (!isPanelLive(host)) {
                stopRuntime();
                startRuntime();
                return;
            }
            const bridge = getBridge?.();
            if (bridge?.isWorker === true) {
                update();
                return;
            }
            // Direct WASM targets display cadence with a measured work budget.
            // The asynchronous legacy native transport retains its read limit.
            const now = performance.now();
            if (now < nextLegacyReadAt) return;
            update();
            const elapsed = Math.max(0, performance.now() - now);
            nextLegacyReadAt = now + (bridge?.isWasm
                ? Math.max(1000 / DISPLAY_HZ, elapsed / DIRECT_MAX_DUTY)
                : LEGACY_READ_INTERVAL_MS);
        } });
    }

    function startRuntime() {
        if (disposed || inapplicable || panel.dataset.applicability !== 'applicable'
            || armSub || liveSub) return;
        if (isPanelLive(host)) {
            ensureGravityCss();
            update();
            subscribeLive();
            return;
        }
        // Defer the heavy scientific path and stylesheet until first show.
        // No arm subscription exists at all while the null control is active.
        armSub = rafCoordinator.subscribe(`${PANEL_ID}-arm`, { hz: 2, cb: () => {
            if (!isPanelLive(host) || disposed || inapplicable) return;
            armSub?.unsubscribe();
            armSub = null;
            ensureGravityCss();
            update();
            subscribeLive();
        } });
    }

    function stopRuntime() {
        stopCoordinators();
        releaseSamplerWants(true);
    }

    function onPanelVisibilityChange() {
        if (disposed) return;
        if (inapplicable || panel.dataset.applicability !== 'applicable'
            || getScale0State().currentScenarioId === EMPTY_SCENARIO_ID) {
            stopRuntime();
            return;
        }
        if (!isPanelLive(host) && (liveSub || samplerWantSignature || samplerBridge)) {
            // Revoke expensive sampler ownership in the same call stack as the
            // tab/collapse/floating transition, then retain only the bounded
            // 2 Hz guard coordinator that can recover an unannounced show.
            stopRuntime();
            startRuntime();
        }
    }
    window.addEventListener(PANEL_VISIBILITY_CHANGE_EVENT, onPanelVisibilityChange);

    function presentInapplicable() {
        panel.dataset.applicability = 'inapplicable-empty';
        panel.classList.add('is-inapplicable');
        applicableContent.hidden = true;
        applicableContent.setAttribute('aria-hidden', 'true');
        inapplicableMessage.hidden = false;
        pendingMessage.hidden = true;
        quantityButtons.forEach((button) => { button.disabled = true; });
    }

    function presentApplicable() {
        panel.dataset.applicability = 'applicable';
        panel.classList.remove('is-inapplicable');
        applicableContent.hidden = false;
        applicableContent.setAttribute('aria-hidden', 'false');
        inapplicableMessage.hidden = true;
        pendingMessage.hidden = true;
        quantityButtons.forEach((button) => { button.disabled = false; });
    }

    function presentPending(qualification = null) {
        const load = qualification?.authoritativeLoad || null;
        const pending = load?.status === 'pending';
        const failed = load?.status === 'failed';
        const invalidated = load?.status === 'invalidated';
        panel.dataset.applicability = pending ? 'pending-load' : 'unavailable-load';
        panel.classList.add('is-inapplicable');
        applicableContent.hidden = true;
        applicableContent.setAttribute('aria-hidden', 'true');
        inapplicableMessage.hidden = true;
        pendingMessage.hidden = false;
        quantityButtons.forEach((button) => { button.disabled = true; });

        if (failed) {
            pendingDetail.textContent = `Authoritative engine setup failed (${load.failureReason || 'setup-failed'}). Gravity observation remains unavailable.`;
        } else if (invalidated) {
            pendingDetail.textContent = 'Authoritative engine setup was invalidated before completion. Gravity observation remains unavailable.';
        } else if (pending) {
            pendingDetail.textContent = `Waiting for scenario generation ${load.loadGeneration} to cross its authoritative engine setup barrier.`;
        } else {
            pendingDetail.textContent = 'Waiting for the current scenario generation to cross its authoritative engine setup barrier.';
        }
    }

    function setEmptyApplicability(nextValue) {
        const next = !!nextValue;
        if (next === inapplicable) {
            if (next) {
                // Reassert after rapid empty → nonempty → empty churn, which
                // may otherwise leave the intermediate pending label visible.
                presentInapplicable();
                stopRuntime();
                resetAnalysisState();
            }
            return;
        }
        inapplicable = next;
        if (next) {
            qualifiedLoadGeneration = null;
            presentInapplicable();
            stopRuntime();
            resetAnalysisState();
        }
    }

    function handleScenarioIntent(scenarioId) {
        // Suspend immediately on intent, before a stale worker generation can
        // publish a flux-derived gravity proxy into the null-control panel.
        if (scenarioId === EMPTY_SCENARIO_ID) {
            setEmptyApplicability(true);
            return;
        }

        // Every scenario load replaces the active bridge generation. Retire
        // old demands/history now and restore only after the canonical store
        // confirms that the requested generation crossed its setup barrier.
        inapplicable = false;
        qualifiedLoadGeneration = null;
        stopRuntime();
        resetAnalysisState();
        presentPending({
            status: 'pending',
            scenarioId,
            authoritativeLoad: { status: 'pending', loadGeneration: '?' },
        });
    }

    function handleQualification(qualification) {
        if (disposed) return;
        const currentScenarioId = getScale0State().currentScenarioId;
        if (currentScenarioId === EMPTY_SCENARIO_ID) {
            setEmptyApplicability(true);
            return;
        }

        inapplicable = false;
        if (!isScale0AuthoritativeGenerationReady(qualification)) {
            qualifiedLoadGeneration = null;
            stopRuntime();
            resetAnalysisState();
            presentPending(qualification);
            return;
        }

        const generation = Number(qualification?.anchor?.loadGeneration);
        if (qualifiedLoadGeneration !== generation) {
            stopRuntime();
            resetAnalysisState();
            qualifiedLoadGeneration = generation;
        }
        presentApplicable();
        startRuntime();
    }

    function rebindScenarioApplicability() {
        scenarioBinding?.bind();
    }

    scenarioBinding = new ScenarioApplicabilityBinding({
        getCurrentScenarioId: () => getScale0State().currentScenarioId,
        onIntent: handleScenarioIntent,
    });
    rebindScenarioApplicability();
    unsubscribeQualification = subscribeScale0Qualification(handleQualification);

    const api = {
        update,
        element: panel,
        get lastMetrics() { return lastMetrics; },
        get lastAgg() { return lastAgg; },
        get activeKind() { return activeKind; },
        get refreshHz() { return getBridge?.()?.isWasm ? DISPLAY_HZ : 1000 / LEGACY_READ_INTERVAL_MS; },
        setKind,
        get sampleTick() { return sampleTick; },
        get observationStamp() { return lastObservationStamp; },
        get historyLength() { return history.length; },
        get telemetryState() { return panel.dataset.telemetryState; },
        get applicability() { return panel.dataset.applicability; },
        get coordinatorActive() { return !!armSub || !!liveSub; },
        get armCoordinatorActive() { return !!armSub; },
        get liveCoordinatorActive() { return !!liveSub; },
        get samplerWantsActive() { return !!samplerWantSignature || !!samplerBridge; },
        get telemetryStride() { return gravityTelemetryStride(getCaps()?.latticeSize); },
        get qualifiedLoadGeneration() { return qualifiedLoadGeneration; },
        get authoritativeGenerationReady() {
            return isScale0AuthoritativeGenerationReady(getScale0State());
        },
        rebindScenarioApplicability,
        dispose: () => {
            disposed = true;
            stopRuntime();
            unsubscribeQualification?.();
            unsubscribeQualification = null;
            scenarioBinding?.dispose();
            scenarioBinding = null;
            window.removeEventListener(PANEL_VISIBILITY_CHANGE_EVENT, onPanelVisibilityChange);
            historyControl.destroy();
            if (typeof window !== 'undefined' && window.__ftdGravityPanel === api) window.__ftdGravityPanel = null;
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdGravityPanel = api;
    return api;
}

export function initGravityPanel() {
    if (typeof document === 'undefined') return null;
    if (typeof window !== 'undefined' && window.__ftdGravityPanel) {
        window.__ftdGravityPanel.rebindScenarioApplicability?.();
        return window.__ftdGravityPanel;
    }
    const host = document.getElementById('panel-gravity');
    if (!host) return null;
    const getBridge = () => resolveActiveScale0BridgeFromWindow();
    return mountGravityPanel(host, getBridge);
}
