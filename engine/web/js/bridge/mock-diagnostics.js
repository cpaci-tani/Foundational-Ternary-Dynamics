/**
 * Scale-0 diagnostics — MockBridge side only.
 *
 * Three public readouts consumed by the diagnostics panel + charts:
 *
 *   getDiagnostics()   per-tick particle + energy summary
 *   getEnergyAudit()   detailed energy decomposition (field, wave, EM, dual)
 *   getLagrangian()    Lagrangian density terms
 *
 * Plus one internal helper:
 *
 *   ensureEnergyCache()  single-pass fill of the three `_cachedXxx` fields
 *                        AND of `_fluxMag[]` (piggyback — saves a second
 *                        full-lattice sqrt loop). Reads + mutates the
 *                        MockBridge instance so cache invalidation from
 *                        the bridge side (reset / setScale0Tick /
 *                        setScale0FluxBuffer / setScale0WaveBuffer) stays
 *                        authoritative.
 *
 * Extracted from `wasm-bridge-dag.js` as Wave 1 ticket 3 of the large-file
 * refactor (see docs/SPEC_REFACTOR_LARGE_FILES.md §4). The extraction is
 * a move, not a rewrite — bodies preserved verbatim; the only structural
 * change is that `this._xxx` field reads/writes go through the live
 * `state` reference instead of direct `this.` access.
 *
 * STATE CONTRACT — `state` must expose (all live references on the
 * MockBridge instance):
 *   Read:
 *     latticeSize       number
 *     _tick             number
 *     _dt               number
 *     _physicalTime     number
 *     _particles        Array
 *     _fluxJ            Float64Array|null
 *     _fluxWV           Float64Array|null
 *     _params           { damping: number }
 *   Read + write (cache fields — writes propagate back through ref):
 *     _energyCacheTick     number
 *     _cachedFieldEnergy   number
 *     _cachedWaveEnergy    number
 *     _cachedFluxMag       number
 *     _fluxMag             Float64Array|null  (also filled by this module)
 *     _fluxDirty           boolean            (cleared by ensureEnergyCache)
 *
 * The MockBridge cache-invalidation sites (4 of them) write directly to
 * `_energyCacheTick = -1` on the MockBridge instance; because `state` IS
 * the MockBridge instance (not a destructured copy), those writes are
 * immediately visible to `ensureEnergyCache()` here. This is the contract
 * protected by Risk 3 in the refactor spec — do NOT destructure `state`.
 */

/**
 * Build the diagnostics object bound to the given bridge-like state.
 *
 * @param {object} state — MockBridge instance (live reference).
 * @returns {object} { getDiagnostics, getEnergyAudit, getLagrangian, ensureEnergyCache }
 */
