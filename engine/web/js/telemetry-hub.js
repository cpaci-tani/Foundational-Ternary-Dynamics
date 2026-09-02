/**
 * TelemetryHub — single source of truth for all FTD simulation telemetry.
 *
 * Responsibilities:
 *  - Own all ring buffers (flux, energy, entropy, lagrangian terms, PE, AE, CS)
 *  - Centralize bridge telemetry calls for every scale (0–5, 11)
 *  - Provide derived / calculated metrics (conservation status, chirality ratio, etc.)
 *  - Expose a clean pull API for panel components and chart renderers
 *
 * Usage pattern (from a controller's animate loop):
 *   telemetryHub.collectScale0(bridge, fluxMock, useFluxMock);   // push data
 *   const diag = telemetryHub.s0.diag;                           // pull latest
 *   const series = telemetryHub.flux;                            // pull ring buffer
 *
 * Ring buffer ownership:
 *   Charts (FluxEnergyChart, ParticleChart, LagrangianChart) receive their
 *   RingBuffer instances from this hub — they do NOT allocate their own.
 *   This ensures a single write path and eliminates duplicate data stores.
 */

import { C_SPEED, G_N } from './constants.js';

// Native CUDA telemetry is published as independent group deltas.  Keep each
// group’s provenance instead of attaching one misleading "current tick" to
// every side-panel value: audit, gravity, and Lagrangian reductions can
// deliberately run less often than the inexpensive diagnostics summary.
const SCALE0_SNAPSHOT_GROUPS = Object.freeze([
    'diagnostics', 'audit', 'lagrangian', 'gravity',
]);

function hasOwn(value, key) {
    return !!value && Object.prototype.hasOwnProperty.call(value, key);
}

function telemetryNow() {
    return (typeof performance !== 'undefined' && typeof performance.now === 'function')
        ? performance.now() : Date.now();
}

// Availability is part of the scientific value, not a rendering concern.
// Keep exact numeric zero intact, but represent every absent/non-finite sample
// as NaN so tables can render an em dash and uPlot can leave a visible gap.
const unavailableSample = () => Number.NaN;
const finiteSample = (value) => (
    typeof value === 'number' && Number.isFinite(value) ? value : unavailableSample()
);
const firstFiniteSample = (...values) => {
    for (const value of values) {
        if (typeof value === 'number' && Number.isFinite(value)) return value;
    }
    return unavailableSample();
};
const finiteAbsSample = (value) => {
    const sample = finiteSample(value);
    return Number.isFinite(sample) ? Math.abs(sample) : unavailableSample();
};
const finiteDifference = (left, right) => (
    Number.isFinite(left) && Number.isFinite(right)
        ? left - right : unavailableSample()
);
const finiteMagnitude = (...components) => (
    components.length > 0 && components.every(Number.isFinite)
        ? Math.hypot(...components) : unavailableSample()
);

function scale0SampleStamp(meta) {
    const identity = meta?.stateVersion ?? meta?.tick ?? meta?.snapshotVersion;
    if (identity === null || identity === undefined) return null;
    return [
        meta?.source ?? 'unknown',
        meta?.sourceEpoch ?? meta?.epoch ?? 'local',
        identity,
    ].join(':');
}

function freshScale0State() {
    return {
        diag: null,
        audit: null,
        lagrangian: null,
        gravity: null,
        // `groups` contains the source provenance of the value currently held
        // in the matching property above. `ageMs` is deliberately a receipt
        // age, not a claim about when a staggered GPU reduction was computed.
        meta: {
            source: null,
            epoch: null,
            // Latest native source boundary observed, which may be newer than
            // an individual cached group after a mutation invalidation.
            // Group `epoch` remains the epoch at which that group was actually
            // reduced; the comparison is what makes retained values stale
            // rather than silently relabelling them as fresh.
            expectedSourceEpoch: null,
            expectedSource: null,
            snapshotVersion: null,
            tick: null,
            stale: true,
            groups: Object.fromEntries(
                SCALE0_SNAPSHOT_GROUPS.map(group => [group, null]),
            ),
        },
    };
}

// ── Ring Buffer ──────────────────────────────────────────────────────────────
// Exported so chart renderers can type-check and legacy code can import it.

export class RingBuffer {
    constructor(size = 500) {
        this.data  = new Float32Array(size);
        this.ticks = new Float64Array(size);
        this.size  = size;
        this.head  = 0;
        this.count = 0;
        this.total = 0;
    }

    _grow() {
        const oldData = this.data;
        const oldTicks = this.ticks;
        const oldSize = this.size;
        const oldCount = this.count;
        const oldHead = this.head;
        const nextSize = Math.max(2, oldSize * 2);
        const nextData = new Float32Array(nextSize);
        const nextTicks = new Float64Array(nextSize);
        const start = (oldHead - oldCount + oldSize) % oldSize;
        for (let i = 0; i < oldCount; i++) {
            const source = (start + i) % oldSize;
            nextData[i] = oldData[source];
            nextTicks[i] = oldTicks[source];
        }
        this.data = nextData;
        this.ticks = nextTicks;
        this.size = nextSize;
        this.head = oldCount;
    }

    push(value, tick = this.total) {
        if (this.count === this.size) this._grow();
        this.data[this.head] = finiteSample(value);
        this.ticks[this.head] = Number.isFinite(Number(tick)) ? Number(tick) : this.total;
        this.head = (this.head + 1) % this.size;
        if (this.count < this.size) this.count++;
        this.total++;
    }

    get(i) {
        if (i < 0 || i >= this.count) return unavailableSample();
        return this.data[(this.head - this.count + i + this.size) % this.size];
    }

    last() {
        if (this.count === 0) return unavailableSample();
        return this.data[(this.head - 1 + this.size) % this.size];
    }

    getTick(i) {
        if (i < 0 || i >= this.count) return unavailableSample();
        return this.ticks[(this.head - this.count + i + this.size) % this.size];
    }

    max() {
        let m = -Infinity;
        for (let i = 0; i < this.count; i++) { const v = this.get(i); if (v > m) m = v; }
        return m === -Infinity ? 1 : m;
    }

    min() {
        let m = Infinity;
        for (let i = 0; i < this.count; i++) { const v = this.get(i); if (v < m) m = v; }
        return m === Infinity ? 0 : m;
    }


    flattenInto(targetArray, maxSamples) {
        const n = Math.min(this.count, maxSamples || this.count, targetArray.length);
        if (n === 0) return 0;
        
        const start = (this.head - this.count + this.size) % this.size;
        const actualStart = (start + this.count - n) % this.size;

        if (actualStart + n <= this.size) {
            targetArray.set(this.data.subarray(actualStart, actualStart + n), 0);
        } else {
            const tailLen = this.size - actualStart;
            targetArray.set(this.data.subarray(actualStart, this.size), 0);
            targetArray.set(this.data.subarray(0, n - tailLen), tailLen);
        }
        return n;
    }

    flattenTicksInto(targetArray, maxSamples) {
        const n = Math.min(this.count, maxSamples || this.count, targetArray.length);
        if (n === 0) return 0;
        const start = (this.head - this.count + this.size) % this.size;
        const actualStart = (start + this.count - n) % this.size;
        if (actualStart + n <= this.size) {
            targetArray.set(this.ticks.subarray(actualStart, actualStart + n), 0);
        } else {
            const tailLen = this.size - actualStart;
            targetArray.set(this.ticks.subarray(actualStart, this.size), 0);
            targetArray.set(this.ticks.subarray(0, n - tailLen), tailLen);
        }
        return n;
    }

    clear() { this.head = 0; this.count = 0; this.total = 0; }
}

// ── Telemetry Hub ────────────────────────────────────────────────────────────


export class MultiRingBuffer {
    constructor(size, channelNames) {
        this.size = size;
        this.channels = channelNames;
        this.numChannels = channelNames.length;
        this.data = new Float32Array(size * this.numChannels);
        this.ticks = new Float64Array(size);
        this.head = 0;
        this.count = 0;
        this.total = 0;
        
        this.views = {};
        channelNames.forEach((name, i) => {
            this.views[name] = new RingBufferView(this, i);
        });
    }

    _grow() {
        const oldData = this.data;
        const oldTicks = this.ticks;
        const oldSize = this.size;
        const oldCount = this.count;
        const oldHead = this.head;
        const nextSize = Math.max(2, oldSize * 2);
        const nextData = new Float32Array(nextSize * this.numChannels);
        const nextTicks = new Float64Array(nextSize);
        const start = (oldHead - oldCount + oldSize) % oldSize;
        for (let sample = 0; sample < oldCount; sample++) {
            const source = (start + sample) % oldSize;
            nextTicks[sample] = oldTicks[source];
            for (let channel = 0; channel < this.numChannels; channel++) {
                nextData[channel * nextSize + sample] = oldData[channel * oldSize + source];
            }
        }
        this.data = nextData;
        this.ticks = nextTicks;
        this.size = nextSize;
        this.head = oldCount;
    }

