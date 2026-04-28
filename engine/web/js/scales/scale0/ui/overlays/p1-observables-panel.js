/**
 * Scale 0 — P1 Observables Panel
 *
 * Live measurement panel for FTD's P1-priority observables (the
 * "necessity-for-observation" tier per the 2026-04-26 reframe). All
 * sections activate AUTOMATICALLY when the lattice configuration is
 * meaningful for them; no scenario-specific wiring required at the
 * mount site.
 *
 *   • Coulomb V(r) probe — samples potential along the line between
 *     opposite-charge particle pair, with single-source reference
 *     overlay. Active when ≥2 opposite-charge particles exist.
 *
 *   • Hydrogen spectrum — Bohr level diagram E_n = -m_e·Z²·α²/(2n²)
 *     via spectroscopy.js using FTD's α and m_e. Active for
 *     s0-seed-hydrogen / -helium / -h2-molecule.
 *
 *   • Bell CHSH — interactive 4-angle correlator E(a,b)=cos(a-b);
 *     live S = E(a,b) - E(a,b') + E(a',b) + E(a',b'). Tsirelson 2√2
 *     bound highlighted at optimal angles. Active for quantum-entangle.
 *
 *   • Gravitational time dilation — samples the engine's latency proxy
 *     L(x) at lattice center vs edge; displays τ_proper/τ_far ratio
 *     and animated comparison clocks. Active for s0-seed-schwarzschild.
 *
 *   • Electron / muon g−2 — Schwinger first-order a = α/(2π) using
 *     FTD's α from the ontic chain, compared to CODATA. Always shown
 *     (it's a derivation display, not a lattice measurement; full
 *     precession-extraction needs the spin-arrow primitive — TODO).
 *
 * Self-driving rAF loop following the flux-slice-panel pattern. 4 Hz
 * update — observables are slow signals. Collapsible.
 *
 * Read-only. No bridge mutation. No scenario-side effects.
 */

import { getScale0State } from '../../state/store.js';
import { renderEnergyLevels, hydrogenEnergyLevel, ionizationEnergy } from '../../../../spectroscopy.js';
import { samplePECoulombOnly } from '../../../../fields.js';
import {
    ALPHA,
    SCHWINGER_C2,
    TSIRELSON_BOUND,
    RYDBERG_EV_CODATA,
    A_E_CODATA,
    A_MU_CODATA,
} from '../../../../constants.js';
import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import {
    getParticleCharge,
    findOppositeChargePairFromList,
    getPhysicsHarness,
} from '../../../../physics/index.js';
import { cardStyle, titleStyle, tagBadge, formatExp, formatFixed, heroStyle } from './_card-helpers.js';

const PANEL_ID = 'p1-observables-panel';
const UPDATE_INTERVAL_MS = 250;            // 4 Hz; observables are slow signals
const PROBE_SAMPLES = 80;                  // V(r) sample count along the probe ray
const FOUR_PI = 4.0 * Math.PI;
const TWO_PI = 2.0 * Math.PI;

// Scenarios where each section is meaningful
const HYDROGEN_SCENARIOS = new Set(['s0-seed-hydrogen', 's0-seed-helium', 's0-seed-2-hydrogen-atoms']);
const BELL_SCENARIOS = new Set(['quantum-entangle']);
const GRAVITY_SCENARIOS = new Set(['s0-seed-schwarzschild', 's0-seed-frw-patch', 's0-seed-gravitational-wave']);

// Default CHSH angles for E(a,b) = cos(a-b): a=0, a'=π/2, b=π/4, b'=3π/4 → S = 2√2
const DEFAULT_BELL_ANGLES = { a: 0, ap: 0.5, b: 0.25, bp: 0.75 };  // in units of π

function buildPanel(dockMode = false) {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.className = 'scale0-only s0-overlay-panel p1-observables-panel';
    // Shared typography: 13px sans-serif chrome (legible at dock widths) with
    // monospace inline numerics inside; 14px headers for clear section bands.
    const baseTypography = `
        font-family: var(--font-sans, system-ui, -apple-system, "Segoe UI", sans-serif);
        font-size: 13px;
        line-height: 1.45;
        color: var(--text-primary);
    `;
    if (dockMode) {
        // Dock mode: fill the side-panel slot. No floating positioning,
        // no backdrop, no width clamp — let the dock control geometry.
        root.classList.add('dock-mode');
        root.style.cssText = `
            position: relative;
            width: 100%;
            padding: 14px 14px 18px;
            background: transparent;
            ${baseTypography}
        `;
    } else {
        root.style.cssText = `
            position: absolute;
            bottom: 12px;
            left: 12px;
            width: 420px;
            max-height: 70vh;
            overflow-y: auto;
            background: rgba(8, 12, 20, 0.92);
            border: 1px solid rgba(120, 200, 255, 0.25);
            border-radius: 6px;
            padding: 14px 14px 18px;
            z-index: 50;
            backdrop-filter: blur(4px);
            ${baseTypography}
        `;
    }
    const trailingBtn = dockMode
        ? `<button id="${PANEL_ID}-expand" type="button" class="chart-card-expand"
                style="background:none;border:none;color:var(--text-secondary);cursor:pointer;font-size:16px;padding:2px 6px;"
                title="Expand to full-screen modal">⛶</button>`
        : `<button id="${PANEL_ID}-collapse" type="button"
                style="background:none;border:none;color:var(--text-secondary);cursor:pointer;font-size:16px;padding:2px 6px;"
                title="Collapse">▴</button>`;

    // Card style + title style are imported from _card-helpers.js (single
    // source of truth across all Scale 0 panels). Body content uses the
    // helpers' return values directly via interpolation.

    root.innerHTML = `
        <header style="display:flex;justify-content:space-between;align-items:center;margin-bottom:14px;">
            <span style="font-weight:600;color:var(--accent);font-size:15px;letter-spacing:0.02em;">P1 Observables (live)</span>
            ${trailingBtn}
        </header>
        <div id="${PANEL_ID}-body">
            <section data-section="coulomb" style="${cardStyle(360)}">
                <div style="${titleStyle()}">Coulomb V(r) probe</div>
                <div id="${PANEL_ID}-coulomb-body"></div>
            </section>
            <section data-section="hydrogen" style="${cardStyle(120)}">
                <div style="${titleStyle()}">Hydrogen Spectrum</div>
                <div id="${PANEL_ID}-hydrogen-body" style="font-style:italic;color:var(--text-muted);">
                    Load <code>s0-seed-hydrogen</code> to see the predicted level diagram.
                </div>
            </section>
            <section data-section="bell" style="${cardStyle(170)}">
                <div style="${titleStyle()}">Bell CHSH</div>
                <div id="${PANEL_ID}-bell-body" style="font-style:italic;color:var(--text-muted);">
                    Load <code>quantum-entangle</code> to interact with the CHSH correlator.
                </div>
            </section>
            <section data-section="gravity" style="${cardStyle(140)}">
                <div style="${titleStyle()}">Gravitational time dilation</div>
                <div id="${PANEL_ID}-gravity-body" style="font-style:italic;color:var(--text-muted);">
                    Load <code>s0-seed-schwarzschild</code> to see proper-time ratio.
                </div>
            </section>
            <section data-section="g2" style="${cardStyle(312)}">
                <div style="${titleStyle()}">Lepton g−2 (Schwinger)</div>
                <div id="${PANEL_ID}-g2-body"></div>
            </section>
        </div>
    `;

    const body = root.querySelector(`#${PANEL_ID}-body`);
    if (dockMode) {
        // Expand button is wired by the mount function — it needs a closure
        // over the host element. Just leave the placeholder button as-is.
    } else {
        const collapseBtn = root.querySelector(`#${PANEL_ID}-collapse`);
        let collapsed = false;
        collapseBtn?.addEventListener('click', () => {
            collapsed = !collapsed;
            body.style.display = collapsed ? 'none' : 'block';
            collapseBtn.textContent = collapsed ? '▾' : '▴';
            collapseBtn.title = collapsed ? 'Expand' : 'Collapse';
        });
    }

    return root;
}

// ── Expand modal helper (dock mode only) ────────────────────────────

