/**
 * Cosmic scale-5 scenarios — galaxy-family.
 *
 * Each setup function is invoked via `.call(bridgeInstance, ctx)` so
 * `this` binds to the CosmicMockBridge. They mutate `this._bodies`,
 * `this._boxSize`, `this._softening`, `this._dt`, `this._enableSubgrid`,
 * `this._stellarEvolution`, `this._hawkingEvaporation`, and `this._gwEvents`.
 *
 * `ctx` carries precomputed helpers: { T, rng, randn, PI2 }.
 *
 * Pure data-generation: only `addBody(...)` writes occur here. No physics
 * tick or force computation happens in these functions.
 */

import { G_N } from '../../constants.js';

export function setupCosmicGalaxy(ctx) {
    const { T, rng, randn, PI2 } = ctx;
    const M_total = 7000;
    const M_bh = 100;
    const M_dm = (M_total - M_bh) * 0.85;
    const M_disk = (M_total - M_bh) * 0.15;
    const r_s = 40;
    const r_disk = 90;

    this.addBody(T.BLACK_HOLE, M_bh, 0, 0, 0);

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

    const N_stars = 700;
    for (let i = 0; i < N_stars; i++) {
        const u = rng();
        const r = 5 + (1 - Math.pow(1 - u, 2)) * r_disk;
        const armOffset = (i % 6) * (Math.PI / 3);
        const winding = -0.5 * Math.log(r + 1);
        const dispersion = randn() * 0.35;
        const ph = armOffset + winding + dispersion;
        const z_scale = 1.0 + r * 0.02;
        const zz = randn() * z_scale;
        const M_enc = M_bh + this._enclosedMass(r, M_dm, r_s) + this._enclosedMass(r, M_disk, r_disk * 0.4);
        const vc = Math.sqrt(G_N * M_enc / Math.max(r, 0.5));
        const vz = randn() * vc * 0.08;
        this.addBody(T.STAR, M_disk * 0.6 / N_stars,
            r * Math.cos(ph), zz, r * Math.sin(ph),
            -vc * Math.sin(ph), vz, vc * Math.cos(ph),
            3000 + rng() * 25000);
    }

    const N_gas = 250;
    for (let i = 0; i < N_gas; i++) {
        const u = rng();
        const r = 8 + (1 - Math.pow(1 - u, 2)) * r_disk * 1.1;
        const armOffset = (i % 6) * (Math.PI / 3);
        const winding = -0.5 * Math.log(r + 1);
        const dispersion = randn() * 0.25;
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

    const N_dust = 600;
    for (let i = 0; i < N_dust; i++) {
        const u = rng();
        const r = 3 + (1 - Math.pow(1 - u, 1.5)) * r_disk * 0.9;
        const armOffset = (i % 6) * (Math.PI / 3);
        const winding = -0.7 * Math.log(r + 1);
        const dispersion = randn() * 0.45;
        const ph = armOffset + winding + dispersion;
        const zz = randn() * (0.5 + r * 0.02);
        const M_enc = M_bh + this._enclosedMass(r, M_dm, r_s) + this._enclosedMass(r, M_disk, r_disk * 0.4);
        const vc = Math.sqrt(G_N * M_enc / Math.max(r, 0.5)) * 0.95;
        const bodyIndex = this._bodies.length;
        this.addBody(T.NEBULA, M_disk * 0.15 / N_dust,
            r * Math.cos(ph), zz, r * Math.sin(ph),
            -vc * Math.sin(ph), randn() * vc * 0.02, vc * Math.cos(ph),
            1500 + rng() * 8000);
        if (this._bodies[bodyIndex]) {
            const shredFactor = Math.min(1.0, r / (r_disk * 0.3));
            this._bodies[bodyIndex].radius = 15.0 + (rng() * 40.0) * shredFactor;
        }
    }

    const N_wisps = 50;
    for (let i = 0; i < N_wisps; i++) {
        const u = rng();
        const r = 3 + (1 - Math.pow(1 - u, 3)) * (r_disk * 0.6);
        const ph = PI2 * rng();
        const zz = randn() * 1.5;
        const M_enc = M_bh + this._enclosedMass(r, M_dm, r_s) + this._enclosedMass(r, M_disk, r_disk * 0.4);
        const vc = Math.sqrt(G_N * M_enc / Math.max(r, 0.5)) * 0.7;
        this.addBody(T.GAS, M_disk * 0.05 / N_wisps,
            r * Math.cos(ph), zz, r * Math.sin(ph),
            -vc * Math.sin(ph) - vc * 0.2 * Math.cos(ph),
            randn() * vc * 0.02,
            vc * Math.cos(ph) - vc * 0.2 * Math.sin(ph),
            35000 + rng() * 15000);
    }

    this._boxSize = 250;
    this._softening = 6.0;
    this._dt = 0.05;
    this._enableSubgrid = false;
}

export function setupCartwheelCollision(ctx) {
    const { T, rng, randn, PI2 } = ctx;
    const M_target = 6000;
    const M_bullet = 1500;
    const target_r_disk = 90;
    const target_r_s = 35;

    this.addBody(T.BLACK_HOLE, M_target * 0.05, 0, 0, 0, 0, 0, 0);

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

    const N_tstar = 700;
    for (let i = 0; i < N_tstar; i++) {
        const u = rng();
        const r = 4 + (1 - Math.pow(1 - u, 2)) * target_r_disk;
        const ph = PI2 * rng();
        const zz = randn() * (1.0 + r * 0.02);
        const M_enc = M_target * 0.05 + this._enclosedMass(r, M_target * 0.95, target_r_s);
        const vc = Math.sqrt(G_N * M_enc / Math.max(r, 0.5));
        const isNebula = (i % 4 === 0);
        const type = isNebula ? T.NEBULA : T.STAR;
        const mass = (M_target * 0.55) / N_tstar;
        const bodyIndex = this._bodies.length;
        this.addBody(type, mass,
            r * Math.cos(ph), zz, r * Math.sin(ph),
            -vc * Math.sin(ph), randn() * vc * 0.05, vc * Math.cos(ph),
            isNebula ? (1000 + rng() * 5000) : (4000 + rng() * 15000));
        if (isNebula && this._bodies[bodyIndex]) {
            const shredFactor = Math.min(1.0, r / (target_r_disk * 0.3));
            this._bodies[bodyIndex].radius = 12.0 + (rng() * 30.0) * shredFactor;
        }
    }

    const b_y0 = -130;
    const b_vy = Math.sqrt(2 * G_N * M_target / Math.abs(b_y0)) * 1.5;
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
            -vc * Math.sin(th) * Math.cos(ph), b_vy + randn() * vc * 0.1, vc * Math.sin(th) * Math.sin(ph),
            8000 + rng() * 10000);
    }

    this._boxSize = 250;
    this._softening = 4.0;
    this._dt = 0.035;
    this._enableSubgrid = false;
}

