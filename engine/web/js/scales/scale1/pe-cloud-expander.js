/**
 * Scale 1 — PE Cloud Expander
 * ────────────────────────────────────────────────────────────────────
 *
 * Fixed-boundary point cloud per particle. Manifestation blink runs in
 * the shader (constant fill, independent slot phases). Orbital rotation
 * of the halo uses L = r × v; intrinsic spin biases phase handedness.
 */

import { getById } from '../../particle-catalog.js';
import { K_B, C_SPEED } from '../../constants.js';
import { PE_VIS_BOUNDARY_R } from '../../viewport/constants.js';
import {
    DEFAULT_TRAIL_SETTINGS,
    TRAIL_HISTORY_CAPACITY,
    trailCapacityForPopulation,
    normalizeTrailSettings,
} from './trail-settings.js?v=2';

export const MAX_CLOUD_TOTAL = 100000;
export const TRAIL_MAX_LENGTH = TRAIL_HISTORY_CAPACITY;
// Canonical value lives in viewport/constants.js; re-exported here so the
// controller's existing import surface keeps working.
export { MANIFEST_FILL } from '../../viewport/constants.js';
export { PE_VIS_BOUNDARY_R };

const _cloudPos = new Float32Array(MAX_CLOUD_TOTAL * 3);
const _cloudCol = new Float32Array(MAX_CLOUD_TOTAL * 3);
const _cloudSize = new Float32Array(MAX_CLOUD_TOTAL);
const _cloudPhase = new Float32Array(MAX_CLOUD_TOTAL);
const _cloudRate = new Float32Array(MAX_CLOUD_TOTAL);
const _cloudRole = new Float32Array(MAX_CLOUD_TOTAL);
const _cloudParticleMap = new Int32Array(MAX_CLOUD_TOTAL);

export const PE_APPEARANCE_ROLE = Object.freeze({
    SUPPORT: 0,
    RECORD_CORE: 1,
    SUPPORT_RIM: 2,
});

const _trailHistory = new Map();
const _activeIdsSet = new Set();
let _fallbackTrailTick = 0;

// Manifestation spawn-flash: the first-seen-id diffing this module already
// performs for trail history (the `!_trailHistory.has(id)` check), reused
// here to drive a real "just spawned" visual instead of only tracking
// position trails. New particles get an elevated blink rate for
// SPAWN_FLASH_DURATION seconds after their first appearance, then settle
// into their normal force/velocity-driven rate — reusing the existing
// manifestPhase/manifestRate shader pipeline (viewport/shaders.js,
// particle-renderer.js setManifestation()), not a new render path.
const _spawnTimes = new Map(); // id -> frameSec first seen
const SPAWN_FLASH_DURATION = 0.6; // seconds
const SPAWN_FLASH_RATE_BOOST = 3.5; // additive rate boost at age=0, decaying to 0

let _unitTemplate = null;

function getUnitTemplate() {
    if (_unitTemplate) return _unitTemplate;
    const n = 4000;
    const offsets = new Float32Array(n * 3);
    const brightness = new Float32Array(n);
    const radialInverse = (index, base) => {
        let value = 0;
        let fraction = 1 / base;
        for (let i = index; i > 0; i = Math.floor(i / base)) {
            value += fraction * (i % base);
            fraction /= base;
        }
        return value;
    };
    for (let i = 0; i < n; i++) {
        // Deterministic low-discrepancy volume sampling: stable across reloads
        // and well distributed even when a low-population particle consumes
        // only the prefix of this template.
        const z = 1 - 2 * radialInverse(i + 1, 2);
        const azimuth = 2 * Math.PI * radialInverse(i + 1, 3);
        const radial = Math.pow(radialInverse(i + 1, 5), 0.56);
        const planar = Math.sqrt(Math.max(0, 1 - z * z));
        const ox = radial * planar * Math.cos(azimuth);
        const oy = radial * planar * Math.sin(azimuth);
        const oz = radial * z;
        offsets[i * 3] = ox;
        offsets[i * 3 + 1] = oy;
        offsets[i * 3 + 2] = oz;
        const dist = Math.sqrt(ox * ox + oy * oy + oz * oz);
        brightness[i] = Math.exp(-dist * dist * 2.5);
    }
    _unitTemplate = { n, offsets, brightness };
    return _unitTemplate;
}

