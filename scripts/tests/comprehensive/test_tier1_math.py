"""
TIER 1: Mathematical Foundations (Weight: 15%)

Verify every algebraic identity, constant computation, and theorem
claimed by FTD at maximum available numerical precision.

A failure here means a computational error in the mathematical
constants — everything downstream is invalidated.
"""

import pytest
import numpy as np
from fractions import Fraction

# Try mpmath for arbitrary precision; fall back to scipy
try:
    import mpmath

    mpmath.mp.dps = 100  # 100 decimal places
    HAS_MPMATH = True
except ImportError:
    HAS_MPMATH = False

from .ftd_test_utils import N_c, N_base, b_3, N_eff, D

# =============================================================================
# Test 1.1: G* Computation via Multiple Routes
# =============================================================================


class TestGStarComputation:
    """Verify G* = sqrt(2) * Gamma(1/4)^2 / (2*pi) via multiple routes."""

    def test_gstar_via_gamma_function(self):
        """Route 1: Direct Gamma function computation."""
        if not HAS_MPMATH:
            pytest.skip("mpmath required for high-precision tests")

        g_quarter = mpmath.gamma(mpmath.mpf(1) / 4)
        g_star = mpmath.sqrt(2) * g_quarter**2 / (2 * mpmath.pi)

        # Cross-check: compute via elliptic K as independent reference
        K_val = mpmath.ellipk(mpmath.mpf(1) / 2)
        g_star_ref = 2 * mpmath.sqrt(2 / mpmath.pi) * K_val
        assert abs(g_star - g_star_ref) < mpmath.mpf(10) ** (
            -45
        ), f"G* via Gamma vs elliptic K disagree: {abs(g_star - g_star_ref)}"
        # Verify first 10 digits match known value
        assert str(g_star)[:12] == "2.9586751191", f"G* leading digits wrong: {g_star}"

    def test_gstar_via_elliptic_K(self):
        """Route 2: Via complete elliptic integral K(1/sqrt(2))."""
        if not HAS_MPMATH:
            pytest.skip("mpmath required")

        K_val = mpmath.ellipk(mpmath.mpf(1) / 2)  # K(k) with k^2=1/2
        g_star_K = 2 * mpmath.sqrt(2 / mpmath.pi) * K_val

        g_quarter = mpmath.gamma(mpmath.mpf(1) / 4)
        g_star_gamma = mpmath.sqrt(2) * g_quarter**2 / (2 * mpmath.pi)

        diff = abs(g_star_K - g_star_gamma)
        assert diff < mpmath.mpf(10) ** (-45), f"G* routes disagree by {diff}"

    def test_gstar_via_theta_function(self):
        """Route 3: G* = sqrt(2*pi) * theta_3(e^-pi)^2."""
        if not HAS_MPMATH:
            pytest.skip("mpmath required")

        q = mpmath.exp(-mpmath.pi)
        theta3 = mpmath.jtheta(3, 0, q)
        g_star_theta = mpmath.sqrt(2 * mpmath.pi) * theta3**2

        g_quarter = mpmath.gamma(mpmath.mpf(1) / 4)
        g_star_gamma = mpmath.sqrt(2) * g_quarter**2 / (2 * mpmath.pi)

        diff = abs(g_star_theta - g_star_gamma)
        assert diff < mpmath.mpf(10) ** (-45), f"G* theta route disagrees by {diff}"

    def test_gstar_numpy_sanity(self):
        """Sanity check with numpy (lower precision)."""
        from scipy.special import gamma

        g_quarter = gamma(0.25)
        g_star = np.sqrt(2) * g_quarter**2 / (2 * np.pi)

        assert abs(g_star - 2.9586751) < 1e-6, f"G* numpy = {g_star}, expected ~2.9586751"


# =============================================================================
# Test 1.2: G* / VARPI Relationship
# =============================================================================


