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
 *                        the bridge side (reset / per-tick advance /
 *                        flux-wave injection mutators) stays authoritative.
 *
 * Extracted from `bridge-init.js` as Wave 1 ticket 3 of the large-file
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

import { ALPHA, COULOMB_K_PE } from '../constants.js';

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

    /**
     * Lattice-derived field diagnostics: B-field energy via curl(J),
     * Poynting vector via -wave_vel × B, Gauss violation via div(J)−s,
     * and particle kinetic energy + angular momentum.
     *
     * Single triple-nested O(L³) pass with periodic boundary conditions
     * matching the C++ engine's `compute_energy_audit` (engine/src/
     * diagnostics_compute.cpp:83). At L=32 this is ~33 K voxels, trivially
     * cheap to compute on the diagnostics tab's 4 Hz cadence.
     *
     * Cached on the bridge by tick number so repeated audits within one
     * tick (diagnostics + charts + lagrangian tabs) reuse one computation.
     */
    function ensureFieldDerivedCache() {
        if (state._fieldDerivedCacheTick === state._tick) return;
        state._fieldDerivedCacheTick = state._tick;

        let bEnergy = 0, sx = 0, sy = 0, sz = 0;
        let gaussSumSq = 0, maxGaussErr = 0;

        if (state._fluxJ) {
            const N = state.latticeSize;
            const J = state._fluxJ;
            const WV = state._fluxWV;
            // Build a per-voxel state map so divergence's source term s(x)
            // can be looked up. Only manifested particles contribute, so the map
            // is sparse: skip it entirely when there are no particles.
            //
            // Persistent grow-in-place scratch (SPEC_SCALE0_PERF §7-G): a fresh
            // Int8Array(N³) per audit is ~2 MB/call at L=129 and churns GC on the
            // audit cadence. Instead keep one buffer (reallocated only on resize)
            // and clear just the voxels stamped last call — O(particles), and the
            // map stays all-zero outside this call's particles (bit-identical).
            let stateMap = null;
            if (state._particles.length > 0) {
                if (!state._derivedStateMap || state._derivedStateMapN !== N) {
                    state._derivedStateMap = new Int8Array(N * N * N);
                    state._derivedStateMapN = N;
                    state._derivedStateMapDirty = [];
                }
                stateMap = state._derivedStateMap;
                const dirty = state._derivedStateMapDirty;
                for (let i = 0; i < dirty.length; i++) stateMap[dirty[i]] = 0;
                dirty.length = 0;
                for (let i = 0; i < state._particles.length; i++) {
                    const p = state._particles[i];
                    if (p.state === 0) continue;
                    const px = ((p.x | 0) % N + N) % N;
                    const py = ((p.y | 0) % N + N) % N;
                    const pz = ((p.z | 0) % N + N) % N;
                    const cell = pz * N * N + py * N + px;
                    stateMap[cell] = p.state;
                    dirty.push(cell);
                }
            }

            // Periodic-wrap helper macros (inlined for hot loop perf).
            const idx = (x, y, z) => z * N * N + y * N + x;
            const wrap = (i) => ((i % N) + N) % N;

            for (let z = 0; z < N; z++) {
                const zp = wrap(z + 1), zm = wrap(z - 1);
                for (let y = 0; y < N; y++) {
                    const yp = wrap(y + 1), ym = wrap(y - 1);
                    for (let x = 0; x < N; x++) {
                        const xp = wrap(x + 1), xm = wrap(x - 1);
                        // Central-difference partials of J for curl + divergence.
                        const ixp = idx(xp, y, z) * 3, ixm = idx(xm, y, z) * 3;
                        const iyp = idx(x, yp, z) * 3, iym = idx(x, ym, z) * 3;
                        const izp = idx(x, y, zp) * 3, izm = idx(x, y, zm) * 3;
                        // curl(J)_x = ∂Jz/∂y − ∂Jy/∂z
                        // curl(J)_y = ∂Jx/∂z − ∂Jz/∂x
                        // curl(J)_z = ∂Jy/∂x − ∂Jx/∂y
                        const Bx = 0.5 * ((J[iyp + 2] - J[iym + 2]) - (J[izp + 1] - J[izm + 1]));
                        const By = 0.5 * ((J[izp]     - J[izm])     - (J[ixp + 2] - J[ixm + 2]));
                        const Bz = 0.5 * ((J[ixp + 1] - J[ixm + 1]) - (J[iyp]     - J[iym]));
                        bEnergy += 0.5 * (Bx * Bx + By * By + Bz * Bz);

                        // E = -wave_vel; Poynting = E × B summed.
                        const ic = idx(x, y, z) * 3;
                        const Ex = -WV[ic], Ey = -WV[ic + 1], Ez = -WV[ic + 2];
                        sx += Ey * Bz - Ez * By;
                        sy += Ez * Bx - Ex * Bz;
                        sz += Ex * By - Ey * Bx;

                        // div(J) = ∂Jx/∂x + ∂Jy/∂y + ∂Jz/∂z
                        const divJ = 0.5 * (
                            (J[ixp]     - J[ixm])     +
                            (J[iyp + 1] - J[iym + 1]) +
                            (J[izp + 2] - J[izm + 2])
                        );
                        const s = stateMap ? stateMap[idx(x, y, z)] : 0;
                        const err = divJ - s;
                        gaussSumSq += err * err;
                        const absErr = err < 0 ? -err : err;
                        if (absErr > maxGaussErr) maxGaussErr = absErr;
                    }
                }
            }
        }

        state._cachedBFieldEnergy = bEnergy;
        state._cachedPoynting = { x: sx, y: sy, z: sz };
        state._cachedGaussViolation = gaussSumSq;
        state._cachedMaxGaussError = maxGaussErr;
    }

    /**
     * Particle kinetic energy + angular momentum (origin = lattice center).
     * Cheap O(N_particles); cache on tick.
     */
    function ensureParticleDerivedCache() {
        if (state._particleDerivedCacheTick === state._tick) return;
        state._particleDerivedCacheTick = state._tick;

        let pKE = 0, lx = 0, ly = 0, lz = 0;
        const N = state.latticeSize;
        const cx = (N - 1) * 0.5, cy = (N - 1) * 0.5, cz = (N - 1) * 0.5;
        const ps = state._particles;
        for (let i = 0; i < ps.length; i++) {
            const p = ps[i];
            if (p.state === 0) continue;
            const vx = p.vx | 0 ? p.vx : (p.vx ?? 0);
            const vy = p.vy | 0 ? p.vy : (p.vy ?? 0);
            const vz = p.vz | 0 ? p.vz : (p.vz ?? 0);
            // Mass = density (MockBridge convention, matches K_B*2 default
            // from injectParticle). 0.5 * m * v² is the kinetic energy.
            const m = (typeof p.density === 'number' && p.density > 0) ? p.density : 1.0;
            pKE += 0.5 * m * (vx * vx + vy * vy + vz * vz);
            // Angular momentum L = r × (m·v), origin at lattice center.
            const rx = (p.x ?? 0) - cx;
            const ry = (p.y ?? 0) - cy;
            const rz = (p.z ?? 0) - cz;
            lx += m * (ry * vz - rz * vy);
            ly += m * (rz * vx - rx * vz);
            lz += m * (rx * vy - ry * vx);
        }
        // Pairwise Coulomb PE: U = Σ_{i<j} α · q_i · q_j / r_ij.
        // Mirrors the WASM-side Σ ½α·q·φ_C convention (the ½ avoids
        // double-counting per-pair contributions in the i<j sum).
        // No Poisson solve in MockBridge — the particle list is small
        // enough that direct pairwise summation is the canonical path.
        // Uses COULOMB_K_PE (= ALPHA) named alias for convention attribution
        // (audit P1-6 fix, 2026-05-27).
        let coulombPE = 0;
        for (let i = 0; i < ps.length; i++) {
            const pi = ps[i];
            if (pi.state === 0) continue;
            const xi = pi.x ?? 0, yi = pi.y ?? 0, zi = pi.z ?? 0;
            for (let j = i + 1; j < ps.length; j++) {
                const pj = ps[j];
                if (pj.state === 0) continue;
                const dx = xi - (pj.x ?? 0);
                const dy = yi - (pj.y ?? 0);
                const dz = zi - (pj.z ?? 0);
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                if (r < 1e-6) continue;
                coulombPE += COULOMB_K_PE * pi.state * pj.state / r;
            }
        }
        state._cachedParticleKE = pKE;
        state._cachedAngMom = { x: lx, y: ly, z: lz };
        state._cachedCoulombPE = coulombPE;
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

        // Angular momentum (per-tick cache; cheap to keep wired even when
        // the dashboard's diagnostics tab is idle).
        ensureParticleDerivedCache();
        const L = state._cachedAngMom;

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
            angMomX: L.x, angMomY: L.y, angMomZ: L.z
        };
    }

    function getEnergyAudit() {
        // Use cached energy sums (computed once per tick via ensureEnergyCache)
        ensureEnergyCache();
        ensureFieldDerivedCache();
        ensureParticleDerivedCache();
        const fieldEnergy = state._cachedFieldEnergy;
        const waveEnergy = state._cachedWaveEnergy;
        // E = -wave_vel; |E|²/2 = ½|wave_vel|² = waveEnergy.
        // B = curl(J); |B|²/2 cached via ensureFieldDerivedCache().
        const EFieldEnergy = waveEnergy;
        const BFieldEnergy = state._cachedBFieldEnergy;
        const poynting = state._cachedPoynting;
        const particleKE = state._cachedParticleKE;
        const gaussViolation = state._cachedGaussViolation;
        const maxGaussError = state._cachedMaxGaussError;

        // Dual-substrate metrics: MockBridge has no separate flux_L/flux_R/
        // wave_vel_L/wave_vel_R arrays (those are WasmBridge-only — see
        // engine/src/render_bridge.cpp). Report 0 with a known-flat marker
        // so the dashboard descriptor's compute path resolves rather than
        // falling through to undefined. Switch to the WASM bridge to
        // populate these.
        const ELTotal = 0, ERTotal = 0, wvLTotal = 0, wvRTotal = 0, chiralityTotal = 0;

        // Charge total + manifested count (re-walked here so the audit is
        // self-contained when called outside the diag path).
        let chargeTotal = 0, manifested = 0;
        for (let i = 0; i < state._particles.length; i++) {
            const p = state._particles[i];
            if (p.state === 0) continue;
            chargeTotal += p.state;
            manifested++;
        }

        return {
            fieldEnergy, waveEnergy, particleKE,
            totalEnergy: fieldEnergy + waveEnergy + particleKE,
            EFieldEnergy, BFieldEnergy,
            totalPoynting: poynting,
            gaussViolation, maxGaussError, selfFieldInjection: 0,
            coulombPE: state._cachedCoulombPE ?? 0, chargeTotal, manifested,
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
            total: waveEnergy - fieldEnergy,
            hamiltonian: waveEnergy + fieldEnergy,
            action: (state._cachedAction || 0) + (waveEnergy - fieldEnergy) * state._params.dt,
            totalAction: (state._cachedAction || 0) + (waveEnergy - fieldEnergy) * state._params.dt,
            gaussViolation: 0, maxGaussError: 0,
            totalFluxMag, totalWaveEnergy: waveEnergy,
            manifested: N, locked: 0
        };
    }

    return { getDiagnostics, getEnergyAudit, getLagrangian, ensureEnergyCache };
}
