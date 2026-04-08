/**
 * CosmicMockBridge — JS-only N-body simulation for cosmic scale (Scale 5).
 *
 * Architecture follows Gadget-2 conventions:
 *   1. _computeForces():  gravity + hydro + external (NO mass changes)
 *   2. tick():            kick-drift-recompute-kick (Velocity Verlet)
 *   3. _postUpdates():    accretion, star formation, mergers, cleanup
 *
 * Softening: fixed per body type (energy-conserving).
 * Pairwise rule: eps = max(eps_i, eps_j) per Gadget-2 standard.
 *
 * Unit system: G = G_N = 0.01 (FTD ontic chain). Masses and distances
 * scaled so v_circular ~ O(1) for visual dynamics.
 */

import { G_N, OMEGA_LAMBDA, OMEGA_MATTER } from '../constants.js';

// Fixed softening per body type (Gadget-2 convention: constant, energy-conserving)
// BH/Quasar:       tiny — point-like, dominates core
// Stars/NS/WD:     medium — collisionless, moderate smoothing
// DM:              large — diffuse halo, suppresses two-body relaxation
// Gas/Nebula:      medium — collisional, SPH-like smoothing
// At N~800, no body should act as a true point source. The softening
// represents the unresolved internal structure at this particle count.
// BH softening is comparable to stars — its dominance comes from MASS
// (500 vs ~2 per star), not from having sharper gravity.
const SOFTENING = {
    [-3]: 3.0,  // DARK_ENERGY
    [-2]: 2.0,  // QUASAR
    [-1]: 2.0,  // BLACK_HOLE — same as stars; dominates via mass, not sharpness
    [0]:  3.5,  // DARK_MATTER — diffuse halo
    [1]:  2.5,  // GAS
    [2]:  2.5,  // STAR
    [3]:  2.0,  // NEUTRON_STAR
    [4]:  2.5,  // NEBULA
    [5]:  2.5,  // WHITE_DWARF
};

export class CosmicMockBridge {
    constructor() {
        this._bodies = [];
        this._tick = 0;
        this._nextId = 0;
        this._dt = 0.01;
        this._a = 1.0;
        this._adot = 0.0;
        this._H0 = 0.001;
        this._boxSize = 200;
        this._softening = 5.0; // base softening (used as fallback)
        this._gwEvents = [];
        this._t_cosmic = 0.0;
        this._enableSubgrid = false;
    }

    static TYPE = {
        DARK_ENERGY: -3, QUASAR: -2, BLACK_HOLE: -1,
        DARK_MATTER: 0, GAS: 1, STAR: 2,
        NEUTRON_STAR: 3, NEBULA: 4, WHITE_DWARF: 5
    };

    addBody(type, mass, x, y, z, vx=0, vy=0, vz=0, temp=0) {
        const id = this._nextId++;
        this._bodies.push({
            id, type, mass,
            x, y, z, vx, vy, vz,
            ax: 0, ay: 0, az: 0,
            temperature: temp,
            internal_energy: Math.max(temp * 0.001, 0.01),
            density: 0, pressure: 0,
            luminosity: type === 2 ? Math.pow(mass, 3.5) : 0,
            radius: Math.cbrt(mass) * 0.1,
        });
        return id;
    }

    _enclosedMass(r, M_total, rs) {
        return M_total * r * r * r / Math.pow(r * r + rs * rs, 1.5);
    }

    // ================================================================
    // SCENARIOS
    // ================================================================

