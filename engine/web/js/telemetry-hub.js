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

import { C_SPEED } from './constants.js';

// ── Ring Buffer ──────────────────────────────────────────────────────────────
// Exported so chart renderers can type-check and legacy code can import it.

export class RingBuffer {
    constructor(size = 500) {
        this.data  = new Float32Array(size);
        this.size  = size;
        this.head  = 0;
        this.count = 0;
        this.total = 0;
    }

    push(value) {
        this.data[this.head] = isFinite(value) ? value : 0;
        this.head = (this.head + 1) % this.size;
        if (this.count < this.size) this.count++;
        this.total++;
    }

    get(i) {
        if (i >= this.count) return 0;
        return this.data[(this.head - this.count + i + this.size) % this.size];
    }

    last() {
        if (this.count === 0) return 0;
        return this.data[(this.head - 1 + this.size) % this.size];
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

    clear() { this.head = 0; this.count = 0; this.total = 0; }
}

// ── Telemetry Hub ────────────────────────────────────────────────────────────


export class MultiRingBuffer {
    constructor(size, channelNames) {
        this.size = size;
        this.channels = channelNames;
        this.numChannels = channelNames.length;
        this.data = new Float32Array(size * this.numChannels);
        this.head = 0;
        this.count = 0;
        this.total = 0;
        
        this.views = {};
        channelNames.forEach((name, i) => {
            this.views[name] = new RingBufferView(this, i * size);
        });
    }

    push(frame) {
        for (let i = 0; i < this.numChannels; i++) {
            const val = frame[this.channels[i]];
            this.data[(i * this.size) + this.head] = isFinite(val) ? val : 0;
        }
        this.head = (this.head + 1) % this.size;
        if (this.count < this.size) this.count++;
        this.total++;
    }

