/**
 * Math helpers shared across pedagogy panels.
 *
 * Extracted from consciousness-pedagogy.js as part of ticket CP-3.
 */

import { G_STAR } from '../../constants.js';

export const GSTAR2 = G_STAR * G_STAR;
export const GSTAR3 = G_STAR * G_STAR * G_STAR;

/** Evaluate Q_k(x) = x^2 - k·G*²·x + k·G*³ */
export function Qk(k, x) {
    return x * x - k * GSTAR2 * x + k * GSTAR3;
}

/** Discriminant Delta_k = k·G*³·(k·G* - 4) */
export function discriminant(k) {
    return k * GSTAR3 * (k * G_STAR - 4);
}

/** Roots of Q_k when Delta >= 0: returns [x_minus, x_plus] */
export function realRoots(k) {
    const disc = discriminant(k);
    if (disc < 0) return null;
    const sqrtDisc = Math.sqrt(disc);
    const half = k * GSTAR2 / 2;
    return [half - sqrtDisc / 2, half + sqrtDisc / 2];
}
