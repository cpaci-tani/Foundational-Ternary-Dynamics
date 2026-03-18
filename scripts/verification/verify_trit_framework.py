"""
Trit Information Theory Verification

Verifies all mathematical claims in the Trit Framework / PbR investigation,
including the G* = sqrt(2pi)*theta_3^2 identity, trit distribution, Shannon
entropy, and lepton mass log-formulas.

Framework: FTD v5.17 + Trit Information Theory Extension
"""

import sys
import math
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))

import numpy as np
from scipy.special import gamma

# Ensure UTF-8 output on Windows
if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8')

from scripts.constants import (
    G_STAR, GAMMA_QUARTER, X_PLUS, X_MINUS,
    N_c, N_base, b_3, N_eff,
    Experimental, ppm_error, percent_error
)


# =============================================================================
# SECTION 1: FUNDAMENTAL CONSTANTS
# =============================================================================

def verify_nome_and_theta():
    """
    Verify the self-dual nome q = e^{-pi} and theta_3 at this nome.

    theta_3(q) = sum_{n=-inf}^{inf} q^{n^2} = 1 + 2*sum_{n=1}^{inf} q^{n^2}

    At q = e^{-pi}, theta_3 has a closed form:
        theta_3(e^{-pi}) = pi^{1/4} / Gamma(3/4)

    This is the UNIQUE Fourier self-dual point: theta_3 equals its own
    Fourier transform here.
    """
    print("=" * 70)
    print("SECTION 1: SELF-DUAL NOME AND THETA FUNCTION")
    print("=" * 70)

    # Nome
    q = math.exp(-math.pi)
    print(f"\n  Nome: q = e^(-pi) = {q:.15f}")

    # Theta_3 via series (converges extremely fast at this nome)
    theta3_series = 1.0
    for n in range(1, 50):
        term = 2.0 * q**(n*n)
        theta3_series += term
        if abs(term) < 1e-30:
            print(f"  Series converged at n = {n} (last term = {term:.2e})")
            break

    # Theta_3 via gamma identity
    theta3_gamma = math.pi**(1/4) / math.gamma(3/4)

    # Cross-check
    diff = abs(theta3_series - theta3_gamma)

    print(f"\n  theta_3(e^(-pi)) via series:  {theta3_series:.15f}")
    print(f"  theta_3(e^(-pi)) via Gamma:   {theta3_gamma:.15f}")
    print(f"  Difference:                    {diff:.2e}")
    print(f"  Match: {'YES' if diff < 1e-12 else 'NO'}")

    return q, theta3_gamma


def verify_gstar_identity(q, theta3):
    """
    Verify the KEY IDENTITY: G* = sqrt(2pi) * theta_3(e^{-pi})^2

    This connects:
    - G* (lemniscatic constant, geometry of self-crossing)
    - theta_3 at self-dual nome (Fourier self-duality, information theory)
    - sqrt(2pi) (Gaussian normalization)

    Status: [THEOREM] - exact mathematical identity
    """
    print("\n" + "=" * 70)
    print("SECTION 2: THE KEY IDENTITY  G* = sqrt(2pi) * theta_3^2")
    print("=" * 70)

    # G* direct computation
    gstar_direct = math.sqrt(2) * math.gamma(0.25)**2 / (2 * math.pi)

    # G* via theta identity
    gstar_theta = math.sqrt(2 * math.pi) * theta3**2

    diff = abs(gstar_direct - gstar_theta)
    rel_err = diff / gstar_direct

    print(f"\n  G* (direct: sqrt(2)*Gamma(1/4)^2/(2pi)): {gstar_direct:.15f}")
    print(f"  G* (theta:  sqrt(2pi)*theta_3^2):         {gstar_theta:.15f}")
    print(f"  Absolute difference:                       {diff:.2e}")
    print(f"  Relative error:                            {rel_err:.2e}")
    print(f"  Status: {'[THEOREM] EXACT IDENTITY' if rel_err < 1e-12 else '[FAILED]'}")

    # Show the decomposition
    print(f"\n  Decomposition:")
    print(f"    sqrt(2pi)     = {math.sqrt(2*math.pi):.15f}")
    print(f"    theta_3^2     = {theta3**2:.15f}")
    print(f"    Product       = {math.sqrt(2*math.pi) * theta3**2:.15f}")

    return gstar_direct


