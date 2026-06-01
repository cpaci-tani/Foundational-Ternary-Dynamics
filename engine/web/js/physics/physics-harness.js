/**
 * PhysicsHarness — single canonical surface for reading and writing
 * Scale-0 lattice physics state.
 *
 * STATUS: LIVE (wired into the running dashboard). Disposition verified
 * 2026-06-01 (ticket W3-3). Consumer chain:
 *   index.html → js/app.js → physics/index.js (getPhysicsHarness factory,
 *   lazily constructs ONE PhysicsHarness per bridge) → this module.
 * Runtime consumers (all reached via app.js, which calls their init fns):
 *   • scales/scale0/ui/overlays/p1-observables-panel.js   — imports the
 *     harness + the `getParticleCharge` / `findOppositeChargePairFromList`
 *     helpers; init'd by app.js initP1ObservablesPanel().
 *   • scales/scale0/ui/overlays/conservation-micropanel.js — reads totals
 *     via getPhysicsHarness(); init'd by app.js initConservationMicropanel().
 * The two free helpers below (getParticleCharge / findOppositeChargePairFromList)
 * are also re-exported from physics/index.js for panels that hold a raw
 * snapshot rather than a harness handle. Do not delete or inline this
 * module: removing it breaks the .charge/.q backend-drift papering that
 * the panels rely on (see AUDIT_WEB_ENGINE_2026-05-27 §particle-drift).
 *
 * The harness wraps a bridge (MockBridge or WasmBridge) and does not
 * replace it: `harness.bridge` exposes the underlying instance.
 * Getters return plain data snapshots (mutations don't propagate to
 * the engine). Setters route to bridge methods that exist on both
 * backends, papering over per-backend duck-typing differences in one
 * place rather than at every callsite.
 *
 * Scenario dispatch (post-C-3 inversion + post-A-1 cleanup): the
 * harness defers `setupScenario` to the underlying bridge. The C++
 * (WASM) implementation is canonical when present; the MockBridge
 * native scenario library handles the JS-only path. There is no
 * intermediate JS registry — the previous five-scenario migration
 * mirror was retired once both bridges' own scenario libraries had
 * absorbed the drift fixes (correct N/12 hydrogen radius, toggle
 * propagation, post-injection spin/color/locked).
 */

// ── Particle accessor helpers ───────────────────────────────────────
//
// Particle records returned by bridges duck-type their charge field —
// MockBridge uses `charge`, the WASM bridge uses `q`, and seed
// scenarios (pre-manifestation) only carry the ternary `state`. Every
// consumer needs the same fallback chain. Centralising it here lets
// panels and overlays import a single accessor instead of duplicating
// `p.charge ?? p.q ?? p.state ?? 0` everywhere — see audit ticket C-2.

/**
 * Canonical particle-charge accessor. Falls back through the duck-type
 * chain (charge → q → state) and returns 0 for a null/undefined
 * particle. Optional `defaultValue` overrides the final fallback (used
 * in a few sites that prefer ±1 over 0 when the particle is missing
 * all three fields — e.g. paired-particle Coulomb formulas).
 */
export function getParticleCharge(p, defaultValue = 0) {
    if (!p) return defaultValue;
    return p.charge ?? p.q ?? p.state ?? defaultValue;
}

/**
 * Scan an arbitrary particle list for the first opposite-charge pair.
 * Standalone form so panels that already hold a snapshot don't need a
 * second bridge round-trip via `harness.findOppositeChargePair()`.
 * Returns `{pPos, pNeg}` or null.
 */
export function findOppositeChargePairFromList(particles) {
    if (!particles || particles.length < 2) return null;
    let pPos = null;
    let pNeg = null;
    for (const p of particles) {
        const q = getParticleCharge(p);
        if (q > 0 && !pPos) pPos = p;
        if (q < 0 && !pNeg) pNeg = p;
        if (pPos && pNeg) return { pPos, pNeg };
    }
    return null;
}

// ── Harness class ───────────────────────────────────────────────────

export class PhysicsHarness {
    /**
     * @param {object} bridge - the WasmBridge or MockBridge instance
     */
    constructor(bridge) {
        this.bridge = bridge;
    }

    // ── Getters: lattice state ─────────────────────────────────────

    /** Lattice edge length (voxels per side). */
    getLatticeSize() { return this.bridge?.latticeSize ?? 32; }

