#!/usr/bin/env python3
import sys, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
"""
Verify the electron mass derivation in FTD.

Tests whether K_B = m_e = m_P * sqrt(2pi) * (16/3) * alpha^11 is:
  1. Numerically correct (matches constants.py)
  2. Structurally special (exponent 11, coefficient 16/3, normalization sqrt(2pi))
  3. Possibly equivalent to RG running
  4. Honest about its information content

The central question: is the absolute mass scale derived or imposed?

Answer: In lattice natural units (m_P = 1), K_B is fully determined by G* and
the framework integers. The "one external calibration" is a unit convention
(1 lattice energy unit = 0.5100 MeV), not a physical input.

Epistemic status: [SELECTION] — the formula uses only derived quantities,
but the exponent 11, coefficient 16/3, and normalization sqrt(2pi) are each
motivated rather than uniquely forced.
"""

import sys
import os
import math

# Add parent directory to path for imports
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import (
    G_STAR, VARPI_CLASSICAL, ALPHA, ALPHA_INV, X_PLUS, X_MINUS,
    N_c, N_base, b_3, N_eff, M_PLANCK, M_ELECTRON_DERIVED
)

# Experimental values
M_ELECTRON_EXP = 0.51099895e-3  # GeV
M_PLANCK_EXP = 1.220890e19      # GeV

# ===========================================================================
# TEST 1: Numerical verification
# ===========================================================================

def test_formula_numerical():
    """Verify K_B/m_P = sqrt(2pi) * (16/3) * alpha^11 matches constants.py."""
    print("=" * 70)
    print("TEST 1: Numerical verification of K_B = m_P*sqrt(2pi)*(16/3)*alpha^11")
    print("=" * 70)

    # Compute from formula
    ratio = math.sqrt(2 * math.pi) * (N_base**2 / N_c) * ALPHA**11
    m_e_formula = M_PLANCK * ratio

    # Compare to constants.py
    m_e_constants = M_ELECTRON_DERIVED

    rel_error_internal = abs(m_e_formula - m_e_constants) / m_e_constants
    rel_error_exp = abs(m_e_formula * 1000 - M_ELECTRON_EXP * 1000) / (M_ELECTRON_EXP * 1000)

    print(f"  K_B / m_P  = {ratio:.6e}")
    print(f"  m_e (formula)     = {m_e_formula * 1000:.6f} MeV")
    print(f"  m_e (constants.py)= {m_e_constants * 1000:.6f} MeV")
    print(f"  m_e (experiment)  = {M_ELECTRON_EXP * 1000:.6f} MeV")
    print(f"  Internal match    = {rel_error_internal:.2e} (should be < 1e-12)")
    print(f"  Exp. error        = {rel_error_exp * 100:.4f}%")
    print()

    # Decompose the suppression
    print("  Factor decomposition:")
    print(f"    sqrt(2pi)        = {math.sqrt(2 * math.pi):.6f}")
    print(f"    16/3          = {16/3:.6f}")
    print(f"    alpha             = {ALPHA:.10f}")
    print(f"    alpha^11           = {ALPHA**11:.6e}")
    print(f"    sqrt(2pi)*(16/3) = {math.sqrt(2*math.pi) * 16/3:.6f}")
    print(f"    Full ratio    = {ratio:.6e}")
    print()

    assert rel_error_internal < 1e-10, f"Internal mismatch: {rel_error_internal}"
    assert rel_error_exp < 0.003, f"Experimental error too large: {rel_error_exp}"
    print("  PASS: Formula matches constants.py and experiment (0.20% error)")
    print()
    return ratio


# ===========================================================================
# TEST 2: Uniqueness of exponent 11
# ===========================================================================

