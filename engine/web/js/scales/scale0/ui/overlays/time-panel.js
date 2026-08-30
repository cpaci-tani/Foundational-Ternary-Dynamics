/**
 * Time Observatory — Scale-0 time-dilation instrument.
 *
 * Four cards:
 *   A — Lab clock & summary: physical time + peak dτ/dt, f_min, FTD γ_max.
 *       Shows a [C++] sub-block from getScale0GravityMetricAgg() when the real
 *       Poisson latency field is live (agg.active); else the derived |J|²
 *       proxy ([~M]).
 *   B — Radial dilation profile: measured dτ/dt(r) across a gravity well (solid
 *       [~M]/[M]) vs a dashed [D] prediction curve, with a residual % row.
 *   C — Twin clocks: two fixed probes (deep vs far) accumulate proper time
 *       τ = Σ√f·dt; shows τ_deep, τ_far, Δτ, and a Δτ-vs-tick sparkline.
 *   D — Kinematic (imposed β): √(1−β²) [T] + selected FTD γ(β) curves vs the baked
 *       FTD-0252 measured points [M] (offline campaign); imposed-v slider
 *       [IMPOSED]; an IR-convergence mini-chart (resid → L⁻²).
 *
 * Gravitational dτ/dt outside the live Poisson readout is the |J|² *proxy*
 * latency → [~M].
 * It becomes [C++]/[M] only when WASM's Poisson γ_ftd is live (agg.active).
 * The kinematic card's measured points are genuine engine output (FTD-0252) but
 * OFFLINE/pre-computed → [M] "campaign". The velocity is [IMPOSED] (rigid
 * cluster translation is [BOUNDARY-blocked]). Curves are [T]/[D].
 *
 * Modelled exactly on gravity-panel.js. See
 * docs/superpowers/specs/2026-06-07-time-dilation-panel-design.md.
 */

import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { cardStyle, titleStyle, heroStyle, tagBadge, formatExp, formatFixed } from './_card-helpers.js';
import {
    lapse, clockRate, ftdGamma, srDilation,
    properTimeStep, radialProfile, radialBins,
} from '../../analysis/time-analysis.js';
import { FTD0252_PROVENANCE, DILATION_VS_V, IR_CONVERGENCE } from '../../data/ftd0252-reference.js';
import { resolveActiveScale0BridgeFromWindow } from '../../state/store.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import { readScale0DiagAudit } from '../../../../telemetry/scale0-read.js';
import { telemetryHub } from '../../../../telemetry-hub.js';
import { C_SPEED } from '../../../../constants.js';

const PANEL_ID = 'time-panel';
const HZ = 2;
const STRIDE = 2;            // latency sampling stride
const SPARK_MAX = 60;       // Δτ trace rolling-window length
const RADIAL_BINS = 12;

function timeSourceBoundary(hub = telemetryHub) {
    const expected = hub?.s0?.meta?.expectedSourceEpoch;
    if (expected !== null && expected !== undefined && expected !== '') {
        return `${hub?.s0?.meta?.expectedSource ?? 'unknown'}:${String(expected)}`;
    }
    const meta = hub?.getScale0TelemetryMeta?.('diagnostics') ?? null;
    const epoch = meta?.sourceEpoch ?? meta?.epoch;
    if (epoch === null || epoch === undefined || epoch === '') return null;
    return `${meta?.source ?? 'unknown'}:${String(epoch)}`;
}

// Inject the panel stylesheet via a JS-injected (async, NON-render-blocking)
// link instead of a <head> <link>, and only on first show — same rationale as
// gravity-panel.js (a render-blocking <link> measurably delayed first paint).
function ensureTimeCss() {
    if (typeof document === 'undefined' || document.getElementById('time-panel-css')) return;
    const l = document.createElement('link');
    l.id = 'time-panel-css';
    l.rel = 'stylesheet';
    l.href = 'css/ui/components/time-panel.css';
    document.head.appendChild(l);
}

// ── small render helpers ──────────────────────────────────────────────────────

function row(label, value, tag = 'D', color = 'var(--text-primary)', tip = '') {
    const t = tip ? ` title="${tip}"` : '';
    return `<div class="time-row"><span class="time-row-l"${t}>${tagBadge(tag)}${label}</span><span class="time-row-v" style="color:${color}">${value}</span></div>`;
}

function sparkline(values, color, w = 220, h = 30) {
    const n = values.length;
    if (n < 2) return `<svg viewBox="0 0 ${w} ${h}" class="time-spark"></svg>`;
    let min = Infinity, max = -Infinity;
    for (const v of values) { if (v < min) min = v; if (v > max) max = v; }
    const span = (max - min) || 1;
    let d = '';
    for (let i = 0; i < n; i++) {
        const x = (i / (n - 1)) * w;
        const y = h - ((values[i] - min) / span) * (h - 2) - 1;
        d += `${i ? 'L' : 'M'}${x.toFixed(1)},${y.toFixed(1)} `;
    }
    return `<svg viewBox="0 0 ${w} ${h}" class="time-spark"><path d="${d}" fill="none" stroke="${color}" stroke-width="1.4"/></svg>`;
}

