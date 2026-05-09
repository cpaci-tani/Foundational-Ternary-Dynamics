/**
 * Particle Catalog: Complete Standard Model + FTD-derived particles
 *
 * Every mass traces back through the ontic chain:
 *   e → γ → Γ(1/4) → θ₃ → ϖ → M → G* → π → α → masses
 *
 * Mass formulas use framework integers: N_c=3, N_base=4, b₃=7, N_eff=13
 * and α = 1/137.036 from the master quadratic.
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
    ALPHA, G_STAR, K_B, N_C, N_BASE, B_3, N_EFF,
    PI_FTD as PI, M_E, MU_RATIO, TAU_RATIO, PROTON_RATIO,
    M_P_PHYS, M_N_PHYS, M_SIGMA_PHYS, M_OMEGA_PHYS,
    M_PI_CH_PHYS, M_PI_0_PHYS, M_K_CH_PHYS, M_K_0_PHYS,
    M_DELTA_PHYS,
    // Wave 2B additions (2026-04-26): replace inline literals with
    // canonical constants. These all live in constants.js as
    // [PARAMETRIC PDG] reference values — used only for catalog
    // display, not for derivations.
    M_U_PHYS, M_D_PHYS, M_S_PHYS, M_C_PHYS, M_B_PHYS, M_T_PHYS,
    // Note: neutrino *_PHYS values exist in constants.js but are NOT
    // imported here — the catalog's nu_e/nu_mu/nu_tau literals carry a
    // ×1e-3 unit-mismatch with the canonical *_PHYS values (see TODO
    // Theme H comments at each neutrino entry).
    M_W_PHYS, M_Z_PHYS, M_HIGGS_PHYS,
    M_LAMBDA_PHYS, M_XI_0_PHYS, M_XI_M_PHYS, M_DELTA_0_PHYS,
    M_ETA_PHYS, M_RHO_PHYS, M_J_PSI_PHYS, M_UPSILON_PHYS,
} from './constants.js';

const PARTICLES = [
    // ═══════════════════════════════════════════════════════════════
    // LEPTONS (charged + neutrinos)
    // ═══════════════════════════════════════════════════════════════
    {
        id: 'electron', name: 'Electron', symbol: 'e⁻',
        category: 'leptons', generation: 1,
        mass_mev: M_E, charge: -1, spin: 0.5,
        color_charge: 'none', antiparticle: 'positron',
        ftd_formula: 'm_P·√(2π)·(16/3)·α¹¹',
        ftd_accuracy: 0.27, ftd_status: 'derived',
        display_color: [0.29, 0.87, 0.50], display_size: 4
    },
    {
        id: 'positron', name: 'Positron', symbol: 'e⁺',
        category: 'leptons', generation: 1,
        mass_mev: M_E, charge: 1, spin: 0.5,
        color_charge: 'none', antiparticle: 'electron',
        ftd_formula: 'm_e (same mass)',
        ftd_accuracy: 0.27, ftd_status: 'derived',
        display_color: [0.97, 0.44, 0.44], display_size: 4
    },
    {
        id: 'muon', name: 'Muon', symbol: 'μ⁻',
        category: 'leptons', generation: 2,
        mass_mev: M_E * MU_RATIO, charge: -1, spin: 0.5,
        color_charge: 'none', antiparticle: 'antimuon',
        ftd_formula: 'm_e·(3·b₃·(b₃+N_c)−N_c) = 207·m_e',
        ftd_accuracy: 0.11, ftd_status: 'derived',
        display_color: [0.20, 0.73, 0.40], display_size: 5
    },
    {
        id: 'antimuon', name: 'Antimuon', symbol: 'μ⁺',
        category: 'leptons', generation: 2,
        mass_mev: M_E * MU_RATIO, charge: 1, spin: 0.5,
        color_charge: 'none', antiparticle: 'muon',
        ftd_formula: 'm_μ (same mass)',
        ftd_accuracy: 0.11, ftd_status: 'derived',
        display_color: [0.90, 0.35, 0.35], display_size: 5
    },
    {
        id: 'tau', name: 'Tau', symbol: 'τ⁻',
        category: 'leptons', generation: 3,
        mass_mev: M_E * TAU_RATIO, charge: -1, spin: 0.5,
        color_charge: 'none', antiparticle: 'antitau',
        ftd_formula: 'm_e·((N_eff+N_base)·207−2N_c·b₃) = 3477·m_e',
        ftd_accuracy: 0.007, ftd_status: 'derived',
        display_color: [0.12, 0.60, 0.32], display_size: 6
    },
    {
        id: 'antitau', name: 'Antitau', symbol: 'τ⁺',
        category: 'leptons', generation: 3,
        mass_mev: M_E * TAU_RATIO, charge: 1, spin: 0.5,
        color_charge: 'none', antiparticle: 'tau',
        ftd_formula: 'm_τ (same mass)',
        ftd_accuracy: 0.007, ftd_status: 'derived',
        display_color: [0.82, 0.28, 0.28], display_size: 6
    },
    {
        id: 'nu_e', name: 'Electron Neutrino', symbol: 'νₑ',
        category: 'leptons', generation: 1,
        // 2026-04-27 audit: dropped a stray ×1e-3 unit-mismatch factor that
        // made the catalog 1000× lighter than M_NU_E_PHYS = 4.1e-9 MeV in
        // constants.js. Now matches the canonical value (sum-of-masses ~0.06 eV).
        mass_mev: 4.1e-9, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'antinu_e',
        ftd_formula: 'm₃·(m_e/m_τ)² ≈ 4.1 neV',
        ftd_accuracy: null, ftd_status: 'derived',
        display_color: [0.70, 0.95, 0.80], display_size: 2
    },
    {
        id: 'antinu_e', name: 'Electron Antineutrino', symbol: 'ν̄ₑ',
        category: 'leptons', generation: 1,
        // 2026-04-27 audit: same ×1e-3 factor drop as nu_e above.
        mass_mev: 4.1e-9, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'nu_e',
        ftd_formula: 'm_ν₁ (same mass)',
        ftd_accuracy: null, ftd_status: 'derived',
        display_color: [0.95, 0.80, 0.80], display_size: 2
    },
    {
        id: 'nu_mu', name: 'Muon Neutrino', symbol: 'νμ',
        category: 'leptons', generation: 2,
        mass_mev: 8.58e-3, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'antinu_mu',
        ftd_formula: 'm₃·√N_c/(b₃+N_c) ≈ 8.6 meV',
        ftd_accuracy: null, ftd_status: 'derived',
        display_color: [0.60, 0.90, 0.72], display_size: 2
    },
    {
        id: 'antinu_mu', name: 'Muon Antineutrino', symbol: 'ν̄μ',
        category: 'leptons', generation: 2,
        mass_mev: 8.58e-3, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'nu_mu',
        ftd_formula: 'm_ν₂ (same mass)',
        ftd_accuracy: null, ftd_status: 'derived',
        display_color: [0.90, 0.72, 0.72], display_size: 2
    },
    {
        id: 'nu_tau', name: 'Tau Neutrino', symbol: 'ντ',
        category: 'leptons', generation: 3,
        mass_mev: 4.955e-2, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'antinu_tau',
        ftd_formula: 'v·(N_base/N_c)·α⁶ ≈ 49.6 meV',
        ftd_accuracy: null, ftd_status: 'derived',
        display_color: [0.50, 0.85, 0.65], display_size: 2
    },
    {
        id: 'antinu_tau', name: 'Tau Antineutrino', symbol: 'ν̄τ',
        category: 'leptons', generation: 3,
        mass_mev: 4.955e-2, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'nu_tau',
        ftd_formula: 'm_ν₃ (same mass)',
        ftd_accuracy: null, ftd_status: 'derived',
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
        ftd_accuracy: 0.02, ftd_status: 'derived',
        display_color: [0.30, 0.60, 0.95], display_size: 7
    },
    {
        id: 'w_minus', name: 'W⁻ Boson', symbol: 'W⁻',
        category: 'gauge_bosons', generation: null,
        mass_mev: M_W_PHYS, charge: -1, spin: 1,
        color_charge: 'none', antiparticle: 'w_plus',
        ftd_formula: 'm_W (same mass)',
        ftd_accuracy: 0.02, ftd_status: 'derived',
        display_color: [0.20, 0.50, 0.85], display_size: 7
    },
    {
        id: 'z_boson', name: 'Z Boson', symbol: 'Z⁰',
        category: 'gauge_bosons', generation: null,
        mass_mev: M_Z_PHYS, charge: 0, spin: 1,
        color_charge: 'none', antiparticle: 'z_boson',
        ftd_formula: 'm_W/cos(θ_W) ≈ 91.2 GeV',
        ftd_accuracy: 0.01, ftd_status: 'derived',
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
        ftd_formula: 'm_e·N_eff/α² ≈ 124.8 GeV',
        ftd_accuracy: 0.24, ftd_status: 'selection',
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
        ftd_accuracy: 0.017, ftd_status: 'derived',
        display_color: [0.95, 0.30, 0.30], display_size: 6
    },
    {
        id: 'antiproton', name: 'Antiproton', symbol: 'p̄',
        category: 'baryons', generation: null,
        mass_mev: M_P_PHYS, charge: -1, spin: 0.5,
        color_charge: 'singlet', antiparticle: 'proton',
        composition: 'ūūd̄',
        ftd_formula: 'm_p (same mass)',
        ftd_accuracy: 0.017, ftd_status: 'derived',
        display_color: [0.30, 0.95, 0.95], display_size: 6
    },
    {
        id: 'neutron', name: 'Neutron', symbol: 'n',
        category: 'baryons', generation: null,
        mass_mev: M_N_PHYS, charge: 0, spin: 0.5,
        color_charge: 'singlet', antiparticle: 'antineutron',
        composition: 'udd',
        ftd_formula: 'm_p + (m_d−m_u)·(1+α/π)',
        ftd_accuracy: 0.02, ftd_status: 'derived',
        display_color: [0.70, 0.25, 0.55], display_size: 6
    },
    {
        id: 'antineutron', name: 'Antineutron', symbol: 'n̄',
        category: 'baryons', generation: null,
        mass_mev: M_N_PHYS, charge: 0, spin: 0.5,
        color_charge: 'singlet', antiparticle: 'neutron',
        composition: 'ūd̄d̄',
        ftd_formula: 'm_n (same mass)',
        ftd_accuracy: 0.02, ftd_status: 'derived',
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
        // [PARAMETRIC PDG] — Σ⁰ isospin partner of Σ⁺; canonical
        // constant not added (M_SIGMA_PHYS covers only Σ⁺).
        mass_mev: 1192.6, charge: 0, spin: 0.5,
        color_charge: 'singlet', antiparticle: null,
        composition: 'uds',
        ftd_formula: 'quark model + FTD masses',
        ftd_accuracy: 1.0, ftd_status: 'parametric',
        display_color: [0.80, 0.28, 0.42], display_size: 6
    },
    {
        id: 'sigma_minus', name: 'Sigma-', symbol: 'Σ⁻',
        category: 'baryons', generation: null,
        // [PARAMETRIC PDG] — Σ⁻ isospin partner of Σ⁺; canonical
        // constant not added (M_SIGMA_PHYS covers only Σ⁺).
        mass_mev: 1197.4, charge: -1, spin: 0.5,
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
    if (mass_mev === 0) return 'massless';
    if (mass_mev < 1e-6) return (mass_mev * 1e6).toFixed(1) + ' neV';
    if (mass_mev < 1e-3) return (mass_mev * 1e3).toFixed(1) + ' eV';
    if (mass_mev < 1) return (mass_mev * 1e3).toFixed(1) + ' keV';
    if (mass_mev < 1000) return mass_mev.toFixed(1) + ' MeV';
    return (mass_mev / 1000).toFixed(2) + ' GeV';
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
