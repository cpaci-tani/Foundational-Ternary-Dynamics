import { formatEnergy } from '../../../units.js';
import { formatSI } from '../../scale-utils.js';
import { telemetryHub } from '../../../telemetry-hub.js';

export function updateDiagnosticsAndPanels(ctx, state) {
    if (ctx.frameCount % 3 !== 0) return;

    // ── Collect via TelemetryHub (single source of truth for all bridge calls) ──
    const diag = telemetryHub.collectScale0(ctx.bridge, state.fluxMock, state.useFluxMock);

    // Always collect extended telemetry if we're rendering anything that might need it
    // (Floating panels like Telemetry Grid require this even if charts tab is closed)
    telemetryHub.collectScale0Audit(ctx.bridge, state.fluxMock, state.useFluxMock);
    telemetryHub.collectScale0Lagrangian(ctx.bridge, state.fluxMock, state.useFluxMock);

    // New panels render from hub state directly; refresh them every frame even
    // when no diag is available (formatters show 0 for missing fields so the
    // UI stays "wired" while the bridge spins up).
    if (ctx.activeTab === 'diagnostics') {
        ctx.diagnosticsPanel?.update();
    } else if (ctx.activeTab === 'charts') {
        ctx.chartsPanel?.update();
    } else if (ctx.activeTab === 'lagrangian') {
        ctx.lagrangianPanel?.update();
    }

    if (!diag) return;

    // ── Status bar ───────────────────────────────────────────────────────────
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

    // ── Tab-specific rendering (only tabs not handled above) ────────────────
    switch (ctx.activeTab) {
        case 'diagnostics':
            if (ctx.peTelemetry) ctx.peTelemetry.drawCharts();
            break;
        case 'inspector':
            ctx.inspector.update();
            break;
        case 'hierarchy':
            ctx.updateHierarchyPanel();
            break;
    }
}