    push(frame, tick = this.total) {
        if (this.count === this.size) this._grow();
        for (let i = 0; i < this.numChannels; i++) {
            const val = frame[this.channels[i]];
            // Missing, NaN, and infinity all mean "measurement unavailable".
            // Exact numeric zero remains zero. This is deliberately the same
            // contract as RingBuffer.push() and RingBufferView.setLast().
            this.data[(i * this.size) + this.head] = finiteSample(val);
        }
        this.ticks[this.head] = Number.isFinite(Number(tick)) ? Number(tick) : this.total;
        this.head = (this.head + 1) % this.size;
        if (this.count < this.size) this.count++;
        this.total++;
    }

    pushArray(frameArray, tick = this.total) {
        if (this.count === this.size) this._grow();
        // Zero-copy array push
        for (let i = 0; i < this.numChannels; i++) {
            this.data[(i * this.size) + this.head] = frameArray[i];
        }
        this.ticks[this.head] = Number.isFinite(Number(tick)) ? Number(tick) : this.total;
        this.head = (this.head + 1) % this.size;
        if (this.count < this.size) this.count++;
        this.total++;
    }
    
    clear() {
        this.head = 0;
        this.count = 0;
        this.total = 0;
    }
}

export class RingBufferView {
    constructor(parent, channelIndex) {
        this.parent = parent;
        this.channelIndex = channelIndex;
    }
    
    get count() { return this.parent.count; }
    get total() { return this.parent.total; }
    get size() { return this.parent.size; }

    get(i) {
        if (i < 0 || i >= this.parent.count) return unavailableSample();
        const pSize = this.parent.size;
        const idx = (this.parent.head - this.parent.count + i + pSize) % pSize;
        return this.parent.data[this.channelIndex * pSize + idx];
    }
    
    last() {
        if (this.parent.count === 0) return unavailableSample();
        const pSize = this.parent.size;
        const idx = (this.parent.head - 1 + pSize) % pSize;
        return this.parent.data[this.channelIndex * pSize + idx];
    }

    getTick(i) {
        if (i < 0 || i >= this.parent.count) return unavailableSample();
        const pSize = this.parent.size;
        const idx = (this.parent.head - this.parent.count + i + pSize) % pSize;
        return this.parent.ticks[idx];
    }

    // Patches this channel's value into the row the parent MultiRingBuffer
    // most recently pushed, WITHOUT advancing head. For producers that split
    // one logical sample across two collector calls (core fields pushed via
    // parent.push({...}), a second pass refining a subset of the same row) —
    // calling push() a second time would double-advance the shared ring and
    // desynchronize that row's columns from the rest. No-op before the first
    // push (nothing to patch yet).
    setLast(value) {
        if (this.parent.count === 0) return;
        const pSize = this.parent.size;
        const idx = (this.parent.head - 1 + pSize) % pSize;
        this.parent.data[this.channelIndex * pSize + idx] = finiteSample(value);
    }

    max() {
        let m = -Infinity;
        for (let i = 0; i < this.parent.count; i++) { const v = this.get(i); if (v > m) m = v; }
        return m === -Infinity ? 1 : m;
    }

    min() {
        let m = Infinity;
        for (let i = 0; i < this.parent.count; i++) { const v = this.get(i); if (v < m) m = v; }
        return m === Infinity ? 0 : m;
    }

    flattenInto(targetArray, maxSamples) {
        const pCount = this.parent.count;
        const pSize = this.parent.size;
        const n = Math.min(pCount, maxSamples || pCount, targetArray.length);
        if (n === 0) return 0;
        
        const start = (this.parent.head - pCount + pSize) % pSize;
        const actualStart = (start + pCount - n) % pSize;
        
        const data = this.parent.data;
        const offset = this.channelIndex * pSize;

        if (actualStart + n <= pSize) {
            targetArray.set(data.subarray(offset + actualStart, offset + actualStart + n), 0);
        } else {
            const tailLen = pSize - actualStart;
            targetArray.set(data.subarray(offset + actualStart, offset + pSize), 0);
            targetArray.set(data.subarray(offset, offset + n - tailLen), tailLen);
        }
        return n;
    }

    flattenTicksInto(targetArray, maxSamples) {
        const pCount = this.parent.count;
        const pSize = this.parent.size;
        const n = Math.min(pCount, maxSamples || pCount, targetArray.length);
        if (n === 0) return 0;
        const start = (this.parent.head - pCount + pSize) % pSize;
        const actualStart = (start + pCount - n) % pSize;
        if (actualStart + n <= pSize) {
            targetArray.set(this.parent.ticks.subarray(actualStart, actualStart + n), 0);
        } else {
            const tailLen = pSize - actualStart;
            targetArray.set(this.parent.ticks.subarray(actualStart, pSize), 0);
            targetArray.set(this.parent.ticks.subarray(0, n - tailLen), tailLen);
        }
        return n;
    }
    
    clear() { } // Handled by parent
}

