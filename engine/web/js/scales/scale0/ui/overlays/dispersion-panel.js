// Dispersion — docked Scale-0 side panel (FTD-0298 / FTD-0299).
//
// Mounts into #panel-dispersion (registry id 'dispersion'). NOT a floating overlay.
// Charts the lattice flux-wave dispersion ω(k) = 2c·|sin(k/2)|, c = C_SPEED = 1/√3:
//   · the analytic curve + group velocity v_g = c·cos(k/2);
//   · the engine-MEASURED dispersion atlas from the FTD-0299 campaign across
//     ⟨100⟩/⟨110⟩/⟨111⟩ (ω_eig == the 18-pt stencil eigenvalue to machine zero;
//     IR phase speed isotropic at 1/√3 — LIGHT-CONFIRMED);
//   · calibrated markers showing that ALL observable light & radio live at the far
//     IR (k/k_zone ≲ 1e-28), one flux-wave sector; the zone edge (k=π) is the UV
//     cutoff at ≈ the Planck frequency where v_g → 0;
//   · the NO-ACOUSTIC-BRANCH contrast — FTD has light but no sound: the lattice IS
//     space, so there is no broken-translation Goldstone (FTD-0298 §5; FTD-0299
//     condensate-compression probe = NULL).
// "Measure live" button is present for UI parity; live re-measurement is not available
// on the WASM engine (WasmBridgeProxy self-ticks; synchronous tick-loop is incompatible).

import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import { resolveActiveScale0BridgeFromWindow } from '../../state/store.js';
import {
    C_SPEED, C_MS, FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M,
} from '../../../../constants.js';

const PANEL_ID = 'dispersion-panel';
const C = C_SPEED;                 // 1/√3
const OMEGA_MAX = 2 * C;           // 2/√3 ≈ 1.1547 rad/tick (per-axis zone edge)
const K_MAX = Math.PI;             // Brillouin zone edge (rad/voxel)

// Engine-measured atlas — FTD-0299 canonical run, L=32 (kmag, ω_eig). ω_eig equals
// the 18-pt stencil eigenvalue to machine precision (LIGHT-CONFIRMED).
const ATLAS = Object.freeze({
    '100': [[0.1963, 0.1132], [0.3927, 0.2253], [0.5890, 0.3352], [0.7854, 0.4419],
            [0.9817, 0.5443], [1.1781, 0.6415], [1.3744, 0.7325], [1.5708, 0.8165]],
    '110': [[0.2777, 0.1598], [0.5554, 0.3166], [0.8330, 0.4673], [1.1107, 0.6095],
            [1.3884, 0.7407], [1.6661, 0.8593], [1.9438, 0.9640], [2.2214, 1.0541]],
    '111': [[0.3401, 0.1954], [0.6802, 0.3852], [1.0203, 0.5640], [1.3603, 0.7270],
            [1.7004, 0.8702], [2.0405, 0.9902], [2.3806, 1.0853], [2.7207, 1.1547]],
});
const DIR_COLOR = { '100': 'var(--accent,#e8b04b)', '110': '#4bb7e8', '111': '#e87a4b' };

// Default electron-primary mapping (FTD-0137 §4.5): one voxel maps to the
// conditional FTD Planck length, so the zone edge is a 2-voxel wavelength.
// Every lab frequency still sits at k/k_zone ≲ 1e-28. The CODATA Planck
// length remains a reference and is intentionally not substituted here.
const L_P = FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M, C_PHYS = C_MS;
function kOverKzone(fHz) { return (2 * L_P) / (C_PHYS / fHz); }   // dimensionless