function chargeFallbackColor(charge) {
    if (charge > 0) return [0.29, 0.87, 0.50];
    if (charge < 0) return [0.97, 0.44, 0.44];
    return [0.60, 0.60, 0.70];
}

function strongColorTint(colorId, base) {
    if (colorId === 1) return [base[0] * 0.7 + 0.3, base[1] * 0.5, base[2] * 0.5];
    if (colorId === 2) return [base[0] * 0.5, base[1] * 0.7 + 0.3, base[2] * 0.5];
    if (colorId === 3) return [base[0] * 0.5, base[1] * 0.55, base[2] * 0.7 + 0.3];
    return base;
}

export function visualLocalizationRadius(massMev, rEff) {
    const m = Math.max(massMev, K_B * 0.05);
    const comptonLike = 0.2 + 2.2 * Math.pow(K_B / m, 0.38);
    return Math.min(6.5, Math.max(0.72, rEff || 0.1, comptonLike));
}

function pointCountForParticle(massMev, radius) {
    const nRaw = Math.round(68 * Math.pow(massMev / K_B, 0.28) * (radius / 2.0));
    return Math.min(Math.max(nRaw, 32), 1200);
}

function betaFromVelocity(vx, vy, vz) {
    const speed = Math.sqrt(vx * vx + vy * vy + vz * vz);
    return Math.min(speed / C_SPEED, 1.0);
}

function stretchOffset(ox, oy, oz, vx, vy, vz, beta) {
    const stretch = 0.4 * beta;
    const vmag2 = vx * vx + vy * vy + vz * vz;
    if (vmag2 < 1e-16 || stretch < 1e-6) return [ox, oy, oz];
    const inv = 1 / Math.sqrt(vmag2);
    const ux = vx * inv;
    const uy = vy * inv;
    const uz = vz * inv;
    const dot = ox * ux + oy * uy + oz * uz;
    const px = dot * ux;
    const py = dot * uy;
    const pz = dot * uz;
    const s = 1 + stretch;
    return [px * s + (ox - px), py * s + (oy - py), pz * s + (oz - pz)];
}

/** Rodrigues rotation of offset vector about unit axis (ux,uy,uz) by angle. */
function rotateOffset(ox, oy, oz, ux, uy, uz, angle) {
    if (Math.abs(angle) < 1e-8) return [ox, oy, oz];
    const c = Math.cos(angle);
    const s = Math.sin(angle);
    const dot = ox * ux + oy * uy + oz * uz;
    const cx = uy * oz - uz * oy;
    const cy = uz * ox - ux * oz;
    const cz = ux * oy - uy * ox;
    return [
        ox * c + cx * s + ux * dot * (1 - c),
        oy * c + cy * s + uy * dot * (1 - c),
        oz * c + cz * s + uz * dot * (1 - c),
    ];
}

/** Classical orbital ω ≈ |L|/(m r²) about origin, from r × v. */
function orbitalSweepAngle(cx, cy, cz, vx, vy, vz, mass, frameSec) {
    const lx = cy * vz - cz * vy;
    const ly = cz * vx - cx * vz;
    const lz = cx * vy - cy * vx;
    const Lmag = Math.sqrt(lx * lx + ly * ly + lz * lz);
    const r2 = cx * cx + cy * cy + cz * cz;
    if (Lmag < 1e-12 || r2 < 1e-12) return { angle: 0, ux: 0, uy: 0, uz: 1 };
    const m = Math.max(mass, K_B * 0.01);
    const omega = Lmag / (m * r2);
    const invL = 1 / Lmag;
    return {
        angle: omega * frameSec * 0.35,
        ux: lx * invL,
        uy: ly * invL,
        uz: lz * invL,
    };
}

function modulateColor(base, beta, keNorm) {
    const boost = 0.82 + 0.18 * keNorm;
    const w = beta * 0.22;
    return [
        base[0] * boost * (1 - w) + w,
        base[1] * boost * (1 - w) + w,
        base[2] * boost * (1 - w) + w,
    ];
}

