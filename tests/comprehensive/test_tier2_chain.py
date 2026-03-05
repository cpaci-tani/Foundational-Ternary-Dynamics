"""
TIER 2: Derivation Chain Integrity (Weight: 20%)

Test whether each step in the FTD derivation chain follows from the
previous step. Identify circular dependencies, hidden assumptions,
and whether alternative integer sets or constants work equally well.

A failure here means the "derivation" is actually curve-fitting or
contains circular reasoning.
"""

import numpy as np
from fractions import Fraction

try:
    import mpmath

    mpmath.mp.dps = 50
    HAS_MPMATH = True
except ImportError:
    HAS_MPMATH = False

from .ftd_test_utils import N_c, N_base, b_3, CODATA, percent_error
from .test_tier3_predictions import compute_ftd_values

# =============================================================================
# Test 2.1: Dependency Graph (Acyclicity Check)
# =============================================================================


class TestDependencyGraph:
    """Build and verify the derivation dependency graph."""

    def _build_graph(self):
        """Build directed graph: node -> [dependencies]."""
        graph = {
            # Level 0: Axioms (no dependencies)
            "N_c": [],
            "N_base": [],
            "G_star": [],  # Mathematical constant, no physics
            "M_Planck": [],  # External input
            # Level 1: Direct derivations from integers
            "b_3": ["N_c", "N_base"],
            "N_eff": ["b_3", "N_c"],
            "D": ["N_c", "N_base"],
            "coefficient_16": ["N_base"],
            # Level 2: Master quadratic
            "master_quadratic": ["G_star", "coefficient_16"],
            "x_plus": ["master_quadratic"],
            "x_minus": ["master_quadratic"],
            # Level 3: Physical identification
            "alpha_inv": ["x_plus"],  # IDENTIFICATION
            "N_c_from_x_minus": ["x_minus"],  # IDENTIFICATION → circularity check!
            # Level 4: Precision formula
            "epsilon": ["b_3", "N_eff"],
            "c1": ["N_c", "D"],
            "c2": ["N_eff", "N_base"],
            "c3": ["N_base", "N_c", "D"],
            "c4": ["N_c", "D", "b_3", "N_base"],
            "alpha_inv_precision": ["alpha_inv", "epsilon", "c1", "c2", "c3", "c4"],
            "alpha": ["alpha_inv_precision"],
            # Level 5: Derived physics
            "m_electron": ["M_Planck", "N_base", "N_c", "alpha"],
            "sin2_theta_w": ["N_c", "N_eff"],
            "m_mu_over_m_e": ["N_c", "b_3"],
            "m_tau_over_m_e": ["m_mu_over_m_e"],
            "v_Higgs": ["M_Planck", "alpha"],
            "alpha_G": ["N_base", "N_c", "N_eff", "b_3", "alpha"],
            "delta_CKM": ["b_3", "N_c"],
        }
        return graph

    def test_graph_is_acyclic(self):
        """Verify the dependency graph has no cycles."""
        graph = self._build_graph()

        # Topological sort via DFS
        visited = set()
        in_stack = set()
        cycles = []

        def dfs(node, path):
            if node in in_stack:
                cycle_start = path.index(node)
                cycles.append(path[cycle_start:] + [node])
                return
            if node in visited:
                return
            visited.add(node)
            in_stack.add(node)
            for dep in graph.get(node, []):
                dfs(dep, path + [node])
            in_stack.discard(node)

        for node in graph:
            dfs(node, [])

        if cycles:
            for c in cycles:
                print(f"\n  CYCLE FOUND: {' -> '.join(c)}")
        assert len(cycles) == 0, f"Found {len(cycles)} cycles in dependency graph"

    def test_nc_circularity_flagged(self):
        """N_c=3 is both an INPUT and an OUTPUT (from floor(x_minus)).

        This is not a cycle in the graph (they're separate nodes)
        but IS a philosophical circularity that must be acknowledged.
        """
        # N_c is used to construct the quadratic
        # x_minus from the quadratic gives floor(x_minus) = 3 = N_c
        # This is consistency, not derivation
        from scipy.special import gamma

        g_quarter = gamma(0.25)
        g_star = np.sqrt(2) * g_quarter**2 / (2 * np.pi)
        disc = (16 * g_star**2) ** 2 - 4 * 16 * g_star**3
        x_minus = (16 * g_star**2 - np.sqrt(disc)) / 2
        assert int(np.floor(x_minus)) == N_c, f"floor(x_minus) = {int(np.floor(x_minus))} != N_c = {N_c}"
        # This test PASSES but the circularity is noted:
        print("\n  NOTE: N_c=3 is both input (integer choice) and output (floor(x_minus))")
        print("  This is CONSISTENCY, not an independent derivation.")