export class TelemetryHub {
    constructor() {
        // ── Latest snapshots per scale ──────────────────
        this.s0  = freshScale0State();
        this.s1  = {
            diag: null, extended: null, runtime: null,
            _overlaySystemOn: false, _overlayVelocitiesOn: false, _overlayTrailsOn: false,
            _overlayEfieldOn: false, _overlayPotentialOn: false, _overlayGravityFieldOn: false,
            _overlayForceOn: false, _orbitPeriod: null,
            _potentialMin: 0, _potentialMax: 0, _overlaySystemL: 0,
            _overlayProvenanceOn: false,
        };
        this.s2  = { diag: null, runtime: null };  // also used for scale3
        this.s4  = { diag: null };
        this.s5  = { diag: null, cosmic: null };

        // Demand-gated telemetry bookkeeping (SPEC_SCALE0_PERF_TELEMETRY_PANELS §5).
        // The audit / Lagrangian streams collect only when a consumer is visible AND
        // the field version advanced (or a panel just opened — catch-up edge).
        this._lastAuditVersion = -1;
        this._prevWantAudit = false;
        this._prevWantLag = false;
        // Synthetic sequence for direct WASM/mock reads. Native snapshots
        // supply their own per-group stateVersion/snapshotVersion metadata.
        this._s0LocalSampleSequence = 0;

        // Last tick trackers for pause freezing
        this._lastTick0 = -1;
        this._lastAuditTick = -1;
        this._lastLagTick = -1;
        this._lastTick1 = -1;
        this._lastTick1Ext = -1;
        this._lastTick2 = -1;
        this._lastTick4 = -1;
        this._lastTick5 = -1;

        // Incremented by resetScale(); consumers with their own derived state
        // can cheaply clear when the active scene/scale baseline changes.
        this._resetVersions = {
            0: 0,
            1: 0,
            2: 0,
            4: 0,
            5: 0,
        };

        // ── Scale 0 — Lattice / Flux ────────────────────
        // Core diagnostics (500-sample history)
                this._s0_core = new MultiRingBuffer(500, ['flux', 'energy', 'manifested', 'entropy', 'charges', 'positive', 'negative', 'fieldSpin', 'fieldHelicity']);
        this.flux = this._s0_core.views['flux'];
        this.energy = this._s0_core.views['energy'];
        this.manifested = this._s0_core.views['manifested'];
        this.entropy = this._s0_core.views['entropy'];
        this.charges = this._s0_core.views['charges'];
        this.positive = this._s0_core.views['positive'];
        this.negative = this._s0_core.views['negative'];
        // Field circulation ledger (2026-07-28): |S| = |Σ J×W| (conserved by
        // the free wave sector) and H = Σ J·curl J (static twist, not conserved).
        this.fieldSpin = this._s0_core.views['fieldSpin'];
        this.fieldHelicity = this._s0_core.views['fieldHelicity'];
        // ebDiff and gauss are pushed from the audit path (separate cadence),
        // so they must be standalone RingBuffers, not views into _s0_core.
        this.ebDiff = new RingBuffer(500);
        this.gauss = new RingBuffer(500);

        // Per-audit-field trend buffers (500-sample) — drive panel-row sparklines.
                this._s0_aud = new MultiRingBuffer(500, ['dynamicEnergy', 'fieldEnergy', 'waveEnergy', 'particleKE', 'coulombPE', 'eFieldEnergy', 'bFieldEnergy', 'poyntingMag', 'maxGaussError', 'selfFieldInjection', 'eLeftEnergy', 'eRightEnergy', 'chirality', 'waveLeft', 'waveRight', 'energyDrift']);
        this.aud = this._s0_aud.views;

        // Sparkline-resolution (80-sample) — legacy PE canvas sparklines + hub history
                this._s0_sp = new MultiRingBuffer(80, ['manifested', 'charges', 'flux', 'energy', 'entropy']);
        this.sp = this._s0_sp.views;

        // ── Lagrangian (400-sample, 10 terms) ──────────
                this._s0_lag = new MultiRingBuffer(400, ['fieldKinetic', 'fieldGradient', 'bornInfeld', 'coupling', 'velocity', 'gauss', 'dissipation', 'total', 'hamiltonian', 'action']);
        this.lag = this._s0_lag.views;

        // ── Scale 1 — Particle Engine (200-sample) ─────
                // peAnnihilations RETIRED (2026-07 revision): the native engine
                // exposes no annihilation counter and deriving one from count
                // drops would conflate removal causes (audit failure mode).
                this._s1_pe = new MultiRingBuffer(200, ['peKE', 'pePE', 'peCoulombPE', 'peGravityPE', 'peTotal', 'peEnergyDrift', 'peCount', 'peLockedCount', 'peMobileCount', 'peMomentum', 'peAngMom', 'peVirial', 'peTemperature', 'peRmsVelocity', 'peSystemRadius', 'peMaxForce', 'peMeanForce', 'peSeparation', 'peRadialVelocity', 'peMaxBeta', 'peCapCount', 'peNetCharge', 'pePosCount', 'peZeroCount', 'peNegCount']);
        const peViews = this._s1_pe.views;
        this.peKE = peViews.peKE; this.pePE = peViews.pePE; this.peCoulombPE = peViews.peCoulombPE; this.peGravityPE = peViews.peGravityPE; this.peTotal = peViews.peTotal; this.peEnergyDrift = peViews.peEnergyDrift; this.peCount = peViews.peCount; this.peLockedCount = peViews.peLockedCount; this.peMobileCount = peViews.peMobileCount; this.peMomentum = peViews.peMomentum; this.peAngMom = peViews.peAngMom; this.peVirial = peViews.peVirial; this.peTemperature = peViews.peTemperature; this.peRmsVelocity = peViews.peRmsVelocity; this.peSystemRadius = peViews.peSystemRadius; this.peMaxForce = peViews.peMaxForce; this.peMeanForce = peViews.peMeanForce; this.peSeparation = peViews.peSeparation; this.peRadialVelocity = peViews.peRadialVelocity; this.peMaxBeta = peViews.peMaxBeta; this.peCapCount = peViews.peCapCount; this.peNetCharge = peViews.peNetCharge; this.pePosCount = peViews.pePosCount; this.peZeroCount = peViews.peZeroCount; this.peNegCount = peViews.peNegCount;
        this._peInitialEnergy = null;
        this._peBaselineFp = null;
        this._s1Runtime = { scenario: '', softening: 0 };
        // 2-body separation-vs-tick history for the Trails overlay's orbit-
        // period estimate (estimateOrbitPeriod, telemetry/orbit-period.js).
        // Populated by the controller (only while ov.trails is on, n===2)
        // from this hub's own tick-gated peSeparation channel below — NOT
        // from the visual trail cache (pe-cloud-expander.js _trailHistory),
        // whose ring-buffer-of-positions shape carries no tick stamps and
        // isn't an array (no .map), and whose sample cadence is per-render-
        // frame, not per-engine-tick.
        this._s1SepHistory = [];
        // Identity key ("lowerId:higherId") for the pair _s1SepHistory was
        // built from — cleared/reset whenever a different pair (or a
        // non-2-body count) is observed, so annihilation+re-injection
        // silently swapping in a new pair can't mix its samples with a
        // stale prior pairing's history.
        this._s1SepPairKey = null;

        // ── Scale 2/3 — Atom / Molecule Engine (200-sample)
        // All values are SIM UNITS (implicit k_B = 1; audit P0-10) — panels
        // label them "(sim)", never MeV / Kelvin. No aeMaxForce buffer on
        // purpose: aeGetForceDecomposition is O(N²) and visibility-gated, so
        // an always-collected force channel would either pay that cost every
        // tick or sit dead when arrows are hidden (the B1 dead-buffer class).
                this._s2_ae = new MultiRingBuffer(200, ['aeKE', 'aeTemp', 'aeEnergy', 'aeBonds', 'aePEIonic', 'aePEVdw', 'aePEBond', 'aeMomentum', 'aeAtomCount', 'aeDrift']);
        const aeVs = this._s2_ae.views;
        this.aeKE = aeVs.aeKE; this.aeTemp = aeVs.aeTemp; this.aeEnergy = aeVs.aeEnergy; this.aeBonds = aeVs.aeBonds; this.aePEIonic = aeVs.aePEIonic; this.aePEVdw = aeVs.aePEVdw; this.aePEBond = aeVs.aePEBond; this.aeMomentum = aeVs.aeMomentum; this.aeAtomCount = aeVs.aeAtomCount; this.aeDrift = aeVs.aeDrift;
        this._aeInitialEnergy = null;

        // ── Scale 4 — Planetary (200-sample) ───────────
                this._s4_pl = new MultiRingBuffer(200, ['plKE', 'plPE', 'plTotal', 'plEnergyDrift', 'plCount', 'plMomentum', 'plVirial', 'plSystemRadius']);
        const plVs = this._s4_pl.views;
        this.plKE = plVs.plKE; this.plPE = plVs.plPE; this.plTotal = plVs.plTotal; this.plEnergyDrift = plVs.plEnergyDrift; this.plCount = plVs.plCount; this.plMomentum = plVs.plMomentum; this.plVirial = plVs.plVirial; this.plSystemRadius = plVs.plSystemRadius;
        this._plInitialEnergy = null;

        // ── Scale 5 — Cosmic (200-sample) ──────────────
                this._s5_cs = new MultiRingBuffer(200, ['csBodies', 'csHubble', 'csDM']);
        const csVs = this._s5_cs.views;
        this.csBodies = csVs.csBodies; this.csHubble = csVs.csHubble; this.csDM = csVs.csDM;
    }

    // ── Scale 0 collection ──────────────────────────────────────────────────

    _directScale0Meta(group, value, source = 'direct', owner = null) {
        const external = owner?.getScale0TelemetryGroupMeta?.(group)
            ?? owner?.capabilities?.scale0?.getScale0TelemetryGroupMeta?.(group)
            ?? null;
        if (external) {
            const externalTick = external.sampleTick ?? external.tick ?? value?.tick ?? null;
            const hasValue = !!value && typeof value === 'object';
            const status = hasValue
                ? (external.status ?? (external.stale ? 'stale' : 'available'))
                : 'unavailable';
            const result = {
                source: external.backend ?? source,
                epoch: external.epoch ?? null,
                sourceEpoch: external.sourceEpoch ?? null,
                stateVersion: Number.isFinite(external.stateVersion)
                    ? external.stateVersion : null,
                snapshotVersion: Number.isFinite(external.snapshotVersion)
                    ? external.snapshotVersion : (Number.isFinite(external.stateVersion)
                        ? external.stateVersion : ++this._s0LocalSampleSequence),
                tick: Number.isFinite(externalTick) ? externalTick : null,
                sampleTick: Number.isFinite(externalTick) ? externalTick : null,
                stale: !hasValue || external.stale === true || status !== 'available',
                status,
                sampledAt: Number.isFinite(external.sampledAt) ? external.sampledAt : null,
                receivedAt: Number.isFinite(external.receivedAt)
                    ? external.receivedAt : telemetryNow(),
            };
            this._observeScale0SourceEpoch(result, result.source);
            return result;
        }
        const tick = Number.isFinite(value?.tick)
            ? value.tick
            : (Number.isFinite(this.s0.diag?.tick) ? this.s0.diag.tick : null);
        const stateVersion = tick ?? ++this._s0LocalSampleSequence;
        const hasValue = !!value && typeof value === 'object';
        const result = {
            source,
            epoch: null,
            sourceEpoch: null,
            stateVersion,
            snapshotVersion: ++this._s0LocalSampleSequence,
            tick,
            sampleTick: tick,
            stale: !hasValue,
            status: hasValue ? 'available' : 'unavailable',
            sampledAt: null,
            receivedAt: telemetryNow(),
        };
        return result;
    }