function hashUint32(a, b, c) {
    let h = (Math.imul(a | 0, 374761393) + Math.imul(b | 0, 668265263) + (c | 0)) >>> 0;
    h = (Math.imul(h ^ (h >>> 13), 1274126177)) >>> 0;
    return h;
}

function writeCloudPoint(out, cx, cy, cz, cr, cg, cb, size, phase, rate, role, pid) {
    _cloudPos[out * 3] = cx;
    _cloudPos[out * 3 + 1] = cy;
    _cloudPos[out * 3 + 2] = cz;
    _cloudCol[out * 3] = cr;
    _cloudCol[out * 3 + 1] = cg;
    _cloudCol[out * 3 + 2] = cb;
    _cloudSize[out] = size;
    _cloudPhase[out] = phase;
    _cloudRate[out] = rate;
    _cloudRole[out] = role;
    _cloudParticleMap[out] = pid;
}

/**
 * @param {number} [frameSec] — current animation clock, seconds. Drives the
 *   spawn-flash window; omit to disable spawn tracking for this call
 *   (e.g. a caller with no clock available falls back to the pre-existing
 *   force/velocity-only blink rate, unchanged from before 2026-07-14).
 */
export function buildPEManifestBlinkRate(peData, forceData, frameSec) {
    const n = peData.count;
    const hasIds = !!peData.ids;
    const trackSpawns = hasIds && Number.isFinite(frameSec);

    // Spawn-tracking (build seen-set + prune stale ids) runs even when n=0 —
    // otherwise a scenario that goes fully empty (e.g. "Clear & Reload")
    // never prunes _spawnTimes, and a subsequently reused id would be
    // treated as "already seen" and silently skip its spawn flash.
    if (trackSpawns) {
        const seenThisFrame = new Set();
        for (let i = 0; i < n; i++) {
            const id = peData.ids[i];
            seenThisFrame.add(id);
            if (!_spawnTimes.has(id)) _spawnTimes.set(id, frameSec);
        }
        // Prune ids no longer present (effective record removed) so the
        // map doesn't grow unboundedly across a long-running session.
        for (const id of _spawnTimes.keys()) {
            if (!seenThisFrame.has(id)) _spawnTimes.delete(id);
        }
    }

    const out = new Float32Array(n);
    if (!n) return out;

    let maxF = forceData?.maxForce ?? 0;
    if (maxF < 1e-30) maxF = 1;

    const hasForces = !!(forceData && forceData.count === n && forceData.forces);
    const hasVel = !!peData.velocities;
    const hasMass = !!peData.masses;
    const hasSpins = !!peData.spins;

    for (let i = 0; i < n; i++) {
        let drive = 0;

        if (hasForces) {
            const i3 = i * 3;
            const fx = forceData.forces[i3];
            const fy = forceData.forces[i3 + 1];
            const fz = forceData.forces[i3 + 2];
            const fmag = Math.sqrt(fx * fx + fy * fy + fz * fz);
            const m = hasMass ? peData.masses[i] : K_B;
            const accel = fmag / Math.max(m, K_B * 0.01);
            drive = Math.max(drive, 0.5 * Math.min(fmag / maxF, 1) + 0.5 * Math.min(accel / (C_SPEED * 0.05), 1));
        }

        if (hasVel) {
            const i3 = i * 3;
            drive = Math.max(drive, betaFromVelocity(
                peData.velocities[i3],
                peData.velocities[i3 + 1],
                peData.velocities[i3 + 2],
            ));
        }

        let rate = 1.6 + Math.min(drive, 1) * 2.8;
        if (hasSpins && peData.spins[i]) rate *= 1.08;

        if (trackSpawns) {
            const age = frameSec - _spawnTimes.get(peData.ids[i]);
            if (age >= 0 && age < SPAWN_FLASH_DURATION) {
                const t = 1 - age / SPAWN_FLASH_DURATION;
                rate += SPAWN_FLASH_RATE_BOOST * t * t; // eased decay
            }
        }

        out[i] = rate;
    }
    return out;
}

