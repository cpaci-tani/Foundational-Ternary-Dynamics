/**
 * Cosmic scale-5 scenarios — exotic / lifecycle family.
 *
 * Invoked via `.call(bridgeInstance, ctx)`. See galaxies.js for
 * the binding contract. Context: { T, rng, randn, PI2 }.
 */

import { G_N } from '../../constants.js';

export function setupStellarLifecycle(ctx) {
    const { T, rng, randn } = ctx;
    const M_cloud = 5000;
    const R_cloud = 50;
    const N_gas = 600;
    const N_dm = 250;

    for (let i = 0; i < N_dm; i++) {
        let rx, ry, rz, r2;
        do {
            rx = randn() * 0.5;
            ry = randn() * 0.5;
            rz = randn() * 0.5;
            r2 = rx * rx + ry * ry + rz * rz;
        } while (r2 > 1.0);
        const x = rx * R_cloud * 0.7;
        const y = ry * R_cloud * 0.7;
        const z = rz * R_cloud * 0.7;
        const sigma = 0.04 * Math.sqrt(G_N * M_cloud / R_cloud);
        this.addBody(T.DARK_MATTER, M_cloud * 0.25 / N_dm,
            x, y, z,
            sigma * randn(), sigma * randn(), sigma * randn());
    }

    for (let i = 0; i < N_gas; i++) {
        let rx, ry, rz, r2;
        do {
            rx = (rng() - 0.5) * 2;
            ry = (rng() - 0.5) * 2;
            rz = (rng() - 0.5) * 2;
            r2 = rx * rx + ry * ry + rz * rz;
        } while (r2 > 1.0);
        const x = rx * R_cloud;
        const y = ry * R_cloud;
        const z = rz * R_cloud;
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
    this._enableSubgrid = true;
    this._stellarEvolution = true;
    this._hawkingEvaporation = true;
}

export function setupFtdCollapse(ctx) {
    const { T, rng, randn } = ctx;
    const M_cloud = 3000;
    const R_cloud = 40;
    const N_gas = 500;
    const N_dm = 200;

    for (let i = 0; i < N_gas; i++) {
        let rx, ry, rz, r2;
        do {
            rx = (rng() - 0.5) * 2;
            ry = (rng() - 0.5) * 2;
            rz = (rng() - 0.5) * 2;
            r2 = rx * rx + ry * ry + rz * rz;
        } while (r2 > 1.0);
        const x = rx * R_cloud;
        const y = ry * R_cloud;
        const z = rz * R_cloud;
        const r = Math.sqrt(x * x + y * y + z * z) + 0.01;
        const v_infall = -0.1 * Math.sqrt(G_N * M_cloud / R_cloud);
        const ph = Math.atan2(z, x);
        const v_tang = 0.15 * Math.sqrt(G_N * M_cloud / R_cloud);
        const type = (i < N_gas * 0.2) ? T.STAR : ((i % 5 === 0) ? T.NEBULA : T.GAS);
        this.addBody(type, M_cloud * 0.8 / N_gas,
            x, y, z,
            v_infall * x / r + v_tang * (-Math.sin(ph)) * (Math.random() * 0.5 + 0.5),
            v_infall * y / r + randn() * v_tang * 0.3,
            v_infall * z / r + v_tang * (Math.cos(ph)) * (Math.random() * 0.5 + 0.5),
            (type === T.STAR) ? 4000 + rng() * 15000 : (100 + rng() * 200));
        if (type === T.NEBULA && this._bodies.length > 0) {
            this._bodies[this._bodies.length - 1].radius = 18.0 + rng() * 35.0;
        }
    }

    for (let i = 0; i < N_dm; i++) {
        let rx, ry, rz, r2;
        do {
            rx = randn() * 0.4;
            ry = randn() * 0.4;
            rz = randn() * 0.4;
            r2 = rx * rx + ry * ry + rz * rz;
        } while (r2 > 1.0);
        const x = rx * R_cloud * 0.5;
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
    this._enableSubgrid = true;
}

export function setupDarkMatterHalo(ctx) {
    const { T, rng, randn, PI2 } = ctx;
    const M_halo = 4000;
    const r_s = 50;
    const N_dm = 1500;
    const N_stars = 60;
    for (let i = 0; i < N_dm; i++) {
        const u = rng() * 0.95;
        const su = Math.sqrt(u);
        const r = Math.min(r_s * su / (1.0 - su), 180);
        const th = Math.acos(2 * rng() - 1);
        const ph = PI2 * rng();
        const M_enc = this._enclosedMass(r, M_halo, r_s);
        const sigma = Math.sqrt(G_N * M_enc / Math.max(r, 3)) * 0.75;
        this.addBody(T.DARK_MATTER, M_halo * 0.95 / N_dm,
            r * Math.sin(th) * Math.cos(ph),
            r * Math.sin(th) * Math.sin(ph),
            r * Math.cos(th),
            sigma * randn(), sigma * randn(), sigma * randn());
    }
    for (let i = 0; i < N_stars; i++) {
        const r = 15 + rng() * 80;
        const ph = PI2 * rng();
        const th = Math.acos(2 * rng() - 1);
        const M_enc = this._enclosedMass(r, M_halo, r_s);
        const vc = Math.sqrt(G_N * M_enc / Math.max(r, 1)) * 0.9;
        this.addBody(T.STAR, M_halo * 0.02 / N_stars,
            r * Math.sin(th) * Math.cos(ph),
            r * Math.sin(th) * Math.sin(ph),
            r * Math.cos(th),
            -vc * Math.sin(ph), 0, vc * Math.cos(ph),
            5500 + rng() * 4000);
    }
    this._boxSize = 250;
    this._softening = 6.0;
    this._dt = 0.05;
    this._enableSubgrid = false;
}

export function setupGravitationalWave(ctx) {
    const { T, rng, randn, PI2 } = ctx;
    const M_bh = 200;
    const orbR = 12;
    const vk = Math.sqrt(G_N * M_bh / (2 * orbR));
    this.addBody(T.BLACK_HOLE, M_bh, orbR, 0, 0, 0, 0, vk);
    this.addBody(T.BLACK_HOLE, M_bh, -orbR, 0, 0, 0, 0, -vk);
    for (let i = 0; i < 300; i++) {
        const r = 25 + rng() * 35;
        const ph = PI2 * rng();
        const zz = randn() * 1.2;
        const M_enc = 2 * M_bh;
        const vc = Math.sqrt(G_N * M_enc / r) * 0.95;
        this.addBody(T.GAS, 0.3,
            r * Math.cos(ph), zz, r * Math.sin(ph),
            -vc * Math.sin(ph), randn() * vc * 0.03, vc * Math.cos(ph),
            1e5 + rng() * 1e5);
    }
    this._gwEvents.push({ tick: 0, x: 0, y: 0, z: 0, amplitude: 1.0 });
    this._boxSize = 120;
    this._softening = 2.5;
    this._dt = 0.02;
    this._enableSubgrid = false;
}

export function setupBaryogenesis(ctx) {
    const { T, rng, randn, PI2 } = ctx;
    const shellR = 55;
    const phi_g = (1 + Math.sqrt(5)) / 2;
    const place = (count, type, tempBase) => {
        for (let i = 0; i < count; i++) {
            const t = (i + 0.5) / count;
            const inclination = Math.acos(1 - 2 * t);
            const azimuth = PI2 * i * phi_g;
            const r = shellR * (0.5 + 0.5 * rng());
            const x = r * Math.sin(inclination) * Math.cos(azimuth);
            const y = r * Math.sin(inclination) * Math.sin(azimuth);
            const z = r * Math.cos(inclination);
            const dist = Math.sqrt(x * x + y * y + z * z) + 0.01;
            const vin = -0.12 * Math.sqrt(G_N * 800 / shellR);
            this.addBody(type, 30,
                x, y, z,
                vin * x / dist, vin * y / dist, vin * z / dist,
                tempBase + rng() * 6000);
        }
    };
    place(8, T.STAR, 6500);
    place(6, T.NEBULA, 2500);
    for (let i = 0; i < 600; i++) {
        const r = shellR * 1.5 * rng();
        const th = Math.acos(2 * rng() - 1);
        const ph = PI2 * rng();
        this.addBody(T.DARK_MATTER, 1.5,
            r * Math.sin(th) * Math.cos(ph),
            r * Math.cos(th),
            r * Math.sin(th) * Math.sin(ph),
            randn() * 0.2, randn() * 0.2, randn() * 0.2);
    }
    this._boxSize = 160;
    this._softening = 4.0;
    this._dt = 0.04;
    this._enableSubgrid = false;
}

export function setupCosmicWebFallback(ctx) {
    const { T, rng } = ctx;
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
        this.addBody(T.GAS, 5, (rng() - 0.5) * 200, (rng() - 0.5) * 200, (rng() - 0.5) * 200, 0, 0, 0, 1e4);
    }
    this._boxSize = 200;
    this._softening = 6.0;
    this._dt = 0.08;
    this._enableSubgrid = false;
}