    _normalizeScale0GroupMeta(snapshot, rawMeta, value, source = 'native') {
        const meta = rawMeta && typeof rawMeta === 'object' ? rawMeta : {};
        const snapshotVersion = meta.snapshotVersion ?? snapshot?.snapshotVersion;
        const stateVersion = meta.stateVersion ?? null;
        const tick = meta.tick ?? snapshot?.tick ?? value?.tick ?? null;
        return {
            source,
            epoch: meta.epoch ?? snapshot?.epoch ?? null,
            sourceEpoch: meta.sourceEpoch ?? snapshot?.sourceEpoch ?? null,
            stateVersion: Number.isFinite(stateVersion) ? stateVersion : null,
            // Older native servers have no version metadata.  Give each
            // response a synthetic monotonic identity so their cache remains
            // safe, while preferring real source stateVersion when present.
            snapshotVersion: Number.isFinite(snapshotVersion)
                ? snapshotVersion : ++this._s0LocalSampleSequence,
            tick: Number.isFinite(tick) ? tick : null,
            sampleTick: Number.isFinite(meta.sampleTick ?? tick)
                ? (meta.sampleTick ?? tick) : null,
            stale: !!(meta.stale || snapshot?.stale),
            sampledAt: Number.isFinite(meta.sampledAt) ? meta.sampledAt : null,
            // Preserve bridge receipt time so a temporarily busy UI still
            // reports the source sample's age rather than its next paint.
            receivedAt: Number.isFinite(meta.receivedAt)
                ? meta.receivedAt : telemetryNow(),
        };
    }

    _compareScale0GroupMeta(incoming, current) {
        if (!current) return 1;
        if (incoming.source !== current.source) return 1;
        if (incoming.sourceEpoch !== null && current.sourceEpoch !== null
            && incoming.sourceEpoch !== current.sourceEpoch) {
            const incomingSource = Number(incoming.sourceEpoch);
            const currentSource = Number(current.sourceEpoch);
            if (Number.isFinite(incomingSource) && Number.isFinite(currentSource)) {
                return Math.sign(incomingSource - currentSource);
            }
            return String(incoming.sourceEpoch).localeCompare(String(current.sourceEpoch));
        }
        if (incoming.epoch !== null && current.epoch !== null
            && incoming.epoch !== current.epoch) {
            const inEpoch = Number(incoming.epoch);
            const currentEpoch = Number(current.epoch);
            if (Number.isFinite(inEpoch) && Number.isFinite(currentEpoch)) {
                return Math.sign(inEpoch - currentEpoch);
            }
            return String(incoming.epoch).localeCompare(String(current.epoch));
        }
        // stateVersion is per-group and therefore takes precedence over a
        // global publication sequence. Equal state versions are duplicates,
        // even if another group advanced the aggregate snapshot meanwhile.
        if (incoming.stateVersion !== null && current.stateVersion !== null) {
            // A later aggregate publication can contain this same cached
            // group. Equal source versions are duplicates, not fresh data.
            return Math.sign(incoming.stateVersion - current.stateVersion);
        }
        if (incoming.tick !== null && current.tick !== null
            && incoming.tick !== current.tick) {
            return Math.sign(incoming.tick - current.tick);
        }
        if (incoming.snapshotVersion !== null && current.snapshotVersion !== null) {
            return Math.sign(incoming.snapshotVersion - current.snapshotVersion);
        }
        return 0;
    }

    _shouldAcceptScale0Group(group, meta) {
        const current = this.s0.meta.groups[group];
        // An invalidation can deliberately resend the same cached value with
        // unchanged provenance but an explicit stale bit. Accept that state
        // transition so the UI stops presenting its old measurement as live
        // while the native publisher obtains the replacement reduction.
        if (meta.stale && current && !current.stale
            && meta.source === current.source && meta.epoch === current.epoch
            && meta.sourceEpoch === current.sourceEpoch
            && meta.stateVersion === current.stateVersion
            && meta.tick === current.tick) {
            return true;
        }
        // A transient getter failure may mark an otherwise unchanged direct
        // state unavailable. Accept its recovery at the same source identity;
        // scientific history remains deduplicated by the unchanged stamp.
        if (!meta.stale && current?.stale
            && meta.source === current.source && meta.epoch === current.epoch
            && meta.sourceEpoch === current.sourceEpoch
            && meta.stateVersion === current.stateVersion
            && meta.tick === current.tick) {
            return true;
        }
        const order = this._compareScale0GroupMeta(meta, current);
        if (order <= 0) return false;
        // A response explicitly marked stale must not replace a newer live
        // sample from the same source epoch merely because it arrived late.
        if (meta.stale && current && !current.stale
            && meta.source === current.source && meta.epoch === current.epoch) {
            return false;
        }
        return true;
    }

    _setScale0GroupMeta(group, meta) {
        const stored = { ...meta, ageMs: 0 };
        this.s0.meta.groups[group] = stored;
        this.s0.meta.source = meta.source;
        this.s0.meta.epoch = meta.epoch;
        this.s0.meta.sourceEpoch = meta.sourceEpoch;
        this.s0.meta.snapshotVersion = meta.snapshotVersion;
        this.s0.meta.tick = meta.tick;
        this.s0.meta.stale = meta.stale;
    }

    _markScale0GroupUnavailable(group, meta) {
        if (!meta || meta.stale !== true) return false;
        const current = this.s0.meta.groups[group];
        if (current && this._compareScale0GroupMeta(meta, current) < 0) return false;
        this._setScale0GroupMeta(group, meta);
        return true;
    }

    _observeScale0SourceEpoch(snapshot, source) {
        const epoch = Number(snapshot?.sourceEpoch);
        if (!Number.isFinite(epoch)) return;
        const current = this.s0.meta.expectedSourceEpoch;
        const currentSource = this.s0.meta.expectedSource;
        if (currentSource === source && current !== null && epoch < current) return;
        const boundaryChanged = currentSource !== source || current === null || epoch > current;
        this.s0.meta.expectedSource = source;
        this.s0.meta.expectedSourceEpoch = epoch;
        if (boundaryChanged) {
            // Do not overwrite the old per-group stamps. They are useful
            // provenance; getScale0TelemetryMeta will label them stale until
            // a fresh group at this source epoch arrives.
            this.s0.meta.stale = true;
            // A source/configuration mutation is an intervention boundary, not
            // conservation drift. The next finite non-zero audit establishes
            // its own reference baseline.
            this._initialEnergy = null;
        }
    }

    _publishScale0Diagnostics(diag, meta) {
        this.s0.diag = diag;
        this._setScale0GroupMeta('diagnostics', meta);

        const sampleStamp = scale0SampleStamp(meta);
        if (!meta.stale && sampleStamp !== null && sampleStamp !== this._lastTick0) {
            this._lastTick0 = sampleStamp;

            const positive = finiteSample(diag.positive);
            const negative = finiteSample(diag.negative);
            const chargeBalance = firstFiniteSample(
                diag.chargeBalance,
                finiteDifference(positive, negative),
            );
            const fieldSpin = finiteMagnitude(
                finiteSample(diag.fieldSpinX),
                finiteSample(diag.fieldSpinY),
                finiteSample(diag.fieldSpinZ),
            );
            // Dynamic energy normally arrives from the engine's cached
            // per-tick EnergyLedger. Older/direct transports may instead join
            // an exact same-state audit into Diagnostics; the observer-baseline
            // total must never be substituted here.
            const auditMeta = this.getScale0TelemetryMeta('audit');
            const retainedAuditEnergy = auditMeta
                && auditMeta.stale !== true
                && (auditMeta.status == null || auditMeta.status === 'available')
                ? this.s0.audit?.dynamicEnergy : undefined;
            // Diagnostics is intentionally cheaper and advances between full
            // audit reductions. Keep the chart's aligned core row continuous
            // with an explicitly sampled zero-order hold of the latest valid
            // audit energy; a fresh same-tick audit still refines this row via
            // setLast() below. Source-boundary invalidation makes auditMeta
            // stale, so old energy never leaks into a new scenario/profile.
            const dynamicEnergy = Number.isFinite(diag.dynamicEnergy)
                ? diag.dynamicEnergy
                : finiteSample(retainedAuditEnergy);

            // 500-sample buffers for charts
            this._s0_core.push({
                flux: finiteSample(diag.totalFlux),
                energy: dynamicEnergy,
                manifested: finiteSample(diag.manifested),
                entropy: finiteSample(diag.entropy),
                positive,
                negative,
                charges: chargeBalance,
                fieldSpin,
                fieldHelicity: finiteSample(diag.fieldHelicity),
            }, meta.tick);

            // 80-sample sparkline buffers
            this._s0_sp.push({
                manifested: finiteSample(diag.manifested),
                charges: chargeBalance,
                flux: finiteSample(diag.totalFlux),
                energy: dynamicEnergy,
                entropy: finiteSample(diag.entropy),
            }, meta.tick);
        }
        return diag;
    }

