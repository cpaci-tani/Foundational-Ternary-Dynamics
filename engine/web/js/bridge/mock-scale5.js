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
 *
 * Refactor note (MS5-1..3): scenario data generation moved to
 * ./cosmic-scenarios/, the force kernel to ./cosmic-physics.js, and
 * post-integration events to ./cosmic-postupdates.js. The class below
 * owns state, the tick schedule, telemetry, and the public API.
 */

import { OMEGA_LAMBDA, OMEGA_MATTER, H0_LATTICE } from '../constants.js';

import { runCosmicScenario } from './cosmic-scenarios/index.js';
import { computeCosmicForces } from './cosmic-physics.js';
import { postCosmicUpdates } from './cosmic-postupdates.js';

// H0_LATTICE migrated to constants.js (Wave 2G, 2026-04-26).
// Note: this is still a static parameter; no Friedmann solver wires
// it into a(t) evolution — see Theme D Scale-5 follow-up for the
// integrator implementation.

export class CosmicMockBridge {
    constructor() {
        this._bodies = [];
        this._tick = 0;
        this._nextId = 0;
        this._dt = 0.01;
        this._a = 1.0;
        this._adot = 0.0;
        this._H0 = H0_LATTICE;
        this._boxSize = 200;
        this._softening = 5.0; // base softening (used as fallback)
        this._gwEvents = [];
        this._t_cosmic = 0.0;
        this._enableSubgrid = false;
        this._stellarEvolution = false;
        this._hawkingEvaporation = false;
    }

    static TYPE = {
        DARK_ENERGY: -3, QUASAR: -2, BLACK_HOLE: -1,
        DARK_MATTER: 0, GAS: 1, STAR: 2,
        NEUTRON_STAR: 3, NEBULA: 4, WHITE_DWARF: 5
    };

