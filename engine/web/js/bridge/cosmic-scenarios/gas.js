/**
 * Cosmic scale-5 scenarios — gas laboratories on Monaghan SPH.
 *
 * Invoked via `.call(bridgeInstance, ctx)`. See galaxies.js for the binding
 * contract. Context: { T, rng, randn, PI2 }.
 *
 * These three scenarios exist to exercise the Monaghan SPH gas solver
 * (Task 4, `sph_monaghan` toggle; ../cosmic-sph.js) with clean, readable
 * setups: a self-similar collapsing ball, a head-on cloud collision, and a
 * rotating disk. Each ends by turning the toggle on and the legacy ad-hoc
 * gas repulsion off, so the SPH pressure/viscosity/energy equation is the
 * only gas force in play.
 *
 * Epistemic status: [IMPOSED effective gas dynamics]. Monaghan SPH is a
 * standard, imported numerical method for the Euler/Navier–Stokes gas
 * equations; nothing here is a substrate-native FTD derivation. See the
 * "Gas profile" card title in the panel-resources template and the toolbar
 * checkbox tooltip for the same tag.
 */

import { G_N } from '../../constants.js';

// Rejection-sample a point uniformly inside a ball of radius R, centred at
// the origin. Matches the style of exotic.js's uniform-ball sampling but
// uses ctx.rng directly (no randn) per the gas-lab brief.
function _sampleUniformBall(rng, R) {
    let x, y, z;
    do {
        x = (rng() * 2 - 1) * R;
        y = (rng() * 2 - 1) * R;
        z = (rng() * 2 - 1) * R;
    } while (x * x + y * y + z * z > R * R);
    return [x, y, z];
}

/**
 * Gas Collapse — 400 GAS bodies uniform in a ball of radius 30, at rest,
 * mass 1 each. Self-gravity plus SPH pressure/viscosity should resist (or
 * slow) collapse; the radial density/velocity profile card shows whether
 * the ball is contracting, static, or rebounding.
 */
export function setupGasCollapse(ctx) {
    const { T, rng } = ctx;
    const R = 30;
    const N = 400;
    for (let i = 0; i < N; i++) {
        const [x, y, z] = _sampleUniformBall(rng, R);
        this.addBody(T.GAS, 1, x, y, z, 0, 0, 0, 20);
    }
    this._boxSize = 80;
    this._softening = 1.5;
    this._dt = 0.02;
    this._toggles.sph_monaghan = true;
    this._enableSubgrid = false;
}

/**
 * Gas Cloud Collision — two balls of 250 GAS bodies (radius 12) centred at
 * x = -25 and x = +25, driven toward each other along x at speed 0.35.
 * The x-axis profile card shows the shock/mixing structure as the clouds
 * interpenetrate.
 */
export function setupGasCloudCollision(ctx) {
    const { T, rng } = ctx;
    const R = 12;
    const N = 250;
    const placeBall = (cx, vx) => {
        for (let i = 0; i < N; i++) {
            const [x, y, z] = _sampleUniformBall(rng, R);
            this.addBody(T.GAS, 1, cx + x, y, z, vx, 0, 0, 20);
        }
    };
    placeBall(-25, 0.35);
    placeBall(25, -0.35);
    this._boxSize = 90;
    this._softening = 1.5;
    this._dt = 0.02;
    this._toggles.sph_monaghan = true;
    this._enableSubgrid = false;
}

/**
 * Gas Rotating Disk — 500 GAS bodies in a thin disk (radius 30, scale
 * height 1.5) in the x-z plane (matching galaxies.js's disk convention),
 * on circular orbits set from a Plummer enclosed-mass profile via
 * `this._enclosedMass`. The radial (in-plane) profile card should show a
 * roughly flat density ring settling under SPH pressure support.
 */
export function setupGasRotatingDisk(ctx) {
    const { T, rng, randn, PI2 } = ctx;
    const N = 500;
    const R_disk = 30;
    const H = 1.5;
    const M_total = N; // mass 1 each
    const r_s = 15;
    for (let i = 0; i < N; i++) {
        const r = Math.sqrt(rng()) * R_disk; // uniform-area sampling
        const ph = PI2 * rng();
        const y = randn() * H;
        const M_enc = this._enclosedMass(r, M_total, r_s);
        const vc = Math.sqrt(G_N * M_enc / Math.max(r, 0.5));
        this.addBody(T.GAS, 1,
            r * Math.cos(ph), y, r * Math.sin(ph),
            -vc * Math.sin(ph), 0, vc * Math.cos(ph),
            20);
    }
    this._boxSize = 80;
    this._softening = 1.5;
    this._dt = 0.02;
    this._toggles.sph_monaghan = true;
    this._enableSubgrid = false;
}