/**
 * Two-curve chart: a solid "measured" series and a dashed "predicted" series,
 * sharing one x-domain and one y-domain. `series` = [{ pts:[{x,y}], color,
 * dashed }]. `marker` (optional) = {x,y} drawn as a dot. Axes are 0-based on y
 * unless yMin/yMax given. Mirrors the p1-observables dashed-vs-solid pattern.
 */
function dualCurveChart(series, { w = 240, h = 110, xMin, xMax, yMin, yMax, marker } = {}) {
    const m = { left: 30, right: 8, top: 8, bottom: 18 };
    const innerW = w - m.left - m.right;
    const innerH = h - m.top - m.bottom;
    let lo = Infinity, hi = -Infinity, xlo = Infinity, xhi = -Infinity;
    for (const s of series) for (const p of s.pts) {
        if (p.y < lo) lo = p.y; if (p.y > hi) hi = p.y;
        if (p.x < xlo) xlo = p.x; if (p.x > xhi) xhi = p.x;
    }
    if (xMin != null) xlo = xMin; if (xMax != null) xhi = xMax;
    if (yMin != null) lo = yMin; if (yMax != null) hi = yMax;
    if (!Number.isFinite(lo) || !Number.isFinite(hi)) { lo = 0; hi = 1; }
    const xspan = (xhi - xlo) || 1, yspan = (hi - lo) || 1;
    const xpx = (x) => m.left + ((x - xlo) / xspan) * innerW;
    const ypx = (y) => m.top + (1 - (y - lo) / yspan) * innerH;
    let svg = `<svg viewBox="0 0 ${w} ${h}" class="time-chart">`;
    svg += `<rect x="${m.left}" y="${m.top}" width="${innerW}" height="${innerH}" fill="rgba(255,255,255,0.02)" stroke="var(--border-light, rgba(255,255,255,0.07))" stroke-width="0.5"/>`;
    // y gridline labels (lo, mid, hi)
    for (const yv of [lo, (lo + hi) / 2, hi]) {
        const y = ypx(yv);
        svg += `<line x1="${m.left}" y1="${y.toFixed(1)}" x2="${(m.left + innerW)}" y2="${y.toFixed(1)}" stroke="var(--border-light, rgba(255,255,255,0.05))" stroke-width="0.4"/>`;
        svg += `<text x="${m.left - 3}" y="${(y + 3).toFixed(1)}" text-anchor="end" fill="var(--text-muted)" font-size="16">${yv.toFixed(2)}</text>`;
    }
    for (const s of series) {
        if (!s.pts.length) continue;
        let d = '';
        for (let i = 0; i < s.pts.length; i++) {
            d += (i ? 'L' : 'M') + xpx(s.pts[i].x).toFixed(1) + ',' + ypx(s.pts[i].y).toFixed(1);
        }
        const dash = s.dashed ? ' stroke-dasharray="4,3"' : '';
        svg += `<path d="${d}" fill="none" stroke="${s.color}" stroke-width="1.5"${dash}/>`;
        if (s.dots) for (const p of s.pts) {
            svg += `<circle cx="${xpx(p.x).toFixed(1)}" cy="${ypx(p.y).toFixed(1)}" r="2.1" fill="${s.color}"/>`;
        }
    }
    if (marker && Number.isFinite(marker.x) && Number.isFinite(marker.y)) {
        const mx = xpx(marker.x), my = ypx(marker.y);
        svg += `<line x1="${mx.toFixed(1)}" y1="${m.top}" x2="${mx.toFixed(1)}" y2="${(m.top + innerH).toFixed(1)}" stroke="var(--accent)" stroke-width="0.8" stroke-dasharray="2,2"/>`;
        svg += `<circle cx="${mx.toFixed(1)}" cy="${my.toFixed(1)}" r="3" fill="var(--accent)" stroke="#000" stroke-width="0.5"/>`;
    }
    svg += `</svg>`;
    return svg;
}

// ── card renderers ─────────────────────────────────────────────────────────────

