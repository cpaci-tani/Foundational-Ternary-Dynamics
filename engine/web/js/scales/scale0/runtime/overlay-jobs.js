/** Pooled overlay job planning and dispatch, independent of frame-loop scheduling. */
import { getActiveScale0Capability, getActiveScale0Bridge, isKnotTrackingActive } from '../state/store.js';
import { renderNativeTransportLegend, readNativeTransportThreshold } from '../ui/overlays/native-transport-legend.js';
import { getFieldLineKnotTracker, forEachKnotTracker } from './field-line-knots.js';
import { commonSampleProvenance, safeCounterNumber } from '../../../lib/exact-counter.js';
import { observeLatticeFields, fieldObservationProvenance } from './fluid-observation.js';
import { appRegistry } from '../../../core/registry.js';
import { isPanelLive } from '../../../ui/panels/panel-visibility.js';
import { wantsStreamlineApply, wantsStreamlineJob } from './knot-streamline-plan.js';
import { eFieldLineSeeds, bFieldLineSeeds, fluxLineSeeds } from './overlay-streamline-seeds.js';
import { buildForceOverlayData, forceItemFlowPlan, applyForceFieldsJob, findForceItem, forceTypeEnabled, FLOW_TYPES } from './overlay-force-data.js';
import { buildDerivedSubstrateData, applyDerivedJob, SCALAR_JOBS } from './overlay-derived-data.js';
import {
    submitStreamlineJob, commitKnotRecord, discardKnotRecord,
} from './overlay-worker-client.js';

export const COST_STREAMLINE = 50;   // E / B / flux / each force-flow field
const COST_FORCE_FIELD = 25;  // a force arrow/heatmap/glyph field (sampler + O(count))
const COST_DERIVED = 20;      // dual-substrate / chirality / mock-derived overlays
const COST_SCALAR = 12;       // a Tier-1/2/3 scalar topology sheet (one O(count) pass)
const COST_PASSTHROUGH = 4;   // poynting / divField / light — forward a sampled buffer

// ── Allocation-free job model (web/engine-optimization-2026-05-31) ───────
// The scheduler MUST NOT allocate per sweep or per frame in steady state, or
// the GC pause rate regresses. The original design built a fresh `jobs` array
// of object-literals-with-closures every sweep (a sweep starts each throttle
// boundary — every few frames under live physics), plus a fresh SCALAR_JOBS
// table of 16 arrow closures, plus per-run `{ ...params }` spreads and per-run
// `{ key: frame }` objects. All of that is converted to PERSISTENT REUSED
// state here, mirroring the codebase's grow-in-place scratch pattern
// (cf. fillFieldParticleBuf / state.weakVectors): a fixed pool of mutable job
// slots lives on `sched`, `rebuildOverlayJobs` refills it IN PLACE (no new
// array, no new slot objects — it mutates slot fields by index), and a single
// module-level `runJob` dispatches on an integer `kind` instead of a per-job
// closure. The per-sweep context the old closures captured (ctx, state,
// viewportAdapter, latticeSize, params, capabilities, acScale0) is stashed on
// `sched` once at sweep start so `runJob` reaches it without closing over it.

// Job-kind discriminants for the closure-free dispatcher. Each maps 1:1 to one
// of the old job closures; the scheduling semantics (cost weights, ordering,
// one-streamline-per-frame, last-flow dash latch) are unchanged.
const JOB_EFIELD = 0;       // E-field streamline (COST_STREAMLINE)
const JOB_BFIELD = 1;       // B-field streamline (COST_STREAMLINE)
const JOB_FLUX = 2;         // flux streamline (COST_STREAMLINE)
const JOB_PASS = 3;         // poynting / divField passthrough (COST_PASSTHROUGH)
const JOB_FORCE_FIELDS = 4; // force sample + non-flow apply (n·COST_FORCE_FIELD)
const JOB_FORCE_FLOW = 5;   // one force-flow streamline (COST_STREAMLINE)
const JOB_DERIVED = 6;      // derived substrate group (COST_DERIVED)
const JOB_SCALAR = 7;       // one scalar/topology sheet (COST_SCALAR)
const JOB_FLUID_OBSERVATION = 8; // summaries from the same active-owner sample cache
const JOB_NATIVE_TRANSPORT = 9;  // lattice-link energy current + native knots (COST_PASSTHROUGH)
export const fluidPanelLive = () => typeof document !== 'undefined' && isPanelLive(document.getElementById('panel-fluid'));

