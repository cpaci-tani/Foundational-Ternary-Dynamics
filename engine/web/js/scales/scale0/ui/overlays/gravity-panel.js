/**
 * Gravity Observatory — Scale-0 gravity-field instrument.
 *
 * Three sections:
 *   ① Gravity field slices — per-axis (yz/xz/xy) 2D heatmaps of a selected gravity
 *      quantity (latency L / Kretschmann K / force |F| / dilation L²). Watch a
 *      gravitational wave propagate across the planes.
 *   ② Gravity telemetry — L/K/|F| stats, peak time-dilation, horizon proximity,
 *      GW strain, gravity PE, G_N / α_G, + spatial histograms.
 *   ③ Live Δ-trace — sparklines of the gravity metrics over recent field updates,
 *      plus "Δ since last field change" (driven by fieldDataVersion). Mutate a
 *      field (inject / toggle / seed) and watch gravity respond.
 *
 * Phase 1 is the WEB **proxy** (|J|²-derived). The genuine C++ Poisson metric is
 * surfaced in Phase 2, tagged [C++]. Every readout here is [proxy] + [M]/[D].
 * Modelled on spectrum-panel.js. See .claude/plans/let-s-plan-for-and-eager-tide.md.
 */

import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { cardStyle, titleStyle, heroStyle, tagBadge, formatExp, formatFixed } from './_card-helpers.js';
import { transposeAndFlipNN, paintSliceToCanvas } from './slice-render.js';
import { aggregateMetrics, forceMagnitudes, gravitySlice, maxRhoOf } from '../../analysis/gravity-analysis.js';
import { getScale0State } from '../../state/store.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import { rampViridis, rampEmEnergy, rampVorticity } from '../../../../viewport/color-ramps.js';

const PANEL_ID = 'gravity-panel';
const HZ = 2;

// Load the panel stylesheet via a JS-injected (async, NON-render-blocking) link
// instead of a <head> <link>, and only on first show. A render-blocking <link>
// measurably delayed first paint enough to flake the tight scale-switch timing;
// this keeps the .css the source of truth while off the critical boot path.
function ensureGravityCss() {
    if (typeof document === 'undefined' || document.getElementById('gravity-panel-css')) return;
    const l = document.createElement('link');
    l.id = 'gravity-panel-css';
    l.rel = 'stylesheet';
    l.href = 'css/ui/components/gravity-panel.css';
    document.head.appendChild(l);
}
const STRIDE = 2;          // telemetry sampling stride
const SPARK_MAX = 60;      // Δ-trace rolling-window length
const TILE_PX = 116;       // slice canvas size

const QUANTITIES = [
    { kind: 'latency',     label: 'L',   name: 'Latency',     ramp: rampViridis,
      help: 'Latency potential L = √(|J|²/max) — the gravity-well depth; a GW perturbation rides on this.' },
    { kind: 'kretschmann', label: 'K',   name: 'Kretschmann', ramp: rampEmEnergy,
      help: 'Curvature K = (∇²L)² — bright where spacetime bends most / near horizons.' },
    { kind: 'force',       label: '|F|', name: 'Force',       ramp: rampVorticity,
      help: 'Gravity force |F| = G_N·|∇ρ| — the pull magnitude on each voxel.' },
    { kind: 'dilation',    label: 'f',   name: 'Dilation',    ramp: rampViridis,
      help: 'Lapse deficit L² (= 1−f) ∝ time-dilation strength; bright = clocks run slowest.' },
];
const AXES = [
    { axis: 0, tag: 'yz' },   // x = mid
    { axis: 1, tag: 'xz' },   // y = mid
    { axis: 2, tag: 'xy' },   // z = mid
];

