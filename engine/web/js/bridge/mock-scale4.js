/**
 * PlanetaryMockBridge — JS-only N-body simulation for Planetary scale (Scale 4).
 *
 * Architecture follows N-body integration:
 *   1. _computeForces():  pure Newton gravity
 *   2. tick():            Velocity Verlet integrator
 *
 * Unit system (positions/masses are heliocentric in both gravity modes):
 * - Distance: Astronomical Units (AU)
 * - Mass: Solar Masses (M_sun)
 * - Time: Earth Years (yr)
 * - Velocity: AU / yr
 *
 * Gravitational-constant mode (P0-1 audit, 2026-05-27 — implemented as a
 * user-facing toggle rather than a one-way switch, to preserve the prior
 * slow/decorative cadence as the default UX):
 * - 'decorative' (DEFAULT): G = G_N = 0.01 (FTD lattice-natural constant).
 *   Orbits crawl; Earth's "year" runs ~63× slow. NOT quantitatively faithful
 *   to AU/M_sun/yr Kepler timing — purely a calm visual.
 * - 'physical': G = G_HELIOCENTRIC = 4π² ≈ 39.478 (Kepler's 3rd law). Earth's
 *   circular orbit at a=1 AU has the correct period T = 1 yr; orbits are
 *   ~63× faster than decorative.
 * The figure-8 three-body scenario always overrides to G=1 regardless of mode
 * (Chenciner–Montgomery natural units).
 */

import { EXOPLANET_SEEDS } from '../config/exoplanet-seeds.js';
import { G_HELIOCENTRIC, G_N } from '../constants.js';

// Gravity-constant presets for the Scale-4 toggle. Both are named exports of
// constants.js; do not inline the numbers.
const G_BY_MODE = {
    decorative: G_N,            // 0.01 — FTD lattice-natural, slow visual default
    physical:   G_HELIOCENTRIC, // 4π²  — Keplerian AU/M_sun/yr, period-faithful
};
const DEFAULT_GRAVITY_MODE = 'decorative';

export class PlanetaryMockBridge {
    constructor() {
        this._bodies = [];
        this._tick = 0;
        this._nextId = 0;
        this._dt = 0.0001; // Exact, tiny step to preserve Verlet integration on dense TRAPPIST arrays

        // Gravity mode is user-selectable (toolbar). Default 'decorative' keeps
        // the historical slow cadence so existing UX is unchanged until the
        // user opts into 'physical'. setGravityMode() updates this.G.
        this._gravityMode = DEFAULT_GRAVITY_MODE;
        this.G = G_BY_MODE[this._gravityMode];
    }

    /**
     * Select the gravitational-constant mode. Takes effect on the next
     * setupScenario() (callers reload the scenario after switching).
     * @param {'decorative'|'physical'} mode
     */
    setGravityMode(mode) {
        if (!(mode in G_BY_MODE)) return;
        this._gravityMode = mode;
        this.G = G_BY_MODE[mode];
    }

    getGravityMode() {
        return this._gravityMode;
    }

    static TYPE = {
        STAR: 0,
        ROCKY_PLANET: 1,
        GAS_GIANT: 2,
        MOON: 3,
        ASTEROID: 4
    };

    addBody(type, mass, r, x, y, z, vx=0, vy=0, vz=0, seed=0) {
        const id = this._nextId++;
        this._bodies.push({
            id, type, mass, r,
            x, y, z, vx, vy, vz,
            ax: 0, ay: 0, az: 0,
            seed: seed || (Math.random() * 10000)
        });
        return id;
    }

    // ================================================================
    // SCENARIOS
    // ================================================================