    pushArray(frameArray) {
        // Zero-copy array push
        for (let i = 0; i < this.numChannels; i++) {
            this.data[(i * this.size) + this.head] = frameArray[i];
        }
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
    constructor(parent, offset) {
        this.parent = parent;
        this.offset = offset;
    }
    
    get count() { return this.parent.count; }
    get total() { return this.parent.total; }
    get size() { return this.parent.size; }

    get(i) {
        if (i >= this.parent.count) return 0;
        const pSize = this.parent.size;
        const idx = (this.parent.head - this.parent.count + i + pSize) % pSize;
        return this.parent.data[this.offset + idx];
    }
    
    last() {
        if (this.parent.count === 0) return 0;
        const pSize = this.parent.size;
        const idx = (this.parent.head - 1 + pSize) % pSize;
        return this.parent.data[this.offset + idx];
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
        this.parent.data[this.offset + idx] = Number.isFinite(value) ? value : 0;
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
        const offset = this.offset;

        if (actualStart + n <= pSize) {
            targetArray.set(data.subarray(offset + actualStart, offset + actualStart + n), 0);
        } else {
            const tailLen = pSize - actualStart;
            targetArray.set(data.subarray(offset + actualStart, offset + pSize), 0);
            targetArray.set(data.subarray(offset, offset + n - tailLen), tailLen);
        }
        return n;
    }
    
    clear() { } // Handled by parent
}

export class TelemetryHub {
    constructor() {
        // ── Latest snapshots per scale ──────────────────
        this.s0  = { diag: null, audit: null, lagrangian: null };
        this.s1  = {
            diag: null, extended: null, runtime: null,
            _overlaySystemOn: false, _overlayVelocitiesOn: false, _overlayTrailsOn: false,
            _overlayEfieldOn: false, _overlayPotentialOn: false, _overlayGravityFieldOn: false,
            _overlayForceOn: false, _orbitPeriod: null,
            _potentialMin: 0, _potentialMax: 0,
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
                this._s0_aud = new MultiRingBuffer(500, ['fieldEnergy', 'waveEnergy', 'particleKE', 'coulombPE', 'eFieldEnergy', 'bFieldEnergy', 'poyntingMag', 'maxGaussError', 'selfFieldInjection', 'eLeftEnergy', 'eRightEnergy', 'chirality', 'waveLeft', 'waveRight', 'energyDrift']);
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

    /**
     * Collect Scale 0 diagnostics. Implements the dual-bridge logic:
     * if a JS flux mock is active, use its snapshot because it owns the
     * field state, particles, and tick for that scenario.
     * @returns {object} the active diag snapshot
     */
    collectScale0(bridge, fluxMock, useFluxMock) {
        const mainCaps = bridge.capabilities?.scale0;
        if (!mainCaps) return null;

        const mockCaps = useFluxMock ? (fluxMock?.capabilities?.scale0 ?? null) : null;
        // Lazy: getScale0Diagnostics() is an uncached O(N^3) sweep on the render
        // thread (render_bridge.cpp -> diagnostics_compute.cpp, plus two more
        // passes inside compute_entropy_cpu). It was evaluated unconditionally
        // and then discarded whenever the worker owned the scenario.
        let _wasmDiag; let _wasmDiagRead = false;
        const wasmDiag = () => {
            if (!_wasmDiagRead) { _wasmDiag = mainCaps.getScale0Diagnostics(); _wasmDiagRead = true; }
            return _wasmDiag;
        };
        const mockDiag = mockCaps ? mockCaps.getScale0Diagnostics() : null;

        const diag = mockDiag || wasmDiag();

        this.s0.diag = diag;

        const currentTick = diag.tick || 0;
        if (currentTick !== this._lastTick0) {
            this._lastTick0 = currentTick;

            // 500-sample buffers for charts
            this._s0_core.push({
                flux: diag.totalFlux || 0,
                energy: diag.totalEnergy || 0,
                manifested: diag.manifested || 0,
                entropy: diag.entropy || 0,
                positive: diag.positive || 0,
                negative: diag.negative || 0,
                charges: (diag.positive || 0) - (diag.negative || 0),
                fieldSpin: Math.hypot(diag.fieldSpinX || 0, diag.fieldSpinY || 0, diag.fieldSpinZ || 0),
                fieldHelicity: diag.fieldHelicity || 0,
            });

            // 80-sample sparkline buffers
            this._s0_sp.push({
                manifested: diag.manifested || 0,
                charges: (diag.positive || 0) - (diag.negative || 0),
                flux: diag.totalFlux || 0,
                energy: diag.totalEnergy || 0,
                entropy: diag.entropy || 0
            });
        }

        return diag;
    }

    /**
     * Collect energy audit for Scale 0 (call when diagnostics or charts tab active).
     */
    collectScale0Audit(bridge, fluxMock, useFluxMock) {
        const mainCaps = bridge.capabilities?.scale0;
        if (!mainCaps) return null;
        const mockCaps = useFluxMock ? (fluxMock?.capabilities?.scale0 ?? null) : null;

        const audit = mockCaps
            ? mockCaps.getScale0EnergyAudit()
            : mainCaps.getScale0EnergyAudit();

        this.s0.audit = audit;
        if (audit) {
            const eF  = audit.EFieldEnergy || audit.eFieldEnergy || 0;
            const bF  = audit.BFieldEnergy || audit.bFieldEnergy || 0;
            const px  = audit.totalPoynting?.x ?? audit.poyntingX ?? 0;
            const py  = audit.totalPoynting?.y ?? audit.poyntingY ?? 0;
            const pz  = audit.totalPoynting?.z ?? audit.poyntingZ ?? 0;
            const pMag = Math.sqrt(px * px + py * py + pz * pz);

            // Energy drift calculation
            const currentH = audit.dynamicEnergy ?? audit.totalEnergy ?? 0;
            if (this._initialEnergy === undefined || this._initialEnergy === null) {
                if (currentH > 1e-12) this._initialEnergy = currentH;
            }
            let drift = 0;
            if (this._initialEnergy) {
                drift = ((currentH - this._initialEnergy) / this._initialEnergy) * 100;
            }
            audit.energyDrift = drift;

            const currentTick = this.s0.diag?.tick || 0;
            if (currentTick !== this._lastAuditTick) {
                this._lastAuditTick = currentTick;

                this.ebDiff.push(eF - bF);
                this.gauss.push(audit.gaussViolation || 0);

                // Per-field trend buffers (drive diagnostics table sparklines)
                this._s0_aud.push({
                    fieldEnergy: audit.fieldEnergy || 0,
                    waveEnergy: audit.waveEnergy || 0,
                    particleKE: audit.particleKE || 0,
                    coulombPE: audit.coulombPE || 0,
                    eFieldEnergy: eF,
                    bFieldEnergy: bF,
                    poyntingMag: pMag,
                    maxGaussError: audit.maxGaussError || 0,
                    selfFieldInjection: audit.selfFieldInjection || 0,
                    eLeftEnergy: audit.ELTotal || audit.eLTotal || 0,
                    eRightEnergy: audit.ERTotal || audit.eRTotal || 0,
                    chirality: audit.chiralityTotal || 0,
                    waveLeft: audit.wvLTotal || 0,
                    waveRight: audit.wvRTotal || 0,
                    energyDrift: drift
                });
            }
        }
        return audit;
    }

    /**
     * Collect Lagrangian data for Scale 0.
     */
    collectScale0Lagrangian(bridge, fluxMock, useFluxMock) {
        const mainCaps = bridge.capabilities?.scale0;
        if (!mainCaps) return null;
        const mockCaps = useFluxMock ? (fluxMock?.capabilities?.scale0 ?? null) : null;

        const lag = mockCaps
            ? mockCaps.getScale0Lagrangian()
            : mainCaps.getScale0Lagrangian();

        this.s0.lagrangian = lag;
        if (lag) {
            const currentTick = this.s0.diag?.tick || 0;
            if (currentTick !== this._lastLagTick) {
                this._lastLagTick = currentTick;

                this._s0_lag.push({
                    fieldKinetic: Math.abs(lag.fieldKinetic || 0),
                    fieldGradient: Math.abs(lag.fieldGradient || 0),
                    bornInfeld: Math.abs(lag.bornInfeld || 0),
                    coupling: Math.abs(lag.coupling || 0),
                    velocity: Math.abs(lag.velocity || 0),
                    gauss: Math.abs(lag.gauss || 0),
                    dissipation: Math.abs(lag.dissipation || 0),
                    total: lag.total || 0,
                    hamiltonian: lag.hamiltonian || 0,
                    action: lag.totalAction || 0
                });
            }
        }
        return lag;
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
        const toggleNames = [
            'coulomb', 'gravity', 'damping', 'lorentz', 'exchange',
            'strong', 'magnetic_dipole', 'spin_orbit', 'radiation',
            'relativistic', 'relativistic_verlet',
        ];
        const toggles = {};
        for (const name of toggleNames) toggles[name] = !!bridge.peGetToggle?.(name);

        // Energy-drift baseline with structural re-latch (2026-07 audit fix:
        // "baseline never re-latches"): any change to the particle count or
        // the active toggle set changes the Hamiltonian being conserved, so
        // the old baseline is meaningless — re-latch instead of reporting a
        // fake integrator drift.
        const baselineFp = cnt + '|' + toggleNames.map(n => (toggles[n] ? 1 : 0)).join('');
        if ((this._peInitialEnergy === null || this._peBaselineFp !== baselineFp)
            && Math.abs(totalEnergy) > 1e-12) {
            this._peInitialEnergy = totalEnergy;
            this._peBaselineFp = baselineFp;
        }
        const energyDrift = this._peInitialEnergy
            ? ((totalEnergy - this._peInitialEnergy) / Math.abs(this._peInitialEnergy)) * 100
            : 0;

        // Runtime metadata is pushed by the controller (setScale1Runtime) —
        // the hub no longer reads the DOM (2026-07 audit fix).
        this.s1.runtime = {
            scenario: this._s1Runtime.scenario,
            dt: bridge.peGetDt?.() ?? 0,
            softening: this._s1Runtime.softening,
            toggles,
            capabilities: bridge.peGetBackendCapabilities?.() ?? null,
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
            });
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
        if (this._aeInitialEnergy === null && Math.abs(totalEnergy) > 1e-12) {
            this._aeInitialEnergy = totalEnergy;
        }
        const energyDrift = this._aeInitialEnergy
            ? ((totalEnergy - this._aeInitialEnergy) / Math.abs(this._aeInitialEnergy)) * 100
            : 0;

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
            });
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

