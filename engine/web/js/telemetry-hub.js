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

// ── Ring Buffer ──────────────────────────────────────────────────────────────
// Exported so chart renderers can type-check and legacy code can import it.

export class RingBuffer {
    constructor(size = 500) {
        this.data  = new Float32Array(size);
        this.size  = size;
        this.head  = 0;
        this.count = 0;
    }

    push(value) {
        this.data[this.head] = isFinite(value) ? value : 0;
        this.head = (this.head + 1) % this.size;
        if (this.count < this.size) this.count++;
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

    clear() { this.head = 0; this.count = 0; }
}

// ── Telemetry Hub ────────────────────────────────────────────────────────────

export class TelemetryHub {
    constructor() {
        // ── Latest snapshots per scale ──────────────────
        this.s0  = { diag: null, audit: null, lagrangian: null };
        this.s1  = { diag: null, extended: null };
        this.s2  = { diag: null };  // also used for scale3
        this.s4  = { diag: null };
        this.s5  = { diag: null, cosmic: null };

        // ── Scale 0 — Lattice / Flux ────────────────────
        // Core diagnostics (500-sample history)
        this.flux      = new RingBuffer(500);  // totalFlux
        this.energy    = new RingBuffer(500);  // totalEnergy
        this.manifested = new RingBuffer(500); // particle count
        this.entropy   = new RingBuffer(500);
        this.charges   = new RingBuffer(500);  // positive − negative
        this.positive  = new RingBuffer(500);
        this.negative  = new RingBuffer(500);

        // Energy audit extras
        this.ebDiff    = new RingBuffer(500);  // E-field energy − B-field energy
        this.gauss     = new RingBuffer(500);  // gaussViolation

        // Per-audit-field trend buffers (500-sample) — drive panel-row sparklines.
        this.aud = {
            fieldEnergy:         new RingBuffer(500),
            waveEnergy:          new RingBuffer(500),
            particleKE:          new RingBuffer(500),
            coulombPE:           new RingBuffer(500),
            eFieldEnergy:        new RingBuffer(500),
            bFieldEnergy:        new RingBuffer(500),
            poyntingMag:         new RingBuffer(500),
            maxGaussError:       new RingBuffer(500),
            selfFieldInjection:  new RingBuffer(500),
            eLeftEnergy:         new RingBuffer(500),
            eRightEnergy:        new RingBuffer(500),
            chirality:           new RingBuffer(500),
            waveLeft:            new RingBuffer(500),
            waveRight:           new RingBuffer(500),
            energyDrift:         new RingBuffer(500),
        };

        // Sparkline-resolution (80-sample) — used by DiagnosticsPanel sparklines
        this.sp = {
            manifested: new RingBuffer(80),
            charges:    new RingBuffer(80),
            flux:       new RingBuffer(80),
            energy:     new RingBuffer(80),
            entropy:    new RingBuffer(80),
        };

        // ── Lagrangian (400-sample, 10 terms) ──────────
        this.lag = {
            fieldKinetic:  new RingBuffer(400),
            fieldGradient: new RingBuffer(400),
            bornInfeld:    new RingBuffer(400),
            coupling:      new RingBuffer(400),
            velocity:      new RingBuffer(400),
            gauss:         new RingBuffer(400),
            dissipation:   new RingBuffer(400),
            total:         new RingBuffer(400),
            hamiltonian:   new RingBuffer(400),
            action:        new RingBuffer(400),
        };

        // ── Scale 1 — Particle Engine (200-sample) ─────
        this.peKE       = new RingBuffer(200);
        this.pePE       = new RingBuffer(200);
        this.peTotal    = new RingBuffer(200);  // KE + PE
        this.peCount    = new RingBuffer(200);
        this.peMomentum = new RingBuffer(200);
        this.peAngMom   = new RingBuffer(200);
        this.peVirial   = new RingBuffer(200);

        // ── Scale 2/3 — Atom / Molecule Engine (200-sample)
        this.aeKE     = new RingBuffer(200);
        this.aeTemp   = new RingBuffer(200);
        this.aeEnergy = new RingBuffer(200);
        this.aeBonds  = new RingBuffer(200);

        // ── Scale 5 — Cosmic (200-sample) ──────────────
        this.csBodies = new RingBuffer(200);
        this.csHubble = new RingBuffer(200);
        this.csDM     = new RingBuffer(200);
    }

