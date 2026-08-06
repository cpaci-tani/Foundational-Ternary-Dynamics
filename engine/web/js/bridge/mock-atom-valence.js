/**
 * Valence / bond-order helpers for Scale-2 mock atom engine.
 * Extracted from mock-atom-engine.js (behavior-preserving move).
 */

import { maxBonds as elemMaxBonds } from '../elements.js';

/**
 * Main-group valence electron count (for VSEPR lone-pair geometry).
 * Transition metals fall back to elemMaxBonds(Z).
 */
export function valenceElectrons(Z) {
    const mainGroup = [
        0,
        1, 2,
        1, 2, 3, 4, 5, 6, 7, 8,
        1, 2, 3, 4, 5, 6, 7, 8,
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

/**
 * Standard covalent valence (target total bond order) by atomic number.
 *
 * Used by bond-order inference (audit P0-12/P0-13): an atom "wants" its
 * summed bond order to equal this value, so the residual valence above its
 * bond *count* drives promotion of single bonds to double/triple/aromatic.
 */
export const COVALENT_VALENCE = {
    1: 1,   // H
    2: 0,   // He (noble)
    5: 3,   // B
    6: 4,   // C
    7: 3,   // N
    8: 2,   // O
    9: 1,   // F
    10: 0,  // Ne (noble)
    11: 0,  // Na (ionic — bonds via electrostatics, not covalent order)
    15: 3,  // P
    16: 2,  // S
    17: 1,  // Cl
    18: 0,  // Ar (noble)
    35: 1,  // Br
};

export function covalentValence(Z) {
    if (Z in COVALENT_VALENCE) return COVALENT_VALENCE[Z];
    return elemMaxBonds(Z);
}

// Bond-order inference tuning (audit P0-12/P0-13).
export const AROMATIC_ORDER = 1.5;
export const MAX_BOND_ORDER = 3;