export function setupBinaryAGN(ctx) {
    const { T, rng, randn, PI2 } = ctx;
    const M_bh = 3000;
    const sep = 30;
    const v_orb = Math.sqrt(G_N * M_bh / (sep * 2));
    this.addBody(T.BLACK_HOLE, M_bh, -sep / 2, 0, 0, 0, 0, -v_orb);
    this.addBody(T.BLACK_HOLE, M_bh, sep / 2, 0, 0, 0, 0, v_orb);

    const N_disk = 1500;
    for (let i = 0; i < N_disk; i++) {
        const u = rng();
        const r = sep * 1.2 + (1 - Math.pow(1 - u, 2)) * 80;
        const ph = PI2 * rng();
        const H = r * 0.15;
        const zz = randn() * H;
        const M_enc = M_bh * 2;
        const vc = Math.sqrt(G_N * M_enc / r);
        const isNebula = (i % 6 === 0) && (r > sep * 2.0);
        const type = isNebula ? T.NEBULA : T.GAS;
        const bodyIndex = this._bodies.length;
        this.addBody(type, isNebula ? 2.0 : 0.5,
            r * Math.cos(ph), zz, r * Math.sin(ph),
            -vc * Math.sin(ph), randn() * vc * 0.05, vc * Math.cos(ph),
            isNebula ? 12000 : 8e5 * Math.pow(sep * 1.2 / r, 0.5));
        if (isNebula && this._bodies[bodyIndex]) {
            const shredFactor = Math.min(1.0, r / 60.0);
            this._bodies[bodyIndex].radius = 10.0 + (rng() * 20.0) * shredFactor;
        }
    }

    for (let i = 0; i < 200; i++) {
        const f = rng();
        const x = -sep / 2 + f * sep;
        const y = randn() * 1.5;
        const z = randn() * 1.5;
        this.addBody(T.GAS, 0.2, x, y, z, randn() * v_orb * 0.2, randn() * v_orb * 0.2, randn() * v_orb * 0.2, 5e6);
    }

    this._boxSize = 150;
    this._softening = 3.0;
    this._dt = 0.02;
    this._enableSubgrid = true;
}

