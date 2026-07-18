// Scale Context — docked Scale-0 side panel (FTD-0306).
//
// Mounts into #panel-scale-context (registry id 'scale-context'). NOT a floating overlay.
// Answers "where does this lattice sit, and what can it reach vs CERN?":
//   · a log-scale length ruler with a live "you are here" bracket (1 → L voxels at
//     a_phys ≡ ℓ_P) and the LHC-reach marker far to the right (≈ 8.98×10¹⁴ voxels);
//   · length/time + CERN-gap readouts; UV cutoff + the manifestation threshold;
//   · live energy readouts (total manifested ≈ N·K_B; largest cluster ≈ N·K_B when
//     knot tracking is on) — labelled with the IDENT-NULL caveat (FTD-0262): never
//     a named SM particle;
//   · the no-linear-Lorentz-violation dispersion line (pointer to the Dispersion panel).
//
// Every number mirrors scripts/exploration/energy_scales_2026.py (the canonical
// hash-locked artifact) and is computed from constants.js. Honest tags throughout:
// scales are [CALIBRATION]-conditional; cluster→MeV is [SMC]; dispersion [MEASURED].

import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import { resolveActiveScale0BridgeFromWindow } from '../../state/store.js';
import {
    PLANCK_LENGTH_M, FTD_TICK_S, HBAR_C_MEV_M, M_PLANCK_GEV,
    K_B, K_GENESIS, M_E_PHYS, C_WAVE,
} from '../../../../constants.js';

const PANEL_ID = 'scale-context-panel';

// ── Calibration-derived scale anchors (mirror energy_scales_2026.py) ──────────
const E_LHC_GEV   = 13600.0;                              // 13.6 TeV (LHC Run 3)
const LHC_LEN_M   = HBAR_C_MEV_M / (E_LHC_GEV * 1000.0);  // ℏc/E ≈ 1.45×10⁻²⁰ m
const LHC_VOXELS  = LHC_LEN_M / PLANCK_LENGTH_M;          // ≈ 8.98×10¹⁴
const OMEGA_MAX   = 2 * C_WAVE;                           // 2/√3 zone-edge frequency
const PAIR_MEV    = 2 * M_E_PHYS;                         // QED pair-production threshold
const GRB_E_GEV   = 10.0;                                 // representative GRB photon

// Reference length markers for the ruler (metres → label). `anchor` keeps the
// extreme labels from clipping the viewBox edges (start at left, end at right).
const MARKERS = [
    { m: PLANCK_LENGTH_M, label: 'Planck / voxel', sub: '10⁻³⁵', anchor: 'start' },
    { m: 1e-15,           label: 'nuclear',        sub: '10⁻¹⁵', anchor: 'middle' },
    { m: 1e-10,           label: 'atom',           sub: '10⁻¹⁰', anchor: 'end' },
];

const LOG_MIN = -35, LOG_MAX = -9;   // ruler spans ℓ_P → atomic (log₁₀ metres)

function sci(x, d = 2) {
    if (!isFinite(x) || x === 0) return '0';
    const s = x.toExponential(d);                 // "1.62e-35"
    const [mant, exp] = s.split('e');
    if (Number(exp) === 0) return mant;           // order-1 value → mantissa only (no bare ×10⁰)
    const sup = String(Number(exp)).replace('-', '⁻').replace(/\d/g, (c) => '⁰¹²³⁴⁵⁶⁷⁸⁹'[c]);
    return `${mant}×10${sup}`;
}