    // ── Scale 0 collection ──────────────────────────────────────────────────

    /**
     * Collect Scale 0 diagnostics. Implements the dual-bridge logic:
     * if a JS flux mock is active and WASM has no manifested particles,
     * use the mock snapshot because it owns both the field state and tick.
     * @returns {object} the active diag snapshot
     */
    collectScale0(bridge, fluxMock, useFluxMock) {
        const mainCaps = bridge.capabilities?.scale0;
        if (!mainCaps) return null;

        const mockCaps = useFluxMock ? (fluxMock?.capabilities?.scale0 ?? null) : null;
        const wasmDiag = mainCaps.getScale0Diagnostics();
        const mockDiag = mockCaps ? mockCaps.getScale0Diagnostics() : null;

        const diag = (mockDiag && !wasmDiag.manifested && mockDiag.totalFlux > 0)
            ? mockDiag
            : wasmDiag;

        this.s0.diag = diag;

        // 500-sample buffers for charts
        this.flux.push(diag.totalFlux     || 0);
        this.energy.push(diag.totalEnergy || 0);
        this.manifested.push(diag.manifested || 0);
        this.entropy.push(diag.entropy    || 0);
        this.positive.push(diag.positive  || 0);
        this.negative.push(diag.negative  || 0);
        this.charges.push((diag.positive || 0) - (diag.negative || 0));

        // 80-sample sparkline buffers
        this.sp.manifested.push(diag.manifested || 0);
        this.sp.charges.push((diag.positive || 0) - (diag.negative || 0));
        this.sp.flux.push(diag.totalFlux  || 0);
        this.sp.energy.push(diag.totalEnergy || 0);
        this.sp.entropy.push(diag.entropy || 0);

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
            const currentH = audit.totalEnergy || 0;
            if (this._initialEnergy === undefined || this._initialEnergy === null) {
                if (currentH > 0.001) this._initialEnergy = currentH;
            }
            let drift = 0;
            if (this._initialEnergy) {
                drift = ((currentH - this._initialEnergy) / this._initialEnergy) * 100;
            }
            audit.energyDrift = drift;

            this.ebDiff.push(eF - bF);
            this.gauss.push(audit.gaussViolation || 0);

            // Per-field trend buffers (drive diagnostics table sparklines)
            this.aud.fieldEnergy.push(       audit.fieldEnergy        || 0);
            this.aud.waveEnergy.push(        audit.waveEnergy         || 0);
            this.aud.particleKE.push(        audit.particleKE         || 0);
            this.aud.coulombPE.push(         audit.coulombPE          || 0);
            this.aud.eFieldEnergy.push(      eF);
            this.aud.bFieldEnergy.push(      bF);
            this.aud.poyntingMag.push(       pMag);
            this.aud.maxGaussError.push(     audit.maxGaussError      || 0);
            this.aud.selfFieldInjection.push(audit.selfFieldInjection || 0);
            this.aud.eLeftEnergy.push(       audit.ELTotal || audit.eLTotal || 0);
            this.aud.eRightEnergy.push(      audit.ERTotal || audit.eRTotal || 0);
            this.aud.chirality.push(         audit.chiralityTotal     || 0);
            this.aud.waveLeft.push(          audit.wvLTotal           || 0);
            this.aud.waveRight.push(         audit.wvRTotal           || 0);
            this.aud.energyDrift.push(       drift);
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
            this.lag.fieldKinetic.push( Math.abs(lag.fieldKinetic  || 0));
            this.lag.fieldGradient.push(Math.abs(lag.fieldGradient || 0));
            this.lag.bornInfeld.push(   Math.abs(lag.bornInfeld    || 0));
            this.lag.coupling.push(     Math.abs(lag.coupling      || 0));
            this.lag.velocity.push(     Math.abs(lag.velocity      || 0));
            this.lag.gauss.push(        Math.abs(lag.gauss         || 0));
            this.lag.dissipation.push(  Math.abs(lag.dissipation   || 0));
            this.lag.total.push(         lag.total                  || 0);
            this.lag.hamiltonian.push(   lag.hamiltonian            || 0);
            this.lag.action.push(        lag.totalAction            || 0);
        }
        return lag;
    }