# =============================================================================
# SECTION 2: TRIT PROBABILITY DISTRIBUTION
# =============================================================================

def verify_trit_distribution(q, theta3):
    """
    Verify the lemniscatic trit distribution derived from theta_3 decomposition.

    theta_3(q) = 1 + 2q + 2q^4 + 2q^9 + ...
                 ^^^   ^^^   ^^^^^^^^^^^^^^^^^
                 DC   n=1      n >= 2

    Normalizing by theta_3:
        P1 (void/memory)  = 1 / theta_3         (DC component)
        P0 (manifest)     = 2q / theta_3         (first harmonic)
        P2 (higher)       = (theta_3 - 1 - 2q) / theta_3  (remainder)

    Status: [THEOREM] - follows from Fourier decomposition
    """
    print("\n" + "=" * 70)
    print("SECTION 3: LEMNISCATIC TRIT DISTRIBUTION")
    print("=" * 70)

    # Raw theta_3 components
    dc = 1.0                    # n = 0 term
    first_harmonic = 2 * q      # n = +/- 1 terms
    higher = theta3 - dc - first_harmonic  # n >= 2 terms

    print(f"\n  Theta_3 decomposition:")
    print(f"    DC (n=0):          {dc:.15f}")
    print(f"    First harmonic:    {first_harmonic:.15f}")
    print(f"    Higher (n>=2):     {higher:.15e}")
    print(f"    Sum check:         {dc + first_harmonic + higher:.15f} = theta_3 = {theta3:.15f}")

    # Normalized probabilities
    P1 = dc / theta3             # Void / Memory state
    P0 = first_harmonic / theta3 # Manifest state
    P2 = higher / theta3         # Higher / Imaginary state

    print(f"\n  Trit probabilities (normalized by theta_3):")
    print(f"    P0 (Manifest) = 2q/theta_3     = {P0:.15f}")
    print(f"    P1 (Void)     = 1/theta_3      = {P1:.15f}")
    print(f"    P2 (Higher)   = remainder       = {P2:.15e}")
    print(f"    Sum check:    P0+P1+P2          = {P0+P1+P2:.15f}")

    # Test P0 ~ 1/(4*pi)
    p0_test = 1.0 / (4 * math.pi)
    p0_err = percent_error(P0, p0_test)
    print(f"\n  Notable relationship:")
    print(f"    P0                = {P0:.10f}")
    print(f"    1/(4*pi)          = {p0_test:.10f}")
    print(f"    Error:              {p0_err:.4f}%")
    print(f"    Equivalently: P0 * 2pi = {P0 * 2 * math.pi:.10f} ~ 1/2")
    print(f"    Status: {'[CONJECTURED] - 0.032%' if p0_err < 0.05 else '[APPROXIMATE]'}")

    return P0, P1, P2