const SECTION_HELP = {
    slices: 'Per-axis 2D slices through the lattice mid-planes (yz / xz / xy). Pick a quantity (L / K / |F| / f) — a gravitational wave appears as a band propagating across the planes. [proxy]: |J|²-derived, not the C++ Poisson field.',
    telemetry: 'Scalar gravity telemetry. L = latency potential, K = Kretschmann curvature, |F| = gravity force. Time-dilation is the peak clock-slowdown 1−√(1−L²); horizon-proximity flags L→1. Gravity PE is the pairwise bound energy. Top rows are |J|²-derived [proxy]; the bottom block is the genuine C++ Poisson latency field [C++] (voxel.latency, field-energy-sourced), shown when the engine runs it.',
    delta: 'How gravity RESPONDS as you mutate fields. Sparklines track L_max / K_max / |F|_mean / dilation% over recent field updates; "Δ since last change" latches the previous field-version and shows the jump. Inject / toggle / seed and watch it move.',
};

// ── compute ──────────────────────────────────────────────────────────────────

function computeGravity(caps) {
    const latency = caps.getScale0FieldSamples?.({ kind: 'latency', stride: STRIDE }) || { values: [], count: 0 };
    const kret = caps.getScale0FieldSamples?.({ kind: 'kretschmann', stride: STRIDE }) || { values: [], count: 0 };
    const force = caps.getScale0ForceField?.('gravity', STRIDE) || { vectors: [], count: 0 };
    const parts = caps.getScale0ParticleFrame?.() || { positions: [], count: 0 };
    const forceMags = forceMagnitudes(force.vectors || [], force.count | 0);
    return aggregateMetrics({
        latencyVals: latency.values, latencyCount: latency.count | 0,
        kretVals: kret.values, kretCount: kret.count | 0,
        forceMags, forceCount: force.count | 0,
        particlePositions: parts.positions, particleCount: parts.count | 0,
    });
}

// ── small render helpers ──────────────────────────────────────────────────────

function row(label, value, tag = 'D', color = 'var(--text-primary)', tip = '') {
    const t = tip ? ` title="${tip}"` : '';
    return `<div class="grav-row"><span class="grav-row-l"${t}>${tagBadge(tag)}${label}</span><span class="grav-row-v" style="color:${color}">${value}</span></div>`;
}

function miniHist(hist, w = 70, h = 20) {
    if (!hist || !hist.counts) return '';
    const c = hist.counts, n = c.length, mx = Math.max(1, ...c), bw = w / n;
    let s = `<svg viewBox="0 0 ${w} ${h}" class="grav-mini-hist">`;
    for (let i = 0; i < n; i++) {
        const bh = (c[i] / mx) * h;
        s += `<rect x="${(i * bw).toFixed(1)}" y="${(h - bh).toFixed(1)}" width="${(bw - 0.3).toFixed(1)}" height="${bh.toFixed(1)}" fill="var(--accent)" opacity="0.7"/>`;
    }
    return s + `</svg>`;
}

function sparkline(values, color, w = 116, h = 26) {
    const n = values.length;
    if (n < 2) return `<svg viewBox="0 0 ${w} ${h}" class="grav-spark"></svg>`;
    let min = Infinity, max = -Infinity;
    for (const v of values) { if (v < min) min = v; if (v > max) max = v; }
    const span = (max - min) || 1;
    let d = '';
    for (let i = 0; i < n; i++) {
        const x = (i / (n - 1)) * w;
        const y = h - ((values[i] - min) / span) * (h - 2) - 1;
        d += `${i ? 'L' : 'M'}${x.toFixed(1)},${y.toFixed(1)} `;
    }
    return `<svg viewBox="0 0 ${w} ${h}" class="grav-spark"><path d="${d}" fill="none" stroke="${color}" stroke-width="1.2"/></svg>`;
}

function deltaSpan(cur, base) {
    if (base == null) return `<span class="grav-delta-na">—</span>`;
    const d = cur - base;
    const col = Math.abs(d) < 1e-12 ? 'var(--text-muted)' : (d > 0 ? 'var(--positive)' : 'var(--negative)');
    const sign = d > 0 ? '+' : '';
    return `<span style="color:${col}">${sign}${formatExp(d)}</span>`;
}

// ── section renderers ─────────────────────────────────────────────────────────

