// Two Sectors — docked Scale-0 causality demo (FTD-0004 Phase G).
//
// Mounts into #panel-two-sectors (registry id 'two-sectors'). NOT a floating overlay.
// Shows the engine's two field sectors side by side, on the REAL engine (captured on
// an isolated bridge by two-sectors-capture.js, then replayed):
//   · LONGITUDINAL · Gauss constraint — a static charge's Coulomb field fills the box
//     in a few ticks ("instant"); it's a constraint, carries no signal.
//   · TRANSVERSE · radiative — a flux-pulse shell expanding at c = 1/√3 voxel/tick.
// The chart fits the shell-radius-vs-tick slope → ≈ 0.577 = 1/√3. Honest framing
// throughout (no superluminal signalling; Postulate 4 holds); asserts NO new tags.

import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import { paintSliceToCanvas } from './slice-render.js';
import { rampEmEnergy, rampViridis } from '../../../../viewport/color-ramps.js';
import { captureTwoSectors, TICKS, C_LATTICE } from './two-sectors-capture.js';

const PANEL_ID = 'two-sectors-panel';
const CAN_PX = 160;                 // canvas internal resolution (square; nearest-neighbour upscale)
const SLOPE_TOL = 0.05;             // ✓ when |fitted slope − 1/√3| < SLOPE_TOL

function ensureCss() {
    if (typeof document === 'undefined' || document.getElementById(`${PANEL_ID}-css`)) return;
    const s = document.createElement('style');
    s.id = `${PANEL_ID}-css`;
    s.textContent = `
    #${PANEL_ID}{font-family:var(--font-sans,sans-serif);font-size:12px;color:var(--text-primary,#eee);padding:2px}
    #${PANEL_ID} .ts-title{font-weight:600;margin:2px 0 6px}
    #${PANEL_ID} .ts-title small{color:var(--text-muted,#888);font-weight:400}
    #${PANEL_ID} .ts-cells{display:flex;gap:8px;margin:2px 0 4px}
    #${PANEL_ID} .ts-cell{flex:1 1 0;min-width:0}
    #${PANEL_ID} .ts-lab{font-size:10px;font-weight:600;margin:0 0 2px;color:var(--text-secondary,#ccc)}
    #${PANEL_ID} .ts-lab small{display:block;font-weight:400;font-size:9px;color:var(--text-muted,#888)}
    #${PANEL_ID} canvas.ts-canvas{width:100%;aspect-ratio:1;display:block;background:#0a0d14;border-radius:4px;image-rendering:pixelated}
    #${PANEL_ID} svg.ts-chart{width:100%;display:block;background:#0c0c11;border-radius:6px;margin-top:2px}
    #${PANEL_ID} .ts-ctl{display:flex;align-items:center;gap:6px;margin:6px 0 2px;flex-wrap:wrap}
    #${PANEL_ID} .ts-ctl button{font:inherit;font-size:11px;padding:2px 8px;border-radius:4px;cursor:pointer;
        background:var(--surface-2,#1b1f2a);color:var(--text-secondary,#ccc);border:0.5px solid var(--border-light,rgba(255,255,255,0.12))}
    #${PANEL_ID} .ts-ctl button:hover{background:var(--surface-3,#262b38)}
    #${PANEL_ID} .ts-counter{font-variant-numeric:tabular-nums;color:var(--text-muted,#888);font-size:10px}
    #${PANEL_ID} .ts-status{margin-left:auto;font-size:10px;color:#7CFC8C;text-align:right}
    #${PANEL_ID} .ts-foot{margin-top:8px;padding-top:7px;border-top:0.5px solid var(--border-light,rgba(255,255,255,0.12));
        font-size:10px;color:var(--text-muted,#888);line-height:1.5}
    #${PANEL_ID} .ts-foot b{color:var(--text-secondary,#aaa)}`;
    document.head.appendChild(s);
}

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.innerHTML = `
        <div class="ts-title">Two Sectors <small>· constraint vs radiative · FTD-0004</small></div>
        <div class="ts-cells">
          <div class="ts-cell">
            <div class="ts-lab">Longitudinal · constraint<small>fills globally — carries no signal</small></div>
            <canvas class="ts-canvas" id="${PANEL_ID}-canA" width="${CAN_PX}" height="${CAN_PX}" role="img"
              aria-label="Longitudinal sector: a static charge's Coulomb field fills the mid-plane within a few ticks — a constraint that carries no signal."></canvas>
          </div>
          <div class="ts-cell">
            <div class="ts-lab">Transverse · radiative<small>c = 1/√3 voxel/tick</small></div>
            <canvas class="ts-canvas" id="${PANEL_ID}-canB" width="${CAN_PX}" height="${CAN_PX}" role="img"
              aria-label="Transverse sector: a flux pulse expanding as a shell at the lattice light-speed, one voxel per √3 ticks."></canvas>
          </div>
        </div>
        <svg class="ts-chart" id="${PANEL_ID}-chart" viewBox="0 0 320 132" preserveAspectRatio="xMidYMid meet"></svg>
        <div class="ts-ctl">
          <button data-act="reset" title="Back to tick 0">◀ Reset</button>
          <button data-act="play" title="Capture (first time) then replay">▶ Play</button>
          <button data-act="step" title="Advance one tick">⏭ Step</button>
          <span class="ts-counter" id="${PANEL_ID}-counter">tick — / ${TICKS - 1}</span>
          <span class="ts-status" id="${PANEL_ID}-status" role="status" aria-live="polite">tap Play or Step to record</span>
        </div>
        <div class="ts-foot" id="${PANEL_ID}-foot"></div>`;
    return root;
}

