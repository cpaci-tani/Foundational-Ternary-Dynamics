import { formatEnergy } from '../../../units.js';
import { formatSI } from '../../scale-utils.js';
import { telemetryHub } from '../../../telemetry-hub.js';
import {
    getScale0TelemetryDemand,
    collectScale0OnDemand,
    collectScale0Unconditional,
} from '../../../telemetry/demand.js';
import { PerfFlags } from '../../../config/perf-flags.js';

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
    ctx.dom.statusEnergy.textContent    = formatEnergy(diag.totalEnergy, 0).text;

    if (ctx.running) {
        ctx.dom.statusDot.classList.remove('idle');
        ctx.dom.statusState.textContent = 'Running';
    } else {
        ctx.dom.statusDot.classList.add('idle');
        ctx.dom.statusState.textContent = 'Idle';
    }

    switch (ctx.activeTab) {
        case 'diagnostics':
            if (ctx.peTelemetry) ctx.peTelemetry.drawCharts();
            break;
        case 'inspector':
            ctx.inspector.update();
            break;
    }
}