def verify_shannon_entropy(P0, P1, P2):
    """
    Verify the Shannon entropy of the lemniscatic trit.

    H = -sum(p_i * log2(p_i)) for i in {0, 1, 2}

    Status: [THEOREM] - standard information theory applied to derived distribution
    """
    print("\n" + "=" * 70)
    print("SECTION 4: SHANNON ENTROPY AND REDUNDANCY")
    print("=" * 70)

    probs = [P0, P1, P2]
    H = sum(-p * math.log2(p) for p in probs if p > 0)
    H_max = math.log2(3)
    R = H_max - H

    print(f"\n  Shannon entropy:")
    print(f"    H = -sum(p_i * log2(p_i))")
    print(f"      = -{P0:.6f}*log2({P0:.6f}) - {P1:.6f}*log2({P1:.6f}) - {P2:.2e}*log2({P2:.2e})")
    print(f"      = {-P0*math.log2(P0):.6f} + {-P1*math.log2(P1):.6f} + {-P2*math.log2(P2):.6f}")
    print(f"    H = {H:.15f} bits")

    print(f"\n  Maximum entropy:")
    print(f"    H_max = log2(3) = {H_max:.15f} bits")

    print(f"\n  Redundancy:")
    print(f"    R = H_max - H = {R:.15f} bits")

    # Test R ~ theta_3^2
    theta3 = math.pi**(1/4) / math.gamma(3/4)
    r_test = theta3**2
    r_err = percent_error(R, r_test)
    print(f"\n  Notable relationship:")
    print(f"    R           = {R:.10f}")
    print(f"    theta_3^2   = {r_test:.10f}")
    print(f"    Error:        {r_err:.4f}%")
    print(f"    Status: [OBSERVED] - approximate ({r_err:.2f}%), not exact")

    # CORRECTION: Address user's original claims
    print(f"\n  CORRECTIONS to original PbR claims:")
    print(f"    Claimed Total = 2.000    | Actual H_max = {H_max:.6f} (NOT 2)")
    print(f"    Claimed Ash   = 0.426    | Actual H     = {H:.6f} (NOT 0.426)")
    print(f"    Claimed Cap   = 1.574    | Actual R     = {R:.6f} (NOT 1.574)")

    return H, H_max, R


# =============================================================================
# SECTION 3: LEPTON MASS FORMULAS
# =============================================================================