def test_exponent_uniqueness():
    """Show that n=11 is the only integer giving m_e in the right range."""
    print("=" * 70)
    print("TEST 2: Uniqueness of exponent n = 11")
    print("=" * 70)

    coeff = math.sqrt(2 * math.pi) * (N_base**2 / N_c)
    print(f"  Formula: m = m_P * {coeff:.4f} * alpha^n")
    print(f"  Target:  m_e = 0.51100 MeV = {M_ELECTRON_EXP * 1e3:.5f} MeV")
    print()
    print(f"  {'n':>4s}  {'m (MeV)':>14s}  {'m/m_e':>12s}  {'Status':>20s}")
    print(f"  {'-'*4}  {'-'*14}  {'-'*12}  {'-'*20}")

    best_n = None
    best_error = float('inf')

    for n in range(1, 25):
        m_gev = M_PLANCK * coeff * ALPHA**n
        m_mev = m_gev * 1000
        ratio = m_mev / (M_ELECTRON_EXP * 1000)
        error = abs(ratio - 1)

        if error < best_error:
            best_error = error
            best_n = n

        # Identify what each mass corresponds to
        if m_mev > 1e19:
            status = "(above Planck)"
        elif m_mev > 1e6:
            status = "(above EW scale)"
        elif m_mev > 1e3:
            status = f"~ {m_mev/1000:.1f} GeV"
        elif m_mev > 1:
            status = f"~ {m_mev:.1f} MeV"
        elif m_mev > 1e-3:
            status = f"~ {m_mev*1000:.1f} keV"
        elif m_mev > 1e-6:
            status = f"~ {m_mev*1e6:.1f} eV"
        else:
            status = f"~ {m_mev*1e9:.2f} meV"

        marker = " <<<" if n == 11 else ""
        print(f"  {n:4d}  {m_mev:14.6e}  {ratio:12.6e}  {status:>20s}{marker}")

    print()
    print(f"  Best match: n = {best_n} (error = {best_error*100:.4f}%)")
    assert best_n == 11, f"Best exponent is {best_n}, not 11!"
    print("  PASS: n = 11 is the unique best-matching integer exponent")
    print()


# ===========================================================================
# TEST 3: Uniqueness of coefficient 16/3
# ===========================================================================

def test_coefficient_uniqueness():
    """Test all ratios of framework integers as the coefficient."""
    print("=" * 70)
    print("TEST 3: Uniqueness of coefficient 16/3 = N_base²/N_c")
    print("=" * 70)

    integers = {
        'N_c': N_c, 'N_base': N_base, 'b_3': b_3, 'N_eff': N_eff,
        'N_c²': N_c**2, 'N_base²': N_base**2, 'b_3²': b_3**2,
    }

    norm = math.sqrt(2 * math.pi)
    target_mev = M_ELECTRON_EXP * 1000  # 0.51100 MeV

    results = []
    for name_num, val_num in integers.items():
        for name_den, val_den in integers.items():
            if val_den == 0:
                continue
            c = val_num / val_den
            if c <= 0 or c > 100:
                continue
            m_mev = M_PLANCK * norm * c * ALPHA**11 * 1000
            error = abs(m_mev - target_mev) / target_mev
            formula = f"{name_num}/{name_den}"
            results.append((error, c, formula, m_mev))

    results.sort()
    print(f"  {'Rank':>4s}  {'Formula':>20s}  {'Value':>8s}  {'m_e (MeV)':>12s}  {'Error':>10s}")
    print(f"  {'-'*4}  {'-'*20}  {'-'*8}  {'-'*12}  {'-'*10}")
    for i, (error, c, formula, m_mev) in enumerate(results[:10]):
        marker = " <<<" if abs(c - 16/3) < 0.001 else ""
        print(f"  {i+1:4d}  {formula:>20s}  {c:8.4f}  {m_mev:12.6f}  {error*100:9.4f}%{marker}")

    print()
    best_formula = results[0][2]
    best_value = results[0][1]
    print(f"  Best coefficient: {best_formula} = {best_value:.4f}")
    assert abs(best_value - 16/3) < 0.01, f"Best coefficient is {best_value}, not 16/3!"
    print("  PASS: 16/3 = N_base²/N_c is the best-matching integer ratio")
    print()


# ===========================================================================
# TEST 4: RG running comparison
# ===========================================================================