export function setupGlobularCluster(ctx) {
    const { T, rng, randn, PI2 } = ctx;
    const M_cluster = 6000;
    const R_c = 15;
    const N_stars = 2000;
    for (let i = 0; i < N_stars; i++) {
        const u = rng();
        const r = R_c / Math.sqrt(Math.pow(1 - u, -2 / 3) - 1);
        const r_cap = Math.min(r, 120);
        const th = Math.acos(2 * rng() - 1);
        const ph = PI2 * rng();
        const M_enc = M_cluster * Math.pow(r_cap, 3) / Math.pow(r_cap * r_cap + R_c * R_c, 1.5);
        const v_circ = Math.sqrt(G_N * M_enc / Math.max(r_cap, 0.5));
        const vx = randn() * v_circ * 0.6;
        const vy = randn() * v_circ * 0.6;
        const vz = randn() * v_circ * 0.6;
        const mass = (M_cluster / N_stars) * (0.2 + rng() * 1.8);
        this.addBody(T.STAR, mass,
            r_cap * Math.sin(th) * Math.cos(ph), r_cap * Math.cos(th), r_cap * Math.sin(th) * Math.sin(ph),
            vx, vy, vz,
            3000 + rng() * 6000);
    }
    this._boxSize = 140;
    this._softening = 1.8;
    this._dt = 0.02;
    this._enableSubgrid = false;
}

export function setupCosmicWebScenario(ctx) {
    const { T, rng, randn, PI2 } = ctx;
    const M_node = 1500;
    const nodes = [
        { x: -120, y: 0, z: -80 },
        { x: 80, y: 30, z: -100 },
        { x: 0, y: -40, z: 120 },
        { x: -90, y: 60, z: 50 },
        { x: 140, y: -20, z: 40 }
    ];
    for (const n of nodes) {
        this.addBody(T.BLACK_HOLE, M_node * 0.05, n.x, n.y, n.z, 0, 0, 0);
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
                this._bodies[this._bodies.length - 1].radius = 15.0 + rng() * 25;
            }
        }
    }
    const filaments = [[0, 1], [0, 3], [1, 4], [2, 3], [2, 4], [3, 1]];
    for (const [i, j] of filaments) {
        const n1 = nodes[i];
        const n2 = nodes[j];
        const dx = n2.x - n1.x, dy = n2.y - n1.y, dz = n2.z - n1.z;
        const dist = Math.sqrt(dx * dx + dy * dy + dz * dz);
        const steps = Math.floor(dist * 2.5);
        for (let step = 0; step < steps; step++) {
            const t = step / steps;
            const cx = n1.x + t * dx;
            const cy = n1.y + t * dy;
            const cz = n1.z + t * dz;
            const thickness = 8 + 4 * Math.sin(t * Math.PI);
            const lx = randn() * thickness;
            const ly = randn() * thickness;
            const lz = randn() * thickness;
            const dir = (t < 0.5) ? -1 : 1;
            const v_flow = 0.8;
            const isGas = rng() < 0.3;
            const type = isGas ? T.NEBULA : T.DARK_MATTER;
            this.addBody(type, isGas ? 0.8 : 2.0,
                cx + lx, cy + ly, cz + lz,
                dir * (dx / dist) * v_flow, dir * (dy / dist) * v_flow, dir * (dz / dist) * v_flow,
                isGas ? 2000 : 0);
            if (isGas && this._bodies[this._bodies.length - 1]) {
                this._bodies[this._bodies.length - 1].radius = 10.0 + rng() * 15;
            }
        }
    }
    for (let i = 0; i < 400; i++) {
        const x = (rng() - 0.5) * 400;
        const y = (rng() - 0.5) * 400;
        const z = (rng() - 0.5) * 400;
        this.addBody(T.GAS, 0.4, x, y, z, randn() * 0.1, randn() * 0.1, randn() * 0.1, 100);
    }
    this._boxSize = 450;
    this._softening = 8.0;
    this._dt = 0.05;
    this._enableSubgrid = true;
}

