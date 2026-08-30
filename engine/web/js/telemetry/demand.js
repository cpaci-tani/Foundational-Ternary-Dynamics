/**
 * Scale-0 telemetry demand gating — decides which expensive hub collectors run.
 * See SPEC_SCALE0_PERF_TELEMETRY_PANELS.md and CONTRACTS.md §5.
 */

import { isScale0AuthoritativeGenerationReady } from '../scales/scale0/state/store.js';

/** Scale-0 chart ids whose series are filled from the energy-audit path. */
const SCALE0_AUDIT_CHART_IDS = Object.freeze(['eb-energy', 'gauss']);

/**
 * Charts is an audit consumer only when a card that *reads* audit buffers is
 * actually active. Default chips (flux, particles, charge, entropy) are cheap
 * collectScale0 history. Treating the whole Charts tab as an audit consumer
 * kept the O(N³) pass running for users who never opened E vs B / Gauss.
 */
function scale0ChartsWantAudit(ctx, visible) {
    if (!visible('charts')) return false;
    const active = ctx?.chartsPanel?.active;
    if (active && typeof active.has === 'function') {
        for (const id of SCALE0_AUDIT_CHART_IDS) {
            if (active.has(id)) return true;
        }
        return false;
    }
    return false;
}

/**
 * @param {object} ctx - scale-0 controller context (isPanelVisible, activeTab, …)
 * @param {object|null} state - optional Scale-0 runtime state for scenario gates
 * @returns {{ diagnostics:boolean, wantAudit:boolean, wantLag:boolean,
 *            wantGravity:boolean, audit:boolean, lagrangian:boolean,
 *            gravity:boolean, everyTicks:object }}
 */
export function getScale0TelemetryDemand(ctx, state = null) {
    const visible = (id) => (typeof ctx?.isPanelVisible === 'function'
        ? ctx.isPanelVisible(id)
        : ctx?.activeTab === id);
    // Conservation is always-on on Scale 0 (viewport overlay). It must NOT pin
    // wantAudit — that undoes the demand gate for the whole session. ΔE/ΔL/ΔQ
    // come from cheap diagnostics; Δp is shown only when a live hub audit
    // already exists (Diagnostics / Lagrangian / Grid / Knots / E−B·Gauss).
    // Knots used to call getScale0EnergyAudit() directly; it is now a named
    // consumer so the worker mask and the hub stay in lockstep.
    const knotsApplicable = state?.currentScenarioId !== 'empty'
        && state?.knotTrackingApplicable !== false;
    const knotsTracking = state == null ? true : !!state.knotTracking;
    const wantAudit = visible('diagnostics')
        || scale0ChartsWantAudit(ctx, visible)
        || visible('lagrangian') || visible('telemetry-grid')
        || (visible('knots') && knotsApplicable && knotsTracking);
    // The Charts panel has no Lagrangian series; requesting the deepest
    // stencil reduction merely because ordinary energy charts are visible
    // made native sidebars compete with playback.  The dedicated Lagrangian
    // panel and the telemetry grid do render Lagrangian channels, so they
    // remain explicit consumers.
    const wantLag = visible('lagrangian') || visible('telemetry-grid');
    // Both the Gravity and Time panels render the native latency aggregate.
    // They therefore share the gravity scheduler stream; neither panel may
    // issue a separate bridge getter/RPC from its own rAF callback.
    // Empty defines neither a gravity source nor a material clock/metric
    // observation. A visible inapplicable panel must not keep the native
    // gravity reduction/RPC stream alive.
    const gravityApplicable = state == null || (
        state?.currentScenarioId !== 'empty'
        && isScale0AuthoritativeGenerationReady(state)
    );
    const wantGravity = gravityApplicable && (visible('gravity') || visible('time'));
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
        fm.setTelemetryMask(wantAudit, wantLag, !!demand.wantGravity);
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
    const gravityReady = state?.currentScenarioId !== 'empty'
        && isScale0AuthoritativeGenerationReady(state);
    const demand = {
        diagnostics: true,
        audit: true,
        lagrangian: true,
        gravity: gravityReady,
        everyTicks: { diagnostics: 1, audit: 1, gravity: 1, lagrangian: 1 },
    };
    if (publishNativeTelemetryDemand(ctx, state, demand)) {
        telemetryHub.collectScale0(ctx.bridge, state.fluxMock, state.useFluxMock);
        return;
    }
    telemetryHub.collectScale0Audit(ctx.bridge, state.fluxMock, state.useFluxMock);
    telemetryHub.collectScale0Lagrangian(ctx.bridge, state.fluxMock, state.useFluxMock);
}