export function ensureOverlaySched(state) {
    if (!state.overlaySched) {
        state.overlaySched = {
            // Sweep liveness: `active` replaces the old `jobs !== null` sentinel
            // so the pooled `jobs` array can persist across sweeps (its slots are
            // reused; `jobCount` is the live length for the current sweep).
            active: false,     // a sweep is in flight (false = idle)
            jobs: [],          // PERSISTENT pool of reusable job slots (grown, never re-created)
            jobCount: 0,       // number of live slots in the current sweep
            cursor: 0,         // index of the next job to run in this sweep
            sampled: null,     // field snapshot shared by every job in the sweep
            sampleCache: null, // lazy per-kind sampler cache for this sweep
            running: false,    // ctx.running latched at sweep start (for sub-anims)
            sweepFrames: 0,    // frames elapsed in the current sweep
            forceAnimated: false, // force-streamline dash advanced once per sweep
            lastVersion: -1,   // fieldDataVersion sampled at the last sweep start
            forceFrame: null,  // force-fields-job output (read by flow jobs)
            flowTypes: [],     // PERSISTENT scratch for active force-flow types
            streamlineWorkerClient: null,
            loadGeneration: 0,
            flowLineSettingsVersion: 0,
            // Per-sweep context the closure-free dispatcher reads in place of a
            // captured closure. All stable for the sweep's duration; set once at
            // sweep start in buildOverlayJobs.
            ctx: null,
            state: null,
            viewportAdapter: null,
            latticeSize: 0,
            params: null,
            fieldCapability: null,
            acScale0: null,
        };
    }
    return state.overlaySched;
}

// Acquire job slot at `index` from the persistent pool, growing the pool by one
// reusable slot object only when the pool has never been that long (a one-time
// amortized allocation that stops once the pool reaches its high-water mark;
// zero allocation on every subsequent sweep). Returns the slot so the caller
// mutates its fields in place.
function jobSlot(sched, index) {
    let slot = sched.jobs[index];
    if (slot === undefined) {
        slot = {
            kind: -1,
            cost: 0,
            scalarIndex: -1,
            flowType: '',
            isLastFlow: false,
            phase: 0,
            workerResult: null,
            requestId: 0,
            workerClient: null,
            sampleTick: 0,
            sampleSource: null,
            sampleSourceEpoch: null,
            sampleEpoch: null,
            sampleStateVersion: null,
            sampleSnapshotVersion: null,
            mutationEpoch: 0,
            knotTracker: null,
            knotConfig: null,
            lines: null,
            maxFlux: 0,
            mags: null,
        };
        sched.jobs[index] = slot;
    }
    return slot;
}

// ── Closure-free job dispatcher ─────────────────────────────────────────
// Single module-level function that executes one pooled job slot. Replaces the
// per-job `run()` closures the old buildOverlayJobs allocated every sweep: the
// slot carries only plain-data fields (kind + payload), and every value the old
// closure captured is read from `sched` (set once per sweep). The body of each
// case is the exact same build+apply the corresponding closure performed, in
// the same order, so visual output is byte-identical.
function measureKnotContributions(tr, sched) {
    const { sampleCache, sampled, latticeSize, params, state } = sched;
    sampleCache.ensureSamples(['eField', 'bField', 'fluxVector', 'divergence']);
    const provenance = commonSampleProvenance([sampled.eField, sampled.bField,
        sampled.fluxVector, sampled.divergence]);
    if (!provenance || safeCounterNumber(provenance.sampleTick) === null) {
        tr.invalidateContributions('sample-provenance-unavailable');
        return;
    }
    if (!tr.hasMatchingWorkerProvenance(provenance)) {
        // A contribution must never join a newer field cache to zones/IDs from
        // an older worker candidate. Wait for the matching tracker snapshot.
        tr.invalidateContributions('knot-zone-provenance-mismatch');
        return;
    }
    tr.measureContributions({
        eField: sampled.eField,
        bField: sampled.bField,
        fluxField: sampled.fluxVector,
        divJ: sampled.divergence,
        latticeSize,
        sampleStride: params.stride,
        tick: safeCounterNumber(provenance.sampleTick),
    });
}