export function buildPEForceActivity(peData, forceData) {
    return buildPEManifestBlinkRate(peData, forceData);
}

export function ensureCloudTemplate(_catalogId, _mass_mev) {
    return getUnitTemplate();
}

/**
 * @param {{ blinkRate?: Float32Array, frameSec?: number }} [opts]
 */
export function expandPEToCloud(peData, typeMap, opts = {}) {
    const blinkRates = opts.blinkRate ?? opts.forceActivity;
    const frameSec = opts.frameSec ?? 0;

    const srcCount = peData.count;
    const tmpl = getUnitTemplate();
    let out = 0;

    const hasVel = !!peData.velocities;
    const hasMass = !!peData.masses;
    const hasREff = !!peData.rEff;
    const hasCharge = !!peData.charges;
    const hasColorId = !!peData.colorIds;
    const hasSpins = !!peData.spins;

    for (let i = 0; i < srcCount && out < MAX_CLOUD_TOTAL; i++) {
        const cx = peData.positions[i * 3];
        const cy = peData.positions[i * 3 + 1];
        const cz = peData.positions[i * 3 + 2];

        const pid = peData.ids ? peData.ids[i] : -1;
        const catId = typeMap ? typeMap.get(pid) : null;
        const catalog = catId ? getById(catId) : null;

        const mass = hasMass ? peData.masses[i] : (catalog?.mass_mev ?? K_B);
        const rEff = hasREff ? peData.rEff[i] : 0.1;
        const charge = hasCharge ? peData.charges[i] : (catalog?.charge ?? 0);
        const colorId = hasColorId ? peData.colorIds[i] : 0;
        const spinSign = hasSpins ? (peData.spins[i] || 0) : 0;

        const vx = hasVel ? peData.velocities[i * 3] : 0;
        const vy = hasVel ? peData.velocities[i * 3 + 1] : 0;
        const vz = hasVel ? peData.velocities[i * 3 + 2] : 0;

        const beta = betaFromVelocity(vx, vy, vz);
        const speed2 = vx * vx + vy * vy + vz * vz;
        const keNorm = Math.min(speed2 / (C_SPEED * C_SPEED * 0.2), 1.0);

        let base = catalog ? catalog.display_color.slice() : chargeFallbackColor(charge);
        base = strongColorTint(colorId, base);
        const [br, bg, bb] = modulateColor(base, beta, keNorm);

        const orbit = orbitalSweepAngle(cx, cy, cz, vx, vy, vz, mass, frameSec);
        const slotRate = blinkRates ? blinkRates[i] : 2.2;
        const radius = visualLocalizationRadius(mass, rEff);
        const n = Math.min(pointCountForParticle(mass, radius), tmpl.n, MAX_CLOUD_TOTAL - out);

        // One persistent effective-record core anchors selection and identity.
        // This is a display marker for the native record coordinate, not a
        // claim that the effective particle has a solid spherical interior.
        const massRatio = Math.max(mass, K_B * 0.01) / K_B;
        const coreSize = Math.min(12.5, 8.4 + 0.72 * Math.log10(1 + massRatio));
        const coreWhite = 0.30 + 0.12 * keNorm;
        const corePhase = (hashUint32(pid, 0, 7) / 4294967296) * Math.PI * 2;
        if (out < MAX_CLOUD_TOTAL) {
            writeCloudPoint(
                out, cx, cy, cz,
                br * (1 - coreWhite) + coreWhite,
                bg * (1 - coreWhite) + coreWhite,
                bb * (1 - coreWhite) + coreWhite,
                coreSize, corePhase, slotRate,
                PE_APPEARANCE_ROLE.RECORD_CORE, pid,
            );
            out++;
        }

        for (let j = 0; j < n && out < MAX_CLOUD_TOTAL; j++) {
            let ox = tmpl.offsets[j * 3] * radius;
            let oy = tmpl.offsets[j * 3 + 1] * radius;
            let oz = tmpl.offsets[j * 3 + 2] * radius;

            [ox, oy, oz] = rotateOffset(ox, oy, oz, orbit.ux, orbit.uy, orbit.uz, orbit.angle);
            [ox, oy, oz] = stretchOffset(ox, oy, oz, vx, vy, vz, beta);

            const b = tmpl.brightness[j];
            const fade = 0.56 + 0.44 * b;
            const ptSize = (1.05 + b * 1.9) * (0.92 + beta * 0.20);
            let phase = (hashUint32(pid, j, 1) / 4294967296) * Math.PI * 2;
            if (spinSign) phase += spinSign * 0.72;

            writeCloudPoint(
                out,
                cx + ox, cy + oy, cz + oz,
                br * fade, bg * fade, bb * fade,
                ptSize,
                phase,
                slotRate,
                PE_APPEARANCE_ROLE.SUPPORT,
                pid,
            );
            out++;
        }

        // A sparse outer rim exposes r_eff / the bounded interaction support.
        // It is deliberately dotted, rather than a solid surface, to avoid
        // presenting a hard particle wall that the effective engine does not
        // contain.
        const rimCount = Math.min(48, Math.max(12, Math.round(Math.sqrt(n) * 1.8)));
        for (let j = 0; j < rimCount && out < MAX_CLOUD_TOTAL; j++) {
            const source = (j * 53 + 17) % tmpl.n;
            let ox = tmpl.offsets[source * 3];
            let oy = tmpl.offsets[source * 3 + 1];
            let oz = tmpl.offsets[source * 3 + 2];
            const inv = 1 / Math.max(1e-8, Math.sqrt(ox * ox + oy * oy + oz * oz));
            ox *= radius * inv;
            oy *= radius * inv;
            oz *= radius * inv;
            [ox, oy, oz] = rotateOffset(ox, oy, oz, orbit.ux, orbit.uy, orbit.uz, orbit.angle);
            [ox, oy, oz] = stretchOffset(ox, oy, oz, vx, vy, vz, beta);
            const rimWhite = 0.24;
            writeCloudPoint(
                out, cx + ox, cy + oy, cz + oz,
                br * (1 - rimWhite) + rimWhite,
                bg * (1 - rimWhite) + rimWhite,
                bb * (1 - rimWhite) + rimWhite,
                1.45 + beta * 0.35,
                corePhase + j * 0.37,
                slotRate * 0.82,
                PE_APPEARANCE_ROLE.SUPPORT_RIM,
                pid,
            );
            out++;
        }
    }

    return {
        positions: _cloudPos,
        colors: _cloudCol,
        sizes: _cloudSize,
        phases: _cloudPhase,
        rates: _cloudRate,
        roles: _cloudRole,
        particleIds: _cloudParticleMap,
        count: out,
    };
}