# =============================================================================
# Test 2.2: b_3 QCD Beta Function Audit
# =============================================================================


class TestB3QCDAudit:
    """Audit QCD claim: does b_3=7 match any QCD beta coefficient?"""

    def test_qcd_beta_nf0_comment_wrong(self):
        """constants.py says b_3=7 from 'N_f=0' but b_0(N_f=0) = 11, NOT 7."""
        # Standard QCD one-loop: b_0 = 11*N_c/3 - 2*N_f/3
        # For SU(3): b_0 = 11 - 2*N_f/3
        b0_nf0 = 11 - Fraction(2, 3) * 0  # N_f=0, SU(3)
        assert b0_nf0 == 11, f"b_0(N_f=0) = {b0_nf0}"
        assert b0_nf0 != b_3, "b_0(N_f=0) should NOT equal b_3=7"
        print("\n  CONFIRMED: The comment 'N_f=0' in constants.py is WRONG.")
        print("  b_0(N_f=0) = 11, not 7.")

    def test_standard_qcd_nf6_gives_7(self):
        """Standard QCD: b_0(N_f=6) = 11 - 2*6/3 = 7. All quarks active."""
        b0_nf6 = 11 - Fraction(2, 3) * 6
        assert b0_nf6 == 7, f"b_0(N_f=6) = {b0_nf6}"
        assert b0_nf6 == b_3
        print("\n  FINDING: b_3=7 DOES equal b_0(N_f=6) in standard QCD!")
        print("  This is the beta coefficient when ALL 6 quarks are active")
        print("  (above top quark mass ~173 GeV).")
        print("  The constants.py comment is wrong about N_f=0,")
        print("  but b_3=7 being a QCD beta coefficient is PLAUSIBLE at N_f=6.")

    def test_b3_is_also_nc_plus_nbase(self):
        """b_3=7 also equals N_c + N_base = 3+4 = 7."""
        assert b_3 == N_c + N_base == 7
        print("\n  NOTE: b_3=7 has dual origin:")
        print("    1. N_c + N_base = 3+4 = 7 (structural)")
        print("    2. b_0(N_f=6) = 11-4 = 7 (QCD, all flavors active)")
        print("  Whether this coincidence is deep or accidental is OPEN.")


# =============================================================================
# Test 2.3: Integer Uniqueness (Exhaustive Search)
# =============================================================================


