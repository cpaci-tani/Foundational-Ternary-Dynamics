/**
 * Scale-0 telemetry demand gating — decides which expensive hub collectors run.
 * See SPEC_SCALE0_PERF_TELEMETRY_PANELS.md and CONTRACTS.md §5.
 */

/**
 * @param {object} ctx - scale-0 controller context (isPanelVisible, activeTab, …)
 * @returns {{ diagnostics:boolean, wantAudit:boolean, wantLag:boolean,
 *            wantGravity:boolean, audit:boolean, lagrangian:boolean,
 *            gravity:boolean, everyTicks:object }}
 */
export function getScale0TelemetryDemand(ctx) {
    const visible = (id) => (typeof ctx?.isPanelVisible === 'function'
        ? ctx.isPanelVisible(id)
        : ctx?.activeTab === id);
    // Keyed on the panel being RENDERED, not merely constructed.
    // mountConservationMicropanel() assigns window.__ftdConservationPanel
    // unconditionally, so testing the reference alone pinned wantAudit true for
    // the entire Scale-0 session on behalf of a consumer that consumed nothing.
    const consPanel = typeof window !== 'undefined' ? window.__ftdConservationPanel : null;
    const consEl = (typeof document !== 'undefined')
        ? document.getElementById('conservation-micropanel') : null;
    const wantsConservationAudit = !!consPanel && !!consEl && consEl.getClientRects().length > 0;
    const wantAudit = visible('diagnostics') || visible('charts')
        || visible('lagrangian') || visible('telemetry-grid')
        || wantsConservationAudit;
    // The Charts panel has no Lagrangian series; requesting the deepest
    // stencil reduction merely because ordinary energy charts are visible
    // made native sidebars compete with playback.  The dedicated Lagrangian
    // panel and the telemetry grid do render Lagrangian channels, so they
    // remain explicit consumers.
    const wantLag = visible('lagrangian') || visible('telemetry-grid');
    // Both the Gravity and Time panels render the native latency aggregate.
    // They therefore share the gravity scheduler stream; neither panel may
    // issue a separate bridge getter/RPC from its own rAF callback.
    const wantGravity = visible('gravity') || visible('time');
    const latticeSize = Math.max(1, Math.trunc(Number(ctx?.bridge?.latticeSize) || 32));
    const everyTicks = latticeSize >= 113
        ? { diagnostics: 1, audit: 8, gravity: 4, lagrangian: 12 }
        : (latticeSize >= 65
            ? { diagnostics: 1, audit: 6, gravity: 3, lagrangian: 8 }
            : { diagnostics: 1, audit: 4, gravity: 2, lagrangian: 6 });
    return {
        diagnostics: true,
        audit: wantAudit,
        lagrangian: wantLag,
        gravity: wantGravity,
        wantAudit,
        wantLag,
        wantGravity,
        everyTicks,
    };
}

/**
 * Register the complete Scale-0 demand with a native snapshot scheduler. The
 * bridge coalesces identical calls, so this is safe from the controller's
 * regular UI pass. WASM/mock owners retain their synchronous collectors.
 */
function publishNativeTelemetryDemand(ctx, state, demand) {
    if (state.useFluxMock || typeof ctx?.bridge?.setTelemetryDemand !== 'function') return false;
    ctx.bridge.setTelemetryDemand({
        diagnostics: !!demand.diagnostics,
        audit: !!demand.audit,
        lagrangian: !!demand.lagrangian,
        gravity: !!demand.gravity,
        everyTicks: demand.everyTicks,
    });
    return typeof ctx.bridge.getTelemetrySnapshot === 'function';
}

/**
 * Apply demand-gated audit/Lagrangian collection with field-version coalescing.
 *
 * @param {import('../telemetry-hub.js').TelemetryHub} telemetryHub
 * @param {object} ctx
 * @param {object} state - scale-0 runtime state (fieldDataVersion, fluxMock, useFluxMock)
 * @param {{ wantAudit: boolean, wantLag: boolean, wantGravity?: boolean,
 *            diagnostics?: boolean, everyTicks?: object }} demand
 */
export function collectScale0OnDemand(telemetryHub, ctx, state, demand) {
    const { wantAudit, wantLag } = demand;

    const fm = state.useFluxMock ? state.fluxMock : null;
    if (fm && typeof fm.setTelemetryMask === 'function') {
        fm.setTelemetryMask(wantAudit, wantLag);
    }

    // Native `getTelemetrySnapshot()` is a read-only versioned store. Ingest
    // it every UI pass so an async push received while paused is visible even
    // though fieldDataVersion has not advanced. This intentionally performs no
    // CUDA reduction and no panel-triggered WebSocket request.
    if (publishNativeTelemetryDemand(ctx, state, demand)) {
        telemetryHub.collectScale0(ctx.bridge, state.fluxMock, state.useFluxMock);
        telemetryHub._prevWantAudit = wantAudit;
        telemetryHub._prevWantLag = wantLag;
        return;
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
    const demand = {
        diagnostics: true,
        audit: true,
        lagrangian: true,
        gravity: true,
        everyTicks: { diagnostics: 1, audit: 1, gravity: 1, lagrangian: 1 },
    };
    if (publishNativeTelemetryDemand(ctx, state, demand)) {
        telemetryHub.collectScale0(ctx.bridge, state.fluxMock, state.useFluxMock);
        return;
    }
    telemetryHub.collectScale0Audit(ctx.bridge, state.fluxMock, state.useFluxMock);
    telemetryHub.collectScale0Lagrangian(ctx.bridge, state.fluxMock, state.useFluxMock);
}