function ensureCss() {
    if (typeof document === 'undefined' || document.getElementById(`${PANEL_ID}-css`)) return;
    const s = document.createElement('style');
    s.id = `${PANEL_ID}-css`;
    s.textContent = `
    #${PANEL_ID}{font-family:var(--font-sans,sans-serif);font-size:12px;color:var(--text-primary,#eee);padding:2px}
    #${PANEL_ID} .sc-title{font-weight:600;margin:2px 0 6px}
    #${PANEL_ID} .sc-title small{color:var(--text-muted,#888);font-weight:400}
    #${PANEL_ID} svg.sc-ruler{width:100%;display:block;background:#0c0c11;border-radius:6px}
    #${PANEL_ID} .sc-legend{display:flex;align-items:center;gap:5px;font-size:9.5px;color:var(--text-muted,#888);margin:3px 1px 0}
    #${PANEL_ID} .sc-legend i{width:7px;height:7px;border-radius:50%;background:#7CFC8C;display:inline-block;flex:0 0 auto}
    #${PANEL_ID} .sc-sec{display:flex;justify-content:space-between;align-items:baseline;font-size:10px;letter-spacing:.04em;text-transform:uppercase;color:var(--text-muted,#888);margin:9px 0 2px}
    #${PANEL_ID} .sc-rows{margin:2px 0}
    #${PANEL_ID} .sc-row{display:flex;justify-content:space-between;gap:10px;padding:2px 0;border-bottom:0.5px solid var(--border-light,rgba(255,255,255,0.06))}
    #${PANEL_ID} .sc-row span:last-child{font-variant-numeric:tabular-nums;color:var(--text-secondary,#ccc);text-align:right;white-space:nowrap}
    #${PANEL_ID} .sc-row span:first-child{color:var(--text-secondary,#ccc)}
    #${PANEL_ID} .sc-row.sc-live span:last-child{color:#7CFC8C}
    #${PANEL_ID} .sc-tag{color:var(--text-muted,#888);font-size:9.5px}
    #${PANEL_ID} .sc-foot{margin-top:9px;padding-top:7px;border-top:0.5px solid var(--border-light,rgba(255,255,255,0.12));font-size:10px;color:var(--text-muted,#888);line-height:1.5}
    #${PANEL_ID} .sc-foot b{color:var(--text-secondary,#aaa)}`;
    document.head.appendChild(s);
}

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.innerHTML = `
        <div class="sc-title">Scale Context <small>· Planck-scale instrument · FTD-0306</small></div>
        <svg class="sc-ruler" id="${PANEL_ID}-ruler" viewBox="0 0 360 150" preserveAspectRatio="xMidYMid meet"></svg>
        <div class="sc-legend"><i></i> live — tracks the running lattice (L, manifested voxels, clusters)</div>
        <div class="sc-sec"><span>Length &amp; time</span> <span class="sc-tag">[CALIBRATION]</span></div>
        <div class="sc-rows" id="${PANEL_ID}-len"></div>
        <div class="sc-sec"><span>Energy</span> <span class="sc-tag">[IMPOSED · SMC]</span></div>
        <div class="sc-rows" id="${PANEL_ID}-energy"></div>
        <div class="sc-foot" id="${PANEL_ID}-foot"></div>`;
    return root;
}

