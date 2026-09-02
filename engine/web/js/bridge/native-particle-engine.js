/**
 * Scale-1 Particle Engine — native C++/WASM adapter.
 *
 * Replaces the retired pure-JS engine (mock-particle-engine.js). Owns one
 * embind `ParticleEngine` instance and forwards the whole `pe*` bridge
 * surface to the native bindings (engine/wasm/bindings_particle.cpp).
 * The native engine is the same Velocity-Verlet KDK integrator with
 * G_PE = G_DERIVED = 1/(4π·m_P²) gravity (FTD-0131) that the CTest suite
 * covers (test_particle_engine, test_pe_forces, …).
 *
 * Embind instance model: `new module.ParticleEngine()` exposes only
 * tick/run/currentTick; every other operation is a module-level free
 * function taking the instance first, e.g. `module.getPEParticleData(pe)`.
 * The instance must be `.delete()`d on teardown (embind heap object).
 *
 * What stays JS-side:
 *   - catalog identity: typeMap<id, catalogId> + pe-catalog-map.js helpers
 *     (spin / color / spin-axis resolution for [PARAMETRIC] Zoo injection)
 *   - equilibrium-orbit seeding: probe the NATIVE force at v=0, solve
 *     m·v²/r = |F_inward|, write the velocity back (peSetVelocity) — same
 *     "ICs derived from the live kernel, not closed-form" contract the JS
 *     engine had, now against the kernel that actually integrates.
 *   - derived read shapes with no native equivalent (peGetFieldSources,
 *     peInspectParticle) — computed from native data, never re-implementing
 *     the force law (peInspectParticle reads the native force buffer).
 *
 * Toggle baseline and claim metadata come from the native Scale-1 registry.
 * The JS adapter never maintains a second physics applicability table.
 *
 * Behavior changes vs the retired JS engine (deliberate, see plan):
 *   - no r=35 boundary reflection (native engine is unbounded)
 *   - selected contact-removal events are explicit and OFF by default
 *   - forces/decomposition come from the one native kernel — the audit's
 *     "second drifted re-implementation" class of defect is structurally gone.
 */

import { C_SPEED, K_B } from '../constants.js';
import { getById } from '../particle-catalog.js';
import {
    catalogColorId, catalogSpin, initSpinAxis, resetColorWheel,
} from './pe-catalog-map.js';

const EMPTY_F32 = new Float32Array(0);
const EMPTY_F64 = new Float64Array(0);
const EMPTY_I8 = new Int8Array(0);
const EMPTY_I32 = new Int32Array(0);
const EMPTY_U8 = new Uint8Array(0);

function emptyParticleData() {
    return {
        positions: EMPTY_F32, colors: EMPTY_F32, sizes: EMPTY_F32,
        charges: EMPTY_I8, ids: EMPTY_I32, velocities: EMPTY_F32,
        masses: EMPTY_F64, rEff: EMPTY_F32, locked: EMPTY_U8,
        spins: EMPTY_I8, colorIds: EMPTY_I8, spinAxes: EMPTY_F32, count: 0,
    };
}

function emptyDiagnostics() {
    return {
        tick: 0, particleCount: 0, totalKE: 0, totalPE: 0, coulombPE: 0,
        gravityPE: 0, totalEnergy: 0, momentumX: 0, momentumY: 0, momentumZ: 0,
        angMomX: 0, angMomY: 0, angMomZ: 0, centerX: 0, centerY: 0, centerZ: 0,
        coveredMask: 0, missingMask: 0, nonconservativeMask: 0,
        stateEnergyComplete: false, driftEligible: false,
        cumulativeDampingSink: 0, cumulativeRadiationSink: 0,
        cumulativeSpeedProjectionSink: 0, cumulativeContactDelta: 0,
        contactEventCount: 0, speedProjectionCount: 0,
    };
}

function emptyDecomposition() {
    return {
        positions: EMPTY_F32, count: 0,
        coulomb: EMPTY_F64, gravity: EMPTY_F64, lorentz: EMPTY_F64,
        exchange: EMPTY_F64, strong: EMPTY_F64, radiation: EMPTY_F64,
        magnetic_dipole: EMPTY_F64, spin_orbit: EMPTY_F64, net: EMPTY_F64,
        maxCoulomb: 0, maxGravity: 0, maxLorentz: 0,
        maxExchange: 0, maxStrong: 0, maxRadiation: 0,
        maxMagneticDipole: 0, maxSpinOrbit: 0, maxNet: 0,
    };
}