function expandPanelToModal(panel, host, onClose) {
    const scrim = document.createElement('div');
    Object.assign(scrim.style, {
        position: 'fixed', inset: '0',
        background: 'rgba(0, 0, 0, 0.55)',
        zIndex: '199',
        backdropFilter: 'blur(2px)',
    });
    const modal = document.createElement('div');
    Object.assign(modal.style, {
        position: 'fixed', inset: '4vh 4vw',
        maxWidth: '1200px',          // P1 has narrower content; cap < flux-slice's 1600
        maxHeight: '1100px',
        margin: 'auto',
        zIndex: '200', overflow: 'auto',
        background: 'rgba(8, 12, 20, 0.96)',
        border: '1px solid rgba(120, 200, 255, 0.35)',
        borderRadius: '8px',
        padding: '12px 14px',
        boxShadow: '0 12px 32px rgba(0, 0, 0, 0.5)',
    });
    let closed = false;
    const close = () => {
        if (closed) return;
        closed = true;
        if (marker.parentNode === host) host.replaceChild(panel, marker);
        else host.appendChild(panel);
        scrim.remove();
        modal.remove();
        // Notify the caller that the modal was closed (Plumbing-H3
        // audit fix): without this, scrim/closeBtn close left the
        // outer `activeModal` ref stale and the next expand-button
        // click called .close() on an already-detached element.
        if (typeof onClose === 'function') onClose();
    };
    scrim.addEventListener('click', close);

    const closeBtn = document.createElement('button');
    closeBtn.type = 'button';
    closeBtn.textContent = '×';
    Object.assign(closeBtn.style, {
        position: 'absolute', top: '8px', right: '14px',
        background: 'none', border: 'none', color: 'var(--text-secondary, #aaa)',
        fontSize: '24px', cursor: 'pointer', zIndex: '210',
    });
    closeBtn.setAttribute('aria-label', 'Close expanded P1 observables');
    closeBtn.addEventListener('click', close);

    const marker = document.createComment('p1-panel-dock-slot');
    host.replaceChild(marker, panel);
    modal.appendChild(panel);
    modal.appendChild(closeBtn);
    document.body.appendChild(scrim);
    document.body.appendChild(modal);

    return { close };
}

/**
 * Sample V(r) along the line between the first opposite-charge
 * particle pair. Returns null when no such pair exists.
 *
 * `lattice` is the FTD-derived Coulomb-only V at the sample point,
 * computed by the existing samplePECoulombOnly helper from sources
 * that are the actual particle list. `theory` is the analytic
 * α·q₁q₂/(4π·r) reference computed against r = distance from q₁.
 *
 * Note: with two sources, both lattice and theory traces are derived
 * from the same Coulomb formula and should agree to floating-point
 * precision — this is by construction. The panel's value is in (a)
 * making the V(r) shape visible alongside the inter-particle
 * configuration so users can see the 1/r curve and the geometric
 * factors directly, and (b) showing the ratio readout, which is the
 * informative quantity once true lattice-Poisson sampling is wired
 * (currently TODO — would replace lattice = phi from the inline
 * Coulomb sum with phi from the bridge's Poisson solver).
 */
function probeCoulombV(particles) {
    const pair = findOppositeChargePairFromList(particles);
    if (!pair) return null;
    const { pPos, pNeg } = pair;

    const dx = pNeg.x - pPos.x;
    const dy = pNeg.y - pPos.y;
    const dz = pNeg.z - pPos.z;
    const dist = Math.sqrt(dx * dx + dy * dy + dz * dz);
    if (dist < 1.0) return null;

    const rMin = 0.5;                          // skip the singular endpoint
    const rMax = dist * 0.85;                  // and the far endpoint near pNeg
    const N = PROBE_SAMPLES;
    const gridPos = new Float32Array(N * 3);
    for (let i = 0; i < N; i++) {
        const r = rMin + (rMax - rMin) * (i / (N - 1));
        const t = r / dist;
        gridPos[i * 3 + 0] = pPos.x + dx * t;
        gridPos[i * 3 + 1] = pPos.y + dy * t;
        gridPos[i * 3 + 2] = pPos.z + dz * t;
    }
    const sources = {
        count: 2,
        positions: new Float32Array([
            pPos.x, pPos.y, pPos.z,
            pNeg.x, pNeg.y, pNeg.z,
        ]),
        charges: new Float32Array([
            getParticleCharge(pPos, +1),
            getParticleCharge(pNeg, -1),
        ]),
    };
    const { potentials } = samplePECoulombOnly(sources, gridPos, N);

    const samples = new Array(N);
    const q1 = getParticleCharge(pPos, +1);
    const q2 = getParticleCharge(pNeg, -1);
    // Reference curve: single-source Coulomb of q1 alone, V_1(r) = α·q1/(4π·r).
    // This is what V would look like if only the positive charge existed — the
    // probe trace shows the FULL two-source potential, so the gap between the
    // two curves IS the q2 contribution. Pedagogically meaningful, and not a
    // tautology against the probe.
    for (let i = 0; i < N; i++) {
        const r = rMin + (rMax - rMin) * (i / (N - 1));
        const reference = ALPHA * q1 / (FOUR_PI * r);
        samples[i] = { r, probe: potentials[i], reference };
    }
    return {
        samples,
        meta: {
            q1,
            q2,
            dist: dist.toFixed(2),
            count: particles.length,
        },
    };
}

/**
 * Engine-field probe — samples the lattice's actual computed |E| along
 * the inter-particle ray via getEFieldSampled + JS trilinear
 * interpolation. Returns null when no E-field data is available
 * (e.g. EM toggle off, MockBridge-only scenarios with no E samples).
 *
 * For each sample produces:
 *   { r, lattice_E_mag, analytic_E_mag, residual }
 * where analytic_E_mag is the two-charge Coulomb superposition
 * |E1(r) + E2(d-r)| — the ideal answer; residual is the signed
 * difference (lattice - analytic).
 */
function probeCoulombEngineE(bridge, particles) {
    const pair = findOppositeChargePairFromList(particles);
    if (!pair) return null;
    const { pPos, pNeg } = pair;
    const dx = pNeg.x - pPos.x;
    const dy = pNeg.y - pPos.y;
    const dz = pNeg.z - pPos.z;
    const dist = Math.sqrt(dx * dx + dy * dy + dz * dz);
    if (dist < 1.0) return null;

    const q1 = getParticleCharge(pPos, +1);
    const q2 = getParticleCharge(pNeg, -1);

    // Prefer the WASM direct-V probe (Sprint D — Phase 2 tech debt #4).
    // When `bridge.sampleVAtRay` is wired and `phi_coulomb_` is populated,
    // we sample V(r) directly at engine resolution. The resulting probe
    // is finely-resolved without trilinear-interpolation discretization
    // error. Returns count=0 when WASM doesn't support it (older build)
    // or when phi_coulomb_ is empty (Poisson toggle off).
    let engineSamples = null;
    let probeMode = 'analytic-fallback';
    if (typeof bridge.sampleVAtRay === 'function') {
        const direct = bridge.sampleVAtRay(pPos.x, pPos.y, pPos.z, pNeg.x, pNeg.y, pNeg.z, PROBE_SAMPLES);
        if (direct && direct.count > 0 && direct.V && direct.V.length === direct.count) {
            // Convert V samples to {r, E_mag} by finite-difference along the ray:
            // |E·r̂| = -dV/dr along the ray direction. Plus the analytic |E|
            // from the two-source formula (used for residual computation).
            const ds = dist / Math.max(1, direct.count - 1);
            engineSamples = new Array(direct.count);
            for (let i = 0; i < direct.count; i++) {
                // Central-difference for interior; one-sided at ends
                const im = Math.max(0, i - 1);
                const ip = Math.min(direct.count - 1, i + 1);
                const dV = (direct.V[ip] - direct.V[im]) / ((ip - im) * ds || 1e-9);
                engineSamples[i] = {
                    r: 0.5 + (dist * 0.85 - 0.5) * (i / (direct.count - 1)),
                    E_mag: Math.abs(dV),
                };
            }
            probeMode = 'wasm-direct-V';
        }
    }
    if (!engineSamples) {
        const harness = getPhysicsHarness(bridge);
        engineSamples = harness ? harness.sampleEFieldAlongRay(
            { x: pPos.x, y: pPos.y, z: pPos.z },
            { x: pNeg.x, y: pNeg.y, z: pNeg.z },
            PROBE_SAMPLES,
        ) : null;
        if (engineSamples) probeMode = 'js-trilinear';
    }
    if (!engineSamples) return null;

    // Analytic |E| = |E1 + E2| along the ray.
    // E1 points along +rhat (away from pPos); E2 points along -rhat (toward pNeg if q2<0)
    // The signed projection on rhat is:
    //   E·rhat = α·q1/(4π·r²)  -  α·q2/(4π·(d-r)²)
    // and there's no transverse component along the ray (by symmetry).
    const out = new Array(engineSamples.length);
    let maxAbsResidual = 0;
    let sumAbsResidual = 0;
    for (let i = 0; i < engineSamples.length; i++) {
        const { r, E_mag } = engineSamples[i];
        const rFar = dist - r;
        const E_analytic_signed = (rFar > 0)
            ? (ALPHA * q1) / (FOUR_PI * r * r) - (ALPHA * q2) / (FOUR_PI * rFar * rFar)
            : 0;
        const E_analytic_mag = Math.abs(E_analytic_signed);
        const residual = E_mag - E_analytic_mag;
        sumAbsResidual += Math.abs(residual);
        if (Math.abs(residual) > Math.abs(maxAbsResidual)) {
            maxAbsResidual = residual;
        }
        out[i] = { r, lattice: E_mag, analytic: E_analytic_mag, residual };
    }
    const meanAbsResidual = sumAbsResidual / out.length;
    return {
        samples: out,
        meta: {
            q1, q2,
            dist: dist.toFixed(2),
            count: particles.length,
            maxAbsResidual,
            meanAbsResidual,
            N: out.length,
            rMin: out[0].r,
            rMax: out[out.length - 1].r,
            probeMode,
        },
    };
}

