/** Preparation only. This seeded sampler sets a finite initial record; it never
 * advances physics and is not a registered ensemble or an equilibrium claim. */
import { HYDRO_VELOCITIES } from '../../../web/js/strict/fluid-observables.js';

export const HYDRO_IDENTITY = Object.freeze({
    law: 'phi-hydro-staged-candidate-1',
    table: 'abf25cf26072c03b5b7865fe84d3f31c270263d2c061e7bf3d81e27d783b5375',
    encoding: '3c10c134dadf3aa6f32f31ba588996e3c4755af67d4c804db567f5c4b361270c',
});
const preparations = {
    'hydro-shear-wave-t2': { label: 'Shear wave · T2', k: [1, 0, 0], polarization: [0, 1, 0], shear: true },
    'hydro-shear-wave-e': { label: 'Shear wave · E', k: [1, 1, 0], polarization: [Math.SQRT1_2, -Math.SQRT1_2, 0], shear: true },
    'hydro-sound-wave': { label: 'Longitudinal wave', k: [1, 0, 0], polarization: [1, 0, 0], shear: false },
    'hydro-taylor-green': { label: 'Taylor–Green pattern', k: [1, 1, 0], polarization: [Math.SQRT1_2, -Math.SQRT1_2, 0], shear: false },
    'hydro-shear-layer': { label: 'Periodic shear layer', k: [0, 1, 0], polarization: [1, 0, 0], shear: false },
    'hydro-vortex-pair': { label: 'Counter-rotating pattern', k: [1, 1, 0], polarization: [Math.SQRT1_2, -Math.SQRT1_2, 0], shear: false },
};
export const PREPARATIONS = Object.freeze(Object.fromEntries(Object.entries(preparations).map(([key, value]) =>
    [key, Object.freeze({ ...value, k: Object.freeze(value.k), polarization: Object.freeze(value.polarization) })])));
function base64(bytes) {
    let s = '';
    for (let i = 0; i < bytes.length; i += 8192) s += String.fromCharCode(...bytes.subarray(i, i + 8192));
    return btoa(s);
}
export function prepareFluid({ L = 16, preparation = 'hydro-shear-wave-t2', seed = 1729, density = 0.25 } = {}) {
    if (![8, 16, 32].includes(L) || !Object.hasOwn(PREPARATIONS, preparation)
        || !Number.isInteger(seed) || seed < 1 || seed > 0xffffffff
        || !Number.isFinite(density) || density < 0 || density > 0.5) throw new Error('Invalid interactive preparation');
    let state = seed >>> 0;
    const random = () => { state ^= state << 13; state ^= state >>> 17; state ^= state << 5; return (state >>> 0) / 4294967296; };
    const sites = L ** 3, bank = new Uint8Array(sites * 192), a = 0.08;
    const config = PREPARATIONS[preparation];
    for (let x = 0; x < L; x++) for (let y = 0; y < L; y++) for (let z = 0; z < L; z++) {
        const X = 2 * Math.PI * x / L, Y = 2 * Math.PI * y / L;
        let u;
        if (preparation === 'hydro-taylor-green') u = [a * Math.sin(X) * Math.cos(Y), -a * Math.cos(X) * Math.sin(Y), 0];
        else if (preparation === 'hydro-shear-layer') u = [a * Math.tanh(4 * Math.sin(Y)), 0.02 * Math.sin(X), 0];
        else if (preparation === 'hydro-vortex-pair') u = [a * Math.sin(X) * Math.cos(Y), -a * Math.cos(X) * Math.sin(Y) / 2, 0];
        else u = config.polarization.map(v => a * v * Math.sin(config.k[0] * X + config.k[1] * Y));
        const offset = ((x * L + y) * L + z) * 192;
        HYDRO_VELOCITIES.forEach((v, c) => {
            // Sum_c v_i v_j=12 delta_ij, rho=24d: expected P/rho=u.
            const p = density * (1 + 2 * v.reduce((s, value, j) => s + value * u[j], 0));
            if (p < 0 || p > 1) throw new Error('Preparation probability outside [0,1]; no clipping allowed');
            if (random() < p) bank[offset + 24 * Math.floor(random() * 4) + c] = 1;
        });
    }
    const arrays = { s: new Uint8Array(sites), bank, sc: new Uint8Array(6 * sites).fill(7),
        fcc: new Uint8Array(12 * sites).fill(7), admitted_sc: new Uint8Array(3 * sites),
        admitted_fcc: new Uint8Array(6 * sites), gate_sc: new Uint8Array(3 * sites), gate_fcc: new Uint8Array(6 * sites) };
    const checkpoint = JSON.stringify({ schema: 'ftd-hydro-checkpoint-2', ...HYDRO_IDENTITY,
        boundary: 'periodic', L, microtick: '0', arrays: Object.fromEntries(Object.entries(arrays).map(([k, v]) => [k, base64(v)])) });
    return { checkpoint, config: { ...config, L, preparation, seed, density, amplitude: a,
        sampler: 'xorshift32-v1', status: 'interactive_preparation', occupied_polarity: 0,
        relation_sector: 'frozen occupied', probability: 'd*(1+2*u.v)',
        kSquared: (2 * Math.PI / L) ** 2 * config.k.reduce((s, v) => s + v * v, 0) } };
}