    _publishScale0Audit(audit, meta) {
        const eF = firstFiniteSample(audit.EFieldEnergy, audit.eFieldEnergy);
        const bF = firstFiniteSample(audit.BFieldEnergy, audit.bFieldEnergy);
        const px = firstFiniteSample(audit.totalPoynting?.x, audit.poyntingX);
        const py = firstFiniteSample(audit.totalPoynting?.y, audit.poyntingY);
        const pz = firstFiniteSample(audit.totalPoynting?.z, audit.poyntingZ);
        const pMag = finiteMagnitude(px, py, pz);

        // Energy drift calculation
        const currentH = firstFiniteSample(audit.dynamicEnergy, audit.totalEnergy);
        let drift = unavailableSample();
        if (!meta.stale && Number.isFinite(currentH)) {
            if (Number.isFinite(this._initialEnergy) && Math.abs(this._initialEnergy) > 1e-12) {
                drift = ((currentH - this._initialEnergy) / this._initialEnergy) * 100;
            } else if (Math.abs(currentH) > 1e-12) {
                this._initialEnergy = currentH;
                drift = 0;
            }
        }
        const enriched = { ...audit, energyDrift: drift };
        this.s0.audit = enriched;
        this._setScale0GroupMeta('audit', meta);

        const sampleStamp = scale0SampleStamp(meta);
        if (!meta.stale && sampleStamp !== null && sampleStamp !== this._lastAuditTick) {
            this._lastAuditTick = sampleStamp;

            this.ebDiff.push(finiteDifference(eF, bF), meta.tick);
            this.gauss.push(finiteSample(audit.gaussViolation), meta.tick);

            // Native diagnostics and audit are immutable independent groups.
            // Join only the same completed tick into the already-created core
            // row; never mutate the diagnostics object or relabel audit data.
            const diagMeta = this.getScale0TelemetryMeta('diagnostics');
            if (Number.isFinite(currentH) && this.energy.count > 0
                && !diagMeta?.stale && Number.isFinite(meta.tick)
                && meta.tick === diagMeta?.tick) {
                this.energy.setLast(currentH);
                this.sp.energy.setLast(currentH);
            }

            // Per-field trend buffers (drive diagnostics table sparklines)
            this._s0_aud.push({
                dynamicEnergy: currentH,
                fieldEnergy: finiteSample(audit.fieldEnergy),
                waveEnergy: finiteSample(audit.waveEnergy),
                particleKE: finiteSample(audit.particleKE),
                coulombPE: finiteSample(audit.coulombPE),
                eFieldEnergy: eF,
                bFieldEnergy: bF,
                poyntingMag: pMag,
                maxGaussError: finiteSample(audit.maxGaussError),
                selfFieldInjection: finiteSample(audit.selfFieldInjection),
                eLeftEnergy: firstFiniteSample(audit.ELTotal, audit.eLTotal),
                eRightEnergy: firstFiniteSample(audit.ERTotal, audit.eRTotal),
                chirality: finiteSample(audit.chiralityTotal),
                waveLeft: firstFiniteSample(audit.wvLTotal, audit.waveLTotal),
                waveRight: firstFiniteSample(audit.wvRTotal, audit.waveRTotal),
                energyDrift: drift,
            }, meta.tick);
        }
        return enriched;
    }

    _publishScale0Lagrangian(lag, meta) {
        this.s0.lagrangian = lag;
        this._setScale0GroupMeta('lagrangian', meta);

        const sampleStamp = scale0SampleStamp(meta);
        if (!meta.stale && sampleStamp !== null && sampleStamp !== this._lastLagTick) {
            this._lastLagTick = sampleStamp;

            this._s0_lag.push({
                // Charts must agree with the signed raw table. Magnitudes are
                // computed only in getLagrangianDecomposition(), where the UI
                // explicitly asks for fractional contribution sizes.
                fieldKinetic: finiteSample(lag.fieldKinetic),
                fieldGradient: finiteSample(lag.fieldGradient),
                bornInfeld: finiteSample(lag.bornInfeld),
                coupling: finiteSample(lag.coupling),
                velocity: finiteSample(lag.velocity),
                gauss: finiteSample(lag.gauss),
                dissipation: finiteSample(lag.dissipation),
                total: finiteSample(lag.total),
                hamiltonian: finiteSample(lag.hamiltonian),
                action: finiteSample(lag.totalAction),
            }, meta.tick);
        }
        return lag;
    }

    _publishScale0Gravity(gravity, meta) {
        this.s0.gravity = gravity;
        this._setScale0GroupMeta('gravity', meta);
        return gravity;
    }

    /**
     * Merge one native snapshot delta into the Scale-0 store. `groups` may be
     * either the scheduler's nested shape or the former flat get_telemetry
     * response. Only groups explicitly present are considered; cached/stale
     * aggregates can never erase a newer local group.
     */
    ingestScale0Snapshot(snapshot, source = 'native') {
        if (!snapshot || typeof snapshot !== 'object') return false;
        this._observeScale0SourceEpoch(snapshot, source);
        if (snapshot.type === 'telemetry_invalidated') {
            for (const group of SCALE0_SNAPSHOT_GROUPS) {
                const meta = this.s0.meta.groups[group];
                if (!meta || meta.source !== source) continue;
                this.s0.meta.groups[group] = { ...meta, stale: true };
            }
            this.s0.meta.stale = true;
            return true;
        }
        const groups = snapshot.groups && typeof snapshot.groups === 'object'
            ? snapshot.groups : snapshot;
        const groupMeta = snapshot.groupMeta && typeof snapshot.groupMeta === 'object'
            ? snapshot.groupMeta : {};
        let accepted = false;

        for (const group of SCALE0_SNAPSHOT_GROUPS) {
            if (!hasOwn(groups, group)) continue;
            const value = groups[group];
            if (!value || typeof value !== 'object') continue;
            const meta = this._normalizeScale0GroupMeta(
                snapshot, groupMeta[group], value, source,
            );
            if (!this._shouldAcceptScale0Group(group, meta)) continue;

            switch (group) {
            case 'diagnostics': this._publishScale0Diagnostics(value, meta); break;
            case 'audit': this._publishScale0Audit(value, meta); break;
            case 'lagrangian': this._publishScale0Lagrangian(value, meta); break;
            case 'gravity': this._publishScale0Gravity(value, meta); break;
            default: break;
            }
            accepted = true;
        }
        return accepted;
    }

    /** Returns a copy with receipt age for a single Scale-0 group. */
    getScale0TelemetryMeta(group) {
        const meta = this.s0.meta.groups[group];
        if (!meta) return null;
        const expectedEpoch = this.s0.meta.expectedSourceEpoch;
        const expectedSource = this.s0.meta.expectedSource;
        const groupEpoch = Number(meta.sourceEpoch);
        const staleBySourceBoundary = expectedSource !== null
            && Number.isFinite(expectedEpoch)
            && (meta.source !== expectedSource
                || !Number.isFinite(groupEpoch) || groupEpoch < expectedEpoch);
        const receivedAt = Number.isFinite(meta.receivedAt) ? meta.receivedAt : telemetryNow();
        return {
            ...meta,
            stale: !!meta.stale || staleBySourceBoundary,
            ageMs: Math.max(0, telemetryNow() - receivedAt),
        };
    }

    _collectNativeScale0Snapshot(bridge, useFluxMock) {
        if (useFluxMock || typeof bridge?.getTelemetrySnapshot !== 'function') return false;
        this.ingestScale0Snapshot(bridge.getTelemetrySnapshot(), 'native');
        return true;
    }

    /**
     * Collect Scale 0 diagnostics. Native bridges expose a cache-only,
     * versioned snapshot store; direct WASM/mock owners keep their synchronous
     * reads so their existing physics paths remain unchanged.
     */
    collectScale0(bridge, fluxMock, useFluxMock) {
        if (this._collectNativeScale0Snapshot(bridge, useFluxMock)) return this.s0.diag;
        const mainCaps = bridge.capabilities?.scale0;
        if (!mainCaps) return null;

        const mockCaps = useFluxMock ? (fluxMock?.capabilities?.scale0 ?? null) : null;
        let wasmDiag; let wasmDiagRead = false;
        const readWasmDiag = () => {
            if (!wasmDiagRead) { wasmDiag = mainCaps.getScale0Diagnostics(); wasmDiagRead = true; }
            return wasmDiag;
        };
        const owner = mockCaps ? fluxMock : bridge;
        const source = mockCaps ? 'mock' : 'wasm';
        const diag = mockCaps ? mockCaps.getScale0Diagnostics() : readWasmDiag();
        if (!diag) {
            this._markScale0GroupUnavailable(
                'diagnostics', this._directScale0Meta('diagnostics', null, source, owner),
            );
            return null;
        }
        return this._publishScale0Diagnostics(
            diag,
            this._directScale0Meta('diagnostics', diag, source, owner),
        );
    }

