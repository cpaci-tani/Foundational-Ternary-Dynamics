/**
 * Decay Rates Module — particle lifetimes from the ontic chain.
 *
 * All masses from framework integers {N_c=3, N_base=4, b_3=7, N_eff=13}.
 * Functional forms from standard Fermi theory (parametric insertions).
 * Energies in MeV, times in seconds.
 */

import {
    ALPHA, K_B, PI_FTD, N_C, B_3, N_EFF, N_BASE,
    MU_RATIO, TAU_RATIO, M_PROTON,
    SIN2_WEINBERG, M_W, M_Z, G_FERMI, HBAR_C_MEV_FM
} from './constants.js';

// ── Masses from Ontic Chain ──────────────────────────────────────────

export const M_ELECTRON = K_B;                      // 0.511 MeV
export const M_MUON     = K_B * MU_RATIO;           // 105.7 MeV
export const M_TAU      = K_B * TAU_RATIO;          // 1776.7 MeV (0.007%)
export const M_PROTON_PHYS = 938.272;                // Physical proton mass (PDG)
export const M_NEUTRON  = M_PROTON_PHYS + 1.293;    // neutron = proton + 1.293 MeV
export const M_PION_CHARGED = 139.57;               // MeV (input)
export const M_PION_NEUTRAL = 135.0;                // MeV (input)

// Conversion: hbar in MeV*s
const HBAR_MEV_S = 6.582119569e-22; // MeV·s

// ── Fermi Coupling ───────────────────────────────────────────────────

/**
 * Fermi coupling constant G_F in MeV^-2.
 * G_F / (hbar*c)^3 = 1.166e-5 GeV^-2 = 1.166e-11 MeV^-2
 * From ontic chain: G_F = pi*alpha / (sqrt(2) * M_W^2)
 * M_W in MeV for consistent units.
 */
const M_W_MEV = M_W * 1000; // GeV -> MeV
const G_F_MEV = PI_FTD * ALPHA / (Math.sqrt(2) * M_W_MEV * M_W_MEV);

// ── Muon Lifetime ────────────────────────────────────────────────────

/**
 * Muon lifetime from Fermi theory.
 * tau_mu = 192 * pi^3 * hbar / (G_F^2 * m_mu^5)
 *
 * Experimental: 2.1969811e-6 s
 */
export function muonLifetime() {
    const numerator = 192 * PI_FTD * PI_FTD * PI_FTD;
    const denominator = G_F_MEV * G_F_MEV * Math.pow(M_MUON, 5);
    return numerator * HBAR_MEV_S / denominator;
}

/**
 * Muon decay width Gamma = hbar / tau.
 */
export function muonDecayWidth() {
    return HBAR_MEV_S / muonLifetime();
}

// ── Tau Lifetime ─────────────────────────────────────────────────────

/**
 * Tau lifetime scaled from muon by mass ratio.
 * tau_tau = tau_mu * (m_mu / m_tau)^5 * BR_correction
 *
 * BR_correction accounts for hadronic channels: ~1/0.1785 for leptonic BR
 * Experimental: 2.903e-13 s
 */
export function tauLifetime() {
    const massRatio = M_MUON / M_TAU;
    // Leptonic branching ratio ~ 17.85% each for e and mu channels
    const BR_leptonic = 0.1785;
    return muonLifetime() * Math.pow(massRatio, 5) * BR_leptonic;
}

// ── Neutron Lifetime ─────────────────────────────────────────────────

/**
 * Free neutron lifetime from Fermi theory.
 * tau_n = 2*pi^3*hbar / (G_F^2 * m_e^5 * |V_ud|^2 * f_n)
 * where f_n is the phase space factor and V_ud is CKM element.
 *
 * Experimental: 878.4 ± 0.5 s
 */