    setupScenario(name) {
        this._bodies = [];
        this._nextId = 0;
        this._tick = 0;
        this._a = 1.0;
        this._gwEvents = [];
        this._t_cosmic = 0.0;

        const T = CosmicMockBridge.TYPE;
        const rng = this._rng(42);
        const PI2 = Math.PI * 2;
        // Box-Muller for Gaussian random numbers (needed for z-dispersion)
        const randn = () => Math.sqrt(-2 * Math.log(rng() + 1e-10)) * Math.cos(PI2 * rng());

        if (name === 'cosmic-galaxy') {
            const M_total = 5000;
            const M_bh = 500; // 10% — dominates core
            const M_dm = (M_total - M_bh) * 0.85;
            const M_disk = (M_total - M_bh) * 0.15;
            const r_s = 30;
            const r_disk = 50;

            this.addBody(T.BLACK_HOLE, M_bh, 0, 0, 0);

            // DM halo (Hernquist profile, virial equilibrium)
            for (let i = 0; i < 300; i++) {
                const u = rng() * 0.95;
                const su = Math.sqrt(u);
                const r = Math.min(r_s * su / (1.0 - su), 120);
                const th = Math.acos(2 * rng() - 1);
                const ph = PI2 * rng();

                const M_enc = this._enclosedMass(r, M_total, r_s);
                // Virial dispersion: sigma ~ 0.7 * v_c for Hernquist equilibrium
                const sigma = Math.sqrt(G_N * M_enc / Math.max(r, 2)) * 0.7;
                this.addBody(T.DARK_MATTER, M_dm / 300,
                    r * Math.sin(th) * Math.cos(ph),
                    r * Math.sin(th) * Math.sin(ph),
                    r * Math.cos(th),
                    sigma * randn(), sigma * randn(), sigma * randn());
            }

            // Stellar disk (with z-dispersion for vertical stability)
            for (let i = 0; i < 350; i++) {
                const r = 2 + rng() * r_disk;
                const arm = Math.floor(rng() * 2);
                const phi_base = arm * Math.PI + 0.4 * Math.log(r + 1);
                const ph = phi_base + (rng() - 0.5) * 0.7;
                const zz = randn() * 1.5;

                const M_enc = M_bh + this._enclosedMass(r, M_dm, r_s) + this._enclosedMass(r, M_disk, r_disk * 0.5);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 2));
                const vz = randn() * vc * 0.1; // z-velocity dispersion for vertical support

                this.addBody(T.STAR, M_disk * 0.6 / 350,
                    r * Math.cos(ph), zz, r * Math.sin(ph),
                    -vc * Math.sin(ph), vz, vc * Math.cos(ph),
                    3000 + rng() * 25000);
            }