function snapshotParticleData(snapshot) {
    const objects = Array.from(snapshot?.objects || []);
    const count = objects.length;
    const positions = new Float32Array(count * 3);
    const velocities = new Float32Array(count * 3);
    const colors = new Float32Array(count * 3);
    const sizes = new Float32Array(count);
    const masses = new Float64Array(count);
    const rEff = new Float32Array(count);
    const charges = new Int8Array(count);
    const ids = new Int32Array(count);
    const locked = new Uint8Array(count);
    const spins = new Int8Array(count);
    const colorIds = new Int8Array(count);
    const spinAxes = new Float32Array(count * 3);
    for (let i = 0; i < count; i++) {
        const object = objects[i];
        positions.set([object.position.x, object.position.y, object.position.z], i * 3);
        velocities.set([object.velocity.x, object.velocity.y, object.velocity.z], i * 3);
        const state = object.effectiveState || 0;
        colors.set(state > 0 ? [0.29, 0.87, 0.50]
            : state < 0 ? [0.97, 0.44, 0.44] : [0.60, 0.60, 0.70], i * 3);
        sizes[i] = object.constituent ? 9 : 7;
        masses[i] = object.massAvailable ? object.mass : 0;
        rEff[i] = object.effectiveRadius || 0.4;
        charges[i] = state;
        ids[i] = object.id;
        locked[i] = object.locked ? 1 : 0;
    }
    return { positions, velocities, colors, sizes, masses, rEff, charges, ids,
        locked, spins, colorIds, spinAxes, count };
}

function snapshotDiagnostics(snapshot) {
    const c = snapshot?.conservation || {};
    const p = c.totalMomentum || {};
    const l = c.totalAngularMomentum || {};
    const center = c.centerOfMass || {};
    return {
        tick: snapshot?.core?.tick ?? 0,
        particleCount: snapshot?.objects?.length ?? 0,
        totalKE: c.kineticEnergy ?? 0,
        totalPE: c.potentialEnergy ?? 0,
        coulombPE: c.coulombPotential ?? 0,
        gravityPE: c.gravityPotential ?? 0,
        totalEnergy: c.stateEnergy ?? 0,
        momentumX: p.x ?? 0, momentumY: p.y ?? 0, momentumZ: p.z ?? 0,
        angMomX: l.x ?? 0, angMomY: l.y ?? 0, angMomZ: l.z ?? 0,
        centerX: center.x ?? 0, centerY: center.y ?? 0, centerZ: center.z ?? 0,
        coveredMask: c.coveredMask ?? 0, missingMask: c.missingMask ?? 0,
        nonconservativeMask: c.nonconservativeMask ?? 0,
        stateEnergyComplete: !!c.stateEnergyComplete,
        driftEligible: !!c.driftEligible,
        cumulativeDampingSink: c.cumulativeDampingSink ?? 0,
        cumulativeRadiationSink: c.cumulativeRadiationSink ?? 0,
        cumulativeSpeedProjectionSink: c.cumulativeSpeedProjectionSink ?? 0,
        cumulativeContactDelta: c.cumulativeContactDelta ?? 0,
        contactEventCount: Array.from(snapshot?.events || [])
            .filter(event => event.type === 'contact_removal').length,
        speedProjectionCount: 0,
    };
}

function normalize3(v) {
    const m = Math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (m < 1e-30) return [0, 0, 0];
    return [v[0] / m, v[1] / m, v[2] / m];
}

function cross3(a, b) {
    return [
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    ];
}

function defaultTangent(pos, center, preferred = [0, 1, 0]) {
    const rHat = normalize3([
        pos[0] - center[0], pos[1] - center[1], pos[2] - center[2],
    ]);
    let t = cross3(rHat, preferred);
    if (Math.sqrt(t[0] * t[0] + t[1] * t[1] + t[2] * t[2]) < 1e-8) {
        t = cross3(rHat, [0, 0, 1]);
    }
    return normalize3(t);
}

