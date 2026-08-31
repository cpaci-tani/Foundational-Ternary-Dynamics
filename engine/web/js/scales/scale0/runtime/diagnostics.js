import { formatEnergySim, SIM_ENERGY_TO_MEV } from '../../../units.js';
import { formatSI } from '../../scale-utils.js';
import { telemetryHub } from '../../../telemetry-hub.js';
import {
    getScale0TelemetryDemand,
    collectScale0OnDemand,
    collectScale0Unconditional,
} from '../../../telemetry/demand.js';
import { PerfFlags } from '../../../config/perf-flags.js';
import {
    isCurrentScale0AuditEnergy,
    isCurrentScale0TelemetryMeta,
    readScale0TotalEnergy,
} from '../../../telemetry/scale0-read.js';

const _num = (v, d = 2) => (Number.isFinite(v) ? v.toFixed(d) : null);

function _firstFinite(...values) {
    return values.find(value => typeof value === 'number' && Number.isFinite(value));
}

/**
 * Live decomposition tooltip for the status-bar energy readout (whole-box
 * per-tick ledger plus optional audit channels, sim units). Feeds the shared ui-tooltip system via
 * dataset.uiTooltip — hover text is read at hover time, so per-frame updates
 * are visible. Keeps the "Current total energy" prefix (asserted by
 * scales.spec.js tooltip coverage).
 */