export function neutronLifetime() {
    const dM = M_NEUTRON - M_PROTON; // mass difference ~1.293 MeV
    const V_ud = 0.974;  // CKM matrix element (from flavor physics)
    // Phase space factor for neutron beta decay
    const x = dM / M_ELECTRON;
    // f(x) ≈ x*sqrt(x^2-1)*(x^2 - 9/4*x + 4/3) — Wilkinson approximation
    const f_n = 1.6887; // standard phase space integral value

    const numerator = 2.0 * PI_FTD * PI_FTD * PI_FTD;
    const denominator = G_F_MEV * G_F_MEV * Math.pow(M_ELECTRON, 5) * V_ud * V_ud * f_n * (1 + 3 * 1.2756 * 1.2756);
    // Factor (1 + 3*g_A^2) where g_A = 1.2756 is the axial coupling
    return numerator * HBAR_MEV_S / denominator;
}

// ── Pion Lifetime ────────────────────────────────────────────────────

/**
 * Charged pion lifetime.
 * tau_pi = hbar / (G_F^2 * f_pi^2 * m_mu^2 * m_pi * (1 - m_mu^2/m_pi^2)^2 / (8*pi))
 *
 * f_pi ≈ 130.2 MeV (pion decay constant — input)
 * Experimental: 2.6033e-8 s
 */
export function pionLifetime() {
    const f_pi = 130.2; // MeV (pion decay constant)
    const ratio = M_MUON * M_MUON / (M_PION_CHARGED * M_PION_CHARGED);
    const width = G_F_MEV * G_F_MEV * f_pi * f_pi * M_MUON * M_MUON *
                  M_PION_CHARGED * (1 - ratio) * (1 - ratio) / (8.0 * PI_FTD);
    return HBAR_MEV_S / width;
}

// ── Gamow Factor (Alpha Decay) ───────────────────────────────────────

/**
 * Gamow tunneling factor for alpha decay.
 * T = exp(-2*pi*Z*alpha * sqrt(2*m_red*c^2 / Q))
 *
 * @param {number} Z - daughter nucleus charge
 * @param {number} Q - Q-value in MeV
 * @returns {number} tunneling probability (dimensionless)
 */
export function gamowFactor(Z, Q) {
    if (Q <= 0) return 0;
    // Alpha particle: Z_alpha = 2, A_alpha = 4
    const m_alpha = 4 * M_PROTON / 1000; // rough alpha mass in GeV
    const m_alpha_MeV = 4 * 931.494; // 4 AMU in MeV
    const m_daughter_MeV = Z * 931.494; // approximate
    const m_red = m_alpha_MeV * m_daughter_MeV / (m_alpha_MeV + m_daughter_MeV);
    const eta = Z * 2 * ALPHA * Math.sqrt(m_red / (2.0 * Q));
    return Math.exp(-2.0 * PI_FTD * eta);
}

// ── Particle Info Catalog ────────────────────────────────────────────

/**
 * Get decay info for a particle type.
 * @param {string} name - particle name
 * @returns {{ name, mass_MeV, lifetime_s, primary_channel, branching }}
 */