def verify_lepton_mass_formulas(gstar):
    """
    Verify two types of lepton mass formulas and compare their precision.

    LOG FORMULAS (new, via G*):
        m_mu / m_e  = exp((9/5) * G*)     where 9/5 = N_c^2 / (N_eff - 2*N_base)
        m_tau / m_e = exp((11/4) * G*)     where 11/4 = (b_3 + N_base) / N_base

    INTEGER FORMULAS (existing FTD):
        m_mu / m_e  = 3*7*(7+3) - 3 = 207
        m_tau / m_e = (13+4)*207 - 2*3*7 = 3477

    Status: [OBSERVED] - numerical coincidences with framework integer decompositions
    """
    print("\n" + "=" * 70)
    print("SECTION 5: LEPTON MASS FORMULAS COMPARISON")
    print("=" * 70)

    m_e = Experimental.m_electron   # 0.51099895 MeV
    m_mu = Experimental.m_muon      # 105.6583755 MeV
    m_tau = Experimental.m_tau       # 1776.86 MeV

    # Exact ratios
    ratio_mu = m_mu / m_e
    ratio_tau = m_tau / m_e

    print(f"\n  Experimental mass ratios:")
    print(f"    m_mu/m_e  = {ratio_mu:.6f}")
    print(f"    m_tau/m_e = {ratio_tau:.2f}")
    print(f"    ln(m_mu/m_e) = {math.log(ratio_mu):.10f}")
    print(f"    ln(m_tau/m_e) = {math.log(ratio_tau):.10f}")

    # --- LOG FORMULAS ---
    print(f"\n  LOG FORMULAS (via G* = {gstar:.10f}):")

    c_mu = 9.0 / 5.0     # = N_c^2 / (N_eff - 2*N_base) = 9/5
    c_tau = 11.0 / 4.0    # = (b_3 + N_base) / N_base = 11/4

    print(f"    c_mu  = 9/5  = N_c^2/(N_eff - 2*N_base) = {c_mu:.4f}")
    print(f"    c_tau = 11/4 = (b_3 + N_base)/N_base     = {c_tau:.4f}")

    pred_mu_log = math.exp(c_mu * gstar)
    pred_tau_log = math.exp(c_tau * gstar)

    err_mu_log = percent_error(pred_mu_log, ratio_mu)
    err_tau_log = percent_error(pred_tau_log, ratio_tau)

    print(f"\n    m_mu/m_e = exp((9/5)*G*)  = exp({c_mu*gstar:.6f}) = {pred_mu_log:.4f}")
    print(f"      Experimental: {ratio_mu:.4f}")
    print(f"      Error: {err_mu_log:.4f}%")

    print(f"\n    m_tau/m_e = exp((11/4)*G*) = exp({c_tau*gstar:.6f}) = {pred_tau_log:.2f}")
    print(f"      Experimental: {ratio_tau:.2f}")
    print(f"      Error: {err_tau_log:.4f}%")

    # --- INTEGER FORMULAS ---
    print(f"\n  INTEGER FORMULAS (existing FTD):")

    pred_mu_int = N_c * b_3 * (b_3 + N_c) - N_c  # 3*7*10 - 3 = 207
    pred_tau_int = (N_eff + N_base) * pred_mu_int - 2 * N_c * b_3  # 17*207 - 42 = 3477

    err_mu_int = percent_error(pred_mu_int, ratio_mu)
    err_tau_int = percent_error(pred_tau_int, ratio_tau)

    print(f"    m_mu/m_e = N_c*b_3*(b_3+N_c) - N_c = 3*7*10 - 3 = {pred_mu_int}")
    print(f"      Experimental: {ratio_mu:.4f}")
    print(f"      Error: {err_mu_int:.4f}%")

    print(f"\n    m_tau/m_e = (N_eff+N_base)*207 - 2*N_c*b_3 = 17*207 - 42 = {pred_tau_int}")
    print(f"      Experimental: {ratio_tau:.2f}")
    print(f"      Error: {err_tau_int:.4f}%")

    # --- COMPARISON ---
    print(f"\n  COMPARISON:")
    print(f"  {'Formula':<25} {'Muon Error':>12} {'Tau Error':>12}")
    print(f"  {'-'*25} {'-'*12} {'-'*12}")
    print(f"  {'Log (G*-based)':<25} {err_mu_log:>11.4f}% {err_tau_log:>11.4f}%")
    print(f"  {'Integer (FTD)':<25} {err_mu_int:>11.4f}% {err_tau_int:>11.4f}%")
    print(f"\n  Verdict: Integer formulas are MORE PRECISE for both particles.")
    print(f"  The log formulas offer a different PERSPECTIVE (geometric/RG),")
    print(f"  not a better FIT.")

    # --- SECONDARY RELATIONSHIPS ---
    print(f"\n  Secondary relationships between c_mu and c_tau:")

    diff = c_tau - c_mu
    prod = c_mu * c_tau

    gstar_over_pi = gstar / math.pi

    print(f"    c_tau - c_mu = {diff:.6f}")
    print(f"    G*/pi        = {gstar_over_pi:.6f}")
    print(f"    Error: {percent_error(diff, gstar_over_pi):.2f}%")

    print(f"\n    c_mu * c_tau = {prod:.6f}")
    print(f"    Nearest int:   5")
    print(f"    Error: {percent_error(prod, 5.0):.2f}%")


# =============================================================================
# SECTION 4: KOIDE FORMULA
# =============================================================================