    addBody(type, mass, x, y, z, vx = 0, vy = 0, vz = 0, temp = 0) {
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
            tidal_stretch: 0, // 0 = normal, grows toward 1.0 as star is disrupted
            original_mass: mass, // for tracking how much has been shed
            fuel_fraction: type === 2 ? 1.0 : 0, // 1.0 = full fuel, 0.0 = exhausted
            fuel_stage: 0,       // 0=H, 1=He, 2=C, 3=O, 4=Si, 5=Fe (dead)
            budget_income: 0,    // fusion energy per tick (for budget overlay)
            budget_expense: 0,   // gravitational + radiation drain per tick
            age: 0,              // ticks since creation
        });
        return id;
    }

    // Plummer sphere enclosed mass: M(r) = M * r^3 / (r^2 + a^2)^(3/2)
    _enclosedMass(r, M_total, rs) {
        return M_total * r * r * r / Math.pow(r * r + rs * rs, 1.5);
    }

    // ================================================================
    // SCENARIOS — delegated to ./cosmic-scenarios/
    // ================================================================

    setupScenario(name) {
        this._bodies = [];
        this._nextId = 0;
        this._tick = 0;
        this._a = 1.0;
        this._gwEvents = [];
        this._t_cosmic = 0.0;
        this._scenarioName = name;
        this._customTelemetry = {};
        this._stellarEvolution = false;
        this._hawkingEvaporation = false;

        const rng = this._rng(42);
        const PI2 = Math.PI * 2;
        // Box-Muller for Gaussian random numbers (needed for z-dispersion)
        const randn = () => Math.sqrt(-2 * Math.log(rng() + 1e-10)) * Math.cos(PI2 * rng());

        const ctx = { T: CosmicMockBridge.TYPE, rng, randn, PI2 };
        runCosmicScenario.call(this, name, ctx);
    }

    // ================================================================
    // FORCE COMPUTATION — delegated to ./cosmic-physics.js
    // ================================================================

    _computeForces() {
        computeCosmicForces.call(this, CosmicMockBridge.TYPE);
    }

    // ================================================================
    // POST-INTEGRATION UPDATES — delegated to ./cosmic-postupdates.js
    // ================================================================

    _postUpdates() {
        postCosmicUpdates.call(this, CosmicMockBridge.TYPE);
    }

    // ================================================================
    // CUSTOM SCENARIO TELEMETRY
    // ================================================================

    _updateTelemetry() {
        const name = this._scenarioName;
        const T = CosmicMockBridge.TYPE;
        const isBH = (t) => t === T.BLACK_HOLE || t === T.QUASAR;
        const isStar = (t) => t === T.STAR || t === T.NEUTRON_STAR || t === T.WHITE_DWARF;

        const tel = {};

        if (name === 'cosmic-merger' || name === 'cosmic-binary-agn') {
            const bhs = this._bodies.filter(b => isBH(b.type)).sort((a, b) => b.mass - a.mass);
            if (bhs.length >= 2) {
                const dx = bhs[0].x - bhs[1].x, dy = bhs[0].y - bhs[1].y, dz = bhs[0].z - bhs[1].z;
                const sep = Math.sqrt(dx * dx + dy * dy + dz * dz);
                tel['Core Separation'] = sep.toFixed(2) + ' lu';
                if (name === 'cosmic-binary-agn') {
                    tel['Peak Jet Power'] = Math.max(bhs[0].luminosity || 0, bhs[1].luminosity || 0).toExponential(2) + ' EJ/s';
                }
            } else if (bhs.length === 1) {
                tel['Status'] = 'Merger Complete';
                tel['Singularity Mass'] = bhs[0].mass.toFixed(1) + ' M\u2299';
            }
        } else if (name === 'cosmic-cartwheel-collision') {
            const bhs = this._bodies.filter(b => isBH(b.type));
            if (bhs.length >= 2) {
                const dx = bhs[0].x - bhs[1].x, dy = bhs[0].y - bhs[1].y, dz = bhs[0].z - bhs[1].z;
                const sep = Math.sqrt(dx * dx + dy * dy + dz * dz);
                tel['Bullet Distance'] = sep.toFixed(1) + ' lu';
                tel['Impact Phase'] = (bhs[1].y > 0) ? 'Post-Collision' : 'Approach';
            }
        } else if (name === 'cosmic-globular-cluster') {
            let coreStars = 0;
            let M_core = 0;
            for (const b of this._bodies) {
                if (isStar(b.type)) {
                    if (b.x * b.x + b.y * b.y + b.z * b.z < 100) { // core radius squared
                        coreStars++;
                        M_core += b.mass;
                    }
                }
            }
            tel['Core Population (r<10)'] = coreStars;
            tel['Core Density'] = (M_core / (4 / 3 * Math.PI * 1000)).toExponential(2) + ' M\u2299/lu\u00B3';
        } else if (name === 'cosmic-stellar-lifecycle') {
            const wd = this._bodies.filter(b => b.type === T.WHITE_DWARF || b.type === T.NEUTRON_STAR).length;
            const bh = this._bodies.filter(b => b.type === T.BLACK_HOLE).length;
            tel['Deceased Stars'] = wd;
            tel['Supernova Remnants (BH)'] = bh;
        } else if (name === 'cosmic-black-hole') {
            const bh = this._bodies.find(b => isBH(b.type));
            if (bh) {
                tel['BH Mass'] = bh.mass.toFixed(2) + ' M\u2299';
                tel['Accretion Disk Lum'] = (bh.luminosity || 0).toExponential(2) + ' W';
            }
        } else if (name === 'cosmic-ftd-collapse') {
            const bh = this._bodies.find(b => isBH(b.type));
            if (bh) {
                tel['Status'] = 'Collapsed (Singularity Born)';
                tel['BH Mass'] = bh.mass.toFixed(1) + ' M\u2299';
            } else {
                tel['Status'] = 'Pre-Collapse (Increasing Density)';
            }
        } else if (name === 'cosmic-web') {
            const nodes = this._bodies.filter(b => isBH(b.type)).length;
            const gas = this._bodies.filter(b => b.type === T.NEBULA).length;
            tel['Stable Anchor Nodes'] = nodes;
            tel['Filament Gas Clumps'] = gas;
        }

        this._customTelemetry = tel;
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
        const stretches = new Float32Array(n);
        const ids = new Int32Array(n); // stable body IDs (survive index shifts)
        const fuel_stages = new Int8Array(n);
        const fuel_fractions = new Float32Array(n);

        for (let i = 0; i < n; i++) {
            const b = this._bodies[i];
            positions[i * 3] = b.x; positions[i * 3 + 1] = b.y; positions[i * 3 + 2] = b.z;
            types[i] = b.type;
            ids[i] = b.id;
            const stretch = b.tidal_stretch || 0;
            temperatures[i] = b.temperature + stretch * 15000;
            // Radius override if present, else fallback to mass-based
            sizes[i] = b.radius || (Math.cbrt(b.mass) * (1 + stretch * 2));
            densities[i] = b.density || 0.1;
            // Note: For BH, luminosity holds the Jet Intensity gauge!
            luminosities[i] = b.luminosity * (1 - stretch * 0.5);
            stretches[i] = stretch;
            fuel_stages[i] = b.fuel_stage || 0;
            fuel_fractions[i] = b.fuel_fraction != null ? b.fuel_fraction : 1.0;
        }
        return { positions, types, temperatures, sizes, densities, luminosities, stretches, ids, fuel_stages, fuel_fractions, count: n };
    }

    cosmicInspectBody(id) {
        const b = this._bodies.find(x => x.id === id);
        if (!b) return null;
        return {
            id: b.id, type: b.type, mass: b.mass,
            x: b.x, y: b.y, z: b.z, vx: b.vx, vy: b.vy, vz: b.vz,
            speed: Math.sqrt(b.vx * b.vx + b.vy * b.vy + b.vz * b.vz),
            temperature: b.temperature || 0,
            luminosity: b.luminosity || 0,
            fuel_fraction: b.fuel_fraction != null ? b.fuel_fraction : 1.0,
            fuel_stage: b.fuel_stage || 0,
            age: b.age || 0,
            radius: b.radius || (Math.cbrt(b.mass) * (1 + (b.tidal_stretch || 0) * 2))
        };
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
            omegaMatter: OMEGA_MATTER, omegaLambda: OMEGA_LAMBDA,
            customTelemetry: this._customTelemetry
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
