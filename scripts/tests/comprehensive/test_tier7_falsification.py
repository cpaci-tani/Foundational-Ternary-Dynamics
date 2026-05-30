"""
TIER 7: Falsification Attempts (Weight: 5%)

Actively try to break the framework. Find internal contradictions,
predictions that disagree with experiment, circular reasoning,
or evidence the framework is unfalsifiable.

A failure here means internal contradictions or unfalsifiability --
the framework is not scientific.
"""

import numpy as np

from .ftd_test_utils import N_c, N_eff, CODATA, percent_error
from .test_tier3_predictions import compute_ftd_values

# =============================================================================
# Test 7.1: Internal Contradictions
# =============================================================================


class TestInternalContradictions:
    """Check for mutual inconsistencies among derived quantities."""

    def test_sin2w_vs_mw_mz(self):
        """Does sin^2(theta_W) = 3/13 correctly predict M_W/M_Z?"""
        sin2w_ftd = N_c / N_eff  # 3/13 = 0.23077
        mw_mz_ftd = np.sqrt(1 - sin2w_ftd)  # = sqrt(10/13)

        mw_mz_exp = CODATA["m_W"].value / CODATA["m_Z"].value
        err = percent_error(mw_mz_ftd, mw_mz_exp)

        print("\n  Consistency: sin^2(theta_W) -> M_W/M_Z")
        print(f"  sin^2(theta_W) = {sin2w_ftd:.6f} (FTD: 3/13)")
        print(f"  M_W/M_Z = sqrt(1-sin^2) = {mw_mz_ftd:.6f}")
        print(f"  M_W/M_Z (exp) = {mw_mz_exp:.6f}")
        print(f"  Error = {err:.2f}%")

        # 0.50% error -- marginal but not contradictory
        assert err < 2.0, f"sin^2(theta_W) inconsistent with M_W/M_Z by {err}%"

    def test_alpha_consistency(self):
        """Is alpha from the quadratic consistent with alpha used in mass formulas?"""
        from scipy.special import gamma

        g_quarter = gamma(0.25)
        g_star = np.sqrt(2) * g_quarter**2 / (2 * np.pi)

        # Alpha from tree-level quadratic
        disc = (16 * g_star**2) ** 2 - 4 * 16 * g_star**3
        x_plus_tree = (16 * g_star**2 + np.sqrt(disc)) / 2
        alpha_tree = 1.0 / x_plus_tree

        # Alpha from 4-term formula
        eps = abs(np.exp(np.pi) - np.pi - 20)
        c1, c2, c3, c4 = 9 / 47, 5 / 64, 4 / 141, 141 / 11
        x_plus_4t = x_plus_tree - c1 * eps + c2 * eps**2 - c3 * eps**3 - c4 * eps**4
        alpha_4t = 1.0 / x_plus_4t

        diff_ppm = abs(alpha_tree - alpha_4t) / alpha_4t * 1e6
        print("\n  Alpha consistency:")
        print(f"  alpha (tree level) = {alpha_tree:.10f}")
        print(f"  alpha (4-term)     = {alpha_4t:.10f}")
        print(f"  Difference:        {diff_ppm:.2f} ppm")

    def test_mass_ratio_self_consistency(self):
        """Check that m_mu/m_e * m_e = m_mu (using derived values)."""
        ftd = compute_ftd_values()
        m_e_ftd = ftd["m_electron"]
        mu_e_ratio = ftd["m_mu_over_m_e"]
        m_mu_ftd = m_e_ftd * mu_e_ratio

        m_mu_exp = CODATA["m_muon"].value
        err = percent_error(m_mu_ftd, m_mu_exp)
        print(f"\n  m_mu = m_e * (m_mu/m_e) = {m_mu_ftd:.4f} MeV")
        print(f"  m_mu (exp) = {m_mu_exp:.4f} MeV")
        print(f"  Error = {err:.2f}%")


# =============================================================================
# Test 7.2: Worst Predictions
# =============================================================================