// Card A — lab clock & summary. The [C++] sub-block surfaces the genuine Poisson
// latency readout when active; otherwise the panel is honestly "proxy only".
function renderClockBlock(agg) {
    const head = `<div class="time-cpp-head" title="The genuine voxel.latency from the engine's Poisson solver (∇²L=4πGρ), ρ from real rest mass — an [IMPOSED] engine model. Distinct from the |J|² proxy.">Real C++ latency field (Poisson) ⓘ</div>`;
    if (!agg || !agg.active)
        return head + `<div class="time-cpp-inactive">${tagBadge('~M')}proxy only — derived |J|² latency (no real Poisson source)</div>`;
    let html = head;
    html += row('Lapse f_min', formatFixed(agg.fMin, 5), 'M', undefined, 'Min lapse f = 1 − L_max² (deepest real time dilation), [C++].');
    html += row('dτ/dt (min)', formatFixed(Math.sqrt(Math.max(0, agg.fMin)), 5), 'M', undefined, 'Slowest clock rate √f_min from the real latency field, [C++].');
    html += row('Time-dilation (peak)', `${formatExp(agg.dilationMaxPct)} %`, 'M', undefined, '(1 − √f_min)·100 from the real latency field, [C++].');
    html += row('γ_ftd (max)', formatFixed(agg.gammaMax, 5), 'M', undefined, 'Max FTD generalized Lorentz factor √f/√(f²−v²), [C++].');
    return html;
}

function renderCardA(container, metrics, agg) {
    const { physicalTime, fMin, dtauMin, gammaMax } = metrics;
    let html = `<div style="${heroStyle()}" title="Lab-frame elapsed time (physical ticks of the substrate clock).">t = ${formatFixed(physicalTime, 1)}</div>`;
    html += `<div style="font-size:16px;color:var(--text-muted);margin:2px 0 8px;">physical time (lab clock)</div>`;
    html += row('Slowest dτ/dt', formatFixed(dtauMin, 5), '~M', undefined, 'Slowest clock rate √f over the sampled latency field (proxy unless the [C++] block below is active).');
    html += row('f_min (lapse)', formatFixed(fMin, 5), '~M', undefined, 'Minimum lapse f = 1 − L_max² over the sampled field.');
    html += row('Peak slowdown', `${formatFixed((1 - dtauMin) * 100, 3)} %`, '~M', undefined, 'Peak clock slowdown (1 − √f_min)·100 from the proxy field.');
    html += row('FTD γ_max', formatFixed(gammaMax, 4), 'D', undefined, 'Max FTD generalized Lorentz factor √f/√(f²−v²) at the deepest sampled point (v=0 ⇒ 1/√f).');
    html += renderClockBlock(agg);
    container.innerHTML = html;
}

// Card B — radial dilation profile (measured vs predicted).
// Predicted: a 1-parameter weak-field fit dτ/dt(r) = √(1 − L_max²·(r0/r)) using
// the deepest sampled latency as the well depth — a [D] reference curve, not a
// fit to the data point-by-point.
function renderCardB(container, prof) {
    if (!prof || !prof.length) {
        container.innerHTML = `<div class="time-empty">No latency field yet — load a gravity-well / Time scenario and press play.</div>`;
        return;
    }
    const bins = radialBins(prof, RADIAL_BINS);
    if (bins.length < 2) {
        container.innerHTML = `<div class="time-empty">Field too sparse for a radial profile — let it propagate a few ticks.</div>`;
        return;
    }
    // Deepest (min dτ/dt) sample drives the prediction well-depth.
    let lMax = 0, rAtMax = bins[0].r;
    for (const p of prof) if (p.L > lMax) { lMax = p.L; rAtMax = p.r || rAtMax; }
    const r0 = Math.max(1e-6, rAtMax);
    // Predicted dτ/dt(r): weak-field 1/r falloff of the lapse deficit, clamped.
    const predAt = (r) => {
        const Lr = lMax * Math.min(1, r0 / Math.max(r0, r));   // deficit ~ L_max at r0, ∝ 1/r beyond
        return clockRate(Lr);
    };
    const measPts = bins.map((b) => ({ x: b.r, y: b.dtau_dt }));
    const rLo = bins[0].r, rHi = bins[bins.length - 1].r;
    const predPts = [];
    const steps = 24;
    for (let i = 0; i <= steps; i++) {
        const r = rLo + (rHi - rLo) * (i / steps);
        predPts.push({ x: r, y: predAt(r) });
    }
    // Residual: mean |measured − predicted| / predicted over the bins (%).
    let resAcc = 0, resN = 0;
    for (const b of bins) {
        const p = predAt(b.r);
        if (p > 1e-9) { resAcc += Math.abs(b.dtau_dt - p) / p; resN++; }
    }
    const residPct = resN ? (resAcc / resN) * 100 : NaN;

    let html = `<div class="time-legend">`
        + `<span class="time-legend-item"><span class="time-swatch" style="background:var(--accent)"></span>${tagBadge('~M')}measured dτ/dt(r)</span>`
        + `<span class="time-legend-item"><span class="time-swatch time-swatch-dash" style="background:var(--caution,#fb8c00)"></span>${tagBadge('D')}predicted √(1−L·r₀/r)</span>`
        + `</div>`;
    html += dualCurveChart([
        { pts: predPts, color: 'var(--caution, #fb8c00)', dashed: true },
        { pts: measPts, color: 'var(--accent)', dots: true },
    ], { w: 240, h: 120, yMin: Math.max(0, Math.min(...measPts.map((p) => p.y), ...predPts.map((p) => p.y)) - 0.02), yMax: 1.0 });
    html += `<div class="time-chart-xlabel">radius r from mass center →</div>`;
    html += row('Well depth L_max', formatFixed(lMax, 4), '~M', undefined, 'Deepest sampled latency (gravity-well depth).');
    html += row('dτ/dt at well floor', formatFixed(clockRate(lMax), 5), '~M', undefined, 'Slowest measured clock rate √(1−L_max²).');
    html += row('Mean residual (meas vs pred)', Number.isFinite(residPct) ? `${formatFixed(residPct, 2)} %` : '—', 'D', undefined, 'Mean |measured − predicted| / predicted across radial bins.');
    container.innerHTML = html;
}