// Real C++ latency (Poisson) sub-block — the genuine voxel.latency field,
// distinct from the |J|² proxy rows. Honest: shows "inactive — proxy only" when
// the engine isn't running the latency solver / there is no source.
function renderCppBlock(agg) {
    const head = `<div style="margin:8px 0 4px;padding-top:6px;border-top:1px solid var(--border-subtle,rgba(255,255,255,.09));font-size:11px;font-weight:600;color:var(--text-secondary);" title="The genuine voxel.latency from the engine's Poisson solver (∇²L=4πGρ), with ρ sourced from field-energy density ½|J|² — an [IMPOSED] engine model. Distinct from the |J|² proxy rows above.">Real C++ latency field (Poisson) ⓘ</div>`;
    if (!agg || !agg.active)
        return head + `<div style="font-size:11px;color:var(--text-muted);padding:3px 0;">${tagBadge('C++')}inactive — proxy only (no latency source)</div>`;
    let html = head;
    html += row('L (mean / max)', `${formatExp(agg.latencyMean)} / ${formatExp(agg.latencyMax)}`, 'C++', undefined, 'Real Poisson latency potential voxel.latency, sourced from field energy.');
    html += row('Lapse f_min', formatFixed(agg.fMin, 5), 'C++', undefined, 'Min lapse f = 1 − L_max² (deepest real time dilation).');
    html += row('Time-dilation (peak)', `${formatExp(agg.dilationMaxPct)} %`, 'C++', undefined, '(1 − √f_min)·100 from the real latency field.');
    html += row('γ_ftd (max)', formatFixed(agg.gammaMax, 5), 'C++', undefined, 'Max generalized Lorentz factor √f/√(f²−v²).');
    html += row('Active voxels', String(agg.voxelCount), 'C++', undefined, 'Voxels carrying non-zero real latency.');
    return html;
}

function renderTelemetry(container, m, agg) {
    const horizonColor = m.horizon >= 0.95 ? 'var(--negative)' : m.horizon >= 0.5 ? 'var(--caution)' : 'var(--positive)';
    let html = '';
    if (m.horizon >= 0.95) html += `<div style="${heroStyle()};color:var(--negative);margin-bottom:6px;">⚠ horizon — L_max ${formatFixed(m.horizon, 3)}</div>`;
    html += row('Latency L (mean / max)', `${formatFixed(m.L.mean, 3)} / ${formatFixed(m.L.max, 3)}`, 'M', undefined, 'Latency potential — the gravity-well depth proxy. max→1 ⇒ event horizon.');
    html += row('Kretschmann K (mean / max)', `${formatExp(m.K.mean)} / ${formatExp(m.K.max)}`, 'M', undefined, 'Curvature (∇²L)² — concentration = strong bending.');
    html += row('Force |F| (mean / max)', `${formatExp(m.F.mean)} / ${formatExp(m.F.max)}`, 'M', undefined, 'Gravity force magnitude G_N·|∇ρ|.');
    html += row('Time-dilation (peak)', `${formatFixed(m.dilationPct, 3)} %`, 'D', undefined, 'Peak clock slowdown 1−√(1−L_max²).');
    html += row('Horizon proximity (L_max)', formatFixed(m.horizon, 3), 'D', horizonColor, 'How close the strongest voxel is to the L→1 horizon clamp.');
    html += row('GW strain proxy', formatExp(m.strain), 'M', undefined, 'L_max − L_mean — how far the peak rises above background.');
    html += row('Gravity PE', formatExp(m.gravPE), 'D', undefined, 'Pairwise bound energy −Σ G_N·K_B²/r over manifested particles (0 with no particles).');
    html += row('G_N / α_G', `${formatFixed(m.gnG, 3)} / ${m.alphaG.toExponential(2)}`, 'D', undefined, 'Framework gravitational coupling and the gravitational fine-structure reference.');
    html += `<div class="grav-hist-row"><span>L ${miniHist(m.histL)}</span><span>K ${miniHist(m.histK)}</span><span>|F| ${miniHist(m.histF)}</span></div>`;
    html += renderCppBlock(agg);
    container.innerHTML = html;
}