function renderRuler(svg, L) {
    const W = 360, H = 150, m = { top: 30, right: 18, bottom: 34, left: 28 };
    const iW = W - m.left - m.right;
    const axisY = H - m.bottom - 26;
    const X = (metres) => m.left + ((Math.log10(metres) - LOG_MIN) / (LOG_MAX - LOG_MIN)) * iW;
    const latM = Math.max(1, L) * PLANCK_LENGTH_M;        // lattice span (m)
    const xP = X(PLANCK_LENGTH_M);

    // N2 — screen-reader text alternative (the ruler is otherwise opaque).
    const aria = `Log-scale length ruler. This lattice spans 1 to ${L} voxels (${sci(latM, 2)} m) at the `
        + `Planck end; CERN / LHC resolves ${sci(LHC_LEN_M, 2)} m, about ${sci(LHC_VOXELS, 2)} voxels.`;
    let s = `<svg viewBox="0 0 ${W} ${H}" xmlns="http://www.w3.org/2000/svg" role="img" aria-label="${aria}">`;
    s += `<title>${aria}</title>`;

    // baseline axis
    s += `<line x1="${m.left}" y1="${axisY}" x2="${W - m.right}" y2="${axisY}" stroke="var(--border-light,#555)" stroke-width="1"/>`;

    // N1 — faint voxel-decade reference ticks (10³, 10⁶ voxels) the live bracket grows against.
    for (const dec of [3, 6]) {
        const x = X(Math.pow(10, dec) * PLANCK_LENGTH_M);
        s += `<line x1="${x.toFixed(1)}" y1="${axisY - 5}" x2="${x.toFixed(1)}" y2="${axisY + 4}" stroke="var(--accent,#4bb7e8)" stroke-width="0.6" opacity="0.3"/>`;
    }
    s += `<text x="${X(1e6 * PLANCK_LENGTH_M).toFixed(1)}" y="${axisY + 24}" text-anchor="middle" font-size="8.5" fill="var(--text-muted,#999)" opacity="0.6">10⁶ vox</text>`;

    // the live lattice bracket: 1 voxel → L voxels (drawn first so the Planck tick sits on top).
    const x1 = Math.max(X(latM), xP + 2);
    s += `<rect x="${xP.toFixed(1)}" y="${axisY - 5}" width="${(x1 - xP).toFixed(1)}" height="10" fill="var(--accent,#4bb7e8)" opacity="0.85" rx="1.5"/>`;
    const xc = (xP + x1) / 2;
    s += `<line x1="${xc.toFixed(1)}" y1="${axisY - 6}" x2="${xc.toFixed(1)}" y2="${m.top + 4}" stroke="var(--accent,#4bb7e8)" stroke-width="0.7" opacity="0.7"/>`;
    s += `<text x="${xP.toFixed(1)}" y="${m.top}" text-anchor="start" font-size="9" fill="var(--accent,#7fd0ff)">▼ your lattice (L=${L})</text>`;
    s += `<text x="${xP.toFixed(1)}" y="${m.top + 10}" text-anchor="start" font-size="8.5" fill="var(--text-muted,#9bd)">1 → ${L} voxels · ${sci(latM, 2)} m</text>`;

    // reference markers (ticks + labels) — on top of the bracket; anchored to avoid edge clipping.
    for (const mk of MARKERS) {
        const x = X(mk.m);
        const anchor = mk.anchor || 'middle';
        s += `<line x1="${x.toFixed(1)}" y1="${axisY - 4}" x2="${x.toFixed(1)}" y2="${axisY + 4}" stroke="var(--text-muted,#888)" stroke-width="0.8"/>`;
        s += `<text x="${x.toFixed(1)}" y="${axisY + 15}" text-anchor="${anchor}" font-size="8.5" fill="var(--text-muted,#999)">${mk.label}</text>`;
        s += `<text x="${x.toFixed(1)}" y="${axisY + 24}" text-anchor="${anchor}" font-size="8.5" fill="var(--text-muted,#999)">${mk.sub} m</text>`;
    }

    // LHC reach marker (CERN, far right of the lattice)
    const xLhc = X(LHC_LEN_M);
    s += `<line x1="${xLhc.toFixed(1)}" y1="${m.top - 6}" x2="${xLhc.toFixed(1)}" y2="${axisY + 4}" stroke="#e8b04b" stroke-width="0.9" stroke-dasharray="2,3" opacity="0.85"/>`;
    s += `<text x="${xLhc.toFixed(1)}" y="${m.top - 9}" text-anchor="middle" font-size="8.5" fill="#e8b04b">CERN / LHC ▲</text>`;
    s += `<text x="${xLhc.toFixed(1)}" y="${axisY - 8}" text-anchor="middle" font-size="8.5" fill="#e8b04b">≈ ${sci(LHC_VOXELS, 2)} voxels</text>`;

    s += `</svg>`;
    svg.outerHTML = s.replace('<svg ', `<svg class="sc-ruler" id="${PANEL_ID}-ruler" `);
}

function row(label, value, live = false, tip = '') {
    return `<div class="sc-row${live ? ' sc-live' : ''}"><span${tip ? ` title="${tip}"` : ''}>${label}</span><span>${value}</span></div>`;
}