// Card C — twin clocks (Δτ).
function renderCardC(container, twin) {
    const { tauDeep, tauFar, history, lDeep, lFar, active } = twin;
    if (!active) {
        container.innerHTML = `<div class="time-empty">Twin clocks idle — load a gravity-well / Time scenario; the deep clock (well floor) ticks slower than the far clock (shallow edge).</div>`;
        return;
    }
    const dtau = tauFar - tauDeep;
    let html = `<div style="${heroStyle()}" title="Accumulated proper-time lead of the far clock over the deep clock — the GPS/twin offset, built from Σ√f·dt at each probe.">Δτ = ${formatExp(dtau)}</div>`;
    html += `<div style="font-size:16px;color:var(--text-muted);margin:2px 0 8px;">${tagBadge('~M')}far clock lead (twin / GPS offset)</div>`;
    html += sparkline(history, 'var(--positive-text)', 232, 32);
    html += `<div class="time-chart-xlabel">Δτ accumulating over ticks →</div>`;
    html += row('τ_deep (well floor)', formatExp(tauDeep), '~M', undefined, `Proper time at the deepest probe (latency L≈${formatFixed(lDeep, 3)}); runs slowest.`);
    html += row('τ_far (shallow edge)', formatExp(tauFar), '~M', undefined, `Proper time at the shallowest probe (latency L≈${formatFixed(lFar, 3)}); runs fastest.`);
    html += row('Δτ = τ_far − τ_deep', formatExp(dtau), '~M', dtau >= 0 ? 'var(--positive-text)' : 'var(--negative-text)', 'The far clock outruns the deep clock — grows monotonically while the well stands.');
    container.innerHTML = html;
}

// Card D — kinematic (imposed v) + baked FTD-0252.
function renderCardD(container, vImposed) {
    const steps = 40, vCap = 0.95;
    // Two clock-rate (dτ/dt) curves over imposed β∈[0, 0.95]. The engine
    // stores raw nodes/tick, so every evaluation converts β to u=β*C_SPEED.
    // At L=0 the selected FTD-0402 clock axiom coincides algebraically with the
    // SR reference. This coincidence is not a substrate derivation of covariance.
    const srPts = [], ftdPts = [];
    for (let i = 0; i <= steps; i++) {
        const v = (i / steps) * vCap;
        const rawSpeed = v * C_SPEED;
        srPts.push({ x: v, y: srDilation(rawSpeed) });
        ftdPts.push({ x: v, y: 1 / ftdGamma(0, rawSpeed) });
    }
    const measPts = DILATION_VS_V.map((d) => ({ x: d.v, y: d.dilation }));
    // Residual of the baked measured points vs √(1−v²).
    let resAcc = 0, resN = 0;
    for (const d of DILATION_VS_V) {
        const t = srDilation(d.v * C_SPEED);
        if (t > 1e-9) { resAcc += Math.abs(d.dilation - t) / t; resN++; }
    }
    const residPct = resN ? (resAcc / resN) * 100 : NaN;
    const markY = srDilation(vImposed * C_SPEED);

    let html = `<div class="time-slider-row">`
        + `<label for="${PANEL_ID}-vslider" title="Imposed boost velocity in units of c. Rigid cluster translation is [BOUNDARY-blocked], so this is an IMPOSED parameter, not an engine-measured boost.">${tagBadge('IMPOSED', 'imposed parameter — rigid translation is [BOUNDARY-blocked]')}v = <span id="${PANEL_ID}-vval" class="time-vval">${vImposed.toFixed(2)}</span> c</label>`
        + `<input type="range" id="${PANEL_ID}-vslider" class="time-vslider" min="0" max="0.95" step="0.01" value="${vImposed}">`
        + `</div>`;
    html += `<div class="time-readout-grid">`
        + `<span>${tagBadge('T')}√(1−β²)</span><span class="time-rg-v">${formatFixed(srDilation(vImposed * C_SPEED), 5)}</span>`
        + `<span>${tagBadge('AXIOM')}FTD γ(β)</span><span class="time-rg-v">${formatFixed(ftdGamma(0, vImposed * C_SPEED), 5)}</span>`
        + `</div>`;
    html += `<div class="time-legend">`
        + `<span class="time-legend-item"><span class="time-swatch" style="background:var(--negative)"></span>${tagBadge('M')}FTD-0252 measured</span>`
        + `<span class="time-legend-item"><span class="time-swatch time-swatch-dash" style="background:var(--accent)"></span>${tagBadge('T')}√(1−v²)</span>`
        + `<span class="time-legend-item"><span class="time-swatch" style="background:var(--caution,#fb8c00)"></span>${tagBadge('AXIOM')}FTD-0402 clock</span>`
        + `</div>`;
    html += dualCurveChart([
        { pts: srPts, color: 'var(--accent)', dashed: true },
        { pts: ftdPts, color: 'var(--caution, #fb8c00)' },
        { pts: measPts, color: 'var(--negative-text)', dots: true },
    ], { w: 240, h: 120, xMin: 0, xMax: vCap, yMin: 0, yMax: 1.0, marker: { x: vImposed, y: markY } });
    html += `<div class="time-chart-xlabel">velocity β = |u| / C_SPEED →   (dτ/dt vs β)</div>`;
    html += row('Mean residual (meas vs √(1−v²))', Number.isFinite(residPct) ? `${formatFixed(residPct, 2)} %` : '—', 'M', undefined, 'Mean |measured − √(1−v²)| / √(1−v²) over the baked FTD-0252 points.');
    html += `<div class="time-provenance" title="${FTD0252_PROVENANCE}">${tagBadge('M')}${FTD0252_PROVENANCE} — campaign, offline (not live)</div>`;

    // IR-convergence mini-chart: residual → 0 as L⁻² (γ emerges in the IR).
    const irPts = IR_CONVERGENCE.map((p) => ({ x: p.L, y: p.resid }));
    html += `<div class="time-subhead" title="Median |dτ/dt − √(1−v²)| vs lattice size L, mass held fixed (k⊥→0). Falls ~ L⁻²: exact Lorentz γ emerges in the IR / continuum limit.">IR convergence — γ emerges as L⁻² ⓘ</div>`;
    html += dualCurveChart([{ pts: irPts, color: 'var(--positive-text)', dots: true }],
        { w: 240, h: 70, yMin: 0 });
    html += `<div class="time-chart-xlabel">${tagBadge('M')}lattice L →   (median residual ↓)</div>`;
    container.innerHTML = html;
}

