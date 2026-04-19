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
} from './constants.js';

// ── Physical Constants (MeV) ─────────────────────────────────────────
// Aliased from constants.js to keep the SEMF code below unchanged.
const M_PROTON   = M_P_PHYS;
const M_NEUTRON  = M_N_PHYS;
const M_ELECTRON = M_E_PHYS;
// K_B here is the precise PDG electron mass (used as the natural-unit
// scale for the SEMF outputs), NOT the rounded 0.511 from constants.js.
// The two differ by ~2e-4 MeV, well below SEMF accuracy, but the local
// name keeps the output format deterministic across refactors.
const K_B        = M_ELECTRON;

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
        // FTD natural units (in units of K_B = m_e)
        massInKB: massEnergy / K_B,
        bindingInKB: B / K_B,
    };
}

/**
 * Approximate total electron binding energy using a simplified
 * Thomas-Fermi model: E_total ≈ -15.73 × Z^(7/3) eV.
 * Returns a negative number (bound state).
 *
 * @param {number} Z — atomic number
 * @returns {number} total electron binding energy in eV (negative)
 */
function approxElectronBinding(Z) {
    if (Z <= 0) return 0;
    // Thomas-Fermi approximation for total electronic binding energy
    // E ≈ -15.73 × Z^(7/3) eV (within ~10% for most elements)
    return -15.73 * Math.pow(Z, 7 / 3);
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
        totalMassInKB: totalMass / K_B,               // FTD units
        totalBindingInKB: totalBinding / K_B,         // FTD units
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
