/**
 * Particle Catalog: Standard Model particles with FTD context.
 *
 * `mass_mev` is the PHYSICAL (measured / PDG) mass — the value the Scale 1
 * engine simulates and the inspector/Zoo display. The electron uses the FTD
 * mass anchor K_B (≡ m_e ≡ 0.511 MeV); every other species uses the measured
 * PDG reference value from constants.js. This is the single source of truth
 * for particle mass across Scale 1 (scenarios, Zoo injection, inspector).
 *
 * `ftd_formula` + `ftd_accuracy` carry FTD's *prediction* for the mass and its
 * deviation from the measured value — they are NOT a substitute for mass_mev,
 * and they are NOT first-principles derivations. Read the FTD-formula column as
 * motivating expressions at their LEDGER status (see `ftd_status`). Several are
 * built on the [PARAMETRIC] sin²θ_W = 3/13 (demoted, FTD-0018), and the lepton
 * mass-ratios are integer-combination conjectures (FTD-0015/0016 family). Not
 * every mass "traces back through the ontic chain."
 *
 * ftd_status → LEDGER epistemic tag (this map copies status, never promotes it):
 *   'axiom'      = [AXIOM]
 *   'selection'  = [SELECTION] / [STRONGLY MOTIVATED CONJECTURE]
 *   'parametric' = [PARAMETRIC]
 *   'derived'    = [DERIVED] / [THEOREM]   (reserved; no SM mass currently qualifies)
 *
 * Categories:
 *   leptons      - e, μ, τ + neutrinos + antiparticles
 *   quarks       - u, d, s, c, b, t + antiquarks
 *   gauge_bosons - γ, W±, Z, g
 *   scalar       - Higgs
 *   baryons      - p, n, Λ, Σ, Ξ, Ω, Δ
 *   mesons       - π, K, η, ρ, J/ψ, Υ
 */

import {
    // M_E = electron mass = FTD anchor K_B (≡ m_e ≡ 0.511 MeV). M_MU_PHYS /
    // M_TAU_PHYS are the measured (PDG) μ / τ masses — the physical values the
    // engine simulates (F3 single-source-of-truth, 2026-06-15 audit). FTD's
    // predicted ratios (207·m_e, 3477·m_e) live in ftd_formula/ftd_accuracy.
    M_E, M_MU_PHYS, M_TAU_PHYS,
    M_P_PHYS, M_N_PHYS, M_SIGMA_PHYS, M_OMEGA_PHYS,
    M_PI_CH_PHYS, M_PI_0_PHYS, M_K_CH_PHYS, M_K_0_PHYS,
    M_DELTA_PHYS,
    // Wave 2B additions (2026-04-26): replace inline literals with
    // canonical constants. These all live in constants.js as
    // [PARAMETRIC PDG] reference values — used only for catalog
    // display, not for derivations.
    M_U_PHYS, M_D_PHYS, M_S_PHYS, M_C_PHYS, M_B_PHYS, M_T_PHYS,
    // Neutrino masses single-sourced from constants.js (2026-06-15): meV-scale
    // [PARAMETRIC PDG] upper bounds (M_NU_MU/TAU were corrected from a ×1e6
    // magnitude error — they had held keV-scale values mislabeled as MeV).
    M_NU_E_PHYS, M_NU_MU_PHYS, M_NU_TAU_PHYS,
    // Σ⁰/Σ⁻ isospin partners (Σ⁺ = M_SIGMA_PHYS above).
    M_SIGMA0_PHYS, M_SIGMA_MINUS_PHYS,
    M_W_PHYS, M_Z_PHYS, M_HIGGS_PHYS, M_HIGGS,
    M_LAMBDA_PHYS, M_XI_0_PHYS, M_XI_M_PHYS,
    M_ETA_PHYS, M_RHO_PHYS, M_J_PSI_PHYS, M_UPSILON_PHYS,
} from './constants.js';
import { formatMassCompat } from './units.js';