function sampleProvenance(sched, slot, sample, tracker, field, includeLineIds) {
    const meta = sample?.provenance ?? sample ?? {};
    const config = tracker.getWorkerRecordConfig();
    const provenance = {
        field,
        loadGeneration: Number(sched.ctx?._loadGeneration || 0),
        mutationEpoch: Number(sched.state?.mutationEpoch || 0),
        source: meta.source ?? sample?.source ?? sample?.backend ?? null,
        nativeInstanceId: meta.nativeInstanceId ?? sample?.nativeInstanceId ?? null,
        sourceEpoch: meta.sourceEpoch ?? sample?.sourceEpoch ?? null,
        epoch: meta.epoch ?? sample?.epoch ?? null,
        stateVersion: meta.stateVersion ?? sample?.stateVersion ?? null,
        snapshotVersion: meta.snapshotVersion ?? sample?.snapshotVersion ?? null,
        sampleTick: sample?.sampleTick ?? meta.sampleTick ?? null,
        latticeSize: sched.latticeSize,
        resetVersion: config.resetVersion,
    };
    slot.sampleTick = provenance.sampleTick;
    slot.sampleSource = provenance.source;
    slot.sampleSourceEpoch = provenance.sourceEpoch;
    slot.sampleEpoch = provenance.epoch;
    slot.sampleStateVersion = provenance.stateVersion;
    slot.sampleSnapshotVersion = provenance.snapshotVersion;
    slot.mutationEpoch = provenance.mutationEpoch;
    slot.knotTracker = tracker;
    slot.knotConfig = { provenance, sensitivity: config.sensitivity, perKnotColor: config.perKnotColor, includeLineIds };
    return slot.knotConfig;
}

function sameKnotProvenance(left, right) {
    if (!left || !right) return false;
    for (const key of ['field', 'loadGeneration', 'mutationEpoch', 'source', 'nativeInstanceId', 'sourceEpoch', 'epoch',
        'stateVersion', 'snapshotVersion', 'sampleTick', 'latticeSize', 'resetVersion']) {
        if (left[key] !== right[key]) return false;
    }
    return true;
}

function knotRequestIsCurrent(sched, slot, record) {
    const provenance = slot.knotConfig?.provenance;
    if (!sameKnotProvenance(provenance, record?.provenance)) return false;
    if (Number(sched.ctx?._loadGeneration || 0) !== provenance.loadGeneration
        || Number(sched.state?.mutationEpoch || 0) !== provenance.mutationEpoch) return false;
    return slot.knotTracker?.getWorkerRecordConfig().resetVersion === provenance.resetVersion;
}

function finishKnotRecord(sched, slot) {
    const result = slot.workerResult;
    if (!result) return null;
    const tracker = slot.knotTracker;
    if (!tracker) return { accepted: false, lineIds: null };
    if (sched.streamlineWorkerClient !== slot.workerClient || slot.workerClient?.failed) {
        tracker.invalidateWorkerRecord('streamline-worker-lost-before-knot-commit', slot.knotConfig?.provenance);
        return { accepted: false, lineIds: null };
    }
    if (result.error || !result.knot) {
        discardKnotRecord(sched, slot.requestId, slot.workerClient);
        tracker.invalidateWorkerRecord(result.error || 'worker-knot-observation-unavailable', slot.knotConfig?.provenance);
        return { accepted: false, lineIds: null };
    }
    const candidateId = slot.requestId;
    if (!knotRequestIsCurrent(sched, slot, result.knot) || !tracker.applyWorkerRecord(result.knot)) {
        discardKnotRecord(sched, candidateId, slot.workerClient);
        return { accepted: false, lineIds: null };
    }
    // commit follows adoption on the same message port, before the scheduler
    // can submit the next tracker request. A rejected/late candidate never
    // advances worker genealogy.
    commitKnotRecord(sched, candidateId, slot.workerClient);
    return { accepted: true, lineIds: result.knot.lineIds ?? null };
}

function directFallbackKnotRecord(slot, lines, sample) {
    const tracker = slot.knotTracker;
    tracker.record(lines, sample, slot.knotConfig.provenance.sampleTick, slot.knotConfig.provenance.latticeSize);
    tracker.adoptDirectRecordProvenance(slot.knotConfig.provenance);
    // This branch is restricted to no-Worker embedded/headless environments.
    // It retains the pre-offload public implementation for those runtimes.
    return tracker.getPerKnotColor() ? tracker.assignLinesToKnots(lines) : null;
}