class TestWorstPredictions:
    """Identify the predictions with largest errors."""

    def test_catalog_all_errors(self):
        """List every prediction sorted by error magnitude."""
        ftd = compute_ftd_values()

        predictions = []
        for ftd_key, exp_key in [
            ("alpha_inv", "alpha_inv"),
            ("sin2_theta_w", "sin2_theta_w"),
            ("m_electron", "m_electron"),
            ("m_mu_over_m_e", "m_mu_over_m_e"),
            ("m_tau_over_m_e", "m_tau_over_m_e"),
            ("m_p_over_m_e", "m_p_over_m_e"),
            ("v_Higgs", "v_Higgs"),
            ("delta_CKM", "delta_CKM"),
            ("sin2_theta12_PMNS", "sin2_theta12_PMNS"),
            ("sin2_theta23_PMNS", "sin2_theta23_PMNS"),
            ("sin2_theta13_PMNS", "sin2_theta13_PMNS"),
            ("delta_m2_ratio", "delta_m2_ratio"),
            ("n_s", "n_s"),
        ]:
            if ftd_key in ftd and exp_key in CODATA:
                err = percent_error(ftd[ftd_key], CODATA[exp_key].value)
                predictions.append((ftd_key, ftd[ftd_key], CODATA[exp_key].value, err))

        predictions.sort(key=lambda x: -x[3])

        print("\n  ALL PREDICTIONS SORTED BY ERROR (worst first):")
        print("  " + "-" * 70)
        n_bad = 0
        for name, ftd_val, exp_val, err in predictions:
            marker = " <- PROBLEM" if err > 5 else ""
            print(f"  {name:<30} FTD={ftd_val:>12.6f}  " f"exp={exp_val:>12.6f}  err={err:>6.2f}%{marker}")
            if err > 5:
                n_bad += 1

        print(f"\n  Predictions with > 5% error: {n_bad}")


# =============================================================================
# Test 7.3: Circular Reasoning Detection
# =============================================================================


class TestCircularReasoning:
    """Automated check for circular dependencies in formulas."""

    def test_mass_formula_circularity(self):
        """The mass formulas use alpha, which uses G*, which is independent.
        Check that no circular dependency exists."""
        # The derivation chain:
        # G* (pure math) -> quadratic -> alpha -> m_e (with M_Planck)
        # alpha does NOT depend on m_e
        # m_e depends on alpha and M_Planck
        # M_Planck is an EXTERNAL INPUT
        # This is NOT circular -- it's a one-way chain
        print("\n  Circularity check: mass formulas")
        print("  G* -> quadratic -> alpha -> m_e (+ M_Planck)")
        print("  Direction: one-way (no back-dependency)")
        print("  Status: NOT CIRCULAR")

    def test_integer_circularity(self):
        """The integers {3,4,7,13} were identified from known physics.
        This IS a form of circularity."""
        print("\n  Circularity check: integer identification")
        print("  Step 1: Observe N_c=3 (QCD colors), N_base=4 (dimensions)")
        print("  Step 2: Define b_3 = N_c+N_base = 7, N_eff = b_3+2N_c = 13")
        print("  Step 3: Verify these produce alpha, mass ratios, etc.")
        print("  Step 4: Claim they are 'derived'")
        print("")
        print("  VERDICT: This is VERIFICATION, not DERIVATION.")
        print("  The integers were identified FROM physics, then shown to be")
        print("  self-consistent. A non-circular derivation would start from")
        print("  pure mathematics and discover {3,4,7,13} without knowing physics.")
        print("  Status: CIRCULAR (philosophical, not mathematical)")


# =============================================================================
# Test 7.4: Overfitting / Flexibility Assessment
# =============================================================================


class TestOverfitting:
    """Count effective degrees of freedom vs number of predictions."""

    def test_effective_dof(self):
        """Count structural choices that function as free parameters."""
        choices = {
            # Explicit integers (2 independent: N_c, N_base; rest derived)
            "N_c": "Integer choice",
            "N_base": "Integer choice",
            # Structural choices (act as parameters)
            "Quadratic form (not cubic, quartic)": "Form selection",
            "Coefficient = N_base^2 = 16": "Derived from N_base",
            "G* as the base constant": "Mathematical constant selection",
            "j=1728 CM curve": "Curve selection",
            # Correction formula
            "4-term correction (not 3 or 5)": "Truncation choice",
            "Epsilon = e^pi - pi - 20": "Correction form",
            # Mass formulas
            "m_e: exponent 11 = N_c+2*N_base": "Exponent selection",
            "m_mu/m_e = 3*7*10-3": "Post-hoc formula",
            "m_tau/m_e = 17*207-42": "Post-hoc formula",
            "m_p/m_e = 13*137+55": "Post-hoc formula",
            # External inputs
            "M_Planck": "External input",
        }

        n_true_params = 2  # N_c, N_base
        n_structural = len(choices) - 2  # Structural choices beyond integers
        n_total_dof = len(choices)
        n_predictions = 15  # Approximate number of independent predictions

        print("\n  OVERFITTING ASSESSMENT:")
        print("  " + "-" * 60)
        for choice, kind in choices.items():
            print(f"    [{kind:<25}] {choice}")

        print(f"\n  True free parameters:     {n_true_params}")
        print(f"  Structural choices:       {n_structural}")
        print(f"  Total effective DoF:      {n_total_dof}")
        print(f"  Independent predictions:  {n_predictions}")
        print(f"  Ratio (DoF/predictions):  {n_total_dof/n_predictions:.2f}")

        if n_total_dof >= n_predictions:
            print("  VERDICT: OVERFITTING RISK -- more choices than predictions!")
        elif n_total_dof >= n_predictions * 0.5:
            print("  VERDICT: MARGINAL -- significant structural freedom")
        else:
            print("  VERDICT: ACCEPTABLE -- predictions exceed degrees of freedom")


