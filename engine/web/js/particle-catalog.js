/**
 * Imported particle reference catalog plus explicitly qualified FTD expressions.
 *
 * mass_mev denotes a reference mass in MeV/c² (c=1); its status, basis, source,
 * and limitations are explicit. Flavor neutrinos have no single assigned mass.
 * These catalog values are not observations or recovered Scale-0 identities.
 * FTD expressions are separately evaluated motivating relations at their
 * canonical selection/parametric status, never first-principles derivations.
 */

import {
    M_E, M_E_PHYS, M_MU_PHYS, M_TAU_PHYS,
    M_P_PHYS, M_N_PHYS, M_SIGMA_PHYS, M_OMEGA_PHYS,
    M_PI_CH_PHYS, M_PI_0_PHYS, M_K_CH_PHYS, M_K_0_PHYS, M_DELTA_PHYS,
    M_U_PHYS, M_D_PHYS, M_S_PHYS, M_C_PHYS, M_B_PHYS, M_T_PHYS,
    M_SIGMA0_PHYS, M_SIGMA_MINUS_PHYS,
    M_W_PHYS, M_Z_PHYS, M_HIGGS_PHYS,
    M_LAMBDA_PHYS, M_XI_0_PHYS, M_XI_M_PHYS,
    M_ETA_PHYS, M_RHO_PHYS, M_J_PSI_PHYS, M_UPSILON_PHYS,
    ALPHA, N_EFF, N_BASE, N_C, B_3, M_PLANCK_MEV,
} from './constants.js';
import { formatMassCompat } from './units.js';