// Card E — de Broglie internal clock (FTD-0271). The manifested cluster carries
// a Klein-Gordon rest-mass clock: dφ/dt = ω₀·dτ/dt, so the internal phase φ
// winds at ω₀ at rest and follows the FTD-0402 causal budget when moving.
// Raw speed is normalized by C_SPEED and combined with the local latency L.
// The relation remains a selected clock axiom, not evidence of covariance.
function renderCardE(container, db) {
    if (!db.hasData) {
        container.innerHTML = `<div class="time-empty">de Broglie clock idle — load the “De Broglie Clock (pilot wave)” scenario (or enable the de_broglie_clock toggle) and press play. The manifested cluster's flux then oscillates at ω₀.</div>`;
        return;
    }
    const { active, omega0, phase, speed, latency, clockRate } = db;
    const period = omega0 > 1e-9 ? (2 * Math.PI / omega0) : Infinity;
    const redshift = omega0 > 1e-9 ? clockRate / omega0 : 1;
    const TWO_PI = 2 * Math.PI;
    const phaseWrapped = ((phase % TWO_PI) + TWO_PI) % TWO_PI;   // clock-hand angle
    const turns = Math.floor(phase / TWO_PI);                    // completed cycles
    let html = `<div style="${heroStyle()}" title="The manifested cluster's internal de Broglie phase, winding at dφ/dt = ω₀·dτ/dt (the rest-frame Compton clock). Shown wrapped to [0, 2π) — the clock hand.">φ = ${formatFixed(phaseWrapped, 3)} rad</div>`;
    html += `<div style="font-size:16px;color:var(--text-muted);margin:2px 0 8px;">${tagBadge(active ? 'M' : '~M')}internal clock phase (centre voxel) — ${active ? 'running' : 'idle'}</div>`;
    html += row('ω₀ (Compton freq.)', formatFixed(omega0, 3), 'IMPOSED', undefined, 'de Broglie internal-clock frequency ω₀∝K_B. IMPOSED — FTD\'s native flux is massless (no restoring term); the substrate fixes the shape, not the absolute scale (no ℏ).');
    html += row('Clock active', active ? 'ON' : 'OFF', 'M', active ? 'var(--positive-text)' : 'var(--text-muted)', 'de_broglie_clock toggle: adds the Klein-Gordon mass term −ω₀²·J at manifested voxels.');
    html += row('cycles ticked', String(turns), 'M', undefined, 'Completed clock cycles = ⌊φ/2π⌋ since the clock started.');
    html += row('Period 2π/ω₀', Number.isFinite(period) ? `${formatFixed(period, 2)} ticks` : '—', 'D', undefined, 'Rest-frame oscillation period of the cluster\'s flux.');
    html += row('cluster speed u_raw', formatFixed(speed, 4), 'M', undefined, `Raw manifested-cluster speed in nodes/tick; β=|u|/C_SPEED=${formatFixed(speed / C_SPEED, 4)}.`);
    html += row('local latency L', formatFixed(latency, 5), 'M', undefined, 'Latency used with β² in the full causal budget B=β²+L².');
    html += row('dφ/dt', formatFixed(clockRate, 5), '~M', undefined, 'Implemented ω₀·√max(1−β²−L²,0) clock rate under the FTD-0402 contract.');
    html += row('clock ratio', formatFixed(redshift, 5), '~M', redshift < 1 ? 'var(--caution,#fb8c00)' : undefined, 'Selected implementation ratio √max(1−β²−L²,0); it is not a derived covariant prediction.');
    html += `<div class="time-provenance" title="FTD-0402 maps raw nodes/tick to the C_SPEED=1/√3 transport cone and freezes B=β²+L².">${tagBadge('AXIOM')}The moving-clock normalization is the existing clock/bandwidth axiom. ω₀ remains IMPOSED and tied to K_B.</div>`;
    container.innerHTML = html;
}