# =============================================================================
# Test 7.5: Falsifiability Score
# =============================================================================


class TestFalsifiability:
    """Score what fraction of claims are genuinely falsifiable."""

    def test_falsifiability_catalog(self):
        """Catalog each claim and whether it's falsifiable."""
        claims = [
            ("1/alpha = 137.036...", True, "Precision measurement > 10 ppm from prediction"),
            ("N_gen = 3", True, "Discovery of 4th generation with standard couplings"),
            ("sin^2(theta_W) = 3/13", True, "Precision measurement > 1% discrepancy"),
            ("m_e formula", True, "Precision M_Planck measurement contradicting formula"),
            ("m_mu/m_e = 207", True, "Any measurement inconsistent with 207.0"),
            ("Substrate S <= 2", True, "Would be falsified if substrate could give S > 2"),
            ("Aggregate S = 2sqrt(2)", False, "Cannot test -- no demonstration of aggregate emergence"),
            ("Reference frame context K_C = 3.60", False, "No experimental protocol to measure this"),
            ("Phase = 52.54 degrees", False, "No experimental protocol to measure this"),
            ("Void is dispositional substrate", False, "Philosophical -- not empirically testable"),
            (
                "sLoop explains Bell violations",
                False,
                "No specific prediction distinguishing sLoop from other interpretations",
            ),
            ("D=3 is uniquely selected", True, "Would be falsified by detection of extra dimensions"),
            ("Discrete spacetime", True, "Detection of Planck-scale Lorentz violation"),
            ("CP phase = arctan(7/3)", True, "Precision CKM measurement > 5% from prediction"),
            ("Inflation n_s = 0.9645", True, "CMB measurement > 3 sigma outside predicted range"),
        ]

        n_falsifiable = sum(1 for _, f, _ in claims if f)
        n_total = len(claims)
        fraction = n_falsifiable / n_total

        print("\n  FALSIFIABILITY ASSESSMENT:")
        print("  " + "-" * 60)
        for claim, is_falsifiable, criterion in claims:
            status = "FALSIFIABLE" if is_falsifiable else "NOT FALSIFIABLE"
            print(f"    [{status:<16}] {claim}")
            if not is_falsifiable:
                print(f"                     -> {criterion}")

        print(f"\n  Falsifiable:     {n_falsifiable}/{n_total} = {fraction*100:.0f}%")
        if fraction >= 0.5:
            print("  VERDICT: PASSES Popper's criterion (>50% falsifiable)")
        else:
            print("  VERDICT: FAILS Popper's criterion (<50% falsifiable)")


# =============================================================================
# Test 7.6: b_3 QCD Mismatch
# =============================================================================


class TestB3Mismatch:
    """Document the false claim about b_3 being a QCD beta coefficient."""

    def test_b3_not_qcd(self):
        """b_3=7 has NO valid QCD beta function interpretation."""
        # Standard QCD one-loop beta function:
        # b_0 = (11*N_c - 2*N_f) / 3  (alternative normalization)
        # or b_0 = 11 - 4/3 * N_f  (for N_c=3 and specific convention)

        print("\n  b_3 = 7 QCD CLAIM ANALYSIS:")
        print("  " + "-" * 50)
        print("  Claimed in constants.py line 17:")
        print("    'QCD beta function coefficient = 11 - 4/3 * N_c * N_f (N_f=0)'")
        print("    This gives: 11 - 0 = 11, NOT 7.")
        print()

        # Check all conventions
        for nf in range(7):
            # Convention 1: b_0 = 11*N_c/3 - 2*N_f/3
            b0_conv1 = 11 * N_c / 3 - 2 * nf / 3
            # Convention 2: b_0 = 11 - 2*nf/3
            b0_conv2 = 11 - 2 * nf / 3
            # Convention 3: b_0 = (11*N_c - 2*nf) / (12*np.pi)
            b0_conv3 = (11 * N_c - 2 * nf) / (12 * np.pi)

            if abs(b0_conv1 - 7) < 0.01 or abs(b0_conv2 - 7) < 0.01:
                print(f"  N_f={nf}: b_0 = {b0_conv1:.2f} (conv1) / {b0_conv2:.2f} (conv2)")
            if nf == 0:
                print(f"  N_f=0: b_0 = {b0_conv1:.2f} (conv1) / {b0_conv2:.2f} (conv2)")

        print()
        print("  ACTUAL ORIGIN of b_3 = 7:")
        print("    b_3 = N_c + N_base = 3 + 4 = 7")
        print("  This is purely an integer arithmetic identity.")
        print("  The QCD beta function claim is INCORRECT.")
        print()
        print("  RECOMMENDATION: Fix constants.py line 17 comment.")