const PARTICLES = [
    {
        id: 'electron', name: 'Electron', symbol: 'e⁻',
        category: 'leptons', generation: 1,
        mass_mev: M_E_PHYS, charge: -1, spin: 0.5,
        color_charge: 'none', antiparticle: 'positron',
        display_color: [0.29, 0.87, 0.50], display_size: 4
    },
    {
        id: 'positron', name: 'Positron', symbol: 'e⁺',
        category: 'leptons', generation: 1,
        mass_mev: M_E_PHYS, charge: 1, spin: 0.5,
        color_charge: 'none', antiparticle: 'electron',
        display_color: [0.97, 0.44, 0.44], display_size: 4
    },
    {
        id: 'muon', name: 'Muon', symbol: 'μ⁻',
        category: 'leptons', generation: 2,
        mass_mev: M_MU_PHYS, charge: -1, spin: 0.5,
        color_charge: 'none', antiparticle: 'antimuon',
        display_color: [0.20, 0.73, 0.40], display_size: 5
    },
    {
        id: 'antimuon', name: 'Antimuon', symbol: 'μ⁺',
        category: 'leptons', generation: 2,
        mass_mev: M_MU_PHYS, charge: 1, spin: 0.5,
        color_charge: 'none', antiparticle: 'muon',
        display_color: [0.90, 0.35, 0.35], display_size: 5
    },
    {
        id: 'tau', name: 'Tau', symbol: 'τ⁻',
        category: 'leptons', generation: 3,
        mass_mev: M_TAU_PHYS, charge: -1, spin: 0.5,
        color_charge: 'none', antiparticle: 'antitau',
        display_color: [0.12, 0.60, 0.32], display_size: 6
    },
    {
        id: 'antitau', name: 'Antitau', symbol: 'τ⁺',
        category: 'leptons', generation: 3,
        mass_mev: M_TAU_PHYS, charge: 1, spin: 0.5,
        color_charge: 'none', antiparticle: 'tau',
        display_color: [0.82, 0.28, 0.28], display_size: 6
    },
    {
        id: 'nu_e', name: 'Electron Neutrino', symbol: 'νₑ',
        category: 'leptons', generation: 1,
        mass_mev: null, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'antinu_e',
        display_color: [0.70, 0.95, 0.80], display_size: 2
    },
    {
        id: 'antinu_e', name: 'Electron Antineutrino', symbol: 'ν̄ₑ',
        category: 'leptons', generation: 1,
        mass_mev: null, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'nu_e',
        display_color: [0.95, 0.80, 0.80], display_size: 2
    },
    {
        id: 'nu_mu', name: 'Muon Neutrino', symbol: 'νμ',
        category: 'leptons', generation: 2,
        mass_mev: null, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'antinu_mu',
        display_color: [0.60, 0.90, 0.72], display_size: 2
    },
    {
        id: 'antinu_mu', name: 'Muon Antineutrino', symbol: 'ν̄μ',
        category: 'leptons', generation: 2,
        mass_mev: null, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'nu_mu',
        display_color: [0.90, 0.72, 0.72], display_size: 2
    },
    {
        id: 'nu_tau', name: 'Tau Neutrino', symbol: 'ντ',
        category: 'leptons', generation: 3,
        mass_mev: null, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'antinu_tau',
        display_color: [0.50, 0.85, 0.65], display_size: 2
    },
    {
        id: 'antinu_tau', name: 'Tau Antineutrino', symbol: 'ν̄τ',
        category: 'leptons', generation: 3,
        mass_mev: null, charge: 0, spin: 0.5,
        color_charge: 'none', antiparticle: 'nu_tau',
        display_color: [0.85, 0.65, 0.65], display_size: 2
    },
    {
        id: 'up', name: 'Up Quark', symbol: 'u',
        category: 'quarks', generation: 1,
        mass_mev: M_U_PHYS, charge: 2/3, spin: 0.5,
        color_charge: 'r/g/b', antiparticle: 'anti_up',
        display_color: [1.00, 0.75, 0.30], display_size: 3
    },
    {
        id: 'anti_up', name: 'Anti-Up', symbol: 'ū',
        category: 'quarks', generation: 1,
        mass_mev: M_U_PHYS, charge: -2/3, spin: 0.5,
        color_charge: 'r̄/ḡ/b̄', antiparticle: 'up',
        display_color: [0.70, 0.50, 0.20], display_size: 3
    },
    {
        id: 'down', name: 'Down Quark', symbol: 'd',
        category: 'quarks', generation: 1,
        mass_mev: M_D_PHYS, charge: -1/3, spin: 0.5,
        color_charge: 'r/g/b', antiparticle: 'anti_down',
        display_color: [0.95, 0.65, 0.25], display_size: 3
    },
    {
        id: 'anti_down', name: 'Anti-Down', symbol: 'd̄',
        category: 'quarks', generation: 1,
        mass_mev: M_D_PHYS, charge: 1/3, spin: 0.5,
        color_charge: 'r̄/ḡ/b̄', antiparticle: 'down',
        display_color: [0.65, 0.45, 0.18], display_size: 3
    },
    {
        id: 'strange', name: 'Strange Quark', symbol: 's',
        category: 'quarks', generation: 2,
        mass_mev: M_S_PHYS, charge: -1/3, spin: 0.5,
        color_charge: 'r/g/b', antiparticle: 'anti_strange',
        display_color: [0.90, 0.55, 0.18], display_size: 4
    },
    {
        id: 'anti_strange', name: 'Anti-Strange', symbol: 's̄',
        category: 'quarks', generation: 2,
        mass_mev: M_S_PHYS, charge: 1/3, spin: 0.5,
        color_charge: 'r̄/ḡ/b̄', antiparticle: 'strange',
        display_color: [0.60, 0.38, 0.12], display_size: 4
    },
    {
        id: 'charm', name: 'Charm Quark', symbol: 'c',
        category: 'quarks', generation: 2,
        mass_mev: M_C_PHYS, charge: 2/3, spin: 0.5,
        color_charge: 'r/g/b', antiparticle: 'anti_charm',
        display_color: [0.85, 0.50, 0.12], display_size: 5
    },
    {
        id: 'anti_charm', name: 'Anti-Charm', symbol: 'c̄',
        category: 'quarks', generation: 2,
        mass_mev: M_C_PHYS, charge: -2/3, spin: 0.5,
        color_charge: 'r̄/ḡ/b̄', antiparticle: 'charm',
        display_color: [0.58, 0.35, 0.08], display_size: 5
    },
    {
        id: 'bottom', name: 'Bottom Quark', symbol: 'b',
        category: 'quarks', generation: 3,
        mass_mev: M_B_PHYS, charge: -1/3, spin: 0.5,
        color_charge: 'r/g/b', antiparticle: 'anti_bottom',
        display_color: [0.80, 0.45, 0.08], display_size: 6
    },
    {
        id: 'anti_bottom', name: 'Anti-Bottom', symbol: 'b̄',
        category: 'quarks', generation: 3,
        mass_mev: M_B_PHYS, charge: 1/3, spin: 0.5,
        color_charge: 'r̄/ḡ/b̄', antiparticle: 'bottom',
        display_color: [0.55, 0.32, 0.06], display_size: 6
    },
    {
        id: 'top', name: 'Top Quark', symbol: 't',
        category: 'quarks', generation: 3,
        mass_mev: M_T_PHYS, charge: 2/3, spin: 0.5,
        color_charge: 'r/g/b', antiparticle: 'anti_top',
        display_color: [0.75, 0.40, 0.05], display_size: 8
    },
    {
        id: 'anti_top', name: 'Anti-Top', symbol: 't̄',
        category: 'quarks', generation: 3,
        mass_mev: M_T_PHYS, charge: -2/3, spin: 0.5,
        color_charge: 'r̄/ḡ/b̄', antiparticle: 'top',
        display_color: [0.50, 0.28, 0.04], display_size: 8
    },
    {
        id: 'photon', name: 'Photon', symbol: 'γ',
        category: 'gauge_bosons', generation: null,
        mass_mev: 0, charge: 0, spin: 1,
        color_charge: 'none', antiparticle: 'photon',
        display_color: [0.95, 0.95, 0.40], display_size: 3
    },
    {
        id: 'gluon', name: 'Gluon', symbol: 'g',
        category: 'gauge_bosons', generation: null,
        mass_mev: 0, charge: 0, spin: 1,
        color_charge: 'octet', antiparticle: 'gluon',
        display_color: [0.40, 0.75, 0.95], display_size: 3
    },
    {
        id: 'w_plus', name: 'W⁺ Boson', symbol: 'W⁺',
        category: 'gauge_bosons', generation: null,
        mass_mev: M_W_PHYS, charge: 1, spin: 1,
        color_charge: 'none', antiparticle: 'w_minus',
        display_color: [0.30, 0.60, 0.95], display_size: 7
    },
    {
        id: 'w_minus', name: 'W⁻ Boson', symbol: 'W⁻',
        category: 'gauge_bosons', generation: null,
        mass_mev: M_W_PHYS, charge: -1, spin: 1,
        color_charge: 'none', antiparticle: 'w_plus',
        display_color: [0.20, 0.50, 0.85], display_size: 7
    },
    {
        id: 'z_boson', name: 'Z Boson', symbol: 'Z⁰',
        category: 'gauge_bosons', generation: null,
        mass_mev: M_Z_PHYS, charge: 0, spin: 1,
        color_charge: 'none', antiparticle: 'z_boson',
        display_color: [0.25, 0.55, 0.90], display_size: 7
    },
    {
        id: 'higgs', name: 'Higgs Boson', symbol: 'H⁰',
        category: 'scalar', generation: null,
        mass_mev: M_HIGGS_PHYS, charge: 0, spin: 0,
        color_charge: 'none', antiparticle: 'higgs',
        display_color: [1.00, 0.84, 0.00], display_size: 8
    },
    {
        id: 'proton', name: 'Proton', symbol: 'p',
        category: 'baryons', generation: null,
        mass_mev: M_P_PHYS, charge: 1, spin: 0.5,
        color_charge: 'singlet', antiparticle: 'antiproton',
        composition: 'uud',
        display_color: [0.95, 0.30, 0.30], display_size: 6
    },
    {
        id: 'antiproton', name: 'Antiproton', symbol: 'p̄',
        category: 'baryons', generation: null,
        mass_mev: M_P_PHYS, charge: -1, spin: 0.5,
        color_charge: 'singlet', antiparticle: 'proton',
        composition: 'ūūd̄',
        display_color: [0.30, 0.95, 0.95], display_size: 6
    },
    {
        id: 'neutron', name: 'Neutron', symbol: 'n',
        category: 'baryons', generation: null,
        mass_mev: M_N_PHYS, charge: 0, spin: 0.5,
        color_charge: 'singlet', antiparticle: 'antineutron',
        composition: 'udd',
        display_color: [0.70, 0.25, 0.55], display_size: 6
    },
    {
        id: 'antineutron', name: 'Antineutron', symbol: 'n̄',
        category: 'baryons', generation: null,
        mass_mev: M_N_PHYS, charge: 0, spin: 0.5,
        color_charge: 'singlet', antiparticle: 'neutron',
        composition: 'ūd̄d̄',
        display_color: [0.55, 0.20, 0.45], display_size: 6
    },
    {
        id: 'lambda', name: 'Lambda', symbol: 'Λ⁰',
        category: 'baryons', generation: null,
        mass_mev: M_LAMBDA_PHYS, charge: 0, spin: 0.5,
        color_charge: 'singlet', antiparticle: 'antilambda',
        composition: 'uds',
        display_color: [0.85, 0.25, 0.40], display_size: 6
    },
    {
        id: 'antilambda', name: 'Anti-Lambda', symbol: 'Λ̄⁰',
        category: 'baryons', generation: null,
        mass_mev: M_LAMBDA_PHYS, charge: 0, spin: 0.5,
        color_charge: 'singlet', antiparticle: 'lambda',
        composition: 'ūd̄s̄',
        display_color: [0.65, 0.18, 0.30], display_size: 6
    },
    {
        id: 'sigma_plus', name: 'Sigma+', symbol: 'Σ⁺',
        category: 'baryons', generation: null,
        mass_mev: M_SIGMA_PHYS, charge: 1, spin: 0.5,
        color_charge: 'singlet', antiparticle: null,
        composition: 'uus',
        display_color: [0.90, 0.30, 0.45], display_size: 6
    },
    {
        id: 'sigma_zero', name: 'Sigma0', symbol: 'Σ⁰',
        category: 'baryons', generation: null,
        mass_mev: M_SIGMA0_PHYS, charge: 0, spin: 0.5,
        color_charge: 'singlet', antiparticle: null,
        composition: 'uds',
        display_color: [0.80, 0.28, 0.42], display_size: 6
    },
    {
        id: 'sigma_minus', name: 'Sigma-', symbol: 'Σ⁻',
        category: 'baryons', generation: null,
        mass_mev: M_SIGMA_MINUS_PHYS, charge: -1, spin: 0.5,
        color_charge: 'singlet', antiparticle: null,
        composition: 'dds',
        display_color: [0.75, 0.22, 0.38], display_size: 6
    },
    {
        id: 'xi_zero', name: 'Xi0', symbol: 'Ξ⁰',
        category: 'baryons', generation: null,
        mass_mev: M_XI_0_PHYS, charge: 0, spin: 0.5,
        color_charge: 'singlet', antiparticle: null,
        composition: 'uss',
        display_color: [0.70, 0.20, 0.50], display_size: 6
    },
    {
        id: 'xi_minus', name: 'Xi-', symbol: 'Ξ⁻',
        category: 'baryons', generation: null,
        mass_mev: M_XI_M_PHYS, charge: -1, spin: 0.5,
        color_charge: 'singlet', antiparticle: null,
        composition: 'dss',
        display_color: [0.65, 0.18, 0.48], display_size: 6
    },
    {
        id: 'omega_minus', name: 'Omega-', symbol: 'Ω⁻',
        category: 'baryons', generation: null,
        mass_mev: M_OMEGA_PHYS, charge: -1, spin: 1.5,
        color_charge: 'singlet', antiparticle: null,
        composition: 'sss',
        display_color: [0.60, 0.15, 0.45], display_size: 7
    },
    {
        id: 'delta_pp', name: 'Delta++', symbol: 'Δ⁺⁺',
        category: 'baryons', generation: null,
        mass_mev: M_DELTA_PHYS, charge: 2, spin: 1.5,
        color_charge: 'singlet', antiparticle: null,
        composition: 'uuu',
        display_color: [0.95, 0.35, 0.55], display_size: 7
    },
    {
        id: 'pion_plus', name: 'Pion+', symbol: 'π⁺',
        category: 'mesons', generation: null,
        mass_mev: M_PI_CH_PHYS, charge: 1, spin: 0,
        color_charge: 'singlet', antiparticle: 'pion_minus',
        composition: 'ud̄',
        display_color: [0.65, 0.40, 0.85], display_size: 5
    },
    {
        id: 'pion_minus', name: 'Pion-', symbol: 'π⁻',
        category: 'mesons', generation: null,
        mass_mev: M_PI_CH_PHYS, charge: -1, spin: 0,
        color_charge: 'singlet', antiparticle: 'pion_plus',
        composition: 'dū',
        display_color: [0.55, 0.32, 0.75], display_size: 5
    },
    {
        id: 'pion_zero', name: 'Pion0', symbol: 'π⁰',
        category: 'mesons', generation: null,
        mass_mev: M_PI_0_PHYS, charge: 0, spin: 0,
        color_charge: 'singlet', antiparticle: 'pion_zero',
        composition: '(uū−dd̄)/√2',
        display_color: [0.60, 0.36, 0.80], display_size: 5
    },
    {
        id: 'kaon_plus', name: 'Kaon+', symbol: 'K⁺',
        category: 'mesons', generation: null,
        mass_mev: M_K_CH_PHYS, charge: 1, spin: 0,
        color_charge: 'singlet', antiparticle: 'kaon_minus',
        composition: 'us̄',
        display_color: [0.70, 0.45, 0.90], display_size: 5
    },
    {
        id: 'kaon_minus', name: 'Kaon-', symbol: 'K⁻',
        category: 'mesons', generation: null,
        mass_mev: M_K_CH_PHYS, charge: -1, spin: 0,
        color_charge: 'singlet', antiparticle: 'kaon_plus',
        composition: 'sū',
        display_color: [0.60, 0.38, 0.82], display_size: 5
    },
    {
        id: 'kaon_zero', name: 'Kaon0', symbol: 'K⁰',
        category: 'mesons', generation: null,
        mass_mev: M_K_0_PHYS, charge: 0, spin: 0,
        color_charge: 'singlet', antiparticle: 'antikaon_zero',
        composition: 'ds̄',
        display_color: [0.58, 0.35, 0.78], display_size: 5
    },
    {
        id: 'antikaon_zero', name: 'Anti-Kaon0', symbol: 'K̄⁰',
        category: 'mesons', generation: null,
        mass_mev: M_K_0_PHYS, charge: 0, spin: 0,
        color_charge: 'singlet', antiparticle: 'kaon_zero',
        composition: 'sd̄',
        display_color: [0.50, 0.30, 0.72], display_size: 5
    },
    {
        id: 'eta', name: 'Eta', symbol: 'η',
        category: 'mesons', generation: null,
        mass_mev: M_ETA_PHYS, charge: 0, spin: 0,
        color_charge: 'singlet', antiparticle: 'eta',
        composition: 'η₈ cos θ − η₁ sin θ (octet–singlet mixture)',
        display_color: [0.55, 0.33, 0.75], display_size: 5
    },
    {
        id: 'rho', name: 'Rho', symbol: 'ρ',
        category: 'mesons', generation: null,
        mass_mev: M_RHO_PHYS, charge: 0, spin: 1,
        color_charge: 'singlet', antiparticle: 'rho',
        composition: '(uū−dd̄)/√2',
        display_color: [0.72, 0.42, 0.88], display_size: 5
    },
    {
        id: 'jpsi', name: 'J/ψ', symbol: 'J/ψ',
        category: 'mesons', generation: null,
        mass_mev: M_J_PSI_PHYS, charge: 0, spin: 1,
        color_charge: 'singlet', antiparticle: 'jpsi',
        composition: 'cc̄',
        display_color: [0.80, 0.50, 0.92], display_size: 6
    },
    {
        id: 'upsilon', name: 'Upsilon', symbol: 'Υ',
        category: 'mesons', generation: null,
        mass_mev: M_UPSILON_PHYS, charge: 0, spin: 1,
        color_charge: 'singlet', antiparticle: 'upsilon',
        composition: 'bb̄',
        display_color: [0.85, 0.55, 0.95], display_size: 7
    },
];

