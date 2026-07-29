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
 * Toggle baseline: initPE()/peClear() apply the historical JS defaults
 * (coulomb ON, everything else OFF) rather than the C++ constructor
 * defaults (coulomb+gravity+damping ON) so scenario presets keep their
 * long-standing meaning; presets then set toggles explicitly on top.
 *
 * Behavior changes vs the retired JS engine (deliberate, see plan):
 *   - no r=35 boundary reflection (native engine is unbounded)
 *   - native annihilation / strong / exchange semantics are live
 *   - forces/decomposition come from the one native kernel — the audit's
 *     "second drifted re-implementation" class of defect is structurally gone.
 */

import { C_SPEED, K_B, COULOMB_K_FORCE } from '../constants.js';
import { getById } from '../particle-catalog.js';
import {
    catalogColorId, catalogSpin, initSpinAxis, resetColorWheel,
} from './pe-catalog-map.js';

// JS-parity toggle baseline applied on init/clear (see header).
const TOGGLE_BASELINE = {
    coulomb: true,
    gravity: false,
    damping: false,
    lorentz: false,
    exchange: false,
    strong: false,
    magnetic_dipole: false,
    spin_orbit: false,
    radiation: false,
    relativistic: false,
    relativistic_verlet: false,
};

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
        angMomX: 0, angMomY: 0, angMomZ: 0,
    };
}

function emptyDecomposition() {
    return {
        positions: EMPTY_F32, count: 0,
        coulomb: EMPTY_F64, gravity: EMPTY_F64, strong: EMPTY_F64,
        magnetic_dipole: EMPTY_F64, spin_orbit: EMPTY_F64, net: EMPTY_F64,
        maxCoulomb: 0, maxGravity: 0, maxStrong: 0,
        maxMagneticDipole: 0, maxSpinOrbit: 0, maxNet: 0,
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
    const typeMap = new Map();    // engine id → catalogId

    function _module() {
        return bridge._module || null;
    }

    function _ensure() {
        const m = _module();
        if (!m || typeof m.ParticleEngine !== 'function') return null;
        if (!_pe) {
            _pe = new m.ParticleEngine();
            _applyBaseline(m);
        }
        return _pe;
    }

    function _applyBaseline(m) {
        for (const [name, value] of Object.entries(TOGGLE_BASELINE)) {
            m.peSetToggle(_pe, name, value);
        }
    }

    function initPE() {
        const m = _module();
        if (!m) return;
        _ensure();
        if (_pe) {
            m.peClear(_pe);
            _applyBaseline(m);
        }
        typeMap.clear();
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
        typeMap.clear();
    }

    // ── Injection ──────────────────────────────────────────────────

    function peAddParticle(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff) {
        const m = _module();
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
        if (_ensure()) _pe.tick();
    }

    function peGetParticleData() {
        const m = _module();
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
        if (!m || !_ensure()) {
            return { positions: EMPTY_F32, forces: EMPTY_F64, count: 0, maxForce: 0 };
        }
        return m.getPEForces(_pe);
    }

    function peGetForceDecomposition() {
        const m = _module();
        if (!m || !_ensure()) return emptyDecomposition();
        return m.getPEForceDecomposition(_pe);
    }

    function peGetDiagnostics() {
        const m = _module();
        if (!m || !_ensure()) return emptyDiagnostics();
        return m.getPEDiagnostics(_pe);
    }

    function peGetExtendedData() {
        const m = _module();
        if (!m || !_ensure()) return null;
        return m.getPEExtendedData(_pe);
    }

    // ── Controls ───────────────────────────────────────────────────

    function _setToggle(name, value) {
        const m = _module();
        if (m && _ensure()) m.peSetToggle(_pe, name, !!value);
    }

    function peSetDt(dt) { const m = _module(); if (m && _ensure()) m.peSetDt(_pe, dt); }
    function peGetDt() { const m = _module(); return (m && _ensure()) ? m.peGetDt(_pe) : 1.0; }
    function peSetSoftening(s) { const m = _module(); if (m && _ensure()) m.peSetSoftening(_pe, s); }

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

    function peSetSpinAxis(id, ax, ay, az) {
        const m = _module();
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
        };
    }

    function peParticleCount() {
        const m = _module();
        return (m && _ensure()) ? m.peParticleCount(_pe) : 0;
    }

    function peClear() {
        initPE();
    }

    function peGetParticleTypes() {
        return typeMap;
    }

    // ── Inspector (derived from native data — no force re-implementation:
    //    fNet comes from the native force buffer) ─────────────────────

    function peInspectParticle(id) {
        const m = _module();
        if (!m || !_ensure()) return null;
        const data = m.getPEExtendedData(_pe);
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

        let nearestId = -1, nearestDist = Infinity, orbitalR = -1;
        for (let j = 0; j < data.count; j++) {
            if (j === idx) continue;
            const dx = data.positions[j * 3] - px;
            const dy = data.positions[j * 3 + 1] - py;
            const dz = data.positions[j * 3 + 2] - pz;
            const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
            if (r < nearestDist) { nearestDist = r; nearestId = data.ids[j]; }
            const qj = data.charges[j];
            if (charge !== 0 && qj !== 0 && Math.sign(charge) !== Math.sign(qj)) {
                if (orbitalR < 0 || r < orbitalR) orbitalR = r;
            }
        }

        let fCoulombNearest = 0;
        if (nearestId >= 0 && Number.isFinite(nearestDist)) {
            for (let j = 0; j < data.count; j++) {
                if (data.ids[j] !== nearestId) continue;
                const r2 = nearestDist * nearestDist;
                fCoulombNearest = Math.abs(
                    COULOMB_K_FORCE * charge * data.charges[j] / (r2 || 1e-30));
                break;
            }
        }

        const pd = m.getPEParticleData(_pe);
        return {
            id, charge, mass,
            rEff: pd.rEff[idx], spin: pd.spins[idx], colorId: pd.colorIds[idx],
            pairId: data.pairIds ? data.pairIds[idx] : -1,
            x: px, y: py, z: pz, vx, vy, vz,
            speed, ke: 0.5 * mass * speed * speed,
            momentum: mass * speed,
            acceleration: mass > 0 ? fNetMag / mass : 0,
            locked: !!data.locked[idx],
            nearestId, nearestDist, orbitalR, fCoulombNearest, fNetMag,
        };
    }

    return {
        initPE, resetPE, dispose,
        peAddParticle, peAddLockedParticle,
        peApplyEquilibriumOrbit, peApplyEquilibriumOrbitBatch, peScaleVelocity,
        peTick,
        peGetParticleData, peGetFieldSources, peGetForces,
        peGetForceDecomposition,
        peGetDiagnostics, peGetExtendedData,
        peSetDt, peGetDt, peSetSoftening,
        peSetCoulomb, peSetDamping, peSetGravity, peSetLorentz,
        peSetExchange, peSetStrong, peSetMagneticDipole,
        peSetSpinOrbit, peSetSpinAxis, peSetRadiation, peSetRelativistic,
        peSetRelativisticVerlet, peGetToggle, peGetBackendCapabilities,
        peParticleCount, peClear, peGetParticleTypes,
        peInspectParticle,
    };
}