    /** Current simulation tick. */
    getTick() { return this.bridge?.getDiagnostics?.()?.tick ?? 0; }

    /**
     * Diagnostics snapshot (energy, charge, angular momentum totals).
     * Plain data — mutations do not affect the engine.
     */
    getDiagnostics() {
        const d = this.bridge?.getDiagnostics?.() || null;
        return d ? { ...d } : null;
    }

    /** Energy audit snapshot (field/wave/particle KE, Poynting, charge). */
    getEnergyAudit() {
        const a = this.bridge?.getEnergyAudit?.() || null;
        return a ? { ...a } : null;
    }

    /**
     * Aggregated conservation totals — single canonical source for
     * the conservation panel and any sweep-time invariant checks.
     */
    getConservationTotals() {
        const d = this.bridge?.getDiagnostics?.() || null;
        const a = this.bridge?.getEnergyAudit?.() || null;
        if (!d) return null;
        return {
            tick:    d.tick ?? 0,
            E:       d.totalEnergy ?? a?.totalEnergy ?? 0,
            // Field momentum proxy (Poynting integral) — true particle
            // momentum sum is a follow-up engine accumulator.
            px:      a?.totalPoynting?.x ?? 0,
            py:      a?.totalPoynting?.y ?? 0,
            pz:      a?.totalPoynting?.z ?? 0,
            Lx:      d.angMomX ?? 0,
            Ly:      d.angMomY ?? 0,
            Lz:      d.angMomZ ?? 0,
            Q:       d.chargeBalance ?? a?.chargeTotal ?? 0,
        };
    }

    // ── Getters: particles ─────────────────────────────────────────

    /** Full manifested-particle list (plain data, mutations do not propagate). */
    getParticleList() {
        const list = this.bridge?.getScale0ParticleList?.() || [];
        return list.map((p) => ({ ...p }));
    }

    /** First opposite-charge pair found in the manifested list (or null). */
    findOppositeChargePair() {
        return findOppositeChargePairFromList(this.getParticleList());
    }

    /** Static accessor mirror — `harness.getParticleCharge(p)` works
     *  for callers that already have a harness handle. */
    getParticleCharge(p, defaultValue = 0) {
        return getParticleCharge(p, defaultValue);
    }

    /** Spin readout for a tracked particle — see g-2 panel for usage. */
    getParticleSpin(particleId) {
        const ps = this.getParticleList();
        const p = ps.find((pp) => pp.id === particleId);
        if (!p) return null;
        // Honest scalar spin (z-projection only); engine has no 3D spin axis yet.
        // omega_z is computed externally from a ring buffer of consecutive samples.
        return { sz: p.spin ?? 0, omega_z: 0 };
    }

    // ── Getters: field samplers ────────────────────────────────────