// Reference mass metadata describe existing constants, not a silent data update.
const PDG_2022 = 'https://pdg.lbl.gov/2022/tables/contents_tables.html';
const PDG_2024 = 'https://pdg.lbl.gov/2024/tables/contents_tables.html';
const NIST_2022 = 'https://physics.nist.gov/cuu/pdf/wall_2022.pdf';
const NEUTRINO_REFERENCE = 'https://pdg.lbl.gov/2025/reviews/rpp2025-rev-neutrino-mixing.pdf';

function massReference(entry) {
    if (entry.mass_mev === null) return {
        mass_status: 'flavor-superposition', mass_basis: 'flavor',
        mass_source: NEUTRINO_REFERENCE,
        mass_note: 'A weak-interaction flavor state is a superposition of mass eigenstates; no single flavor mass is assigned. Absolute masses and Dirac/Majorana nature remain unresolved.',
    };
    if (entry.mass_mev === 0) return {
        mass_status: 'massless-reference', mass_basis: 'gauge-boson',
        mass_source: PDG_2022,
        mass_note: 'Massless gauge-boson reference from the Standard Model; not a recovered lattice particle or an FTD primitive.',
    };
    if (entry.category === 'quarks') {
        const id = entry.id.replace(/^anti_/, '');
        if (id === 'top') return {
            mass_status: 'approximate-reference', mass_basis: 'unspecified-top-scheme',
            mass_source: null,
            mass_note: 'Legacy 172.76 GeV top-mass reference from constants.js; measurement scheme and uncertainty are not recorded, so this is not a precision mass determination.',
        };
        const scale = ['up', 'down', 'strange'].includes(id)
            ? 'at renormalization scale 2 GeV' : 'at the quark mass scale μ=m̄';
        return {
            mass_status: 'running-reference', mass_basis: 'MSbar',
            mass_source: PDG_2022,
            mass_note: `Adopted PDG-2022 running mass in the MS-bar scheme ${scale}; not a constituent mass or a free quark mass measurement.`,
        };
    }
    if (entry.id === 'rho' || entry.id === 'delta_pp') return {
        mass_status: 'approximate-reference', mass_basis: 'resonance-parameter',
        mass_source: entry.id === 'rho'
            ? 'https://pdg.lbl.gov/2025/listings/rpp2025-list-rho-770.pdf' : PDG_2022,
        mass_note: entry.id === 'rho'
            ? 'Legacy nominal neutral-ρ mass of 770 MeV/c²; the cited PDG-2025 average is 775.26±0.23 MeV/c². This constant has not been updated.'
            : 'Nominal Δ(1232) resonance mass; a broad-resonance parameter, not an exact stable-particle mass.',
    };
    const codata = ['electron', 'positron', 'muon', 'antimuon'].includes(entry.id);
    return {
        mass_status: 'measured-reference', mass_basis: 'rest-mass',
        mass_source: codata ? NIST_2022
            : ['w_plus', 'w_minus', 'higgs'].includes(entry.id) ? PDG_2024 : PDG_2022,
        mass_note: 'Adopted reference value from constants.js in MeV/c² (c=1); finite stored precision and source edition apply. It is not an FTD mass measurement.',
    };
}