            const G = bridge.G || 0.01;
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
            });
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

            const counts = diag.countsByType || [];
            const total = diag.bodyCount || 1;
            const dmFraction = ((counts[3] || 0) / total) * 100;
            // Single push({...}) — see the collectScale4 comment above for
            // why per-channel .push() on a RingBufferView throws.
            this._s5_cs.push({
                csBodies: diag.bodyCount || diag.count || 0,
                csHubble: diag.hubbleParameter || diag.hubble || diag.hubbleParam || 0,
                csDM: dmFraction,
            });
        }
        return diag;
    }

    // ── Derived metrics ─────────────────────────────────────────────────────

    /** Scale 0: particle composition ratios */
    getScale0Derived() {
        const d = this.s0.diag;
        if (!d || !d.manifested) return { chiralityRatio: 1, colorFraction: 0, spinAsymmetry: 0 };
        const n = d.manifested;
        return {
            chiralityRatio:  d.positive && d.negative ? d.positive / d.negative : 1,
            chargeImbalance: Math.abs((d.positive || 0) - (d.negative || 0)),
            colorFraction:   ((d.colorRed || 0) + (d.colorGreen || 0) + (d.colorBlue || 0)) / n,
            spinAsymmetry:   Math.abs((d.spinUp || 0) - (d.spinDown || 0)) / n,
            colorlessRatio:  (d.colorless || 0) / n,
        };
    }

    /** Scale 0: constraint violation status */
    getConservationStatus() {
        const a = this.s0.audit;
        if (!a) return { ok: true, gaussViolation: 0, maxGaussError: 0 };
        const gv = a.gaussViolation || 0;
        return {
            ok:             gv < 1e-4,
            gaussViolation: gv,
            maxGaussError:  a.maxGaussError || 0,
            selfFieldRatio: a.selfFieldInjection && a.totalEnergy
                ? Math.abs(a.selfFieldInjection) / Math.max(a.totalEnergy, 1e-12)
                : 0,
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
        if (!lag) return null;
        const field      = Math.abs(lag.fieldKinetic || 0) + Math.abs(lag.fieldGradient || 0);
        const particle   = Math.abs(lag.bornInfeld   || 0);
        const interaction = Math.abs(lag.coupling    || 0) + Math.abs(lag.velocity || 0);
        const constraint  = Math.abs(lag.gauss       || 0);
        const dissipation = Math.abs(lag.dissipation || 0);
        const total       = field + particle + interaction + constraint + dissipation || 1;
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
                this.s0 = { diag: null, audit: null, lagrangian: null };
                this._initialEnergy = null;
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
                    _potentialMin: 0, _potentialMax: 0,
                };
                this._s1SepHistory = [];
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