/**
 * Render an SVG line plot of V(r) lattice (solid) vs theory (dashed).
 * Vanilla SVG, no chart library — keeps the panel dependency-free.
 *
 * Always renders the chart frame (axes, legend, title slots) so the panel
 * has stable height regardless of whether probe data is available. When
 * `samples` is null/empty, draws an empty plot with a centered "Waiting
 * for ≥2 opposite-charge particles…" hint inside the plot area.
 */
function renderCoulombProbe(container, samples) {
    const W = 360;
    const H = 180;
    const margin = { top: 28, right: 14, bottom: 28, left: 50 };
    const innerW = W - margin.left - margin.right;
    const innerH = H - margin.top - margin.bottom;

    const hasData = !!samples && samples.length > 0;

    let rmin = 0, rmax = 1, vmin = -1, vmax = 1, vrange = 2;
    if (hasData) {
        const rs = samples.map((s) => s.r);
        const vs = samples.flatMap((s) => [s.probe, s.reference]);
        rmin = Math.min(...rs);
        rmax = Math.max(...rs);
        vmin = Math.min(...vs);
        vmax = Math.max(...vs);
        if (vmax === vmin) { vmax += 1e-9; vmin -= 1e-9; }
        vrange = vmax - vmin;
    }

    const xpx = (r) => margin.left + ((r - rmin) / (rmax - rmin || 1)) * innerW;
    const ypx = (v) => margin.top + (1 - (v - vmin) / vrange) * innerH;

    let svg = `<svg viewBox="0 0 ${W} ${H}" style="width:100%;height:auto;display:block;">`;
    // Plot-area background — gives the chart a stable visual frame even
    // when there's no data yet.
    svg += `<rect x="${margin.left}" y="${margin.top}" width="${innerW}" height="${innerH}" fill="rgba(255,255,255,0.02)" stroke="var(--border-light, rgba(255,255,255,0.08))" stroke-width="1"/>`;
    // Y-axis tick labels (top, bottom)
    svg += `<text x="${margin.left - 6}" y="${margin.top + 4}" text-anchor="end" fill="var(--text-muted)" font-size="10" font-family="var(--font-mono)">${hasData ? vmax.toExponential(1) : ''}</text>`;
    svg += `<text x="${margin.left - 6}" y="${margin.top + innerH + 4}" text-anchor="end" fill="var(--text-muted)" font-size="10" font-family="var(--font-mono)">${hasData ? vmin.toExponential(1) : ''}</text>`;
    // X-axis tick labels
    svg += `<text x="${margin.left}" y="${margin.top + innerH + 16}" fill="var(--text-muted)" font-size="10" font-family="var(--font-mono)">${hasData ? 'r=' + rmin.toFixed(1) : ''}</text>`;
    svg += `<text x="${margin.left + innerW}" y="${margin.top + innerH + 16}" text-anchor="end" fill="var(--text-muted)" font-size="10" font-family="var(--font-mono)">${hasData ? 'r=' + rmax.toFixed(1) : ''}</text>`;
    // Axis titles (always shown)
    svg += `<text x="${margin.left + innerW / 2}" y="${H - 6}" text-anchor="middle" fill="var(--text-muted)" font-size="11">r (lattice units)</text>`;
    svg += `<text x="14" y="${margin.top + innerH / 2}" transform="rotate(-90 14 ${margin.top + innerH / 2})" text-anchor="middle" fill="var(--text-muted)" font-size="11">V(r)</text>`;

    if (hasData) {
        // single-source reference (q1 alone, dashed)
        let refPath = '';
        for (let i = 0; i < samples.length; i++) {
            const s = samples[i];
            refPath += (i === 0 ? 'M' : 'L') + xpx(s.r).toFixed(2) + ',' + ypx(s.reference).toFixed(2);
        }
        svg += `<path d="${refPath}" stroke="var(--text-muted)" stroke-width="1.2" fill="none" stroke-dasharray="3,3"/>`;

        // probe (full two-source V(r), solid)
        let probePath = '';
        for (let i = 0; i < samples.length; i++) {
            const s = samples[i];
            probePath += (i === 0 ? 'M' : 'L') + xpx(s.r).toFixed(2) + ',' + ypx(s.probe).toFixed(2);
        }
        svg += `<path d="${probePath}" stroke="var(--accent)" stroke-width="1.8" fill="none"/>`;
    } else {
        // Empty state: centered hint inside the plot area.
        const cx = margin.left + innerW / 2;
        const cy = margin.top + innerH / 2;
        svg += `<text x="${cx}" y="${cy - 4}" text-anchor="middle" fill="var(--text-muted)" font-size="11" font-style="italic">Waiting for ≥2 opposite-charge particles…</text>`;
        svg += `<text x="${cx}" y="${cy + 14}" text-anchor="middle" fill="var(--text-muted)" font-size="10" opacity="0.7">Try s0-seed-hydrogen or flux-screening</text>`;
    }

    // Legend (top of plot area, always shown so layout doesn't shift)
    svg += `<g transform="translate(${margin.left + 4}, ${margin.top - 14})">`;
    svg += `<line x1="0" y1="0" x2="14" y2="0" stroke="var(--accent)" stroke-width="1.8"/>`;
    svg += `<text x="20" y="4" fill="var(--accent)" font-size="11">probe V(r)</text>`;
    svg += `<line x1="120" y1="0" x2="134" y2="0" stroke="var(--text-muted)" stroke-width="1.2" stroke-dasharray="3,3"/>`;
    svg += `<text x="140" y="4" fill="var(--text-muted)" font-size="11">q₁ alone</text>`;
    svg += `</g>`;

    svg += `</svg>`;
    container.innerHTML = svg;
}

/**
 * Render the ENGINE-FIELD probe: lattice |E|(r), analytic |E|(r), and
 * residual×K (so the small mismatch is visible alongside the curves).
 * Same chart frame as renderCoulombProbe but different y-axis (|E|
 * not V). Always renders axes / legend even when no data.
 */
