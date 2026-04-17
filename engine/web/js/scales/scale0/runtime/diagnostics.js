import { formatEnergy } from '../../../units.js';
import { formatSI } from '../../scale-utils.js';
import { telemetryHub } from '../../../telemetry-hub.js';

export function updateDiagnosticsAndPanels(ctx, state) {
    if (ctx.frameCount % 3 !== 0) return;

    // ── Collect via TelemetryHub (single source of truth for all bridge calls) ──
    const diag = telemetryHub.collectScale0(ctx.bridge, state.fluxMock, state.useFluxMock);

    // New panel renders from hub state directly; refresh it every frame even
    // when no diag is available (formatters show 0 for missing fields so the
    // UI stays "wired" while the bridge spins up).
    if (ctx.activeTab === 'diagnostics') {
        telemetryHub.collectScale0Audit(ctx.bridge, state.fluxMock, state.useFluxMock);
        ctx.diagnosticsPanel?.update();
    }

    if (!diag) return;

    // ── Status bar ───────────────────────────────────────────────────────────
    ctx.dom.statusTick.textContent      = formatSI(diag.tick);
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

    // ── Panel objects receive latest hub data ────────────────────────────────
    // Legacy: drives status bar + any non-migrated DOM (no-ops on missing IDs).
    ctx.diagnostics.update(diag);

    // ── Tab-specific rendering ───────────────────────────────────────────────
    switch (ctx.activeTab) {
        case 'diagnostics': {
            // The new diagnostics panel was already refreshed above;
            // only the legacy peTelemetry drawing is left here.
            if (ctx.peTelemetry) ctx.peTelemetry.drawCharts();
            break;
        }
        case 'charts': {
            // Collect audit + lagrangian so chart buffers stay fresh.
            telemetryHub.collectScale0Audit(ctx.bridge, state.fluxMock, state.useFluxMock);
            telemetryHub.collectScale0Lagrangian(ctx.bridge, state.fluxMock, state.useFluxMock);
            ctx.chartsPanel?.update();
            break;
        }
        case 'lagrangian':
            telemetryHub.collectScale0Lagrangian(ctx.bridge, state.fluxMock, state.useFluxMock);
            ctx.lagrangianChart.draw();
            break;
        case 'inspector':
            ctx.inspector.update();
            break;
        case 'hierarchy':
            ctx.updateHierarchyPanel();
            break;
    }
}