def verify_koide():
    """
    Verify the Koide formula Q = (m_e + m_mu + m_tau) / (sqrt(m_e) + sqrt(m_mu) + sqrt(m_tau))^2

    Koide conjectured Q = 2/3 exactly.

    Status: [OBSERVED] - independent of FTD; confirmed to 0.001%
    """
    print("\n" + "=" * 70)
    print("SECTION 6: KOIDE FORMULA")
    print("=" * 70)

    m_e = Experimental.m_electron
    m_mu = Experimental.m_muon
    m_tau = Experimental.m_tau

    numerator = m_e + m_mu + m_tau
    denominator = (math.sqrt(m_e) + math.sqrt(m_mu) + math.sqrt(m_tau))**2
    Q = numerator / denominator

    err = percent_error(Q, 2.0/3.0)

    print(f"\n  Q = (m_e + m_mu + m_tau) / (sqrt(m_e) + sqrt(m_mu) + sqrt(m_tau))^2")
    print(f"    Numerator:   {numerator:.6f} MeV")
    print(f"    Denominator: {denominator:.6f} MeV")
    print(f"    Q = {Q:.10f}")
    print(f"    2/3 = {2/3:.10f}")
    print(f"    Error: {err:.4f}%")
    print(f"    Status: [OBSERVED] - {err:.4f}% from exact 2/3")


# =============================================================================
# SECTION 5: SPECULATIVE RELATIONSHIPS
# =============================================================================

def verify_speculative_relationships(H, P0, gstar):
    """
    Test speculative relationships that may be unit-dependent or approximate.

    Status: [SPECULATIVE] unless noted otherwise
    """
    print("\n" + "=" * 70)
    print("SECTION 7: SPECULATIVE RELATIONSHIPS")
    print("=" * 70)

    m_e = Experimental.m_electron  # MeV

    # H * 4/pi ~ m_e (MeV)
    h_test = H * 4.0 / math.pi
    h_err = percent_error(h_test, m_e)
    print(f"\n  1. H * 4/pi vs m_e:")
    print(f"     H * 4/pi = {h_test:.6f}")
    print(f"     m_e      = {m_e:.6f} MeV")
    print(f"     Error: {h_err:.4f}%")
    print(f"     Status: [SPECULATIVE] - unit dependent (requires MeV)")

    # Consciousness threshold / H ~ 2pi
    K_C = 3.597  # sqrt(G*^3/2) from consciousness quadratic
    kc_h_ratio = K_C / H
    kc_err = percent_error(kc_h_ratio, 2 * math.pi)
    print(f"\n  2. K_C / H vs 2pi:")
    print(f"     K_C = sqrt(G*^3/2) = {K_C:.6f}")
    print(f"     K_C / H = {kc_h_ratio:.6f}")
    print(f"     2pi     = {2*math.pi:.6f}")
    print(f"     Error: {kc_err:.2f}%")
    print(f"     Status: [SPECULATIVE] - {kc_err:.1f}% off")

    # Mass curvature ~ H*pi
    ln_mu = math.log(Experimental.m_muon / m_e)
    ln_tau = math.log(Experimental.m_tau / m_e)
    c_mu_actual = ln_mu / gstar
    c_tau_actual = ln_tau / gstar
    curvature = c_tau_actual - c_mu_actual - (11/4 - 9/5)
    print(f"\n  3. Actual log coefficients:")
    print(f"     c_mu (actual)  = ln(m_mu/m_e)/G* = {c_mu_actual:.10f}")
    print(f"     c_mu (claimed) = 9/5             = {9/5:.10f}")
    print(f"     c_tau (actual)  = ln(m_tau/m_e)/G* = {c_tau_actual:.10f}")
    print(f"     c_tau (claimed) = 11/4             = {11/4:.10f}")

    # 92% void fraction vs cosmological dark fraction?
    print(f"\n  4. Void fraction:")
    print(f"     P1 (void) = {1.0/math.pi**(1/4)*math.gamma(3/4):.6f}")
    dark_frac = 0.268 + 0.684  # dark matter + dark energy from Planck
    print(f"     Dark matter + dark energy fraction: {dark_frac:.3f}")
    print(f"     Status: [SPECULATIVE] - different context, no clear mechanism")


# =============================================================================
# SECTION 6: FOURIER SELF-DUALITY
# =============================================================================