    setupScenario(name) {
        this._bodies = [];
        this._nextId = 0;
        this._tick = 0;
        this._scenarioName = name;
        // Apply the currently-selected gravity mode (figure-8 scenario below
        // overrides to G=1 for its natural units, regardless of mode).
        this.G = G_BY_MODE[this._gravityMode];

        const T = PlanetaryMockBridge.TYPE;
        const TAU = Math.PI * 2;

        if (name === 'planetary-solar') {
            // Sun
            this.addBody(T.STAR, 1.0, 1.0, 0, 0, 0, 0, 0, 0);

            // Inner planets (approx circular)
            // Mercury
            this.addBody(T.ROCKY_PLANET, 1.66e-7, 0.003, 0.387, 0, 0, 0, Math.sqrt(this.G / 0.387), 0);
            // Venus
            this.addBody(T.ROCKY_PLANET, 2.45e-6, 0.008, 0.723, 0, 0, 0, Math.sqrt(this.G / 0.723), 0);
            // Earth
            this.addBody(T.ROCKY_PLANET, 3.00e-6, 0.009, 1.0, 0, 0, 0, Math.sqrt(this.G / 1.0), 0);
            // Mars
            this.addBody(T.ROCKY_PLANET, 3.23e-7, 0.004, 1.524, 0, 0, 0, Math.sqrt(this.G / 1.524), 0);
            // Jupiter
            this.addBody(T.GAS_GIANT, 9.55e-4, 0.1, 5.204, 0, 0, 0, Math.sqrt(this.G / 5.204), 0);
        } else if (name === 'planetary-binary') {
            // Twin suns
            const a = 2.0; 
            const M = 1.0;
            // v = sqrt(G*M/(4a))
            const v = Math.sqrt(this.G * M / (4 * a));
            this.addBody(T.STAR, M, 1.0, a, 0, 0, 0, v, 0);
            this.addBody(T.STAR, M, 1.0, -a, 0, 0, 0, -v, 0);

            // Circumbinary planet
            const rp = 6.0;
            const vp = Math.sqrt(this.G * (2 * M) / rp);
            this.addBody(T.GAS_GIANT, 3.00e-6, 0.05, rp, 0, 0, 0, vp, 0);
        } else if (name === 'planetary-threebody') {
            // Figure-8 (Chenciner-Montgomery 2000). Requires G=1, so temporarily override.
            // The G=1.0 below is the intentional figure-8 unit convention
            // (Chenciner–Montgomery natural units) and is NOT a physics value.
            const savedG = this.G;
            this.G = 1.0;
            this.addBody(T.STAR, 1.0, 0.5, 0.97000436, -0.24308753, 0, 0.466203685, 0.43236573, 0);
            this.addBody(T.STAR, 1.0, 0.5, -0.97000436, 0.24308753, 0, 0.466203685, 0.43236573, 0);
            this.addBody(T.STAR, 1.0, 0.5, 0, 0, 0, -2 * 0.466203685, -2 * 0.43236573, 0);
            this._threebody_G = 1.0; // keep G=1 for this scenario
        } else if (name.startsWith('exo-') && EXOPLANET_SEEDS) {
            const host = name.substring(4);
            const system = EXOPLANET_SEEDS[host];
            if (system && system.length > 0) {
                // Add star (assumed stationary roughly)
                const M_star = system[0].st_mass || 1.0;
                const R_star = system[0].st_rad || 1.0;
                this.addBody(T.STAR, M_star, R_star * 0.1, 0, 0, 0, 0, 0, 0); // Scale down R visually

                // Add planets
                for (let i = 0; i < system.length; i++) {
                    const p = system[i];
                    const M_p = p.mass_sol;
                    const a = p.pl_orbsmax;
                    const e = p.pl_orbeccen;
                    const r = a * (1 - e);
                    // vis-viva at perihelion: v = sqrt(G*M*(1+e)/(a*(1-e)))
                    const v = Math.sqrt(this.G * M_star * (1 + e) / (a * (1 - e)));
                    
                    const r_vis = p.pl_rade ? p.pl_rade * 0.05 : 0.05;
                    
                    // Simple hash from string for deterministic traits
                    let nameHash = 0;
                    if (p.pl_name) {
                        for(let c=0; c<p.pl_name.length; c++) nameHash += p.pl_name.charCodeAt(c) * (c+1);
                    }
                    
                    // Prevent catastrophic gravitational syzygy by distributing starting phase deterministicly
                    const theta = (nameHash % 360) * (Math.PI / 180);
                    const start_x = r * Math.cos(theta);
                    const start_y = r * Math.sin(theta);
                    const start_vx = -v * Math.sin(theta);
                    const start_vy = v * Math.cos(theta);

                    this.addBody(T.ROCKY_PLANET, M_p, r_vis, start_x, start_y, 0, start_vx, start_vy, 0, nameHash);
                }
            } else {
                this.addBody(T.STAR, 1.0, 1.0, 0, 0, 0, 0, 0, 0);
            }
        } else {
            // Fallback empty
            this.addBody(T.STAR, 1.0, 1.0, 0, 0, 0, 0, 0, 0);
        }
        
        this._computeForces();
    }

