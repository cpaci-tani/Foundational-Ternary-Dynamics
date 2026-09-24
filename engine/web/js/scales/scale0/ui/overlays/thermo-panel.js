// Thermodynamics — docked Scale-0 side panel (FTD-0274).
//
// Mounts into #panel-thermo (registry id 'thermo'). NOT a floating overlay.
// Surfaces the lattice's thermodynamic state: a temperature control (the Langevin
// bath langevin_T) plus live telemetries — kinetic temperature T_kin, condensate
// fraction m, phase, the energy ledger — and a flux |J| HEAT MAP slice. The
// temperature slider drives langevin_T across the first-order condensation point
// T_up~0.05 so the user can ignite the lattice and watch m and the heat map.
//
// Historical FTD-0274 measurements concern a bounded Langevin reference
// experiment. A finite heating run does not establish unlimited stability,
// absence of a maximum temperature, or recovery of physical matter.

import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import {
    commitScale0ScientificMutation,
    resolveActiveScale0BridgeFromWindow,
    SCALE0_MUTATION_REASONS,
    SCALE0_MUTATION_SOURCES,
} from '../../state/store.js';
import { paintSliceToCanvas, transposeAndFlipNN } from './slice-render.js';
import { rampEmEnergy } from '../../../../viewport/color-ramps.js';
import { C_SPEED } from '../../../../constants.js';
import {
    isCurrentScale0TelemetryMeta,
    readScale0TotalEnergy,
    readScale0WaveEnergy,
    readScale0FieldEnergy,
} from '../../../../telemetry/scale0-read.js';
import { telemetryHub } from '../../../../telemetry-hub.js';
import { TickHistoryControl } from '../../../../ui/charts/history-window.js';
import { projectHistoryIndices } from '../../../../ui/charts/history-index.js';
import { setTextIfChanged } from '../../../../ui/utils/dom-text.js';

const PANEL_ID = 'thermo-panel';
const HZ = 4;
const T_UP = 0.05;        // measured first-order condensation point (lattice units)
const C2 = C_SPEED * C_SPEED;
const SPARK_MAX = 80;

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.innerHTML = `
        <div class="tp-title">Thermodynamics <small>· FTD-0274</small></div>
        <div class="tp-ctl">
            <span class="tp-temperature-symbol">T</span>
            <input id="${PANEL_ID}-slider" type="range" min="0" max="0.20" step="0.0025" value="0.03">
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
        <div class="tp-rows" id="${PANEL_ID}-rows">
            <div class="tp-row"><span title="Langevin bath temperature langevin_T (lattice units; c²=1/3).">T (bath)</span><span id="${PANEL_ID}-row-bath">—</span></div>
            <div class="tp-row"><span title="Kinetic temperature ⟨½|wave_vel|²⟩/(3/2) (equipartition, k_B≡1).">T_kin</span><span id="${PANEL_ID}-row-kinetic">—</span></div>
            <div class="tp-row"><span title="Manifestation fraction N/L³ — the condensate order parameter.">m (condensate)</span><span id="${PANEL_ID}-row-condensate">—</span></div>
            <div class="tp-row"><span title="Manifested voxels (the condensate &quot;particles&quot;) out of L³.">N voxels</span><span id="${PANEL_ID}-row-voxels">—</span></div>
            <div class="tp-row"><span title="Flux field energy.">E field ½Σ|J|²</span><span id="${PANEL_ID}-row-field">—</span></div>
            <div class="tp-row"><span title="Wave (kinetic) energy — sources T_kin.">E wave ½Σ|ẇ|²</span><span id="${PANEL_ID}-row-wave">—</span></div>
            <div class="tp-row"><span title="Current dynamic energy: field + wave + particle KE. Observer/vacuum baseline energy is excluded.">E total</span><span id="${PANEL_ID}-row-total">—</span></div>
        </div>
        <div class="tp-heatwrap">
            <div class="tp-heatlabel"><span>flux |J| heat map (z-slice)</span><span id="${PANEL_ID}-hmax"></span></div>
            <canvas id="${PANEL_ID}-heat" class="tp-heat" width="64" height="64"></canvas>
        </div>
        <svg id="${PANEL_ID}-spark" class="tp-spark" viewBox="0 0 240 34" preserveAspectRatio="none"><path id="${PANEL_ID}-spark-path" fill="none" stroke="var(--accent,#e8b04b)" stroke-width="1.4"/></svg>
        <div class="tp-foot"><b>[REFERENCE ENGINE]</b> T is an imposed Langevin bath
        parameter; T_kin uses an equipartition convention. Manifestation fraction
        is an occupancy diagnostic, not recovered matter. The historical onset
        near 0.05 is preparation-dependent. A finite heating campaign cannot
        establish the absence of a maximum temperature or guarantee stability
        under arbitrary heating.</div>`;
    return root;
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