def test_rg_running():
    """Check whether the formula matches 1-loop RG running from m_P to m_e."""
    print("=" * 70)
    print("TEST 4: Comparison with 1-loop RG running")
    print("=" * 70)

    # In QED, the 1-loop running of the electron mass is:
    #   m_e(μ) = m_e(Lambda) * [alpha(μ)/alpha(Lambda)]^(3/(2pi*b₀))
    # where b₀ = -4/3 for QED (one charged lepton)
    #
    # But this runs the mass at FIXED alpha. What we want is:
    # Given alpha and m_P, what mass does RG give?
    #
    # The mass hierarchy m_e/m_P ~ alpha^n suggests "dimensional transmutation"
    # In QCD: Lambda_QCD = μ * exp(-2pi/(b₃*alpha_s(μ)))
    # In FTD: m_e = m_P * exp(-something/alpha)?

    # Check: does sqrt(2pi) * (16/3) * alpha^11 look like exp(-c/alpha)?
    ratio = math.sqrt(2 * math.pi) * (16/3) * ALPHA**11
    ln_ratio = math.log(ratio)

    # If ratio = exp(-c/alpha), then c = -alpha * ln(ratio)
    c_effective = -ALPHA * ln_ratio

    print(f"  K_B/m_P = {ratio:.6e}")
    print(f"  ln(K_B/m_P) = {ln_ratio:.4f}")
    print(f"  If ratio = exp(-c/alpha): c = {c_effective:.6f}")
    print()

    # Compare: 11 * ln(1/alpha) = 11 * ln(137.036) = 54.13
    n_ln_alpha_inv = 11 * math.log(ALPHA_INV)
    ln_prefactor = math.log(math.sqrt(2*math.pi) * 16/3)

    print(f"  Decomposition: ln(ratio) = ln(sqrt(2pi)*16/3) + 11*ln(alpha)")
    print(f"    ln(sqrt(2pi)*16/3) = {ln_prefactor:.4f}")
    print(f"    11*ln(alpha)        = {11*math.log(ALPHA):.4f}")
    print(f"    Sum              = {ln_prefactor + 11*math.log(ALPHA):.4f}")
    print(f"    ln(ratio)        = {ln_ratio:.4f}")
    print(f"    Match: {abs(ln_prefactor + 11*math.log(ALPHA) - ln_ratio) < 1e-10}")
    print()

    # QCD dimensional transmutation check
    # Lambda_QCD/m_P = exp(-2pi/(b₃*alpha_s))
    # With b₃ = 7 and alpha_s ~ alpha (at unification): exp(-2pi/(7*0.00730)) = exp(-122.8) ~ 10⁻⁵⁴
    # That's WAY too small. So the formula is NOT simple dimensional transmutation.

    lambda_qcd_ratio = math.exp(-2*math.pi / (b_3 * ALPHA))
    print(f"  QCD-style Lambda/m_P = exp(-2pi/(b₃*alpha)) = {lambda_qcd_ratio:.2e}")
    print(f"  This is {lambda_qcd_ratio/ratio:.2e} times K_B/m_P")
    print(f"  Verdict: NOT simple dimensional transmutation (off by ~10³⁰)")
    print()
    print("  The formula is a POWER LAW (alpha^11), not an exponential (e^{-c/alpha}).")
    print("  Power laws arise from perturbative corrections, not nonperturbative effects.")
    print("  This is consistent with the ladder interpretation: each rung adds one")
    print("  power of alpha from a perturbative loop correction.")
    print()


# ===========================================================================
# TEST 5: Information content / honest accounting
# ===========================================================================

