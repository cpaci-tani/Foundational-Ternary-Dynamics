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
        this.s1  = { diag: null, extended: null, runtime: null };
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
        this._lastTick5 = -1;

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
        this.peCoulombPE = new RingBuffer(200);
        this.peGravityPE = new RingBuffer(200);
        this.peTotal    = new RingBuffer(200);  // KE + PE
        this.peEnergyDrift = new RingBuffer(200);
        this.peCount    = new RingBuffer(200);
        this.peLockedCount = new RingBuffer(200);
        this.peMobileCount = new RingBuffer(200);
        this.peMomentum = new RingBuffer(200);
        this.peAngMom   = new RingBuffer(200);
        this.peVirial   = new RingBuffer(200);
        this.peTemperature = new RingBuffer(200);
        this.peRmsVelocity = new RingBuffer(200);
        this.peSystemRadius = new RingBuffer(200);
        this.peMaxForce = new RingBuffer(200);
        this.peMeanForce = new RingBuffer(200);
        this.peSeparation = new RingBuffer(200);
        this.peRadialVelocity = new RingBuffer(200);
        this._peInitialEnergy = null;

        // ── Scale 2/3 — Atom / Molecule Engine (200-sample)
        // All values are SIM UNITS (implicit k_B = 1; audit P0-10) — panels
        // label them "(sim)", never MeV / Kelvin. No aeMaxForce buffer on
        // purpose: aeGetForceDecomposition is O(N²) and visibility-gated, so
        // an always-collected force channel would either pay that cost every
        // tick or sit dead when arrows are hidden (the B1 dead-buffer class).
        this.aeKE        = new RingBuffer(200);
        this.aeTemp      = new RingBuffer(200);
        this.aeEnergy    = new RingBuffer(200);
        this.aeBonds     = new RingBuffer(200);
        this.aePEIonic   = new RingBuffer(200);
        this.aePEVdw     = new RingBuffer(200);
        this.aePEBond    = new RingBuffer(200);
        this.aeMomentum  = new RingBuffer(200);
        this.aeAtomCount = new RingBuffer(200);
        this.aeDrift     = new RingBuffer(200);
        this._aeInitialEnergy = null;

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

        const currentTick = diag.tick || 0;
        if (currentTick !== this._lastTick0) {
            this._lastTick0 = currentTick;

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
            const currentH = audit.totalEnergy || 0;
            if (this._initialEnergy === undefined || this._initialEnergy === null) {
                if (currentH > 0.001) this._initialEnergy = currentH;
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
        const coulombPE = diag.coulombPE || 0;
        const gravityPE = diag.gravityPE || 0;
        const totalEnergy = diag.totalEnergy ?? (ke + pe);
        const pMag = diag.totalMomentum ?? diag.momentum ??
            Math.sqrt((diag.momentumX || 0) ** 2 + (diag.momentumY || 0) ** 2 + (diag.momentumZ || 0) ** 2);
        const lMag = diag.totalAngMom ?? diag.angularMomentum ??
            Math.sqrt((diag.angMomX || 0) ** 2 + (diag.angMomY || 0) ** 2 + (diag.angMomZ || 0) ** 2);
        const virial = diag.virialRatio ?? (pe !== 0 ? (2 * ke / Math.abs(pe)) : 0);
        if (this._peInitialEnergy === null && Math.abs(totalEnergy) > 1e-12) {
            this._peInitialEnergy = totalEnergy;
        }
        const energyDrift = this._peInitialEnergy
            ? ((totalEnergy - this._peInitialEnergy) / Math.abs(this._peInitialEnergy)) * 100
            : 0;
        const temperature = cnt > 0 ? (2 / 3) * ke / cnt : 0;
        const toggleNames = [
            'coulomb', 'gravity', 'damping', 'lorentz', 'exchange',
            'strong', 'magnetic_dipole', 'spin_orbit', 'radiation',
            'relativistic', 'relativistic_verlet',
        ];
        const toggles = {};
        for (const name of toggleNames) toggles[name] = !!bridge.peGetToggle?.(name);
        let scenario = '';
        let softening = 0;
        if (typeof document !== 'undefined') {
            const scenarioSelect = document.getElementById('pe-scenario-select');
            scenario = scenarioSelect?.selectedOptions?.[0]?.textContent || scenarioSelect?.value || '';
            softening = Number.parseFloat(document.getElementById('pe-soft-slider')?.value || '0') || 0;
        }
        this.s1.runtime = {
            scenario,
            dt: bridge.peGetDt?.() ?? 0,
            softening,
            toggles,
            capabilities: bridge.peGetBackendCapabilities?.() ?? null,
        };

        const currentTick = diag.tick || 0;
        if (currentTick !== this._lastTick1) {
            this._lastTick1 = currentTick;

            this.peKE.push(ke);
            this.pePE.push(pe);
            this.peCoulombPE.push(coulombPE);
            this.peGravityPE.push(gravityPE);
            this.peTotal.push(totalEnergy);
            this.peEnergyDrift.push(energyDrift);
            this.peCount.push(cnt);
            this.peMomentum.push(pMag);
            this.peAngMom.push(lMag);
            this.peVirial.push(virial);
            this.peTemperature.push(temperature);
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
                v2sum += vx * vx + vy * vy + vz * vz;
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

                this.peLockedCount.push(locked);
                this.peMobileCount.push(Math.max(0, n - locked));
                this.peRmsVelocity.push(n > 0 ? Math.sqrt(v2sum / n) : 0);
                this.peSystemRadius.push(systemRadius);
                this.peMaxForce.push(maxForce);
                this.peMeanForce.push(n > 0 ? sumForce / n : 0);
                this.peSeparation.push(separation);
                this.peRadialVelocity.push(radialVelocity);
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

            this.aeKE.push(ke);
            this.aeTemp.push(diag.temperature || 0);
            this.aeEnergy.push(totalEnergy);
            this.aeBonds.push(diag.bondCount || 0);
            this.aePEIonic.push(diag.totalPEIonic || 0);
            this.aePEVdw.push(diag.totalPEVdw || 0);
            this.aePEBond.push(diag.totalPEBond || 0);
            this.aeMomentum.push(pMag);
            this.aeAtomCount.push(diag.atomCount || 0);
            this.aeDrift.push(energyDrift);
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

            this.csBodies.push(diag.bodyCount  || diag.count || 0);
            // Bridge emits diag.hubbleParameter; keep legacy aliases for forward-compat.
            // (audit P0-3 fix, 2026-05-27 — csHubble was dead-on-arrival because
            // neither 'hubble' nor 'hubbleParam' matched the emitted key.)
            this.csHubble.push(diag.hubbleParameter || diag.hubble || diag.hubbleParam || 0);
            this.csDM.push(    diag.darkMatter || diag.dmFraction  || 0);
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
                this._lastAuditVersion = -1;
                this._prevWantAudit = false;
                this._prevWantLag = false;
                this._lastTick0 = -1;
                this._lastAuditTick = -1;
                this._lastLagTick = -1;
                break;
            case 1:
                for (const b of [
                    this.peKE, this.pePE, this.peCoulombPE, this.peGravityPE,
                    this.peTotal, this.peEnergyDrift, this.peCount,
                    this.peLockedCount, this.peMobileCount,
                    this.peMomentum, this.peAngMom, this.peVirial,
                    this.peTemperature, this.peRmsVelocity, this.peSystemRadius,
                    this.peMaxForce, this.peMeanForce,
                    this.peSeparation, this.peRadialVelocity,
                ]) b.clear();
                this.s1 = { diag: null, extended: null, runtime: null };
                this._peInitialEnergy = null;
                this._lastTick1 = -1;
                this._lastTick1Ext = -1;
                break;
            case 2:
            case 3:
                for (const b of [
                    this.aeKE, this.aeTemp, this.aeEnergy, this.aeBonds,
                    this.aePEIonic, this.aePEVdw, this.aePEBond,
                    this.aeMomentum, this.aeAtomCount, this.aeDrift,
                ]) b.clear();
                this.s2 = { diag: null, runtime: null };
                this._aeInitialEnergy = null;
                this._lastTick2 = -1;
                break;
            case 5:
                for (const b of [this.csBodies, this.csHubble, this.csDM]) b.clear();
                this.s5 = { diag: null, cosmic: null };
                this._lastTick5 = -1;
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