const PARTICLES = [
    // ═══════════════════════════════════════════════════════════════
    // LEPTONS (charged + neutrinos)
    // ═══════════════════════════════════════════════════════════════
    {
        id: 'electron', name: 'Electron', symbol: 'e⁻',
        category: 'leptons', generation: 1,
        mass_mev: M_E, charge: -1, spin: 0.5,   // K_B anchor (≡ measured m_e)
        color_charge: 'none', antiparticle: 'positron',
        ftd_formula: 'm_P·√(2π)·(16/3)·α¹¹',
        // FTD-0015 [STRONGLY MOTIVATED CONJECTURE] (only the n=11 exponent is
        // [DERIVED]); 'derived'→'selection' to match LEDGER. 2026-06-15 audit.
        // ftd_accuracy 0.19% = canonical m_e match (LEDGER FTD-0015 / CLAUDE.md).
        ftd_accuracy: 0.19, ftd_status: 'selection',
        display_color: [0.29, 0.87, 0.50], display_size: 4
    },
    {
        id: 'positron', name: 'Positron', symbol: 'e⁺',
        category: 'leptons', generation: 1,
        mass_mev: M_E, charge: 1, spin: 0.5,
        color_charge: 'none', antiparticle: 'electron',
        ftd_formula: 'm_e (same mass)',
        ftd_accuracy: 0.19, ftd_status: 'selection',  // CPT partner of e⁻ → same FTD-0015 status (0.19% canonical)
        display_color: [0.97, 0.44, 0.44], display_size: 4
    },
    {
        id: 'muon', name: 'Muon', symbol: 'μ⁻',
        category: 'leptons', generation: 2,
        mass_mev: M_MU_PHYS, charge: -1, spin: 0.5,   // physical (PDG); FTD 207·m_e ≈ 105.78 in ftd_formula
        color_charge: 'none', antiparticle: 'antimuon',
        ftd_formula: 'm_e·(3·b₃·(b₃+N_c)−N_c) = 207·m_e',
        // Integer-ratio conjecture (FTD-0015/0016 family); no axioms→mass
        // chain → [STRONGLY MOTIVATED CONJECTURE]. 'derived'→'selection'. 2026-06-15 audit.
        ftd_accuracy: 0.11, ftd_status: 'selection',
        display_color: [0.20, 0.73, 0.40], display_size: 5
    },
    {
        id: 'antimuon', name: 'Antimuon', symbol: 'μ⁺',
        category: 'leptons', generation: 2,
        mass_mev: M_MU_PHYS, charge: 1, spin: 0.5,
        color_charge: 'none', antiparticle: 'muon',
        ftd_formula: 'm_μ (same mass)',
        ftd_accuracy: 0.11, ftd_status: 'selection',
        display_color: [0.90, 0.35, 0.35], display_size: 5
    },
    {
        id: 'tau', name: 'Tau', symbol: 'τ⁻',
        category: 'leptons', generation: 3,
        mass_mev: M_TAU_PHYS, charge: -1, spin: 0.5,   // physical (PDG); FTD 3477·m_e ≈ 1776.7 in ftd_formula
        color_charge: 'none', antiparticle: 'antitau',
        ftd_formula: 'm_e·((N_eff+N_base)·207−2N_c·b₃) = 3477·m_e',
        ftd_accuracy: 0.007, ftd_status: 'selection',
        display_color: [0.12, 0.60, 0.32], display_size: 6
    },
    {
        id: 'antitau', name: 'Antitau', symbol: 'τ⁺',
        category: 'leptons', generation: 3,
        mass_mev: M_TAU_PHYS, charge: 1, spin: 0.5,
        color_charge: 'none', antiparticle: 'tau',
        ftd_formula: 'm_τ (same mass)',
        ftd_accuracy: 0.007, ftd_status: 'selection',
        display_color: [0.82, 0.28, 0.28], display_size: 6
    },
    {
        id: 'nu_e', name: 'Electron Neutrino', symbol: 'νₑ',
        category: 'leptons', generation: 1,
        // Single-sourced from constants.js: 4.1e-15 MeV = 4.1 neV.
        mass_mev: M_NU_E_PHYS, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'antinu_e',
        ftd_formula: 'm₃·(m_e/m_τ)² ≈ 4.1 neV',
        // Neutrino masses are NOT derivable from the current FTD chain
        // (constants.js: [PARAMETRIC PDG] bounds); ftd_formula is a motivating
        // match only. 'derived'→'parametric'. 2026-06-15 audit.
        ftd_accuracy: null, ftd_status: 'parametric',
        display_color: [0.70, 0.95, 0.80], display_size: 2
    },
    {
        id: 'antinu_e', name: 'Electron Antineutrino', symbol: 'ν̄ₑ',
        category: 'leptons', generation: 1,
        // Single-sourced from constants.js (2026-06-15): same as nu_e.
        mass_mev: M_NU_E_PHYS, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'nu_e',
        ftd_formula: 'm_ν₁ (same mass)',
        ftd_accuracy: null, ftd_status: 'parametric',
        display_color: [0.95, 0.80, 0.80], display_size: 2
    },
    {
        id: 'nu_mu', name: 'Muon Neutrino', symbol: 'νμ',
        category: 'leptons', generation: 2,
        mass_mev: M_NU_MU_PHYS, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'antinu_mu',
        ftd_formula: 'm₃·√N_c/(b₃+N_c) ≈ 8.6 meV',
        ftd_accuracy: null, ftd_status: 'parametric',
        display_color: [0.60, 0.90, 0.72], display_size: 2
    },
    {
        id: 'antinu_mu', name: 'Muon Antineutrino', symbol: 'ν̄μ',
        category: 'leptons', generation: 2,
        mass_mev: M_NU_MU_PHYS, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'nu_mu',
        ftd_formula: 'm_ν₂ (same mass)',
        ftd_accuracy: null, ftd_status: 'parametric',
        display_color: [0.90, 0.72, 0.72], display_size: 2
    },
    {
        id: 'nu_tau', name: 'Tau Neutrino', symbol: 'ντ',
        category: 'leptons', generation: 3,
        mass_mev: M_NU_TAU_PHYS, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'antinu_tau',
        ftd_formula: 'v·(N_base/N_c)·α⁶ ≈ 49.6 meV',
        ftd_accuracy: null, ftd_status: 'parametric',
        display_color: [0.50, 0.85, 0.65], display_size: 2
    },
    {
        id: 'antinu_tau', name: 'Tau Antineutrino', symbol: 'ν̄τ',
        category: 'leptons', generation: 3,
        mass_mev: M_NU_TAU_PHYS, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'nu_tau',
        ftd_formula: 'm_ν₃ (same mass)',
        ftd_accuracy: null, ftd_status: 'parametric',
        display_color: [0.85, 0.65, 0.65], display_size: 2
    },

    // ═══════════════════════════════════════════════════════════════
    // QUARKS
    // ═══════════════════════════════════════════════════════════════
    {
        id: 'up', name: 'Up Quark', symbol: 'u',
        category: 'quarks', generation: 1,
        mass_mev: M_U_PHYS, charge: 2/3, spin: 0.5,
        color_charge: 'r/g/b', antiparticle: 'anti_up',
        ftd_formula: 'm_e·N_base·sin²θ_W ≈ 2.16 MeV',
        ftd_accuracy: 5.0, ftd_status: 'selection',
        display_color: [1.00, 0.75, 0.30], display_size: 3
    },
    {
        id: 'anti_up', name: 'Anti-Up', symbol: 'ū',
        category: 'quarks', generation: 1,
        mass_mev: M_U_PHYS, charge: -2/3, spin: 0.5,
        color_charge: 'r̄/ḡ/b̄', antiparticle: 'up',
        ftd_formula: 'm_u (same mass)',
        ftd_accuracy: 5.0, ftd_status: 'selection',
        display_color: [0.70, 0.50, 0.20], display_size: 3
    },
    {
        id: 'down', name: 'Down Quark', symbol: 'd',
        category: 'quarks', generation: 1,
        mass_mev: M_D_PHYS, charge: -1/3, spin: 0.5,
        color_charge: 'r/g/b', antiparticle: 'anti_down',
        ftd_formula: 'm_e·(b₃+N_c−1)·sin²θ_W ≈ 4.67 MeV',
        ftd_accuracy: 3.0, ftd_status: 'selection',
        display_color: [0.95, 0.65, 0.25], display_size: 3
    },
    {
        id: 'anti_down', name: 'Anti-Down', symbol: 'd̄',
        category: 'quarks', generation: 1,
        mass_mev: M_D_PHYS, charge: 1/3, spin: 0.5,
        color_charge: 'r̄/ḡ/b̄', antiparticle: 'down',
        ftd_formula: 'm_d (same mass)',
        ftd_accuracy: 3.0, ftd_status: 'selection',
        display_color: [0.65, 0.45, 0.18], display_size: 3
    },
    {
        id: 'strange', name: 'Strange Quark', symbol: 's',
        category: 'quarks', generation: 2,
        mass_mev: M_S_PHYS, charge: -1/3, spin: 0.5,
        color_charge: 'r/g/b', antiparticle: 'anti_strange',
        ftd_formula: 'm_e·MU_RATIO·sin²θ_W·N_c/N_base',
        ftd_accuracy: 2.0, ftd_status: 'selection',
        display_color: [0.90, 0.55, 0.18], display_size: 4
    },
    {
        id: 'anti_strange', name: 'Anti-Strange', symbol: 's̄',
        category: 'quarks', generation: 2,
        mass_mev: M_S_PHYS, charge: 1/3, spin: 0.5,
        color_charge: 'r̄/ḡ/b̄', antiparticle: 'strange',
        ftd_formula: 'm_s (same mass)',
        ftd_accuracy: 2.0, ftd_status: 'selection',
        display_color: [0.60, 0.38, 0.12], display_size: 4
    },
    {
        id: 'charm', name: 'Charm Quark', symbol: 'c',
        category: 'quarks', generation: 2,
        mass_mev: M_C_PHYS, charge: 2/3, spin: 0.5,
        color_charge: 'r/g/b', antiparticle: 'anti_charm',
        ftd_formula: 'm_e·MU_RATIO·b₃·sin²θ_W/α',
        ftd_accuracy: 1.5, ftd_status: 'selection',
        display_color: [0.85, 0.50, 0.12], display_size: 5
    },
    {
        id: 'anti_charm', name: 'Anti-Charm', symbol: 'c̄',
        category: 'quarks', generation: 2,
        mass_mev: M_C_PHYS, charge: -2/3, spin: 0.5,
        color_charge: 'r̄/ḡ/b̄', antiparticle: 'charm',
        ftd_formula: 'm_c (same mass)',
        ftd_accuracy: 1.5, ftd_status: 'selection',
        display_color: [0.58, 0.35, 0.08], display_size: 5
    },
    {
        id: 'bottom', name: 'Bottom Quark', symbol: 'b',
        category: 'quarks', generation: 3,
        mass_mev: M_B_PHYS, charge: -1/3, spin: 0.5,
        color_charge: 'r/g/b', antiparticle: 'anti_bottom',
        ftd_formula: 'm_τ·N_c·sin²θ_W/α',
        ftd_accuracy: 1.0, ftd_status: 'selection',
        display_color: [0.80, 0.45, 0.08], display_size: 6
    },
    {
        id: 'anti_bottom', name: 'Anti-Bottom', symbol: 'b̄',
        category: 'quarks', generation: 3,
        mass_mev: M_B_PHYS, charge: 1/3, spin: 0.5,
        color_charge: 'r̄/ḡ/b̄', antiparticle: 'bottom',
        ftd_formula: 'm_b (same mass)',
        ftd_accuracy: 1.0, ftd_status: 'selection',
        display_color: [0.55, 0.32, 0.06], display_size: 6
    },
    {
        id: 'top', name: 'Top Quark', symbol: 't',
        category: 'quarks', generation: 3,
        mass_mev: M_T_PHYS, charge: 2/3, spin: 0.5,
        color_charge: 'r/g/b', antiparticle: 'anti_top',
        ftd_formula: 'v_Higgs/√2 ≈ 173 GeV',
        ftd_accuracy: 0.3, ftd_status: 'selection',
        display_color: [0.75, 0.40, 0.05], display_size: 8
    },
    {
        id: 'anti_top', name: 'Anti-Top', symbol: 't̄',
        category: 'quarks', generation: 3,
        mass_mev: M_T_PHYS, charge: -2/3, spin: 0.5,
        color_charge: 'r̄/ḡ/b̄', antiparticle: 'top',
        ftd_formula: 'm_t (same mass)',
        ftd_accuracy: 0.3, ftd_status: 'selection',
        display_color: [0.50, 0.28, 0.04], display_size: 8
    },

    // ═══════════════════════════════════════════════════════════════
    // GAUGE BOSONS
    // ═══════════════════════════════════════════════════════════════
    {
        id: 'photon', name: 'Photon', symbol: 'γ',
        category: 'gauge_bosons', generation: null,
        mass_mev: 0, charge: 0, spin: 1,
        color_charge: 'none', antiparticle: 'photon',
        ftd_formula: 'massless (flux wave, s=0)',
        ftd_accuracy: null, ftd_status: 'axiom',
        display_color: [0.95, 0.95, 0.40], display_size: 3
    },
    {
        id: 'gluon', name: 'Gluon', symbol: 'g',
        category: 'gauge_bosons', generation: null,
        mass_mev: 0, charge: 0, spin: 1,
        color_charge: 'octet', antiparticle: 'gluon',
        ftd_formula: 'massless (color flux mode)',
        ftd_accuracy: null, ftd_status: 'axiom',
        display_color: [0.40, 0.75, 0.95], display_size: 3
    },
    {
        id: 'w_plus', name: 'W⁺ Boson', symbol: 'W⁺',
        category: 'gauge_bosons', generation: null,
        mass_mev: M_W_PHYS, charge: 1, spin: 1,
        color_charge: 'none', antiparticle: 'w_minus',
        ftd_formula: 'm_e·67/(8α²) ≈ 80.4 GeV',
        // SPEC_SM_REPLACEMENT_COMPLETE.md row 10: M_W is [STRUCTURALLY
        // MOTIVATED PARAMETRIC] (depends on demoted SM-3 sin²θ_W = 3/13).
        // Retagged 'derived'→'parametric'. Audit Section C, 2026-05-27.
        ftd_accuracy: 0.02, ftd_status: 'parametric',
        display_color: [0.30, 0.60, 0.95], display_size: 7
    },
    {
        id: 'w_minus', name: 'W⁻ Boson', symbol: 'W⁻',
        category: 'gauge_bosons', generation: null,
        mass_mev: M_W_PHYS, charge: -1, spin: 1,
        color_charge: 'none', antiparticle: 'w_plus',
        ftd_formula: 'm_W (same mass)',
        // Mirrors W⁺ (same mass) → same status as M_W.
        ftd_accuracy: 0.02, ftd_status: 'parametric',
        display_color: [0.20, 0.50, 0.85], display_size: 7
    },
    {
        id: 'z_boson', name: 'Z Boson', symbol: 'Z⁰',
        category: 'gauge_bosons', generation: null,
        mass_mev: M_Z_PHYS, charge: 0, spin: 1,
        color_charge: 'none', antiparticle: 'z_boson',
        ftd_formula: 'm_W/cos(θ_W) ≈ 91.2 GeV',
        // SPEC_SM_REPLACEMENT_COMPLETE.md row 11: M_Z is [STRUCTURALLY
        // MOTIVATED PARAMETRIC] (M_W/cos θ_W; depends on demoted sin²θ_W).
        // Retagged 'derived'→'parametric'. Audit Section C, 2026-05-27.
        ftd_accuracy: 0.01, ftd_status: 'parametric',
        display_color: [0.25, 0.55, 0.90], display_size: 7
    },

    // ═══════════════════════════════════════════════════════════════
    // SCALAR BOSONS
    // ═══════════════════════════════════════════════════════════════
    {
        id: 'higgs', name: 'Higgs Boson', symbol: 'H⁰',
        category: 'scalar', generation: null,
        mass_mev: M_HIGGS_PHYS, charge: 0, spin: 0,
        color_charge: 'none', antiparticle: 'higgs',
        ftd_formula: `m_e·N_eff/α² ≈ ${M_HIGGS.toFixed(2)} GeV (−0.36%; excluded at PDG-2024 precision)`,
        ftd_accuracy: 0.36, ftd_status: 'parametric',
        display_color: [1.00, 0.84, 0.00], display_size: 8
    },

    // ═══════════════════════════════════════════════════════════════
    // BARYONS (composite: 3 quarks)
    // ═══════════════════════════════════════════════════════════════
    {
        id: 'proton', name: 'Proton', symbol: 'p',
        category: 'baryons', generation: null,
        mass_mev: M_P_PHYS, charge: 1, spin: 0.5,
        color_charge: 'singlet', antiparticle: 'antiproton',
        composition: 'uud',
        ftd_formula: 'm_e·(N_eff/α + N_base·N_eff + N_c) ≈ 1836.47·m_e',
        // LEDGER FTD-0016: m_p/m_e formula is [STRONGLY MOTIVATED CONJECTURE],
        // not a derivation (no axioms→m_p chain). Retagged 'derived'→'selection'
        // to match LEDGER. Audit Section C, 2026-05-27.
        ftd_accuracy: 0.017, ftd_status: 'selection',
        display_color: [0.95, 0.30, 0.30], display_size: 6
    },
    {
        id: 'antiproton', name: 'Antiproton', symbol: 'p̄',
        category: 'baryons', generation: null,
        mass_mev: M_P_PHYS, charge: -1, spin: 0.5,
        color_charge: 'singlet', antiparticle: 'proton',
        composition: 'ūūd̄',
        ftd_formula: 'm_p (same mass)',
        // Mirrors proton (CPT partner, same mass) → same status as m_p.
        ftd_accuracy: 0.017, ftd_status: 'selection',
        display_color: [0.30, 0.95, 0.95], display_size: 6
    },
    {
        id: 'neutron', name: 'Neutron', symbol: 'n',
        category: 'baryons', generation: null,
        mass_mev: M_N_PHYS, charge: 0, spin: 0.5,
        color_charge: 'singlet', antiparticle: 'antineutron',
        composition: 'udd',
        ftd_formula: 'm_p + (m_d−m_u)·(1+α/π)',
        // Built on m_p (FTD-0016 [STRONGLY MOTIVATED CONJECTURE]) plus a
        // quark-mass-difference + EM-correction insertion (PDG inputs).
        // Retagged 'derived'→'parametric'. Audit Section C, 2026-05-27.
        ftd_accuracy: 0.02, ftd_status: 'parametric',
        display_color: [0.70, 0.25, 0.55], display_size: 6
    },
    {
        id: 'antineutron', name: 'Antineutron', symbol: 'n̄',
        category: 'baryons', generation: null,
        mass_mev: M_N_PHYS, charge: 0, spin: 0.5,
        color_charge: 'singlet', antiparticle: 'neutron',
        composition: 'ūd̄d̄',
        ftd_formula: 'm_n (same mass)',
        // Mirrors neutron (CPT partner, same mass) → same status as m_n.
        ftd_accuracy: 0.02, ftd_status: 'parametric',
        display_color: [0.55, 0.20, 0.45], display_size: 6
    },
    {
        id: 'lambda', name: 'Lambda', symbol: 'Λ⁰',
        category: 'baryons', generation: null,
        mass_mev: M_LAMBDA_PHYS, charge: 0, spin: 0.5,
        color_charge: 'singlet', antiparticle: 'antilambda',
        composition: 'uds',
        ftd_formula: 'm_p + m_s (constituent)',
        ftd_accuracy: 1.0, ftd_status: 'parametric',
        display_color: [0.85, 0.25, 0.40], display_size: 6
    },
    {
        id: 'antilambda', name: 'Anti-Lambda', symbol: 'Λ̄⁰',
        category: 'baryons', generation: null,
        mass_mev: M_LAMBDA_PHYS, charge: 0, spin: 0.5,
        color_charge: 'singlet', antiparticle: 'lambda',
        composition: 'ūd̄s̄',
        ftd_formula: 'm_Λ (same mass)',
        ftd_accuracy: 1.0, ftd_status: 'parametric',
        display_color: [0.65, 0.18, 0.30], display_size: 6
    },
    {
        id: 'sigma_plus', name: 'Sigma+', symbol: 'Σ⁺',
        category: 'baryons', generation: null,
        mass_mev: M_SIGMA_PHYS, charge: 1, spin: 0.5,
        color_charge: 'singlet', antiparticle: null,
        composition: 'uus',
        ftd_formula: 'quark model + FTD masses',
        ftd_accuracy: 1.0, ftd_status: 'parametric',
        display_color: [0.90, 0.30, 0.45], display_size: 6
    },
    {
        id: 'sigma_zero', name: 'Sigma0', symbol: 'Σ⁰',
        category: 'baryons', generation: null,
        // [PARAMETRIC PDG] — Σ⁰ isospin partner of Σ⁺ (2026-06-15: now imports
        // the canonical M_SIGMA0_PHYS added to constants.js).
        mass_mev: M_SIGMA0_PHYS, charge: 0, spin: 0.5,
        color_charge: 'singlet', antiparticle: null,
        composition: 'uds',
        ftd_formula: 'quark model + FTD masses',
        ftd_accuracy: 1.0, ftd_status: 'parametric',
        display_color: [0.80, 0.28, 0.42], display_size: 6
    },
    {
        id: 'sigma_minus', name: 'Sigma-', symbol: 'Σ⁻',
        category: 'baryons', generation: null,
        // [PARAMETRIC PDG] — Σ⁻ isospin partner of Σ⁺ (2026-06-15: now imports
        // the canonical M_SIGMA_MINUS_PHYS added to constants.js).
        mass_mev: M_SIGMA_MINUS_PHYS, charge: -1, spin: 0.5,
        color_charge: 'singlet', antiparticle: null,
        composition: 'dds',
        ftd_formula: 'quark model + FTD masses',
        ftd_accuracy: 1.0, ftd_status: 'parametric',
        display_color: [0.75, 0.22, 0.38], display_size: 6
    },
    {
        id: 'xi_zero', name: 'Xi0', symbol: 'Ξ⁰',
        category: 'baryons', generation: null,
        mass_mev: M_XI_0_PHYS, charge: 0, spin: 0.5,
        color_charge: 'singlet', antiparticle: null,
        composition: 'uss',
        ftd_formula: 'quark model + FTD masses',
        ftd_accuracy: 1.0, ftd_status: 'parametric',
        display_color: [0.70, 0.20, 0.50], display_size: 6
    },
    {
        id: 'xi_minus', name: 'Xi-', symbol: 'Ξ⁻',
        category: 'baryons', generation: null,
        mass_mev: M_XI_M_PHYS, charge: -1, spin: 0.5,
        color_charge: 'singlet', antiparticle: null,
        composition: 'dss',
        ftd_formula: 'quark model + FTD masses',
        ftd_accuracy: 1.0, ftd_status: 'parametric',
        display_color: [0.65, 0.18, 0.48], display_size: 6
    },
    {
        id: 'omega_minus', name: 'Omega-', symbol: 'Ω⁻',
        category: 'baryons', generation: null,
        mass_mev: M_OMEGA_PHYS, charge: -1, spin: 1.5,
        color_charge: 'singlet', antiparticle: null,
        composition: 'sss',
        ftd_formula: 'quark model + FTD masses',
        ftd_accuracy: 1.0, ftd_status: 'parametric',
        display_color: [0.60, 0.15, 0.45], display_size: 7
    },
    {
        id: 'delta_pp', name: 'Delta++', symbol: 'Δ⁺⁺',
        category: 'baryons', generation: null,
        mass_mev: M_DELTA_PHYS, charge: 2, spin: 1.5,
        color_charge: 'singlet', antiparticle: null,
        composition: 'uuu',
        ftd_formula: 'quark model + FTD masses',
        ftd_accuracy: 1.0, ftd_status: 'parametric',
        display_color: [0.95, 0.35, 0.55], display_size: 7
    },

    // ═══════════════════════════════════════════════════════════════
    // MESONS (composite: quark-antiquark)
    // ═══════════════════════════════════════════════════════════════
    {
        id: 'pion_plus', name: 'Pion+', symbol: 'π⁺',
        category: 'mesons', generation: null,
        mass_mev: M_PI_CH_PHYS, charge: 1, spin: 0,
        color_charge: 'singlet', antiparticle: 'pion_minus',
        composition: 'ud̄',
        ftd_formula: 'm_e·MU_RATIO·N_c·sin²θ_W/α',
        ftd_accuracy: 2.0, ftd_status: 'parametric',
        display_color: [0.65, 0.40, 0.85], display_size: 5
    },
    {
        id: 'pion_minus', name: 'Pion-', symbol: 'π⁻',
        category: 'mesons', generation: null,
        mass_mev: M_PI_CH_PHYS, charge: -1, spin: 0,
        color_charge: 'singlet', antiparticle: 'pion_plus',
        composition: 'dū',
        ftd_formula: 'm_π⁺ (same mass)',
        ftd_accuracy: 2.0, ftd_status: 'parametric',
        display_color: [0.55, 0.32, 0.75], display_size: 5
    },
    {
        id: 'pion_zero', name: 'Pion0', symbol: 'π⁰',
        category: 'mesons', generation: null,
        mass_mev: M_PI_0_PHYS, charge: 0, spin: 0,
        color_charge: 'singlet', antiparticle: 'pion_zero',
        composition: '(uū−dd̄)/√2',
        ftd_formula: 'm_π± − EM correction',
        ftd_accuracy: 2.0, ftd_status: 'parametric',
        display_color: [0.60, 0.36, 0.80], display_size: 5
    },
    {
        id: 'kaon_plus', name: 'Kaon+', symbol: 'K⁺',
        category: 'mesons', generation: null,
        mass_mev: M_K_CH_PHYS, charge: 1, spin: 0,
        color_charge: 'singlet', antiparticle: 'kaon_minus',
        composition: 'us̄',
        ftd_formula: 'ChPT with FTD quark masses',
        ftd_accuracy: 2.0, ftd_status: 'parametric',
        display_color: [0.70, 0.45, 0.90], display_size: 5
    },
    {
        id: 'kaon_minus', name: 'Kaon-', symbol: 'K⁻',
        category: 'mesons', generation: null,
        mass_mev: M_K_CH_PHYS, charge: -1, spin: 0,
        color_charge: 'singlet', antiparticle: 'kaon_plus',
        composition: 'sū',
        ftd_formula: 'm_K⁺ (same mass)',
        ftd_accuracy: 2.0, ftd_status: 'parametric',
        display_color: [0.60, 0.38, 0.82], display_size: 5
    },
    {
        id: 'kaon_zero', name: 'Kaon0', symbol: 'K⁰',
        category: 'mesons', generation: null,
        mass_mev: M_K_0_PHYS, charge: 0, spin: 0,
        color_charge: 'singlet', antiparticle: 'antikaon_zero',
        composition: 'ds̄',
        ftd_formula: 'ChPT with FTD quark masses',
        ftd_accuracy: 2.0, ftd_status: 'parametric',
        display_color: [0.58, 0.35, 0.78], display_size: 5
    },
    {
        id: 'antikaon_zero', name: 'Anti-Kaon0', symbol: 'K̄⁰',
        category: 'mesons', generation: null,
        mass_mev: M_K_0_PHYS, charge: 0, spin: 0,
        color_charge: 'singlet', antiparticle: 'kaon_zero',
        composition: 'sd̄',
        ftd_formula: 'm_K⁰ (same mass)',
        ftd_accuracy: 2.0, ftd_status: 'parametric',
        display_color: [0.50, 0.30, 0.72], display_size: 5
    },
    {
        id: 'eta', name: 'Eta', symbol: 'η',
        category: 'mesons', generation: null,
        mass_mev: M_ETA_PHYS, charge: 0, spin: 0,
        color_charge: 'singlet', antiparticle: 'eta',
        composition: '(uū+dd̄−2ss̄)/√6',
        ftd_formula: 'ChPT with FTD quark masses',
        ftd_accuracy: 2.0, ftd_status: 'parametric',
        display_color: [0.55, 0.33, 0.75], display_size: 5
    },
    {
        id: 'rho', name: 'Rho', symbol: 'ρ',
        category: 'mesons', generation: null,
        mass_mev: M_RHO_PHYS, charge: 0, spin: 1,
        color_charge: 'singlet', antiparticle: 'rho',
        composition: '(uū−dd̄)/√2',
        ftd_formula: 'vector meson mass formula',
        ftd_accuracy: 2.0, ftd_status: 'parametric',
        display_color: [0.72, 0.42, 0.88], display_size: 5
    },
    {
        id: 'jpsi', name: 'J/ψ', symbol: 'J/ψ',
        category: 'mesons', generation: null,
        mass_mev: M_J_PSI_PHYS, charge: 0, spin: 1,
        color_charge: 'singlet', antiparticle: 'jpsi',
        composition: 'cc̄',
        ftd_formula: '2·m_c (charmonium ground state)',
        ftd_accuracy: 1.0, ftd_status: 'parametric',
        display_color: [0.80, 0.50, 0.92], display_size: 6
    },
    {
        id: 'upsilon', name: 'Upsilon', symbol: 'Υ',
        category: 'mesons', generation: null,
        mass_mev: M_UPSILON_PHYS, charge: 0, spin: 1,
        color_charge: 'singlet', antiparticle: 'upsilon',
        composition: 'bb̄',
        ftd_formula: '2·m_b (bottomonium ground state)',
        ftd_accuracy: 1.0, ftd_status: 'parametric',
        display_color: [0.85, 0.55, 0.95], display_size: 7
    },
];

