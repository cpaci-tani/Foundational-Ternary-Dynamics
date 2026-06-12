// Thermodynamics — docked Scale-0 side panel (FTD-0274).
//
// Mounts into #panel-thermo (registry id 'thermo'). NOT a floating overlay.
// Surfaces the lattice's thermodynamic state: a temperature control (the Langevin
// bath langevin_T) plus live telemetries — kinetic temperature T_kin, condensate
// fraction m, phase, the energy ledger — and a flux |J| HEAT MAP slice. The
// temperature slider drives langevin_T across the first-order condensation point
// T_up~0.05 so the user can ignite the lattice and watch m and the heat map.
//
// FINDINGS (load-bearing, [MEASURED — BOUNDARY], FTD-0274): the lattice condenses
// void→matter in a first-order jump at T_up~0.05, but has NO maximum temperature
// (manifestation is a safety valve absorbing arbitrary heat; T_kin tested to 27×c²
// with no blow-up). Tighter (smaller) lattices ignite at lower T. These are engine
// MEASUREMENTS, not derivations. The footer states this.

import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import { getScale0State } from '../../state/store.js';
import { paintSliceToCanvas } from './slice-render.js';
import { rampEmEnergy } from '../../../../viewport/color-ramps.js';

const PANEL_ID = 'thermo-panel';
const HZ = 4;
const T_UP = 0.05;        // measured first-order condensation point (lattice units)
const C2 = 1.0 / 3.0;     // lattice c² (C_WAVE = 1/√3)
const SPARK_MAX = 80;

function ensureCss() {
    if (typeof document === 'undefined' || document.getElementById('thermo-panel-css')) return;
    const s = document.createElement('style');
    s.id = 'thermo-panel-css';
    s.textContent = `
    #${PANEL_ID}{font-family:var(--font-sans,sans-serif);font-size:12px;color:var(--text-primary,#eee);padding:2px}
    #${PANEL_ID} .tp-title{font-weight:600;letter-spacing:0.2px;margin:2px 0 8px}
    #${PANEL_ID} .tp-title small{color:var(--text-muted,#888);font-weight:400}
    #${PANEL_ID} .tp-ctl{display:flex;align-items:center;gap:8px;margin-bottom:4px}
    #${PANEL_ID} .tp-ctl input[type=range]{flex:1}
    #${PANEL_ID} .tp-ctl .tp-tval{width:46px;text-align:right;font-variant-numeric:tabular-nums}
    #${PANEL_ID} .tp-scale{display:flex;justify-content:space-between;font-size:9.5px;color:var(--text-muted,#888);margin-bottom:7px}
    #${PANEL_ID} .tp-tup{color:var(--accent,#e8b04b)}
    #${PANEL_ID} .tp-presets{display:flex;gap:6px;margin-bottom:9px}
    #${PANEL_ID} .tp-presets button{flex:1;padding:5px;border-radius:6px;cursor:pointer;border:0.5px solid var(--border-light,rgba(255,255,255,0.18));background:var(--surface-2,rgba(255,255,255,0.06));color:inherit;font-size:11px}
    #${PANEL_ID} .tp-phase{display:flex;align-items:center;gap:8px;margin-bottom:8px}
    #${PANEL_ID} .tp-phase .tp-plabel{font-weight:600;min-width:80px}
    #${PANEL_ID} .tp-bar{flex:1;height:11px;border-radius:6px;background:#0c0c11;overflow:hidden;border:0.5px solid var(--border-light,rgba(255,255,255,0.12))}
    #${PANEL_ID} .tp-bar>div{height:100%;width:0%;background:var(--accent,#e8b04b)}
    #${PANEL_ID} .tp-mpct{width:38px;text-align:right;font-variant-numeric:tabular-nums}
    #${PANEL_ID} .tp-rows{margin:2px 0 8px}
    #${PANEL_ID} .tp-row{display:flex;justify-content:space-between;padding:2px 0;border-bottom:0.5px solid var(--border-light,rgba(255,255,255,0.05))}
    #${PANEL_ID} .tp-row span:last-child{font-variant-numeric:tabular-nums;color:var(--text-secondary,#ccc)}
    #${PANEL_ID} .tp-heatwrap{margin:4px 0}
    #${PANEL_ID} .tp-heatlabel{font-size:10px;color:var(--text-muted,#888);margin-bottom:3px;display:flex;justify-content:space-between}
    #${PANEL_ID} canvas.tp-heat{width:100%;display:block;border-radius:6px;background:#0c0c11;image-rendering:pixelated;aspect-ratio:1/1}
    #${PANEL_ID} .tp-spark{width:100%;height:34px;display:block;margin-top:4px}
    #${PANEL_ID} .tp-foot{margin-top:9px;padding-top:8px;border-top:0.5px solid var(--border-light,rgba(255,255,255,0.12));font-size:10px;color:var(--text-muted,#888);line-height:1.45}
    #${PANEL_ID} .tp-foot b{color:var(--text-secondary,#aaa)}`;
    document.head.appendChild(s);
}

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.innerHTML = `
        <div class="tp-title">Thermodynamics <small>· FTD-0274</small></div>
        <div class="tp-ctl">
            <span style="opacity:0.8">T</span>
            <input id="${PANEL_ID}-slider" type="range" min="0" max="0.14" step="0.0025" value="0.03">
            <span id="${PANEL_ID}-tval" class="tp-tval">0.030</span>
        </div>
        <div class="tp-scale"><span>0 (abs. zero)</span><span class="tp-tup">↑ T_up≈0.05</span><span>hot</span></div>
        <div class="tp-presets">
            <button data-t="0.02">Cold</button>
            <button data-t="0.07">Ignite</button>
            <button data-t="0.20">Hot</button>
        </div>
        <div class="tp-phase">
            <span id="${PANEL_ID}-phase" class="tp-plabel">VACUUM</span>
            <div class="tp-bar"><div id="${PANEL_ID}-bar"></div></div>
            <span id="${PANEL_ID}-mpct" class="tp-mpct">0%</span>
        </div>
        <div class="tp-rows" id="${PANEL_ID}-rows"></div>
        <div class="tp-heatwrap">
            <div class="tp-heatlabel"><span>flux |J| heat map (z-slice)</span><span id="${PANEL_ID}-hmax"></span></div>
            <canvas id="${PANEL_ID}-heat" class="tp-heat" width="64" height="64"></canvas>
        </div>
        <svg id="${PANEL_ID}-spark" class="tp-spark" viewBox="0 0 240 34" preserveAspectRatio="none"></svg>
        <div class="tp-foot"><b>[MEASURED — BOUNDARY]</b> the void condenses to matter in a
        <b>first-order</b> jump at T<sub>up</sub>≈0.05; there is <b>no maximum temperature</b>
        (manifestation is a safety valve) — you cannot explode the lattice by overheating,
        the explosion <i>is</i> the condensation.</div>`;
    return root;
}

