import { formatEnergySim, SIM_ENERGY_TO_MEV } from '../../../units.js';
import { formatSI } from '../../scale-utils.js';
import { telemetryHub } from '../../../telemetry-hub.js';
import {
    getScale0TelemetryDemand,
    collectScale0OnDemand,
    collectScale0Unconditional,
} from '../../../telemetry/demand.js';
import { PerfFlags } from '../../../config/perf-flags.js';

const _num = (v, d = 2) => (Number.isFinite(v) ? v.toFixed(d) : null);

/**
 * Live decomposition tooltip for the status-bar energy readout (whole-box
 * audit channels, sim units). Feeds the shared ui-tooltip system via
 * dataset.uiTooltip — hover text is read at hover time, so per-frame updates
 * are visible. Keeps the "Current total energy" prefix (asserted by
 * scales.spec.js tooltip coverage).
 */
function _updateEnergyTooltip(el, diag) {
    if (!el) return;
    const parts = [];
    // Decomposition lives on the energy-audit snapshot. When audit is gated
    // off, cheap diag may omit these channels — prefer a live hub audit, then
    // fall back to whatever the diag copy still carries.
    const src = telemetryHub.s0?.audit || diag;
    const f = _num(src.fieldEnergy ?? diag.fieldEnergy, 1);
    const w = _num(src.waveEnergy ?? diag.waveEnergy, 1);
    const k = _num(src.particleKE ?? diag.particleKE);
    const r = _num(src.restEnergy ?? diag.restEnergy);
    if (f !== null) parts.push(`field ½|J|²: ${f}`);
    if (w !== null) parts.push(`wave ½|v|²: ${w}`);
    if (k !== null) parts.push(`particle KE: ${k}`);
    if (r !== null) parts.push(`rest (excluded): ${r}`);
    const decomp = parts.length ? ` — ${parts.join(' · ')}` : '';
    const text = 'Current total energy: whole-box dynamic sum in engine sim '
        + `units (rest mass excluded)${decomp}. MeV value is the `
        + `electron-primary calibration (1 sim ≡ ${SIM_ENERGY_TO_MEV} MeV via `
        + 'E_REST = K_B·C² ≙ 0.511 MeV) [CALIBRATION], not a derived energy.';
    el.dataset.uiTooltip = text;
    el.dataset.uiTooltipSource = 'scale0-live';
    const item = el.closest('.status-item');
    if (item) {
        item.dataset.uiTooltip = text;
        item.dataset.uiTooltipSource = 'scale0-live';
    }
}

export function updateDiagnosticsAndPanels(ctx, state) {
    if (ctx.frameCount % 3 !== 0) return;

    // collectScale0 is cheap (status bar + primary history) — always runs.
    const diag = telemetryHub.collectScale0(ctx.bridge, state.fluxMock, state.useFluxMock);

    if (PerfFlags.telemetryOnDemand) {
        const demand = getScale0TelemetryDemand(ctx);
        collectScale0OnDemand(telemetryHub, ctx, state, demand);
    } else {
        collectScale0Unconditional(telemetryHub, ctx, state);
    }

    if (ctx.activeTab === 'diagnostics') {
        ctx.diagnosticsPanel?.update();
    } else if (ctx.activeTab === 'charts') {
        ctx.chartsPanel?.update();
    } else if (ctx.activeTab === 'lagrangian') {
        ctx.lagrangianPanel?.update();
    }

    if (!diag) return;

    ctx.dom.statusPtime.textContent     = formatSI(Math.round(diag.physicalTime !== undefined ? diag.physicalTime : diag.tick));
    ctx.dom.statusParticles.textContent = diag.manifested || 0;
    // Scale-0 energy is a WHOLE-BOX sum in engine sim units (dynamic channel:
    // Σ½|J|² + Σ½|wave_vel|² + particle KE; rest mass excluded). The "MeV"
    // value is the electron-primary [CALIBRATION] (1 sim ≡ 3 MeV), not a
    // derived physical energy — see units.js SIM_ENERGY_TO_MEV.
    ctx.dom.statusEnergy.textContent    = formatEnergySim(diag.totalEnergy).text;
    _updateEnergyTooltip(ctx.dom.statusEnergy, diag);

    if (ctx.running) {
        ctx.dom.statusDot.classList.remove('idle');
        ctx.dom.statusState.textContent = 'Running';
    } else {
        ctx.dom.statusDot.classList.add('idle');
        ctx.dom.statusState.textContent = 'Idle';
    }

    if (ctx.activeTab === 'diagnostics' && ctx.peTelemetry) {
        ctx.peTelemetry.drawCharts();
    }
    // Floated inspector is still visible after the dock activates another tab.
    // Gate on isPanelVisible, not activeTab — same contract as Scale 1.
    const inspectorLive = typeof ctx.isPanelVisible === 'function'
        ? ctx.isPanelVisible('inspector')
        : ctx.activeTab === 'inspector';
    if (inspectorLive) ctx.inspector?.update();
}