def test_information_content():
    """Quantify how much the formula "explains" vs how much it "fits"."""
    print("=" * 70)
    print("TEST 5: Information content and honest accounting")
    print("=" * 70)

    # The formula: m_e/m_P = sqrt(2pi) * (N_base²/N_c) * alpha^11
    # Structural choices:
    #   1. Normalization: sqrt(2pi) vs √(pi) vs 2pi vs pi vs 1
    #   2. Coefficient: N_base²/N_c vs other integer combinations
    #   3. Exponent: 11 vs other integers
    #
    # Count "reasonable" alternatives for each:

    # Choice 1: Normalization factor
    # Reasonable options: 1, √2, √pi, sqrt(2pi), 2pi, pi, 2, 4
    n_norm_options = 8
    print(f"  Normalization options: ~{n_norm_options} reasonable choices")

    # Choice 2: Coefficient from framework integers
    # N_c, N_base, b_3, N_eff and their squares, ratios
    # ~20 distinct values in (0.1, 100)
    n_coeff_options = 20
    print(f"  Coefficient options:  ~{n_coeff_options} integer ratios")

    # Choice 3: Exponent (integer 1-20)
    n_exp_options = 20
    print(f"  Exponent options:     ~{n_exp_options} integers")

    total_combinations = n_norm_options * n_coeff_options * n_exp_options
    print(f"  Total combinations:   ~{total_combinations}")

    # Target: m_e = 0.51100 MeV, m_P = 1.221e19 GeV
    # Ratio = 4.19e-23. Log-space: about 22.4 decades
    # Precision needed: 0.3% = 1 part in 300
    # Probability of ONE random combination hitting: ~1/300
    # Expected hits among 3200 combinations: ~10

    expected_hits = total_combinations / 300
    print(f"  Expected accidental hits (0.3%): ~{expected_hits:.0f}")
    print()

    # But: are these choices INDEPENDENT?
    print("  Independence analysis:")
    print(f"    sqrt(2pi) arises naturally from Gaussian flux normalization")
    print(f"    16/3 = N_base²/N_c connects spinor dimension to color")
    print(f"    11 = 4 + N_base + N_c from the alpha-power ladder")
    print(f"    All three reference the SAME framework integers")
    print(f"    So the choices are NOT independent — they share structure")
    print()

    # The honest verdict
    print("  VERDICT:")
    print("    The formula has ~3200 'competitors' if choices are independent.")
    print("    ~10 of those would accidentally match to 0.3%.")
    print("    But the choices are NOT independent — they all derive from the")
    print("    framework integers {3, 4, 7, 13}, which are themselves derived")
    print("    from G* via the master quadratic.")
    print()
    print("    This is better than fitting (which would use a continuous parameter)")
    print("    but weaker than a unique derivation (which would have no alternatives).")
    print("    Status: [SELECTION] — motivated, not forced.")
    print()


# ===========================================================================
# TEST 6: Self-energy coefficient check
# ===========================================================================

def test_self_energy_coefficient():
    """Check if E_field/K_B² = 0.118 is a clean function of G* and integers."""
    print("=" * 70)
    print("TEST 6: Self-energy coefficient E_field/K_B² = 0.118")
    print("=" * 70)

    measured = 0.118  # From GPU K_comp shell analysis

    candidates = [
        ("alpha*N_eff",              ALPHA * N_eff),
        ("alpha*(b_3+N_c)",          ALPHA * (b_3 + N_c)),
        ("1/(b_3 + N_base/N_c)", 1 / (b_3 + N_base/N_c)),
        ("3/(b_3 + N_eff)",      3 / (b_3 + N_eff)),
        ("N_c/(b_3+N_eff)",      N_c / (b_3 + N_eff)),
        ("alpha^2*N_eff²",            ALPHA**2 * N_eff**2),
        ("1/N_base²*N_c",        1 / (N_base**2 * N_c)),
        ("N_c/(N_eff+b_3+N_c)",  N_c / (N_eff + b_3 + N_c)),
        ("3/26 (3/Moore)",       3 / 26),
        ("N_base/(2*N_eff+N_base)", N_base / (2*N_eff + N_base)),
        ("1/(2*N_base+1/N_c)",  1 / (2*N_base + 1/N_c)),
        ("alpha*16",                 ALPHA * 16),
        ("(N_c/26)*(1+1/alpha^0.1)",  (N_c/26) * (1 + ALPHA**0.1)),
        ("1/(N_base*(N_c-1/N_c))", 1/(N_base*(N_c - 1/N_c))),
    ]

    print(f"  Measured: E_field/K_B² = {measured}")
    print(f"  {'Formula':>30s}  {'Value':>10s}  {'Error':>10s}")
    print(f"  {'-'*30}  {'-'*10}  {'-'*10}")

    best = None
    best_err = 1.0
    for name, val in candidates:
        err = abs(val - measured) / measured
        if err < best_err:
            best_err = err
            best = name
        marker = " <<<" if err < 0.02 else ""
        print(f"  {name:>30s}  {val:10.6f}  {err*100:9.2f}%{marker}")

    print()
    if best_err < 0.02:
        print(f"  Best match: {best} (error {best_err*100:.2f}%)")
        print(f"  This MIGHT be the analytical form of E_field/K_B²")
    else:
        print(f"  Best match: {best} (error {best_err*100:.2f}%)")
        print(f"  No clean match found among simple integer combinations.")
        print(f"  The coefficient 0.118 likely depends on lattice geometry")
        print(f"  (stencil weights, SOR convergence) rather than fundamental constants.")
    print()