    /** Collect a Scale-0 energy audit without triggering native RPC. */
    collectScale0Audit(bridge, fluxMock, useFluxMock) {
        if (this._collectNativeScale0Snapshot(bridge, useFluxMock)) return this.s0.audit;
        const mainCaps = bridge.capabilities?.scale0;
        if (!mainCaps) return null;
        const mockCaps = useFluxMock ? (fluxMock?.capabilities?.scale0 ?? null) : null;
        const audit = mockCaps
            ? mockCaps.getScale0EnergyAudit()
            : mainCaps.getScale0EnergyAudit();
        const owner = mockCaps ? fluxMock : bridge;
        const source = mockCaps ? 'mock' : 'wasm';
        if (!audit) {
            this._markScale0GroupUnavailable(
                'audit', this._directScale0Meta('audit', null, source, owner),
            );
            return null;
        }
        return this._publishScale0Audit(
            audit,
            this._directScale0Meta('audit', audit, source, owner),
        );
    }

    /** Collect Scale-0 Lagrangian data without triggering native RPC. */
    collectScale0Lagrangian(bridge, fluxMock, useFluxMock) {
        if (this._collectNativeScale0Snapshot(bridge, useFluxMock)) return this.s0.lagrangian;
        const mainCaps = bridge.capabilities?.scale0;
        if (!mainCaps) return null;
        const mockCaps = useFluxMock ? (fluxMock?.capabilities?.scale0 ?? null) : null;
        const lag = mockCaps
            ? mockCaps.getScale0Lagrangian()
            : mainCaps.getScale0Lagrangian();
        const owner = mockCaps ? fluxMock : bridge;
        const source = mockCaps ? 'mock' : 'wasm';
        if (!lag) {
            this._markScale0GroupUnavailable(
                'lagrangian', this._directScale0Meta('lagrangian', null, source, owner),
            );
            return null;
        }
        return this._publishScale0Lagrangian(
            lag,
            this._directScale0Meta('lagrangian', lag, source, owner),
        );
    }

    // ── Scale 1 collection ──────────────────────────────────────────────────

    /**
     * Runtime metadata push (scenario label, softening) from the Scale-1
     * controller/UI. Replaces the hub's former direct DOM reads of
     * #pe-scenario-select / #pe-soft-slider (2026-07 audit fix — the hub
     * must not be coupled to specific element ids).
     */
    setScale1Runtime(patch) {
        Object.assign(this._s1Runtime, patch);
    }

    collectScale1(bridge) {
        const diag = bridge.peGetDiagnostics?.();
        if (!diag) return null;
        this.s1.diag = diag;

        const ke  = diag.totalKE       || diag.kineticEnergy || 0;
        const pe  = diag.totalPE       || diag.potentialEnergy || 0;
        const cnt = diag.particleCount || diag.count || 0;
        const coulombPE = diag.coulombPE || 0;
        const gravityPE = diag.gravityPE || 0;
        const totalEnergy = diag.totalEnergy ?? (ke + pe);
        const pMag = diag.totalMomentum ?? diag.momentum ??
            Math.sqrt((diag.momentumX || 0) ** 2 + (diag.momentumY || 0) ** 2 + (diag.momentumZ || 0) ** 2);
        const lMag = diag.totalAngMom ?? diag.angularMomentum ??
            Math.sqrt((diag.angMomX || 0) ** 2 + (diag.angMomY || 0) ** 2 + (diag.angMomZ || 0) ** 2);
        const virial = diag.virialRatio ?? (pe !== 0 ? (2 * ke / Math.abs(pe)) : 0);
        const temperature = cnt > 0 ? (2 / 3) * ke / cnt : 0;
        const registry = bridge.peGetPhysicsRegistry?.();
        const toggleNames = Array.from(registry?.physics || [])
            .filter(spec => spec.available)
            .map(spec => spec.toggle);
        const toggles = {};
        for (const name of toggleNames) toggles[name] = !!bridge.peGetToggle?.(name);

        // Energy-drift baseline with structural re-latch (2026-07 audit fix:
        // "baseline never re-latches"): any change to the particle count or
        // the active toggle set changes the Hamiltonian being conserved, so
        // the old baseline is meaningless — re-latch instead of reporting a
        // fake integrator drift.
        const baselineFp = cnt + '|' + toggleNames.map(n => (toggles[n] ? 1 : 0)).join('');
        const driftAvailable = diag.driftEligible === true;
        if (!driftAvailable) {
            this._peInitialEnergy = null;
            this._peBaselineFp = null;
        } else if ((this._peInitialEnergy === null || this._peBaselineFp !== baselineFp)
            && Math.abs(totalEnergy) > 1e-12) {
            this._peInitialEnergy = totalEnergy;
            this._peBaselineFp = baselineFp;
        }
        const energyDrift = driftAvailable && this._peInitialEnergy
            ? ((totalEnergy - this._peInitialEnergy) / Math.abs(this._peInitialEnergy)) * 100
            : unavailableSample();

        // Runtime metadata is pushed by the controller (setScale1Runtime) —
        // the hub no longer reads the DOM (2026-07 audit fix).
        this.s1.runtime = {
            scenario: this._s1Runtime.scenario,
            dt: bridge.peGetDt?.() ?? 0,
            softening: this._s1Runtime.softening,
            mode: this._s1Runtime.mode,
            toggles,
            capabilities: bridge.peGetBackendCapabilities?.() ?? null,
            registry,
        };

        const currentTick = diag.tick || 0;
        if (currentTick !== this._lastTick1) {
            this._lastTick1 = currentTick;

            this._s1_pe.push({
                peKE: ke,
                pePE: pe,
                peCoulombPE: coulombPE,
                peGravityPE: gravityPE,
                peTotal: totalEnergy,
                peEnergyDrift: energyDrift,
                peCount: cnt,
                peMomentum: pMag,
                peAngMom: lMag,
                peVirial: virial,
                peTemperature: temperature,
                peLockedCount: this.peLockedCount.last(),
                peMobileCount: this.peMobileCount.last(),
                peRmsVelocity: this.peRmsVelocity.last(),
                peSystemRadius: this.peSystemRadius.last(),
                peMaxForce: this.peMaxForce.last(),
                peMeanForce: this.peMeanForce.last(),
                peSeparation: this.peSeparation.last(),
                peRadialVelocity: this.peRadialVelocity.last(),
                peMaxBeta: this.peMaxBeta.last(),
                peCapCount: this.peCapCount.last(),
                peNetCharge: this.peNetCharge.last(),
                pePosCount: this.pePosCount.last(),
                peZeroCount: this.peZeroCount.last(),
                peNegCount: this.peNegCount.last()
            }, currentTick);
        }
        return diag;
    }