    // ── Scale 1 collection ──────────────────────────────────────────────────

    collectScale1(bridge) {
        const diag = bridge.peGetDiagnostics?.();
        if (!diag) return null;
        this.s1.diag = diag;

        const ke  = diag.totalKE       || diag.kineticEnergy || 0;
        const pe  = diag.totalPE       || diag.potentialEnergy || 0;
        const cnt = diag.particleCount || diag.count || 0;

        this.peKE.push(ke);
        this.pePE.push(pe);
        this.peTotal.push(ke + pe);
        this.peCount.push(cnt);
        this.peMomentum.push(diag.totalMomentum || diag.momentum || 0);
        this.peAngMom.push(diag.totalAngMom || diag.angularMomentum || 0);
        this.peVirial.push(diag.virialRatio || 0);
        return diag;
    }

    collectScale1Extended(bridge) {
        const ext = bridge.peGetExtendedData?.();
        if (ext) this.s1.extended = ext;
        return ext;
    }

    // ── Scale 2/3 collection ────────────────────────────────────────────────

    collectScale2(bridge) {
        const diag = bridge.aeGetDiagnostics?.();
        if (!diag) return null;
        this.s2.diag = diag;

        this.aeKE.push(diag.kineticEnergy || 0);
        this.aeTemp.push(diag.temperature || 0);
        this.aeEnergy.push((diag.kineticEnergy || 0) + (diag.totalPotential || diag.potentialEnergy || 0));
        this.aeBonds.push(diag.bondCount || 0);
        return diag;
    }

    // ── Scale 5 collection ──────────────────────────────────────────────────

    collectScale5(cosmicBridge) {
        if (!cosmicBridge) return null;
        const diag = cosmicBridge.getDiagnostics?.();
        if (!diag) return null;
        this.s5.diag = diag;
        this.csBodies.push(diag.bodyCount  || diag.count || 0);
        // Bridge emits diag.hubbleParameter; keep legacy aliases for forward-compat.
        // (audit P0-3 fix, 2026-05-27 — csHubble was dead-on-arrival because
        // neither 'hubble' nor 'hubbleParam' matched the emitted key.)
        this.csHubble.push(diag.hubbleParameter || diag.hubble || diag.hubbleParam || 0);
        this.csDM.push(    diag.darkMatter || diag.dmFraction  || 0);
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
        return {
            temperature:    (2 / 3) * ke / n,   // equipartition T = 2KE/3N
            virialRatio:    d.virialRatio || 0,
            rmsVelocity:    d.rmsVelocity || 0,
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

    resetScale(scale) {
        switch (scale) {
            case 0:
                for (const b of [
                    this.flux, this.energy, this.manifested, this.entropy,
                    this.positive, this.negative, this.charges,
                    this.ebDiff, this.gauss,
                    ...Object.values(this.sp),
                    ...Object.values(this.aud),
                    ...Object.values(this.lag),
                ]) b.clear();
                this.s0 = { diag: null, audit: null, lagrangian: null };
                this._initialEnergy = null;
                break;
            case 1:
                for (const b of [
                    this.peKE, this.pePE, this.peTotal, this.peCount,
                    this.peMomentum, this.peAngMom, this.peVirial,
                ]) b.clear();
                this.s1 = { diag: null, extended: null };
                break;
            case 2:
            case 3:
                for (const b of [this.aeKE, this.aeTemp, this.aeEnergy, this.aeBonds]) b.clear();
                this.s2 = { diag: null };
                break;
            case 5:
                for (const b of [this.csBodies, this.csHubble, this.csDM]) b.clear();
                this.s5 = { diag: null, cosmic: null };
                break;
        }
    }

    resetAll() {
        for (const scale of [0, 1, 2, 5]) this.resetScale(scale);
    }
}

// ── Singleton export ─────────────────────────────────────────────────────────
// All controllers import this instance; no constructor args needed.
export const telemetryHub = new TelemetryHub();