function currentScale0Meta(group) {
    const meta = telemetryHub.getScale0TelemetryMeta?.(group) ?? null;
    return isCurrentScale0TelemetryMeta(meta) ? meta : null;
}

function telemetryStamp(meta) {
    if (!meta) return 'waiting';
    return [
        meta.source ?? 'unknown',
        meta.sourceEpoch ?? meta.epoch ?? 'local',
        meta.stateVersion ?? meta.snapshotVersion ?? 'unversioned',
        meta.tick,
    ].join(':');
}

export function mountThermoPanel(host, getBridge) {
    if (!host) return null;
    document.getElementById(PANEL_ID)?.remove();
    const panel = buildPanel();
    host.appendChild(panel);
    const el = (id) => panel.querySelector(`#${PANEL_ID}-${id}`);

    const slider = el('slider'), tvalEl = el('tval');
    const phaseEl = el('phase'), barEl = el('bar'), mpctEl = el('mpct');
    const heat = el('heat'), hmaxEl = el('hmax'), sparkPathEl = el('spark-path');
    const rowEls = {
        bath: el('row-bath'),
        kinetic: el('row-kinetic'),
        condensate: el('row-condensate'),
        voxels: el('row-voxels'),
        field: el('row-field'),
        wave: el('row-wave'),
        total: el('row-total'),
    };
    const mHist = [];
    let historyGeneration = 0;
    const historyBuffer = {
        get count() { return mHist.length; },
        get total() { return mHist.length; },
        get generation() { return historyGeneration; },
        get: index => mHist[index]?.value,
        getTick: index => mHist[index]?.tick,
    };
    const historyControl = new TickHistoryControl(panel, {
        id: 'thermo-panel',
        defaultTicks: SPARK_MAX,
        onChange: () => renderHistory(),
    });
    let bridgeId = null;
    let resetVersion = -1;
    let lastHistoryStamp = null;
    let lastRenderStamp = null;
    let renderCount = 0;
    let tempFrame = null;
    let pendingTemp = null;

    function renderHistory() {
        let visible;
        if (historyControl.isAll) {
            const indices = projectHistoryIndices([historyBuffer], mHist.length, 240);
            visible = indices ? indices.map(index => mHist[index]) : mHist.slice();
        } else {
            visible = historyControl.slice(mHist, entry => entry.tick);
        }
        const d = sparkPath(visible.map(entry => entry.value));
        if (sparkPathEl.getAttribute('d') !== d) sparkPathEl.setAttribute('d', d);
        if (sparkPathEl.style.display === (d ? 'none' : '')) {
            sparkPathEl.style.display = d ? '' : 'none';
        }
    }

    function commitTemp(T, ctx, owner, loadGeneration) {
        if (!Number.isFinite(T) || T < 0 || T > 0.20) return false;
        if (!ctx || !owner || typeof owner.setLangevinTemp !== 'function') return false;
        try {
            return commitScale0ScientificMutation(ctx, {
                reason: SCALE0_MUTATION_REASONS.PARAMETER_CHANGE,
                source: SCALE0_MUTATION_SOURCES.THERMODYNAMICS,
                loadGeneration,
                owner,
            }, (activeOwner) => activeOwner.setLangevinTemp(T)).accepted;
        } catch (e) {
            return false;
        }
    }

    function setTemp(T) {
        // Presets and imperative API calls are newer explicit intents than any
        // high-frequency slider sample queued for the next animation frame.
        cancelScheduledTemp();
        const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        return commitTemp(T, ctx, getBridge?.(), Number(ctx?._loadGeneration));
    }

    function scheduleTemp(T) {
        const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        pendingTemp = {
            T,
            ctx,
            owner: getBridge?.(),
            loadGeneration: Number(ctx?._loadGeneration),
        };
        if (tempFrame) return;
        const flush = () => {
            const job = pendingTemp;
            pendingTemp = null;
            tempFrame = null;
            if (!job || job.ctx !== ((typeof window !== 'undefined') ? window.__ftdCtx : null)) return;
            commitTemp(job.T, job.ctx, job.owner, job.loadGeneration);
        };
        tempFrame = (typeof requestAnimationFrame === 'function')
            ? { kind: 'raf', id: requestAnimationFrame(flush) }
            : { kind: 'timeout', id: setTimeout(flush, 0) };
    }

    function cancelScheduledTemp() {
        if (tempFrame?.kind === 'raf' && typeof cancelAnimationFrame === 'function') {
            cancelAnimationFrame(tempFrame.id);
        } else if (tempFrame?.kind === 'timeout') {
            clearTimeout(tempFrame.id);
        }
        tempFrame = null;
        pendingTemp = null;
    }

    slider.addEventListener('input', () => {
        const T = parseFloat(slider.value);
        tvalEl.textContent = T.toFixed(3);
        scheduleTemp(T);
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
        const nextResetVersion = telemetryHub.getResetVersion?.(0) ?? 0;
        if (b !== bridgeId || nextResetVersion !== resetVersion) {
            bridgeId = b;
            resetVersion = nextResetVersion;
            lastHistoryStamp = null;
            lastRenderStamp = null;
            mHist.length = 0;
            historyGeneration++;
        }
        if (!isPanelLive(host)) return;

        // A finite group tick is part of the measurement contract. Retained
        // objects without one are awaiting provenance, not a current zero-valued
        // thermodynamic state.
        const diagMeta = currentScale0Meta('diagnostics');
        const auditMeta = currentScale0Meta('audit');
        // A fallback cannot acquire the missing producer provenance. Read the
        // qualified groups directly instead of computing and discarding it.
        const diag = diagMeta ? telemetryHub.s0?.diag ?? null : null;
        const audit = auditMeta ? telemetryHub.s0?.audit ?? null : null;
        const L = Number.isFinite(b.latticeSize) && b.latticeSize > 0
            ? b.latticeSize : null;
        const Nvox = Number.isFinite(L) ? L * L * L : null;
        const N = Number.isFinite(diag?.manifested) ? diag.manifested : null;
        const m = Number.isFinite(N) && Number.isFinite(Nvox)
            ? Math.min(1, N / Nvox) : null;
        const waveE = readScale0WaveEnergy(diag, audit);
        const fieldE = readScale0FieldEnergy(audit);
        const totalE = readScale0TotalEnergy(diag, audit, { diagMeta, auditMeta });
        const tKin = Number.isFinite(waveE) && Number.isFinite(Nvox)
            ? waveE / (1.5 * Nvox) : null;
        const fmt = (value, digits) => Number.isFinite(value) ? value.toFixed(digits) : '—';
        // Actual bath temperature from the engine (falls back to the slider).
        const bathReadback = (typeof b.getLangevinTemp === 'function')
            ? b.getLangevinTemp() : Number.NaN;
        const imposedBath = parseFloat(slider.value);
        const Tset = Number.isFinite(bathReadback)
            ? bathReadback : (Number.isFinite(imposedBath) ? imposedBath : null);

        const freshnessState = diagMeta && auditMeta
            ? (diagMeta.tick === auditMeta.tick ? 'current' : 'mixed')
            : (diagMeta || auditMeta ? 'mixed-waiting' : 'waiting');
        const renderStamp = [
            resetVersion,
            telemetryStamp(diagMeta),
            telemetryStamp(auditMeta),
            Number.isFinite(L) ? L : 'no-lattice',
            Number.isFinite(Tset) ? Tset : 'no-temperature',
        ].join('|');
        if (panel.dataset.telemetryState !== freshnessState) {
            panel.dataset.telemetryState = freshnessState;
        }
        if (renderStamp === lastRenderStamp) return;
        lastRenderStamp = renderStamp;
        renderCount++;

        // phase + bar
        let label = 'UNAVAILABLE', color = 'var(--warning-text,#e8b04b)';
        if (Number.isFinite(m)) {
            label = 'VACUUM'; color = 'var(--text-secondary,#aaa)';
            if (m > 0.9) { label = 'CONDENSED'; color = 'var(--accent,#e8b04b)'; }
            else if (m > 0.05) { label = 'IGNITING'; color = '#e87a4b'; }
        }
        setTextIfChanged(phaseEl, label);
        if (phaseEl.style.color !== color) phaseEl.style.color = color;
        const barWidth = Number.isFinite(m) ? `${(m * 100).toFixed(1)}%` : '0%';
        if (barEl.style.width !== barWidth) barEl.style.width = barWidth;
        setTextIfChanged(mpctEl, Number.isFinite(m) ? `${(m * 100).toFixed(0)}%` : '—');

        // telemetry rows
        setTextIfChanged(rowEls.bath, Number.isFinite(Tset)
            ? `${Tset.toFixed(3)}  (${(Tset / C2).toFixed(2)} c²)` : '—');
        setTextIfChanged(rowEls.kinetic, fmt(tKin, 4));
        setTextIfChanged(rowEls.condensate, fmt(m, 4));
        setTextIfChanged(rowEls.voxels, Number.isFinite(N) && Number.isFinite(Nvox)
            ? `${N} / ${Nvox}` : `— / ${Number.isFinite(Nvox) ? Nvox : '—'}`);
        setTextIfChanged(rowEls.field, fmt(fieldE, 3));
        setTextIfChanged(rowEls.wave, fmt(waveE, 3));
        setTextIfChanged(rowEls.total, fmt(totalE, 3));

        // flux |J| heat map (z mid-slice)
        let paintedSlice = false;
        try {
            const mid = (L / 2) | 0;
            const s = diagMeta && Number.isFinite(L) && typeof b.getFluxSlice === 'function'
                ? b.getFluxSlice(2, mid) : null;
            if (s && s.length >= L * L) {
                let measuredMax = 0;
                for (let i = 0; i < s.length; i++) {
                    if (Number.isFinite(s[i]) && s[i] > measuredMax) measuredMax = s[i];
                }
                if (heat.width !== L) { heat.width = L; heat.height = L; }
                paintSliceToCanvas(heat, transposeAndFlipNN(s, L), L, {
                    ramp: rampEmEnergy,
                    norm: 1 / Math.max(measuredMax, 1e-9),
                });
                setTextIfChanged(hmaxEl, `|J|max ${measuredMax.toFixed(2)}`);
                paintedSlice = true;
            }
        } catch (e) { /* slice unavailable on this bridge */ }
        if (!paintedSlice) {
            heat.getContext('2d')?.clearRect(0, 0, heat.width, heat.height);
            setTextIfChanged(hmaxEl, '—');
        }

        // m sparkline
        const tick = Number.isFinite(diag?.tick) ? diag.tick : null;
        const historyStamp = diagMeta ? telemetryStamp(diagMeta) : null;
        if (Number.isFinite(m) && tick !== null && historyStamp !== lastHistoryStamp) {
            lastHistoryStamp = historyStamp;
            mHist.push({ tick, value: m });
        }
        renderHistory();
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
        get renderCount() { return renderCount; },
        get historyLength() { return mHist.length; },
        setTemp: (T) => {
            slider.value = String(T);
            tvalEl.textContent = (+T).toFixed(3);
            return setTemp(+T);
        },
        dispose: () => {
            cancelScheduledTemp();
            armSub.unsubscribe();
            liveSub?.unsubscribe();
            historyControl.destroy();
            if (typeof window !== 'undefined' && window.__ftdThermoPanel === api) window.__ftdThermoPanel = null;
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdThermoPanel = api;
    return api;
}

export function initThermoPanel() {
    if (typeof document === 'undefined') return null;
    if (typeof window !== 'undefined' && window.__ftdThermoPanel) return window.__ftdThermoPanel;
    const host = document.getElementById('panel-thermo');
    if (!host) return null;
    const getBridge = () => resolveActiveScale0BridgeFromWindow();
    return mountThermoPanel(host, getBridge);
}