// Retain only expressions with an explicit canonical record and executable
// arithmetic. M_E remains the declared 0.511 MeV model anchor; M_E_PHYS is the
// separate catalog reference used in the residual. No expression is refitted.
const muRatio = 3 * B_3 * (B_3 + N_C) - N_C;
const tauRatio = (N_EFF + N_BASE) * muRatio - 2 * N_C * B_3;
const FTD_RELATIONS = {
    electron: {
        formula: 'm_P·√(2π)·(16/3)·α¹¹',
        mass: M_PLANCK_MEV * Math.sqrt(2 * Math.PI) * (16 / 3) * ALPHA ** 11,
        status: 'selection', source: 'docs/theory/07_assessment/core_ledgers/LEDGER.md#ftd-0015-record',
    },
    muon: {
        formula: 'm_e·[3·b₃·(b₃+N_c)−N_c] = 207·m_e',
        mass: M_E * muRatio, status: 'parametric',
        source: 'docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md',
    },
    tau: {
        formula: 'm_e·[(N_eff+N_base)·207−2N_c·b₃] = 3477·m_e',
        mass: M_E * tauRatio, status: 'parametric',
        source: 'docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md',
    },
    proton: {
        formula: 'm_e·(N_eff/α+N_base·N_eff+N_c)',
        mass: M_E * (N_EFF / ALPHA + N_BASE * N_EFF + N_C),
        status: 'selection', source: 'docs/theory/07_assessment/core_ledgers/LEDGER.md',
    },
    higgs: {
        formula: 'm_e·N_eff/α²', mass: M_E * N_EFF / ALPHA ** 2,
        status: 'parametric', source: 'docs/theory/07_assessment/core_ledgers/LEDGER.md',
    },
};
const FTD_PARTNERS = {
    positron: 'electron', antimuon: 'muon', antitau: 'tau', antiproton: 'proton',
};
for (const entry of PARTICLES) {
    Object.assign(entry, massReference(entry));
    entry.antiparticle_status = entry.antiparticle === null ? 'not-listed'
        : entry.antiparticle === entry.id ? 'self-conjugate' : 'listed-partner';
    // Flavor names label weak production/detection channels; their paired names
    // do not settle whether the underlying massive neutrinos are Majorana.
    entry.antiparticle_note = entry.mass_basis === 'flavor'
        ? 'Weak-interaction neutrino/antineutrino labels; Dirac/Majorana nature is unresolved.'
        : entry.antiparticle === null ? 'The antiparticle exists but is omitted from this catalog.' : '';
    const relation = FTD_RELATIONS[FTD_PARTNERS[entry.id] || entry.id];
    entry.ftd_formula = relation?.formula ?? null;
    entry.ftd_mass_mev = relation?.mass ?? null;
    entry.ftd_status = relation?.status ?? null;
    entry.ftd_source = relation?.source ?? null;
    entry.ftd_accuracy = relation
        ? Math.abs((relation.mass - entry.mass_mev) / entry.mass_mev) * 100 : null;
    entry.ftd_note = relation
        ? 'Motivating relation only, evaluated with the calibrated dashboard alpha and the stated model anchors. The discrepancy is an absolute percentage against the stored reference, not a precision, confidence level, or recovered particle identity.'
        : 'No validated, reproducible FTD mass expression is supplied for this entry.';
    if (entry.id === 'higgs') entry.ftd_note += ' The exact Higgs mass relation is excluded at the cited PDG-2024 precision; a small percentage difference is not agreement within experimental uncertainty.';
}

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