export function setupBlackHoleScenario(ctx) {
    const { T, rng, randn, PI2 } = ctx;
    const M_bh = 500;
    this.addBody(T.BLACK_HOLE, M_bh, 0, 0, 0);
    for (let i = 0; i < 500; i++) {
        const u = rng();
        const r = 5 + u * u * 55;
        const ph = PI2 * rng();
        const H = r * (0.15 + 0.1 * (r / 60));
        const zz = randn() * H;
        const vk = Math.sqrt(G_N * M_bh / r);
        const v_factor = 0.99 - 0.01 * rng();
        const vz = randn() * vk * 0.05;
        this.addBody(T.GAS, 0.2,
            r * Math.cos(ph), zz, r * Math.sin(ph),
            -vk * v_factor * Math.sin(ph), vz, vk * v_factor * Math.cos(ph),
            1e6 * Math.pow(5 / r, 0.75));
    }
    const N_dust_bh = 150;
    for (let i = 0; i < N_dust_bh; i++) {
        const u = rng();
        const r = 15 + u * 60;
        const ph = PI2 * rng();
        const H = r * 0.25;
        const zz = randn() * H;
        const vk = Math.sqrt(G_N * M_bh / r);
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
}

export function setupMerger(ctx) {
    const { T, rng, randn, PI2 } = ctx;
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
}

export function setupSuperCluster(ctx) {
    const { T, rng, randn, PI2 } = ctx;
    const clusterRadius = 140;
    const galMass = 2500;
    const r_s = 20;
    const r_disk = 45;
    for (let gal = 0; gal < 3; gal++) {
        const angle = gal * (PI2 / 3);
        const cx = Math.cos(angle) * clusterRadius;
        const cz = Math.sin(angle) * clusterRadius;
        const orbitV = Math.sqrt(G_N * (galMass * 3) / clusterRadius) * 0.45;
        const vx_sys = -Math.sin(angle) * orbitV;
        const vz_sys = Math.cos(angle) * orbitV;
        this.addBody(T.BLACK_HOLE, galMass * 0.05, cx, 0, cz, vx_sys, 0, vz_sys);
        for (let i = 0; i < 350; i++) {
            const u = rng();
            const r = 4 + (1 - Math.pow(1 - u, 2)) * r_disk;
            const arm = (i % 6) * (Math.PI / 3);
            const winding = -0.6 * Math.log(r + 1);
            const ph = arm + winding + randn() * 0.3;
            const zz = randn() * (1.1 + r * 0.03);
            const M_enc = this._enclosedMass(r, galMass, r_s);
            const vc = Math.sqrt(G_N * M_enc / Math.max(r, 0.5));
            this.addBody(T.STAR, (galMass * 0.6) / 1000,
                cx + r * Math.cos(ph), zz, cz + r * Math.sin(ph),
                vx_sys - vc * Math.sin(ph), randn() * vc * 0.06, vz_sys + vc * Math.cos(ph),
                3000 + rng() * 25000);
        }
        const N_dust = 200;
        for (let i = 0; i < N_dust; i++) {
            const u = rng();
            const r = 4 + (1 - Math.pow(1 - u, 1.5)) * r_disk * 0.9;
            const arm = (i % 6) * (Math.PI / 3);
            const winding = -0.7 * Math.log(r + 1);
            const dispersion = randn() * 0.45;
            const ph = arm + winding + dispersion;
            const zz = randn() * (0.5 + r * 0.02);
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
        for (let i = 0; i < 150; i++) {
            const r = rng() * r_s * 2.5;
            const th = Math.acos(2 * rng() - 1);
            const ph = PI2 * rng();
            const M_enc = this._enclosedMass(r, galMass, r_s);
            const vc = Math.sqrt(G_N * M_enc / Math.max(r, 1));
            this.addBody(T.DARK_MATTER, (galMass * 0.35) / 500,
                cx + r * Math.sin(th) * Math.cos(ph), r * Math.cos(th), cz + r * Math.sin(th) * Math.sin(ph),
                vx_sys - vc * Math.sin(th) * Math.cos(ph), randn() * vc * 0.1, vz_sys + vc * Math.sin(th) * Math.sin(ph),
                0);
        }
    }
    this._boxSize = 450;
    this._softening = 4.5;
    this._dt = 0.05;
    this._enableSubgrid = false;
}