def verify_fourier_self_duality():
    """
    Verify the Fourier self-duality of theta_3 at q = e^{-pi}.

    The Jacobi identity (Poisson summation):
        theta_3(e^{-pi*t}) = (1/sqrt(t)) * theta_3(e^{-pi/t})

    At t = 1: theta_3(e^{-pi}) = theta_3(e^{-pi})  (trivially self-dual)

    The non-trivial content: this is the UNIQUE nome where the function
    equals its own Fourier transform.

    Status: [THEOREM] - classical result (Jacobi)
    """
    print("\n" + "=" * 70)
    print("SECTION 8: FOURIER SELF-DUALITY VERIFICATION")
    print("=" * 70)

    # Test at t = 1 (trivial self-dual point)
    q1 = math.exp(-math.pi)
    theta3_at_q1 = sum(q1**(n*n) for n in range(-30, 31))

    print(f"\n  Poisson summation: theta_3(e^(-pi*t)) = (1/sqrt(t)) * theta_3(e^(-pi/t))")
    print(f"\n  At t = 1 (self-dual point):")
    print(f"    LHS = theta_3(e^(-pi)) = {theta3_at_q1:.15f}")
    print(f"    RHS = (1/sqrt(1)) * theta_3(e^(-pi)) = {theta3_at_q1:.15f}")
    print(f"    Self-dual: YES (by definition)")

    # Test at t != 1 to show transformation works
    for t in [0.5, 2.0, 0.25]:
        q_lhs = math.exp(-math.pi * t)
        q_rhs = math.exp(-math.pi / t)
        theta_lhs = sum(q_lhs**(n*n) for n in range(-50, 51))
        theta_rhs = sum(q_rhs**(n*n) for n in range(-50, 51))
        transformed = theta_rhs / math.sqrt(t)
        diff = abs(theta_lhs - transformed)
        print(f"\n  At t = {t}:")
        print(f"    theta_3(e^(-pi*{t}))        = {theta_lhs:.12f}")
        print(f"    (1/sqrt({t}))*theta_3(e^(-pi/{t})) = {transformed:.12f}")
        print(f"    Difference: {diff:.2e}")

    print(f"\n  Meaning: The Jacobi theta function at q = e^(-pi) is the")
    print(f"  spectral analogue of the lemniscate's geometric self-intersection.")
    print(f"  Self-reference in geometry = self-duality in information theory.")


# =============================================================================
# SECTION 7: COMPREHENSIVE RESULTS TABLE
# =============================================================================