function ensureCss() {
    if (typeof document === 'undefined' || document.getElementById('dispersion-panel-css')) return;
    const s = document.createElement('style');
    s.id = 'dispersion-panel-css';
    s.textContent = `
    #${PANEL_ID}{font-family:var(--font-sans,sans-serif);font-size:16px;color:var(--text-primary,#eee);padding:2px}
    #${PANEL_ID} .dp-title{font-weight:600;margin:2px 0 6px}
    #${PANEL_ID} .dp-title small{color:var(--text-muted,#888);font-weight:400}
    #${PANEL_ID} svg.dp-plot{width:100%;display:block;background:#0c0c11;border-radius:6px}
    #${PANEL_ID} .dp-legend{display:flex;flex-wrap:wrap;gap:10px;margin:6px 0;font-size:16px;color:var(--text-secondary,#ccc)}
    #${PANEL_ID} .dp-legend i{display:inline-block;width:9px;height:9px;border-radius:50%;margin-right:4px;vertical-align:middle}
    #${PANEL_ID} .dp-rows{margin:4px 0}
    #${PANEL_ID} .dp-row{display:flex;justify-content:space-between;padding:2px 0;border-bottom:0.5px solid var(--border-light,rgba(255,255,255,0.06))}
    #${PANEL_ID} .dp-row span:last-child{font-variant-numeric:tabular-nums;color:var(--text-secondary,#ccc)}
    #${PANEL_ID} .dp-actions{display:flex;gap:6px;align-items:center;margin:6px 0 2px}
    #${PANEL_ID} .dp-actions button{padding:5px 9px;border-radius:6px;cursor:pointer;border:0.5px solid var(--border-light,rgba(255,255,255,0.18));background:var(--surface-2,rgba(255,255,255,0.06));color:inherit;font-size:16px}
    #${PANEL_ID} .dp-status{font-size:16px;color:var(--text-muted,#888)}
    #${PANEL_ID} .dp-foot{margin-top:8px;padding-top:7px;border-top:0.5px solid var(--border-light,rgba(255,255,255,0.12));font-size:16px;color:var(--text-muted,#888);line-height:1.45}
    #${PANEL_ID} .dp-foot b{color:var(--text-secondary,#aaa)}`;
    document.head.appendChild(s);
}

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.dataset.applicability = 'reference-atlas';
    root.innerHTML = `
        <div class="dp-title">Dispersion ω(k) <small>· light = radio · FTD-0298/0299</small></div>
        <svg class="dp-plot" id="${PANEL_ID}-plot" viewBox="0 0 360 220" preserveAspectRatio="xMidYMid meet"></svg>
        <div class="dp-legend">
            <span><i style="background:var(--text-muted,#888)"></i>analytic 2c·sin(k/2)</span>
            <span><i style="background:${DIR_COLOR['100']}"></i>⟨100⟩</span>
            <span><i style="background:${DIR_COLOR['110']}"></i>⟨110⟩</span>
            <span><i style="background:${DIR_COLOR['111']}"></i>⟨111⟩</span>
            <span id="${PANEL_ID}-livelegend" hidden><i style="background:#7CFC8C"></i>live</span>
        </div>
        <div class="dp-rows" id="${PANEL_ID}-rows"></div>
        <div class="dp-actions">
            <button id="${PANEL_ID}-measure" type="button">Measure live ▸</button>
            <span class="dp-status" id="${PANEL_ID}-status">engine-measured: FTD-0299 atlas</span>
        </div>
        <div class="dp-foot"><b>Light & radio are one flux-wave sector</b> — only k differs; every
        observable frequency sits at the far IR (k/k_zone ≲ 1e-28), so FTD predicts no vacuum
        dispersion. The zone edge (k=π, 2-voxel wavelength ≈ Planck scale) is the UV cutoff
        where v_g→0. <b>No acoustic branch:</b> the lattice <i>is</i> space ⇒ no broken-translation
        Goldstone ⇒ no sound (FTD-0298 §5; FTD-0299 condensate probe = NULL).</div>`;
    return root;
}

function renderPlot(svg, live) {
    const W = 360, H = 220, m = { top: 14, right: 12, bottom: 30, left: 40 };
    const iW = W - m.left - m.right, iH = H - m.top - m.bottom;
    const X = (k) => m.left + (k / K_MAX) * iW;
    const Y = (w) => m.top + (1 - w / OMEGA_MAX) * iH;
    let s = `<svg viewBox="0 0 ${W} ${H}" xmlns="http://www.w3.org/2000/svg">`;
    s += `<rect x="${m.left}" y="${m.top}" width="${iW}" height="${iH}" fill="rgba(255,255,255,0.02)" stroke="var(--border-light,#444)" stroke-width="0.5"/>`;

    // zone edge + IR region markers
    s += `<line x1="${X(K_MAX)}" y1="${m.top}" x2="${X(K_MAX)}" y2="${m.top + iH}" stroke="var(--warning,#e8b04b)" stroke-width="0.8" stroke-dasharray="2,3" opacity="0.7"/>`;
    s += `<text x="${X(K_MAX) - 2}" y="${m.top + 9}" text-anchor="end" font-size="16" fill="var(--warning,#e8b04b)">zone edge (v_g→0)</text>`;
    s += `<text x="${X(0) + 3}" y="${m.top + iH - 4}" font-size="16" fill="#7aa7ff">◄ radio · visible (k/k_zone≲1e-28)</text>`;

    // no-acoustic-branch contrast (greyed line where ω_s=c·k would sit, struck out)
    s += `<line x1="${X(0)}" y1="${Y(0)}" x2="${X(0.55)}" y2="${Y(C * 0.55)}" stroke="#666" stroke-width="1" stroke-dasharray="3,3" opacity="0.6"/>`;
    s += `<text x="${X(0.58)}" y="${Y(C * 0.5)}" font-size="16" fill="#888">no acoustic branch ✗</text>`;

    // analytic curve ω = 2c|sin(k/2)|
    let path = '';
    for (let i = 0; i <= 80; i++) { const k = (i / 80) * K_MAX; const w = 2 * C * Math.abs(Math.sin(k / 2)); path += `${i ? 'L' : 'M'}${X(k).toFixed(1)},${Y(w).toFixed(1)} `; }
    s += `<path d="${path}" fill="none" stroke="var(--text-muted,#888)" stroke-width="1.4"/>`;
    // group velocity v_g = c·cos(k/2) (scaled to ω axis for shape) — dashed
    let vg = '';
    for (let i = 0; i <= 80; i++) { const k = (i / 80) * K_MAX; const v = C * Math.cos(k / 2); vg += `${i ? 'L' : 'M'}${X(k).toFixed(1)},${Y(v).toFixed(1)} `; }
    s += `<path d="${vg}" fill="none" stroke="#5a6" stroke-width="0.8" stroke-dasharray="4,3" opacity="0.6"/>`;
    s += `<text x="${X(0.15)}" y="${Y(C) - 2}" font-size="16" fill="#5a6">v_g</text>`;

    // measured atlas points
    for (const dir of Object.keys(ATLAS)) {
        for (const [k, w] of ATLAS[dir]) s += `<circle cx="${X(k).toFixed(1)}" cy="${Y(w).toFixed(1)}" r="2.1" fill="${DIR_COLOR[dir]}"/>`;
    }
    // live points (green)
    if (live && live.length) for (const [k, w] of live) s += `<circle cx="${X(k).toFixed(1)}" cy="${Y(w).toFixed(1)}" r="2.4" fill="none" stroke="#7CFC8C" stroke-width="1.4"/>`;

    // axes labels
    s += `<text x="${m.left + iW / 2}" y="${H - 4}" text-anchor="middle" font-size="16" fill="var(--text-muted,#888)">k (rad/voxel) · 0 → π</text>`;
    s += `<text x="10" y="${m.top + iH / 2}" transform="rotate(-90 10 ${m.top + iH / 2})" text-anchor="middle" font-size="16" fill="var(--text-muted,#888)">ω (rad/tick)</text>`;
    s += `<text x="${m.left - 3}" y="${Y(OMEGA_MAX) + 3}" text-anchor="end" font-size="16" fill="var(--text-muted,#888)">${OMEGA_MAX.toFixed(2)}</text>`;
    s += `</svg>`;
    svg.outerHTML = s.replace('<svg ', `<svg class="dp-plot" id="${PANEL_ID}-plot" `);
}