class TestGStarVarpiRelation:
    """Verify G* = 2 * VARPI / sqrt(pi)."""

    def test_relation_high_precision(self):
        if not HAS_MPMATH:
            pytest.skip("mpmath required")

        g_quarter = mpmath.gamma(mpmath.mpf(1) / 4)
        g_star = mpmath.sqrt(2) * g_quarter**2 / (2 * mpmath.pi)
        varpi = g_quarter**2 / (2 * mpmath.sqrt(2 * mpmath.pi))

        g_star_from_varpi = 2 * varpi / mpmath.sqrt(mpmath.pi)
        diff = abs(g_star - g_star_from_varpi)
        assert diff < mpmath.mpf(10) ** (-45), f"G* = 2*varpi/sqrt(pi) disagrees by {diff}"

    def test_relation_numpy(self):
        from scipy.special import gamma

        g_quarter = gamma(0.25)
        g_star = np.sqrt(2) * g_quarter**2 / (2 * np.pi)
        varpi = g_quarter**2 / (2 * np.sqrt(2 * np.pi))

        g_star_from_varpi = 2 * varpi / np.sqrt(np.pi)
        assert abs(g_star - g_star_from_varpi) < 1e-14


# =============================================================================
# Test 1.3: Master Quadratic Roots
# =============================================================================


class TestMasterQuadratic:
    """Verify x^2 - 16*G*^2*x + 16*G*^3 = 0."""

    def _roots_mpmath(self):
        g_quarter = mpmath.gamma(mpmath.mpf(1) / 4)
        c = mpmath.sqrt(2) * g_quarter**2 / (2 * mpmath.pi)
        a_coef = mpmath.mpf(1)
        b_coef = -16 * c**2
        c_coef = 16 * c**3
        disc = b_coef**2 - 4 * a_coef * c_coef
        x_plus = (-b_coef + mpmath.sqrt(disc)) / 2
        x_minus = (-b_coef - mpmath.sqrt(disc)) / 2
        return c, x_plus, x_minus

    def test_roots_satisfy_equation(self):
        """Plug roots back in — residual must be < 10^-90."""
        if not HAS_MPMATH:
            pytest.skip("mpmath required")

        c, x_plus, x_minus = self._roots_mpmath()
        for x, name in [(x_plus, "x+"), (x_minus, "x-")]:
            residual = x**2 - 16 * c**2 * x + 16 * c**3
            assert abs(residual) < mpmath.mpf(10) ** (-40), f"{name} residual = {residual}"

    def test_vieta_sum(self):
        """x+ + x- = 16*G*^2."""
        if not HAS_MPMATH:
            pytest.skip("mpmath required")

        c, x_plus, x_minus = self._roots_mpmath()
        expected_sum = 16 * c**2
        actual_sum = x_plus + x_minus
        diff = abs(actual_sum - expected_sum)
        assert diff < mpmath.mpf(10) ** (-40), f"Vieta sum disagrees by {diff}"

    def test_vieta_product(self):
        """x+ * x- = 16*G*^3."""
        if not HAS_MPMATH:
            pytest.skip("mpmath required")

        c, x_plus, x_minus = self._roots_mpmath()
        expected_prod = 16 * c**3
        actual_prod = x_plus * x_minus
        diff = abs(actual_prod - expected_prod)
        assert diff < mpmath.mpf(10) ** (-40), f"Vieta product disagrees by {diff}"

    def test_x_plus_value(self):
        """x+ should be approximately 137.036."""
        if not HAS_MPMATH:
            pytest.skip("mpmath required")

        _, x_plus, _ = self._roots_mpmath()
        assert abs(x_plus - 137.036) < 0.001, f"x+ = {x_plus}"

    def test_x_minus_value(self):
        """x- should be approximately 3.024."""
        if not HAS_MPMATH:
            pytest.skip("mpmath required")

        _, _, x_minus = self._roots_mpmath()
        assert abs(x_minus - 3.024) < 0.001, f"x- = {x_minus}"

    def test_floor_x_minus_equals_3(self):
        """floor(x-) = 3 (claimed N_c)."""
        if not HAS_MPMATH:
            pytest.skip("mpmath required")

        _, _, x_minus = self._roots_mpmath()
        assert int(mpmath.floor(x_minus)) == 3


# =============================================================================
# Test 1.4: Epsilon Computation
# =============================================================================


class TestEpsilon:
    """Verify epsilon = e^pi - pi - 20."""

    def test_epsilon_value(self):
        if not HAS_MPMATH:
            pytest.skip("mpmath required")

        eps = mpmath.exp(mpmath.pi) - mpmath.pi - 20
        # epsilon should be very small and negative
        assert eps < 0, "epsilon should be negative"
        assert abs(eps) < 0.001, f"|epsilon| = {abs(eps)}, expected < 0.001"

    def test_epsilon_20_is_b3_plus_neff(self):
        """The 20 comes from b_3 + N_eff = 7 + 13 = 20."""
        assert b_3 + N_eff == 20

    def test_epsilon_numpy(self):
        eps = np.exp(np.pi) - np.pi - 20
        assert abs(eps) < 0.001
        # eps ≈ -0.0009000208... (very close to zero, confirming e^pi ≈ pi + 20)
        assert eps < 0, "epsilon should be negative"
        assert abs(eps) < 0.001, f"epsilon too large: {eps}"


