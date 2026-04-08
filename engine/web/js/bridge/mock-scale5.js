/**
 * CosmicMockBridge — JS-only N-body simulation for cosmic scale (Scale 5).
 *
 * Extracted from wasm-bridge.js to reduce monolith size.
 * Uses direct O(N^2) summation (sufficient for <5000 bodies in dev mode).
 */

import { G_N, OMEGA_LAMBDA, OMEGA_MATTER } from '../constants.js';

export class CosmicMockBridge {
    constructor() {
        this._bodies = [];
        this._tick = 0;
        this._nextId = 0;
        this._dt = 0.001;
        this._a = 1.0;        // Scale factor
        this._adot = 0.0;
        this._H0 = 0.07;
        this._boxSize = 1000;
        this._softening = 1.0;
        this._gwEvents = [];
    }

    // Body types matching CosmicBodyType enum
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
            // Spiral galaxy: ~800 bodies for smooth JS performance
            // Central SMBH
            this.addBody(T.BLACK_HOLE, 1e9, 0, 0, 0, 0, 0, 0, 0);
            // DM halo (spherical, NFW-ish)
            for (let i = 0; i < 300; i++) {
                const r = 5 + rng() * 195;
                const th = Math.acos(2*rng()-1);
                const ph = PI2 * rng();
                const x = r*Math.sin(th)*Math.cos(ph);
                const y = r*Math.sin(th)*Math.sin(ph);
                const z = r*Math.cos(th);
                const vc = Math.sqrt(G_N * 5e11 / (r + 10));
                const sig = vc * 0.3;
                this.addBody(T.DARK_MATTER, 5e6, x, y, z,
                    sig*(rng()-0.5)*2, sig*(rng()-0.5)*2, sig*(rng()-0.5)*2);
            }
            // Stellar disk (flat, spiral structure via density wave)
            for (let i = 0; i < 350; i++) {
                const r = 3 + rng() * 55;
                const arm = Math.floor(rng() * 2); // 2 spiral arms
                const phi_base = arm * Math.PI + 0.4 * Math.log(r + 1);
                const ph = phi_base + (rng()-0.5) * 0.8;
                const z = (rng()-0.5) * 2.0;
                const vc = Math.sqrt(G_N * 8e11 * r / (r*r + 25));
                this.addBody(T.STAR, 1e5, r*Math.cos(ph), z, r*Math.sin(ph),
                    -vc*Math.sin(ph), 0, vc*Math.cos(ph), 3000 + rng()*25000);
            }
            // Gas disk (wider, with spiral)
            for (let i = 0; i < 150; i++) {
                const r = 5 + rng() * 70;
                const arm = Math.floor(rng() * 2);
                const phi_base = arm * Math.PI + 0.4 * Math.log(r + 1);
                const ph = phi_base + (rng()-0.5) * 1.2;
                const z = (rng()-0.5) * 1.5;
                const vc = Math.sqrt(G_N * 8e11 * r / (r*r + 25));
                this.addBody(T.GAS, 5e4, r*Math.cos(ph), z, r*Math.sin(ph),
                    -vc*Math.sin(ph), 0, vc*Math.cos(ph), 5000 + rng()*15000);
            }
            this._boxSize = 300;
            this._dt = 0.002;
        } else if (name === 'cosmic-black-hole') {
            // Black hole with accretion disk: ~500 bodies
            const bhMass = 1e10;
            this.addBody(T.BLACK_HOLE, bhMass, 0, 0, 0);
            const rs = 2 * G_N * bhMass;
            for (let i = 0; i < 500; i++) {
                const r = rs*6 + rng() * rs * 60;
                const ph = PI2 * rng();
                const z = (rng()-0.5) * rs * 0.3;
                const vk = Math.sqrt(G_N * bhMass / r);
                this.addBody(T.GAS, 1e3, r*Math.cos(ph), z, r*Math.sin(ph),
                    -vk*Math.sin(ph), 0, vk*Math.cos(ph), 5e5 + rng()*5e6);
            }
            this._boxSize = rs * 150;
            this._dt = 0.0005;
        } else if (name === 'cosmic-merger') {
            // Galaxy merger: ~600 bodies total
            const sep = 120;
            const v_app = 0.3;
            // Galaxy 1
            this.addBody(T.BLACK_HOLE, 5e8, -sep/2, 0, 0, v_app, v_app*0.2, 0);
            for (let i = 0; i < 250; i++) {
                const r = rng()*25; const ph = PI2*rng(); const z = (rng()-0.5)*2;
                const vc = Math.sqrt(G_N * 3e11 * r / (r*r + 16));
                const t = i < 120 ? T.DARK_MATTER : T.STAR;
                this.addBody(t, 2e5, -sep/2+r*Math.cos(ph), z, r*Math.sin(ph),
                    v_app - vc*Math.sin(ph), 0, vc*Math.cos(ph),
                    t === T.STAR ? 4000+rng()*18000 : 0);
            }
            // Galaxy 2
            this.addBody(T.BLACK_HOLE, 3e8, sep/2, 0, 0, -v_app, -v_app*0.2, 0);
            for (let i = 0; i < 200; i++) {
                const r = rng()*20; const ph = PI2*rng(); const z = (rng()-0.5)*2;
                const vc = Math.sqrt(G_N * 2e11 * r / (r*r + 16));
                const t = i < 100 ? T.DARK_MATTER : T.STAR;
                this.addBody(t, 2e5, sep/2+r*Math.cos(ph), z, r*Math.sin(ph),
                    -v_app - vc*Math.sin(ph), 0, vc*Math.cos(ph),
                    t === T.STAR ? 4000+rng()*18000 : 0);
            }
            this._boxSize = 400;
            this._dt = 0.002;
        } else {
            // Default: cosmic web (~800 DM + some gas)
            for (let i = 0; i < 700; i++) {
                const x = (rng()-0.5)*800;
                const y = (rng()-0.5)*800;
                const z = (rng()-0.5)*800;
                const kx = 2*Math.PI/400;
                this.addBody(T.DARK_MATTER, 5e5, x, y, z,
                    -0.05*kx*Math.sin(kx*x)*(1+0.5*Math.cos(kx*y)),
                    -0.05*kx*Math.sin(kx*y)*(1+0.5*Math.cos(kx*z)),
                    -0.05*kx*Math.sin(kx*z)*(1+0.5*Math.cos(kx*x)));
            }
            for (let i = 0; i < 100; i++) {
                const x = (rng()-0.5)*800;
                const y = (rng()-0.5)*800;
                const z = (rng()-0.5)*800;
                this.addBody(T.GAS, 5e4, x, y, z, 0, 0, 0, 1e4);
            }
            this._boxSize = 800;
            this._dt = 0.003;
        }

        this._softening = this._boxSize * G_N; // Softening ~ G_N * box_size
    }

    tick() {
        const G = G_N; // From ontic chain: 1/(b_3+N_c)^2 = 0.01
        const n = this._bodies.length;
        if (n === 0) return;

        // Reset accelerations
        for (const b of this._bodies) { b.ax = 0; b.ay = 0; b.az = 0; }

        // Direct O(N^2) gravity
        for (let i = 0; i < n; i++) {
            const bi = this._bodies[i];
            for (let j = i + 1; j < n; j++) {
                const bj = this._bodies[j];
                const dx = bj.x - bi.x;
                const dy = bj.y - bi.y;
                const dz = bj.z - bi.z;
                const r2 = dx*dx + dy*dy + dz*dz + this._softening*this._softening;
                const r = Math.sqrt(r2);
                const fmag = G / r2;
                const fx = fmag * dx / r;
                const fy = fmag * dy / r;
                const fz = fmag * dz / r;
                bi.ax += fx * bj.mass; bi.ay += fy * bj.mass; bi.az += fz * bj.mass;
                bj.ax -= fx * bi.mass; bj.ay -= fy * bi.mass; bj.az -= fz * bi.mass;
            }
        }

        // Velocity Verlet integration
        for (const b of this._bodies) {
            b.vx += 0.5 * this._dt * b.ax;
            b.vy += 0.5 * this._dt * b.ay;
            b.vz += 0.5 * this._dt * b.az;
            b.x += this._dt * b.vx;
            b.y += this._dt * b.vy;
            b.z += this._dt * b.vz;
            b.vx += 0.5 * this._dt * b.ax;
            b.vy += 0.5 * this._dt * b.ay;
            b.vz += 0.5 * this._dt * b.az;
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
            positions[i*3] = b.x;
            positions[i*3+1] = b.y;
            positions[i*3+2] = b.z;
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
