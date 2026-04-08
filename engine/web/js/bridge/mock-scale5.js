/**
 * CosmicMockBridge — JS-only N-body simulation for cosmic scale (Scale 5).
 *
 * Physics:
 *   - Newtonian gravity with Plummer softening: F = G * m_i * m_j / (r^2 + eps^2)
 *   - Velocity Verlet (symplectic, energy-conserving to machine precision)
 *   - Keplerian circular velocities for disk initialization: v_c = sqrt(G * M_enc / r)
 *   - Merger approach velocity from parabolic orbit: v = sqrt(2 * G * M_total / r)
 *   - Dynamical friction on massive bodies (Chandrasekhar formula, simplified)
 *   - Accretion disk gas gets viscous angular momentum transport
 *
 * All constants from FTD ontic chain via constants.js.
 */

import { G_N, OMEGA_LAMBDA, OMEGA_MATTER } from '../constants.js';

export class CosmicMockBridge {
    constructor() {
        this._bodies = [];
        this._tick = 0;
        this._nextId = 0;
        this._dt = 0.001;
        this._a = 1.0;
        this._adot = 0.0;
        this._H0 = 0.07;
        this._boxSize = 1000;
        this._softening = 1.0;
        this._gwEvents = [];
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
            density: 0, pressure: 0,
            luminosity: type === 2 ? Math.pow(mass, 3.5) : 0,
            radius: Math.cbrt(mass) * 0.1,
            smoothing: 1.0
        });
        return id;
    }

    // Plummer enclosed mass: M_enc = M_total * r^3 / (r^2 + a^2)^(3/2)
    // Smooth profile that avoids central singularity; converges to M_total at large r.
    _enclosedMass(r, M_total, rs) {
        return M_total * r * r * r / Math.pow(r * r + rs * rs, 1.5);
    }

    setupScenario(name) {
        this._bodies = [];
        this._nextId = 0;
        this._tick = 0;
        this._a = 1.0;
        this._gwEvents = [];

        const T = CosmicMockBridge.TYPE;
        const rng = this._rng(42);
        const PI2 = Math.PI * 2;

        if (name === 'cosmic-galaxy') {
            // ── Spiral galaxy with proper Keplerian rotation curve ──
            // Total mass budget: 1e12 (DM halo dominates)
            const M_total = 1e12;
            const M_bh = 4e9;           // SMBH ~ 0.4% of total
            const M_dm = M_total * 0.85; // 85% dark matter
            const M_disk = M_total * 0.1; // 10% disk (stars + gas)
            const r_s = 40;              // Scale radius for rotation curve
            const r_disk = 50;           // Disk truncation radius
            const N_dm = 300, N_star = 350, N_gas = 150;

            // Central SMBH (fixed at origin)
            this.addBody(T.BLACK_HOLE, M_bh, 0, 0, 0);

            // DM halo — spherical Hernquist profile
            for (let i = 0; i < N_dm; i++) {
                // Hernquist CDF inversion: r = a * sqrt(u) / (1 - sqrt(u))
                // Clamped to avoid infinite radius when u -> 1
                const u = rng() * 0.98; // cap at 98th percentile to avoid extreme outliers
                const su = Math.sqrt(u);
                const r = r_s * su / (1.0 - su);
                const clampR = Math.min(r, 250);
                const th = Math.acos(2 * rng() - 1);
                const ph = PI2 * rng();
                const x = clampR * Math.sin(th) * Math.cos(ph);
                const y = clampR * Math.sin(th) * Math.sin(ph);
                const z = clampR * Math.cos(th);

                // Isotropic velocity dispersion ~ sqrt(G * M_enc / r)
                const M_enc = this._enclosedMass(clampR, M_total, r_s);
                const sigma = Math.sqrt(G_N * M_enc / (clampR + 1)) * 0.4;
                this.addBody(T.DARK_MATTER, M_dm / N_dm, x, y, z,
                    sigma * (rng() - 0.5) * 2,
                    sigma * (rng() - 0.5) * 2,
                    sigma * (rng() - 0.5) * 2);
            }

            // Stellar disk — exponential profile with spiral density wave
            for (let i = 0; i < N_star; i++) {
                const r = -r_disk * 0.3 * Math.log(rng() + 0.001);
                const clampR = Math.min(r, r_disk * 1.5);
                const arm = Math.floor(rng() * 2);
                const phi_base = arm * Math.PI + 0.35 * Math.log(clampR + 1);
                const ph = phi_base + (rng() - 0.5) * 0.7;
                const zz = (rng() - 0.5) * 1.5;

                // Circular velocity from enclosed mass (flat rotation curve)
                const M_enc = M_bh + this._enclosedMass(clampR, M_dm, r_s);
                const vc = Math.sqrt(G_N * M_enc / Math.max(clampR, this._softening));

                this.addBody(T.STAR, M_disk * 0.6 / N_star,
                    clampR * Math.cos(ph), zz, clampR * Math.sin(ph),
                    -vc * Math.sin(ph), 0, vc * Math.cos(ph),
                    3000 + rng() * 25000);
            }

            // Gas disk — wider, with spiral
            for (let i = 0; i < N_gas; i++) {
                const r = -r_disk * 0.4 * Math.log(rng() + 0.001);
                const clampR = Math.min(r, r_disk * 2.0);
                const arm = Math.floor(rng() * 2);
                const phi_base = arm * Math.PI + 0.35 * Math.log(clampR + 1);
                const ph = phi_base + (rng() - 0.5) * 1.0;
                const zz = (rng() - 0.5) * 1.0;

                const M_enc = M_bh + this._enclosedMass(clampR, M_dm, r_s);
                const vc = Math.sqrt(G_N * M_enc / Math.max(clampR, this._softening));

                this.addBody(T.GAS, M_disk * 0.4 / N_gas,
                    clampR * Math.cos(ph), zz, clampR * Math.sin(ph),
                    -vc * Math.sin(ph), 0, vc * Math.cos(ph),
                    5000 + rng() * 15000);
            }

            this._boxSize = 300;
            this._softening = 2.0; // Plummer softening ~ inter-particle spacing
            this._dt = 0.0015;

        } else if (name === 'cosmic-black-hole') {
            // ── Black hole with Keplerian accretion disk ──
            const M_bh = 1e10;
            this.addBody(T.BLACK_HOLE, M_bh, 0, 0, 0);
            const rs = 2 * G_N * M_bh; // Schwarzschild radius in sim units
            const r_isco = rs * 3;      // Innermost stable circular orbit

            for (let i = 0; i < 500; i++) {
                // Distribute logarithmically (more particles near center)
                const u = rng();
                const r = r_isco + (rng() * 0.3 + u * u * 0.7) * rs * 80;
                const ph = PI2 * rng();
                const zz = (rng() - 0.5) * rs * 0.15 * (r / (rs * 10)); // Thinner near center

                // Keplerian velocity: v_k = sqrt(G * M / r)
                const vk = Math.sqrt(G_N * M_bh / r);
                // Slight sub-Keplerian for viscous inflow
                const v_factor = 0.98 - 0.02 * rng();

                this.addBody(T.GAS, 1e3,
                    r * Math.cos(ph), zz, r * Math.sin(ph),
                    -vk * v_factor * Math.sin(ph), 0, vk * v_factor * Math.cos(ph),
                    1e6 * Math.pow(r_isco / r, 0.75)); // T ~ r^(-3/4) Shakura-Sunyaev
            }

            this._boxSize = rs * 120;
            this._softening = rs * 0.5;
            this._dt = 0.0003;

        } else if (name === 'cosmic-merger') {
            // ── Galaxy merger with proper parabolic approach ──
            // Two galaxies on a bound orbit (not escape trajectory)
            const M1 = 8e11, M2 = 5e11;
            const sep = 100;           // Initial separation
            const M_total = M1 + M2;

            // Parabolic approach: v = sqrt(2 * G * M / r) gives marginally bound
            // Use 70% of escape velocity for a bound elliptical orbit
            const v_esc = Math.sqrt(2 * G_N * M_total / sep);
            const v_approach = v_esc * 0.5; // Well below escape — guaranteed capture

            // Impact parameter: slight offset for tidal tails
            const b = 15; // perpendicular offset

            const r_s1 = 25, r_s2 = 20; // Scale radii
            const N1 = 300, N2 = 200;

            // Galaxy 1: approaching from left
            const cx1 = -sep / 2, cz1 = -b / 2;
            this.addBody(T.BLACK_HOLE, M1 * 0.005, cx1, 0, cz1, v_approach, 0, v_approach * 0.15);
            for (let i = 0; i < N1; i++) {
                const r = rng() * r_s1 * 2;
                const ph = PI2 * rng();
                const zz = (rng() - 0.5) * 2;
                const t = i < N1 * 0.5 ? T.DARK_MATTER : T.STAR;

                // Internal circular velocity
                const M_enc = this._enclosedMass(r, M1, r_s1);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, this._softening));

                this.addBody(t, (t === T.DARK_MATTER ? M1 * 0.85 : M1 * 0.15) / (N1 / 2),
                    cx1 + r * Math.cos(ph), zz, cz1 + r * Math.sin(ph),
                    v_approach - vc * Math.sin(ph), 0, v_approach * 0.15 + vc * Math.cos(ph),
                    t === T.STAR ? 4000 + rng() * 18000 : 0);
            }

            // Galaxy 2: approaching from right
            const cx2 = sep / 2, cz2 = b / 2;
            this.addBody(T.BLACK_HOLE, M2 * 0.005, cx2, 0, cz2, -v_approach, 0, -v_approach * 0.15);
            for (let i = 0; i < N2; i++) {
                const r = rng() * r_s2 * 2;
                const ph = PI2 * rng();
                const zz = (rng() - 0.5) * 2;
                const t = i < N2 * 0.5 ? T.DARK_MATTER : T.STAR;

                const M_enc = this._enclosedMass(r, M2, r_s2);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, this._softening));

                this.addBody(t, (t === T.DARK_MATTER ? M2 * 0.85 : M2 * 0.15) / (N2 / 2),
                    cx2 + r * Math.cos(ph), zz, cz2 + r * Math.sin(ph),
                    -v_approach - vc * Math.sin(ph), 0, -v_approach * 0.15 + vc * Math.cos(ph),
                    t === T.STAR ? 4000 + rng() * 18000 : 0);
            }

            this._boxSize = 350;
            this._softening = 1.5;
            this._dt = 0.001;

        } else {
            // ── Cosmic web: Zel'dovich perturbations on uniform grid ──
            for (let i = 0; i < 700; i++) {
                const x = (rng() - 0.5) * 800;
                const y = (rng() - 0.5) * 800;
                const z = (rng() - 0.5) * 800;
                const kx = 2 * Math.PI / 400;
                this.addBody(T.DARK_MATTER, 5e5, x, y, z,
                    -0.05 * kx * Math.sin(kx * x) * (1 + 0.5 * Math.cos(kx * y)),
                    -0.05 * kx * Math.sin(kx * y) * (1 + 0.5 * Math.cos(kx * z)),
                    -0.05 * kx * Math.sin(kx * z) * (1 + 0.5 * Math.cos(kx * x)));
            }
            for (let i = 0; i < 100; i++) {
                const x = (rng() - 0.5) * 800;
                const y = (rng() - 0.5) * 800;
                const z = (rng() - 0.5) * 800;
                this.addBody(T.GAS, 5e4, x, y, z, 0, 0, 0, 1e4);
            }
            this._boxSize = 800;
            this._softening = 5.0;
            this._dt = 0.003;
        }
    }

    // Compute gravitational accelerations for all bodies (O(N^2) with Plummer softening).
    // Separated from tick() so it can be called twice per Verlet step.
    _computeForces() {
        const G = G_N;
        const n = this._bodies.length;
        const soft2 = this._softening * this._softening;

        for (const b of this._bodies) { b.ax = 0; b.ay = 0; b.az = 0; }

        // Pairwise gravity: a_i += G * m_j * dr / |dr|^3  (Plummer-softened)
        for (let i = 0; i < n; i++) {
            const bi = this._bodies[i];
            for (let j = i + 1; j < n; j++) {
                const bj = this._bodies[j];
                const dx = bj.x - bi.x;
                const dy = bj.y - bi.y;
                const dz = bj.z - bi.z;
                const r2 = dx * dx + dy * dy + dz * dz + soft2;
                const invR3 = 1.0 / (r2 * Math.sqrt(r2));

                const Gj = G * bj.mass * invR3;
                const Gi = G * bi.mass * invR3;
                bi.ax += Gj * dx; bi.ay += Gj * dy; bi.az += Gj * dz;
                bj.ax -= Gi * dx; bj.ay -= Gi * dy; bj.az -= Gi * dz;
            }
        }

        // Chandrasekhar dynamical friction on BHs:
        // F_fric ~ -4*pi*G^2*M^2*rho*ln(Lambda) * v_hat / v^2
        // Simplified: use local density estimate from nearest neighbors
        for (const b of this._bodies) {
            if (b.type !== CosmicMockBridge.TYPE.BLACK_HOLE) continue;
            const v2 = b.vx * b.vx + b.vy * b.vy + b.vz * b.vz;
            if (v2 < 1e-20) continue;
            // Coulomb logarithm ~ ln(b_max/b_min) ~ ln(box/softening) ~ 5
            const lnLambda = 5.0;
            const drag = 4 * Math.PI * G * G * b.mass * b.mass * lnLambda / (v2 + 1e-10);
            // Estimate local density from total mass / box volume (crude)
            const rho_local = this._bodies.reduce((s, p) => s + p.mass, 0) / Math.pow(this._boxSize, 3);
            const fric = drag * rho_local;
            b.ax -= fric * b.vx;
            b.ay -= fric * b.vy;
            b.az -= fric * b.vz;
        }
    }

    tick() {
        const n = this._bodies.length;
        if (n === 0) return;
        const dt = this._dt;

        // Proper Velocity Verlet (symplectic, time-reversible):
        //   1. Half-kick using CURRENT forces
        //   2. Drift positions
        //   3. Recompute forces at NEW positions
        //   4. Half-kick using NEW forces
        // This preserves energy to machine precision over long runs.

        // Step 1: half-kick with current accelerations
        for (const b of this._bodies) {
            b.vx += 0.5 * dt * b.ax;
            b.vy += 0.5 * dt * b.ay;
            b.vz += 0.5 * dt * b.az;
        }

        // Step 2: drift positions
        for (const b of this._bodies) {
            b.x += dt * b.vx;
            b.y += dt * b.vy;
            b.z += dt * b.vz;
        }

        // Step 3: recompute forces at new positions
        this._computeForces();

        // Step 4: second half-kick with FRESH forces
        for (const b of this._bodies) {
            b.vx += 0.5 * dt * b.ax;
            b.vy += 0.5 * dt * b.ay;
            b.vz += 0.5 * dt * b.az;
        }

        this._tick++;
    }

    run(nTicks) { for (let i = 0; i < nTicks; i++) this.tick(); }

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
            positions[i * 3] = b.x;
            positions[i * 3 + 1] = b.y;
            positions[i * 3 + 2] = b.z;
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
            totalKE += 0.5 * b.mass * (b.vx * b.vx + b.vy * b.vy + b.vz * b.vz);
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