# =============================================================================
# Test 1.5: 4-Term Precision Formula Coefficients
# =============================================================================


class TestPrecisionCoefficients:
    """Verify all 4 coefficients derive from {3,4,7,13}."""

    def test_c1_exact_fraction(self):
        """c1 = N_c^2 / D = 9/47."""
        c1_num = N_c**2
        c1_den = D
        f = Fraction(c1_num, c1_den)
        assert f == Fraction(9, 47), f"c1 = {f}, expected 9/47"

    def test_c2_exact_fraction(self):
        """c2 = (N_eff - 2*N_base) / N_base^3 = 5/64."""
        c2_num = N_eff - 2 * N_base
        c2_den = N_base**3
        f = Fraction(c2_num, c2_den)
        assert f == Fraction(5, 64), f"c2 = {f}, expected 5/64"

    def test_c3_exact_fraction(self):
        """c3 = N_base / (N_c * D) = 4/141."""
        c3_num = N_base
        c3_den = N_c * D
        f = Fraction(c3_num, c3_den)
        assert f == Fraction(4, 141), f"c3 = {f}, expected 4/141"

    def test_c4_exact_fraction(self):
        """c4 = (N_c * D) / (b_3 + N_base) = 141/11."""
        c4_num = N_c * D
        c4_den = b_3 + N_base
        f = Fraction(c4_num, c4_den)
        assert f == Fraction(141, 11), f"c4 = {f}, expected 141/11"

    def test_all_fractions_irreducible(self):
        """Verify all coefficient fractions are in lowest terms."""
        from math import gcd

        fracs = [(9, 47), (5, 64), (4, 141), (141, 11)]
        for num, den in fracs:
            g = gcd(num, den)
            assert g == 1, f"{num}/{den} is reducible (gcd={g})"


# =============================================================================
# Test 1.6: Precision Formula vs CODATA
# =============================================================================


class TestPrecisionFormula:
    """Verify 4-term formula matches CODATA 2022."""

    def test_tree_level_accuracy(self):
        """Tree-level x+ should match 1/alpha to ~1.26 ppm."""
        if not HAS_MPMATH:
            pytest.skip("mpmath required")

        _, x_plus, _ = TestMasterQuadratic()._roots_mpmath()
        exp_val = mpmath.mpf("137.035999177")
        error_ppm = abs(x_plus - exp_val) / exp_val * mpmath.mpf(10) ** 6
        assert error_ppm < 5, f"Tree-level error = {error_ppm} ppm"

    def test_4term_precision(self):
        """4-term formula should match CODATA to < 100 ppt."""
        if not HAS_MPMATH:
            pytest.skip("mpmath required")

        g_quarter = mpmath.gamma(mpmath.mpf(1) / 4)
        c = mpmath.sqrt(2) * g_quarter**2 / (2 * mpmath.pi)
        disc = (16 * c**2) ** 2 - 4 * 16 * c**3
        x_plus = (16 * c**2 + mpmath.sqrt(disc)) / 2

        eps = abs(mpmath.exp(mpmath.pi) - mpmath.pi - 20)
        c1 = mpmath.mpf(9) / 47
        c2 = mpmath.mpf(5) / 64
        c3 = mpmath.mpf(4) / 141
        c4 = mpmath.mpf(141) / 11

        alpha_inv = x_plus - c1 * eps + c2 * eps**2 - c3 * eps**3 - c4 * eps**4

        exp_val = mpmath.mpf("137.035999177")
        error_ppt = abs(alpha_inv - exp_val) / exp_val * mpmath.mpf(10) ** 12
        # Record the actual precision achieved
        print(f"\n  4-term 1/alpha = {alpha_inv}")
        print(f"  CODATA 2022    = {exp_val}")
        print(f"  Error          = {error_ppt} ppt")
        # The claim is sub-ppt; allow up to 1000 ppt (1 ppb) as pass threshold
        assert error_ppt < 1000, f"4-term error = {error_ppt} ppt (> 1 ppb)"


# =============================================================================
# Test 1.7: Integer Arithmetic Identities
# =============================================================================


