/**
 * Scale-0 telemetry demand gating — decides which expensive hub collectors run.
 * See SPEC_SCALE0_PERF_TELEMETRY_PANELS.md and CONTRACTS.md §5.
 */

import { isScale0AuthoritativeGenerationReady } from '../scales/scale0/state/store.js';
import '../bridge/sampler-cadence.classic.js?v=5';

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
 * @returns {{ diagnostics:boolean, wantAudit:boolean, wantLag:boolean, wantProperTime:boolean,
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
        || visible('lagrangian') || visible('telemetry-grid') || visible('thermo')
        // Spectrum renders the E/B/wave/field partition and conservation
        // drift from the audit stream. Without this explicit ownership its
        // energy card races the demand gate and can remain permanently empty.
        || visible('spectrum')
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
    // Proper-time is a sampled field aggregate, not a state diagnostic. Its
    // one collector is demanded by either consumer; opening the grid alone
    // must not rely on the Time panel's rAF loop or render Time's heavy cards.
    const wantProperTime = gravityApplicable && (visible('time') || visible('telemetry-grid'));
    const activeOwner = state?.useFluxMock ? state?.fluxMock : ctx?.bridge;
    const latticeSize = Math.max(1, Math.trunc(Number(activeOwner?.latticeSize) || 32));
    const properTimeStride = Math.max(2, Math.ceil(latticeSize / 25));
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
        wantProperTime,
        properTimeStride,
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

function exactSampleCounter(value) {
    if (value === null || value === undefined || value === '') return null;
    const n = Number(value);
    return Number.isSafeInteger(n) && n >= 0 ? n : null;
}

function demandNow(telemetryHub) {
    if (typeof telemetryHub?._demandNow === 'function') return telemetryHub._demandNow();
    return typeof performance !== 'undefined' && typeof performance.now === 'function'
        ? performance.now() : Date.now();
}

function directLagrangianCadence(telemetryHub) {
    if (telemetryHub._directLagrangianCadence) return telemetryHub._directLagrangianCadence;
    const create = globalThis.FTD_SAMPLER_CADENCE?.createBoundedReductionCadence;
    if (typeof create !== 'function') {
        throw new Error('bounded Lagrangian cadence is unavailable');
    }
    telemetryHub._directLagrangianCadence = create({
        // A synchronous direct-WASM reduction runs on the render thread. A
        // cheap lattice may therefore publish at the display budget, while its
        // measured cost still limits duty. Getter failures retry much slower.
        targetIntervalMs: 1000 / 60,
        retryIntervalMs: 125,
    });
    return telemetryHub._directLagrangianCadence;
}

/**
 * Full direct-WASM Lagrangian extraction is an O(L^3) reduction.  Unlike the
 * worker cache, it must be paced by the simulation observation clock, never by
 * the rAF/UI refresh clock or a field-data version that advances every tick.
 * A source/owner boundary and a panel opening deliberately take one immediate
 * observation; later calls retain it until the shared wall-time/duty-budget
 * cadence permits a new simulation-clock observation.
 */
function directLagrangianDue(telemetryHub, owner, state, demand, opened) {
    const diagnosticsMeta = telemetryHub.getScale0TelemetryMeta?.('diagnostics')
        ?? owner?.getScale0TelemetryGroupMeta?.('diagnostics')
        ?? null;
    const sampleTick = exactSampleCounter(
        diagnosticsMeta?.sampleTick ?? diagnosticsMeta?.tick ?? state?.fieldDataVersion,
    );
    const sourceEpoch = exactSampleCounter(
        diagnosticsMeta?.sourceEpoch ?? diagnosticsMeta?.epoch,
    );
    const previous = telemetryHub._directLagrangianSchedule;
    const cadence = directLagrangianCadence(telemetryHub);
    const now = demandNow(telemetryHub);
    const ownerBoundary = opened || !previous || previous.owner !== owner
        || previous.sourceEpoch !== sourceEpoch
        || (previous.sampleTick !== null && sampleTick < previous.sampleTick);
    if (ownerBoundary) cadence.reset();
    const due = cadence.shouldRun(true, previous?.hasSample === true, now, sampleTick);
    if (due) {
        // Store the attempted observation too. A missing ABI/error must not
        // turn a demanded panel into one full reduction per UI frame.
        telemetryHub._directLagrangianSchedule = { owner, sourceEpoch, sampleTick, startedAt: now };
    }
    return due;
}

function finishDirectLagrangianAttempt(telemetryHub, result) {
    const schedule = telemetryHub._directLagrangianSchedule;
    if (!schedule) return;
    const finishedAt = demandNow(telemetryHub);
    directLagrangianCadence(telemetryHub).complete(
        finishedAt, schedule.sampleTick, Math.max(0, finishedAt - schedule.startedAt),
        { retry: !result },
    );
    schedule.hasSample = !!result;
    delete schedule.startedAt;
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
    const { wantAudit, wantLag, wantProperTime = false, properTimeStride = 2 } = demand;

    const fm = state.useFluxMock ? state.fluxMock : null;
    const samplerOwner = fm || ctx?.bridge;
    if (samplerOwner && typeof samplerOwner.setTelemetryMask === 'function') {
        samplerOwner.setTelemetryMask(wantAudit, wantLag, !!demand.wantGravity, wantProperTime);
    }
    samplerOwner?.replaceSamplerWants?.('proper-time-telemetry', wantProperTime
        ? [`tau@${properTimeStride}`, `lapse@${properTimeStride}`, `dbPhase@${properTimeStride}`] : []);

    // Native `getTelemetrySnapshot()` is a read-only versioned store. Ingest
    // it every UI pass so an async push received while paused is visible even
    // though fieldDataVersion has not advanced. This intentionally performs no
    // CUDA reduction and no panel-triggered WebSocket request.
    if (publishNativeTelemetryDemand(ctx, state, demand)) {
        telemetryHub.collectScale0(ctx.bridge, state.fluxMock, state.useFluxMock);
        // Native snapshots do not yet contain a proper-time group. The
        // capability is cache-only, so this marks the group unavailable rather
        // than creating a per-panel WebSocket reduction or holding old data.
        if (wantProperTime) telemetryHub.collectScale0ProperTime(
            ctx.bridge, state.fluxMock, state.useFluxMock, properTimeStride,
        );
        telemetryHub._prevWantAudit = wantAudit;
        telemetryHub._prevWantLag = wantLag;
        telemetryHub._prevWantProperTime = wantProperTime;
        return;
    }

    const ver = exactSampleCounter(state.fieldDataVersion) ?? -1;
    const verChanged = ver !== telemetryHub._lastAuditVersion;
    const properTimeVersionChanged = ver !== telemetryHub._lastProperTimeVersion;
    const openedA = wantAudit && !telemetryHub._prevWantAudit;
    const openedL = wantLag && !telemetryHub._prevWantLag;
    // A paused worker can finish an asynchronous telemetry request without a
    // physics-data version change. Its getters only read completed caches;
    // ingest them on UI passes while demanded, preserving their own group
    // versions/timestamps. Direct WASM getters still perform reductions and
    // retain the existing field-version gate below.
    const workerCache = samplerOwner?.isWorker === true;
    const openedP = wantProperTime && !telemetryHub._prevWantProperTime;

    if (wantAudit && (workerCache || verChanged || openedA)) {
        telemetryHub.collectScale0Audit(ctx.bridge, state.fluxMock, state.useFluxMock);
    }
    const directLagDue = wantLag && !workerCache && directLagrangianDue(
        telemetryHub, samplerOwner, state, demand, openedL,
    );
    if (wantLag && (workerCache || directLagDue)) {
        const result = telemetryHub.collectScale0Lagrangian(
            ctx.bridge, state.fluxMock, state.useFluxMock,
        );
        if (directLagDue) finishDirectLagrangianAttempt(telemetryHub, result);
    }
    if (wantProperTime && (workerCache || properTimeVersionChanged || openedP)) {
        telemetryHub.collectScale0ProperTime(
            ctx.bridge, state.fluxMock, state.useFluxMock, properTimeStride,
        );
    }
    if (wantAudit || wantLag) telemetryHub._lastAuditVersion = ver;
    if (wantProperTime) telemetryHub._lastProperTimeVersion = ver;
    if (!wantLag) {
        telemetryHub._directLagrangianSchedule = null;
        telemetryHub._directLagrangianCadence?.reset();
    }
    telemetryHub._prevWantAudit = wantAudit;
    telemetryHub._prevWantLag = wantLag;
    telemetryHub._prevWantProperTime = wantProperTime;
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
        wantProperTime: gravityReady,
        properTimeStride: Math.max(2, Math.ceil(Math.max(1,
            Math.trunc(Number((state.useFluxMock ? state.fluxMock : ctx?.bridge)?.latticeSize) || 32)) / 25)),
        everyTicks: { diagnostics: 1, audit: 1, gravity: 1, lagrangian: 1 },
    };
    // The rollback path must restore the worker transport mask as well as read
    // the streams. Reading a proxy whose default mask remains false only
    // returns null/stale audit state, which made "always collect" behave like
    // demand gating for every worker-owned scenario.
    const fm = state.useFluxMock ? state.fluxMock : null;
    const samplerOwner = fm || ctx?.bridge;
    samplerOwner?.setTelemetryMask?.(true, true, gravityReady, gravityReady);
    samplerOwner?.replaceSamplerWants?.('proper-time-telemetry', gravityReady
        ? [`tau@${demand.properTimeStride}`, `lapse@${demand.properTimeStride}`,
            `dbPhase@${demand.properTimeStride}`] : []);
    if (publishNativeTelemetryDemand(ctx, state, demand)) {
        telemetryHub.collectScale0(ctx.bridge, state.fluxMock, state.useFluxMock);
        if (gravityReady) telemetryHub.collectScale0ProperTime(
            ctx.bridge, state.fluxMock, state.useFluxMock, demand.properTimeStride,
        );
        return;
    }
    telemetryHub.collectScale0Audit(ctx.bridge, state.fluxMock, state.useFluxMock);
    telemetryHub.collectScale0Lagrangian(ctx.bridge, state.fluxMock, state.useFluxMock);
    if (gravityReady) telemetryHub.collectScale0ProperTime(
        ctx.bridge, state.fluxMock, state.useFluxMock, demand.properTimeStride,
    );
}