    collectScale1Extended(bridge) {
        const ext = bridge.peGetExtendedData?.();
        if (ext) {
            this.s1.extended = ext;
            const n = ext.count || 0;
            let locked = 0;
            let v2sum = 0;
            let totalMass = 0;
            let cmx = 0, cmy = 0, cmz = 0;
            let maxForce = 0;
            let sumForce = 0;
            // FTD additions: causal saturation + ternary charge composition
            let maxV2 = 0;
            let capCount = 0;
            const capThresh2 = (C_SPEED * 0.999) ** 2;
            let netCharge = 0, nPos = 0, nZero = 0, nNeg = 0;

            for (let i = 0; i < n; i++) {
                if (ext.locked?.[i]) locked++;
                const m = ext.masses?.[i] || 0;
                const px = ext.positions?.[i * 3] || 0;
                const py = ext.positions?.[i * 3 + 1] || 0;
                const pz = ext.positions?.[i * 3 + 2] || 0;
                const vx = ext.velocities?.[i * 3] || 0;
                const vy = ext.velocities?.[i * 3 + 1] || 0;
                const vz = ext.velocities?.[i * 3 + 2] || 0;
                const fx = ext.forces?.[i * 3] || 0;
                const fy = ext.forces?.[i * 3 + 1] || 0;
                const fz = ext.forces?.[i * 3 + 2] || 0;
                const fMag = Math.sqrt(fx * fx + fy * fy + fz * fz);
                maxForce = Math.max(maxForce, fMag);
                sumForce += fMag;
                const vi2 = vx * vx + vy * vy + vz * vz;
                v2sum += vi2;
                if (vi2 > maxV2) maxV2 = vi2;
                if (vi2 >= capThresh2) capCount++;
                const qi = ext.charges?.[i] || 0;
                netCharge += qi;
                if (qi > 0) nPos++; else if (qi < 0) nNeg++; else nZero++;
                totalMass += m;
                cmx += m * px;
                cmy += m * py;
                cmz += m * pz;
            }

            if (totalMass > 0) {
                cmx /= totalMass;
                cmy /= totalMass;
                cmz /= totalMass;
            }

            let systemRadius = 0;
            for (let i = 0; i < n; i++) {
                const dx = (ext.positions?.[i * 3] || 0) - cmx;
                const dy = (ext.positions?.[i * 3 + 1] || 0) - cmy;
                const dz = (ext.positions?.[i * 3 + 2] || 0) - cmz;
                systemRadius = Math.max(systemRadius, Math.sqrt(dx * dx + dy * dy + dz * dz));
            }

            let separation = 0;
            let radialVelocity = 0;
            if (n === 2) {
                const dx = (ext.positions?.[3] || 0) - (ext.positions?.[0] || 0);
                const dy = (ext.positions?.[4] || 0) - (ext.positions?.[1] || 0);
                const dz = (ext.positions?.[5] || 0) - (ext.positions?.[2] || 0);
                const dvx = (ext.velocities?.[3] || 0) - (ext.velocities?.[0] || 0);
                const dvy = (ext.velocities?.[4] || 0) - (ext.velocities?.[1] || 0);
                const dvz = (ext.velocities?.[5] || 0) - (ext.velocities?.[2] || 0);
                separation = Math.sqrt(dx * dx + dy * dy + dz * dz);
                radialVelocity = separation > 0 ? (dx * dvx + dy * dvy + dz * dvz) / separation : 0;
            }

            const currentTick = this.s1.diag?.tick || 0;
            if (currentTick !== this._lastTick1Ext) {
                this._lastTick1Ext = currentTick;

                // setLast(), not push(): collectScale1() already advanced
                // _s1_pe's shared ring this tick (with stale .last() values
                // for these same channels, per its own push({...}) call
                // above) — these calls patch this tick's row with the fresh
                // values just computed, rather than pushing a second,
                // desynchronized row into the shared ring.
                this.peLockedCount.setLast(locked);
                this.peMobileCount.setLast(Math.max(0, n - locked));
                this.peRmsVelocity.setLast(n > 0 ? Math.sqrt(v2sum / n) / C_SPEED : 0);
                this.peSystemRadius.setLast(systemRadius);
                this.peMaxForce.setLast(maxForce);
                this.peMeanForce.setLast(n > 0 ? sumForce / n : 0);
                this.peSeparation.setLast(separation);
                this.peRadialVelocity.setLast(radialVelocity / C_SPEED);
                this.peMaxBeta.setLast(maxV2 > 0 ? Math.sqrt(maxV2) / C_SPEED : 0);
                this.peCapCount.setLast(capCount);
                this.peNetCharge.setLast(netCharge);
                this.pePosCount.setLast(nPos);
                this.peZeroCount.setLast(nZero);
                this.peNegCount.setLast(nNeg);
            }
        }
        return ext;
    }

    // ── Scale 2/3 collection ────────────────────────────────────────────────

    collectScale2(bridge) {
        const diag = bridge.aeGetDiagnostics?.();
        if (!diag) return null;
        this.s2.diag = diag;

        // Field names per mock-atom-engine.js aeGetDiagnostics() — the only
        // AE backend (WASM AtomEngine disabled, wasm-bridge._aeHasWasm).
        // B1 fix 2026-06-10: this previously read diag.kineticEnergy /
        // diag.totalPotential, fields that do not exist, so aeKE and
        // aeEnergy pushed 0 forever.
        const ke = diag.totalKE || 0;
        const totalEnergy = diag.totalEnergy ?? (
            ke + (diag.totalPEIonic || 0) + (diag.totalPEVdw || 0) + (diag.totalPEBond || 0)
        );
        const pMag = Math.sqrt(
            (diag.momentumX || 0) ** 2 + (diag.momentumY || 0) ** 2 + (diag.momentumZ || 0) ** 2
        );
        const driftAvailable = diag.energyComplete === true && diag.energyConservative === true;
        if (!driftAvailable) {
            this._aeInitialEnergy = null;
        } else if (this._aeInitialEnergy === null && Math.abs(totalEnergy) > 1e-12) {
            this._aeInitialEnergy = totalEnergy;
        }
        const energyDrift = driftAvailable && this._aeInitialEnergy
            ? ((totalEnergy - this._aeInitialEnergy) / Math.abs(this._aeInitialEnergy)) * 100
            : Number.NaN;

        // Runtime snapshot (engine truth for the diagnostics descriptors).
        // Scenario label from the DOM: scale 3 owns mol-scenario-select,
        // scale 2 owns ae-scenario-select (mirrors collectScale1).
        const runtime = bridge.aeGetRuntimeState?.() ?? null;
        let scenario = '';
        if (typeof document !== 'undefined') {
            const app = document.getElementById('app');
            const selectId = app?.dataset.activeScale === '3' ? 'mol-scenario-select' : 'ae-scenario-select';
            const scenarioSelect = document.getElementById(selectId);
            scenario = scenarioSelect?.selectedOptions?.[0]?.textContent || scenarioSelect?.value || '';
            // Molecule option labels carry literal markup (e.g. "H<sub>2</sub>") —
            // strip tags so the diagnostics text row doesn't show raw HTML.
            scenario = scenario.replace(/<[^>]*>/g, '');
        }
        this.s2.runtime = runtime ? { scenario, ...runtime } : { scenario };

        const currentTick = diag.tick || 0;
        if (currentTick !== this._lastTick2) {
            this._lastTick2 = currentTick;

            this._s2_ae.push({
                aeKE: ke,
                aeTemp: diag.temperature || 0,
                aeEnergy: totalEnergy,
                aeBonds: diag.bondCount || 0,
                aePEIonic: diag.totalPEIonic || 0,
                aePEVdw: diag.totalPEVdw || 0,
                aePEBond: diag.totalPEBond || 0,
                aeMomentum: pMag,
                aeAtomCount: diag.atomCount || 0,
                aeDrift: energyDrift
            }, currentTick);
        }
        return diag;
    }

    // ── Scale 4 collection ──────────────────────────────────────────────────

    collectScale4(bridge) {
        if (!bridge) return null;
        const diag = bridge.getDiagnostics?.();
        if (!diag) return null;
        this.s4.diag = diag;

        const currentTick = diag.tick || 0;
        if (currentTick !== this._lastTick4) {
            this._lastTick4 = currentTick;

            const bodies = bridge._bodies || [];
            const N = bodies.length;

            let ke = 0;
            let pe = 0;
            let sumPx = 0, sumPy = 0, sumPz = 0;
            let totalMass = 0;
            let sumMx = 0, sumMy = 0, sumMz = 0;

            for (let i = 0; i < N; i++) {
                const b = bodies[i];
                const m = b.mass || 0;
                totalMass += m;
                ke += 0.5 * m * (b.vx * b.vx + b.vy * b.vy + b.vz * b.vz);
                sumPx += m * b.vx;
                sumPy += m * b.vy;
                sumPz += m * b.vz;
                sumMx += m * b.x;
                sumMy += m * b.y;
                sumMz += m * b.z;
            }

            // A bridge may intentionally disable gravity with G = 0. Only a
            // missing value falls back to the canonical lattice coupling.
            const G = bridge.G ?? G_N;
            for (let i = 0; i < N; i++) {
                const bi = bodies[i];
                const mi = bi.mass || 0;
                for (let j = i + 1; j < N; j++) {
                    const bj = bodies[j];
                    const mj = bj.mass || 0;
                    const dx = bi.x - bj.x;
                    const dy = bi.y - bj.y;
                    const dz = bi.z - bj.z;
                    const r2 = dx * dx + dy * dy + dz * dz;
                    pe -= G * mi * mj / Math.sqrt(r2 + 1e-6);
                }
            }

            const totalEnergy = ke + pe;
            const momentum = Math.sqrt(sumPx * sumPx + sumPy * sumPy + sumPz * sumPz);

            let systemRadius = 0;
            if (totalMass > 0) {
                const comX = sumMx / totalMass;
                const comY = sumMy / totalMass;
                const comZ = sumMz / totalMass;
                for (let i = 0; i < N; i++) {
                    const b = bodies[i];
                    const dx = b.x - comX;
                    const dy = b.y - comY;
                    const dz = b.z - comZ;
                    const dist = Math.sqrt(dx * dx + dy * dy + dz * dz);
                    if (dist > systemRadius) {
                        systemRadius = dist;
                    }
                }
            }

            const virial = pe !== 0 ? (2 * ke / Math.abs(pe)) : 0;

            if (this._plInitialEnergy === null && Math.abs(totalEnergy) > 1e-12) {
                this._plInitialEnergy = totalEnergy;
            }
            const drift = this._plInitialEnergy
                ? ((totalEnergy - this._plInitialEnergy) / Math.abs(this._plInitialEnergy)) * 100
                : 0;

            // Single push({...}) into the owning MultiRingBuffer — plKE etc.
            // are RingBufferViews (this._s4_pl.views.plKE); RingBufferView
            // has no push() of its own (an individual-channel .push() call
            // here threw every tick, TypeError, confirmed by execution).
            this._s4_pl.push({
                plKE: ke,
                plPE: pe,
                plTotal: totalEnergy,
                plEnergyDrift: drift,
                plCount: N,
                plMomentum: momentum,
                plVirial: virial,
                plSystemRadius: systemRadius,
            }, currentTick);
        }
        return diag;
    }