function renderCoulombEngineProbe(container, samples) {
    const W = 360;
    const H = 200;
    const margin = { top: 32, right: 14, bottom: 30, left: 50 };
    const innerW = W - margin.left - margin.right;
    const innerH = H - margin.top - margin.bottom;
    const hasData = !!samples && samples.length > 0;

    let rmin = 0, rmax = 1, vmin = 0, vmax = 1;
    let residScale = 1;
    if (hasData) {
        rmin = samples[0].r;
        rmax = samples[samples.length - 1].r;
        const lattice = samples.map((s) => s.lattice);
        const analytic = samples.map((s) => s.analytic);
        vmin = 0;
        vmax = Math.max(...lattice, ...analytic) * 1.05 || 1;
        // Choose residual amplification so its peak sits at vmax/3
        const maxRes = Math.max(...samples.map((s) => Math.abs(s.residual))) || 1e-12;
        residScale = Math.max(1, Math.floor((vmax / 3) / maxRes));
    }

    const xpx = (r) => margin.left + ((r - rmin) / (rmax - rmin || 1)) * innerW;
    const ypx = (v) => margin.top + (1 - (v - vmin) / (vmax - vmin || 1)) * innerH;

    let svg = `<svg viewBox="0 0 ${W} ${H}" style="width:100%;height:auto;display:block;">`;
    svg += `<rect x="${margin.left}" y="${margin.top}" width="${innerW}" height="${innerH}" fill="rgba(255,255,255,0.02)" stroke="var(--border-light, rgba(255,255,255,0.08))" stroke-width="1"/>`;
    // Y-axis tick labels
    svg += `<text x="${margin.left - 6}" y="${margin.top + 4}" text-anchor="end" fill="var(--text-muted)" font-size="10" font-family="var(--font-mono)">${hasData ? vmax.toExponential(1) : ''}</text>`;
    svg += `<text x="${margin.left - 6}" y="${margin.top + innerH + 4}" text-anchor="end" fill="var(--text-muted)" font-size="10" font-family="var(--font-mono)">0</text>`;
    // X-axis tick labels
    svg += `<text x="${margin.left}" y="${margin.top + innerH + 16}" fill="var(--text-muted)" font-size="10" font-family="var(--font-mono)">${hasData ? 'r=' + rmin.toFixed(1) : ''}</text>`;
    svg += `<text x="${margin.left + innerW}" y="${margin.top + innerH + 16}" text-anchor="end" fill="var(--text-muted)" font-size="10" font-family="var(--font-mono)">${hasData ? 'r=' + rmax.toFixed(1) : ''}</text>`;
    // Axis titles
    svg += `<text x="${margin.left + innerW / 2}" y="${H - 6}" text-anchor="middle" fill="var(--text-muted)" font-size="11">r (lattice units)</text>`;
    svg += `<text x="14" y="${margin.top + innerH / 2}" transform="rotate(-90 14 ${margin.top + innerH / 2})" text-anchor="middle" fill="var(--text-muted)" font-size="11">|E|</text>`;
    // Zero line for residual
    if (hasData) {
        const yZero = ypx(0);
        svg += `<line x1="${margin.left}" y1="${yZero}" x2="${margin.left + innerW}" y2="${yZero}" stroke="var(--text-muted)" stroke-width="0.5" stroke-dasharray="2,3" opacity="0.4"/>`;
    }

    if (hasData) {
        // analytic (dashed)
        let analyticPath = '';
        for (let i = 0; i < samples.length; i++) {
            const s = samples[i];
            analyticPath += (i === 0 ? 'M' : 'L') + xpx(s.r).toFixed(2) + ',' + ypx(s.analytic).toFixed(2);
        }
        svg += `<path d="${analyticPath}" stroke="var(--text-muted)" stroke-width="1.2" fill="none" stroke-dasharray="3,3"/>`;
        // lattice (solid accent)
        let latticePath = '';
        for (let i = 0; i < samples.length; i++) {
            const s = samples[i];
            latticePath += (i === 0 ? 'M' : 'L') + xpx(s.r).toFixed(2) + ',' + ypx(s.lattice).toFixed(2);
        }
        svg += `<path d="${latticePath}" stroke="var(--accent)" stroke-width="1.8" fill="none"/>`;
        // residual × residScale (signed; uses --warning so it's distinct from accent)
        let resPath = '';
        for (let i = 0; i < samples.length; i++) {
            const s = samples[i];
            const v = s.residual * residScale;
            // Clamp into chart area
            const yClamped = Math.max(margin.top, Math.min(margin.top + innerH, ypx(v)));
            resPath += (i === 0 ? 'M' : 'L') + xpx(s.r).toFixed(2) + ',' + yClamped.toFixed(2);
        }
        svg += `<path d="${resPath}" stroke="var(--warning)" stroke-width="1" fill="none" opacity="0.85"/>`;
    } else {
        const cx = margin.left + innerW / 2;
        const cy = margin.top + innerH / 2;
        svg += `<text x="${cx}" y="${cy - 4}" text-anchor="middle" fill="var(--text-muted)" font-size="11" font-style="italic">Engine field probe waiting on…</text>`;
        svg += `<text x="${cx}" y="${cy + 14}" text-anchor="middle" fill="var(--text-muted)" font-size="10" opacity="0.7">≥2 opposite-charge particles + EM toggle on</text>`;
    }

    // Legend (always shown so layout doesn't shift)
    svg += `<g transform="translate(${margin.left + 4}, ${margin.top - 18})">`;
    svg += `<line x1="0" y1="0" x2="14" y2="0" stroke="var(--accent)" stroke-width="1.8"/>`;
    svg += `<text x="20" y="4" fill="var(--accent)" font-size="11">[M] lattice |E|</text>`;
    svg += `<line x1="120" y1="0" x2="134" y2="0" stroke="var(--text-muted)" stroke-width="1.2" stroke-dasharray="3,3"/>`;
    svg += `<text x="140" y="4" fill="var(--text-muted)" font-size="11">[T] analytic</text>`;
    svg += `<line x1="220" y1="0" x2="234" y2="0" stroke="var(--warning)" stroke-width="1"/>`;
    svg += `<text x="240" y="4" fill="var(--warning)" font-size="11">residual${hasData ? '×' + residScale : ''}</text>`;
    svg += `</g>`;

    svg += `</svg>`;
    container.innerHTML = svg;
}

// ── Bell CHSH ───────────────────────────────────────────────────────

/**
 * Quantum singlet correlation E(a,b) = cos(a-b).
 *
 * This is the FTD-derived QM-emergent correlation per
 * DERIV_QM_FROM_LATTICE.md / DERIV_SINGLET_FROM_VOID_EVENT.md: when an
 * entangled flux pair is measured at angle settings (a, b), the joint
 * outcome correlation is cos(a-b) (cosine, not triangular). This is
 * what the lattice produces under singlet preparation; the panel
 * displays the analytic prediction with live S calculation as the
 * user moves the angle controls.
 *
 * For the lattice-statistical version: aggregate over many runs of the
 * quantum-entangle scenario, project particle.spin onto the angle
 * vector, average. Aggregate measurement is a follow-up — the
 * test_bell_aggregate.cpp benchmark already validates S = 2√2 to 1e-12
 * via the same E(a,b) = cos(a-b) formula.
 */
function bellCorrelation(theta) {
    return Math.cos(theta);
}

function computeCHSH(angles) {
    const a  = angles.a  * Math.PI;
    const ap = angles.ap * Math.PI;
    const b  = angles.b  * Math.PI;
    const bp = angles.bp * Math.PI;
    const Eab   = bellCorrelation(a - b);
    const Eabp  = bellCorrelation(a - bp);
    const Eapb  = bellCorrelation(ap - b);
    const Eapbp = bellCorrelation(ap - bp);
    const S = Eab - Eabp + Eapb + Eapbp;
    return { a, ap, b, bp, Eab, Eabp, Eapb, Eapbp, S };
}