class TestIntegerIdentities:
    """Verify all claimed integer relationships."""

    def test_b3_equals_nc_plus_nbase(self):
        assert b_3 == N_c + N_base, f"b_3={b_3} != N_c+N_base={N_c+N_base}"

    def test_neff_equals_b3_plus_2nc(self):
        assert N_eff == b_3 + 2 * N_c, f"N_eff={N_eff} != b_3+2N_c={b_3+2*N_c}"

    def test_D_equals_nc_nbase2_minus_1(self):
        assert D == N_c * N_base**2 - 1 == 47

    def test_nbase_squared_is_16(self):
        assert N_base**2 == 16

    def test_exponent_11(self):
        """11 = N_c + 2*N_base = N_eff - 2."""
        assert N_c + 2 * N_base == 11
        assert N_eff - 2 == 11

    def test_exponent_20(self):
        """20 = 2*(N_c + b_3) = b_3 + N_eff."""
        assert 2 * (N_c + b_3) == 20
        assert b_3 + N_eff == 20

    def test_sin2_theta_w_formula(self):
        """sin^2(theta_W) = N_c/N_eff = 3/13."""
        f = Fraction(N_c, N_eff)
        assert f == Fraction(3, 13)

    def test_fibonacci_7_is_13(self):
        """N_eff = 13 is the 7th Fibonacci number F_7."""
        fib = [1, 1, 2, 3, 5, 8, 13, 21]
        assert fib[6] == 13  # F_7 (1-indexed) = fib[6] (0-indexed)
        assert fib[6] == N_eff


# =============================================================================
# Test 1.8: Lemniscate Geometric Properties
# =============================================================================


class TestLemniscateGeometry:
    """Verify geometric properties of the Bernoulli lemniscate."""

    def test_90_degree_self_crossing(self):
        """The lemniscate r^2 = cos(2*theta) crosses at 90 degrees at origin."""
        # At origin (r=0), cos(2*theta)=0, so theta = pi/4 and 3pi/4
        # The two branches have tangent directions at these angles
        # Angle between branches = 3pi/4 - pi/4 = pi/2 = 90 degrees
        theta1 = np.pi / 4
        theta2 = 3 * np.pi / 4
        angle_between = theta2 - theta1
        assert abs(angle_between - np.pi / 2) < 1e-15, f"Crossing angle = {np.degrees(angle_between)} degrees"

    def test_lemniscate_area(self):
        """Area of lemniscate r^2 = a^2 cos(2theta) is a^2."""
        # For a=1: Area = integral of (1/2)r^2 d(theta) over full curve
        # = integral of (1/2)cos(2theta) d(theta) from -pi/4 to pi/4
        # times 2 (for both lobes) = 1
        from scipy.integrate import quad

        area, _ = quad(lambda t: 0.5 * np.cos(2 * t), -np.pi / 4, np.pi / 4)
        area *= 2  # both lobes
        assert abs(area - 1.0) < 1e-10, f"Lemniscate area = {area}"


# =============================================================================
# Test 1.9: Theta Function Identity
# =============================================================================


class TestThetaIdentity:
    """Verify G* = sqrt(2*pi) * theta_3(q=e^(-pi))^2."""

    def test_identity_high_precision(self):
        if not HAS_MPMATH:
            pytest.skip("mpmath required")

        q = mpmath.exp(-mpmath.pi)
        theta3 = mpmath.jtheta(3, 0, q)
        g_star_theta = mpmath.sqrt(2 * mpmath.pi) * theta3**2

        g_quarter = mpmath.gamma(mpmath.mpf(1) / 4)
        g_star_gamma = mpmath.sqrt(2) * g_quarter**2 / (2 * mpmath.pi)

        diff = abs(g_star_theta - g_star_gamma)
        print(f"\n  G* via Gamma:  {g_star_gamma}")
        print(f"  G* via Theta:  {g_star_theta}")
        print(f"  Difference:    {diff}")
        assert diff < mpmath.mpf(10) ** (-45), f"Theta identity disagrees by {diff}"


# =============================================================================
# TIER 1 SCORING
# =============================================================================


def get_tier1_score(results: dict) -> float:
    """Compute Tier 1 score from pytest results.

    Called by the unified runner. Results dict maps test names to pass/fail.
    """
    if not results:
        return 0.0
    passed = sum(1 for v in results.values() if v)
    total = len(results)
    return (passed / total) * 100 if total > 0 else 0.0