            // Gas disk
            for (let i = 0; i < 150; i++) {
                const r = 3 + rng() * r_disk * 1.3;
                const arm = Math.floor(rng() * 2);
                const phi_base = arm * Math.PI + 0.4 * Math.log(r + 1);
                const ph = phi_base + (rng() - 0.5) * 1.0;
                const zz = randn() * 1.0;

                const M_enc = M_bh + this._enclosedMass(r, M_dm, r_s) + this._enclosedMass(r, M_disk, r_disk * 0.5);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 2));

                this.addBody(T.GAS, M_disk * 0.4 / 150,
                    r * Math.cos(ph), zz, r * Math.sin(ph),
                    -vc * Math.sin(ph), 0, vc * Math.cos(ph),
                    5000 + rng() * 15000);
            }

            this._boxSize = 200;
            this._softening = 5.0;
            this._dt = 0.05;
            this._enableSubgrid = false;

        } else if (name === 'cosmic-black-hole') {
            const M_bh = 500;
            this.addBody(T.BLACK_HOLE, M_bh, 0, 0, 0);

            for (let i = 0; i < 500; i++) {
                const u = rng();
                const r = 4 + u * u * 46;
                const ph = PI2 * rng();
                const zz = randn() * 0.25 * (r / 10);

                const vk = Math.sqrt(G_N * M_bh / r);
                const v_factor = 0.99 - 0.01 * rng();

                this.addBody(T.GAS, 0.2,
                    r * Math.cos(ph), zz, r * Math.sin(ph),
                    -vk * v_factor * Math.sin(ph), 0, vk * v_factor * Math.cos(ph),
                    1e6 * Math.pow(4 / r, 0.75));
            }

            this._boxSize = 120;
            this._softening = 2.0;
            this._dt = 0.03;
            this._enableSubgrid = true;

        } else if (name === 'cosmic-merger') {
            const M1 = 3000, M2 = 2000;
            const sep = 80;
            const v_esc = Math.sqrt(2 * G_N * (M1 + M2) / sep);
            const v_approach = v_esc * 0.45;
            const b = 10;
            const r_s1 = 20, r_s2 = 16;

            const cx1 = -sep / 2, cz1 = -b / 2;
            this.addBody(T.BLACK_HOLE, M1 * 0.05, cx1, 0, cz1, v_approach, 0, v_approach * 0.15);
            for (let i = 0; i < 250; i++) {
                const r = rng() * r_s1 * 1.8;
                const ph = PI2 * rng();
                const zz = randn() * 1.5;
                const t = i < 125 ? T.DARK_MATTER : T.STAR;
                const M_enc = this._enclosedMass(r, M1, r_s1);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 1));
                const M1_remaining = M1 * 0.95;
                this.addBody(t, (t === T.DARK_MATTER ? M1_remaining * 0.85 : M1_remaining * 0.15) / 125,
                    cx1 + r * Math.cos(ph), zz, cz1 + r * Math.sin(ph),
                    v_approach - vc * Math.sin(ph), randn() * vc * 0.05, v_approach * 0.15 + vc * Math.cos(ph),
                    t === T.STAR ? 4000 + rng() * 18000 : 0);
            }

            const cx2 = sep / 2, cz2 = b / 2;
            this.addBody(T.BLACK_HOLE, M2 * 0.05, cx2, 0, cz2, -v_approach, 0, -v_approach * 0.15);
            for (let i = 0; i < 200; i++) {
                const r = rng() * r_s2 * 1.8;
                const ph = PI2 * rng();
                const zz = randn() * 1.5;
                const t = i < 100 ? T.DARK_MATTER : T.STAR;
                const M_enc = this._enclosedMass(r, M2, r_s2);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 1));
                const M2_remaining = M2 * 0.95;
                this.addBody(t, (t === T.DARK_MATTER ? M2_remaining * 0.85 : M2_remaining * 0.15) / 100,
                    cx2 + r * Math.cos(ph), zz, cz2 + r * Math.sin(ph),
                    -v_approach - vc * Math.sin(ph), randn() * vc * 0.05, -v_approach * 0.15 + vc * Math.cos(ph),
                    t === T.STAR ? 4000 + rng() * 18000 : 0);
            }

            this._boxSize = 250;
            this._softening = 4.0;
            this._dt = 0.04;
            this._enableSubgrid = false;

        } else {
            // Cosmic web
            for (let i = 0; i < 700; i++) {
                const x = (rng() - 0.5) * 200;
                const y = (rng() - 0.5) * 200;
                const z = (rng() - 0.5) * 200;
                const kx = 2 * Math.PI / 100;
                const amp = 0.3;
                this.addBody(T.DARK_MATTER, 5, x, y, z,
                    -amp * kx * Math.sin(kx * x) * (1 + 0.5 * Math.cos(kx * y)),
                    -amp * kx * Math.sin(kx * y) * (1 + 0.5 * Math.cos(kx * z)),
                    -amp * kx * Math.sin(kx * z) * (1 + 0.5 * Math.cos(kx * x)));
            }
            for (let i = 0; i < 100; i++) {
                this.addBody(T.GAS, 5, (rng()-0.5)*200, (rng()-0.5)*200, (rng()-0.5)*200, 0, 0, 0, 1e4);
            }
            this._boxSize = 200;
            this._softening = 6.0;
            this._dt = 0.08;
            this._enableSubgrid = false;
        }
    }

    // ================================================================
    // FORCE COMPUTATION — pure forces only, no mass changes
    // ================================================================

    _computeForces() {
        const G = G_N;
        const n = this._bodies.length;

        for (const b of this._bodies) { b.ax = 0; b.ay = 0; b.az = 0; }

        // Pairwise gravity with fixed per-type softening
        for (let i = 0; i < n; i++) {
            const bi = this._bodies[i];
            const si = SOFTENING[bi.type] || 2.0;
            for (let j = i + 1; j < n; j++) {
                const bj = this._bodies[j];
                const sj = SOFTENING[bj.type] || 2.0;
                // Gadget-2 rule: eps = max(eps_i, eps_j)
                const eps = Math.max(si, sj);
                const eps2 = eps * eps;

                const dx = bj.x - bi.x;
                const dy = bj.y - bi.y;
                const dz = bj.z - bi.z;
                const r2 = dx * dx + dy * dy + dz * dz + eps2;
                const invR3 = 1.0 / (r2 * Math.sqrt(r2));

                const Gj = G * bj.mass * invR3;
                const Gi = G * bi.mass * invR3;
                bi.ax += Gj * dx; bi.ay += Gj * dy; bi.az += Gj * dz;
                bj.ax -= Gi * dx; bj.ay -= Gi * dy; bj.az -= Gi * dz;
            }
        }

        // Sub-grid physics (BH accretion scenario only)
        if (!this._enableSubgrid) return;

        const T = CosmicMockBridge.TYPE;
        const isGas = (t) => t === T.GAS || t === T.NEBULA;
        const isStar = (t) => t === T.STAR || t === T.NEUTRON_STAR || t === T.WHITE_DWARF;
        const isBH = (t) => t === T.BLACK_HOLE || t === T.QUASAR;
        const baseSoft2 = this._softening * this._softening;

        // Tidal spaghettification (conservative — radial stretch only)
        for (const bh of this._bodies) {
            if (!isBH(bh.type)) continue;
            const r_tidal = Math.max(8.0, Math.cbrt(bh.mass) * 1.5);
            const r_tidal2 = r_tidal * r_tidal;
            for (const b of this._bodies) {
                if (b.id === bh.id) continue;
                const dx = b.x - bh.x, dy = b.y - bh.y, dz = b.z - bh.z;
                const r2 = dx * dx + dy * dy + dz * dz;
                if (r2 > r_tidal2 || r2 < 0.01) continue;
                const r = Math.sqrt(r2);
                const invR = 1.0 / r;
                const rx = dx * invR, ry = dy * invR, rz = dz * invR;
                // Conservative tidal stretch: a = +2*G*M/(r^3) along r-hat
                const tidalStrength = 2.0 * G * bh.mass / (r2 * r) * 0.3;
                b.ax += tidalStrength * rx;
                b.ay += tidalStrength * ry;
                b.az += tidalStrength * rz;
            }
        }

        // Gas cooling — reduces internal energy (NOT velocity drag)
        for (const b of this._bodies) {
            if (!isGas(b.type)) continue;
            let localDensity = b.mass;
            for (const other of this._bodies) {
                if (other.id === b.id || !isGas(other.type)) continue;
                const dr2 = (b.x-other.x)**2 + (b.y-other.y)**2 + (b.z-other.z)**2;
                if (dr2 < baseSoft2 * 25) localDensity += other.mass;
            }
            const coolingRate = Math.min(0.0002, 0.000002 * localDensity);
            // Energy-based cooling: reduce internal energy → pressure drops naturally
            b.internal_energy = Math.max(0.001, b.internal_energy * (1 - coolingRate));
            b.temperature = Math.max(100, b.internal_energy * 1000);
        }

        // Gas pressure (SPH-like repulsion)
        const h_press = this._softening * 2.5;
        const h_press2 = h_press * h_press;
        for (let i = 0; i < n; i++) {
            const bi = this._bodies[i];
            if (!isGas(bi.type)) continue;
            for (let j = i + 1; j < n; j++) {
                const bj = this._bodies[j];
                if (!isGas(bj.type)) continue;
                const dx = bj.x - bi.x, dy = bj.y - bi.y, dz = bj.z - bi.z;
                const r2 = dx * dx + dy * dy + dz * dz;
                if (r2 > h_press2 || r2 < 1e-10) continue;
                const r = Math.sqrt(r2);
                const q = r / h_press;
                const T_avg = 0.5 * (bi.internal_energy + bj.internal_energy);
                const pressScale = 1.0 + T_avg * 0.1;
                const fmag = G * pressScale * 0.3 * (bi.mass + bj.mass) * (1 - q) * (1 - q) / (r2 + baseSoft2);
                bi.ax -= fmag * dx / r; bi.ay -= fmag * dy / r; bi.az -= fmag * dz / r;
                bj.ax += fmag * dx / r; bj.ay += fmag * dy / r; bj.az += fmag * dz / r;
            }
        }

        // Stellar radiation pressure on gas
        for (const star of this._bodies) {
            if (!isStar(star.type) || star.luminosity <= 0) continue;
            for (const gas of this._bodies) {
                if (!isGas(gas.type)) continue;
                const dx = gas.x - star.x, dy = gas.y - star.y, dz = gas.z - star.z;
                const r2 = dx * dx + dy * dy + dz * dz + baseSoft2;
                if (r2 > 400) continue;
                const r = Math.sqrt(r2);
                const f_rad = star.luminosity / (4 * Math.PI * r2 * 0.577) * 0.001;
                gas.ax += f_rad * dx / r;
                gas.ay += f_rad * dy / r;
                gas.az += f_rad * dz / r;
            }
        }
    }

    // ================================================================
    // POST-INTEGRATION UPDATES — mass changes, mergers, cleanup
    // Called AFTER velocity Verlet is complete (Gadget-2 convention)
    // ================================================================

    _postUpdates() {
        if (!this._enableSubgrid) {
            // Even without subgrid, enforce speed limit
            this._enforceSpeedLimit();
            return;
        }

        const T = CosmicMockBridge.TYPE;
        const G = G_N;
        const isGas = (t) => t === T.GAS || t === T.NEBULA;
        const isBH = (t) => t === T.BLACK_HOLE || t === T.QUASAR;
        const baseSoft2 = this._softening * this._softening;

        // Star formation (dense cold gas → star)
        const newStars = [];
        for (const b of this._bodies) {
            if (!isGas(b.type) || b.mass < 0.5) continue;
            let nearby = 0;
            for (const other of this._bodies) {
                if (other.id === b.id || !isGas(other.type)) continue;
                const dr2 = (b.x-other.x)**2 + (b.y-other.y)**2 + (b.z-other.z)**2;
                if (dr2 < baseSoft2 * 9) nearby++;
            }
            if (nearby > 10 && b.temperature < 3000 && Math.random() < 0.01) {
                const starMass = b.mass * 0.15;
                b.mass -= starMass;
                newStars.push({type: T.STAR, mass: starMass,
                    x: b.x, y: b.y, z: b.z, vx: b.vx, vy: b.vy, vz: b.vz,
                    temp: 5800, lum: Math.pow(starMass, 3.5)});
            }
        }
        for (const s of newStars) {
            this.addBody(s.type, s.mass, s.x, s.y, s.z, s.vx, s.vy, s.vz, s.temp);
            this._bodies[this._bodies.length - 1].luminosity = s.lum;
        }

        // BH accretion (bound gas → BH)
        for (const bh of this._bodies) {
            if (!isBH(bh.type)) continue;
            const r_acc = Math.max(1.5, Math.cbrt(bh.mass) * 0.3);
            const r_acc2 = r_acc * r_acc;
            for (const gas of this._bodies) {
                if (!isGas(gas.type) || gas.mass <= 0) continue;
                const dx = gas.x - bh.x, dy = gas.y - bh.y, dz = gas.z - bh.z;
                const r2 = dx * dx + dy * dy + dz * dz;
                if (r2 > r_acc2) continue;
                const dvx = gas.vx - bh.vx, dvy = gas.vy - bh.vy, dvz = gas.vz - bh.vz;
                const v_rel2 = dvx*dvx + dvy*dvy + dvz*dvz;
                const r = Math.sqrt(r2 + 0.01);
                if (v_rel2 > 2 * G * bh.mass / r) continue; // only bound gas
                const rate = 0.005 * bh.mass / (v_rel2 + 0.1);
                const dm = Math.min(gas.mass * 0.1, gas.mass * rate * 0.001);
                bh.mass += dm;
                gas.mass -= dm;
            }
        }

        // BH-BH mergers
        for (let i = 0; i < this._bodies.length; i++) {
            const bi = this._bodies[i];
            if (!isBH(bi.type) || bi.mass <= 0) continue;
            for (let j = i + 1; j < this._bodies.length; j++) {
                const bj = this._bodies[j];
                if (!isBH(bj.type) || bj.mass <= 0) continue;
                const dx = bj.x-bi.x, dy = bj.y-bi.y, dz = bj.z-bi.z;
                const r2 = dx*dx + dy*dy + dz*dz;
                const r_merge = Math.cbrt(bi.mass + bj.mass) * 0.3;
                if (r2 > r_merge * r_merge) continue;
                const m_total = bi.mass + bj.mass;
                bi.vx = (bi.vx*bi.mass + bj.vx*bj.mass) / m_total;
                bi.vy = (bi.vy*bi.mass + bj.vy*bj.mass) / m_total;
                bi.vz = (bi.vz*bi.mass + bj.vz*bj.mass) / m_total;
                bi.mass = m_total * 0.95;
                bj.mass = 0;
            }
        }

        // Speed limit
        this._enforceSpeedLimit();

        // Remove depleted bodies
        this._bodies = this._bodies.filter(b => b.mass > 0.01);
    }

    _enforceSpeedLimit() {
        const c2 = 1.0 / 3.0;
        for (const b of this._bodies) {
            const v2 = b.vx*b.vx + b.vy*b.vy + b.vz*b.vz;
            if (v2 > c2) {
                const s = Math.sqrt(c2 / v2);
                b.vx *= s; b.vy *= s; b.vz *= s;
            }
        }
    }

    // ================================================================
    // TICK — Velocity Verlet (Gadget-2 kick-drift-kick)
    // ================================================================

    tick() {
        const n = this._bodies.length;
        if (n === 0) return;
        const dt = this._dt;

        // Step 1: half-kick with CURRENT accelerations
        for (const b of this._bodies) {
            b.vx += 0.5 * dt * b.ax;
            b.vy += 0.5 * dt * b.ay;
            b.vz += 0.5 * dt * b.az;
        }
        // Step 2: drift
        for (const b of this._bodies) {
            b.x += dt * b.vx;
            b.y += dt * b.vy;
            b.z += dt * b.vz;
        }
        // Step 3: recompute forces at NEW positions
        this._computeForces();
        // Step 4: second half-kick with FRESH forces
        for (const b of this._bodies) {
            b.vx += 0.5 * dt * b.ax;
            b.vy += 0.5 * dt * b.ay;
            b.vz += 0.5 * dt * b.az;
        }
        // Step 5: post-integration updates (mass changes, mergers, cleanup)
        this._postUpdates();

        this._t_cosmic += dt;
        this._tick++;
    }

    run(nTicks) { for (let i = 0; i < nTicks; i++) this.tick(); }

    // ================================================================
    // DATA OUTPUT
    // ================================================================

    getCosmicData() {
        const n = this._bodies.length;
        const positions = new Float32Array(n * 3);
        const types = new Int8Array(n);
        const temperatures = new Float32Array(n);
        const sizes = new Float32Array(n);
        const densities = new Float32Array(n);
        const luminosities = new Float32Array(n);

        for (let i = 0; i < n; i++) {
            const b = this._bodies[i];
            positions[i*3] = b.x; positions[i*3+1] = b.y; positions[i*3+2] = b.z;
            types[i] = b.type;
            temperatures[i] = b.temperature;
            sizes[i] = Math.cbrt(b.mass);
            densities[i] = b.density || 0.1;
            luminosities[i] = b.luminosity;
        }
        return { positions, types, temperatures, sizes, densities, luminosities, count: n };
    }

    getDiagnostics() {
        let totalMass = 0, totalKE = 0;
        const counts = new Array(9).fill(0);
        for (const b of this._bodies) {
            totalMass += b.mass;
            totalKE += 0.5 * b.mass * (b.vx*b.vx + b.vy*b.vy + b.vz*b.vz);
            const idx = b.type + 3;
            if (idx >= 0 && idx < 9) counts[idx]++;
        }
        return {
            tick: this._tick, bodyCount: this._bodies.length,
            countsByType: counts, totalMass, totalKE,
            hubbleParameter: this._H0, scaleFactor: this._a,
            omegaMatter: OMEGA_MATTER, omegaLambda: OMEGA_LAMBDA
        };
    }

    setDt(dt) { this._dt = dt; }
    getDt() { return this._dt; }
    clear() { this._bodies = []; this._tick = 0; this._nextId = 0; }

    _rng(seed) {
        let s = seed;
        return () => { s = (s * 16807) % 2147483647; return s / 2147483647; };
    }
}