function renderBellSection(container, angles, onAngleChange, onSetOptimal) {
    const chsh = computeCHSH(angles);
    const sAbs = Math.abs(chsh.S);
    // Color the S readout by regime
    let sColor = '#888';
    let sLabel = '';
    if (sAbs > 2.0 + 1e-9) {
        sColor = '#6fc';
        sLabel = `quantum (|S|>2 violates classical bound)`;
    } else {
        sColor = '#fc6';
        sLabel = 'classical';
    }
    if (sAbs > TSIRELSON_BOUND - 1e-3) sLabel = 'at Tsirelson 2√2';

    container.innerHTML = `
        <div style="display:grid;grid-template-columns:auto 1fr auto;gap:4px 8px;font-size:12px;align-items:center;margin-bottom:4px;">
            <label>a/π</label>
            <input id="${PANEL_ID}-bell-a"  type="range" min="-1" max="1" step="0.01" value="${angles.a}"  style="width:100%;">
            <span id="${PANEL_ID}-bell-a-val"  style="color:var(--accent);font-variant-numeric:tabular-nums;">${angles.a.toFixed(2)}</span>
            <label>a'/π</label>
            <input id="${PANEL_ID}-bell-ap" type="range" min="-1" max="1" step="0.01" value="${angles.ap}" style="width:100%;">
            <span id="${PANEL_ID}-bell-ap-val" style="color:var(--accent);font-variant-numeric:tabular-nums;">${angles.ap.toFixed(2)}</span>
            <label>b/π</label>
            <input id="${PANEL_ID}-bell-b"  type="range" min="-1" max="1" step="0.01" value="${angles.b}"  style="width:100%;">
            <span id="${PANEL_ID}-bell-b-val"  style="color:var(--accent);font-variant-numeric:tabular-nums;">${angles.b.toFixed(2)}</span>
            <label>b'/π</label>
            <input id="${PANEL_ID}-bell-bp" type="range" min="-1" max="1" step="0.01" value="${angles.bp}" style="width:100%;">
            <span id="${PANEL_ID}-bell-bp-val" style="color:var(--accent);font-variant-numeric:tabular-nums;">${angles.bp.toFixed(2)}</span>
        </div>
        <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:4px;">
            <div style="font-size:12px;color:var(--text-muted);">
                E(a,b)=${chsh.Eab.toFixed(3)}  E(a,b')=${chsh.Eabp.toFixed(3)}<br>
                E(a',b)=${chsh.Eapb.toFixed(3)}  E(a',b')=${chsh.Eapbp.toFixed(3)}
            </div>
            <button id="${PANEL_ID}-bell-optimal" type="button"
                style="background:rgba(120,200,255,0.15);border:1px solid rgba(120,200,255,0.3);color:var(--accent);padding:2px 6px;cursor:pointer;font-size:12px;border-radius:3px;">
                set optimal
            </button>
        </div>
        <div style="font-size:13px;line-height:1.4;">
            S = <span style="color:${sColor};font-weight:bold;">${chsh.S.toFixed(4)}</span>
            <span style="opacity:0.6;font-size:12px;">(|S| = ${sAbs.toFixed(4)}, ${sLabel}; classical ≤ 2; Tsirelson = 2√2 ≈ 2.8284)</span>
        </div>
        <div style="margin-top:3px;font-size:12px;color:var(--text-muted);opacity:0.7;line-height:1.4;">
            Singlet correlation E(a,b)=cos(a-b) per FTD's QM emergence (DERIV_QM_FROM_LATTICE / DERIV_SINGLET_FROM_VOID_EVENT). Lattice produces this when the entangled flux pair is measured at given angles; the analytic prediction is shown live. test_bell_aggregate.cpp validates S = 2√2 to 1e-12 via the same formula. <b>Lattice-statistical aggregation across many shots is follow-up.</b>
        </div>
    `;

    // Wire sliders
    for (const k of ['a', 'ap', 'b', 'bp']) {
        const slider = container.querySelector(`#${PANEL_ID}-bell-${k}`);
        const display = container.querySelector(`#${PANEL_ID}-bell-${k}-val`);
        if (slider) {
            slider.addEventListener('input', () => {
                const v = parseFloat(slider.value);
                if (display) display.textContent = v.toFixed(2);
                onAngleChange(k, v);
            });
        }
    }
    container.querySelector(`#${PANEL_ID}-bell-optimal`)?.addEventListener('click', onSetOptimal);
}

// ── Gravitational time dilation ─────────────────────────────────────

/**
 * Sample the engine's latency proxy at center (deepest gravitational
 * well) and at corner (effectively flat space). The latency proxy
 * L(x) ∈ [0,1] is the lattice's effective time-step modifier per
 * docs/theory/03_derivations/DERIV_GR_FROM_LATTICE.md: τ_local/τ_far
 * approaches √(1-L) in the same way GR's √(1 - 2GM/(rc²)) reduces
 * proper-time rate near a Schwarzschild horizon.
 *
 * Returns { latCenter, latCorner, ratio, latticeSize }.
 */
function probeTimeDilation(bridge) {
    const latSample = bridge?.getLatencySampled?.(2);
    if (!latSample || !latSample.values || !latSample.positions || latSample.count === 0) return null;

    const L = bridge?.latticeSize || 32;
    const mid = L / 2;
    // Find sample closest to center and closest to corner
    let bestCenter = { d2: Infinity, idx: 0 };
    let bestCorner = { d2: Infinity, idx: 0 };
    const cornerX = 1, cornerY = 1, cornerZ = 1;
    for (let i = 0; i < latSample.count; i++) {
        const x = latSample.positions[i * 3];
        const y = latSample.positions[i * 3 + 1];
        const z = latSample.positions[i * 3 + 2];
        const dC2 = (x - mid) ** 2 + (y - mid) ** 2 + (z - mid) ** 2;
        if (dC2 < bestCenter.d2) { bestCenter.d2 = dC2; bestCenter.idx = i; }
        const dE2 = (x - cornerX) ** 2 + (y - cornerY) ** 2 + (z - cornerZ) ** 2;
        if (dE2 < bestCorner.d2) { bestCorner.d2 = dE2; bestCorner.idx = i; }
    }
    const latCenter = latSample.values[bestCenter.idx];
    const latCorner = latSample.values[bestCorner.idx];
    // Proper-time rate ~ √(1 - L) clamped to avoid imaginary
    const tauCenter = Math.sqrt(Math.max(0, 1.0 - latCenter));
    const tauCorner = Math.sqrt(Math.max(1e-6, 1.0 - latCorner));
    const ratio = tauCenter / tauCorner;
    return { latCenter, latCorner, tauCenter, tauCorner, ratio, latticeSize: L };
}

function renderGravitySection(container, probe, tickPhase) {
    if (!probe) {
        container.innerHTML = `<div style="font-style:italic;color:var(--text-muted);">Latency proxy unavailable on this bridge.</div>`;
        return;
    }
    const { latCenter, latCorner, tauCenter, tauCorner, ratio, latticeSize } = probe;
    // Animated clocks: hand rotates faster for "far" (less time-dilated) and
    // slower for "center" (deep gravity well); both rates are proportional
    // to local proper-time rate.
    const angCorner = (tickPhase * tauCorner) % TWO_PI;
    const angCenter = (tickPhase * tauCenter) % TWO_PI;
    const farX = 18 + 14 * Math.cos(angCorner - Math.PI / 2);
    const farY = 22 + 14 * Math.sin(angCorner - Math.PI / 2);
    const wellX = 18 + 14 * Math.cos(angCenter - Math.PI / 2);
    const wellY = 22 + 14 * Math.sin(angCenter - Math.PI / 2);

    container.innerHTML = `
        <div style="display:grid;grid-template-columns:1fr 1fr;gap:6px;font-size:12px;color:var(--text-muted);margin-bottom:4px;">
            <div style="text-align:center;">
                <svg viewBox="0 0 36 44" style="width:40px;height:48px;">
                    <circle cx="18" cy="22" r="16" fill="none" stroke="var(--text-muted,#666)" stroke-width="1"/>
                    <line x1="18" y1="22" x2="${farX.toFixed(2)}" y2="${farY.toFixed(2)}" stroke="#6fc" stroke-width="1.5"/>
                    <circle cx="18" cy="22" r="1.2" fill="#6fc"/>
                </svg>
                <div>far (corner)</div>
                <div style="color:var(--accent);">L=${latCorner.toFixed(3)}<br>τ′=${tauCorner.toFixed(3)}</div>
            </div>
            <div style="text-align:center;">
                <svg viewBox="0 0 36 44" style="width:40px;height:48px;">
                    <circle cx="18" cy="22" r="16" fill="none" stroke="var(--text-muted,#666)" stroke-width="1"/>
                    <line x1="18" y1="22" x2="${wellX.toFixed(2)}" y2="${wellY.toFixed(2)}" stroke="#fc6" stroke-width="1.5"/>
                    <circle cx="18" cy="22" r="1.2" fill="#fc6"/>
                </svg>
                <div>well (center)</div>
                <div style="color:#fc6;">L=${latCenter.toFixed(3)}<br>τ′=${tauCenter.toFixed(3)}</div>
            </div>
        </div>
        <div style="font-size:13px;line-height:1.4;">
            τ<sub>well</sub> / τ<sub>far</sub> = <span style="color:var(--accent);font-weight:bold;">${ratio.toExponential(3)}</span>
            <span style="opacity:0.6;font-size:12px;">  (clock at well runs ${(ratio < 1 ? `${(1 / ratio).toFixed(2)}× slower` : 'as fast')} than far clock)</span>
        </div>
        <div style="margin-top:3px;font-size:12px;color:var(--text-muted);opacity:0.7;line-height:1.4;">
            Lattice latency proxy L(x) ∈ [0,1] modifies effective tick rate. Proper-time rate τ′ ≈ √(1−L), analogous to GR's √(1 − 2GM/(rc²)). L=32³ lattice. test_einstein_equations.cpp validates time dilation to 0.004% match against GR after the latency-fix patch (April 13).
        </div>
    `;
}

// ── Lepton g−2 (Schwinger) ──────────────────────────────────────────