function fallbackKineticEnergyDensity(peData, index) {
    const mass = Math.max(0, Number(peData.masses?.[index]) || 0);
    const vx = Number(peData.velocities?.[index * 3]) || 0;
    const vy = Number(peData.velocities?.[index * 3 + 1]) || 0;
    const vz = Number(peData.velocities?.[index * 3 + 2]) || 0;
    const radius = Math.max(0.001, Number(peData.rEff?.[index]) || 0.4);
    const kineticEnergy = 0.5 * mass * (vx * vx + vy * vy + vz * vz);
    const effectiveVolume = (4 / 3) * Math.PI * radius * radius * radius;
    return kineticEnergy / effectiveVolume;
}

function appendTrailSample(trail, peData, index, tick, kineticEnergyDensity) {
    const h = trail.head;
    trail.positions[h * 3] = peData.positions[index * 3];
    trail.positions[h * 3 + 1] = peData.positions[index * 3 + 1];
    trail.positions[h * 3 + 2] = peData.positions[index * 3 + 2];
    if (peData.velocities) {
        const vx = peData.velocities[index * 3];
        const vy = peData.velocities[index * 3 + 1];
        const vz = peData.velocities[index * 3 + 2];
        trail.speeds[h] = Math.sqrt(vx * vx + vy * vy + vz * vz);
    } else {
        trail.speeds[h] = 0;
    }
    trail.ticks[h] = tick;
    trail.energyDensities[h] = Math.max(0, Number(kineticEnergyDensity) || 0);
    trail.head = (h + 1) % trail.capacity;
    trail.length = Math.min(trail.length + 1, trail.capacity);
    trail.lastSampleTick = tick;
}

