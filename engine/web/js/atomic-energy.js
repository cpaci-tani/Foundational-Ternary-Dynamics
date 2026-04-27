/**
 * Atomic Energy Calculator
 *
 * Computes physical atomic energies for all 118 elements:
 *   - Nuclear binding energy (Bethe-Weizsäcker semi-empirical mass formula)
 *   - Total rest mass energy (protons + neutrons + electrons - binding)
 *   - Electron binding energy (approximate from Slater's rules)
 *   - Binding energy per nucleon (classic B/A curve peaking at Fe-56)
 *
 * All energies in MeV unless noted otherwise.
 */

import { defaultNeutronCount } from './elements.js';
import {
    M_P_PHYS, M_N_PHYS, M_E_PHYS,
    SEMF_A_VOL, SEMF_A_SURF, SEMF_A_COUL, SEMF_A_ASYM, SEMF_A_PAIR,
    THOMAS_FERMI_PREFACTOR_EV,
} from './constants.js';

// ── Physical Constants (MeV) ─────────────────────────────────────────
// Aliased from constants.js to keep the SEMF code below unchanged.
// 2026-04-26 (Wave 2D): dropped the local `K_B = M_E_PHYS` shadow that
// previously confused which K_B was in use here. SEMF outputs are
// PDG-calibrated, so divisions use M_E_PHYS = 0.51099895 explicitly.
// Output keys `*InKB` retained for backward compatibility with
// scale2/controller.js, but they semantically mean "in units of the
// PDG electron mass" — not the framework K_B = 0.511 anchor.
const M_PROTON   = M_P_PHYS;
const M_NEUTRON  = M_N_PHYS;
const M_ELECTRON = M_E_PHYS;

// ── Bethe-Weizsacker Parameters (MeV) — aliased from constants.js ───
const A_VOL  = SEMF_A_VOL;
const A_SURF = SEMF_A_SURF;
const A_COUL = SEMF_A_COUL;
const A_ASYM = SEMF_A_ASYM;
const A_PAIR = SEMF_A_PAIR;

/**
 * Nuclear binding energy via the semi-empirical mass formula.
 * B > 0 means the nucleus is bound (energy released during formation).
 *
 * @param {number} Z — proton count
 * @param {number} N — neutron count
 * @returns {number} binding energy in MeV
 */
export function nuclearBindingEnergy(Z, N) {
    const A = Z + N;
    if (A <= 0) return 0;
    if (A === 1) return 0; // single nucleon has no binding

    const A13  = Math.pow(A, 1 / 3);
    const A23  = A13 * A13;

    // Volume: proportional to number of nucleons
    const vol = A_VOL * A;

    // Surface: nucleons at surface have fewer neighbors
    const surf = A_SURF * A23;

    // Coulomb: proton-proton electrostatic repulsion
    const coul = A_COUL * Z * (Z - 1) / A13;

    // Asymmetry: preference for N ≈ Z
    const asym = A_ASYM * (N - Z) * (N - Z) / (4 * A);

    // Pairing: even-even nuclei are more stable
    let delta = 0;
    const zEven = (Z % 2 === 0);
    const nEven = (N % 2 === 0);
    if (zEven && nEven) delta = A_PAIR / Math.sqrt(A);       // even-even
    else if (!zEven && !nEven) delta = -A_PAIR / Math.sqrt(A); // odd-odd
    // odd-A: delta = 0

    const B = vol - surf - coul - asym + delta;
    return Math.max(B, 0); // can't have negative binding for very light nuclei
}

/**
 * Total atomic rest mass energy.
 * M_atom = Z·m_p + N·m_n + Z·m_e - B(Z,N)
 *
 * @param {number} Z — atomic number
 * @returns {{ massEnergy, bindingEnergy, bindingPerNucleon, massDeficit, electronBinding, massNumber }}
 */