function slopeOk(slope) { return Math.abs(slope - C_LATTICE) < SLOPE_TOL; }

// Build the radius-vs-tick chart as an SVG string (data series + 1/√3 reference +
// the Coulomb-fill trace + a cursor + the fitted-slope readout).
function renderChart(cache, cursor) {
    const W = 320, H = 132, m = { top: 22, right: 10, bottom: 18, left: 30 };
    const iW = W - m.left - m.right, iH = H - m.top - m.bottom;
    const ticks = cache ? cache.ticks : TICKS;
    const L = cache ? cache.L : 41;
    const maxR = L / 2;
    const X = (t) => m.left + (t / (ticks - 1)) * iW;
    const Y = (r) => m.top + iH - (Math.min(r, maxR) / maxR) * iH;

    let s = `<svg viewBox="0 0 ${W} ${H}" xmlns="http://www.w3.org/2000/svg" role="img" `
        + `aria-label="Wavefront radius versus tick. The transverse pulse front grows linearly at slope 1/√3; the longitudinal Coulomb extent fills almost immediately.">`;
    // axes
    s += `<line x1="${m.left}" y1="${m.top + iH}" x2="${W - m.right}" y2="${m.top + iH}" stroke="var(--border-light,#555)" stroke-width="1"/>`;
    s += `<line x1="${m.left}" y1="${m.top}" x2="${m.left}" y2="${m.top + iH}" stroke="var(--border-light,#555)" stroke-width="1"/>`;
    s += `<text x="2" y="${m.top + 6}" font-size="7.5" fill="var(--text-muted,#999)">r=${maxR.toFixed(0)}</text>`;
    s += `<text x="${m.left}" y="${H - 6}" font-size="7.5" fill="var(--text-muted,#999)">tick 0</text>`;
    s += `<text x="${W - m.right}" y="${H - 6}" text-anchor="end" font-size="7.5" fill="var(--text-muted,#999)">${ticks - 1}</text>`;

    // 1/√3 reference cone: r = c·t (dashed)
    const tEnd = Math.min(ticks - 1, maxR / C_LATTICE);
    s += `<line x1="${X(0).toFixed(1)}" y1="${Y(0).toFixed(1)}" x2="${X(tEnd).toFixed(1)}" y2="${Y(C_LATTICE * tEnd).toFixed(1)}" `
        + `stroke="#e8b04b" stroke-width="0.9" stroke-dasharray="3,2" opacity="0.85"/>`;
    // label the reference along its mid-line (keeps it clear of the top-right readout)
    const tMid = tEnd * 0.5;
    s += `<text x="${(X(tMid) + 4).toFixed(1)}" y="${(Y(C_LATTICE * tMid) - 2).toFixed(1)}" text-anchor="start" font-size="7.5" fill="#e8b04b">slope 1/√3 (=0.577)</text>`;

    if (cache) {
        // longitudinal Coulomb fill-extent (faint) — extent∈[0,1] scaled to the radius axis
        let pl = '';
        for (let t = 0; t < ticks; t++) pl += `${X(t).toFixed(1)},${Y(cache.longitudinal.extent[t] * maxR).toFixed(1)} `;
        s += `<polyline points="${pl.trim()}" fill="none" stroke="#9aa3b2" stroke-width="1" stroke-dasharray="1,2" opacity="0.7"/>`;
        s += `<text x="${(m.left + 3)}" y="${(m.top + 8)}" font-size="7.5" fill="#9aa3b2">Coulomb fill (constraint)</text>`;
        // transverse shell radius (solid) — the demo's protagonist
        let ps = '';
        for (let t = 0; t < ticks; t++) ps += `${X(t).toFixed(1)},${Y(cache.transverse.radius[t]).toFixed(1)} `;
        s += `<polyline points="${ps.trim()}" fill="none" stroke="#7fd0ff" stroke-width="1.6"/>`;
        s += `<text x="${(m.left + 3)}" y="${(m.top + 18)}" font-size="7.5" fill="#7fd0ff">transverse front (radiative)</text>`;
        // cursor
        const cx = X(cursor);
        s += `<line x1="${cx.toFixed(1)}" y1="${m.top}" x2="${cx.toFixed(1)}" y2="${m.top + iH}" stroke="#7CFC8C" stroke-width="0.8" opacity="0.7"/>`;
        // fitted-slope readout
        const fs = cache.fit.slope;
        s += `<text x="${W - m.right}" y="${m.top - 12}" text-anchor="end" font-size="9" fill="var(--text-secondary,#ccc)">`
            + `front slope ${fs.toFixed(3)} ≈ 1/√3 ${slopeOk(fs) ? '✓' : '✗'}</text>`;
    } else {
        s += `<text x="${W / 2}" y="${m.top + iH / 2}" text-anchor="middle" font-size="9" fill="var(--text-muted,#999)">press Play to record the two sectors</text>`;
    }
    s += `</svg>`;
    return s.replace('<svg ', `<svg class="ts-chart" id="${PANEL_ID}-chart" `);
}

