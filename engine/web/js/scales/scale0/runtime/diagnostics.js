import { formatEnergy } from '../../../units.js';
import { formatSI } from '../../scale-utils.js';
import { telemetryHub } from '../../../telemetry-hub.js';
import { PerfFlags } from '../../../config/perf-flags.js';

export function updateDiagnosticsAndPanels(ctx, state) {
    if (ctx.frameCount % 3 !== 0) return;

    // ── Collect via TelemetryHub (single source of truth for all bridge calls) ──
    // collectScale0 is cheap (status bar + primary history) and stays unconditional.
    const diag = telemetryHub.collectScale0(ctx.bridge, state.fluxMock, state.useFluxMock);

    if (PerfFlags.telemetryOnDemand) {
        // Demand-gate the EXPENSIVE audit/Lagrangian streams on a visible consumer
        // + a field-version change. Keeping collectScale0 (above) unconditional means
        // the primary sparkline history is never interrupted (SPEC_SCALE0_PERF §3).
        const wantsConservationAudit = typeof window !== 'undefined' && !!window.__ftdConservationPanel;
        const wantAudit = ctx.isPanelVisible('diagnostics') || ctx.isPanelVisible('charts')
            || ctx.isPanelVisible('lagrangian') || ctx.isPanelVisible('telemetry-grid')
            || wantsConservationAudit;
        const wantLag = ctx.isPanelVisible('charts') || ctx.isPanelVisible('lagrangian')
            || ctx.isPanelVisible('telemetry-grid');

        // Forward the want-mask to the worker so it stops computing these ~60×/s when
        // nothing consumes them (and resumes, with catch-up, on panel open).
        const fm = state.useFluxMock ? state.fluxMock : null;
        if (fm && typeof fm.setTelemetryMask === 'function') fm.setTelemetryMask(wantAudit, wantLag);

        // Version-gate so a paused/static field never re-pays the O(N³) collect; the
        // want-edge (panel just opened) forces a one-shot catch-up even if static.
        const ver = state.fieldDataVersion | 0;
        const verChanged = ver !== telemetryHub._lastAuditVersion;
        const openedA = wantAudit && !telemetryHub._prevWantAudit;
        const openedL = wantLag && !telemetryHub._prevWantLag;
        if (wantAudit && (verChanged || openedA)) {
            telemetryHub.collectScale0Audit(ctx.bridge, state.fluxMock, state.useFluxMock);
        }
        if (wantLag && (verChanged || openedL)) {
            telemetryHub.collectScale0Lagrangian(ctx.bridge, state.fluxMock, state.useFluxMock);
        }
        if (wantAudit || wantLag) telemetryHub._lastAuditVersion = ver;
        telemetryHub._prevWantAudit = wantAudit;
        telemetryHub._prevWantLag = wantLag;
    } else {
        // Legacy (flag off): collect unconditionally every 3rd frame — the 2026-06-05
        // "all sidepanels live" behavior, preserved verbatim as the rollback path.
        telemetryHub.collectScale0Audit(ctx.bridge, state.fluxMock, state.useFluxMock);
        telemetryHub.collectScale0Lagrangian(ctx.bridge, state.fluxMock, state.useFluxMock);
    }

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
    }
}