# ===========================================================================
# TEST 7: The tautology argument — formalized
# ===========================================================================

def test_tautology_chain():
    """Verify the logical chain: K_B = m_e follows from identification."""
    print("=" * 70)
    print("TEST 7: The tautology chain")
    print("=" * 70)

    print("""
  Step 1. [AXIOM]  The Born-Infeld Lagrangian has a rest-energy parameter K_B.
          Every manifested voxel costs K_B in energy.

  Step 2. [THEOREM] Manifestation creates charge.
          s = +/-1 couples to div*J via the Gauss constraint.
          There is no neutral manifestation (neutrinos have s = 0).

  Step 3. [THEOREM] A single manifested voxel is the minimum-energy charged state.
          BI energy = K_B per voxel. Two voxels cost >= 2K_B.
          Sub-voxel excitations don't exist on a discrete lattice.

  Step 4. [SELECTION] The minimum-energy charged state = the electron.
          The lightest charged fermion in nature is the electron.
          The lightest charged excitation on the lattice is one voxel.
          This identification names a lattice object, not fits a parameter.

  Step 5. [FOLLOWS] K_B = m_e.

  Step 6. [SELECTION] m_e / m_P = sqrt(2pi) * (16/3) * alpha^11.
          This gives the VALUE of K_B in Planck units.
          Every factor is computable from G* and framework integers.

  Step 7. [FOLLOWS] m_P = K_B / (sqrt(2pi) * (16/3) * alpha^11).
          The Planck mass is DERIVED from K_B, not the other way around.
          The "external calibration" is the unit convention: K_B = 0.5100 MeV.
""")

    # Numerical verification of Step 7
    K_B_mev = 0.5100  # MeV (FTD-derived value)
    ratio = math.sqrt(2 * math.pi) * (16/3) * ALPHA**11
    m_P_derived = K_B_mev * 1e-3 / ratio  # GeV
    m_P_known = 1.220890e19  # GeV

    rel_err = abs(m_P_derived - m_P_known) / m_P_known
    print(f"  Verification of Step 7:")
    print(f"    K_B              = {K_B_mev} MeV")
    print(f"    K_B/m_P ratio    = {ratio:.6e}")
    print(f"    m_P (derived)    = {m_P_derived:.6e} GeV")
    print(f"    m_P (known)      = {m_P_known:.6e} GeV")
    print(f"    Relative error   = {rel_err*100:.4f}%")
    print()

    # The 0.20% error comes from the formula's 0.20% error on m_e
    assert rel_err < 0.003, f"m_P derivation error too large: {rel_err}"
    print("  PASS: Planck mass recovered from K_B with 0.20% accuracy")
    print("  (Error inherited from m_e formula, not independent)")
    print()


# ===========================================================================
# MAIN
# ===========================================================================

if __name__ == '__main__':
    print()
    print("ELECTRON MASS DERIVATION VERIFICATION")
    print("Foundational Ternary Dynamics — Epistemic Audit")
    print()

    ratio = test_formula_numerical()
    test_exponent_uniqueness()
    test_coefficient_uniqueness()
    test_rg_running()
    test_information_content()
    test_self_energy_coefficient()
    test_tautology_chain()

    print("=" * 70)
    print("ALL TESTS PASSED")
    print("=" * 70)
    print()
    print("SUMMARY:")
    print("  K_B = m_e is [SELECTION], not [IMPOSED] and not [THEOREM].")
    print("  The formula m_e/m_P = sqrt(2pi)*(16/3)*alpha^11 uses only derived quantities.")
    print("  The absolute mass scale is set by one unit convention (K_B = 0.5100 MeV),")
    print("  not by importing the Planck mass as a physical input.")
    print("  The Planck mass is DERIVED: m_P = K_B/(sqrt(2pi)*(16/3)*alpha^11).")
    print()