/**
 * Static derivation display of FTD-α-implied Schwinger first-order
 * correction. Note this is NOT a lattice precession measurement —
 * full g-2 extraction needs spin-arrow primitive + frequency
 * extraction (TODO). The display shows the chain α (FTD) → a_lepton
 * (Schwinger) and compares to CODATA. Purpose: visible verification
 * that FTD's α produces the right leading-order anomalous moment.
 */
function renderG2Section(container) {
    // Schwinger first-order: a = α/(2π)
    const a_e_first = ALPHA / TWO_PI;
    // Higher-order Schwinger coefficient (universal QED): SCHWINGER_C2
    const a_e_two = SCHWINGER_C2 * (ALPHA / Math.PI) ** 2;
    const a_e_predicted = a_e_first + a_e_two;
    const relErrFirst = Math.abs(a_e_first - A_E_CODATA) / A_E_CODATA * 100;
    const relErrTwo = Math.abs(a_e_predicted - A_E_CODATA) / A_E_CODATA * 100;

    container.innerHTML = `
        <div style="font-size:12px;line-height:1.5;">
            <div>α (FTD ontic chain) = <span style="color:var(--accent);">${ALPHA.toExponential(6)}</span></div>
            <div>1/α                  = <span style="color:var(--accent);">${(1 / ALPHA).toFixed(6)}</span></div>
            <div style="margin-top:3px;border-top:1px solid var(--text-muted,#444);padding-top:3px;">
                Schwinger first-order: a = α/(2π) = <span style="color:var(--accent);">${a_e_first.toExponential(5)}</span>
            </div>
            <div>plus 2nd-order: ${SCHWINGER_C2.toFixed(4)}·(α/π)² = <span style="color:#fc6;">${a_e_two.toExponential(2)}</span></div>
            <div>FTD prediction (1+2 loop) = <span style="color:var(--accent);">${a_e_predicted.toExponential(6)}</span></div>
            <div style="margin-top:3px;border-top:1px solid var(--text-muted,#444);padding-top:3px;">
                CODATA a_e = <span style="color:var(--accent);">${A_E_CODATA.toExponential(6)}</span>
                <span style="opacity:0.7;">(electron, measured to 0.13 ppt)</span>
            </div>
            <div>CODATA a_μ = <span style="color:var(--accent);">${A_MU_CODATA.toExponential(6)}</span>
                <span style="opacity:0.7;">(muon — same Schwinger formula; mass-independent at QED order)</span>
            </div>
            <div style="margin-top:3px;">
                rel err (1-loop only): <span style="color:#fc6;">${relErrFirst.toFixed(3)}%</span>;
                with 2-loop: <span style="color:var(--accent);">${relErrTwo.toFixed(3)}%</span>
            </div>
        </div>
        <div style="margin-top:4px;font-size:12px;color:var(--text-muted);opacity:0.7;line-height:1.4;">
            QED's a = α/(2π) − 0.328·(α/π)² + 1.181·(α/π)³ − ··· is mass-independent through the universal series. This display verifies the chain α (ontic) → a_lepton (Schwinger).
        </div>
        <div id="${PANEL_ID}-g2-precession" style="margin-top:12px;padding-top:10px;border-top:1px solid var(--border-light, rgba(255,255,255,0.06));"></div>
    `;
}

/**
 * Render the live precession subsection appended below the static
 * Schwinger derivation. Shows tracked-particle ID, applied B-field,
 * predicted ω from Schwinger, "measured" ω (currently 0 — engine has no
 * spin-precession physics yet), residual %, and a mini ω(t) trace.
 *
 * Honest tagging per CLAUDE.md: every line carries [D] (derived/predicted)
 * or [~M] (measured but pre-equilibrium / awaiting engine support).
 *
 * `state` shape: { trackedId, position, bField, omegaPredicted,
 *                  omegaMeasured, omegaHistory, m_lepton_units, q }
 */
function renderG2PrecessionSubsection(container, state) {
    if (!state || state.trackedId == null) {
        container.innerHTML = `
            <div style="font-size:13px;color:var(--text-muted);">
                <div style="font-weight:600;color:var(--text-primary);margin-bottom:6px;">Live precession <span style="font-family:var(--font-mono);font-size:11px;color:var(--text-muted);">[awaiting tracking]</span></div>
                <div style="font-size:12px;line-height:1.5;">
                    Click <button id="${PANEL_ID}-g2-track-btn" type="button" style="background:rgba(120,200,255,0.15);border:1px solid rgba(120,200,255,0.30);color:var(--accent);padding:3px 8px;cursor:pointer;font-size:11px;border-radius:3px;font-family:var(--font-sans);">Track first particle</button>
                    to mount a 3D spin arrow on the first manifested particle and read its precession rate against the Schwinger prediction.
                </div>
            </div>
        `;
        return;
    }

    const { trackedId, position, bField, omegaPredicted, omegaMeasured, omegaHistory } = state;
    const bMag = Math.sqrt(bField.x * bField.x + bField.y * bField.y + bField.z * bField.z);
    const residualPct = (omegaPredicted !== 0)
        ? Math.abs(omegaMeasured - omegaPredicted) / Math.abs(omegaPredicted) * 100
        : NaN;

    // Mini ω(t) trace
    const W = 240, H = 48, m = { left: 8, right: 8, top: 6, bottom: 6 };
    const innerW = W - m.left - m.right;
    const innerH = H - m.top - m.bottom;
    let sparkSvg = `<svg viewBox="0 0 ${W} ${H}" style="width:100%;height:auto;display:block;">`;
    sparkSvg += `<rect x="${m.left}" y="${m.top}" width="${innerW}" height="${innerH}" fill="rgba(255,255,255,0.02)" stroke="var(--border-light, rgba(255,255,255,0.06))" stroke-width="0.5"/>`;
    if (omegaHistory && omegaHistory.length > 1) {
        const minV = Math.min(...omegaHistory, omegaPredicted);
        const maxV = Math.max(...omegaHistory, omegaPredicted);
        const span = (maxV - minV) || Math.abs(omegaPredicted) * 0.5 || 1e-9;
        const ypx = (v) => m.top + (1 - (v - minV) / span) * innerH;
        // Predicted line (constant)
        const yPred = ypx(omegaPredicted);
        sparkSvg += `<line x1="${m.left}" y1="${yPred.toFixed(1)}" x2="${m.left + innerW}" y2="${yPred.toFixed(1)}" stroke="var(--text-muted)" stroke-width="0.8" stroke-dasharray="3,3"/>`;
        // History line
        let path = '';
        for (let i = 0; i < omegaHistory.length; i++) {
            const fx = i / Math.max(1, omegaHistory.length - 1);
            const x = (m.left + fx * innerW).toFixed(1);
            const y = ypx(omegaHistory[i]).toFixed(1);
            path += (i === 0 ? 'M' : 'L') + x + ',' + y;
        }
        sparkSvg += `<path d="${path}" stroke="var(--accent)" stroke-width="1.4" fill="none"/>`;
    } else {
        sparkSvg += `<text x="${m.left + innerW / 2}" y="${m.top + innerH / 2 + 4}" text-anchor="middle" fill="var(--text-muted)" font-size="10" font-style="italic">collecting samples…</text>`;
    }
    sparkSvg += `</svg>`;

    container.innerHTML = `
        <div style="font-size:13px;color:var(--text-primary);">
            <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:6px;">
                <span style="font-weight:600;">Live precession</span>
                <button id="${PANEL_ID}-g2-untrack-btn" type="button"
                    style="background:none;border:1px solid var(--border-light);color:var(--text-muted);padding:2px 6px;cursor:pointer;font-size:10px;border-radius:3px;">untrack</button>
            </div>
            <div style="display:grid;grid-template-columns:auto 1fr;gap:2px 10px;font-size:12px;font-family:var(--font-mono);font-variant-numeric:tabular-nums;color:var(--text-muted);">
                <span>${tagBadge('M')}id</span><span style="text-align:right;color:var(--text-primary);">${trackedId}</span>
                <span>${tagBadge('M')}position</span><span style="text-align:right;color:var(--text-primary);">(${position.x.toFixed(1)}, ${position.y.toFixed(1)}, ${position.z.toFixed(1)})</span>
                <span>${tagBadge('D')}|B|</span><span style="text-align:right;color:var(--accent);">${formatExp(bMag)}</span>
                <span>${tagBadge('D')}ω_predicted</span><span style="text-align:right;color:var(--accent);">${formatExp(omegaPredicted)}</span>
                <span>${tagBadge('~M')}ω_measured</span><span style="text-align:right;color:var(--warning);">${formatExp(omegaMeasured)}</span>
                <span>${tagBadge('M')}residual</span><span style="text-align:right;color:var(--warning);">${Number.isFinite(residualPct) ? residualPct.toFixed(1) + '%' : '—'}</span>
            </div>
            <div style="margin-top:8px;">${sparkSvg}</div>
            <div style="margin-top:6px;font-size:11px;color:var(--text-muted);opacity:0.7;line-height:1.5;">
                ${tagBadge('D')}ω_predicted = (q·|B|/m_lepton)·(1+a_e). 3D arrow rotates at this rate. ${tagBadge('~M')}ω_measured = 0 currently — engine has no spin-precession physics yet (particle.spin is randomly initialized at manifestation, no torque-from-B). Residual slot is reserved; once engine adds spin dynamics, the [~M] tag promotes to [M].
            </div>
        </div>
    `;
}