// ── panel shell ─────────────────────────────────────────────────────────────

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.className = 'scale0-only time-panel';
    const SECTION_HELP = {
        a: 'Lab-frame physical time + the slowest clock rate dτ/dt, minimum lapse f, and FTD γ over the sampled latency field. The [C++] block is the genuine Poisson latency readout (only when the engine sources it); otherwise the rows are the derived |J|² proxy [~M].',
        b: 'Measured proper-time rate dτ/dt as a function of radius from the mass center (solid [~M]) vs a weak-field prediction curve (dashed [D]), with a residual. Clocks slow toward the well.',
        c: 'Two fixed probes — deep (near the mass) and far (near the box edge) — each accumulate proper time τ = Σ√f·dt. The far clock outruns the deep clock; Δτ is the live twin/GPS offset.',
        d: 'Kinematic time dilation. The √(1−v²) [T] and FTD γ(v) [D] curves vs this session’s baked FTD-0252 measured points [M] (offline campaign). The velocity is [IMPOSED] (rigid translation is [BOUNDARY-blocked]). Inset: the departure from exact γ vanishes as L⁻² — γ emerges in the IR.',
        e: 'The imposed de Broglie internal clock (FTD-0271). FTD-0402 normalizes raw speed by C_SPEED and advances phase with the full selected budget B=β²+L². This card is implementation telemetry, not evidence of physical covariance.',
    };
    root.innerHTML = `
        <header class="time-header">
            <span class="time-title">Time Observatory</span>
            <span class="time-mode" id="${PANEL_ID}-mode" title="Gravitational dτ/dt outside the live Poisson readout is the |J|² proxy; kinematic points are baked FTD-0252 (offline). Honest tags per card.">proxy + [M] baked</span>
        </header>
        <section style="${cardStyle(150)}">
            <div style="${titleStyle()}" title="${SECTION_HELP.a}">A · Lab clock &amp; summary ⓘ</div>
            <div id="${PANEL_ID}-card-a"></div>
        </section>
        <section style="${cardStyle(200)}">
            <div style="${titleStyle()}" title="${SECTION_HELP.b}">B · Radial dilation profile (measured vs predicted) ⓘ</div>
            <div id="${PANEL_ID}-card-b"></div>
        </section>
        <section style="${cardStyle(180)}">
            <div style="${titleStyle()}" title="${SECTION_HELP.c}">C · Twin clocks (Δτ) ⓘ</div>
            <div id="${PANEL_ID}-card-c"></div>
        </section>
        <section style="${cardStyle(260)}">
            <div style="${titleStyle()}" title="${SECTION_HELP.d}">D · Kinematic dilation (imposed v) ⓘ</div>
            <div id="${PANEL_ID}-card-d"></div>
        </section>
        <section style="${cardStyle(200)}">
            <div style="${titleStyle()}" title="${SECTION_HELP.e}">E · De Broglie internal clock (FTD-0271) ⓘ</div>
            <div id="${PANEL_ID}-card-e"></div>
        </section>
    `;
    return root;
}