class TestIntegerUniqueness:
    """Search for alternative integer sets that produce similar physics."""

    def test_exhaustive_alpha_search(self):
        """Find all (N_c, N_base) pairs producing 1/alpha within 100 ppm."""
        from scipy.special import gamma

        g_quarter = gamma(0.25)
        g_star = np.sqrt(2) * g_quarter**2 / (2 * np.pi)

        target = CODATA["alpha_inv"].value
        matches = []

        for nc in range(2, 31):
            for nb in range(2, 31):
                coeff = nb**2
                b_coef = -coeff * g_star**2
                c_coef = coeff * g_star**3
                disc = b_coef**2 - 4 * c_coef
                if disc < 0:
                    continue
                x_plus = (-b_coef + np.sqrt(disc)) / 2
                error = abs(x_plus - target) / target * 1e6
                if error < 100:  # within 100 ppm
                    matches.append((nc, nb, x_plus, error))

        print("\n  Integer sets producing 1/alpha within 100 ppm:")
        for nc, nb, xp, err in sorted(matches, key=lambda x: x[3]):
            print(f"    N_c={nc}, N_base={nb}: x+ = {xp:.6f}, error = {err:.2f} ppm")

        # The test: {3,4} should be among the matches
        found_3_4 = any(m[0] == 3 and m[1] == 4 for m in matches)
        assert found_3_4, "{3,4} not found in matches!"

        # Report uniqueness
        if len(matches) == 1:
            print("  UNIQUE: Only {3,4} matches!")
        else:
            print(f"  WARNING: {len(matches)} integer sets match within 100 ppm")

    def test_comprehensive_chi2_search(self):
        """Find ALL (N_c, N_base) with comparable chi^2 to {3,4}."""
        baseline_err = self._compute_multi_error(3, 4)
        competitors = []

        for nc in range(2, 21):
            for nb in range(2, 21):
                if nc == 3 and nb == 4:
                    continue
                err = self._compute_multi_error(nc, nb)
                if err < baseline_err * 3:  # within 3x of baseline
                    competitors.append((nc, nb, err, err / max(baseline_err, 1e-10)))

        print(f"\n  Baseline {{3,4}} error = {baseline_err:.4f}")
        print("  Competitors within 3x:")
        for nc, nb, err, ratio in sorted(competitors, key=lambda x: x[2]):
            print(f"    N_c={nc}, N_base={nb}: error = {err:.4f} ({ratio:.1f}x)")

        if len(competitors) == 0:
            print("  {3,4} is UNIQUE — no competitors within 3x!")

    def _compute_multi_error(self, nc, nb):
        """Total squared percent error across key predictions."""
        ftd = compute_ftd_values(nc, nb)
        if ftd is None:
            return float("inf")

        total = 0
        for key in ["alpha_inv", "sin2_theta_w", "m_mu_over_m_e", "m_tau_over_m_e"]:
            if key in ftd and key in CODATA:
                val = ftd[key]
                if np.isfinite(val):
                    total += percent_error(val, CODATA[key].value) ** 2
                else:
                    total += 1e6
        return total


# =============================================================================
# Test 2.4: Alternative Mathematical Constants
# =============================================================================


class TestAlternativeConstants:
    """Test whether other mathematical constants produce 1/alpha."""

    def test_alternative_constants(self):
        """No alternative constant should produce 1/alpha as well as G*."""
        from scipy.special import gamma

        g_quarter = gamma(0.25)
        g_star = np.sqrt(2) * g_quarter**2 / (2 * np.pi)

        constants = {
            "G*": g_star,
            "pi": np.pi,
            "e": np.e,
            "phi": (1 + np.sqrt(5)) / 2,
            "sqrt(2)": np.sqrt(2),
            "sqrt(3)": np.sqrt(3),
            "Euler-gamma": 0.5772156649,
            "Catalan": 0.9159655941,
            "Apery (zeta(3))": 1.2020569031,
            "Feigenbaum delta": 4.6692016091,
            "Feigenbaum alpha": 2.5029078751,
        }

        target = CODATA["alpha_inv"].value
        results = []

        for name, c in constants.items():
            # Standard quadratic: x^2 - 16*c^2*x + 16*c^3 = 0
            disc = (16 * c**2) ** 2 - 4 * 16 * c**3
            if disc < 0:
                results.append((name, c, None, float("inf")))
                continue
            x_plus = (16 * c**2 + np.sqrt(disc)) / 2
            error = abs(x_plus - target) / target * 1e6
            results.append((name, c, x_plus, error))

        print(f"\n  Alternative constant test (target: 1/alpha = {target}):")
        for name, c, xp, err in sorted(results, key=lambda x: x[3]):
            if xp is not None:
                print(f"    {name:<25} c={c:.6f}  x+={xp:.4f}  error={err:.0f} ppm")
            else:
                print(f"    {name:<25} c={c:.6f}  COMPLEX ROOTS")

        # G* should be the best
        g_star_error = [r[3] for r in results if r[0] == "G*"][0]
        others_better = [r for r in results if r[0] != "G*" and r[3] < g_star_error]

        if len(others_better) == 0:
            print("  G* is THE BEST constant — no alternatives match better!")
        else:
            print(f"  WARNING: {len(others_better)} constants match better than G*!")

        assert g_star_error < 5, f"G* error = {g_star_error} ppm (should be ~1.26)"