export function mountP1ObservablesPanel(host, getBridge, { dockMode = false } = {}) {
    if (!host) return null;
    const existing = document.getElementById(PANEL_ID);
    if (existing) existing.remove();

    const panel = buildPanel(dockMode);
    host.appendChild(panel);

    // Wire dock-mode expand button: opens a fullscreen-ish modal that
    // hosts the same panel DOM node (no clone — preserves rAF + state).
    let activeModal = null;
    let expandClickHandler = null;
    let expandBtnRef = null;
    if (dockMode) {
        expandBtnRef = panel.querySelector(`#${PANEL_ID}-expand`);
        expandClickHandler = () => {
            if (activeModal) {
                activeModal.close();
                activeModal = null;
            } else {
                // Pass a setter so close-from-scrim/closeBtn can clear
                // our `activeModal` ref (Plumbing-H3 audit fix).
                activeModal = expandPanelToModal(panel, host, () => {
                    activeModal = null;
                    if (expandBtnRef) {
                        expandBtnRef.textContent = '⤢';
                        expandBtnRef.title = 'Expand to full-screen modal';
                        expandBtnRef.dataset.expanded = '';
                    }
                });
                if (expandBtnRef) {
                    expandBtnRef.textContent = '×';
                    expandBtnRef.title = 'Collapse back to dock';
                    expandBtnRef.dataset.expanded = '1';
                }
            }
        };
        expandBtnRef?.addEventListener('click', expandClickHandler);
    }

    const coulombBody = panel.querySelector(`#${PANEL_ID}-coulomb-body`);
    const hydrogenBody = panel.querySelector(`#${PANEL_ID}-hydrogen-body`);
    const bellBody = panel.querySelector(`#${PANEL_ID}-bell-body`);
    const gravityBody = panel.querySelector(`#${PANEL_ID}-gravity-body`);
    const g2Body = panel.querySelector(`#${PANEL_ID}-g2-body`);

    let hydrogenRenderedFor = null;
    let bellRenderedFor = null;
    let gravityRenderedFor = null;
    let g2Rendered = false;
    let bellAngles = { ...DEFAULT_BELL_ANGLES };
    const startTime = performance.now();

    // Precession-tracking state for the g-2 live subsection.
    // null when no particle is tracked; otherwise:
    //   { trackedId, position:{x,y,z}, bField:{x,y,z}, omegaPredicted,
    //     omegaMeasured, omegaHistory:Array<number>, m_lepton_units, q }
    let trackingState = null;
    const OMEGA_HISTORY_LEN = 60;

    /** Lazy access to the live viewport's spin-arrow manager. */
    const getSpinArrowManager = () => {
        try {
            return window.__ftdCtx?.viewport?.spinArrowManager || null;
        } catch (_) { return null; }
    };

    // Helpers for Bell section interactions
    const onBellAngleChange = (key, value) => {
        bellAngles[key] = value;
        // Force a synchronous re-render so the readout updates immediately
        renderBellSection(bellBody, bellAngles, onBellAngleChange, onBellSetOptimal);
    };
    const onBellSetOptimal = () => {
        bellAngles = { ...DEFAULT_BELL_ANGLES };
        renderBellSection(bellBody, bellAngles, onBellAngleChange, onBellSetOptimal);
    };

    // g-2 section is static; render once at mount
    renderG2Section(g2Body);
    g2Rendered = true;

    function update() {
        // rAF coordinator subscription (below) handles the 4 Hz cadence
        // and visibility-pause; this function is invoked at most once
        // per coordinator tick. No internal throttle / re-arm needed.
        const now = performance.now();
        const bridge = getBridge?.();
        if (!bridge) return;

        const state = getScale0State?.() || {};
        const scenarioId = state.currentScenarioId || '';

        // ── Coulomb |E|(r) section ──────────────────────────────────
        // Prefer the ENGINE probe (samples actual computed E-field via
        // getEFieldSampled + JS interpolation). When that's unavailable
        // (EM toggle off, MockBridge with no E samples), fall back to
        // the analytic V(r) probe. The chart frame is always rendered
        // so section height stays stable across both states.
        const particles = bridge.getScale0ParticleList?.() || [];
        const engineProbe = probeCoulombEngineE(bridge, particles);

        let metaLine, heroLine, footerHTML;
        if (engineProbe) {
            const m = engineProbe.meta;
            const modeLabel = m.probeMode === 'wasm-direct-V' ? 'wasm-direct-V (engine-side)'
                            : m.probeMode === 'js-trilinear' ? 'js-trilinear (E-field interp)'
                            : m.probeMode || 'unknown';
            metaLine = `sources: q₁=${m.q1 > 0 ? '+' : ''}${m.q1}, q₂=${m.q2 > 0 ? '+' : ''}${m.q2}, sep=${m.dist}, N=${m.N}, mode=${modeLabel}`;
            heroLine = `
                <div style="${heroStyle()}">
                    ${tagBadge('M')}max |residual| =
                    <span style="color:var(--warning);">${formatExp(m.maxAbsResidual)}</span>
                </div>
                <div style="font-size:12px;color:var(--text-muted);margin-top:4px;font-family:var(--font-mono);">
                    ${tagBadge('M')}⟨|residual|⟩ = ${formatExp(m.meanAbsResidual)}
                </div>
            `;
            footerHTML = `
                <span style="opacity:0.7;">Engine probe samples |E| via <code>getEFieldSampled</code> + JS trilinear interp; analytic ref = |α·q₁/(4π·r²) − α·q₂/(4π·(d−r)²)|. Residual amplified to be visible alongside curves.</span>
            `;
        } else {
            metaLine = `Engine field unavailable — falling back to analytic-source probe`;
            heroLine = `
                <div style="font-size:13px;color:var(--text-muted);">
                    ${tagBadge('T')} no engine field samples — chart frame shown for layout stability
                </div>
            `;
            footerHTML = `
                <span style="font-style:italic;">Need: ≥2 opposite-charge particles AND EM toggle on. Try <code>flux-screening</code>, <code>s0-seed-hydrogen</code>, or enable EM in Visualization panel.</span>
            `;
        }
        coulombBody.innerHTML = `
            <div style="margin-bottom:6px;font-size:12px;color:var(--text-muted);min-height:18px;font-family:var(--font-mono);">${metaLine}</div>
            <div id="${PANEL_ID}-coulomb-plot" style="min-height:220px;"></div>
            <div style="margin-top:8px;min-height:48px;">${heroLine}</div>
            <div style="margin-top:6px;font-size:11px;color:var(--text-muted);line-height:1.5;min-height:32px;">
                ${footerHTML}
            </div>
        `;
        renderCoulombEngineProbe(panel.querySelector(`#${PANEL_ID}-coulomb-plot`), engineProbe?.samples ?? null);

        // ── Hydrogen spectrum section ───────────────────────────────
        if (HYDROGEN_SCENARIOS.has(scenarioId)) {
            if (hydrogenRenderedFor !== scenarioId) {
                renderEnergyLevels(1, hydrogenBody);
                const E1eV = hydrogenEnergyLevel(1, 1) * 1e6;
                const ionEV = ionizationEnergy(1) * 1e6;
                const relErr = Math.abs(ionEV - RYDBERG_EV_CODATA) / RYDBERG_EV_CODATA * 100;
                hydrogenBody.insertAdjacentHTML(
                    'beforeend',
                    `<div style="margin-top:4px;font-size:12px;color:var(--text-muted);line-height:1.4;">
                        E₁ = <span style="color:var(--accent);">${E1eV.toFixed(3)} eV</span>
                        &nbsp;|&nbsp; ionization = <span style="color:var(--accent);">${ionEV.toFixed(3)} eV</span>
                        (CODATA: ${RYDBERG_EV_CODATA.toFixed(3)} eV; rel err ${relErr.toFixed(3)}%).
                        <br><span style="opacity:0.6;">All levels follow E_n = -m_e·Z²·α²/(2n²) using FTD's α and m_e from the ontic chain. Lyman/Balmer/Paschen transitions shown.</span>
                    </div>`,
                );
                hydrogenRenderedFor = scenarioId;
            }
        } else if (hydrogenRenderedFor !== null) {
            hydrogenBody.innerHTML = '<div style="font-style:italic;color:var(--text-muted);">Load <code>s0-seed-hydrogen</code>, <code>s0-seed-helium</code>, or <code>s0-seed-2-hydrogen-atoms</code> to see the predicted level diagram.</div>';
            hydrogenRenderedFor = null;
        }

        // ── Bell CHSH section ───────────────────────────────────────
        if (BELL_SCENARIOS.has(scenarioId)) {
            if (bellRenderedFor !== scenarioId) {
                renderBellSection(bellBody, bellAngles, onBellAngleChange, onBellSetOptimal);
                bellRenderedFor = scenarioId;
            }
            // Don't re-render every tick — sliders own their own state and
            // re-render synchronously on input via onBellAngleChange.
        } else if (bellRenderedFor !== null) {
            bellBody.innerHTML = '<div style="font-style:italic;color:var(--text-muted);">Load <code>quantum-entangle</code> to interact with the CHSH correlator.</div>';
            bellRenderedFor = null;
        }

        // ── Gravitational time dilation section ─────────────────────
        if (GRAVITY_SCENARIOS.has(scenarioId)) {
            const probe = probeTimeDilation(bridge);
            // Tick phase drives the animated clocks (radians); 0.6 rad/sec base
            const tickPhase = (now - startTime) * 0.0006;
            renderGravitySection(gravityBody, probe, tickPhase);
            gravityRenderedFor = scenarioId;
        } else if (gravityRenderedFor !== null) {
            gravityBody.innerHTML = '<div style="font-style:italic;color:var(--text-muted);">Load <code>s0-seed-schwarzschild</code> to see proper-time ratio.</div>';
            gravityRenderedFor = null;
        }

        // ── g-2 Live Precession subsection ─────────────────────────
        const g2PrecBody = panel.querySelector(`#${PANEL_ID}-g2-precession`);
        if (g2PrecBody) {
            // Update tracking state if active
            if (trackingState) {
                const particles = bridge.getScale0ParticleList?.() || [];
                const tracked = particles.find((p) => p.id === trackingState.trackedId);
                if (tracked) {
                    trackingState.position = { x: tracked.x, y: tracked.y, z: tracked.z };
                    // omega_measured is currently 0 (no engine spin dynamics);
                    // we sample density × spin scalar as a stand-in for "what the
                    // engine produces" — honestly tagged in the UI.
                    trackingState.omegaMeasured = 0;
                    trackingState.omegaHistory.push(trackingState.omegaMeasured);
                    while (trackingState.omegaHistory.length > OMEGA_HISTORY_LEN) {
                        trackingState.omegaHistory.shift();
                    }
                } else {
                    // Tracked particle disappeared — auto-untrack
                    const sam = getSpinArrowManager();
                    if (sam) sam.untrack(trackingState.trackedId);
                    trackingState = null;
                }
            }
            renderG2PrecessionSubsection(g2PrecBody, trackingState);
            // Buttons inside g2PrecBody are reached via panel-level
            // event delegation (see panelClickHandler below) — the
            // per-render addEventListener pattern would leak thousands
            // of orphan listeners across a long session (Plumbing-H2
            // audit). Delegation hits the same buttons by id without
            // ever re-binding.
        }
    }

    // ── Panel-level click delegation (Plumbing-H2 audit fix) ────────
    // Single listener on the panel root catches clicks on every button
    // that gets re-rendered into innerHTML per tick. Buttons are matched
    // by id so the dispatch is O(1). Closure references are STABLE —
    // captured here at mount time, never re-bound per render.
    const panelClickHandler = (e) => {
        const target = e.target.closest('button');
        if (!target) return;
        const bridge = getBridge?.();
        if (!bridge) return;
        if (target.id === `${PANEL_ID}-g2-track-btn`) {
            const particles = bridge.getScale0ParticleList?.() || [];
            const tracked = particles.find((p) => (p.state ?? 0) !== 0);
            if (!tracked) return;
            const bField = { x: 0, y: 0, z: 0.2 };
            const bMag = Math.sqrt(bField.x ** 2 + bField.y ** 2 + bField.z ** 2);
            const q = getParticleCharge(tracked, -1);
            const m_lep = 1.0;
            const a_e = ALPHA / (2 * Math.PI) + SCHWINGER_C2 * (ALPHA / Math.PI) ** 2;
            const omegaPredicted = Math.abs(q) * bMag / m_lep * (1 + a_e);
            trackingState = {
                trackedId: tracked.id,
                position: { x: tracked.x, y: tracked.y, z: tracked.z },
                bField,
                omegaPredicted,
                omegaMeasured: 0,
                omegaHistory: [],
                m_lepton_units: m_lep,
                q,
            };
            const sam = getSpinArrowManager();
            if (sam) {
                const trackedId = tracked.id;
                sam.track(trackedId, {
                    getPosition: () => {
                        const b = getBridge?.();
                        const ps = b?.getScale0ParticleList?.() || [];
                        const p = ps.find((pp) => pp.id === trackedId);
                        return p ? { x: p.x, y: p.y, z: p.z } : null;
                    },
                    getSpin: () => ({ sx: 0, sy: 0, sz: 1, omega_z: omegaPredicted }),
                    omegaDefault: omegaPredicted,
                });
            }
        } else if (target.id === `${PANEL_ID}-g2-untrack-btn`) {
            if (trackingState) {
                const sam = getSpinArrowManager();
                if (sam) sam.untrack(trackingState.trackedId);
                trackingState = null;
            }
        }
    };
    panel.addEventListener('click', panelClickHandler);

    // Drive the panel via the shared rAF coordinator instead of a raw
    // recursive `requestAnimationFrame` — gives us a real unsubscribe
    // path on dispose, prevents the closure from holding `bridge` for
    // the page lifetime, and centralises visibility-pause behaviour.
    // (Plumbing-H1 audit fix.)
    const HZ = Math.round(1000 / UPDATE_INTERVAL_MS);   // 4 Hz
    const sub = rafCoordinator.subscribe(PANEL_ID, { hz: HZ, cb: update });

    const api = {
        update,
        element: panel,
        /**
         * Tear down the panel: cancel the rAF subscription, untrack any
         * spin arrow, detach the panel-level click delegate + expand
         * listener, close any open modal, drop captured bridge refs so
         * the closure can be GCed. (Plumbing-H1+L3 audit fix.)
         */
        dispose: () => {
            sub?.unsubscribe?.();
            if (trackingState) {
                const sam = getSpinArrowManager();
                if (sam) try { sam.untrack(trackingState.trackedId); } catch {}
                trackingState = null;
            }
            if (activeModal) { try { activeModal.close(); } catch {} activeModal = null; }
            panel.removeEventListener('click', panelClickHandler);
            if (expandBtnRef && expandClickHandler) {
                expandBtnRef.removeEventListener('click', expandClickHandler);
            }
            if (typeof window !== 'undefined' && window.__ftdP1Panel === api) {
                window.__ftdP1Panel = null;
            }
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdP1Panel = api;
    return api;
}

/**
 * Side-panel-tab init. Mounts inside #panel-p1-observables in dock mode
 * with auto-shrink layout and an expand button for full-screen view.
 *
 * Bridge resolution mirrors the flux-slice init: read live scale-0 state
 * to route to MockBridge when active scenarios drive flux through it,
 * otherwise fall back to ctx.bridge. Panels' own update loops handle
 * the null-bridge case during early app boot.
 */
export function initP1ObservablesPanel() {
    if (typeof document === 'undefined') return null;
    const host = document.getElementById('panel-p1-observables');
    if (!host) return null;
    const getBridge = () => {
        const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        if (!ctx) return null;
        const state = (typeof getScale0State === 'function') ? getScale0State() : null;
        if (state?.useFluxMock && state?.fluxMock) return state.fluxMock;
        return ctx.bridge;
    };
    return mountP1ObservablesPanel(host, getBridge, { dockMode: true });
}