function resizeTrail(trail, requestedCapacity) {
    const capacity = Math.max(24, Math.floor(requestedCapacity));
    if (trail.capacity === capacity) return trail;
    const positions = new Float32Array(capacity * 3);
    const speeds = new Float32Array(capacity);
    const ticks = new Float64Array(capacity);
    const energyDensities = new Float64Array(capacity);
    const retained = Math.min(trail.length, capacity);
    const oldest = (trail.head - retained + trail.capacity) % trail.capacity;
    for (let i = 0; i < retained; i++) {
        const source = (oldest + i) % trail.capacity;
        positions.set(trail.positions.subarray(source * 3, source * 3 + 3), i * 3);
        speeds[i] = trail.speeds[source];
        ticks[i] = trail.ticks[source];
        energyDensities[i] = trail.energyDensities[source];
    }
    trail.positions = positions;
    trail.speeds = speeds;
    trail.ticks = ticks;
    trail.energyDensities = energyDensities;
    trail.capacity = capacity;
    trail.length = retained;
    trail.head = retained % capacity;
    return trail;
}

/**
 * Capture tick-aligned trajectory history and retain despawned records long
 * enough for their tails to fade. Sampling is intentionally keyed to PE
 * ticks, not render frames, so 30/60/144 Hz displays show the same history.
 */
export function updateTrailHistory(
    peData,
    currentTick,
    candidateSettings = DEFAULT_TRAIL_SETTINGS,
    kineticEnergyDensityById = null,
) {
    const settings = normalizeTrailSettings(candidateSettings);
    const tick = Number.isFinite(Number(currentTick))
        ? Number(currentTick) : ++_fallbackTrailTick;
    const targetCapacity = trailCapacityForPopulation(
        Math.max(peData.count, _trailHistory.size));
    _activeIdsSet.clear();
    for (let i = 0; i < peData.count; i++) {
        const id = peData.ids[i];
        _activeIdsSet.add(id);
        if (!_trailHistory.has(id)) {
            _trailHistory.set(id, {
                positions: new Float32Array(targetCapacity * 3),
                head: 0,
                length: 0,
                capacity: targetCapacity,
                speeds: new Float32Array(targetCapacity),
                ticks: new Float64Array(targetCapacity),
                energyDensities: new Float64Array(targetCapacity),
                lastSampleTick: Number.NEGATIVE_INFINITY,
                lastSeenTick: tick,
                inactiveSinceTick: null,
            });
        }
        const trail = _trailHistory.get(id);
        if (targetCapacity > trail.capacity || targetCapacity * 2 < trail.capacity) {
            resizeTrail(trail, targetCapacity);
        }
        if (tick < trail.lastSampleTick) {
            trail.head = 0;
            trail.length = 0;
            trail.lastSampleTick = Number.NEGATIVE_INFINITY;
        }
        trail.lastSeenTick = tick;
        trail.inactiveSinceTick = null;
        if (tick - trail.lastSampleTick >= settings.sampleEveryTicks) {
            const nativeDensity = kineticEnergyDensityById?.get?.(Number(id));
            appendTrailSample(
                trail,
                peData,
                i,
                tick,
                Number.isFinite(nativeDensity)
                    ? nativeDensity : fallbackKineticEnergyDensity(peData, i),
            );
        }
    }

    for (const [id, trail] of _trailHistory) {
        if (_activeIdsSet.has(id)) continue;
        if (!Number.isFinite(trail.inactiveSinceTick)) trail.inactiveSinceTick = tick;
        if (settings.disappearDelayTicks === 0
            || tick - trail.inactiveSinceTick > settings.disappearDelayTicks) {
            _trailHistory.delete(id);
        }
    }
}

export function getCloudParticleMap() { return _cloudParticleMap; }
export function getTrailHistory() { return _trailHistory; }

export function clearCloudAndTrails() {
    _unitTemplate = null;
    _trailHistory.clear();
    _spawnTimes.clear();
    _fallbackTrailTick = 0;
}