function invalidateKnotWorkerNamespace(message) {
    forEachKnotTracker((tracker) => {
        tracker.reset();
        tracker.invalidateWorkerRecord(`streamline-worker-failed: ${message}`);
    });
}

// The link energy observer costs one 18-neighbour pass per engine tick, so it
// runs only while Native transport is shown. Observation-only on every owner.
export function syncNativeTransportObservation(ctx, state, sched) {
    const want = !!state.fieldFlags.showNativeTransport;
    if (sched.nativeObservation === want) return;
    if (!want) renderNativeTransportLegend(null);
    const owner = getActiveScale0Bridge(ctx, state);
    if (typeof owner?.setLinkEnergyObservation === 'function') owner.setLinkEnergyObservation(want);
    sched.nativeObservation = want;
    if (want) sched.nativeKnotSettled = null;  // re-assert native knots once when the overlay is switched on
}

export function runJob(sched, slot) {
    const { ctx, state, viewportAdapter, latticeSize, params, sampled, sampleCache } = sched;
    const { stride } = params;
    const knotTrackingActive = isKnotTrackingActive(state);
    switch (slot.kind) {
        case JOB_EFIELD: {
            if (slot.phase === 0) {
                sampleCache.ensureSample('eField');
                sampleCache.ensureParticleData();
                if (!sampled.eField?.count) return true;
                if (knotTrackingActive) sampleProvenance(sched, slot, sampled.eField, getFieldLineKnotTracker('e'), 'e', true);
                else { slot.knotTracker = null; slot.knotConfig = null; }
                submitStreamlineJob(
                    sched,
                    slot,
                    'e',
                    sampled.eField,
                    eFieldLineSeeds(sched.acScale0, state, sampled, latticeSize, params),
                    {
                        N: latticeSize,
                        stride,
                        maxSteps: params.maxSteps,
                        stepSize: params.stepSize,
                        maxLines: params.maxLines,
                        bidirectional: true,
                    },
                    slot.knotConfig,
                );
                slot.sampleTick = sampled.eField.sampleTick ?? null;
                slot.phase = 1;
                return false;
            }
            if (!slot.workerResult) return false;
            if (slot.workerResult.error) {
                discardKnotRecord(sched, slot.requestId, slot.workerClient);
                slot.knotTracker?.invalidateWorkerRecord(slot.workerResult.error, slot.knotConfig?.provenance);
                console.error('[Scale0] E streamline job failed:', slot.workerResult.error);
                return true;
            }
            const lines = slot.workerResult.lines;
            if (!knotTrackingActive || !slot.knotTracker) {
                discardKnotRecord(sched, slot.requestId, slot.workerClient);
                if (wantsStreamlineApply(state.fieldFlags, 'e')) viewportAdapter.applyEFieldLines(lines, null);
                return true;
            }
            const recorded = slot.workerResult.localFallback
                ? { accepted: true, lineIds: directFallbackKnotRecord(slot, lines, sampled.eField) }
                : finishKnotRecord(sched, slot);
            const coloring = recorded.accepted && slot.knotTracker.getPerKnotColor() && recorded.lineIds
                ? { lineIds: recorded.lineIds, selectedId: slot.knotTracker.getSelected(), perKnotColor: true } : null;
            if (recorded.accepted && slot.knotTracker.isContribEnabled()) measureKnotContributions(slot.knotTracker, sched);
            if (wantsStreamlineApply(state.fieldFlags, 'e')) viewportAdapter.applyEFieldLines(lines, coloring);
            return true;
        }
        case JOB_BFIELD: {
            if (slot.phase === 0) {
                sampleCache.ensureSample('bField');
                sampleCache.ensureParticleData();
                if (!sampled.bField?.count) return true;
                if (knotTrackingActive) sampleProvenance(sched, slot, sampled.bField, getFieldLineKnotTracker('b'), 'b', true);
                else { slot.knotTracker = null; slot.knotConfig = null; }
                submitStreamlineJob(
                    sched,
                    slot,
                    'b',
                    sampled.bField,
                    bFieldLineSeeds(sched.acScale0, state, sampled, latticeSize, params),
                    {
                        N: latticeSize,
                        stride,
                        maxSteps: Math.ceil(params.maxSteps * 1.5),
                        stepSize: params.stepSize,
                        maxLines: params.maxLines,
                        bidirectional: true,
                    },
                    slot.knotConfig,
                );
                slot.sampleTick = sampled.bField.sampleTick ?? null;
                slot.phase = 1;
                return false;
            }
            if (!slot.workerResult) return false;
            if (slot.workerResult.error) {
                discardKnotRecord(sched, slot.requestId, slot.workerClient);
                slot.knotTracker?.invalidateWorkerRecord(slot.workerResult.error, slot.knotConfig?.provenance);
                console.error('[Scale0] B streamline job failed:', slot.workerResult.error);
                return true;
            }
            const lines = slot.workerResult.lines;
            if (!knotTrackingActive || !slot.knotTracker) {
                discardKnotRecord(sched, slot.requestId, slot.workerClient);
                if (wantsStreamlineApply(state.fieldFlags, 'b')) viewportAdapter.applyBFieldLines(lines, null);
                return true;
            }
            const recorded = slot.workerResult.localFallback
                ? { accepted: true, lineIds: directFallbackKnotRecord(slot, lines, sampled.bField) }
                : finishKnotRecord(sched, slot);
            const coloring = recorded.accepted && slot.knotTracker.getPerKnotColor() && recorded.lineIds
                ? { lineIds: recorded.lineIds, selectedId: slot.knotTracker.getSelected(), perKnotColor: true } : null;
            if (recorded.accepted && slot.knotTracker.isContribEnabled()) measureKnotContributions(slot.knotTracker, sched);
            if (wantsStreamlineApply(state.fieldFlags, 'b')) viewportAdapter.applyBFieldLines(lines, coloring);
            return true;
        }
        case JOB_FLUX: {
            if (slot.phase === 0) {
                sampleCache.ensureSample('fluxVector');
                if (!sampled.fluxVector?.count) return true;
                if (knotTrackingActive) sampleProvenance(sched, slot, sampled.fluxVector, getFieldLineKnotTracker('flux'), 'flux', false);
                else { slot.knotTracker = null; slot.knotConfig = null; }
                submitStreamlineJob(
                    sched,
                    slot,
                    'flux',
                    sampled.fluxVector,
                    fluxLineSeeds(state, sampled, latticeSize, params),
                    {
                        N: latticeSize,
                        stride,
                        maxSteps: params.maxSteps,
                        stepSize: params.stepSize,
                        maxLines: params.maxLines,
                        bidirectional: true,
                    },
                    slot.knotConfig,
                );
                slot.sampleTick = sampled.fluxVector.sampleTick ?? null;
                slot.phase = 1;
                return false;
            }
            if (!slot.workerResult) return false;
            if (slot.workerResult.error) {
                discardKnotRecord(sched, slot.requestId, slot.workerClient);
                slot.knotTracker?.invalidateWorkerRecord(slot.workerResult.error, slot.knotConfig?.provenance);
                console.error('[Scale0] Flux streamline job failed:', slot.workerResult.error);
                return true;
            }
            const fs = slot.workerResult;
            if (!knotTrackingActive || !slot.knotTracker) {
                discardKnotRecord(sched, slot.requestId, slot.workerClient);
                if (wantsStreamlineApply(state.fieldFlags, 'flux')) viewportAdapter.applyFluxStreamlines(fs.lines, fs.maxFlux, fs.mags);
                return true;
            }
            const recorded = fs.localFallback
                ? { accepted: true, lineIds: directFallbackKnotRecord(slot, fs.lines, sampled.fluxVector) }
                : finishKnotRecord(sched, slot);
            if (recorded.accepted && slot.knotTracker.isContribEnabled()) measureKnotContributions(slot.knotTracker, sched);
            if (wantsStreamlineApply(state.fieldFlags, 'flux')) viewportAdapter.applyFluxStreamlines(fs.lines, fs.maxFlux, fs.mags);
            return true;
        }
        case JOB_PASS: {
            const flags = state.fieldFlags;
            if (flags.showPoynting) {
                sampleCache.ensureSample('poynting');
                if (sampled.poynting?.count > 0) viewportAdapter.applyPoynting(sampled.poynting);
            }
            if (flags.showDivField) {
                sampleCache.ensureSample('divergence');
                if (sampled.divergence?.count > 0) viewportAdapter.applyDivergence(sampled.divergence);
            }
            break;
        }
        case JOB_NATIVE_TRANSPORT: {
            const owner = getActiveScale0Bridge(ctx, state);
            if (typeof owner?.getLinkEnergyCurrent !== 'function') {
                renderNativeTransportLegend(viewportAdapter.applyNativeTransport({
                    sample: { status: 'unavailable', reason: 'this engine connection does not provide the link energy observer' },
                    knots: null,
                }));
                break;
            }
            const linkSample = sampleCache.ensureSample('linkEnergy');
            // A scenario load rebuilds the engine bridge and drops the observation
            // switch; the engine then reports 'off'. Re-send it (observation-only).
            if (!linkSample || linkSample.status === 'off') owner?.setLinkEnergyObservation?.(true);
            // Native knots are the engine's own manifested-cluster tracker, which every
            // scenario setup switches off. Re-assert it once per scenario load (and once
            // when this overlay is switched on), then respect the user's Knot Tracking
            // checkbox until the next load.
            const loadGeneration = Number(ctx?._loadGeneration || 0);
            if (sched.nativeKnotSettled !== loadGeneration) {
                const trackerOn = typeof owner?.getEngineTruthToggle === 'function'
                    ? owner.getEngineTruthToggle('knot_tracking')
                    : owner?.getToggle?.('knot_tracking');
                if (trackerOn === false) owner?.setToggle?.('knot_tracking', true);
                else if (trackerOn === true) sched.nativeKnotSettled = loadGeneration;
            }
            const knots = owner?.getKnotTelemetry?.() ?? null;
            const summary = viewportAdapter.applyNativeTransport({ sample: linkSample, knots, fraction: readNativeTransportThreshold() });
            renderNativeTransportLegend(summary);
            break;
        }
        case JOB_FORCE_FIELDS: {
            // params.deferFlow was set true once for this sweep (see
            // buildOverlayJobs), so the heavy flow integration is deferred out
            // of this fields job into the per-force JOB_FORCE_FLOW jobs.
            if (state.fieldFlags.showForceWeak) sampleCache.ensureSample('curlJ');
            sched.forceFrame = buildForceOverlayData(
                state, sched.fieldCapability, sampled, latticeSize, stride,
                params.stepsScale, params.seedSpacing, params, sched.forceCache);
            applyForceFieldsJob(sched, viewportAdapter);
            break;
        }
        case JOB_FORCE_FLOW: {
            const ff = sched.forceFrame;
            const item = findForceItem(ff?.items, slot.flowType);
            if (slot.phase === 0) {
                // Current UI truth wins over the immutable sweep snapshot. This
                // prevents a late async result from repainting a style/type the
                // user disabled while the worker was integrating.
                if (state.forceStyle !== 'flow' || !forceTypeEnabled(state, slot.flowType) || !item) {
                    viewportAdapter.clearForceVisualization(slot.flowType, 'flow');
                    return true;
                }
                const plan = forceItemFlowPlan(item, latticeSize, stride, params, state);
                submitStreamlineJob(
                    sched, slot, `force-${item.type}`, item.data, plan.seeds, plan.opts,
                );
                slot.phase = 1;
                return false;
            }
            if (!slot.workerResult) return false;
            if (slot.workerResult.error) {
                viewportAdapter.clearForceVisualization(slot.flowType, 'flow');
                console.error(`[Scale0] ${slot.flowType} force-flow job failed:`, slot.workerResult.error);
                return true;
            }
            if (state.forceStyle === 'flow' && forceTypeEnabled(state, slot.flowType)) {
                viewportAdapter.applyForceStreamlines(slot.workerResult.lines, slot.flowType);
            } else {
                viewportAdapter.clearForceVisualization(slot.flowType, 'flow');
            }
            // Advance the dash-offset animation exactly ONCE per sweep, on the
            // last flow job, only while running — matching the pre-amortization
            // cadence (one advance per overlay refresh).
            if (slot.isLastFlow && sched.running && !sched.forceAnimated) {
                viewportAdapter.animateForceStreamlines(0.016);
                sched.forceAnimated = true;
            }
            break;
        }
        case JOB_DERIVED: {
            const flags = state.fieldFlags;
            if (flags.showDualSubstrate || flags.showChirality) sampleCache.ensureSample('fluxVector');
            const frame = buildDerivedSubstrateData(state, sampled, sched.fieldCapability, sched.latticeSize);
            applyDerivedJob(frame, viewportAdapter);
            break;
        }
        case JOB_SCALAR: {
            // compute returns the bare frame value (NOT a `{ key: value }`
            // wrapper) and apply forwards it directly — eliminating the per-run
            // object the old `(s) => ({ key: ... })` closures boxed every frame.
            const entry = SCALAR_JOBS[slot.scalarIndex];
            sampleCache.ensureScalarDeps(entry[0]);
            if (state.scalarRenderMode === 'volume' && entry[0] === 'showEmEnergy'
                && !fieldObservationProvenance([sampled.eField, sampled.bField], getActiveScale0Bridge(ctx, state))) {
                viewportAdapter.applyEmEnergy(null); break;
            }
            const value = entry[1](sampled, ctx, state);
            entry[2](viewportAdapter, value);
            break;
        }
        case JOB_FLUID_OBSERVATION: {
            if (!fluidPanelLive()) break;
            sampleCache.ensureSamples(['eField', 'bField', 'fluxVector', 'poynting']);
            const value = observeLatticeFields(sampled, getActiveScale0Bridge(ctx, state), state, stride);
            const panel = appRegistry.get('panel:fluid');
            if (value) panel?.update(value);
            else panel?.clear('Waiting for field samples from the same lattice tick.');
            break;
        }
    }
    return true;
}