def print_results_table(gstar, H, H_max, R, P0, P1, P2):
    """Print the complete verified results ranked by precision."""

    print("\n" + "=" * 70)
    print("COMPREHENSIVE RESULTS TABLE (ranked by precision)")
    print("=" * 70)

    m_e = Experimental.m_electron
    m_mu = Experimental.m_muon
    m_tau = Experimental.m_tau

    # Compute all values
    theta3 = math.pi**(1/4) / math.gamma(3/4)
    gstar_theta = math.sqrt(2 * math.pi) * theta3**2
    gstar_direct = math.sqrt(2) * math.gamma(0.25)**2 / (2 * math.pi)

    ratio_mu = m_mu / m_e
    ratio_tau = m_tau / m_e

    Q_koide = (m_e + m_mu + m_tau) / (math.sqrt(m_e) + math.sqrt(m_mu) + math.sqrt(m_tau))**2

    results = []

    # 1. G* identity (exact)
    err1 = abs(gstar_theta - gstar_direct) / gstar_direct * 100
    results.append(("G* = sqrt(2pi)*theta_3^2", "Exact identity", err1, "[THEOREM]"))

    # 2. 1/alpha from master quadratic
    err2 = ppm_error(X_PLUS, Experimental.alpha_inv) / 10000
    results.append(("1/alpha = x_+ from quadratic", f"{X_PLUS:.6f} vs {Experimental.alpha_inv:.6f}", err2, "[OBSERVED]"))

    # 3. Koide Q = 2/3
    err3 = percent_error(Q_koide, 2/3)
    results.append(("Koide Q = 2/3", f"{Q_koide:.8f} vs {2/3:.8f}", err3, "[OBSERVED]"))

    # 4. P0 ~ 1/(4pi)
    err4 = percent_error(P0, 1/(4*math.pi))
    results.append(("P0 = 1/(4pi)", f"{P0:.8f} vs {1/(4*math.pi):.8f}", err4, "[CONJECTURED]"))

    # 5. Muon log formula
    pred_mu = math.exp(9/5 * gstar_direct)
    err5 = percent_error(pred_mu, ratio_mu)
    results.append(("ln(m_mu/m_e)/G* = 9/5", f"{pred_mu:.4f} vs {ratio_mu:.4f}", err5, "[OBSERVED]"))

    # 6. H * 4/pi ~ m_e
    err6 = percent_error(H * 4/math.pi, m_e)
    results.append(("H * 4/pi = m_e (MeV)", f"{H*4/math.pi:.6f} vs {m_e:.6f}", err6, "[SPECULATIVE]"))

    # 7. Tau log formula
    pred_tau = math.exp(11/4 * gstar_direct)
    err7 = percent_error(pred_tau, ratio_tau)
    results.append(("ln(m_tau/m_e)/G* = 11/4", f"{pred_tau:.2f} vs {ratio_tau:.2f}", err7, "[OBSERVED]"))

    # 8. R ~ theta_3^2
    err8 = percent_error(R, theta3**2)
    results.append(("R = theta_3^2", f"{R:.6f} vs {theta3**2:.6f}", err8, "[OBSERVED]"))

    # Sort by precision
    results.sort(key=lambda x: x[2])

    print(f"\n  {'#':<3} {'Claim':<30} {'Error':>10} {'Status':<15}")
    print(f"  {'-'*3} {'-'*30} {'-'*10} {'-'*15}")
    for i, (claim, detail, err, status) in enumerate(results, 1):
        if err < 0.0001:
            err_str = f"{err:.2e}%"
        else:
            err_str = f"{err:.4f}%"
        print(f"  {i:<3} {claim:<30} {err_str:>10} {status:<15}")

    print(f"\n  Notes:")
    print(f"  - #1 is an EXACT mathematical identity, not a numerical coincidence")
    print(f"  - #6 is unit-dependent (works only in MeV) and likely coincidental")
    print(f"  - #4 and #8 may be approximate, not exact")


# =============================================================================
# MAIN
# =============================================================================

def run_all_verifications():
    """Run the complete Trit Framework verification suite."""

    print("\n" + "=" * 70)
    print("TRIT INFORMATION THEORY - COMPLETE VERIFICATION")
    print("Framework: FTD v5.17 + PbR Extension")
    print("=" * 70)

    # Section 1: Nome and theta function
    q, theta3 = verify_nome_and_theta()

    # Section 2: G* identity
    gstar = verify_gstar_identity(q, theta3)

    # Section 3: Trit distribution
    P0, P1, P2 = verify_trit_distribution(q, theta3)

    # Section 4: Shannon entropy
    H, H_max, R = verify_shannon_entropy(P0, P1, P2)

    # Section 5: Lepton mass formulas
    verify_lepton_mass_formulas(gstar)

    # Section 6: Koide formula
    verify_koide()

    # Section 7: Speculative relationships
    verify_speculative_relationships(H, P0, gstar)

    # Section 8: Fourier self-duality
    verify_fourier_self_duality()

    # Summary table
    print_results_table(gstar, H, H_max, R, P0, P1, P2)

    print("\n" + "=" * 70)
    print("VERIFICATION COMPLETE")
    print("=" * 70)
    print(f"\nKey identity verified: G* = sqrt(2pi) * theta_3(e^(-pi))^2  [EXACT]")
    print(f"Core discovery: Self-reference in geometry = self-duality in information theory")
    print()


if __name__ == "__main__":
    run_all_verifications()