    _computeForces() {
        const N = this._bodies.length;
        const eps2 = 1e-6; // very small softening to avoid singularity division but allow collisions

        // Reset
        for (let i = 0; i < N; i++) {
            const bi = this._bodies[i];
            bi.ax = bi.ay = bi.az = 0;
        }

        // Compute direct gravity (O(N^2))
        for (let i = 0; i < N; i++) {
            const bi = this._bodies[i];
            for (let j = i + 1; j < N; j++) {
                const bj = this._bodies[j];
                const dx = bj.x - bi.x;
                const dy = bj.y - bi.y;
                const dz = bj.z - bi.z;
                
                const r2 = dx*dx + dy*dy + dz*dz + eps2;
                const r_inv = 1.0 / Math.sqrt(r2);
                const r3_inv = r_inv * r_inv * r_inv;
                
                const f_mag = this.G * r3_inv;
                
                const fx = f_mag * dx;
                const fy = f_mag * dy;
                const fz = f_mag * dz;
                
                bi.ax += fx * bj.mass;
                bi.ay += fy * bj.mass;
                bi.az += fz * bj.mass;
                
                bj.ax -= fx * bi.mass;
                bj.ay -= fy * bi.mass;
                bj.az -= fz * bi.mass;
            }
        }
    }

    run(ticks) {
        // Perform multiple substeps to maintain graphical visual speed without breaking mechanical stability limits
        const SUBSTEPS = 100; 
        for (let k = 0; k < ticks * SUBSTEPS; k++) {
            this.tick();
        }
    }

    tick() {
        const N = this._bodies.length;
        const dt = this._dt;
        const dt2 = 0.5 * dt;

        // V-Verlet kick 1 & drift
        for (let i = 0; i < N; i++) {
            const b = this._bodies[i];
            b.vx += b.ax * dt2;
            b.vy += b.ay * dt2;
            b.vz += b.az * dt2;

            b.x += b.vx * dt;
            b.y += b.vy * dt;
            b.z += b.vz * dt;
        }

        // Recompute forces at new positions
        this._computeForces();

        // V-Verlet kick 2
        for (let i = 0; i < N; i++) {
            const b = this._bodies[i];
            b.vx += b.ax * dt2;
            b.vy += b.ay * dt2;
            b.vz += b.az * dt2;
        }

        this._tick++;
    }

    getPlanetaryData() {
        // Just return a shallow clone array or flat array for renderer
        const flatBuffer = new Float32Array(this._bodies.length * 16);
        for(let i=0; i<this._bodies.length; i++) {
            const b = this._bodies[i];
            const off = i*16;
            flatBuffer[off+0] = b.x;
            flatBuffer[off+1] = b.y;
            flatBuffer[off+2] = b.z;
            flatBuffer[off+3] = b.type;
            flatBuffer[off+4] = b.mass;
            flatBuffer[off+5] = b.r;
            flatBuffer[off+6] = b.id;
            flatBuffer[off+7] = b.seed; // inject generated seed to fragment shaders map
        }
        return { count: this._bodies.length, buffer: flatBuffer };
    }

    getDiagnostics() {
        return {
            tick: this._tick,
            bodyCount: this._bodies.length,
            timeYears: this._tick * this._dt
        };
    }
}