function renderDelta(container, history, latched, cur) {
    if (!history.length) { container.innerHTML = `<div class="grav-empty">No field data yet — load a gravity scenario.</div>`; return; }
    const series = [
        { key: 'Lmax', label: 'L max', color: 'var(--accent)', sel: (h) => h.Lmax, c: cur.L.max },
        { key: 'Kmax', label: 'K max', color: 'var(--caution, #fb8c00)', sel: (h) => h.Kmax, c: cur.K.max },
        { key: 'Fmean', label: '|F| mean', color: 'var(--positive)', sel: (h) => h.Fmean, c: cur.F.mean },
        { key: 'dil', label: 'dilation %', color: 'var(--negative)', sel: (h) => h.dil, c: cur.dilationPct },
    ];
    let html = '';
    for (const s of series) {
        const vals = history.map(s.sel);
        html += `<div class="grav-spark-row">
            <span class="grav-spark-label">${s.label}</span>
            ${sparkline(vals, s.color)}
            <span class="grav-spark-now">${formatExp(s.c)}</span>
            <span class="grav-spark-delta">Δ ${deltaSpan(s.c, latched ? latched[s.key] : null)}</span>
        </div>`;
    }
    container.innerHTML = html;
}

// ── panel shell ───────────────────────────────────────────────────────────────

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.className = 'scale0-only gravity-panel';
    const qbtns = QUANTITIES.map((q, i) =>
        `<button type="button" class="grav-qbtn${i === 0 ? ' active' : ''}" data-kind="${q.kind}" title="${q.help}">${q.label}</button>`).join('');
    const tiles = AXES.map((a) =>
        `<div class="grav-tile"><canvas id="${PANEL_ID}-tile-${a.axis}" width="${TILE_PX}" height="${TILE_PX}"></canvas><div class="grav-tile-meta"><span>${a.tag}</span><span id="${PANEL_ID}-rd-${a.axis}" class="grav-tile-readout">—</span></div></div>`).join('');
    root.innerHTML = `
        <header class="grav-header">
            <span class="grav-title">Gravity Observatory</span>
            <span class="grav-mode" id="${PANEL_ID}-mode" title="Web proxy gravity (|J|²-derived). The real C++ Poisson metric is Phase 2.">proxy</span>
        </header>
        <section style="${cardStyle(210)}">
            <div style="${titleStyle()}" title="${SECTION_HELP.slices}">Gravity field slices ⓘ</div>
            <div class="grav-qsel" id="${PANEL_ID}-qsel">${qbtns}</div>
            <div class="grav-slice-tiles">${tiles}</div>
        </section>
        <section style="${cardStyle(220)}">
            <div style="${titleStyle()}" title="${SECTION_HELP.telemetry}">Gravity telemetry ⓘ</div>
            <div id="${PANEL_ID}-telemetry"></div>
        </section>
        <section style="${cardStyle(170)}">
            <div style="${titleStyle()}" title="${SECTION_HELP.delta}">Live Δ-trace ⓘ</div>
            <div id="${PANEL_ID}-delta"></div>
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
    const telBody = el('telemetry'), deltaBody = el('delta'), modeBadge = el('mode');
    const tiles = AXES.map((a) => ({ axis: a.axis, tag: a.tag, canvas: el(`tile-${a.axis}`), readout: el(`rd-${a.axis}`) }));

    let activeKind = 'latency';
    let lastMetrics = null;
    let lastAgg = null;
    let bridgeId = null;
    let history = [];     // [{ ver, Lmax, Kmax, Fmean, dil }]
    let latched = null;   // metric vector at the previous field-version
    let lastVer = -1;

    panel.querySelector(`#${PANEL_ID}-qsel`).addEventListener('click', (e) => {
        const btn = e.target.closest('.grav-qbtn');
        if (!btn) return;
        activeKind = btn.dataset.kind;
        panel.querySelectorAll('.grav-qbtn').forEach((b) => b.classList.toggle('active', b === btn));
        const caps = getCaps();
        if (caps) paintSlices(caps);
    });

    function getCaps() {
        const b = getBridge?.();
        return b?.capabilities?.scale0 || null;
    }

    function paintSlices(caps) {
        const L = caps.latticeSize || 33;
        const q = QUANTITIES.find((x) => x.kind === activeKind) || QUANTITIES[0];
        const mid = L >> 1;
        const M = L * L * L;
        // The dense |J| volume — available on BOTH bridges (Mock + Wasm), so the
        // slice is bridge-agnostic. maxRho computed once, shared across the 3 axes.
        const mag = caps.getScale0FluxVolume?.();
        if (!mag || mag.length < M) {
            for (const t of tiles) { paintSliceToCanvas(t.canvas, null, L, {}); t.readout.textContent = '—'; }
            return false;
        }
        const rho = maxRhoOf(mag, M);
        let anyData = false;
        for (const t of tiles) {
            const raw = gravitySlice(mag, L, t.axis, mid, activeKind, rho);
            const data = transposeAndFlipNN(raw, L);
            let max = 0;
            for (let i = 0; i < data.length; i++) if (data[i] > max) max = data[i];
            if (max > 1e-30) anyData = true;
            const norm = max > 1e-30 ? 1 / max : 1;
            paintSliceToCanvas(t.canvas, data, L, { ramp: q.ramp, signed: false, norm });
            t.readout.textContent = `max ${formatExp(max)}`;
        }
        return anyData;
    }

    function update() {
        const b = getBridge?.();
        const caps = b?.capabilities?.scale0 || null;
        if (!caps) return;
        // reset trace if the bridge identity changed (scenario / scale switch)
        if (b !== bridgeId) { bridgeId = b; history = []; latched = null; lastVer = -1; }

        // Gate the heavy work (full-volume read + O(N³) maxRho + 3 slices +
        // samplers) on visibility — when the Gravity tab isn't shown, do nothing.
        // This keeps the panel from loading the main thread (which otherwise
        // slows scale switches) and is the established panel pattern (isPanelLive).
        if (!isPanelLive(host)) return;

        const m = computeGravity(caps);
        lastMetrics = m;
        const agg = caps.getScale0GravityMetricAgg?.() || null;
        lastAgg = agg;
        paintSlices(caps);
        renderTelemetry(telBody, m, agg);

        const ver = (getScale0State()?.fieldDataVersion) | 0;
        if (ver !== lastVer) {
            latched = history.length ? history[history.length - 1] : null;   // previous field-version = baseline
            lastVer = ver;
            history.push({ ver, Lmax: m.L.max, Kmax: m.K.max, Fmean: m.F.mean, dil: m.dilationPct });
            if (history.length > SPARK_MAX) history.shift();
        }
        renderDelta(deltaBody, history, latched, m);
    }

    // Defer the rAF update loop + the stylesheet to first show. A light 2 Hz arm
    // poll watches visibility; the heavy loop (volume read + slices + samplers)
    // and the CSS fetch never touch the boot / scale-switch critical path until
    // the Gravity tab is actually opened.
    let liveSub = null;
    const armSub = rafCoordinator.subscribe(`${PANEL_ID}-arm`, { hz: 2, cb: () => {
        if (!isPanelLive(host)) return;
        armSub.unsubscribe();
        ensureGravityCss();
        update();
        liveSub = rafCoordinator.subscribe(PANEL_ID, { hz: HZ, cb: update });
    } });

    const api = {
        update,
        element: panel,
        get lastMetrics() { return lastMetrics; },
        get lastAgg() { return lastAgg; },
        get activeKind() { return activeKind; },
        setKind: (k) => { activeKind = k; const caps = getCaps(); if (caps) paintSlices(caps); },
        get historyLength() { return history.length; },
        dispose: () => {
            armSub.unsubscribe();
            liveSub?.unsubscribe();
            if (typeof window !== 'undefined' && window.__ftdGravityPanel === api) window.__ftdGravityPanel = null;
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdGravityPanel = api;
    return api;
}

export function initGravityPanel() {
    if (typeof document === 'undefined') return null;
    const host = document.getElementById('panel-gravity');
    if (!host) return null;
    const getBridge = () => {
        const state = getScale0State?.();
        if (state?.useFluxMock && state?.fluxMock) return state.fluxMock;
        const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        return ctx?.bridge || null;
    };
    return mountGravityPanel(host, getBridge);
}