/** Compatibility with current classical PE injection, not SM recovery. */
export function getCatalogSimulationSupport(entry) {
    if (!entry || typeof entry !== 'object') {
        return { supported: false, reason: 'Unknown catalog entry.' };
    }
    if (entry.mass_status === 'flavor-superposition' || entry.mass_basis === 'flavor') {
        return { supported: false, reason: 'Neutrino flavors require mass mixing; the classical particle engine has no flavor-state dynamics.' };
    }
    if (!Number.isFinite(entry.charge)) {
        return { supported: false, reason: 'A finite electric charge is required.' };
    }
    if (!Number.isInteger(entry.charge)) {
        return { supported: false, reason: 'Fractional quark charge is not representable by the current integer-charge particle engine.' };
    }
    if (entry.charge < -128 || entry.charge > 127) {
        return { supported: false, reason: 'Electric charge is outside the native signed 8-bit range [-128, 127].' };
    }
    if (!Number.isFinite(entry.mass_mev) || entry.mass_mev <= 0) {
        return { supported: false, reason: 'A finite positive reference mass is required; massless dynamics are unavailable.' };
    }
    if (entry.charge === 0) {
        return { supported: false, reason: 'Neutral reference entries are not supported by the current charged-particle catalog injection path.' };
    }
    return { supported: true, reason: 'Available as an imported classical reference; quantum statistics, particle identity, and decay are not recovered.' };
}

export function getSimulableParticles() {
    return PARTICLES.filter(p => getCatalogSimulationSupport(p).supported);
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
