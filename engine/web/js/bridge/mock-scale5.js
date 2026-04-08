/**
 * CosmicMockBridge — JS-only N-body simulation for cosmic scale (Scale 5).
 *
 * Unit system (simulation units):
 *   G = G_N = 0.01 (from FTD ontic chain)
 *   Masses chosen so v_circular = sqrt(G*M/r) ~ O(1) for visual dynamics
 *   Positions span ~100 units, velocities ~ 0.5-2 units/tick
 *   dt chosen so displacement ~ 0.1-0.5 units/frame (3 ticks/frame)
 *
 * This is NOT in physical CGS/SI — it's a dimensionless system tuned for
 * visual dynamics while preserving correct gravitational scaling.
 */

import { G_N, OMEGA_LAMBDA, OMEGA_MATTER } from '../constants.js';

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
        this._softening = 1.0;
        this._gwEvents = [];
        this._t_cosmic = 0.0;
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

    // Plummer enclosed mass: M_enc = M * r^3 / (r^2 + a^2)^(3/2)
    _enclosedMass(r, M_total, rs) {
        return M_total * r * r * r / Math.pow(r * r + rs * rs, 1.5);
    }

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

        // ================================================================
        // All scenarios use natural units where G=0.01:
        //   v_c = sqrt(G*M/r) ~ sqrt(0.01 * M_total / r_scale) ~ 1
        //   => M_total / r_scale ~ 100
        // ================================================================

        if (name === 'cosmic-galaxy') {
            // Spiral galaxy: M_total=5000, r_disk=50 => v_c ~ sqrt(0.01*5000/50) = 1.0
            // BH = 2% of total (100). Each DM particle ~ 14, so BH is 7x a particle.
            // BH doesn't receive N-body kicks (pinned by code), so even 7x is enough
            // to be the dominant attractor in the inner region.
            const M_total = 5000;
            const M_bh = 100;
            const M_dm = (M_total - M_bh) * 0.85;
            const M_disk = (M_total - M_bh) * 0.15;
            const r_s = 30;      // Scale radius
            const r_disk = 50;   // Disk extent

            // Central black hole
            this.addBody(T.BLACK_HOLE, M_bh, 0, 0, 0);

            // DM halo (spherical, Hernquist profile)
            for (let i = 0; i < 300; i++) {
                const u = rng() * 0.95;
                const su = Math.sqrt(u);
                const r = Math.min(r_s * su / (1.0 - su), 120);
                const th = Math.acos(2 * rng() - 1);
                const ph = PI2 * rng();
                const x = r * Math.sin(th) * Math.cos(ph);
                const y = r * Math.sin(th) * Math.sin(ph);
                const z = r * Math.cos(th);

                const M_enc = this._enclosedMass(r, M_total, r_s);
                const sigma = Math.sqrt(G_N * M_enc / Math.max(r, 2)) * 0.35;
                this.addBody(T.DARK_MATTER, M_dm / 300, x, y, z,
                    sigma * (rng() - 0.5) * 2,
                    sigma * (rng() - 0.5) * 2,
                    sigma * (rng() - 0.5) * 2);
            }

            // Stellar disk with spiral arms
            for (let i = 0; i < 350; i++) {
                const r = 2 + rng() * r_disk;
                const arm = Math.floor(rng() * 2);
                const phi_base = arm * Math.PI + 0.4 * Math.log(r + 1);
                const ph = phi_base + (rng() - 0.5) * 0.7;
                const zz = (rng() - 0.5) * 1.5;

                const M_enc = M_bh + this._enclosedMass(r, M_dm, r_s);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 2));

                this.addBody(T.STAR, M_disk * 0.6 / 350,
                    r * Math.cos(ph), zz, r * Math.sin(ph),
                    -vc * Math.sin(ph), 0, vc * Math.cos(ph),
                    3000 + rng() * 25000);
            }

            // Gas disk
            for (let i = 0; i < 150; i++) {
                const r = 3 + rng() * r_disk * 1.3;
                const arm = Math.floor(rng() * 2);
                const phi_base = arm * Math.PI + 0.4 * Math.log(r + 1);
                const ph = phi_base + (rng() - 0.5) * 1.0;
                const zz = (rng() - 0.5) * 1.0;

                const M_enc = M_bh + this._enclosedMass(r, M_dm, r_s);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 2));

                this.addBody(T.GAS, M_disk * 0.4 / 150,
                    r * Math.cos(ph), zz, r * Math.sin(ph),
                    -vc * Math.sin(ph), 0, vc * Math.cos(ph),
                    5000 + rng() * 15000);
            }

            this._boxSize = 200;
            this._softening = 2.0;
            this._dt = 0.05;

        } else if (name === 'cosmic-black-hole') {
            // BH accretion: M_bh=500, disk at r=5..50 => v_k = sqrt(0.01*500/10) ~ 0.7
            const M_bh = 500;
            this.addBody(T.BLACK_HOLE, M_bh, 0, 0, 0);

            for (let i = 0; i < 500; i++) {
                const u = rng();
                const r = 4 + u * u * 46; // r in [4, 50], concentrated near center
                const ph = PI2 * rng();
                const zz = (rng() - 0.5) * 0.5 * (r / 10); // thinner near center

                const vk = Math.sqrt(G_N * M_bh / r);
                const v_factor = 0.98 - 0.02 * rng(); // sub-Keplerian for viscous inflow

                this.addBody(T.GAS, 0.2,
                    r * Math.cos(ph), zz, r * Math.sin(ph),
                    -vk * v_factor * Math.sin(ph), 0, vk * v_factor * Math.cos(ph),
                    1e6 * Math.pow(4 / r, 0.75)); // T ~ r^(-3/4)
            }

            this._boxSize = 120;
            this._softening = 0.8;
            this._dt = 0.03;

        } else if (name === 'cosmic-merger') {
            // Two galaxies: M1=3000, M2=2000, sep=80
            // v_esc = sqrt(2*G*(M1+M2)/sep) = sqrt(2*0.01*5000/80) = 1.12
            // v_approach = 0.5 * v_esc = 0.56
            const M1 = 3000, M2 = 2000;
            const sep = 80;
            const M_total = M1 + M2;
            const v_esc = Math.sqrt(2 * G_N * M_total / sep);
            const v_approach = v_esc * 0.45;
            const b = 10; // impact parameter

            const r_s1 = 20, r_s2 = 16;

            // Galaxy 1
            const cx1 = -sep / 2, cz1 = -b / 2;
            this.addBody(T.BLACK_HOLE, M1 * 0.02, cx1, 0, cz1, v_approach, 0, v_approach * 0.15);
            for (let i = 0; i < 250; i++) {
                const r = rng() * r_s1 * 1.8;
                const ph = PI2 * rng();
                const zz = (rng() - 0.5) * 1.5;
                const t = i < 125 ? T.DARK_MATTER : T.STAR;

                const M_enc = this._enclosedMass(r, M1, r_s1);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 1));

                const M1_remaining = M1 * 0.98; // after BH takes 2%
                this.addBody(t, (t === T.DARK_MATTER ? M1_remaining * 0.85 : M1_remaining * 0.15) / 125,
                    cx1 + r * Math.cos(ph), zz, cz1 + r * Math.sin(ph),
                    v_approach - vc * Math.sin(ph), 0, v_approach * 0.15 + vc * Math.cos(ph),
                    t === T.STAR ? 4000 + rng() * 18000 : 0);
            }

            // Galaxy 2
            const cx2 = sep / 2, cz2 = b / 2;
            this.addBody(T.BLACK_HOLE, M2 * 0.02, cx2, 0, cz2, -v_approach, 0, -v_approach * 0.15);
            for (let i = 0; i < 200; i++) {
                const r = rng() * r_s2 * 1.8;
                const ph = PI2 * rng();
                const zz = (rng() - 0.5) * 1.5;
                const t = i < 100 ? T.DARK_MATTER : T.STAR;

                const M_enc = this._enclosedMass(r, M2, r_s2);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 1));

                const M2_remaining = M2 * 0.98;
                this.addBody(t, (t === T.DARK_MATTER ? M2_remaining * 0.85 : M2_remaining * 0.15) / 100,
                    cx2 + r * Math.cos(ph), zz, cz2 + r * Math.sin(ph),
                    -v_approach - vc * Math.sin(ph), 0, -v_approach * 0.15 + vc * Math.cos(ph),
                    t === T.STAR ? 4000 + rng() * 18000 : 0);
            }

            this._boxSize = 250;
            this._softening = 1.5;
            this._dt = 0.04;

        } else {
            // Cosmic web: DM particles with Zel'dovich perturbations
            // M_per_particle=5, box=200 => total M = 3500+500 = 4000
            // Collapse timescale ~ 1/sqrt(G*rho) ~ 1/sqrt(0.01*4000/200^3) ~ 70
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
                const x = (rng() - 0.5) * 200;
                const y = (rng() - 0.5) * 200;
                const z = (rng() - 0.5) * 200;
                this.addBody(T.GAS, 5, x, y, z, 0, 0, 0, 1e4);
            }
            this._boxSize = 200;
            this._softening = 3.0;
            this._dt = 0.08;
        }
    }

    // Compute gravitational accelerations (O(N^2) with Plummer softening)
    _computeForces() {
        const G = G_N;
        const n = this._bodies.length;
        const soft2 = this._softening * this._softening;

        for (const b of this._bodies) { b.ax = 0; b.ay = 0; b.az = 0; }

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

        // ============================================================
        // Body-type-specific physics (beyond universal gravity)
        //
        // Interaction matrix:
        //   DM, Stars, WD, NS: collisionless — gravity only
        //   Gas, Nebula:        collisional — gravity + pressure + cooling + viscosity
        //   BH, Quasar:         accretors — eat gas, merge with other BHs
        //   Star → Gas:         radiation pressure heats/pushes nearby gas
        //   Dense Gas → Star:   Jeans-like star formation
        // ============================================================

        const T = CosmicMockBridge.TYPE;
        const isGas = (t) => t === T.GAS || t === T.NEBULA;
        const isBH  = (t) => t === T.BLACK_HOLE || t === T.QUASAR;
        const isStar = (t) => t === T.STAR || t === T.NEUTRON_STAR || t === T.WHITE_DWARF;

        // ── 1. Gas radiative cooling (density-dependent) ──
        // Cooling rate scales as n² (bremsstrahlung): denser gas loses energy faster.
        // This drives gas to sink toward center and form accretion disks/structures.
        // DM and stars are collisionless — they cannot radiate.
        for (const b of this._bodies) {
            if (!isGas(b.type)) continue;
            // Estimate local density from number of gas neighbors within smoothing
            let localDensity = b.mass; // self
            for (const other of this._bodies) {
                if (other.id === b.id || !isGas(other.type)) continue;
                const dr2 = (b.x-other.x)**2 + (b.y-other.y)**2 + (b.z-other.z)**2;
                if (dr2 < soft2 * 25) localDensity += other.mass;
            }
            // Cooling ~ rho^2 * Lambda(T), capped to prevent overcooling
            // Gentle: ~0.02% per tick for isolated gas, up to 0.2% in dense regions
            const coolingRate = Math.min(0.002, 0.00005 * localDensity);
            b.ax -= coolingRate * b.vx;
            b.ay -= coolingRate * b.vy;
            b.az -= coolingRate * b.vz;
            // Temperature decreases with cooling (gas cools as it radiates)
            b.temperature = Math.max(100, b.temperature * (1 - coolingRate * 0.1));
        }

        // ── 2. Gas pressure (SPH-like repulsion) ──
        // Prevents gas from collapsing to a point. Stars/DM pass through freely.
        const h_press = this._softening * 2.5;
        const h_press2 = h_press * h_press;
        for (let i = 0; i < n; i++) {
            const bi = this._bodies[i];
            if (!isGas(bi.type)) continue;
            for (let j = i + 1; j < n; j++) {
                const bj = this._bodies[j];
                if (!isGas(bj.type)) continue;
                const dx = bj.x - bi.x;
                const dy = bj.y - bi.y;
                const dz = bj.z - bi.z;
                const r2 = dx * dx + dy * dy + dz * dz;
                if (r2 > h_press2 || r2 < 1e-10) continue;
                const r = Math.sqrt(r2);
                const q = r / h_press;
                // Pressure force: repulsive, scales with (1-q)^2
                // Stronger for hotter gas (temperature-dependent)
                const T_avg = 0.5 * (bi.temperature + bj.temperature);
                const pressScale = 1.0 + T_avg * 1e-6; // hotter gas pushes harder
                const fmag = G * pressScale * 0.3 * (bi.mass + bj.mass) * (1 - q) * (1 - q) / (r2 + soft2);
                bi.ax -= fmag * dx / r; bi.ay -= fmag * dy / r; bi.az -= fmag * dz / r;
                bj.ax += fmag * dx / r; bj.ay += fmag * dy / r; bj.az += fmag * dz / r;
                // Compression heats gas (PdV work)
                const vij_dot_rij = (bi.vx-bj.vx)*dx + (bi.vy-bj.vy)*dy + (bi.vz-bj.vz)*dz;
                if (vij_dot_rij < 0) { // approaching → compressive heating
                    const heating = -vij_dot_rij * 0.001 * (1 - q);
                    bi.temperature += heating;
                    bj.temperature += heating;
                }
            }
        }

        // ── 3. Star → Gas radiation pressure ──
        // Luminous stars heat and push nearby gas (stellar feedback).
        // Prevents runaway gas collapse, creates feedback-regulated star formation.
        for (const star of this._bodies) {
            if (!isStar(star.type) || star.luminosity <= 0) continue;
            for (const gas of this._bodies) {
                if (!isGas(gas.type)) continue;
                const dx = gas.x - star.x;
                const dy = gas.y - star.y;
                const dz = gas.z - star.z;
                const r2 = dx * dx + dy * dy + dz * dz + soft2;
                if (r2 > 400) continue; // only within 20 units
                const r = Math.sqrt(r2);
                // Radiation pressure: F = L / (4*pi*r^2*c)
                const c_sim = 0.577; // C_SPEED in sim units
                const f_rad = star.luminosity / (4 * Math.PI * r2 * c_sim) * 0.01; // scaled down
                gas.ax += f_rad * dx / r;
                gas.ay += f_rad * dy / r;
                gas.az += f_rad * dz / r;
                // Heating from radiation
                gas.temperature += star.luminosity * 0.0001 / (r2 + 1);
            }
        }

        // ── 4. Dense gas → star formation (Jeans criterion) ──
        // When gas density exceeds threshold and temperature is low, convert to star.
        const newStars = [];
        for (const b of this._bodies) {
            if (!isGas(b.type) || b.mass < 0.5) continue;
            // Count nearby gas (density proxy)
            let nearby = 0;
            for (const other of this._bodies) {
                if (other.id === b.id || !isGas(other.type)) continue;
                const dr2 = (b.x-other.x)**2 + (b.y-other.y)**2 + (b.z-other.z)**2;
                if (dr2 < soft2 * 9) nearby++;
            }
            // Jeans: form star when dense (>5 neighbors) and cool (<5000 K)
            if (nearby > 5 && b.temperature < 5000) {
                const starMass = b.mass * 0.3;
                b.mass -= starMass;
                newStars.push({
                    type: T.STAR, mass: starMass,
                    x: b.x, y: b.y, z: b.z,
                    vx: b.vx, vy: b.vy, vz: b.vz,
                    temperature: 5800, // newborn star
                    luminosity: Math.pow(starMass, 3.5)
                });
            }
        }
        for (const s of newStars) {
            this.addBody(s.type, s.mass, s.x, s.y, s.z, s.vx, s.vy, s.vz, s.temperature);
            // Set luminosity on the newly added body
            this._bodies[this._bodies.length - 1].luminosity = s.luminosity;
        }

        // ── 5. BH/Quasar accretion (gas → BH mass transfer) ──
        // Only gas that is gravitationally bound AND slow (relative to BH)
        // gets accreted. Fast-moving gas in a flyby escapes.
        // Rate is Bondi-like: mdot ~ M^2 / (cs^2 + v_rel^2)^(3/2)
        for (const bh of this._bodies) {
            if (!isBH(bh.type)) continue;
            const r_acc = Math.max(1.5, Math.cbrt(bh.mass) * 0.3);
            const r_acc2 = r_acc * r_acc;
            for (const gas of this._bodies) {
                if (!isGas(gas.type) || gas.mass <= 0) continue;
                const dx = gas.x - bh.x;
                const dy = gas.y - bh.y;
                const dz = gas.z - bh.z;
                const r2 = dx * dx + dy * dy + dz * dz;
                if (r2 > r_acc2) continue;
                // Relative velocity determines if gas is bound
                const dvx = gas.vx - bh.vx, dvy = gas.vy - bh.vy, dvz = gas.vz - bh.vz;
                const v_rel2 = dvx*dvx + dvy*dvy + dvz*dvz;
                const r = Math.sqrt(r2 + 0.01);
                const v_esc2 = 2 * G * bh.mass / r; // escape velocity squared
                // Only accrete if relative velocity < escape velocity (bound gas)
                if (v_rel2 > v_esc2) continue;
                // Bondi-like rate: slower gas accretes faster
                const rate = 0.005 * bh.mass / (v_rel2 + 0.1);
                const dm = Math.min(gas.mass * 0.1, gas.mass * rate * 0.001);
                bh.mass += dm;
                gas.mass -= dm;
            }
        }

        // ── 6. BH-BH / compact object merger ──
        for (let i = 0; i < this._bodies.length; i++) {
            const bi = this._bodies[i];
            if (!isBH(bi.type)) continue;
            if (bi.mass <= 0) continue;
            for (let j = i + 1; j < this._bodies.length; j++) {
                const bj = this._bodies[j];
                if (!isBH(bj.type) || bj.mass <= 0) continue;
                const dx = bj.x - bi.x;
                const dy = bj.y - bi.y;
                const dz = bj.z - bi.z;
                const r2 = dx * dx + dy * dy + dz * dz;
                const r_merge = Math.cbrt(bi.mass + bj.mass) * 0.3;
                if (r2 > r_merge * r_merge) continue;
                const m_total = bi.mass + bj.mass;
                bi.vx = (bi.vx * bi.mass + bj.vx * bj.mass) / m_total;
                bi.vy = (bi.vy * bi.mass + bj.vy * bj.mass) / m_total;
                bi.vz = (bi.vz * bi.mass + bj.vz * bj.mass) / m_total;
                bi.mass = m_total * 0.95; // 5% GW radiation
                bj.mass = 0;
            }
        }

        // ── 7. Speed limit: v < c = 1/sqrt(3) ──
        const c2 = 1.0 / 3.0; // C_SPEED^2
        for (const b of this._bodies) {
            const v2 = b.vx * b.vx + b.vy * b.vy + b.vz * b.vz;
            if (v2 > c2) {
                const scale = Math.sqrt(c2 / v2);
                b.vx *= scale; b.vy *= scale; b.vz *= scale;
            }
        }

        // ── Cleanup: remove depleted bodies ──
        this._bodies = this._bodies.filter(b => b.mass > 0.01);
    }

    tick() {
        const n = this._bodies.length;
        if (n === 0) return;
        const dt = this._dt;

        // Velocity Verlet (symplectic): kick-drift-recompute-kick
        // All bodies (including BHs) integrated uniformly.
        for (const b of this._bodies) {
            b.vx += 0.5 * dt * b.ax;
            b.vy += 0.5 * dt * b.ay;
            b.vz += 0.5 * dt * b.az;
        }
        for (const b of this._bodies) {
            b.x += dt * b.vx;
            b.y += dt * b.vy;
            b.z += dt * b.vz;
        }
        this._computeForces();
        for (const b of this._bodies) {
            b.vx += 0.5 * dt * b.ax;
            b.vy += 0.5 * dt * b.ay;
            b.vz += 0.5 * dt * b.az;
        }

        this._t_cosmic += dt;
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