    // ── Scale 5 collection ──────────────────────────────────────────────────

    collectScale5(cosmicBridge) {
        if (!cosmicBridge) return null;
        const diag = cosmicBridge.getDiagnostics?.();
        if (!diag) return null;
        this.s5.diag = diag;

        const currentTick = diag.tick || 0;
        if (currentTick !== this._lastTick5) {
            this._lastTick5 = currentTick;

            const dmMass = diag.dmMass || 0;
            const totalMass = diag.totalMass || 0;
            const dmFraction = totalMass > 0 ? (dmMass / totalMass) * 100 : 0;
            this._s5_cs.push({
                csBodies: diag.bodyCount || diag.count || 0,
                csHubble: diag.hubbleParameter || diag.hubble || diag.hubbleParam || 0,
                csDM: dmFraction,
            }, currentTick);
        }
        return diag;
    }

    // ── Derived metrics ─────────────────────────────────────────────────────

    /** Scale 0: particle composition ratios */
    getScale0Derived() {
        const d = this.s0.diag;
        const meta = this.getScale0TelemetryMeta('diagnostics');
        const n = finiteSample(d?.manifested);
        if (!d || meta?.stale || !Number.isFinite(n) || n <= 0) return null;
        const positive = finiteSample(d.positive);
        const negative = finiteSample(d.negative);
        const red = finiteSample(d.colorRed);
        const green = finiteSample(d.colorGreen);
        const blue = finiteSample(d.colorBlue);
        const spinUp = finiteSample(d.spinUp);
        const spinDown = finiteSample(d.spinDown);
        const colorless = finiteSample(d.colorless);
        return {
            chiralityRatio: Number.isFinite(positive) && Number.isFinite(negative) && negative !== 0
                ? positive / negative : unavailableSample(),
            chargeImbalance: Number.isFinite(positive) && Number.isFinite(negative)
                ? Math.abs(positive - negative) : unavailableSample(),
            colorFraction: [red, green, blue].every(Number.isFinite)
                ? (red + green + blue) / n : unavailableSample(),
            spinAsymmetry: [spinUp, spinDown].every(Number.isFinite)
                ? Math.abs(spinUp - spinDown) / n : unavailableSample(),
            colorlessRatio: Number.isFinite(colorless)
                ? colorless / n : unavailableSample(),
        };
    }

    /** Scale 0: constraint violation status */
    getConservationStatus() {
        const a = this.s0.audit;
        const meta = this.getScale0TelemetryMeta('audit');
        if (!a || meta?.stale) return null;
        const gv = finiteSample(a.gaussViolation);
        const maxGaussError = finiteSample(a.maxGaussError);
        const selfFieldInjection = finiteSample(a.selfFieldInjection);
        const totalEnergy = finiteSample(a.totalEnergy);
        return {
            ok:             Number.isFinite(gv) ? gv < 1e-4 : null,
            gaussViolation: gv,
            maxGaussError,
            selfFieldRatio: Number.isFinite(selfFieldInjection)
                && Number.isFinite(totalEnergy) && Math.abs(totalEnergy) > 1e-12
                ? Math.abs(selfFieldInjection) / Math.abs(totalEnergy)
                : unavailableSample(),
        };
    }

    /** Scale 1: orbital mechanics summary (2-body only) */
    getScale1OrbitalMetrics() {
        const ext = this.s1.extended;
        if (!ext) return null;
        return ext.orbital ?? ext.orbitParams ?? null;
    }

    /** Scale 1: thermodynamic quantities */
    getScale1Thermo() {
        const d = this.s1.diag;
        if (!d) return null;
        const ke = d.totalKE || d.kineticEnergy || 0;
        const n  = d.particleCount || d.count || 1;
        const ext = this.s1.extended;
        let rmsVelocity = d.rmsVelocity || 0;
        if (!rmsVelocity && ext?.velocities && ext.count > 0) {
            let v2 = 0;
            for (let i = 0; i < ext.count; i++) {
                const vx = ext.velocities[i * 3] || 0;
                const vy = ext.velocities[i * 3 + 1] || 0;
                const vz = ext.velocities[i * 3 + 2] || 0;
                v2 += vx * vx + vy * vy + vz * vz;
            }
            rmsVelocity = Math.sqrt(v2 / ext.count);
        }
        return {
            temperature:    (2 / 3) * ke / n,   // equipartition T = 2KE/3N
            virialRatio:    d.virialRatio ?? (d.totalPE ? 2 * ke / Math.abs(d.totalPE) : 0),
            rmsVelocity,
        };
    }

    /** Scale 0: Lagrangian field/particle/constraint decomposition */
    getLagrangianDecomposition() {
        const lag = this.s0.lagrangian;
        const meta = this.getScale0TelemetryMeta('lagrangian');
        if (!lag || meta?.stale) return null;
        const samples = [
            lag.fieldKinetic, lag.fieldGradient, lag.bornInfeld,
            lag.coupling, lag.velocity, lag.gauss, lag.dissipation,
        ].map(finiteAbsSample);
        if (!samples.every(Number.isFinite)) return null;
        const [fieldKinetic, fieldGradient, particle, coupling, velocity, constraint, dissipation]
            = samples;
        const field = fieldKinetic + fieldGradient;
        const interaction = coupling + velocity;
        const total = field + particle + interaction + constraint + dissipation;
        if (!(total > 0)) return null;
        return {
            fieldFraction:       field       / total,
            particleFraction:    particle    / total,
            interactionFraction: interaction / total,
            constraintFraction:  constraint  / total,
            dissipationFraction: dissipation / total,
        };
    }

    // ── Reset ────────────────────────────────────────────────────────────────

    getResetVersion(scale) {
        return this._resetVersions?.[scale] ?? 0;
    }

    resetScale(scale) {
        const resetKey = scale === 3 ? 2 : scale;
        if (Object.prototype.hasOwnProperty.call(this._resetVersions, resetKey)) {
            this._resetVersions[resetKey]++;
        }
        switch (scale) {
                        case 0:
                this._s0_core.clear();
                this._s0_sp.clear();
                this._s0_aud.clear();
                this._s0_lag.clear();
                this.ebDiff.clear();
                this.gauss.clear();
                this.s0 = freshScale0State();
                this._initialEnergy = null;
                this._s0LocalSampleSequence = 0;
                this._lastAuditVersion = -1;
                this._prevWantAudit = false;
                this._prevWantLag = false;
                this._lastTick0 = -1;
                this._lastAuditTick = -1;
                this._lastLagTick = -1;
                break;
                        case 1:
                this._s1_pe.clear();
                this.s1 = {
                    diag: null, extended: null, runtime: null,
                    _overlaySystemOn: false, _overlayVelocitiesOn: false, _overlayTrailsOn: false,
                    _overlayEfieldOn: false, _overlayPotentialOn: false, _overlayGravityFieldOn: false,
                    _overlayForceOn: false, _orbitPeriod: null,
                    _potentialMin: 0, _potentialMax: 0, _overlaySystemL: 0,
                    _overlayProvenanceOn: false,
                };
                this._s1SepHistory = [];
                this._s1SepPairKey = null;
                this._peInitialEnergy = null;
                this._peBaselineFp = null;
                this._lastTick1 = -1;
                this._lastTick1Ext = -1;
                break;
                        case 2:
            case 3:
                this._s2_ae.clear();
                this.s2 = { diag: null, runtime: null };
                this._aeInitialEnergy = null;
                this._lastTick2 = -1;
                break;
                        case 4:
                this._s4_pl.clear();
                this.s4 = { diag: null };
                this._plInitialEnergy = null;
                this._lastTick4 = -1;
                break;
                        case 5:
                this._s5_cs.clear();
                this.s5 = { diag: null, cosmic: null };
                this._lastTick5 = -1;
                break;
        }
    }

    resetAll() {
        for (const scale of [0, 1, 2, 4, 5]) this.resetScale(scale);
    }
}

// ── Singleton export ─────────────────────────────────────────────────────────
// All controllers import this instance; no constructor args needed.
export const telemetryHub = new TelemetryHub();