export function mountTwoSectorsPanel(host) {
    if (!host) return null;
    ensureCss();
    document.getElementById(PANEL_ID)?.remove();
    const panel = buildPanel();
    host.appendChild(panel);
    const el = (id) => panel.querySelector(`#${PANEL_ID}-${id}`);
    const canA = el('canA'), canB = el('canB');

    const state = { cache: null, cursor: 0, playing: false, capturing: false, disposed: false };
    const setStatus = (t) => { const n = el('status'); if (n) n.textContent = t; };

    function drawChart() {
        const svg = el('chart');
        if (svg) svg.outerHTML = renderChart(state.cache, state.cursor);
    }

    function repaintFrame(t) {
        if (!state.cache) { drawChart(); return; }
        const { L, longitudinal, transverse, ticks } = state.cache;
        paintSliceToCanvas(canA, longitudinal.frames[t], L, { ramp: rampEmEnergy, norm: longitudinal.norm });
        paintSliceToCanvas(canB, transverse.frames[t], L, { ramp: rampViridis, norm: transverse.norm });
        const counter = el('counter');
        if (counter) counter.textContent = `tick ${t} / ${ticks - 1}`;
        drawChart();
    }

    async function onPlay() {
        if (state.capturing) return;
        if (state.cache) { state.playing = true; setStatus('replaying'); return; }
        state.capturing = true; state.playing = false;
        setStatus('recording… 0%');
        try {
            const cache = await captureTwoSectors(
                (p) => setStatus(`recording… ${Math.round(p * 100)}%`),
                () => state.disposed,
            );
            if (state.disposed) return;
            state.capturing = false;
            if (!cache) { setStatus('capture aborted'); return; }
            state.cache = cache;
            state.cursor = 0;
            repaintFrame(0);
            state.playing = true;
            const fs = cache.fit.slope;
            setStatus(`front slope ${fs.toFixed(3)} ≈ 1/√3 ${slopeOk(fs) ? '✓' : '✗'}`);
        } catch (e) {
            state.capturing = false;
            setStatus('capture engine unavailable — tap Play to retry');
        }
    }

    function onStep() {
        if (!state.cache) { if (!state.capturing) onPlay(); return; }
        state.playing = false;
        state.cursor = (state.cursor + 1) % state.cache.ticks;
        repaintFrame(state.cursor);
    }

    function onReset() {
        state.playing = false;
        state.cursor = 0;
        if (state.cache) repaintFrame(0);
    }

    panel.querySelector('.ts-ctl').addEventListener('click', (e) => {
        const act = e.target?.getAttribute?.('data-act');
        if (act === 'play') onPlay();
        else if (act === 'step') onStep();
        else if (act === 'reset') onReset();
    });

    el('foot').innerHTML = `The <b>longitudinal / Gauss-constraint</b> sector is a global solve (like the Coulomb
        gauge in EM): fixing ∇·J = ρ everywhere is a <i>constraint, not a signal</i> — it carries no information.
        The <b>transverse / radiative</b> sector is the physical signal and propagates at <b>c = 1/√3 voxel/tick</b>.
        No superluminal signalling; <b>Postulate 4</b> (local causality) holds. See FTD-0004 Phase G geometric
        Coulomb <b>[THEOREM]</b> and the cone-forced locality <b>[THEOREM]</b>.`;

    // initial empty chart
    drawChart();

    // Replay loop: advances the cursor only while playing; gated by panel visibility.
    // (The CAPTURE loop in two-sectors-capture.js is intentionally NOT gated — it must
    // finish even if the user tabs away — so the cache is never left half-built.)
    const sub = rafCoordinator.subscribe(`${PANEL_ID}-loop`, { hz: 16, cb: () => {
        if (!isPanelLive(host)) return;
        if (state.playing && state.cache) {
            state.cursor = (state.cursor + 1) % state.cache.ticks;
            repaintFrame(state.cursor);
        }
    } });

    const api = {
        element: panel,
        play: onPlay,
        step: onStep,
        reset: onReset,
        get cache() { return state.cache; },
        get cursor() { return state.cursor; },
        get isCapturing() { return state.capturing; },
        get fitSlope() { return state.cache?.fit?.slope ?? null; },
        dispose: () => {
            state.disposed = true;
            sub.unsubscribe();
            if (typeof window !== 'undefined' && window.__ftdTwoSectorsPanel === api) window.__ftdTwoSectorsPanel = null;
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdTwoSectorsPanel = api;
    return api;
}

export function initTwoSectorsPanel() {
    if (typeof document === 'undefined') return null;
    const host = document.getElementById('panel-two-sectors');
    if (!host) return null;
    return mountTwoSectorsPanel(host);
}