export function mountTimePanel(host, getBridge) {
    if (!host) return null;
    document.getElementById(PANEL_ID)?.remove();
    const panel = buildPanel();
    host.appendChild(panel);

    const el = (id) => panel.querySelector(`#${PANEL_ID}-${id}`);
    const cardA = el('card-a'), cardB = el('card-b'), cardC = el('card-c'), cardD = el('card-d'), cardE = el('card-e');

    let lastMetrics = null;     // { physicalTime, fMin, dtauMin, gammaMax }
    let bridgeId = null;
    let resetVersion = -1;
    let sourceBoundary = null;
    let vImposed = 0.30;        // [IMPOSED] slider state
    // twin-clock accumulators
    let twin = { tauDeep: 0, tauFar: 0, history: [], lDeep: 0, lFar: 0, active: false };
    let lastTick = -1;

    function resetTwin() {
        twin = { tauDeep: 0, tauFar: 0, history: [], lDeep: 0, lFar: 0, active: false };
        lastTick = -1;
    }

    // Card D is event-driven (slider) AND rAF-refreshed. Render once up-front so
    // the baked FTD-0252 curve + tags exist even before the first rAF tick.
    function renderD() { renderCardD(cardD, vImposed); }
    renderD();
    // Delegate slider input (the input is re-created on each renderD()).
    cardD.addEventListener('input', (e) => {
        const slider = e.target.closest(`#${PANEL_ID}-vslider`);
        if (!slider) return;
        vImposed = parseFloat(slider.value);
        const valEl = el('vval');
        if (valEl) valEl.textContent = vImposed.toFixed(2);
        renderD();
    });

    // Sample the latency field once per tick: returns the radial profile + the
    // two twin-probe latencies. The DEEP probe sits at the well floor (the
    // highest latency = slowest clock); the FAR probe sits at the shallowest
    // sampled point (lowest latency = fastest clock). Choosing by well depth
    // (rather than raw radius) makes Δτ = τ_far − τ_deep ≥ 0 for any well
    // geometry — the far/orbiting clock always outruns the deep/surface clock.
    function sampleField(caps) {
        const s = caps.getScale0FieldSamples?.({ kind: 'latency', stride: STRIDE })
            || { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
        const L = caps.latticeSize || 33;
        const c = (L - 1) / 2;
        const center = { x: c, y: c, z: c };
        const prof = (s.values && s.values.length)
            ? radialProfile(s.positions, s.values, center) : [];
        let lDeep = 0, lFar = 0;
        if (prof.length) {
            let mx = -Infinity, mn = Infinity;
            for (const p of prof) { if (p.L > mx) mx = p.L; if (p.L < mn) mn = p.L; }
            lDeep = mx;   // well floor → slowest clock
            lFar = mn;    // shallowest → fastest clock
        }
        return { prof, lDeep, lFar, hasField: prof.length > 0 };
    }

    function update() {
        const b = getBridge?.();
        const caps = b?.capabilities?.scale0 || null;
        if (!caps) return;
        // Scenario reloads can retain the same worker owner. Reset accumulated
        // proper time on the hub reset/source boundary as well as bridge identity
        // so two distinct scientific runs are never joined into one twin trace.
        const nextResetVersion = telemetryHub.getResetVersion?.(0) ?? 0;
        const nextSourceBoundary = timeSourceBoundary();
        const sourceChanged = sourceBoundary !== null && nextSourceBoundary !== null
            && sourceBoundary !== nextSourceBoundary;
        if (b !== bridgeId || nextResetVersion !== resetVersion || sourceChanged) {
            bridgeId = b;
            resetVersion = nextResetVersion;
            resetTwin();
        }
        if (nextSourceBoundary !== null) sourceBoundary = nextSourceBoundary;
        // Gate the heavy work (latency sampler + radial bins) on visibility —
        // the established panel pattern (isPanelLive); idle when the tab is hidden.
        if (!isPanelLive(host)) {
            getBridge?.()?.replaceSamplerWants?.('time-panel', []);
            return;
        }
        getBridge?.()?.replaceSamplerWants?.('time-panel', [`latency@${STRIDE}`]);

        const diagMeta = telemetryHub.getScale0TelemetryMeta?.('diagnostics') ?? null;
        const { diag: hubDiag } = readScale0DiagAudit(b);
        const diag = diagMeta && diagMeta.stale !== true && Number.isFinite(diagMeta.tick)
            ? hubDiag : null;
        const tick = Number.isFinite(diag?.tick) ? diag.tick : null;
        const physicalTime = Number.isFinite(diag?.physicalTime)
            ? diag.physicalTime : tick;
        const dt = Number.isFinite(diag?.dt) && diag.dt > 0 ? diag.dt : null;
        if (!diag || tick === null || !Number.isFinite(physicalTime)) {
            const unavailable = '<div class="time-empty">Current Scale-0 telemetry is unavailable; no zero baseline has been synthesized.</div>';
            cardA.innerHTML = unavailable;
            cardB.innerHTML = unavailable;
            cardC.innerHTML = unavailable;
            lastMetrics = null;
            renderD();
            renderCardE(cardE, {
                hasData: false, active: false, omega0: Number.NaN,
                phase: Number.NaN, speed: Number.NaN, latency: Number.NaN,
                clockRate: Number.NaN,
            });
            return;
        }

        const agg = caps.getScale0GravityMetricAgg?.() || null;
        const { prof, lDeep, lFar, hasField } = sampleField(caps);

        // Card A metrics: deepest latency over the profile drives f_min / dτ/dt.
        let lMax = Number.NaN;
        if (hasField) {
            lMax = 0;
            for (const p of prof) if (p.L > lMax) lMax = p.L;
        }
        const fMin = hasField ? lapse(lMax) : Number.NaN;
        const dtauMin = hasField ? clockRate(lMax) : Number.NaN;
        const gammaMax = hasField ? ftdGamma(lMax, 0) : Number.NaN;   // = 1/√f at v=0
        lastMetrics = { physicalTime, fMin, dtauMin, gammaMax };
        renderCardA(cardA, lastMetrics, agg);

        // Card B: radial profile.
        renderCardB(cardB, prof);

        // Card C: accumulate proper time at the two probes, once per NEW tick.
        if (hasField) {
            if (tick !== lastTick) {
                // On the first valid tick just latch; thereafter accumulate.
                if (lastTick >= 0 && Number.isFinite(dt)) {
                    twin.tauDeep += properTimeStep(lDeep, dt);
                    twin.tauFar += properTimeStep(lFar, dt);
                    twin.history.push(twin.tauFar - twin.tauDeep);
                    if (twin.history.length > SPARK_MAX) twin.history.shift();
                }
                twin.lDeep = lDeep; twin.lFar = lFar; twin.active = true;
                lastTick = tick;
            }
        }
        renderCardC(cardC, twin);

        // Card D is slider/event-driven; keep it fresh too (cheap).
        renderD();

        // Card E — de Broglie internal clock (FTD-0271). Read the toggle + ω₀
        // off the bridge, and sample the centre voxel for the clock phase φ and
        // cluster speed (clock rate dφ/dt = ω₀·√(1−β²−L²)). Only the WASM engine
        // exposes voxel.phase; bridges without that inspection surface cannot
        // provide a per-voxel clock phase.
        const dbActive = (typeof b.getToggle === 'function') ? !!b.getToggle('de_broglie_clock') : false;
        const omega0 = (typeof b.getOmega0 === 'function') ? b.getOmega0() : 1.0;
        let phase = 0, speed = 0, latency = 0, hasPhase = false;
        if (typeof b.inspectVoxel === 'function') {
            const L = Number.isFinite(caps.latticeSize) ? caps.latticeSize
                : (Number.isFinite(diag.latticeSize) ? diag.latticeSize : null);
            const mc = Number.isFinite(L) ? Math.round((L - 1) / 2) : null;
            const vox = mc === null ? null : b.inspectVoxel(mc, mc, mc);
            if (vox && vox.phase !== undefined) {
                phase = Number.isFinite(vox.phase) ? vox.phase : Number.NaN;
                speed = Number.isFinite(vox.speed) ? vox.speed : Number.NaN;
                latency = Number.isFinite(vox.latency) ? vox.latency : Number.NaN;
                hasPhase = [phase, speed, latency].every(Number.isFinite);
            }
        }
        const clockRateNow = hasPhase && Number.isFinite(omega0)
            ? omega0 * clockRate(latency, speed) : Number.NaN;
        renderCardE(cardE, {
            hasData: dbActive || (hasPhase && phase !== 0),
            active: dbActive, omega0, phase, speed, latency, clockRate: clockRateNow,
        });
    }

    // Defer the rAF update loop + stylesheet to first show. A light 2 Hz arm poll
    // watches visibility; the heavy loop never touches the boot / scale-switch
    // critical path until the Time tab is actually opened. (gravity-panel pattern.)
    let liveSub = null;
    const armSub = rafCoordinator.subscribe(`${PANEL_ID}-arm`, { hz: 2, cb: () => {
        if (!isPanelLive(host)) return;
        armSub.unsubscribe();
        ensureTimeCss();
        update();
        liveSub = rafCoordinator.subscribe(PANEL_ID, { hz: HZ, cb: update });
    } });

    const api = {
        update,
        element: panel,
        get lastMetrics() { return lastMetrics; },
        get historyLength() { return twin.history.length; },
        get twin() { return twin; },
        get sourceBoundary() { return sourceBoundary; },
        setImposedV: (v) => { vImposed = Math.max(0, Math.min(0.95, +v || 0)); renderD(); },
        dispose: () => {
            armSub.unsubscribe();
            liveSub?.unsubscribe();
            if (typeof window !== 'undefined' && window.__ftdTimePanel === api) window.__ftdTimePanel = null;
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdTimePanel = api;
    return api;
}

export function initTimePanel() {
    if (typeof document === 'undefined') return null;
    if (typeof window !== 'undefined' && window.__ftdTimePanel) return window.__ftdTimePanel;
    const host = document.getElementById('panel-time');
    if (!host) return null;
    const getBridge = () => resolveActiveScale0BridgeFromWindow();
    return mountTimePanel(host, getBridge);
}