/**
 * Build the native particle-engine provider bound to a WasmBridge-like
 * owner exposing `_module` (the loaded embind module, or null).
 *
 * Returns the same method surface the retired JS factory had, minus the
 * internal-only `_peComputeForces`.
 */
export function createNativeParticleEngine(bridge) {
    let _pe = null;               // embind ParticleEngine instance
    let _finitePortBattery = null; // isolated FTD-0884 reference observer
    let finitePortBatteryConfig = null;
    const typeMap = new Map();    // engine id → catalogId
    let registryCache = null;
    let nativeReplayCache = null;
    let nativeObservationCache = null;
    let viewMode = 'native_matter';

    function _module() {
        return bridge._module || null;
    }

    function _ensure() {
        const m = _module();
        if (!m || typeof m.ParticleEngine !== 'function') return null;
        if (!_pe) {
            _pe = new m.ParticleEngine();
        }
        return _pe;
    }

    function initPE() {
        const m = _module();
        if (!m) return;
        if (_pe) {
            try { _pe.delete(); } catch { /* module teardown */ }
            _pe = null;
        }
        if (_finitePortBattery) {
            try { _finitePortBattery.delete(); } catch { /* module teardown */ }
            _finitePortBattery = null;
        }
        finitePortBatteryConfig = null;
        _ensure(); // C++ constructor applies the registry's verified profile.
        typeMap.clear();
        nativeObservationCache = null;
        resetColorWheel();
    }

    function resetPE() {
        initPE();
    }

    /** Release the embind heap object. Call on bridge teardown/reset. */
    function dispose() {
        if (_pe) {
            try { _pe.delete(); } catch { /* module already torn down */ }
            _pe = null;
        }
        if (_finitePortBattery) {
            try { _finitePortBattery.delete(); } catch { /* module teardown */ }
            _finitePortBattery = null;
        }
        finitePortBatteryConfig = null;
        typeMap.clear();
        nativeObservationCache = null;
    }

    // ── Injection ──────────────────────────────────────────────────

    function peAddParticle(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff) {
        const m = _module();
        if (viewMode === 'native_matter') return -1;
        if (!m || !_ensure()) return -1;
        if (mass <= 0) {
            console.warn('NativePE: rejecting massless particle:', catalogId);
            return -1;
        }
        const entry = catalogId ? getById(catalogId) : null;
        const spin = catalogSpin(entry);
        const color = entry ? catalogColorId(entry.color_charge) : 0;
        const axis = initSpinAxis(entry, spin);
        const id = m.peAddParticleEx(_pe, charge, x, y, z, vx, vy, vz,
                                     mass, r_eff, spin, color,
                                     axis[0], axis[1], axis[2]);
        typeMap.set(id, catalogId);
        return id;
    }

    /** Pedagogical anchor only — prefer dynamic particles for genuine dynamics. */
    function peAddLockedParticle(catalogId, charge, x, y, z, mass, r_eff = 0.1) {
        const m = _module();
        if (viewMode === 'native_matter') return -1;
        if (!m || !_ensure()) return -1;
        if (mass <= 0) {
            console.warn('NativePE: rejecting massless particle:', catalogId);
            return -1;
        }
        const entry = catalogId ? getById(catalogId) : null;
        const spin = catalogSpin(entry);
        const color = entry ? catalogColorId(entry.color_charge) : 0;
        const axis = initSpinAxis(entry, spin);
        const id = m.peAddLockedParticleEx(_pe, charge, x, y, z, mass, r_eff,
                                           spin, color,
                                           axis[0], axis[1], axis[2]);
        typeMap.set(id, catalogId);
        return id;
    }

    // ── Equilibrium-orbit seeding (native force probe) ─────────────

    function _findIndexById(m, id) {
        const data = m.getPEExtendedData(_pe);
        for (let i = 0; i < data.count; i++) {
            if (data.ids[i] === id) return { idx: i, data };
        }
        return { idx: -1, data };
    }

    function peApplyEquilibriumOrbit(particleId, options = {}) {
        const m = _module();
        if (viewMode === 'native_matter') return false;
        if (!m || !_ensure()) return false;
        // Zero the velocity FIRST (by id) so velocity-dependent terms
        // (Lorentz, spin-orbit, radiation, relativistic) vanish from the
        // probe. getPEExtendedData then computes a fresh native force
        // snapshot at v=0 — peGetForceDiag would read the stale per-tick
        // buffer, which is empty before the first tick.
        m.peSetVelocity(_pe, particleId, 0, 0, 0);
        const { idx, data } = _findIndexById(m, particleId);
        if (idx < 0) return false;

        const center = options.center || [0, 0, 0];
        const sign = options.sign ?? 1;
        const px = data.positions[idx * 3];
        const py = data.positions[idx * 3 + 1];
        const pz = data.positions[idx * 3 + 2];
        const mass = data.masses[idx];
        const rx = px - center[0], ry = py - center[1], rz = pz - center[2];
        const r = Math.sqrt(rx * rx + ry * ry + rz * rz);
        if (r < 1e-12 || mass <= 0) return false;

        // Solve m·v²/r = |F_inward| from the native force at v=0.
        const fRad = data.forces[idx * 3] * (rx / r)
                   + data.forces[idx * 3 + 1] * (ry / r)
                   + data.forces[idx * 3 + 2] * (rz / r);
        const fInward = -fRad;
        const speed = fInward > 0
            ? Math.min(Math.sqrt(fInward * r / mass), C_SPEED * 0.95)
            : 0;

        const tangent = options.tangent || defaultTangent([px, py, pz], center);
        m.peSetVelocity(_pe, particleId,
                        tangent[0] * speed * sign,
                        tangent[1] * speed * sign,
                        tangent[2] * speed * sign);
        return true;
    }

    function peApplyEquilibriumOrbitBatch(entries) {
        if (!entries?.length) return;
        for (const { particleId, center, tangent, sign } of entries) {
            const opts = {};
            if (center) opts.center = center;
            if (tangent) opts.tangent = tangent;
            if (sign !== undefined) opts.sign = sign;
            peApplyEquilibriumOrbit(particleId, opts);
        }
    }

    function peScaleVelocity(particleId, scale) {
        const m = _module();
        if (viewMode === 'native_matter') return false;
        if (!m || !_ensure() || scale === 1) return false;
        const { idx, data } = _findIndexById(m, particleId);
        if (idx < 0) return false;
        m.peSetVelocity(_pe, particleId,
                        data.velocities[idx * 3] * scale,
                        data.velocities[idx * 3 + 1] * scale,
                        data.velocities[idx * 3 + 2] * scale);
        return true;
    }

    // ── Tick + reads ───────────────────────────────────────────────

    function peTick() {
        if (viewMode !== 'native_matter' && _ensure()) _pe.tick();
    }

    function peGetTick() {
        if (viewMode === 'native_matter') {
            return Number(peGetNativeMatterReplay()?.core?.tick ?? 0);
        }
        return _ensure() ? Number(_pe.currentTick()) : 0;
    }

    function peGetObservationRevision() {
        if (viewMode === 'native_matter') return 0;
        return _ensure() ? Number(_pe.observationRevision()) : 0;
    }

    function peGetParticleData() {
        const m = _module();
        if (viewMode === 'native_matter') {
            return snapshotParticleData(peGetNativeMatterReplay());
        }
        if (!m || !_ensure()) return emptyParticleData();
        const data = m.getPEParticleData(_pe);
        // Visual parity with the retired JS engine: its point-size law was
        // 6 + 4·log10(m/K_B + 1) clamped at 60; the native binding uses a
        // half-scale law. Recompute here (display-only field).
        for (let i = 0; i < data.count; i++) {
            const s = 6.0 + 4.0 * Math.log10(data.masses[i] / K_B + 1.0);
            data.sizes[i] = s > 60 ? 60 : s;
        }
        return data;
    }

    function peGetFieldSources() {
        const m = _module();
        if (viewMode === 'native_matter') {
            return { positions: EMPTY_F32, charges: EMPTY_F32, masses: EMPTY_F32, count: 0 };
        }
        if (!m || !_ensure()) {
            return { positions: EMPTY_F32, charges: EMPTY_F32, masses: EMPTY_F32, count: 0 };
        }
        const data = m.getPEParticleData(_pe);
        const n = data.count;
        const charges = new Float32Array(n);
        const masses = new Float32Array(n);
        for (let i = 0; i < n; i++) {
            charges[i] = data.charges[i];
            masses[i] = data.masses[i];
        }
        return { positions: data.positions, charges, masses, count: n };
    }

    function peGetForces() {
        const m = _module();
        if (viewMode === 'native_matter') {
            return { positions: EMPTY_F32, forces: EMPTY_F64, count: 0, maxForce: 0 };
        }
        if (!m || !_ensure()) {
            return { positions: EMPTY_F32, forces: EMPTY_F64, count: 0, maxForce: 0 };
        }
        return m.getPEForces(_pe);
    }

    function peGetForceDecomposition() {
        const m = _module();
        if (viewMode === 'native_matter') return emptyDecomposition();
        if (!m || !_ensure()) return emptyDecomposition();
        return m.getPEForceDecomposition(_pe);
    }

    function peGetDiagnostics() {
        const m = _module();
        if (viewMode === 'native_matter') {
            return snapshotDiagnostics(peGetNativeMatterReplay());
        }
        if (!m || !_ensure()) return emptyDiagnostics();
        return m.getPEDiagnostics(_pe);
    }

    function peGetSnapshot(scenario = '') {
        const m = _module();
        if (viewMode === 'native_matter') return peGetNativeMatterReplay();
        if (!m || !_ensure() || typeof m.getPESnapshot !== 'function') return null;
        return m.getPESnapshot(_pe, scenario);
    }

    function peGetNativeMatterReplay() {
        const m = _module();
        if (!m || typeof m.getScale1NativeMatterReplay !== 'function') return null;
        if (nativeObservationCache) return nativeObservationCache;
        if (!nativeReplayCache) nativeReplayCache = m.getScale1NativeMatterReplay();
        return nativeReplayCache;
    }

    function peUseRegisteredM3Replay() {
        nativeObservationCache = null;
        return peGetNativeMatterReplay();
    }

    function peObserveSourceClusters(payload) {
        const m = _module();
        if (!m || typeof m.getScale1LiveClusterObservation !== 'function') return null;
        const seeds = Array.from(payload?.seeds || []);
        const count = seeds.length;
        const ids = new Int32Array(count);
        const supports = new Int32Array(count);
        const signs = new Int32Array(count);
        const centers = new Float64Array(count * 3);
        const velocities = new Float64Array(count * 3);
        const L = payload?.latticeSize || 1;
        const scale = payload?.displayScale || 1;
        const origin = (L - 1) * 0.5;
        for (let i = 0; i < count; i++) {
            const seed = seeds[i];
            ids[i] = seed.clusterId ?? i;
            supports[i] = seed.size ?? 0;
            signs[i] = seed.stateSign ?? Math.sign(seed.charge || 0);
            const center = seed.sourceCentroid || [
                (seed.position?.[0] || 0) / scale + origin,
                (seed.position?.[1] || 0) / scale + origin,
                (seed.position?.[2] || 0) / scale + origin,
            ];
            const velocity = seed.sourceVelocity || seed.velocity || [0, 0, 0];
            centers.set(center, i * 3);
            velocities.set(velocity, i * 3);
        }
        nativeObservationCache = m.getScale1LiveClusterObservation(
            ids, supports, signs, centers, velocities,
            payload?.sourceTick ?? 0,
            payload?.sourceScenario ?? 'no-capture',
            payload?.sourceRevision ?? 'ephemeral-capture',
            L, scale,
        );
        return nativeObservationCache;
    }

    function peGetPhysicsRegistry() {
        const m = _module();
        if (!m || typeof m.getScale1Registry !== 'function') return null;
        if (!registryCache) registryCache = m.getScale1Registry();
        return registryCache;
    }

    function peSetMode(mode) {
        const supported = new Set([
            'native_matter', 'effective_lab', 'catalog_reference',
        ]);
        viewMode = supported.has(mode) ? mode : 'effective_lab';
        return viewMode;
    }

    function peGetExtendedData() {
        const m = _module();
        if (viewMode === 'native_matter') {
            const data = peGetParticleData();
            return { ...data, forces: new Float64Array(data.count * 3),
                accelerations: new Float64Array(data.count * 3),
                pairIds: new Int32Array(data.count).fill(-1) };
        }
        if (!m || !_ensure()) return null;
        return m.getPEExtendedData(_pe);
    }

    // ── Controls ───────────────────────────────────────────────────

    function _setToggle(name, value) {
        const m = _module();
        if (viewMode === 'native_matter') return false;
        return !!(m && _ensure() && m.peSetToggle(_pe, name, !!value));
    }

    function peSetDt(dt) { const m = _module(); if (viewMode !== 'native_matter' && m && _ensure()) m.peSetDt(_pe, dt); }
    function peGetDt() { const m = _module(); return (m && _ensure()) ? m.peGetDt(_pe) : 1.0; }
    function peSetSoftening(s) { const m = _module(); if (viewMode !== 'native_matter' && m && _ensure()) m.peSetSoftening(_pe, s); }

    function peConfigureInsulatingBox(cx, cy, cz, hx, hy, hz) {
        const m = _module();
        if (viewMode === 'native_matter' || !m || !_ensure()) return false;
        m.peConfigureInsulatingBox(_pe, cx, cy, cz, hx, hy, hz);
        return true;
    }

    function peAddInsulatingPort(axis, side, centerU, centerV, halfU, halfV,
                                 requiredChargeSign = 0, crossingDirection = 0) {
        const m = _module();
        if (viewMode === 'native_matter' || !m || !_ensure()) return false;
        m.peAddInsulatingPort(_pe, axis, side, centerU, centerV, halfU, halfV,
                              requiredChargeSign, crossingDirection);
        return true;
    }

    function peClearInsulatingBox() {
        const m = _module();
        if (viewMode === 'native_matter' || !m || !_ensure()) return false;
        m.peClearInsulatingBox(_pe);
        return true;
    }

    const peSetCoulomb = (e) => _setToggle('coulomb', e);
    const peSetDamping = (e) => _setToggle('damping', e);
    const peSetGravity = (e) => _setToggle('gravity', e);
    const peSetLorentz = (e) => _setToggle('lorentz', e);
    const peSetExchange = (e) => _setToggle('exchange', e);
    const peSetStrong = (e) => _setToggle('strong', e);
    const peSetMagneticDipole = (e) => _setToggle('magnetic_dipole', e);
    const peSetSpinOrbit = (e) => _setToggle('spin_orbit', e);
    const peSetRadiation = (e) => _setToggle('radiation', e);
    const peSetRelativistic = (e) => _setToggle('relativistic', e);
    const peSetRelativisticVerlet = (e) => _setToggle('relativistic_verlet', e);
    const peSetContactEvents = (e) => _setToggle('contact_events', e);
    const peSetToggle = (name, value) => _setToggle(name, value);

    function peSetSpinAxis(id, ax, ay, az) {
        const m = _module();
        if (viewMode === 'native_matter') return false;
        if (!m || !_ensure()) return false;
        if (ax * ax + ay * ay + az * az < 1e-60) return false;
        m.peSetSpinAxis(_pe, id, ax, ay, az);
        return true;
    }

    function peGetToggle(name) {
        const m = _module();
        return (m && _ensure()) ? m.peGetToggle(_pe, name) : false;
    }

    function peGetBackendCapabilities() {
        return {
            backend: _module() ? 'wasm' : 'unavailable',
            velocities: true,
            masses: true,
            locked: true,
            forces: true,
            extended: true,
            nativeExtended: true,
            nativeForces: true,
            advancedForces: true,
            sharedScale1Schema: true,
            nativeMatterReplay: true,
        };
    }

    function peParticleCount() {
        const m = _module();
        if (viewMode === 'native_matter') {
            return peGetNativeMatterReplay()?.objects?.length ?? 0;
        }
        return (m && _ensure()) ? m.peParticleCount(_pe) : 0;
    }

    function peClear() {
        if (viewMode === 'native_matter') return false;
        initPE();
        return true;
    }

    function peExportCheckpoint() {
        const m = _module();
        if (viewMode === 'native_matter' || !m || !_ensure()
            || typeof m.exportPECheckpoint !== 'function') return null;
        return {
            schema: 'ftd.scale1.dashboard-checkpoint',
            schemaVersion: 1,
            capturedAt: new Date().toISOString(),
            native: m.exportPECheckpoint(_pe),
            catalogTypes: Array.from(typeMap.entries()),
            finitePortBattery: finitePortBatteryConfig
                ? { ...finitePortBatteryConfig } : null,
        };
    }

    function peRestoreCheckpoint(checkpoint) {
        const m = _module();
        if (!m || !_ensure() || typeof m.restorePECheckpoint !== 'function') return false;
        if (!checkpoint || checkpoint.schema !== 'ftd.scale1.dashboard-checkpoint'
            || checkpoint.schemaVersion !== 1 || !checkpoint.native) {
            throw new TypeError('Unsupported Scale 1 dashboard checkpoint');
        }
        const restored = !!m.restorePECheckpoint(_pe, checkpoint.native);
        if (!restored) return false;
        typeMap.clear();
        for (const row of Array.from(checkpoint.catalogTypes || [])) {
            if (Array.isArray(row) && Number.isInteger(Number(row[0]))) {
                typeMap.set(Number(row[0]), row[1] ?? null);
            }
        }
        viewMode = 'effective_lab';
        nativeObservationCache = null;
        if (_finitePortBattery) {
            try { _finitePortBattery.delete(); } catch { /* module teardown */ }
            _finitePortBattery = null;
        }
        finitePortBatteryConfig = null;
        const battery = checkpoint.finitePortBattery;
        if (battery && typeof m.Scale1FinitePortBatteryObserver === 'function') {
            _finitePortBattery = new m.Scale1FinitePortBatteryObserver(
                battery.size, battery.capacity,
                battery.chargeAmplitude, battery.batteryAmplitude,
            );
            finitePortBatteryConfig = { ...battery, steps: 0 };
            for (let i = 0; i < Math.max(0, Number(battery.steps) || 0); i++) {
                if (!_finitePortBattery.step()) break;
                finitePortBatteryConfig.steps++;
            }
        }
        return true;
    }

    function peConfigureFinitePortBattery(size = 6, capacity = 8,
                                          chargeAmplitude = 1,
                                          batteryAmplitude = 10) {
        const m = _module();
        if (!m || typeof m.Scale1FinitePortBatteryObserver !== 'function') return false;
        if (_finitePortBattery) {
            try { _finitePortBattery.delete(); } catch { /* module teardown */ }
        }
        _finitePortBattery = new m.Scale1FinitePortBatteryObserver(
            size, capacity, chargeAmplitude, batteryAmplitude,
        );
        finitePortBatteryConfig = {
            size, capacity, chargeAmplitude, batteryAmplitude, steps: 0,
        };
        return true;
    }

    function peStepFinitePortBattery() {
        if (!_finitePortBattery || !_finitePortBattery.step()) return false;
        finitePortBatteryConfig.steps++;
        return true;
    }

    function peReverseFinitePortBattery() {
        if (!_finitePortBattery || !_finitePortBattery.reverse()) return false;
        finitePortBatteryConfig.steps = Math.max(0, finitePortBatteryConfig.steps - 1);
        return true;
    }

    function peGetFinitePortBatterySnapshot() {
        return _finitePortBattery ? _finitePortBattery.snapshot() : null;
    }

    function peGetParticleTypes() {
        return typeMap;
    }

    // ── Inspector (derived from native data — no force re-implementation:
    //    fNet comes from the native force buffer) ─────────────────────

    function peInspectParticle(id) {
        const m = _module();
        if (!m || !_ensure()) return null;
        if (viewMode === 'native_matter') {
            const object = Array.from(peGetNativeMatterReplay()?.objects || [])
                .find(row => row.id === id);
            if (!object) return null;
            const momentum = object.momentum || { x: 0, y: 0, z: 0 };
            return {
                id, charge: object.effectiveState,
                mass: object.massAvailable ? object.mass : null,
                rEff: object.effectiveRadius || null,
                spin: null, colorId: null, pairId: -1,
                x: object.position.x, y: object.position.y, z: object.position.z,
                vx: object.velocity.x, vy: object.velocity.y, vz: object.velocity.z,
                speed: Math.hypot(object.velocity.x, object.velocity.y, object.velocity.z),
                ke: object.kineticEnergyAvailable ? object.kineticEnergy : null,
                momentum: Math.hypot(momentum.x, momentum.y, momentum.z),
                acceleration: null, locked: !!object.locked,
                nearestId: -1, nearestDist: Infinity, orbitalR: -1,
                fCoulombNearest: null, fNetMag: null,
                provenance: object.provenance,
            };
        }
        const data = m.getPEExtendedData(_pe);
        const snapshot = m.getPESnapshot(_pe, '');
        let idx = -1;
        for (let i = 0; i < data.count; i++) {
            if (data.ids[i] === id) { idx = i; break; }
        }
        if (idx < 0) return null;

        const px = data.positions[idx * 3];
        const py = data.positions[idx * 3 + 1];
        const pz = data.positions[idx * 3 + 2];
        const vx = data.velocities[idx * 3];
        const vy = data.velocities[idx * 3 + 1];
        const vz = data.velocities[idx * 3 + 2];
        const mass = data.masses[idx];
        const charge = data.charges[idx];
        const speed = Math.sqrt(vx * vx + vy * vy + vz * vz);

        const fx = data.forces[idx * 3];
        const fy = data.forces[idx * 3 + 1];
        const fz = data.forces[idx * 3 + 2];
        const fNetMag = Math.sqrt(fx * fx + fy * fy + fz * fz);

        let nearestId = -1, nearestIdx = -1, nearestDist = Infinity, orbitalR = -1;
        for (let j = 0; j < data.count; j++) {
            if (j === idx) continue;
            const dx = data.positions[j * 3] - px;
            const dy = data.positions[j * 3 + 1] - py;
            const dz = data.positions[j * 3 + 2] - pz;
            const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
            if (r < nearestDist) {
                nearestDist = r;
                nearestId = data.ids[j];
                nearestIdx = j;
            }
            const qj = data.charges[j];
            if (charge !== 0 && qj !== 0 && Math.sign(charge) !== Math.sign(qj)) {
                if (orbitalR < 0 || r < orbitalR) orbitalR = r;
            }
        }

        // The inspector must use the same toggle-aware, softened pair
        // evaluator as integration. Do not reconstruct Coulomb from distance.
        const fCoulombNearest = nearestIdx >= 0
            ? m.getPECoulombPairForceMagnitude(_pe, idx, nearestIdx)
            : 0;

        const pd = m.getPEParticleData(_pe);
        const object = Array.from(snapshot.objects || []).find(row => row.id === id);
        const pVec = object?.momentum || { x: mass * vx, y: mass * vy, z: mass * vz };
        const pMag = Math.sqrt(pVec.x * pVec.x + pVec.y * pVec.y + pVec.z * pVec.z);
        return {
            id, charge, mass,
            rEff: pd.rEff[idx], spin: pd.spins[idx], colorId: pd.colorIds[idx],
            pairId: data.pairIds ? data.pairIds[idx] : -1,
            x: px, y: py, z: pz, vx, vy, vz,
            speed,
            ke: object?.kineticEnergyAvailable ? object.kineticEnergy : null,
            momentum: pMag,
            acceleration: mass > 0 ? fNetMag / mass : 0,
            locked: !!data.locked[idx],
            nearestId, nearestDist, orbitalR, fCoulombNearest, fNetMag,
        };
    }

    return {
        initPE, resetPE, dispose,
        peAddParticle, peAddLockedParticle,
        peApplyEquilibriumOrbit, peApplyEquilibriumOrbitBatch, peScaleVelocity,
        peTick, peGetTick, peGetObservationRevision,
        peGetParticleData, peGetFieldSources, peGetForces,
        peGetForceDecomposition,
        peGetDiagnostics, peGetExtendedData, peGetSnapshot,
        peGetNativeMatterReplay, peUseRegisteredM3Replay, peObserveSourceClusters,
        peGetPhysicsRegistry, peSetMode,
        peSetDt, peGetDt, peSetSoftening,
        peConfigureInsulatingBox, peAddInsulatingPort, peClearInsulatingBox,
        peSetCoulomb, peSetDamping, peSetGravity, peSetLorentz,
        peSetExchange, peSetStrong, peSetMagneticDipole,
        peSetSpinOrbit, peSetSpinAxis, peSetRadiation, peSetRelativistic,
        peSetRelativisticVerlet, peSetContactEvents, peSetToggle,
        peGetToggle, peGetBackendCapabilities,
        peParticleCount, peClear, peGetParticleTypes,
        peExportCheckpoint, peRestoreCheckpoint,
        peConfigureFinitePortBattery, peStepFinitePortBattery,
        peReverseFinitePortBattery, peGetFinitePortBatterySnapshot,
        peInspectParticle,
    };
}
