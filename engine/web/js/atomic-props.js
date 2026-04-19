import { defaultNeutronCount as elemNeutrons, maxBonds as elemMaxBonds } from './elements.js';
import { N_BASE } from './constants.js';

// ── Atom property helper (simulation units: Bohr-scaled) ──────────
export const AE_EPS_BASE = 0.005;  // LJ well depth for Z=1 (tuned for visible dynamics)
export const AE_K_COULOMB = 2.0;    // Ionic coupling (qualitatively correct Coulomb >> vdW)
export const AE_K_BOND = 50.0;   // Bond spring stiffness multiplier
export const AE_SPEED_MAX = 10.0;   // Speed limit in simulation units
export const AE_H_BOND_EPS = 0.001;  // H-bond LJ 10-12 well depth (sim units; ~1/5 covalent)
export const AE_K_ANGLE = 0.05;   // VSEPR angle strain spring constant (sim units)
export const AE_THERMOSTAT_TAU = 10.0;   // Berendsen coupling timescale (in dt units)

// Pauling electronegativity table (Z=0..18)
export const AE_CHI_TABLE = [
    0, 2.20, 0, 0.98, 1.57, 2.04, 2.55, 3.04, 3.44, 3.98,
    0, 0.93, 1.31, 1.61, 1.90, 2.19, 2.58, 3.16, 0.0
];

export function valenceElectrons(Z) {
    const mainGroup = [
        0,                                    // Z=0 placeholder
        1, 2,                                // H, He
        1, 2, 3, 4, 5, 6, 7, 8,            // Li-Ne
        1, 2, 3, 4, 5, 6, 7, 8,            // Na-Ar
    ];
    if (Z <= 18) return mainGroup[Z] || 0;
    const col = [
        /*K*/ 1, /*Ca*/ 2,
        /*Sc-Zn (3d transition)*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*Ga*/ 3, /*Ge*/ 4, /*As*/ 5, /*Se*/ 6, /*Br*/ 7, /*Kr*/ 8
    ];
    const offset = (Z - 19) % 18;
    if (offset >= 0 && offset < col.length) {
        const v = col[offset];
        return v > 0 ? v : elemMaxBonds(Z);
    }
    return elemMaxBonds(Z);
}

export function computeAtomicProps(Z, N = 0) {
    const mass = Z + N * 1.001;
    const z_cbrt = Math.cbrt(Z);
    const radius = z_cbrt > 0 ? 1.0 / z_cbrt : 1.0;
    const vdw_epsilon = AE_EPS_BASE * Math.pow(Z, 2.0 / 3.0);
    const vdw_sigma = radius * N_BASE;
    const max_bonds = elemMaxBonds(Z);
    const electronegativity = (Z >= 1 && Z <= 18) ? AE_CHI_TABLE[Z]
                            : (Z > 18 ? 1.5 + 0.3 * Math.log(Z) : 0);
    return { mass, radius, vdw_epsilon, vdw_sigma, max_bonds, electronegativity };
}