export function atomicEnergy(Z) {
    const N = defaultNeutronCount(Z);
    const A = Z + N;

    // Nuclear binding energy
    const B = nuclearBindingEnergy(Z, N);

    // Binding per nucleon
    const BA = A > 0 ? B / A : 0;

    // Free constituent mass
    const freeEnergy = Z * M_PROTON + N * M_NEUTRON + Z * M_ELECTRON;

    // Mass deficit (energy released during formation)
    const massDeficit = B;

    // Total atomic rest mass energy
    const massEnergy = freeEnergy - B;

    // Approximate electron binding energy using hydrogen-like scaling
    // E ≈ -13.6 eV × Z_eff² / n² summed over all electrons
    // This is a rough estimate; actual values need Hartree-Fock
    const electronBinding = approxElectronBinding(Z);

    return {
        massEnergy,        // total atomic mass-energy (MeV)
        bindingEnergy: B,  // nuclear binding energy (MeV)
        bindingPerNucleon: BA, // B/A (MeV/nucleon)
        massDeficit,       // mass deficit = B (MeV)
        electronBinding,   // total electron binding energy (eV, negative)
        massNumber: A,
        protons: Z,
        neutrons: N,
        // FTD natural units (in units of the PDG electron mass).
        // Key name retained from pre-2026-04-26 API; numerical value
        // computed against M_E_PHYS = 0.51099895 (precise PDG).
        massInKB: massEnergy / M_E_PHYS,
        bindingInKB: B / M_E_PHYS,
    };
}

/**
 * Approximate total electron binding energy using a simplified
 * Thomas-Fermi model: E_total ≈ -20.93 × Z^(7/3) eV.
 * Returns a negative number (bound state).
 *
 * Theme D1 fix (2026-04-26): the prefactor was previously hardcoded
 * at -15.73 eV·Z^(7/3) — that's a ~33% drift below the standard
 * derivation. The Thomas-Fermi result for total atomic binding is
 * E = -0.7687·Z^(7/3) Hartree = -20.93·Z^(7/3) eV (Hartree·27.2114
 * eV/Hartree). Now sourced from constants.js as
 * THOMAS_FERMI_PREFACTOR_EV. Magnitude shifts in the periodic-table
 * panel `electronBinding` column are expected and physically correct.
 *
 * @param {number} Z — atomic number
 * @returns {number} total electron binding energy in eV (negative)
 */
function approxElectronBinding(Z) {
    if (Z <= 0) return 0;
    return -THOMAS_FERMI_PREFACTOR_EV * Math.pow(Z, 7 / 3);
}

/**
 * Compute energy data for all 118 elements.
 * Returns a Map keyed by Z.
 */
export function allElementEnergies() {
    const map = new Map();
    for (let Z = 1; Z <= 118; Z++) {
        map.set(Z, atomicEnergy(Z));
    }
    return map;
}

/**
 * Compute the total energy of one of each element (Z=1..118).
 * Returns { totalMass, totalBinding, avgBindingPerNucleon, totalElectronBinding }.
 */
export function periodicTableTotalEnergy() {
    let totalMass = 0;
    let totalBinding = 0;
    let totalNucleons = 0;
    let totalElectronBinding = 0;

    for (let Z = 1; Z <= 118; Z++) {
        const e = atomicEnergy(Z);
        totalMass += e.massEnergy;
        totalBinding += e.bindingEnergy;
        totalNucleons += e.massNumber;
        totalElectronBinding += e.electronBinding;
    }

    return {
        totalMass,                                    // MeV
        totalBinding,                                 // MeV
        avgBindingPerNucleon: totalBinding / totalNucleons, // MeV
        totalElectronBinding,                         // eV
        // PDG electron-mass natural units (key name retained for API
        // compatibility; see atomicEnergy() docstring re: shadow drop).
        totalMassInKB: totalMass / M_E_PHYS,
        totalBindingInKB: totalBinding / M_E_PHYS,
    };
}

/**
 * Format energy value for display.
 */
export function formatEnergy(mev) {
    if (Math.abs(mev) >= 1e6) return (mev / 1e6).toFixed(3) + ' TeV';
    if (Math.abs(mev) >= 1e3) return (mev / 1e3).toFixed(3) + ' GeV';
    if (Math.abs(mev) >= 1) return mev.toFixed(3) + ' MeV';
    if (Math.abs(mev) >= 1e-3) return (mev * 1e3).toFixed(3) + ' keV';
    return (mev * 1e6).toFixed(3) + ' eV';
}