export function mountScaleContextPanel(host, getBridge) {
    if (!host) return null;
    ensureCss();
    document.getElementById(PANEL_ID)?.remove();
    const panel = buildPanel();
    host.appendChild(panel);
    const el = (id) => panel.querySelector(`#${PANEL_ID}-${id}`);

    function readLive() {
        const b = getBridge?.();
        const L = b?.latticeSize ?? 33;
        const cap = b?.capabilities?.scale0 ?? null;
        let manifested = null, maxN = 0, knotLive = false;
        try {
            const diag = cap?.getScale0Diagnostics?.();
            if (diag && typeof diag.manifested === 'number') manifested = diag.manifested;
            const knot = cap?.getScale0KnotTelemetry?.();
            if (knot && knot.count && knot.size) {
                knotLive = true;
                for (let k = 0; k < knot.count; k++) if (knot.size[k] > maxN) maxN = knot.size[k];
            }
        } catch { /* mock/proxy not ready — degrade gracefully */ }
        return { L, manifested, maxN, knotLive };
    }

    function paint() {
        const { L, manifested, maxN, knotLive } = readLive();

        const svg = el('ruler');
        if (svg) renderRuler(svg, L);

        const latM = L * PLANCK_LENGTH_M;
        const gap = latM / LHC_LEN_M;     // lattice span ÷ one LHC resolution element
        el('len').innerHTML =
            row('1 voxel = ℓ_P', `${sci(PLANCK_LENGTH_M)} m`, false, 'a_phys ≡ ℓ_P [CALIBRATION] (FTD-0059 no-go).') +
            row('1 tick', `${sci(FTD_TICK_S)} s`, false, 't_phys = ℓ_P/(√3·c) = t_P/√3 [CALIBRATION].') +
            row(`this lattice (L=${L})`, `${sci(latM)} m · ${L}³ voxels`, true, 'Span = L·ℓ_P; updates with the lattice-size control.') +
            row('LHC resolves', `${sci(LHC_LEN_M)} m`, false, 'ℏc / 13.6 TeV — one resolution element.') +
            row('… in voxels', `${sci(LHC_VOXELS)}`, false, 'A CERN-probed structure spans ~10¹⁵ voxels — infeasible to simulate.') +
            row('lattice vs that', `${sci(gap)}× shorter`, true, 'The largest practical lattice is far shorter than one LHC resolution element.');

        const manifMev = manifested != null ? manifested * K_B : null;
        const clusterMev = maxN > 0 ? maxN * K_B : null;
        const dvc = Math.pow(GRB_E_GEV / M_PLANCK_GEV, 2) / 8;
        el('energy').innerHTML =
            row('UV cutoff ω_max', `${OMEGA_MAX.toFixed(3)} rad/tick`, false, 'Zone edge 2/√3; physically ≈ Planck-scale (E_P ≈ 1.22×10¹⁹ GeV).') +
            row('manifestation K_GENESIS', `${K_GENESIS.toFixed(3)} MeV`, false, '= 3·K_MANIFEST = 3·W_SC [SELECTION — ADOPTED, FTD-0388]; vs QED pair threshold below.') +
            row('QED pair threshold 2mₑ', `${PAIR_MEV.toFixed(3)} MeV`, false, '2·mₑ — the factor 3-vs-2 is flagged, not claimed.') +
            row('manifested energy', manifMev != null ? `≈ ${manifMev.toFixed(1)} MeV` : '— (start the sim)', true, 'Σ manifested voxels × K_B — engine-level aggregate; mass = N·K_B is [IMPOSED] (FTD-0250). Not a particle identification.') +
            row('largest cluster',
                clusterMev != null ? `≈ ${maxN} vox · ${clusterMev.toFixed(1)} MeV`
                                   : (knotLive ? 'none yet' : '— (enable Knots tracking)'),
                true, 'N·K_B [SMC] — IDENT-NULL (FTD-0262): an energy, NOT a named SM particle.') +
            row('Δv/c at 10 GeV (LV)', `${sci(dvc, 1)}`, false, '(E/E_P)²/8 — no linear term [MEASURED structure, FTD-0299]; continued lab nulls are the [PREDICTION] (FP-3).');

        el('foot').innerHTML = `<b>You are at the Planck substrate.</b> CERN sits ~10¹⁵ voxels "up" — so the
        engine's genuine content is dimensionless structure, UV-suppressed deviations and structural nulls,
        <i>not</i> collision dynamics. Absolute scales are <b>[CALIBRATION]</b>-conditional; cluster→MeV is
        <b>[SMC]</b> / IDENT-NULL. See <b>SPEC_ENERGY_SCALES_AND_DETECTABILITY</b> (FTD-0306).`;
    }

    paint();

    const sub = rafCoordinator.subscribe(`${PANEL_ID}-loop`, { hz: 2, cb: () => {
        if (!isPanelLive(host)) return;
        paint();
    } });

    const api = {
        update: paint,
        element: panel,
        dispose: () => {
            sub.unsubscribe();
            if (typeof window !== 'undefined' && window.__ftdScaleContextPanel === api) window.__ftdScaleContextPanel = null;
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdScaleContextPanel = api;
    return api;
}

export function initScaleContextPanel() {
    if (typeof document === 'undefined') return null;
    const host = document.getElementById('panel-scale-context');
    if (!host) return null;
    return mountScaleContextPanel(host, () => resolveActiveScale0BridgeFromWindow());
}
