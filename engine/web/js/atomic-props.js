import { defaultNeutronCount as elemNeutrons, maxBonds as elemMaxBonds } from './elements.js';
import {
    N_BASE,
    // Wave 2E (2026-04-26): all AE_* tuning constants now live in
    // constants.js as the single [IMPOSED] source of truth. Re-exported
    // from this module so existing consumers (mock-atom-engine.js,
    // scale2 controller, etc.) keep their imports unchanged.
    AE_EPS_BASE,
    AE_K_COULOMB,
    AE_K_BOND,
    AE_SPEED_MAX,
    AE_H_BOND_EPS,
    AE_K_ANGLE,
    AE_THERMOSTAT_TAU,
    PAULING_CHI,
    ATOMIC_RADII_PM,
} from './constants.js';

// ── Re-exports for back-compat ──────────────────────────────────────
// Pre-existing import names retained verbatim so mock-atom-engine.js
// (and any other consumers) need no edits. AE_CHI_TABLE is the alias
// for PAULING_CHI; the canonical name is PAULING_CHI in constants.js.
export {
    AE_EPS_BASE,
    AE_K_COULOMB,
    AE_K_BOND,
    AE_SPEED_MAX,
    AE_H_BOND_EPS,
    AE_K_ANGLE,
    AE_THERMOSTAT_TAU,
};
export const AE_CHI_TABLE = PAULING_CHI;

// ── Pauling atomic radius (physical, picometers) ────────────────────
// Theme D2 (2026-04-26): exposed for consumers that want experimentally
// grounded atomic radii. NOT yet wired into computeAtomicProps below —
// the simulation `radius` is a TUNED LJ parameter scale, not a physical
// lookup, and replacing it shifts every LJ sigma in the atom-engine
// (~4.5× for Li, ~5× for Cs vs the prior monotone 1/∛Z form). MD tuning
// recalibration should accompany the swap; tracked as Theme D2 follow-up.
//
// Returns picometers (PDG empirical) or 0 for Z without a table entry.
export function paulingRadiusPm(Z) {
    if (Z >= 0 && Z < ATOMIC_RADII_PM.length) {
        return ATOMIC_RADII_PM[Z] || 0;
    }
    return 0;
}

// ── Atom property helper (simulation units: Bohr-scaled) ──────────
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
    // Simulation-tuned radius (LJ scale, NOT physical). Theme D2
    // follow-up: swap to ATOMIC_RADII_PM-derived sim units once the MD
    // tuning is recalibrated; for now this monotone form preserves
    // existing molecular-dynamics behaviour.
    const z_cbrt = Math.cbrt(Z);
    const radius = z_cbrt > 0 ? 1.0 / z_cbrt : 1.0;
    const vdw_epsilon = AE_EPS_BASE * Math.pow(Z, 2.0 / 3.0);
    const vdw_sigma = radius * N_BASE;
    const max_bonds = elemMaxBonds(Z);
    // Theme D3 (2026-04-26): electronegativity fallback now reads the
    // full Pauling table from constants.js (Z=1..86) instead of the
    // drift-prone `1.5 + 0.3*log(Z)` formula that used to kick in for
    // Z>18 (it predicted χ(Cs)≈2.7 vs the empirical 0.79). Falls back
    // to the formula only for Z without a table entry, or when the
    // table value is 0 (noble gases — no Pauling χ defined).
    let electronegativity = 0;
    if (Z >= 1 && Z < PAULING_CHI.length) {
        const chi = PAULING_CHI[Z];
        if (chi > 0) {
            electronegativity = chi;
        }
    }
    if (electronegativity === 0 && Z > 86) {
        // Pauling values not tabulated for Z > 86; fall back to the
        // logarithmic guess. Keep the formula explicit so future audits
        // can spot it.
        electronegativity = 1.5 + 0.3 * Math.log(Z);
    }
    return { mass, radius, vdw_epsilon, vdw_sigma, max_bonds, electronegativity };
}