// Refill the persistent job pool IN PLACE for one sweep. Mutates pre-allocated
// slot objects by index (sched.jobs / jobSlot) and sets sched.jobCount — it
// allocates no new array and (after the pool reaches its high-water mark) no
// new slot objects, so a steady-state sweep is allocation-free. The ordering
// and per-job cost weights are identical to the old closure-building version,
// so the scheduling semantics (one streamline/frame, budget packing, last-flow
// latch) are unchanged. The per-sweep context every job needs is stashed on
// `sched` here, once, in place of the closures' captured variables.
export function buildOverlayJobs(ctx, state, sched, viewportAdapter, latticeSize, params) {
    const fieldCapability = getActiveScale0Bridge(ctx, state)?.capabilities?.scale0
        ?? ctx.bridge.capabilities.scale0;
    const flags = state.fieldFlags;
    const knotTrackingActive = isKnotTrackingActive(state);
    const acScale0 = (getActiveScale0Capability(ctx, state) ?? ctx.bridge.capabilities.scale0);

    // Stash the sweep context the closure-free dispatcher reads (replaces the
    // old per-closure captured variables). All stable for the sweep duration.
    sched.ctx = ctx;
    sched.state = state;
    sched.viewportAdapter = viewportAdapter;
    sched.latticeSize = latticeSize;
    sched.params = params;
    sched.fieldCapability = fieldCapability;
    sched.acScale0 = acScale0;
    sched.onStreamlineWorkerFailure = invalidateKnotWorkerNamespace;
    // The force-fields job builds with flow deferred. Set the flag once on the
    // sweep params object instead of spreading `{ ...params, deferFlow: true }`
    // per run (an allocation). `deferFlow` is read only by buildForceOverlayData;
    // the streamline builders ignore the extra key, so this is inert for them.
    params.deferFlow = true;

    let n = 0; // running job count; jobSlot(sched, n) reuses the pooled slot

    // ── EM streamline overlays — E, B, flux each as an INDEPENDENT job ────
    // These three are the heaviest work (fresh spatial index + bidirectional
    // RK4). Each becomes an independent visualization-worker transaction. The
    // scheduler waits for that transaction before advancing, then applies one
    // complete atomic line set; no partial geometry or UI-thread integration is
    // exposed.
    // Job planning: visual flags OR knot tracking. Tracking rebuilds E/B/flux
    // streamlines so clumps can be recorded without drawing the lines.
    // Each job still applies to the viewport only when its visual flag is on.
    if (wantsStreamlineJob(flags, knotTrackingActive, 'e')) {
        const slot = jobSlot(sched, n++); slot.kind = JOB_EFIELD; slot.cost = COST_STREAMLINE; slot.phase = 0;
    }
    if (wantsStreamlineJob(flags, knotTrackingActive, 'b')) {
        const slot = jobSlot(sched, n++); slot.kind = JOB_BFIELD; slot.cost = COST_STREAMLINE; slot.phase = 0;
    }
    if (wantsStreamlineJob(flags, knotTrackingActive, 'flux')) {
        const slot = jobSlot(sched, n++); slot.kind = JOB_FLUX; slot.cost = COST_STREAMLINE; slot.phase = 0; slot.workerResult = null;
    }
    // Poynting / divergence are zero-cost passthroughs (forward a sampled
    // buffer); batch them as one cheap job.
    if (flags.showPoynting || flags.showDivField) {
        const slot = jobSlot(sched, n++); slot.kind = JOB_PASS; slot.cost = COST_PASSTHROUGH;
    }
    if (flags.showNativeTransport) {
        const slot = jobSlot(sched, n++); slot.kind = JOB_NATIVE_TRANSPORT; slot.cost = COST_PASSTHROUGH;
    }

    // ── Force group ──────────────────────────────────────────────────────
    // One job samples + builds the force fields and applies the non-flow style
    // (arrows / heatmap / glyphs — all cheap). When the style is 'flow', the
    // heavy part is up to four extra full streamline integrations; those are
    // split into ONE job per force so the budget admits a single force-flow
    // integration per frame, exactly as for E/B/flux. This is what stops the
    // 4-force flow configuration from re-stacking a multi-streamline spike.
    const anyForceOn = flags.showForceEM || flags.showForceGravity ||
        flags.showForceStrong || flags.showForceWeak;
    if (anyForceOn) {
        const isFlow = state.forceStyle === 'flow';
        let activeForces = 0;
        if (flags.showForceEM) activeForces++;
        if (flags.showForceGravity) activeForces++;
        if (flags.showForceStrong) activeForces++;
        if (flags.showForceWeak) activeForces++;
        // Fields job: sample all forces, apply non-flow style. Cost is the
        // sampler + O(count) passes; flow integration is deferred out of it.
        const fSlot = jobSlot(sched, n++);
        fSlot.kind = JOB_FORCE_FIELDS;
        fSlot.cost = activeForces * COST_FORCE_FIELD;
        if (isFlow) {
            // One flow job per active force type. Each looks its item up in the
            // fields-job output (built on an earlier frame of this sweep) and
            // computes+applies just that force's streamlines. The last one to
            // run advances the dash animation once (forceAnimated latch). Filter
            // the static FLOW_TYPES into the persistent flowTypes scratch in
            // place (no new array), then emit one pooled slot per active type.
            const flowTypes = sched.flowTypes;
            flowTypes.length = 0;
            for (let i = 0; i < FLOW_TYPES.length; i++) {
                if (flags[FLOW_TYPES[i][0]]) flowTypes.push(FLOW_TYPES[i][1]);
            }
            for (let i = 0; i < flowTypes.length; i++) {
                const slot = jobSlot(sched, n++);
                slot.kind = JOB_FORCE_FLOW;
                slot.cost = COST_STREAMLINE;
                slot.flowType = flowTypes[i];
                slot.isLastFlow = i === flowTypes.length - 1;
                slot.phase = 0;
                slot.workerResult = null;
                slot.requestId = 0;
            }
        }
    }

    // ── Derived substrate group (dual / chirality / mock overlays) ─
    const derivedActive = flags.showDarkMatterHalo || flags.showDampingZones || flags.showKnotZones ||
        flags.showGenesisIsosurface || flags.showDualSubstrate ||
        flags.showChirality;
    if (derivedActive) {
        const slot = jobSlot(sched, n++); slot.kind = JOB_DERIVED; slot.cost = COST_DERIVED;
    }

    // ── Quantum / topology scalar sheets ─────────────────────────────────
    // Each is a single O(count) pass; cheap individually but there are up to
    // ~15 of them. Emit ONE job per active scalar so the budget can pack as
    // many as fit per frame and defer the rest — this is what unstacks the
    // "all 33 overlays" spike. The scalar table (SCALAR_JOBS) is module-scope
    // (allocated once); a scalar job stores only its row index, and runJob
    // calls that row's compute+apply with no per-job allocation. Buffer reuse
    // (overlay-frames ensureTier1Buffers and the per-overlay state caches) is
    // unaffected: a job still computes its full frame in one shot, it just may
    // run a frame later.
    for (let i = 0; i < SCALAR_JOBS.length; i++) {
        if (!flags[SCALAR_JOBS[i][0]]) continue;
        const slot = jobSlot(sched, n++);
        slot.kind = JOB_SCALAR;
        slot.cost = COST_SCALAR;
        slot.scalarIndex = i;
    }

    if (fluidPanelLive()) {
        const slot = jobSlot(sched, n++); slot.kind = JOB_FLUID_OBSERVATION; slot.cost = COST_SCALAR;
    }
    sched.jobCount = n;
}
