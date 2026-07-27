/**
 * Scale-0 telemetry demand gating — decides which expensive hub collectors run.
 * See SPEC_SCALE0_PERF_TELEMETRY_PANELS.md and CONTRACTS.md §5.
 */

/**
 * @param {object} ctx - scale-0 controller context (isPanelVisible, activeTab, …)
 * @returns {{ wantAudit: boolean, wantLag: boolean }}
 */
export function getScale0TelemetryDemand(ctx) {
    // Keyed on the panel being RENDERED, not merely constructed.
    // mountConservationMicropanel() assigns window.__ftdConservationPanel
    // unconditionally, so testing the reference alone pinned wantAudit true for
    // the entire Scale-0 session on behalf of a consumer that consumed nothing.
    const consPanel = typeof window !== 'undefined' ? window.__ftdConservationPanel : null;
    const consEl = (typeof document !== 'undefined')
        ? document.getElementById('conservation-micropanel') : null;
    const wantsConservationAudit = !!consPanel && !!consEl && consEl.getClientRects().length > 0;
    const wantAudit = ctx.isPanelVisible('diagnostics') || ctx.isPanelVisible('charts')
        || ctx.isPanelVisible('lagrangian') || ctx.isPanelVisible('telemetry-grid')
        || wantsConservationAudit;
    const wantLag = ctx.isPanelVisible('charts') || ctx.isPanelVisible('lagrangian')
        || ctx.isPanelVisible('telemetry-grid');
    return { wantAudit, wantLag };
}

/**
 * Apply demand-gated audit/Lagrangian collection with field-version coalescing.
 *
 * @param {import('../telemetry-hub.js').TelemetryHub} telemetryHub
 * @param {object} ctx
 * @param {object} state - scale-0 runtime state (fieldDataVersion, fluxMock, useFluxMock)
 * @param {{ wantAudit: boolean, wantLag: boolean }} demand
 */
export function collectScale0OnDemand(telemetryHub, ctx, state, demand) {
    const { wantAudit, wantLag } = demand;

    const fm = state.useFluxMock ? state.fluxMock : null;
    if (fm && typeof fm.setTelemetryMask === 'function') {
        fm.setTelemetryMask(wantAudit, wantLag);
    }

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
}

/**
 * Unconditional audit + Lagrangian collection (rollback path when PerfFlags.telemetryOnDemand is off).
 */
export function collectScale0Unconditional(telemetryHub, ctx, state) {
    telemetryHub.collectScale0Audit(ctx.bridge, state.fluxMock, state.useFluxMock);
    telemetryHub.collectScale0Lagrangian(ctx.bridge, state.fluxMock, state.useFluxMock);
}