function rowHTML(label, value, tip = '') {
    return `<div class="dp-row"><span${tip ? ` title="${tip}"` : ''}>${label}</span><span>${value}</span></div>`;
}

export function mountDispersionPanel(host, getBridge) {
    if (!host) return null;
    ensureCss();
    document.getElementById(PANEL_ID)?.remove();
    const panel = buildPanel();
    host.appendChild(panel);
    const el = (id) => panel.querySelector(`#${PANEL_ID}-${id}`);

    let livePts = null;

    function paint() {
        const svg = el('plot');
        if (svg) renderPlot(svg, livePts);
        el('rows').innerHTML =
            rowHTML('c (IR phase speed)', `${C.toFixed(5)} = 1/√3`, 'Selected lattice speed [SELECTION, FTD-0407]. The production 18-point stencil has an actual stability ceiling of c ≤ √3/2 ≈ 0.866, so 1/√3 is conservative, not forced. SI value is calibration-dependent.') +
            rowHTML('ω_max (zone edge)', `${OMEGA_MAX.toFixed(4)} = 2/√3`, 'Per-axis Nyquist; physically ≈ the Planck frequency (~2×10⁴² Hz).') +
            rowHTML('visible k/k_zone', kOverKzone(5e14).toExponential(1), 'A 5×10¹⁴ Hz wave on a voxel≡ℓ_P lattice — deep IR.') +
            rowHTML('FM radio k/k_zone', kOverKzone(1e8).toExponential(1), 'A 10⁸ Hz wave — even deeper IR; co-propagates with light at c.') +
            rowHTML('atlas', 'LIGHT-CONFIRMED', 'Engine ω_eig matches the 18-pt stencil to machine zero; c_eff isotropic at 1/√3.');
    }

    function measureLive() {
        el('status').textContent = 'Live remeasurement is not implemented on this dashboard path — the displayed points are the canonical FTD-0299 engine atlas.';
    }

    el('measure').addEventListener('click', measureLive);
    paint();

    // light repaint cadence (mostly static; keeps it responsive to theme/resize)
    const armSub = rafCoordinator.subscribe(`${PANEL_ID}-arm`, { hz: 1, cb: () => {
        if (!isPanelLive(host)) return;
        armSub.unsubscribe();
        paint();
    } });

    const api = {
        update: paint,
        element: panel,
        measureLive,
        get applicability() { return panel.dataset.applicability; },
        get armCoordinatorActive() { return rafCoordinator._subs.has(`${PANEL_ID}-arm`); },
        dispose: () => {
            armSub.unsubscribe();
            if (typeof window !== 'undefined' && window.__ftdDispersionPanel === api) window.__ftdDispersionPanel = null;
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdDispersionPanel = api;
    return api;
}

export function initDispersionPanel() {
    if (typeof document === 'undefined') return null;
    if (typeof window !== 'undefined' && window.__ftdDispersionPanel) return window.__ftdDispersionPanel;
    const host = document.getElementById('panel-dispersion');
    if (!host) return null;
    const getBridge = () => resolveActiveScale0BridgeFromWindow();
    return mountDispersionPanel(host, getBridge);
}
