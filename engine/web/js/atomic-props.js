import { maxBonds as elemMaxBonds } from './elements.js';
import {
    electronConfig,
    slaterZeff,
} from './orbitals/quantum-chemistry.js';
import {
    N_BASE,
    R_BOHR,
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
    NEUTRON_PROTON_MASS_RATIO,
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

const CLOSED_SHELL_Z = new Set([2, 10, 18, 36, 54, 86, 118]);
const OPENING_SHELL_Z = new Set([1, 3, 11, 19, 37, 55, 87]);

function subshellCapacity(l) {
    return l === 0 ? 2 : l === 1 ? 6 : l === 2 ? 10 : l === 3 ? 14 : 0;
}

function closureRegime(Z) {
    if (Z <= 0) return 'invalid';
    if (Z >= 57 && Z <= 70) return 'lanthanide';
    if (Z >= 89 && Z <= 102) return 'actinide';
    if ((Z >= 21 && Z <= 30) || (Z >= 39 && Z <= 48) ||
        (Z >= 72 && Z <= 80) || (Z >= 104 && Z <= 112)) {
        return 'transition-block';
    }
    if (CLOSED_SHELL_Z.has(Z)) return 'shell-closed';
    if (OPENING_SHELL_Z.has(Z)) return 'shell-opening';
    return 'shell-active';
}

/**
 * Physics-facing atomic scale vector.
 *
 * This is the browser mirror of ftd::AtomicClosureContext. It does not replace
 * computeAtomicProps().radius, which remains the simulation-tuned LJ scale.
 * Slater screening is an [IMPOSED] empirical rule; r_cloud = R_BOHR*n^2/Z_eff
 * is a parametric shell-context estimate, not an FTD derivation of atomic radii.
 */
export function computeAtomicClosureContext(Z, {
    latticeSpacing = 1.0,
    boxExtent = 0.0,
    tauReference = 1.0,
} = {}) {
    const context = {
        Z,
        electron_count: Z > 0 ? Math.min(Z, 118) : 0,
        n_shell: 0,
        target_l: 0,
        valence_electrons: 0,
        active_electrons: 0,
        active_capacity: 0,
        source_loading: Math.max(0, Z),
        shielding: 0,
        z_eff: 0,
        shell_fill_fraction: 0,
        r_cloud: 0,
        delta_valence: 0,
        xi_orbital: 0,
        tau_electronic: 0,
        kappa: 0,
        zeta: 0,
        beta: 0,
        xi_ratio: 0,
        theta: 0,
        heavy_corrections_likely: Z >= 55,
        regime: closureRegime(Z),
    };

    if (Z <= 0) return context;

    const config = electronConfig(Math.min(Z, 118)).filter(sub => sub.count > 0);
    if (config.length === 0) return context;

    context.n_shell = Math.max(...config.map(sub => sub.n));
    for (const sub of config) {
        if (sub.n === context.n_shell) {
            context.valence_electrons += sub.count;
            context.active_electrons += sub.count;
            context.active_capacity += subshellCapacity(sub.l);
            context.target_l = Math.max(context.target_l, sub.l);
        }
    }

    for (const sub of config) {
        if (sub.l >= 2 && sub.n < context.n_shell && sub.n >= context.n_shell - 2) {
            context.active_electrons += sub.count;
            context.active_capacity += subshellCapacity(sub.l);
        }
    }

    if (context.active_capacity > 0) {
        context.shell_fill_fraction = context.active_electrons / context.active_capacity;
    }

    context.z_eff = slaterZeff(Math.min(Z, 118), context.n_shell, context.target_l);
    context.shielding = Math.max(0, Z - context.z_eff);

    const n = Math.max(1, context.n_shell);
    context.r_cloud = R_BOHR * n * n / context.z_eff;
    context.delta_valence = context.r_cloud / n;
    context.xi_orbital = Math.max(
        context.delta_valence,
        context.r_cloud / Math.sqrt(Math.max(1, context.active_electrons))
    );
    context.tau_electronic = 2.0 * Math.PI * n * n * n / (context.z_eff * context.z_eff);

    const a = latticeSpacing > 0 ? latticeSpacing : 1.0;
    context.kappa = context.r_cloud / a;
    context.zeta = boxExtent > 0 ? context.r_cloud / boxExtent : 0;
    context.beta = context.r_cloud > 0 ? context.delta_valence / context.r_cloud : 0;
    context.xi_ratio = context.r_cloud > 0 ? context.xi_orbital / context.r_cloud : 0;
    context.theta = tauReference > 0 ? context.tau_electronic / tauReference : 0;

    return context;
}

export function computeAtomicProps(Z, N = 0) {
    // Mass in PROTON-MASS units: Z protons + N neutrons, with the
    // neutron/proton ratio derived from the canonical PDG masses
    // (≈1.0013784 — replaces the hand-rounded 1.001, 2026-06-10).
    const mass = Z + N * NEUTRON_PROTON_MASS_RATIO;
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
        // [IMPOSED] Pauling values not tabulated for Z > 86; fall back to
        // this logarithmic guess. It is NOT from Pauling or any literature
        // source — an uncited interpolation kept only because superheavy
        // elements have no measured χ. Keep the formula explicit so future
        // audits can spot it.
        electronegativity = 1.5 + 0.3 * Math.log(Z);
    }
    const closure_context = computeAtomicClosureContext(Z);

    const alpha_pol = 4.0 * Math.PI * Math.pow(closure_context.r_cloud, 3.0);
    const n = Math.max(1, closure_context.n_shell);
    const e_ion = Math.pow(closure_context.z_eff / n, 2.0);
    const e_aff = electronegativity * 0.5;
    const sigma_scatter = Math.PI * vdw_sigma * vdw_sigma;

    return { mass, radius, vdw_epsilon, vdw_sigma, max_bonds, electronegativity, closure_context, alpha_pol, e_ion, e_aff, sigma_scatter };
}