// ── Baryon / lepton numbers (descriptive SM quantum numbers) ─────────
// Derived once from category + matter/antimatter (id convention: antimatter
// ids start with 'anti', plus 'positron'); antimatter carries opposite sign.
// NOTE (true-to-FTD): FTD treats baryon number as an EMERGENT cluster label,
// not a fundamental conserved charge (FTD-0301). Conventional SM assignments,
// for catalog reference only.
for (const p of PARTICLES) {
    const anti = p.id.startsWith('anti') || p.id === 'positron';
    let baryon = 0, lepton = 0;
    switch (p.category) {
        case 'leptons': lepton = anti ? -1 : 1; break;
        case 'quarks':  baryon = anti ? -1 / 3 : 1 / 3; break;
        case 'baryons': baryon = anti ? -1 : 1; break;
        // mesons, gauge_bosons, scalar: baryon = lepton = 0
    }
    p.baryon = baryon;
    p.lepton = lepton;
}

// ── Category metadata ──────────────────────────────────────────────
const CATEGORIES = {
    leptons:      { label: 'Leptons',      color: '#4ADE80', order: 0 },
    quarks:       { label: 'Quarks',       color: '#F59E0B', order: 1 },
    gauge_bosons: { label: 'Gauge Bosons', color: '#3B82F6', order: 2 },
    scalar:       { label: 'Scalar Boson', color: '#FFD700', order: 3 },
    baryons:      { label: 'Baryons',      color: '#EF4444', order: 4 },
    mesons:       { label: 'Mesons',       color: '#A855F7', order: 5 },
};

// ── Public API ─────────────────────────────────────────────────────

export function getAllParticles() {
    return PARTICLES;
}

export function getById(id) {
    return PARTICLES.find(p => p.id === id) || null;
}

export function getByCategory(category) {
    return PARTICLES.filter(p => p.category === category);
}

export function getCategories() {
    return CATEGORIES;
}

export function getSimulableParticles() {
    // Particles that can meaningfully interact in ParticleEngine
    // (charged particles only — neutral ones just drift via gravity)
    return PARTICLES.filter(p => p.charge !== 0 && p.mass_mev > 0);
}

export function formatMass(mass_mev) {
    return formatMassCompat(mass_mev);
}

export function chargeLabel(charge) {
    if (charge === 0) return '0';
    if (charge === 1) return '+1';
    if (charge === -1) return '−1';
    if (charge === 2) return '+2';
    if (charge === 2/3) return '+2/3';
    if (charge === -2/3) return '−2/3';
    if (charge === 1/3) return '+1/3';
    if (charge === -1/3) return '−1/3';
    return charge > 0 ? '+' + charge : '' + charge;
}