export function createDiagnosticsProvider(state) {
    /**
     * Compute and cache field/wave energy sums from _fluxJ/_fluxWV.
     * Called once per tick at most; subsequent calls return cached values.
     * At L=128 this avoids 3x redundant O(2M) loops per diagnostics frame.
     *
     * Also populates _fluxMag[] in the same pass, so _updateFluxMag() becomes
     * a no-op when the energy cache is fresh. This eliminates a second full-
     * lattice sqrt loop (2M sqrt calls saved at L=128).
     */
    function ensureEnergyCache() {
        if (state._energyCacheTick === state._tick) return;
        state._energyCacheTick = state._tick;
        let fieldE = 0, waveE = 0, fluxMag = 0;
        if (state._fluxJ) {
            const total = state.latticeSize ** 3;
            const J = state._fluxJ, WV = state._fluxWV;
            const M = state._fluxMag; // also fill magnitude cache
            for (let i = 0, k = 0; i < total; i++, k += 3) {
                const jx = J[k], jy = J[k + 1], jz = J[k + 2];
                const mag2 = jx * jx + jy * jy + jz * jz;
                fieldE += mag2;
                const m = Math.sqrt(mag2);
                fluxMag += m;
                if (M) M[i] = m;  // piggyback: fill _fluxMag in same pass
                const wx = WV[k], wy = WV[k + 1], wz = WV[k + 2];
                waveE += wx * wx + wy * wy + wz * wz;
            }
            fieldE *= 0.5;
            waveE *= 0.5;
            // _fluxMag is now fresh — clear dirty flag so _updateFluxMag() is a no-op
            if (M) state._fluxDirty = false;
        }
        state._cachedFieldEnergy = fieldE;
        state._cachedWaveEnergy = waveE;
        state._cachedFluxMag = fluxMag;
    }

    function getDiagnostics() {
        // Single-pass counting (replaces 8x .filter() per frame)
        let manifestedCount = 0, positive = 0, negative = 0;
        let spinUp = 0, spinDown = 0;
        let colorless = 0, colorRed = 0, colorGreen = 0, colorBlue = 0;
        let totalFlux = 0;
        for (let i = 0; i < state._particles.length; i++) {
            const p = state._particles[i];
            totalFlux += p.density;
            if (p.state === 0) continue;
            manifestedCount++;
            if (p.state === 1) positive++;
            else if (p.state === -1) negative++;
            if (p.spin === 1) spinUp++;
            else if (p.spin === -1) spinDown++;
            if (!p.color || p.color === 0) colorless++;
            else if (p.color === 1) colorRed++;
            else if (p.color === 2) colorGreen++;
            else if (p.color === 3) colorBlue++;
        }

        // Use cached energy sums (computed once per tick, avoids redundant O(L^3) loop)
        ensureEnergyCache();
        const fieldEnergy = state._cachedFieldEnergy;
        const waveEnergy = state._cachedWaveEnergy;
        if (state._fluxJ) {
            totalFlux = Math.sqrt(fieldEnergy * 2);  // RMS flux magnitude
        }

        const totalEnergy = fieldEnergy + waveEnergy;
        return {
            tick: state._tick, physicalTime: state._physicalTime, dt: state._dt,
            manifested: manifestedCount, positive, negative,
            totalFlux: +totalFlux.toFixed(4),
            totalEnergy: +totalEnergy.toFixed(4),
            maxBandwidth: 0, avgDrag: 0,
            entropy: totalEnergy > 0 ? Math.log(totalEnergy + 1) : 0,
            chargeBalance: positive - negative,
            spinUp, spinDown, colorless, colorRed, colorGreen, colorBlue,
            angMomX: 0, angMomY: 0, angMomZ: 0
        };
    }

    function getEnergyAudit() {
        // Use cached energy sums (computed once per tick via ensureEnergyCache)
        ensureEnergyCache();
        const fieldEnergy = state._cachedFieldEnergy;
        const waveEnergy = state._cachedWaveEnergy;
        // E = -wave_vel, B = curl(J) — compute EM field energies
        const EFieldEnergy = waveEnergy; // |E|^2/2 = |wave_vel|^2/2
        const BFieldEnergy = 0;
        const poyntingX = 0, poyntingY = 0, poyntingZ = 0;
        // Dual substrate energies
        const ELTotal = 0, ERTotal = 0, wvLTotal = 0, wvRTotal = 0, chiralityTotal = 0;

        return {
            fieldEnergy, waveEnergy, particleKE: 0,
            totalEnergy: fieldEnergy + waveEnergy,
            EFieldEnergy, BFieldEnergy,
            totalPoynting: { x: poyntingX, y: poyntingY, z: poyntingZ },
            gaussViolation: 0, maxGaussError: 0, selfFieldInjection: 0,
            coulombPE: 0, chargeTotal: 0, manifested: 0,
            ELTotal, ERTotal, chiralityTotal, wvLTotal, wvRTotal,
        };
    }

    function getLagrangian() {
        // Count manifested particles without allocating a filtered array
        let N = 0;
        for (let i = 0; i < state._particles.length; i++) {
            if (state._particles[i].state !== 0) N++;
        }
        // Use cached energy sums (computed once per tick via ensureEnergyCache)
        ensureEnergyCache();
        const fieldEnergy = state._cachedFieldEnergy;
        const waveEnergy = state._cachedWaveEnergy;
        const totalFluxMag = state._cachedFluxMag;
        const dissipation = (fieldEnergy + waveEnergy) * state._params.damping;
        const total = waveEnergy + fieldEnergy;
        return {
            fieldKinetic: waveEnergy,       // ½|wave_vel|² (field kinetic energy)
            fieldGradient: -fieldEnergy,    // -½c²|∇J|² (approximated from field energy)
            bornInfeld: 0,                  // -K_B√(1-v²) (zero in MockBridge)
            coupling: 0,                    // g_c·s·∇·J (zero without particles)
            velocity: 0,                    // g_c·s·(v·J) (zero without particles)
            gauss: 0,                       // Gauss constraint (zero in free wave)
            dissipation,                    // γ·½|J|²
            total,
            hamiltonian: total,
            totalAction: total,
            gaussViolation: 0, maxGaussError: 0,
            totalFluxMag, totalWaveEnergy: waveEnergy,
            manifested: N, locked: 0
        };
    }

    return { getDiagnostics, getEnergyAudit, getLagrangian, ensureEnergyCache };
}