function rowHTML(label, value, tip = '') {
    const t = tip ? ` title="${tip}"` : '';
    return `<div class="tp-row"><span${t}>${label}</span><span>${value}</span></div>`;
}

function sparkPath(values, w = 240, h = 34) {
    const n = values.length;
    if (n < 2) return '';
    let mn = Infinity, mx = -Infinity;
    for (const v of values) { if (v < mn) mn = v; if (v > mx) mx = v; }
    const span = (mx - mn) || 1;
    let d = '';
    for (let i = 0; i < n; i++) {
        const x = (i / (n - 1)) * w;
        const y = h - ((values[i] - mn) / span) * (h - 3) - 1.5;
        d += `${i ? 'L' : 'M'}${x.toFixed(1)},${y.toFixed(1)} `;
    }
    return d;
}

export function mountThermoPanel(host, getBridge) {
    if (!host) return null;
    ensureCss();
    document.getElementById(PANEL_ID)?.remove();
    const panel = buildPanel();
    host.appendChild(panel);
    const el = (id) => panel.querySelector(`#${PANEL_ID}-${id}`);

    const slider = el('slider'), tvalEl = el('tval');
    const phaseEl = el('phase'), barEl = el('bar'), mpctEl = el('mpct');
    const rowsEl = el('rows'), heat = el('heat'), hmaxEl = el('hmax'), sparkEl = el('spark');
    const mHist = [];
    let bridgeId = null;

    function setTemp(T) {
        const b = getBridge?.();
        try { if (b && typeof b.setLangevinTemp === 'function') b.setLangevinTemp(T); } catch (e) { /* noop */ }
    }
    slider.addEventListener('input', () => {
        const T = parseFloat(slider.value);
        tvalEl.textContent = T.toFixed(3);
        setTemp(T);
    });
    panel.querySelectorAll('.tp-presets button').forEach((btn) => {
        btn.addEventListener('click', () => {
            const T = parseFloat(btn.dataset.t);
            slider.value = String(T); tvalEl.textContent = T.toFixed(3); setTemp(T);
        });
    });

    function update() {
        const b = getBridge?.();
        if (!b) return;
        if (b !== bridgeId) { bridgeId = b; mHist.length = 0; }
        if (!isPanelLive(host)) return;

        const diag = (typeof b.getDiagnostics === 'function') ? b.getDiagnostics() : {};
        const audit = (typeof b.getEnergyAudit === 'function') ? b.getEnergyAudit() : null;
        const L = b.latticeSize || 33;
        const Nvox = L * L * L;
        const N = diag.manifested || 0;
        const m = Math.min(1, N / Nvox);
        const waveE = audit ? (audit.waveEnergy ?? audit.totalWaveEnergy ?? 0)
                            : (diag.totalWaveEnergy ?? 0);
        const fieldE = audit ? (audit.fieldEnergy ?? 0) : 0;
        const totalE = audit ? (audit.totalEnergy ?? 0) : (diag.totalEnergy ?? 0);
        const tKin = waveE / (1.5 * Nvox);
        // Actual bath temperature from the engine (falls back to the slider).
        const Tset = (typeof b.getLangevinTemp === 'function') ? b.getLangevinTemp()
                                                               : parseFloat(slider.value);

        // phase + bar
        let label = 'VACUUM', color = 'var(--text-secondary,#aaa)';
        if (m > 0.9) { label = 'CONDENSED'; color = 'var(--accent,#e8b04b)'; }
        else if (m > 0.05) { label = 'IGNITING'; color = '#e87a4b'; }
        phaseEl.textContent = label; phaseEl.style.color = color;
        barEl.style.width = (m * 100).toFixed(1) + '%';
        mpctEl.textContent = (m * 100).toFixed(0) + '%';

        // telemetry rows
        rowsEl.innerHTML =
            rowHTML('T (bath)', `${Tset.toFixed(3)}  (${(Tset / C2).toFixed(2)} c²)`, 'Langevin bath temperature langevin_T (lattice units; c²=1/3).') +
            rowHTML('T_kin', tKin.toFixed(4), 'Kinetic temperature ⟨½|wave_vel|²⟩/(3/2) (equipartition, k_B≡1).') +
            rowHTML('m (condensate)', m.toFixed(4), 'Manifestation fraction N/L³ — the condensate order parameter.') +
            rowHTML('N voxels', `${N} / ${Nvox}`, 'Manifested voxels (the condensate "particles") out of L³.') +
            rowHTML('E field ½Σ|J|²', fieldE.toFixed(3), 'Flux field energy.') +
            rowHTML('E wave ½Σ|ẇ|²', waveE.toFixed(3), 'Wave (kinetic) energy — sources T_kin.') +
            rowHTML('E total', totalE.toFixed(3), 'field + wave + particle KE.');

        // flux |J| heat map (z mid-slice)
        try {
            const mid = (L / 2) | 0;
            const s = (typeof b.getFluxSlice === 'function') ? b.getFluxSlice(2, mid) : null;
            if (s && s.length >= L * L) {
                let vmax = 1e-9;
                for (let i = 0; i < s.length; i++) if (s[i] > vmax) vmax = s[i];
                if (heat.width !== L) { heat.width = L; heat.height = L; }
                paintSliceToCanvas(heat, s, L, { ramp: rampEmEnergy, norm: vmax });
                hmaxEl.textContent = `|J|max ${vmax.toFixed(2)}`;
            }
        } catch (e) { /* slice unavailable on this bridge */ }

        // m sparkline
        mHist.push(m); if (mHist.length > SPARK_MAX) mHist.shift();
        const d = sparkPath(mHist);
        sparkEl.innerHTML = d
            ? `<path d="${d}" fill="none" stroke="var(--accent,#e8b04b)" stroke-width="1.4"/>`
            : '';
    }

    const armSub = rafCoordinator.subscribe(`${PANEL_ID}-arm`, { hz: 2, cb: () => {
        if (!isPanelLive(host)) return;
        armSub.unsubscribe();
        update();
        liveSub = rafCoordinator.subscribe(PANEL_ID, { hz: HZ, cb: update });
    } });
    let liveSub = null;

    const api = {
        update,
        element: panel,
        setTemp: (T) => { slider.value = String(T); tvalEl.textContent = (+T).toFixed(3); setTemp(+T); },
        dispose: () => {
            armSub.unsubscribe();
            liveSub?.unsubscribe();
            if (typeof window !== 'undefined' && window.__ftdThermoPanel === api) window.__ftdThermoPanel = null;
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdThermoPanel = api;
    return api;
}

export function initThermoPanel() {
    if (typeof document === 'undefined') return null;
    const host = document.getElementById('panel-thermo');
    if (!host) return null;
    const getBridge = () => {
        const state = getScale0State?.();
        if (state?.useFluxMock && state?.fluxMock) return state.fluxMock;
        const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        return ctx?.bridge || null;
    };
    return mountThermoPanel(host, getBridge);
}
