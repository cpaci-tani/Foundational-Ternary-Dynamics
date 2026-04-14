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
// At N~800, softening must be large to suppress two-body relaxation.
// Mean inter-particle spacing ~ (200^3/800)^(1/3) ~ 13.5 units.
// Softening should be ~50% of this (~6-7) for stability.
// BH gets smaller softening so it can anchor the core via close-range gravity.
const SOFTENING = {
    [-3]: 6.0,  // DARK_ENERGY
    [-2]: 3.0,  // QUASAR
    [-1]: 1.5,  // BLACK_HOLE — very tight to anchor core correctly
    [0]:  8.0,  // DARK_MATTER — keeps halo from collapsing
    [1]:  3.0,  // GAS
    [2]:  2.5,  // STAR — tight enough to allow valid keplerian math
    [3]:  2.0,  // NEUTRON_STAR
    [4]:  3.0,  // NEBULA
    [5]:  2.0,  // WHITE_DWARF
};
const SOFTENING_SQ = {
    [-3]: 36.0, [-2]: 9.0, [-1]: 2.25,
    [0]: 64.0, [1]: 9.0, [2]: 6.25,
    [3]: 4.0, [4]: 9.0, [5]: 4.0
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
        this._stellarEvolution = false;
        this._hawkingEvaporation = false;
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

    // Plummer sphere enclosed mass: M(r) = M * r³ / (r² + a²)^(3/2)
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
        this._scenarioName = name;
        this._customTelemetry = {};

        const T = CosmicMockBridge.TYPE;
        const rng = this._rng(42);
        const PI2 = Math.PI * 2;
        // Box-Muller for Gaussian random numbers (needed for z-dispersion)
        const randn = () => Math.sqrt(-2 * Math.log(rng() + 1e-10)) * Math.cos(PI2 * rng());

        if (name === 'cosmic-galaxy') {
            const M_total = 7000;
            const M_bh = 100;
            const M_dm = (M_total - M_bh) * 0.85;
            const M_disk = (M_total - M_bh) * 0.15;
            const r_s = 40;       // Scale radius
            const r_disk = 90;    // Increased disk extent

            this.addBody(T.BLACK_HOLE, M_bh, 0, 0, 0);

            // DM halo (Plummer profile — sampling uses Hernquist CDF)
            const N_dm = 1000;
            for (let i = 0; i < N_dm; i++) {
                const u = rng() * 0.95;
                const su = Math.sqrt(u);
                const r = Math.min(r_s * su / (1.0 - su), 150);
                const th = Math.acos(2 * rng() - 1);
                const ph = PI2 * rng();

                const M_enc = this._enclosedMass(r, M_total, r_s);
                const sigma = Math.sqrt(G_N * M_enc / Math.max(r, 3)) * 0.7;
                this.addBody(T.DARK_MATTER, M_dm / N_dm,
                    r * Math.sin(th) * Math.cos(ph),
                    r * Math.sin(th) * Math.sin(ph),
                    r * Math.cos(th),
                    sigma * randn(), sigma * randn(), sigma * randn());
            }

            // Stellar disk — sharp, 6-arm spiral structure
            const N_stars = 700; // Reduced from 2500 for massive performance boost
            for (let i = 0; i < N_stars; i++) {
                const u = rng();
                const r = 5 + (1 - Math.pow(1 - u, 2)) * r_disk;
                
                // Structural arms: base angle + logarithmic winding + tightness scatter
                const armOffset = (i % 6) * (Math.PI / 3); // 6 distinct arms
                const winding = -0.5 * Math.log(r + 1); // Negative winding for trailing arms
                const dispersion = randn() * 0.35; // tighter spiral definition
                const ph = armOffset + winding + dispersion;
                
                // Flared thick disk, but much flatter than before
                const z_scale = 1.0 + r * 0.02;
                const zz = randn() * z_scale;

                // Keplerian velocity matched to exact enclosed mass calculation
                const M_enc = M_bh + this._enclosedMass(r, M_dm, r_s) + this._enclosedMass(r, M_disk, r_disk * 0.4);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 0.5)); 
                const vz = randn() * vc * 0.08; // cooler disk

                this.addBody(T.STAR, M_disk * 0.6 / N_stars,
                    r * Math.cos(ph), zz, r * Math.sin(ph),
                    -vc * Math.sin(ph), vz, vc * Math.cos(ph), 
                    3000 + rng() * 25000);
            }

            // Gas — traces the spiral arms to form nebulae
            const N_gas = 250; // Reduced from 800
            for (let i = 0; i < N_gas; i++) {
                const u = rng();
                const r = 8 + (1 - Math.pow(1 - u, 2)) * r_disk * 1.1;
                const armOffset = (i % 6) * (Math.PI / 3);
                const winding = -0.5 * Math.log(r + 1);
                const dispersion = randn() * 0.25; // tracks arms closer than stars
                const ph = armOffset + winding + dispersion;
                const z_scale = 0.8 + r * 0.015;
                const zz = randn() * z_scale;

                const M_enc = M_bh + this._enclosedMass(r, M_dm, r_s) + this._enclosedMass(r, M_disk, r_disk * 0.4);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 0.5));

                this.addBody(T.GAS, M_disk * 0.4 / N_gas,
                    r * Math.cos(ph), zz, r * Math.sin(ph),
                    -vc * Math.sin(ph), randn() * vc * 0.03, vc * Math.cos(ph),
                    5000 + rng() * 15000);
            }
            
            // Giant Stellar Dust Clouds (Nebulae) formed into elongated spiral arms
            const N_dust = 600; // Increased count for dense lanes
            for (let i = 0; i < N_dust; i++) {
                // Skew distribution towards the inner disk where density is higher
                const u = rng();
                const r = 3 + (1 - Math.pow(1 - u, 1.5)) * r_disk * 0.9;
                
                // Align with the 6 spiral arms, but with wider dispersion
                const armOffset = (i % 6) * (Math.PI / 3);
                // Stronger winding for gas to create tightly wrapped dust lanes
                const winding = -0.7 * Math.log(r + 1); 
                // Stretch tangentially: dispersion mostly along the arm rather than radially
                const dispersion = randn() * 0.45; 
                const ph = armOffset + winding + dispersion;
                
                // Very tight Z distribution to make them look like sharp, sheared disks
                const zz = randn() * (0.5 + r * 0.02); 
                
                const M_enc = M_bh + this._enclosedMass(r, M_dm, r_s) + this._enclosedMass(r, M_disk, r_disk * 0.4);
                
                // Add a small sub-Keplerian drag so the dust lanes slowly spiral inwards
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 0.5)) * 0.95;
                
                const bodyIndex = this._bodies.length;
                this.addBody(T.NEBULA, M_disk * 0.15 / N_dust,
                    r * Math.cos(ph), zz, r * Math.sin(ph),
                    -vc * Math.sin(ph), randn() * vc * 0.02, vc * Math.cos(ph), 
                    1500 + rng() * 8000); // Cooler gas
                    
                if (this._bodies[bodyIndex]) {
                    // Elongated scale stretching: size inversely proportional to orbital radius 
                    // (inner clouds get shredded smaller, outer clouds remain massive)
                    const shredFactor = Math.min(1.0, r / (r_disk * 0.3));
                    this._bodies[bodyIndex].radius = 15.0 + (rng() * 40.0) * shredFactor; 
                }
            }
            
            // Gas "Wisps" — elongated structures spiraling inwards (falling into the BH)
            const N_wisps = 50; 
            for (let i = 0; i < N_wisps; i++) {
                const u = rng();
                const r = 3 + (1 - Math.pow(1 - u, 3)) * (r_disk * 0.6); // closer to the center
                const ph = PI2 * rng(); // random phase
                const zz = randn() * 1.5; // highly flattened
                
                const M_enc = M_bh + this._enclosedMass(r, M_dm, r_s) + this._enclosedMass(r, M_disk, r_disk * 0.4);
                
                // Sub-Keplerian velocity ensures they spiral inwards!
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 0.5)) * 0.7; // only 70% orbital speed
                
                this.addBody(T.GAS, M_disk * 0.05 / N_wisps,
                    r * Math.cos(ph), zz, r * Math.sin(ph),
                    -vc * Math.sin(ph) - vc*0.2*Math.cos(ph), // slightly angled inwards
                    randn() * vc * 0.02, 
                    vc * Math.cos(ph) - vc*0.2*Math.sin(ph), 
                    35000 + rng() * 15000); // Hotter than standard gas
            }

            this._boxSize = 250;
            this._softening = 6.0;
            this._dt = 0.05;
            this._enableSubgrid = false;

        } else if (name === 'cosmic-cartwheel-collision') {
            // ============================================================
            // Ring Galaxy Collision (Cartwheel)
            // A small dense galaxy punches through a larger disk galaxy.
            // ============================================================
            const M_target = 6000;
            const M_bullet = 1500;
            const target_r_disk = 90;
            const target_r_s = 35;
            
            // Target Galaxy (Face-on disk)
            this.addBody(T.BLACK_HOLE, M_target * 0.05, 0, 0, 0, 0, 0, 0); // Stationary center
            
            // Target DM
            const N_tdm = 600;
            for (let i = 0; i < N_tdm; i++) {
                const r = rng() * target_r_s * 2.5;
                const th = Math.acos(2 * rng() - 1);
                const ph = PI2 * rng();
                const M_enc = this._enclosedMass(r, M_target * 0.95, target_r_s);
                const sigma = Math.sqrt(G_N * M_enc / Math.max(r, 1)) * 0.7;
                this.addBody(T.DARK_MATTER, (M_target * 0.40) / N_tdm,
                    r * Math.sin(th) * Math.cos(ph), r * Math.cos(th), r * Math.sin(th) * Math.sin(ph),
                    sigma * randn(), sigma * randn(), sigma * randn());
            }

            // Target Stars + Nebula
            const N_tstar = 700;
            for (let i = 0; i < N_tstar; i++) {
                const u = rng();
                const r = 4 + (1 - Math.pow(1 - u, 2)) * target_r_disk;
                const ph = PI2 * rng();
                const zz = randn() * (1.0 + r*0.02);
                
                const M_enc = M_target * 0.05 + this._enclosedMass(r, M_target*0.95, target_r_s);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 0.5));
                
                const isNebula = (i % 4 === 0);
                const type = isNebula ? T.NEBULA : T.STAR;
                const mass = (M_target * 0.55) / N_tstar;
                
                const bodyIndex = this._bodies.length;
                this.addBody(type, mass,
                    r * Math.cos(ph), zz, r * Math.sin(ph),
                    -vc * Math.sin(ph), randn() * vc * 0.05, vc * Math.cos(ph),
                    isNebula ? (1000 + rng()*5000) : (4000 + rng()*15000));
                    
                if (isNebula && this._bodies[bodyIndex]) {
                    const shredFactor = Math.min(1.0, r / (target_r_disk * 0.3));
                    this._bodies[bodyIndex].radius = 12.0 + (rng() * 30.0) * shredFactor;
                }
            }
            
            // Bullet Galaxy (plunging along the Y axis)
            const b_y0 = -130;
            const b_vy = Math.sqrt(2 * G_N * M_target / Math.abs(b_y0)) * 1.5; // Hyperbolic plunge
            const bullet_r_s = 15;
            
            this.addBody(T.BLACK_HOLE, M_bullet * 0.1, 0, b_y0, 0, 0, b_vy, 0);
            
            const N_bullet_stars = 300;
            for (let i = 0; i < N_bullet_stars; i++) {
                const r = rng() * bullet_r_s * 2;
                const th = Math.acos(2 * rng() - 1);
                const ph = PI2 * rng();
                const M_enc = this._enclosedMass(r, M_bullet * 0.9, bullet_r_s);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 1));
                
                this.addBody(T.STAR, (M_bullet * 0.9) / N_bullet_stars,
                    r * Math.sin(th) * Math.cos(ph), b_y0 + r * Math.cos(th), r * Math.sin(th) * Math.sin(ph),
                    -vc * Math.sin(th)*Math.cos(ph), b_vy + randn()*vc*0.1, vc * Math.sin(th)*Math.sin(ph),
                    8000 + rng() * 10000); // Blue younger bullet stars
            }
            
            this._boxSize = 250;
            this._softening = 4.0;
            this._dt = 0.035;
            this._enableSubgrid = false;

        } else if (name === 'cosmic-binary-agn') {
            // ============================================================
            // Binary Quasars (Precessing Jets)
            // Two supermassive black holes orbit closely within a shared
            // massive accretion disk, forming precessing plasma jets.
            // ============================================================
            const M_bh = 3000;
            const sep = 30; // Close orbit
            const v_orb = Math.sqrt(G_N * M_bh / (sep * 2));
            
            // The two black holes
            this.addBody(T.BLACK_HOLE, M_bh, -sep/2, 0, 0, 0, 0, -v_orb);
            this.addBody(T.BLACK_HOLE, M_bh, sep/2, 0, 0, 0, 0, v_orb);
            
            // Circumbinary Accretion Disk (Gas + Nebula)
            const N_disk = 1500;
            for (let i = 0; i < N_disk; i++) {
                const u = rng();
                const r = sep * 1.2 + (1 - Math.pow(1 - u, 2)) * 80;
                const ph = PI2 * rng();
                const H = r * 0.15;
                const zz = randn() * H;
                
                const M_enc = M_bh * 2; // Treat as central point mass for outer disk
                const vc = Math.sqrt(G_N * M_enc / r);
                const isNebula = (i % 6 === 0) && (r > sep * 2.0);
                const type = isNebula ? T.NEBULA : T.GAS;
                
                const bodyIndex = this._bodies.length;
                this.addBody(type, isNebula ? 2.0 : 0.5,
                    r * Math.cos(ph), zz, r * Math.sin(ph),
                    -vc * Math.sin(ph), randn() * vc * 0.05, vc * Math.cos(ph),
                    isNebula ? 12000 : 8e5 * Math.pow(sep*1.2 / r, 0.5));
                    
                if (isNebula && this._bodies[bodyIndex]) {
                    const shredFactor = Math.min(1.0, r / 60.0);
                    this._bodies[bodyIndex].radius = 10.0 + (rng() * 20.0) * shredFactor;
                }
            }
            
            // Mass transfer stream connecting them
            for (let i = 0; i < 200; i++) {
                const f = rng();
                const x = -sep/2 + f * sep;
                const y = randn() * 1.5;
                const z = randn() * 1.5;
                this.addBody(T.GAS, 0.2, x, y, z, randn()*v_orb*0.2, randn()*v_orb*0.2, randn()*v_orb*0.2, 5e6);
            }
            
            this._boxSize = 150;
            this._softening = 3.0;
            this._dt = 0.02; // Fine dt for close binary
            this._enableSubgrid = true; // Required for jet tracking
            
        } else if (name === 'cosmic-globular-cluster') {
            // ============================================================
            // Globular Cluster Core Collapse
            // Dense spherical system of stars. No central BH. Starts 
            // in equilibrium, but dynamical friction leads to core collapse.
            // ============================================================
            const M_cluster = 6000;
            const R_c = 15; // Small core radius
            const N_stars = 2000;
            
            for (let i = 0; i < N_stars; i++) {
                // Plummer model distribution
                const u = rng();
                const r = R_c / Math.sqrt(Math.pow(1 - u, -2/3) - 1);
                // Cap radius to avoid edge cases
                const r_cap = Math.min(r, 120);
                
                const th = Math.acos(2 * rng() - 1);
                const ph = PI2 * rng();
                
                // Velocity distribution for Plummer is complex, we approximate isotropic Gaussian
                // scaled by local potential
                const M_enc = M_cluster * Math.pow(r_cap, 3) / Math.pow(r_cap*r_cap + R_c*R_c, 1.5);
                const v_circ = Math.sqrt(G_N * M_enc / Math.max(r_cap, 0.5));
                
                // Isotropic 3D velocity
                const vx = randn() * v_circ * 0.6;
                const vy = randn() * v_circ * 0.6;
                const vz = randn() * v_circ * 0.6;
                
                // Mass varies (some heavy stars sink to the core faster)
                const mass = (M_cluster / N_stars) * (0.2 + rng() * 1.8);
                
                this.addBody(T.STAR, mass,
                    r_cap * Math.sin(th) * Math.cos(ph), r_cap * Math.cos(th), r_cap * Math.sin(th) * Math.sin(ph),
                    vx, vy, vz,
                    3000 + rng() * 6000); // Old, red/yellow population
            }
            
            this._boxSize = 140;
            this._softening = 1.8; // Lower softening to encourage tight interactions
            this._dt = 0.02;
            this._enableSubgrid = false;
            
        } else if (name === 'cosmic-web') {
            // ============================================================
            // Cosmic Filament Web
            // Simulates the mega-scale structure of the universe with 
            // massive nodes connected by dark matter filaments and gas flowing.
            // ============================================================
            const M_node = 1500;
            const nodes = [
                {x: -120, y: 0, z: -80},
                {x: 80, y: 30, z: -100},
                {x: 0, y: -40, z: 120},
                {x: -90, y: 60, z: 50},
                {x: 140, y: -20, z: 40}
            ];
            
            // Add massive nodes (Cluster seeds)
            for (const n of nodes) {
                this.addBody(T.BLACK_HOLE, M_node * 0.05, n.x, n.y, n.z, 0, 0, 0); // Stationary anchors
                
                // Gas surrounding nodes
                for (let i = 0; i < 150; i++) {
                    const r = 2 + rng() * 25;
                    const th = Math.acos(2 * rng() - 1);
                    const ph = PI2 * rng();
                    const vc = Math.sqrt(G_N * M_node / r);
                    this.addBody(T.NEBULA, (M_node * 0.1) / 150,
                        n.x + r * Math.sin(th) * Math.cos(ph),
                        n.y + r * Math.cos(th),
                        n.z + r * Math.sin(th) * Math.sin(ph),
                        -vc * Math.sin(ph), 0, vc * Math.cos(ph),
                        8000 + rng() * 5000);
                    
                    if (this._bodies[this._bodies.length - 1]) {
                        this._bodies[this._bodies.length - 1].radius = 15.0 + rng()*25;
                    }
                }
            }
            
            // Create filaments connecting some nodes
            const filaments = [
                [0, 1], [0, 3], [1, 4], [2, 3], [2, 4], [3, 1]
            ];
            
            // Populate filaments with DM / Gas bridging the voids
            for (const [i, j] of filaments) {
                const n1 = nodes[i];
                const n2 = nodes[j];
                const dx = n2.x - n1.x, dy = n2.y - n1.y, dz = n2.z - n1.z;
                const dist = Math.sqrt(dx*dx + dy*dy + dz*dz);
                const steps = Math.floor(dist * 2.5); // Dense bridge
                
                for (let step = 0; step < steps; step++) {
                    const t = step / steps;
                    // Position along filament + lateral dispersion
                    const cx = n1.x + t * dx;
                    const cy = n1.y + t * dy;
                    const cz = n1.z + t * dz;
                    
                    const thickness = 8 + 4 * Math.sin(t * Math.PI); // wider in the middle
                    const lx = randn() * thickness;
                    const ly = randn() * thickness;
                    const lz = randn() * thickness;
                    
                    // Gas flows towards nearest node
                    const dir = (t < 0.5) ? -1 : 1; 
                    const v_flow = 0.8;
                    const isGas = rng() < 0.3;
                    const type = isGas ? T.NEBULA : T.DARK_MATTER;
                    
                    this.addBody(type, isGas ? 0.8 : 2.0,
                        cx + lx, cy + ly, cz + lz,
                        dir * (dx/dist)*v_flow, dir * (dy/dist)*v_flow, dir * (dz/dist)*v_flow,
                        isGas ? 2000 : 0);
                        
                    if (isGas && this._bodies[this._bodies.length - 1]) {
                        this._bodies[this._bodies.length - 1].radius = 10.0 + rng()*15;
                    }
                }
            }
            
            // Background diffuse sparse gas
            for (let i = 0; i < 400; i++) {
                const x = (rng() - 0.5) * 400;
                const y = (rng() - 0.5) * 400;
                const z = (rng() - 0.5) * 400;
                this.addBody(T.GAS, 0.4, x, y, z, randn()*0.1, randn()*0.1, randn()*0.1, 100);
            }
            
            this._boxSize = 450;
            this._softening = 8.0;
            this._dt = 0.05;
            this._enableSubgrid = true;
            
        } else if (name === 'cosmic-black-hole') {
            const M_bh = 500;
            this.addBody(T.BLACK_HOLE, M_bh, 0, 0, 0);

            // Accretion disk with volume — not razor thin.
            // Real disks have H/r ~ 0.1-0.3 (geometrically thick for hot disks).
            // Inner region is thinner (geometrically thin), outer is puffier.
            for (let i = 0; i < 500; i++) {
                const u = rng();
                const r = 5 + u * u * 55;  // r in [5, 60], wider spread
                const ph = PI2 * rng();
                // Disk thickness: H/r ~ 0.15 inner, ~0.25 outer (flared)
                const H = r * (0.15 + 0.1 * (r / 60));
                const zz = randn() * H;

                const vk = Math.sqrt(G_N * M_bh / r);
                const v_factor = 0.99 - 0.01 * rng();
                // Small z-velocity to support the disk thickness
                const vz = randn() * vk * 0.05;

                this.addBody(T.GAS, 0.2,
                    r * Math.cos(ph), zz, r * Math.sin(ph),
                    -vk * v_factor * Math.sin(ph), vz, vk * v_factor * Math.cos(ph),
                    1e6 * Math.pow(5 / r, 0.75));
            }
            
            // Outer dust clumps falling inwards
            const N_dust_bh = 150;
            for (let i = 0; i < N_dust_bh; i++) {
                const u = rng();
                const r = 15 + u * 60; // Outer regions
                const ph = PI2 * rng();
                const H = r * 0.25;
                const zz = randn() * H;
                
                const vk = Math.sqrt(G_N * M_bh / r);
                // Sub-Keplerian velocity so they spiral inwards
                const v_factor = 0.90 + 0.05 * rng(); 
                const vz = randn() * vk * 0.02;
                
                const bodyIndex = this._bodies.length;
                this.addBody(T.NEBULA, 0.5,
                    r * Math.cos(ph), zz, r * Math.sin(ph),
                    -vk * v_factor * Math.sin(ph), vz, vk * v_factor * Math.cos(ph),
                    25000 * Math.pow(15 / r, 0.5));
                    
                if (this._bodies[bodyIndex]) {
                    const shredFactor = Math.min(1.0, r / 40.0);
                    this._bodies[bodyIndex].radius = 12.0 + (rng() * 25.0) * shredFactor;
                }
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
            for (let i = 0; i < 1600; i++) {
                const r = rng() * r_s1 * 1.8;
                const ph = PI2 * rng();
                const zz = randn() * 1.5;
                const t = i < 800 ? T.DARK_MATTER : T.STAR;
                const M_enc = this._enclosedMass(r, M1, r_s1);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 1));
                const M1_remaining = M1 * 0.95;
                this.addBody(t, (t === T.DARK_MATTER ? M1_remaining * 0.85 : M1_remaining * 0.15) / 800,
                    cx1 + r * Math.cos(ph), zz, cz1 + r * Math.sin(ph),
                    v_approach - vc * Math.sin(ph), randn() * vc * 0.05, v_approach * 0.15 + vc * Math.cos(ph),
                    t === T.STAR ? 4000 + rng() * 18000 : 0);
            }
            
            // Add Nebulae to Galaxy 1
            const N_dust1 = 200;
            for (let i = 0; i < N_dust1; i++) {
                const u = rng();
                const r = 2 + (1 - Math.pow(1 - u, 1.5)) * r_s1 * 1.5;
                const ph = PI2 * rng();
                const zz = randn() * 0.5;
                const M_enc = this._enclosedMass(r, M1, r_s1);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 1)) * 0.95;
                const M1_remaining = M1 * 0.95;
                
                const bodyIndex = this._bodies.length;
                this.addBody(T.NEBULA, (M1_remaining * 0.1) / N_dust1,
                    cx1 + r * Math.cos(ph), zz, cz1 + r * Math.sin(ph),
                    v_approach - vc * Math.sin(ph), randn() * vc * 0.02, v_approach * 0.15 + vc * Math.cos(ph),
                    1500 + rng() * 8000);
                    
                if (this._bodies[bodyIndex]) {
                    const shredFactor = Math.min(1.0, r / (r_s1 * 0.5));
                    this._bodies[bodyIndex].radius = 15.0 + (rng() * 40.0) * shredFactor;
                }
            }

            const cx2 = sep / 2, cz2 = b / 2;
            this.addBody(T.BLACK_HOLE, M2 * 0.05, cx2, 0, cz2, -v_approach, 0, -v_approach * 0.15);
            for (let i = 0; i < 1400; i++) {
                const r = rng() * r_s2 * 1.8;
                const ph = PI2 * rng();
                const zz = randn() * 1.5;
                const t = i < 700 ? T.DARK_MATTER : T.STAR;
                const M_enc = this._enclosedMass(r, M2, r_s2);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 1));
                const M2_remaining = M2 * 0.95;
                this.addBody(t, (t === T.DARK_MATTER ? M2_remaining * 0.85 : M2_remaining * 0.15) / 700,
                    cx2 + r * Math.cos(ph), zz, cz2 + r * Math.sin(ph),
                    -v_approach - vc * Math.sin(ph), randn() * vc * 0.05, -v_approach * 0.15 + vc * Math.cos(ph),
                    t === T.STAR ? 4000 + rng() * 18000 : 0);
            }
            
            // Add Nebulae to Galaxy 2
            const N_dust2 = 180;
            for (let i = 0; i < N_dust2; i++) {
                const u = rng();
                const r = 2 + (1 - Math.pow(1 - u, 1.5)) * r_s2 * 1.5;
                const ph = PI2 * rng();
                const zz = randn() * 0.5;
                const M_enc = this._enclosedMass(r, M2, r_s2);
                const vc = Math.sqrt(G_N * M_enc / Math.max(r, 1)) * 0.95;
                const M2_remaining = M2 * 0.95;
                
                const bodyIndex = this._bodies.length;
                this.addBody(T.NEBULA, (M2_remaining * 0.1) / N_dust2,
                    cx2 + r * Math.cos(ph), zz, cz2 + r * Math.sin(ph),
                    -v_approach - vc * Math.sin(ph), randn() * vc * 0.02, -v_approach * 0.15 + vc * Math.cos(ph),
                    1500 + rng() * 8000);
                    
                if (this._bodies[bodyIndex]) {
                    const shredFactor = Math.min(1.0, r / (r_s2 * 0.5));
                    this._bodies[bodyIndex].radius = 15.0 + (rng() * 40.0) * shredFactor;
                }
            }

            this._boxSize = 250;
            this._softening = 4.0;
            this._dt = 0.04;
            this._enableSubgrid = false;

        } else if (name === 'cosmic-super-cluster') {
            // Three massive galaxies orbiting in a ternary system
            const clusterRadius = 140;
            const galMass = 2500;
            const r_s = 20;
            const r_disk = 45;
            
            for (let gal = 0; gal < 3; gal++) {
                const angle = gal * (PI2 / 3);
                const cx = Math.cos(angle) * clusterRadius;
                const cz = Math.sin(angle) * clusterRadius;
                
                // Calculate pseudo-stable orbital velocity for ternary barycenter
                const orbitV = Math.sqrt(G_N * (galMass*3) / clusterRadius) * 0.45; 
                const vx_sys = -Math.sin(angle) * orbitV;
                const vz_sys = Math.cos(angle) * orbitV;

                this.addBody(T.BLACK_HOLE, galMass * 0.05, cx, 0, cz, vx_sys, 0, vz_sys);
                
                // Stellar arms for each galaxy
                for (let i = 0; i < 350; i++) {
                    const u = rng();
                    const r = 4 + (1 - Math.pow(1 - u, 2)) * r_disk;
                    const arm = (i % 6) * (Math.PI / 3);
                    const winding = -0.6 * Math.log(r + 1);
                    const ph = arm + winding + randn() * 0.3;
                    const zz = randn() * (1.1 + r*0.03);
                    
                    const M_enc = this._enclosedMass(r, galMass, r_s);
                    const vc = Math.sqrt(G_N * M_enc / Math.max(r, 0.5));
                    
                    this.addBody(T.STAR, (galMass * 0.6) / 1000,
                        cx + r * Math.cos(ph), zz, cz + r * Math.sin(ph),
                        vx_sys - vc * Math.sin(ph), randn() * vc * 0.06, vz_sys + vc * Math.cos(ph),
                        3000 + rng() * 25000);
                }
                
                // Add Nebulae for each galaxy
                const N_dust = 200;
                for (let i = 0; i < N_dust; i++) {
                    const u = rng();
                    const r = 4 + (1 - Math.pow(1 - u, 1.5)) * r_disk * 0.9;
                    const arm = (i % 6) * (Math.PI / 3);
                    const winding = -0.7 * Math.log(r + 1);
                    const dispersion = randn() * 0.45; 
                    const ph = arm + winding + dispersion;
                    const zz = randn() * (0.5 + r*0.02);
                    
                    const M_enc = this._enclosedMass(r, galMass, r_s);
                    const vc = Math.sqrt(G_N * M_enc / Math.max(r, 0.5)) * 0.95;
                    
                    const bodyIndex = this._bodies.length;
                    this.addBody(T.NEBULA, (galMass * 0.15) / N_dust,
                        cx + r * Math.cos(ph), zz, cz + r * Math.sin(ph),
                        vx_sys - vc * Math.sin(ph), randn() * vc * 0.02, vz_sys + vc * Math.cos(ph),
                        1500 + rng() * 8000);
                        
                    if (this._bodies[bodyIndex]) {
                        const shredFactor = Math.min(1.0, r / (r_disk * 0.3));
                        this._bodies[bodyIndex].radius = 15.0 + (rng() * 40.0) * shredFactor;
                    }
                }
                
                // DM halo for each galaxy
                for (let i = 0; i < 150; i++) {
                    const r = rng() * r_s * 2.5;
                    const th = Math.acos(2 * rng() - 1);
                    const ph = PI2 * rng();
                    const M_enc = this._enclosedMass(r, galMass, r_s);
                    const vc = Math.sqrt(G_N * M_enc / Math.max(r, 1));
                    this.addBody(T.DARK_MATTER, (galMass * 0.35) / 500,
                        cx + r * Math.sin(th) * Math.cos(ph), r * Math.cos(th), cz + r * Math.sin(th) * Math.sin(ph),
                        vx_sys - vc * Math.sin(th)*Math.cos(ph), randn()*vc*0.1, vz_sys + vc*Math.sin(th)*Math.sin(ph),
                        0);
                }
            }
            
            this._boxSize = 450;
            this._softening = 4.5;
            this._dt = 0.05;
            this._enableSubgrid = false;

        } else if (name === 'cosmic-stellar-lifecycle') {
            // ============================================================
            // Stellar Lifecycle: Birth → Main Sequence → Death → Remnant
            //
            // A massive gas cloud seeded by dark matter scaffolding collapses,
            // forms stars, the most massive star evolves through fusion stages,
            // exhausts fuel, and collapses to a remnant (WD, NS, or BH
            // depending on mass). Demonstrates the full lifecycle from the
            // theory narrative (DERIV_STELLAR_LIFECYCLE_LATTICE.md):
            //
            //   Stage 3: Cloud gathers under self-gravity
            //   Stage 4: Dark matter scaffolding seeds collapse
            //   Stage 5: Star ignites — balanced budget era
            //   Stage 6: Fuel exhaustion → budget deficit → death
            //   Stage 7-8: Remnant formation (BH if massive enough)
            //   Stage 9: Hawking evaporation (if BH, accelerated timescale)
            //
            // Stars track fuel_fraction (1.0 = full, 0.0 = exhausted).
            // Fuel burns at a rate proportional to luminosity.
            // When fuel hits 0, the star enters death sequence:
            //   - mass < 1.4 M_ch → WHITE_DWARF
            //   - mass < 3.0 M_ch → NEUTRON_STAR (+ supernova ejecta)
            //   - mass > 3.0 M_ch → BLACK_HOLE (+ supernova ejecta)
            // ============================================================

            const M_cloud = 5000;
            const R_cloud = 50;
            const N_gas = 600;
            const N_dm = 250;

            // Dark matter scaffolding — forms the potential well first
            // (Stage 4: invisible architecture that seeds baryonic collapse)
            for (let i = 0; i < N_dm; i++) {
                let rx, ry, rz, r2;
                do {
                    rx = randn() * 0.5;
                    ry = randn() * 0.5;
                    rz = randn() * 0.5;
                    r2 = rx*rx + ry*ry + rz*rz;
                } while (r2 > 1.0);
                const x = rx * R_cloud * 0.7;
                const y = ry * R_cloud * 0.7;
                const z = rz * R_cloud * 0.7;
                const sigma = 0.04 * Math.sqrt(G_N * M_cloud / R_cloud);
                this.addBody(T.DARK_MATTER, M_cloud * 0.25 / N_dm,
                    x, y, z,
                    sigma * randn(), sigma * randn(), sigma * randn());
            }

            // Gas cloud — diffuse, slowly collapsing
            // (Stage 3: the gathering, Jeans instability in progress)
            for (let i = 0; i < N_gas; i++) {
                let rx, ry, rz, r2;
                do {
                    rx = (rng() - 0.5) * 2;
                    ry = (rng() - 0.5) * 2;
                    rz = (rng() - 0.5) * 2;
                    r2 = rx*rx + ry*ry + rz*rz;
                } while (r2 > 1.0);
                const x = rx * R_cloud;
                const y = ry * R_cloud;
                const z = rz * R_cloud;

                // Very small velocity (cold gas)
                const v = 0.05 * Math.sqrt(G_N * M_cloud / R_cloud);
                this.addBody((i % 4 === 0) ? T.NEBULA : T.GAS, M_cloud * 0.75 / N_gas,
                    x, y, z,
                    v * randn(), v * randn(), v * randn(),
                    50 + rng() * 100);
                    
                if (i % 4 === 0 && this._bodies.length > 0) {
                    this._bodies[this._bodies.length - 1].radius = 15.0 + rng() * 30.0;
                }
            }

            this._boxSize = 140;
            this._softening = 3.5;
            this._dt = 0.025;
            this._enableSubgrid = true;  // Enable fuel tracking + death sequence
            this._stellarEvolution = true;  // Enable fuel tracking + death sequence
            this._hawkingEvaporation = true; // Enable BH mass loss

        } else if (name === 'cosmic-ftd-collapse') {
            // ============================================================
            // FTD Black Hole: Emergent gravitational collapse
            //
            // No pre-placed black hole. A dense cloud of gas and stars
            // collapses under self-gravity. When central density exceeds
            // the critical threshold (escape velocity > c = 1/sqrt(3)),
            // the region becomes an emergent "black hole" — not a
            // singularity, but a saturated latency well on the lattice.
            //
            // FTD predicts:
            //   - No infinities (discrete lattice, finite states)
            //   - No true event horizon (L → 1 but never reaches it)
            //   - BH is a configuration, not a fundamental object
            //   - Must form FROM something (has a seed)
            //   - Information trapped for cosmological time, not forever
            //
            // The scenario starts as a uniform-density gas cloud with
            // slight random perturbations. Over time:
            //   1. Cloud contracts under self-gravity
            //   2. Core density increases, temperature rises
            //   3. Some gas converts to stars (Jeans collapse)
            //   4. Central region reaches v_escape > c → emergent BH
            //   5. The "BH" grows by accreting surrounding material
            // ============================================================

            const M_cloud = 3000;  // total cloud mass
            const R_cloud = 40;    // initial cloud radius
            const N_gas = 500;
            const N_dm = 200;      // small DM seed to help collapse

            // Dense gas cloud — uniform sphere with random perturbations
            // The cloud is mostly gas, with some initial stars and dense nebulae
            for (let i = 0; i < N_gas; i++) {
                // Uniform random position in sphere
                let rx, ry, rz, r2;
                do {
                    rx = (rng() - 0.5) * 2;
                    ry = (rng() - 0.5) * 2;
                    rz = (rng() - 0.5) * 2;
                    r2 = rx*rx + ry*ry + rz*rz;
                } while (r2 > 1.0);
                const x = rx * R_cloud;
                const y = ry * R_cloud;
                const z = rz * R_cloud;

                // Small initial inward velocity (cloud is collapsing)
                const r = Math.sqrt(x*x + y*y + z*z) + 0.01;
                const v_infall = -0.1 * Math.sqrt(G_N * M_cloud / R_cloud);
                // Plus small random tangential velocity (angular momentum)
                const ph = Math.atan2(z, x);
                const v_tang = 0.15 * Math.sqrt(G_N * M_cloud / R_cloud);
                
                const type = (i < N_gas * 0.2) ? T.STAR : ((i % 5 === 0) ? T.NEBULA : T.GAS);

                this.addBody(type, M_cloud * 0.8 / N_gas,
                    x, y, z,
                    v_infall * x/r + v_tang * (-Math.sin(ph)) * (Math.random()*0.5 + 0.5),
                    v_infall * y/r + randn() * v_tang * 0.3,
                    v_infall * z/r + v_tang * (Math.cos(ph)) * (Math.random()*0.5 + 0.5),
                    (type === T.STAR) ? 4000 + rng() * 15000 : (100 + rng() * 200));
                    
                if (type === T.NEBULA && this._bodies.length > 0) {
                    this._bodies[this._bodies.length - 1].radius = 18.0 + rng() * 35.0;
                }
            }

            // DM seed — concentrated near center, helps initiate collapse
            for (let i = 0; i < N_dm; i++) {
                let rx, ry, rz, r2;
                do {
                    rx = randn() * 0.4; // Gaussian, concentrated
                    ry = randn() * 0.4;
                    rz = randn() * 0.4;
                    r2 = rx*rx + ry*ry + rz*rz;
                } while (r2 > 1.0);
                const x = rx * R_cloud * 0.5;  // inner half of cloud
                const y = ry * R_cloud * 0.5;
                const z = rz * R_cloud * 0.5;

                const sigma = 0.05 * Math.sqrt(G_N * M_cloud / R_cloud);
                this.addBody(T.DARK_MATTER, M_cloud * 0.2 / N_dm,
                    x, y, z,
                    sigma * randn(), sigma * randn(), sigma * randn());
            }

            this._boxSize = 120;
            this._softening = 3.0;
            this._dt = 0.03;
            this._enableSubgrid = true;  // need cooling for collapse + star formation

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

        // JIT SoA (Structure of Arrays) Initialization
        // Maintain local Float64Arrays scoped to the class to avoid GC churn
        if (!this._soa || this._soa.n < n) {
            const MathMaxOffset = 1000; // Buffer space
            const capacity = n + MathMaxOffset; 
            this._soa = {
                n: capacity,
                x: new Float64Array(capacity),
                y: new Float64Array(capacity),
                z: new Float64Array(capacity),
                mass: new Float64Array(capacity),
                soft: new Float64Array(capacity),
                softSq: new Float64Array(capacity),
                ax: new Float64Array(capacity),
                ay: new Float64Array(capacity),
                az: new Float64Array(capacity)
            };
        }

        const soa = this._soa;
        const X = soa.x, Y = soa.y, Z = soa.z, M = soa.mass;
        const SOFT = soa.soft, SQ = soa.softSq;
        const AX = soa.ax, AY = soa.ay, AZ = soa.az;

        // 1. O(N) memory flattening: Unpack JS objects into flat typed arrays
        for (let i = 0; i < n; i++) {
            const b = this._bodies[i];
            X[i] = b.x;
            Y[i] = b.y;
            Z[i] = b.z;
            M[i] = b.mass;
            SOFT[i] = SOFTENING[b.type] || 2.0;
            SQ[i] = SOFTENING_SQ[b.type] || 4.0;
            AX[i] = 0.0;
            AY[i] = 0.0;
            AZ[i] = 0.0;
        }

        // 2. O(N^2) cache-local, pure Float64Array iteration
        // V8 SIMD auto-vectorizes this loop beautifully with absolutely no dictionary lookups!
        for (let i = 0; i < n; i++) {
            const bix = X[i], biy = Y[i], biz = Z[i], bim = M[i];
            const s_i = SOFT[i], sq_i = SQ[i];
            
            let ax = AX[i], ay = AY[i], az = AZ[i];

            for (let j = i + 1; j < n; j++) {
                // Gadget-2 rule: eps = max(eps_i, eps_j)
                const s_j = SOFT[j];
                const eps2 = s_i > s_j ? sq_i : SQ[j];

                const dx = X[j] - bix;
                const dy = Y[j] - biy;
                const dz = Z[j] - biz;
                const r2 = dx * dx + dy * dy + dz * dz + eps2;
                const invR3 = 1.0 / (r2 * Math.sqrt(r2));

                const f_j = G * M[j] * invR3;
                const f_i = G * bim * invR3;
                
                ax += f_j * dx;
                ay += f_j * dy;
                az += f_j * dz;
                
                AX[j] -= f_i * dx;
                AY[j] -= f_i * dy;
                AZ[j] -= f_i * dz;
            }
            AX[i] = ax;
            AY[i] = ay;
            AZ[i] = az;
        }
        
        // 3. O(N) mapping: Restitute results back to Javascript object instances
        for (let i = 0; i < n; i++) {
            const b = this._bodies[i];
            b.ax = AX[i];
            b.ay = AY[i];
            b.az = AZ[i];
        }

        // No hard-core repulsion needed — bodies that reach the event horizon
        // are absorbed in _postUpdates(). The Plummer softening (BH eps=3.0)
        // prevents force singularity at r=0. Bodies on radial orbits either
        // get tidally disrupted (stars) or absorbed (everything) at the horizon.

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
        const T = CosmicMockBridge.TYPE;
        const G = G_N;
        const isGas = (t) => t === T.GAS || t === T.NEBULA;
        const isBH = (t) => t === T.BLACK_HOLE || t === T.QUASAR;
        const isStar = (t) => t === T.STAR || t === T.NEUTRON_STAR || t === T.WHITE_DWARF;

        // ── ALWAYS ACTIVE (all scenarios) ──

        // Event horizon absorption: ANY body crossing the Schwarzschild radius
        // is consumed by the BH. Mass transfers, body is destroyed.
        // AGN Relativistic Jet logic: Tracks continuous accretion to power a shader
        for (const bh of this._bodies) {
            if (!isBH(bh.type)) continue;
            
            // Cool down jet intensity each tick
            bh.luminosity = (bh.luminosity || 0) * 0.96;
            
            // The Schwarzschild boundary where things completely vanish.
            // Visually let them travel into the shadow by halving the physical kill radius
            const r_horizon = Math.max(0.8, Math.cbrt(bh.mass) * 0.12);
            const r_kill = r_horizon * 0.4; // They survive physically longer so the renderer draws them crossing the void
            const r_h2 = r_kill * r_kill;
            
            for (const b of this._bodies) {
                if (b.id === bh.id || b.mass <= 0) continue;
                const dx = b.x - bh.x, dy = b.y - bh.y, dz = b.z - bh.z;
                if (dx*dx + dy*dy + dz*dz < r_h2) {
                    bh.mass += b.mass;
                    
                    // Fuel the AGN jet intensity proportionally to consumed mass
                    bh.luminosity = Math.min((bh.luminosity || 0) + b.mass * 8.0, 50.0);
                    
                    b.mass = 0;
                }
            }
        }

        // Gradual tidal disruption (spaghettification):
        // Stars near a BH don't instantly convert — they gradually stretch
        // and shed mass along their orbit over many ticks.
        //
        // Phase 1 (r < 2*r_tidal): tidal_stretch increases each tick
        // Phase 2 (stretch > 0.3): star starts shedding gas along velocity vector
        // Phase 3 (stretch > 1.0 or mass < 20% original): star fully dissolved
        //
        // The shed gas forms a thin stream along the orbit — visible spaghettification.
        const newGas = [];
        for (const bh of this._bodies) {
            if (!isBH(bh.type)) continue;
            for (const star of this._bodies) {
                if (!isStar(star.type) || star.mass <= 0) continue;
                const dx = star.x - bh.x, dy = star.y - bh.y, dz = star.z - bh.z;
                const r2 = dx*dx + dy*dy + dz*dz;
                const r = Math.sqrt(r2 + 0.01);

                // Tidal disruption radius (Rees 1988):
                //   r_t = R_star × (M_BH / M_star)^(1/3)
                // R_star is the body's physical radius (set at creation as cbrt(mass)*0.1)
                const R_star = star.radius || Math.cbrt(star.mass) * 0.1;
                const r_tidal = R_star * Math.pow(bh.mass / (star.mass + 0.01), 1/3);

                if (r < r_tidal * 1.5) {
                    // Inside tidal influence zone — stretch increases
                    // Closer = faster stretching (tidal force ~ 1/r^3)
                    const tidalForce = bh.mass / (r2 * r + 0.01);
                    star.tidal_stretch = Math.min(1.5, (star.tidal_stretch || 0) + tidalForce * 0.0005);
                } else {
                    // Outside zone — stretch relaxes slowly (star re-compacts)
                    star.tidal_stretch = Math.max(0, (star.tidal_stretch || 0) - 0.002);
                }

                // Phase 2: shedding mass when stretched enough
                if ((star.tidal_stretch || 0) > 0.3 && r < r_tidal * 1.2) {
                    // Shed a small gas fragment along the velocity direction each tick
                    // This creates the visible "spaghetti stream"
                    const shedFraction = Math.min(0.05, star.tidal_stretch * 0.02);
                    const shedMass = star.mass * shedFraction;
                    if (shedMass > 0.01) {
                        star.mass -= shedMass;
                        // Place fragment slightly behind the star along its velocity
                        const v = Math.sqrt(star.vx*star.vx + star.vy*star.vy + star.vz*star.vz) + 0.01;
                        const jitter = 0.15; // small random spread for stream width
                        newGas.push({
                            mass: shedMass,
                            x: star.x - star.vx/v * 0.5 + (Math.random()-0.5)*jitter,
                            y: star.y - star.vy/v * 0.5 + (Math.random()-0.5)*jitter,
                            z: star.z - star.vz/v * 0.5 + (Math.random()-0.5)*jitter,
                            vx: star.vx * (0.9 + Math.random()*0.2), // slight velocity spread
                            vy: star.vy * (0.9 + Math.random()*0.2),
                            vz: star.vz * (0.9 + Math.random()*0.2),
                            temp: 5e4 * (1 + star.tidal_stretch) // hotter as more stretched
                        });
                    }
                }

                // Phase 3: fully dissolved when mass drops below 20% of original
                if (star.mass < (star.original_mass || star.mass) * 0.2 && (star.tidal_stretch || 0) > 0.8) {
                    // Final burst — remaining mass becomes gas
                    if (star.mass > 0.02) {
                        newGas.push({
                            mass: star.mass, x: star.x, y: star.y, z: star.z,
                            vx: star.vx, vy: star.vy, vz: star.vz, temp: 1e5
                        });
                    }
                    star.mass = 0;
                }
            }
        }
        for (const g of newGas) {
            this.addBody(T.GAS, g.mass, g.x, g.y, g.z, g.vx, g.vy, g.vz, g.temp);
        }

        // BH-BH mergers (always active — needed for merger scenario)
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
                bi.mass = m_total * 0.95; // 5% GW
                bj.mass = 0;
            }
        }

        // ── Emergent BH formation (FTD prediction) ──
        // When a region becomes dense enough that v_escape > c = 1/sqrt(3),
        // the densest body converts to a BLACK_HOLE. This is NOT a singularity —
        // it's a lattice configuration where the latency field saturates.
        // Check: for each non-BH body, count nearby mass within softening radius.
        // If enclosed mass gives v_esc = sqrt(2*G*M_enc/r) > C_SPEED, convert.
        const C_SPEED = 1.0 / Math.sqrt(3.0);
        // Only form a BH if none exist yet — one seed per simulation.
        // Find the single densest point and convert only that one body.
        const hasBH = this._bodies.some(b => isBH(b.type));
        if (!hasBH) {
            let bestBody = null, bestMenc = 0;
            const checkR = this._softening * 2;
            const checkR2 = checkR * checkR;
            for (const b of this._bodies) {
                if (b.mass <= 0) continue;
                let M_enc = 0;
                for (const other of this._bodies) {
                    if (other.id === b.id) continue;
                    const dr2 = (b.x-other.x)**2 + (b.y-other.y)**2 + (b.z-other.z)**2;
                    if (dr2 < checkR2) M_enc += other.mass;
                }
                if (M_enc > bestMenc) { bestMenc = M_enc; bestBody = b; }
            }
            if (bestBody) {
                const v_esc = Math.sqrt(2 * G * bestMenc / checkR);
                if (v_esc > C_SPEED && bestMenc > 50) {
                    // The densest point has collapsed past the FTD threshold.
                    // One body becomes the seed black hole.
                    bestBody.type = T.BLACK_HOLE;
                    bestBody.temperature = 0;
                    bestBody.luminosity = 0;
                    bestBody.tidal_stretch = 0;
                }
            }
        }

        // ── SUBGRID ONLY (BH accretion / FTD collapse scenarios) ──

        if (this._enableSubgrid) {
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

            // BH accretion of bound gas (Bondi-like)
            for (const bh of this._bodies) {
                if (!isBH(bh.type)) continue;
                const r_acc = Math.max(1.5, Math.cbrt(bh.mass) * 0.3);
                const r_acc2 = r_acc * r_acc;
                for (const gas of this._bodies) {
                    if (!isGas(gas.type) || gas.mass <= 0) continue;
                    const dx = gas.x - bh.x, dy = gas.y - bh.y, dz = gas.z - bh.z;
                    const r2 = dx * dx + dy * dy + dz * dz;
                    if (r2 > r_acc2) continue;
                    const dvx = gas.vx-bh.vx, dvy = gas.vy-bh.vy, dvz = gas.vz-bh.vz;
                    const v_rel2 = dvx*dvx + dvy*dvy + dvz*dvz;
                    const r = Math.sqrt(r2 + 0.01);
                    if (v_rel2 > 2 * G * bh.mass / r) continue;
                    const rate = 0.005 * bh.mass / (v_rel2 + 0.1);
                    const dm = Math.min(gas.mass * 0.1, gas.mass * rate * 0.001);
                    bh.mass += dm;
                    gas.mass -= dm;
                }
            }
        }

        // ── STELLAR EVOLUTION (lifecycle scenario) ──
        // Stars burn fuel over time. When fuel runs out, they die.
        // Fuel burn rate ~ luminosity (L ~ M^3.5), so massive stars die fast.
        if (this._stellarEvolution) {
            const M_chandrasekhar = 70;  // ~1.4 solar masses in lattice units
            const M_tov = 150;           // ~3 solar masses (Tolman-Oppenheimer-Volkoff)
            const newEjecta = [];

            for (const b of this._bodies) {
                b.age = (b.age || 0) + 1;

                if (!isStar(b.type) || b.mass <= 0) continue;
                if (b.type === T.NEUTRON_STAR || b.type === T.WHITE_DWARF) continue;

                // Fuel consumption: rate proportional to luminosity
                // L ~ M^3.5, so massive stars burn ~1000x faster than small ones
                const fuelRate = 0.00002 * Math.pow(b.mass / 50, 2.5);
                b.fuel_fraction = Math.max(0, (b.fuel_fraction || 1.0) - fuelRate);

                // Energy budget tracking
                const fusionIncome = b.luminosity * 0.001;
                const gravDrain = G_N * b.mass * b.mass * 0.0001;
                const radLoss = b.luminosity * 0.0005;
                b.budget_income = fusionIncome * (b.fuel_fraction > 0 ? 1 : 0);
                b.budget_expense = gravDrain + radLoss;

                // Update luminosity based on fuel stage
                // As fuel depletes, star evolves: luminosity changes
                if (b.fuel_fraction > 0.3) {
                    // Main sequence: steady luminosity (Stage 5: balanced budget)
                    b.luminosity = Math.pow(b.mass, 3.5);
                    b.temperature = 5800 * Math.pow(b.mass / 50, 0.5);
                    b.fuel_stage = 0;
                } else if (b.fuel_fraction > 0.15) {
                    // Red giant phase: luminosity spikes, temp drops
                    b.luminosity = Math.pow(b.mass, 3.5) * 3.0;
                    b.temperature = 3500;
                    b.radius = Math.cbrt(b.mass) * 0.4; // expanded
                    b.fuel_stage = 1;
                } else if (b.fuel_fraction > 0.05) {
                    // Late burning (He/C/O): shrinks, heats up
                    b.luminosity = Math.pow(b.mass, 3.5) * 1.5;
                    b.temperature = 15000;
                    b.radius = Math.cbrt(b.mass) * 0.08;
                    b.fuel_stage = Math.min(4, Math.floor((0.15 - b.fuel_fraction) / 0.025) + 2);
                } else if (b.fuel_fraction <= 0) {
                    // ========================================
                    // DEATH — Stage 6: Budget Deficit
                    // ========================================
                    b.fuel_stage = 5; // Iron — no more fusion

                    if (b.mass < M_chandrasekhar) {
                        // White dwarf: electron degeneracy halts collapse
                        b.type = T.WHITE_DWARF;
                        b.luminosity = Math.pow(b.mass, 0.5) * 0.01;
                        b.temperature = 12000;
                        b.radius = Math.cbrt(b.mass) * 0.02;
                        b.fuel_fraction = 0;

                    } else if (b.mass < M_tov) {
                        // Neutron star: supernova ejects outer layers
                        const ejectMass = b.mass * 0.7;
                        b.mass -= ejectMass;
                        b.type = T.NEUTRON_STAR;
                        b.luminosity = 0.1;
                        b.temperature = 1e6;
                        b.radius = Math.cbrt(b.mass) * 0.005;
                        b.fuel_fraction = 0;
                        // Supernova ejecta — expanding shell of hot gas
                        for (let k = 0; k < 12; k++) {
                            const theta = Math.acos(2 * Math.random() - 1);
                            const phi = Math.PI * 2 * Math.random();
                            const v_eject = 2.0 + Math.random() * 1.0;
                            newEjecta.push({
                                mass: ejectMass / 12,
                                x: b.x + Math.sin(theta) * Math.cos(phi) * 1.5,
                                y: b.y + Math.sin(theta) * Math.sin(phi) * 1.5,
                                z: b.z + Math.cos(theta) * 1.5,
                                vx: b.vx + v_eject * Math.sin(theta) * Math.cos(phi),
                                vy: b.vy + v_eject * Math.sin(theta) * Math.sin(phi),
                                vz: b.vz + v_eject * Math.cos(theta),
                                temp: 1e6
                            });
                        }

                    } else {
                        // Black hole: nothing stops collapse (Stage 7)
                        const ejectMass = b.mass * 0.5;
                        b.mass -= ejectMass;
                        b.type = T.BLACK_HOLE;
                        b.luminosity = 0;
                        b.temperature = 0;
                        b.fuel_fraction = 0;
                        // Supernova ejecta
                        for (let k = 0; k < 15; k++) {
                            const theta = Math.acos(2 * Math.random() - 1);
                            const phi = Math.PI * 2 * Math.random();
                            const v_eject = 2.5 + Math.random() * 1.5;
                            newEjecta.push({
                                mass: ejectMass / 15,
                                x: b.x + Math.sin(theta) * Math.cos(phi) * 2.0,
                                y: b.y + Math.sin(theta) * Math.sin(phi) * 2.0,
                                z: b.z + Math.cos(theta) * 2.0,
                                vx: b.vx + v_eject * Math.sin(theta) * Math.cos(phi),
                                vy: b.vy + v_eject * Math.sin(theta) * Math.sin(phi),
                                vz: b.vz + v_eject * Math.cos(theta),
                                temp: 2e6
                            });
                        }
                    }
                }
            }
            // Spawn supernova ejecta as nebula gas
            for (const e of newEjecta) {
                this.addBody(T.NEBULA, e.mass, e.x, e.y, e.z, e.vx, e.vy, e.vz, e.temp);
            }
        }

        // ── HAWKING EVAPORATION (Stage 9) ──
        // BHs slowly lose mass via Hawking radiation. T_H ~ 1/M, so
        // smaller BHs evaporate faster. The mass loss rate is:
        //   dM/dt ~ -1/M^2 (in natural units)
        // We use accelerated timescale for visual effect.
        if (this._hawkingEvaporation) {
            for (const b of this._bodies) {
                if (!isBH(b.type) || b.mass <= 0) continue;

                // Hawking temperature (arbitrary units, scaled for visibility)
                const T_hawking = 500.0 / (b.mass + 1);

                // Mass loss rate: dM/dt = -sigma * T^4 * A ~ -1/M^2
                // Accelerated by a factor for visual dynamics
                const hawkingRate = 0.0001 / (b.mass * b.mass + 1);
                const dm = Math.min(b.mass * 0.01, hawkingRate);
                b.mass -= dm;

                // Store Hawking temperature for renderer (glow effect)
                b.hawking_temp = T_hawking;
                b.budget_expense = dm; // budget leak rate

                // When BH mass drops very low, it "pops" — final burst
                if (b.mass < 2.0) {
                    // Final evaporation burst (Stage 9 endgame)
                    const burstEnergy = b.mass;
                    b.mass = 0; // BH gone
                    // Emit burst radiation as hot nebula
                    for (let k = 0; k < 6; k++) {
                        const theta = Math.acos(2 * Math.random() - 1);
                        const phi = Math.PI * 2 * Math.random();
                        const v_burst = 3.0;
                        this.addBody(T.NEBULA, burstEnergy / 6,
                            b.x + Math.sin(theta) * Math.cos(phi),
                            b.y + Math.sin(theta) * Math.sin(phi),
                            b.z + Math.cos(theta),
                            v_burst * Math.sin(theta) * Math.cos(phi),
                            v_burst * Math.sin(theta) * Math.sin(phi),
                            v_burst * Math.cos(theta),
                            1e7);
                    }
                }
            }
        }

        // ── CUSTOM SCENARIO TELEMETRY ──
        this._updateTelemetry();

        // Speed limit + cleanup (always)
        this._enforceSpeedLimit();
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

    _updateTelemetry() {
        const name = this._scenarioName;
        const T = CosmicMockBridge.TYPE;
        const G = G_N;
        const isBH = (t) => t === T.BLACK_HOLE || t === T.QUASAR;
        const isStar = (t) => t === T.STAR || t === T.NEUTRON_STAR || t === T.WHITE_DWARF;

        const tel = {};

        if (name === 'cosmic-merger' || name === 'cosmic-binary-agn') {
            const bhs = this._bodies.filter(b => isBH(b.type)).sort((a,b) => b.mass - a.mass);
            if (bhs.length >= 2) {
                const dx = bhs[0].x - bhs[1].x, dy = bhs[0].y - bhs[1].y, dz = bhs[0].z - bhs[1].z;
                const sep = Math.sqrt(dx*dx + dy*dy + dz*dz);
                tel['Core Separation'] = sep.toFixed(2) + ' lu';
                if (name === 'cosmic-binary-agn') {
                    tel['Peak Jet Power'] = Math.max(bhs[0].luminosity||0, bhs[1].luminosity||0).toExponential(2) + ' EJ/s';
                }
            } else if (bhs.length === 1) {
                tel['Status'] = 'Merger Complete';
                tel['Singularity Mass'] = bhs[0].mass.toFixed(1) + ' M\u2299';
            }
        } else if (name === 'cosmic-cartwheel-collision') {
            const bhs = this._bodies.filter(b => isBH(b.type));
            if (bhs.length >= 2) {
                const dx = bhs[0].x - bhs[1].x, dy = bhs[0].y - bhs[1].y, dz = bhs[0].z - bhs[1].z;
                const sep = Math.sqrt(dx*dx + dy*dy + dz*dz);
                tel['Bullet Distance'] = sep.toFixed(1) + ' lu';
                tel['Impact Phase'] = (bhs[1].y > 0) ? 'Post-Collision' : 'Approach';
            }
        } else if (name === 'cosmic-globular-cluster') {
            let coreStars = 0;
            let M_core = 0;
            for (const b of this._bodies) {
                if (isStar(b.type)) {
                    if (b.x*b.x + b.y*b.y + b.z*b.z < 100) { // core radius squared
                        coreStars++;
                        M_core += b.mass;
                    }
                }
            }
            tel['Core Population (r<10)'] = coreStars;
            tel['Core Density'] = (M_core / (4/3 * Math.PI * 1000)).toExponential(2) + ' M\u2299/lu\u00B3';
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
            positions[i*3] = b.x; positions[i*3+1] = b.y; positions[i*3+2] = b.z;
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
            speed: Math.sqrt(b.vx*b.vx + b.vy*b.vy + b.vz*b.vz),
            temperature: b.temperature || 0,
            luminosity: b.luminosity || 0,
            fuel_fraction: b.fuel_fraction != null ? b.fuel_fraction : 1.0,
            fuel_stage: b.fuel_stage || 0,
            age: b.age || 0,
            radius: b.radius || (Math.cbrt(b.mass) * (1 + (b.tidal_stretch||0) * 2))
        };
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