    /** E-field bulk sampler — { positions, vectors, count }. */
    sampleEField(stride = 1) {
        return this.bridge?.getEFieldSampled?.(stride) || { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
    }

    /** B-field bulk sampler. */
    sampleBField(stride = 1) {
        return this.bridge?.getBFieldSampled?.(stride) || { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
    }

    /** Latency proxy (gravitational time-dilation field). */
    sampleLatency(stride = 2) {
        return this.bridge?.getLatencySampled?.(stride) || { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
    }

    /** Direct Coulomb-potential ray sampler — uses WASM if available. */
    sampleVAtRay(p1, p2, n) {
        if (typeof this.bridge?.sampleVAtRay === 'function') {
            return this.bridge.sampleVAtRay(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, n);
        }
        return { positions: new Float32Array(0), V: new Float32Array(0), count: 0 };
    }

    /**
     * |E| samples along an arbitrary ray, via JS-side trilinear
     * interpolation of `getEFieldSampled` output. Used by the P1
     * Coulomb panel as the MockBridge fallback path when WASM
     * `sampleVAtRay` (which returns V, not E) is unavailable.
     *
     * Returns null if no E-field samples exist (toggle off / scenario
     * hasn't seeded the field) or if the ray is degenerate.
     * Otherwise: array of `{r, E_mag, E_dot_rhat}` of length n.
     */
    sampleEFieldAlongRay(p1, p2, n) {
        const bridge = this.bridge;
        if (!bridge || typeof bridge.getEFieldSampled !== 'function') return null;
        const efs = bridge.getEFieldSampled(1);
        if (!efs || efs.count === 0) return null;
        const latticeSize = bridge.latticeSize || 32;
        const dx = p2.x - p1.x;
        const dy = p2.y - p1.y;
        const dz = p2.z - p1.z;
        const dist = Math.sqrt(dx * dx + dy * dy + dz * dz);
        if (dist < 1) return null;
        const rhat = [dx / dist, dy / dist, dz / dist];

        // Position-index Map cache lives ON THE HARNESS, keyed by
        // (latticeSize, count) — NOT mutated onto the bridge-owned `efs`
        // object. The bridge may either (a) reuse one buffer across ticks
        // [WasmBridge: positions get rewritten in place; mutating the
        // shared object would falsify keys on the next call] or (b) emit
        // a fresh object every call [MockBridge post-RF-7: mutating it
        // builds a Map for a transient that GCs immediately, so the
        // cache never warms]. Both cases are wrong. Keying on
        // (latticeSize, count) gives a stable identity that matches
        // either backend and lets the cache survive across ticks while
        // still invalidating on lattice resize. (Audit Bridge-H4 fix,
        // 2026-04-27.)
        const cacheKey = `${latticeSize}:${efs.count}`;
        let cached = this._efsIndex;
        if (!cached || cached.key !== cacheKey) {
            const map = new Map();
            for (let i = 0; i < efs.count; i++) {
                const gx = Math.floor(efs.positions[i * 3]);
                const gy = Math.floor(efs.positions[i * 3 + 1]);
                const gz = Math.floor(efs.positions[i * 3 + 2]);
                map.set(gx + ',' + gy + ',' + gz, i);
            }
            cached = {
                key: cacheKey,
                map,
                stride: Math.max(1, Math.round(latticeSize / Math.cbrt(efs.count))),
            };
            this._efsIndex = cached;
        }
        const stride = cached.stride;
        const indexMap = cached.map;

        function getE(gx, gy, gz) {
            const wx = ((gx % latticeSize) + latticeSize) % latticeSize;
            const wy = ((gy % latticeSize) + latticeSize) % latticeSize;
            const wz = ((gz % latticeSize) + latticeSize) % latticeSize;
            const i = indexMap.get(wx + ',' + wy + ',' + wz);
            if (i == null) return [0, 0, 0];
            return [efs.vectors[i * 3], efs.vectors[i * 3 + 1], efs.vectors[i * 3 + 2]];
        }

        const samples = new Array(n);
        const rMin = 0.5;
        const rMax = dist * 0.85;
        for (let i = 0; i < n; i++) {
            const r = rMin + (rMax - rMin) * (i / (n - 1));
            const t = r / dist;
            const x = p1.x + dx * t;
            const y = p1.y + dy * t;
            const z = p1.z + dz * t;
            const sx = Math.floor(x / stride) * stride;
            const sy = Math.floor(y / stride) * stride;
            const sz = Math.floor(z / stride) * stride;
            const fx = (x - sx) / stride;
            const fy = (y - sy) / stride;
            const fz = (z - sz) / stride;
            const c000 = getE(sx, sy, sz);
            const c100 = getE(sx + stride, sy, sz);
            const c010 = getE(sx, sy + stride, sz);
            const c110 = getE(sx + stride, sy + stride, sz);
            const c001 = getE(sx, sy, sz + stride);
            const c101 = getE(sx + stride, sy, sz + stride);
            const c011 = getE(sx, sy + stride, sz + stride);
            const c111 = getE(sx + stride, sy + stride, sz + stride);
            const E = [0, 0, 0];
            for (let k = 0; k < 3; k++) {
                const c00 = c000[k] * (1 - fx) + c100[k] * fx;
                const c10 = c010[k] * (1 - fx) + c110[k] * fx;
                const c01 = c001[k] * (1 - fx) + c101[k] * fx;
                const c11 = c011[k] * (1 - fx) + c111[k] * fx;
                const c0 = c00 * (1 - fy) + c10 * fy;
                const c1 = c01 * (1 - fy) + c11 * fy;
                E[k] = c0 * (1 - fz) + c1 * fz;
            }
            const Emag = Math.sqrt(E[0] * E[0] + E[1] * E[1] + E[2] * E[2]);
            const Edot = E[0] * rhat[0] + E[1] * rhat[1] + E[2] * rhat[2];
            samples[i] = { r, E_mag: Emag, E_dot_rhat: Edot };
        }
        return samples;
    }

    /** Flux 2D slice — { axis, mid, data, n }. */
    getFluxSlice(axis, mid) {
        return this.bridge?.getFluxSlice?.(axis, mid) || null;
    }

    // ── Setters: toggles (drift point #1 fix) ──────────────────────

    /**
     * Read a toggle state. Returns false when the bridge doesn't
     * expose the toggle (silently — mirrors current MockBridge behavior).
     */
    getToggle(name) {
        try {
            if (typeof this.bridge?.getToggle === 'function') {
                return !!this.bridge.getToggle(name);
            }
            // MockBridge stores toggles as plain object property
            const t = this.bridge?._toggles ?? this.bridge?.toggles;
            return !!(t && t[name]);
        } catch { return false; }
    }

    /** Set a toggle on the underlying bridge. */
    setToggle(name, value) {
        const b = this.bridge;
        if (!b) return;
        if (typeof b.setToggle === 'function') {
            b.setToggle(name, !!value);
        } else if (b._toggles && name in b._toggles) {
            b._toggles[name] = !!value;
        } else if (b.toggles && name in b.toggles) {
            b.toggles[name] = !!value;
        }
    }

    // ── Setters: injection ─────────────────────────────────────────

    /**
     * Inject a manifested particle, applying optional spin/color/locked
     * /density at injection time. Mirrors the C++ IPF(rb, x, y, z,
     * state, spin, color) + LOCK(rb, x, y, z) signature, papering over
     * the MockBridge convention of post-mutating the last entry.
     */
    injectParticle(x, y, z, state, opts = {}) {
        const before = (this.bridge?._particles?.length ?? 0);
        this.bridge?.injectParticle?.(x, y, z, state);
        const after = (this.bridge?._particles?.length ?? 0);
        // Apply spin/color/locked to the just-injected particle, if any.
        if (after > before && opts) {
            const last = this.bridge._particles[after - 1];
            if (last) {
                if (Number.isFinite(opts.spin))   last.spin = opts.spin;
                if (Number.isFinite(opts.color))  last.color = opts.color;
                if (typeof opts.locked === 'boolean') last.locked = opts.locked;
                if (Number.isFinite(opts.density)) last.density = opts.density;
            }
        }
    }

    /**
     * Inject flux at a voxel. Additive — repeated calls accumulate.
     * Mirrors C++ IF macro / inject_flux_add.
     */
    injectFlux(x, y, z, fx, fy, fz) {
        if (typeof this.bridge?._injectFlux === 'function') {
            this.bridge._injectFlux(x, y, z, fx, fy, fz);
        } else if (typeof this.bridge?.injectFlux === 'function') {
            this.bridge.injectFlux(x, y, z, fx, fy, fz);
        }
    }

    /**
     * Inject wave-velocity at a voxel. Additive.
     * Mirrors C++ IW macro / inject_wave_vel_add.
     */
    injectWaveVel(x, y, z, vx, vy, vz) {
        if (typeof this.bridge?._injectWaveVel === 'function') {
            this.bridge._injectWaveVel(x, y, z, vx, vy, vz);
        } else if (typeof this.bridge?.injectWaveVel === 'function') {
            this.bridge.injectWaveVel(x, y, z, vx, vy, vz);
        }
    }

    /** Create an entangled flux pair (genesis-coupled). */
    createEntangledPair(x, y, z, fx, fy, fz) {
        this.bridge?.createEntangledPair?.(x, y, z, fx, fy, fz);
    }

    /** Inject a flux/state wavepacket. */
    injectWavepacket(x, y, z, state) {
        this.bridge?.injectWavepacket?.(x, y, z, state);
    }

    // ── Scenario orchestration ─────────────────────────────────────

    /** Reset the lattice (clear particles, flux, wave-velocity, tick). */
    reset() {
        this.bridge?.reset?.();
    }

    /**
     * Set up a scenario by name. Defers to the underlying bridge: the
     * WASM bridge dispatches to the canonical C++ scenario library
     * (`engine/src/scenarios/*.cpp`); the MockBridge dispatches to its
     * native JS scenario library (`engine/web/js/bridge/scenarios/*`).
     * Returns true iff the bridge handled the name.
     */
    setupScenario(name) {
        const bridge = this.bridge;
        if (typeof bridge?.setupScenario !== 'function') return false;
        bridge.setupScenario(name);
        return true;
    }
}