# =============================================================================
# Test 2.5: Hidden Inputs Audit
# =============================================================================


class TestHiddenInputs:
    """Audit which predictions require external inputs."""

    def test_inputs_catalog(self):
        """Document all external inputs required by each prediction."""
        audit = {
            # Truly derived (no external inputs beyond G*)
            "alpha_inv": ["G*", "integers {3,4,7,13}"],
            "sin2_theta_w": ["integers {3,13}"],
            "delta_CKM": ["integers {3,7}"],
            "PMNS angles": ["integers"],
            "m_mu/m_e": ["integers {3,7}"],
            "m_tau/m_e": ["integers {3,7} via m_mu/m_e"],
            # Require M_Planck (external input)
            "m_electron": ["M_Planck", "alpha", "integers"],
            "v_Higgs": ["M_Planck", "alpha"],
            "alpha_G": ["alpha", "integers"],
            # Require additional external inputs
            "decay_rates": ["G_F (Fermi constant)", "standard QFT formulas"],
            "meson_masses": ["Lambda_QCD", "ChPT formulas"],
            "running_couplings": ["RG equations from SM"],
        }

        print("\n  HIDDEN INPUTS AUDIT:")
        print("  " + "-" * 60)
        n_genuine = 0
        n_external = 0
        for quantity, inputs in audit.items():
            has_external = any(
                i
                in [
                    "M_Planck",
                    "G_F (Fermi constant)",
                    "Lambda_QCD",
                    "ChPT formulas",
                    "standard QFT formulas",
                    "RG equations from SM",
                ]
                for i in inputs
            )
            status = "EXTERNAL" if has_external else "GENUINE"
            if has_external:
                n_external += 1
            else:
                n_genuine += 1
            print(f"    {quantity:<25} [{status}] ← {', '.join(inputs)}")

        print(f"\n  Genuine derivations: {n_genuine}")
        print(f"  Require external inputs: {n_external}")
        print("  'Zero free parameters' claim: FALSE")

    def test_m_planck_is_required(self):
        """Electron mass derivation requires M_Planck as external input."""
        m_planck = 1.220890e19  # GeV
        from scipy.special import gamma

        g_quarter = gamma(0.25)
        g_star = np.sqrt(2) * g_quarter**2 / (2 * np.pi)
        disc = (16 * g_star**2) ** 2 - 4 * 16 * g_star**3
        x_plus = (16 * g_star**2 + np.sqrt(disc)) / 2
        alpha = 1.0 / x_plus

        m_e = m_planck * np.sqrt(2 * np.pi) * (16 / 3) * alpha**11
        m_e_mev = m_e * 1000

        # The formula works, but M_Planck is an INPUT
        assert abs(m_e_mev - 0.511) < 0.01, f"m_e = {m_e_mev} MeV"
        print(f"\n  m_e = {m_e_mev:.4f} MeV (uses M_Planck = {m_planck:.3e} GeV as INPUT)")


# =============================================================================
# Test 2.6: Coefficient Freedom
# =============================================================================


class TestCoefficientFreedom:
    """Test whether other polynomial forms also produce 1/alpha."""

    def test_quadratic_is_special(self):
        """Test generalized quadratics x^2 - a*c^2*x + a*c^3 = 0 for various a."""
        from scipy.special import gamma

        g_quarter = gamma(0.25)
        g_star = np.sqrt(2) * g_quarter**2 / (2 * np.pi)
        target = CODATA["alpha_inv"].value

        print(f"\n  Testing generalized coefficients (c = G* = {g_star:.6f}):")
        matches = []
        for a in range(1, 50):
            disc = (a * g_star**2) ** 2 - 4 * a * g_star**3
            if disc < 0:
                continue
            x_plus = (a * g_star**2 + np.sqrt(disc)) / 2
            error = abs(x_plus - target) / target * 1e6
            if error < 100:
                matches.append((a, x_plus, error))
                print(f"    a={a}: x+ = {x_plus:.6f}, error = {error:.2f} ppm")

        if len(matches) == 1 and matches[0][0] == 16:
            print("  UNIQUE: Only a=16 matches!")
        else:
            print(f"  {len(matches)} values of a match within 100 ppm")