export function particleDecayInfo(name) {
    const catalog = {
        'muon': {
            name: 'Muon (μ⁻)',
            mass_MeV: M_MUON,
            lifetime_s: muonLifetime(),
            primary_channel: 'μ⁻ → e⁻ + ν̄ₑ + νμ',
            branching: '~100%',
            origin: `m_μ = m_e × ${MU_RATIO} (3×7×(7+3)−3)`
        },
        'tau': {
            name: 'Tau (τ⁻)',
            mass_MeV: M_TAU,
            lifetime_s: tauLifetime(),
            primary_channel: 'τ⁻ → e⁻/μ⁻ + ν̄ + ντ (leptonic)',
            branching: '~35.7%',
            origin: `m_τ = m_e × ${TAU_RATIO} ((13+4)×207−2×3×7)`
        },
        'neutron': {
            name: 'Neutron (n)',
            mass_MeV: M_NEUTRON,
            lifetime_s: neutronLifetime(),
            primary_channel: 'n → p + e⁻ + ν̄ₑ',
            branching: '~100%',
            origin: 'Composite (udd triad)'
        },
        'pion': {
            name: 'Pion (π⁺)',
            mass_MeV: M_PION_CHARGED,
            lifetime_s: pionLifetime(),
            primary_channel: 'π⁺ → μ⁺ + νμ',
            branching: '~99.99%',
            origin: 'ud̄ bound state'
        },
        'electron': {
            name: 'Electron (e⁻)',
            mass_MeV: M_ELECTRON,
            lifetime_s: Infinity,
            primary_channel: 'Stable',
            branching: 'N/A',
            origin: `m_e = K_B = ${K_B} MeV`
        },
        'proton': {
            name: 'Proton (p)',
            mass_MeV: 938.272,  // Physical mass (PDG); framework scale = K_B * PROTON_RATIO
            lifetime_s: Infinity,
            primary_channel: 'Stable (τ > 10³⁴ yr)',
            branching: 'N/A',
            origin: `Physical: 938.3 MeV; FTD scale: ${M_PROTON.toFixed(1)} MeV`
        }
    };
    return catalog[name] || null;
}

/**
 * Return all particle names in the catalog.
 */
export function particleNames() {
    return ['electron', 'muon', 'tau', 'pion', 'neutron', 'proton'];
}

// ── DOM Rendering ────────────────────────────────────────────────────

/**
 * Render decay rates table into a container.
 */
export function renderDecayRates(container) {
    const particles = particleNames();

    let rows = '';
    for (const p of particles) {
        const info = particleDecayInfo(p);
        if (!info) continue;

        const mass = info.mass_MeV < 1 ? `${(info.mass_MeV * 1e3).toFixed(1)} keV`
                   : info.mass_MeV < 1000 ? `${info.mass_MeV.toFixed(1)} MeV`
                   : `${(info.mass_MeV / 1000).toFixed(3)} GeV`;

        const tau = info.lifetime_s === Infinity ? 'Stable'
                  : info.lifetime_s > 1 ? `${info.lifetime_s.toFixed(0)} s`
                  : info.lifetime_s > 1e-6 ? `${(info.lifetime_s * 1e6).toFixed(2)} μs`
                  : info.lifetime_s > 1e-9 ? `${(info.lifetime_s * 1e9).toFixed(2)} ns`
                  : info.lifetime_s > 1e-12 ? `${(info.lifetime_s * 1e12).toFixed(2)} ps`
                  : `${(info.lifetime_s * 1e15).toFixed(2)} fs`;

        rows += `<tr>
            <td style="color:var(--text-primary)">${info.name}</td>
            <td style="color:var(--accent)">${mass}</td>
            <td>${tau}</td>
            <td style="font-size:9px;color:var(--text-muted)">${info.primary_channel}</td>
        </tr>`;
    }

    container.innerHTML = `
        <div class="card-title">Decay Rates (Fermi theory + ontic masses)</div>
        <table style="width:100%;font-size:10px;border-collapse:collapse">
            <thead>
                <tr style="border-bottom:1px solid var(--bg-card);color:var(--text-muted)">
                    <th style="text-align:left;padding:2px 4px">Particle</th>
                    <th style="text-align:left;padding:2px 4px">Mass</th>
                    <th style="text-align:left;padding:2px 4px">Lifetime</th>
                    <th style="text-align:left;padding:2px 4px">Channel</th>
                </tr>
            </thead>
            <tbody>${rows}</tbody>
        </table>
        <div style="margin-top:6px;font-size:9px;color:var(--text-muted)">
            Masses from integers {N<sub>c</sub>=3, b<sub>3</sub>=7, N<sub>eff</sub>=13} via ontic chain.
            Decay formulas: standard Fermi theory (parametric insertion).
        </div>`;
}