function _updateEnergyTooltip(el, {
    audit = null,
    auditCurrent = false,
    energyCurrent = false,
    sampleHeld = false,
    energySource = null,
    sampleTick = null,
    stateTick = null,
} = {}) {
    if (!el) return;
    const parts = [];
    // Audit-derived decomposition must keep audit provenance. In particular,
    // worker diagnostics can carry a deliberately staggered audit cache; when
    // that audit group is stale, falling back to the copied diagnostic fields
    // would relabel an old reduction as a current measurement.
    const src = auditCurrent && audit ? audit : null;
    const f = _num(src?.fieldEnergy, 1);
    const w = _num(src?.waveEnergy, 1);
    const k = _num(src?.particleKE);
    const r = _num(src?.restEnergy ?? src?.particleRestEnergy);
    if (f !== null) parts.push(`field ½|J|²: ${f}`);
    if (w !== null) parts.push(`wave ½|v|²: ${w}`);
    if (k !== null) parts.push(`particle KE: ${k}`);
    if (r !== null) parts.push(`rest (excluded): ${r}`);
    const decomp = parts.length ? ` — ${parts.join(' · ')}` : '';
    const availability = energyCurrent
        ? ''
        : ' Current energy telemetry is unavailable; retained values are not presented as live.';
    const provenance = sampleHeld
        ? ` Latest completed audit sample t=${sampleTick} is held while state telemetry is t=${stateTick}; the value is not relabelled as a same-tick measurement.`
        : (energySource === 'per-tick-ledger'
            ? ' The live total is read from the engine per-tick energy ledger.'
            : (energySource === 'same-tick-audit'
                ? ' The live total is read from a completed same-tick energy audit.' : ''));
    const text = 'Current total energy: whole-box dynamic sum in engine sim '
        + `units (rest mass excluded)${decomp}.${availability}${provenance} MeV value is the `
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
    // Present cached worker/native telemetry at 30+ Hz on ordinary 60–75 Hz
    // displays. The expensive audit itself remains demand/cadence gated; this
    // only consumes completed samples and removes the visibly stepped 20–24 Hz
    // chart path.
    if (ctx.frameCount % 2 !== 0) return;

    // collectScale0 is cheap (status bar + primary history) — always runs.
    const diag = telemetryHub.collectScale0(ctx.bridge, state.fluxMock, state.useFluxMock);

    if (PerfFlags.telemetryOnDemand) {
        const demand = getScale0TelemetryDemand(ctx, state);
        collectScale0OnDemand(telemetryHub, ctx, state, demand);
    } else {
        collectScale0Unconditional(telemetryHub, ctx, state);
    }

    if (ctx.activeTab === 'diagnostics') {
        ctx.diagnosticsPanel?.update();
    } else if (ctx.activeTab === 'charts') {
        ctx.chartsPanel?.update();
    } else if (ctx.activeTab === 'telemetry-grid') {
        // Consume the sample in the same frame that published it. The app-level
        // fallback is reserved for floated/non-Scale-0 grids; its former 125 ms
        // gate dropped most Scale-0 samples and made these sparklines step.
        ctx.telemetryGridPanel?.update();
    } else if (ctx.activeTab === 'lagrangian') {
        ctx.lagrangianPanel?.update();
    }

    const diagMeta = telemetryHub.getScale0TelemetryMeta?.('diagnostics') ?? null;
    const auditMeta = telemetryHub.getScale0TelemetryMeta?.('audit') ?? null;
    const diagCurrent = !!diag && isCurrentScale0TelemetryMeta(diagMeta);
    const auditCurrent = !!telemetryHub.s0?.audit
        && isCurrentScale0AuditEnergy(diagMeta, auditMeta);
    const liveAudit = auditCurrent ? telemetryHub.s0.audit : null;
    const physicalTime = diagCurrent
        ? _firstFinite(diag.physicalTime, diag.tick) : undefined;
    ctx.viewport?.setGlobalClockState?.({
        tick: diagCurrent && Number.isFinite(diag.tick) ? diag.tick : 0,
        running: Boolean(ctx.running),
        maxCausalBudget: diagCurrent && Number.isFinite(diag.maxCausalBudget)
            ? diag.maxCausalBudget : null,
        causalProjectionEvents: diagCurrent && Number.isFinite(diag.causalProjectionEvents)
            ? diag.causalProjectionEvents : 0,
    });
    ctx.dom.statusPtime.textContent = Number.isFinite(physicalTime)
        ? formatSI(Math.round(physicalTime)) : '—';
    ctx.dom.statusParticles.textContent = diagCurrent && Number.isFinite(diag.manifested)
        ? String(diag.manifested) : '—';
    // Scale-0 energy is a WHOLE-BOX sum in engine sim units (dynamic channel:
    // Σ½|J|² + Σ½|wave_vel|² + particle KE; rest mass excluded). The "MeV"
    // value is the electron-primary [CALIBRATION] (1 sim ≡ 3 MeV), not a
    // derived physical energy — see units.js SIM_ENERGY_TO_MEV.
    const statusEnergy = readScale0TotalEnergy(diag, telemetryHub.s0?.audit, {
        diagMeta,
        auditMeta,
        allowAuditSampleHold: true,
    });
    const statusUsesDiagnosticEnergy = diagCurrent && Number.isFinite(diag?.dynamicEnergy);
    const statusUsesAlignedAudit = auditCurrent
        && Number.isFinite(telemetryHub.s0?.audit?.dynamicEnergy);
    const sampleHeld = Number.isFinite(statusEnergy)
        && !statusUsesDiagnosticEnergy
        && !statusUsesAlignedAudit
        && isCurrentScale0TelemetryMeta(auditMeta)
        && Number.isFinite(telemetryHub.s0?.audit?.dynamicEnergy);
    ctx.dom.statusEnergy.textContent = Number.isFinite(statusEnergy)
        ? formatEnergySim(statusEnergy).text : '—';
    ctx.dom.statusEnergy.dataset.sampleHeld = sampleHeld ? 'true' : 'false';
    ctx.dom.statusEnergy.dataset.sampleTick = Number.isFinite(auditMeta?.tick)
        ? String(auditMeta.tick) : '';
    ctx.dom.statusEnergy.dataset.stateTick = Number.isFinite(diagMeta?.tick)
        ? String(diagMeta.tick) : '';
    _updateEnergyTooltip(ctx.dom.statusEnergy, {
        audit: liveAudit,
        auditCurrent,
        energyCurrent: Number.isFinite(statusEnergy),
        sampleHeld,
        energySource: statusUsesDiagnosticEnergy
            ? diag?.energySampleSource
            : (statusUsesAlignedAudit ? 'same-tick-audit' : null),
        sampleTick: auditMeta?.tick,
        stateTick: diagMeta?.tick,
    });

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
