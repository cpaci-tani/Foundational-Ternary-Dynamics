"""
test_master_quadratic.py: Verify the Fine Structure Constant Derivation
=======================================================================

The master quadratic is the central equation of FTD:

    x^2 - 16*G*^2*x + 16*G*^3 = 0

Where G* is the lemniscatic constant (Gauss's constant):

    G* = sqrt(2) * Gamma(1/4)^2 / (2*pi) = 2.9586751192...

The larger root x_+ = 137.0360... is identified with 1/alpha.

This test verifies:
1. G* computation from mathematical constants
2. Master quadratic solution
3. Agreement with experimental 1/alpha to 1.26 ppm
4. The smaller root x_- ~ 3.024 relates to N_c = 3
"""

import unittest
import numpy as np
from scipy.special import gamma

# Experimental value (CODATA 2022)
ALPHA_INV_EXP = 137.035999177  # +/- 0.000000021


class TestLemniscaticConstant(unittest.TestCase):
    """Test the derivation of G* (lemniscatic constant)."""

    def test_g_star_formula(self):
        """
        Verify G* = sqrt(2) * Gamma(1/4)^2 / (2*pi)

        The lemniscatic constant arises from elliptic integrals
        and appears in the arc length of the lemniscate curve.
        """
        gamma_quarter = gamma(0.25)
        g_star = np.sqrt(2) * gamma_quarter**2 / (2 * np.pi)

        # Expected value from mathematical tables
        expected = 2.9586751192
        self.assertAlmostEqual(g_star, expected, places=8)

        print(f"\n  Gamma(1/4) = {gamma_quarter:.10f}")
        print("  G* = sqrt(2) * Gamma(1/4)^2 / (2*pi)")
        print(f"     = sqrt(2) * {gamma_quarter:.6f}^2 / (2*pi)")
        print(f"     = {g_star:.10f} [PASS]")

    def test_g_star_alternative_forms(self):
        """
        Verify G* computation via elliptic integral K.

        G* = sqrt(2) * Gamma(1/4)^2 / (2*pi) is the primary definition.
        This relates to the complete elliptic integral K(1/sqrt(2)).

        The relation is: K(1/sqrt(2)) = Gamma(1/4)^2 / (4*sqrt(pi))
        And G* = sqrt(2) * Gamma(1/4)^2 / (2*pi) = 2*sqrt(2/pi) * K(1/sqrt(2))
        """
        from scipy.special import ellipk

        # Standard definition of G* via Gamma function
        gamma_quarter = gamma(0.25)
        g_star_gamma = np.sqrt(2) * gamma_quarter**2 / (2 * np.pi)

        # Via complete elliptic integral K(1/sqrt(2))
        # K(k) = integral from 0 to pi/2 of 1/sqrt(1 - k^2 sin^2(t)) dt
        k = 1 / np.sqrt(2)
        K_val = ellipk(k**2)  # scipy takes k^2, not k

        # The relation: Gamma(1/4)^2 = 4*sqrt(pi) * K(1/sqrt(2))
        # So G* = sqrt(2) * 4*sqrt(pi)*K / (2*pi) = 2*sqrt(2/pi) * K
        g_star_elliptic = 2 * np.sqrt(2 / np.pi) * K_val

        # These should match
        self.assertAlmostEqual(g_star_gamma, g_star_elliptic, places=8)
        print(f"\n  G* via Gamma(1/4): {g_star_gamma:.10f}")
        print(f"  G* via K(1/sqrt(2)): {g_star_elliptic:.10f}")
        print("  Agreement verified [PASS]")


class TestMasterQuadratic(unittest.TestCase):
    """Test the master quadratic equation and its roots."""

    def setUp(self):
        """Compute G* once for all tests."""
        self.gamma_quarter = gamma(0.25)
        self.g_star = np.sqrt(2) * self.gamma_quarter**2 / (2 * np.pi)

    def test_quadratic_coefficients(self):
        """
        Verify the quadratic coefficients.

        x^2 - 16*G*^2*x + 16*G*^3 = 0

        The coefficient 16 = N_base^2 = 4^2 comes from lattice degrees of freedom.
        """
        c = self.g_star

        a_coef = 1
        b_coef = -16 * c**2
        c_coef = 16 * c**3

        # Verify structure
        self.assertEqual(a_coef, 1)
        self.assertAlmostEqual(b_coef, -16 * self.g_star**2, places=10)
        self.assertAlmostEqual(c_coef, 16 * self.g_star**3, places=10)

        print(f"\n  Master quadratic: x^2 + ({b_coef:.6f})x + ({c_coef:.6f}) = 0")
        print(f"  Coefficients derived from G* = {self.g_star:.10f} [PASS]")

    def test_quadratic_roots(self):
        """
        Solve the master quadratic and verify both roots.

        x^2 - 16*G*^2*x + 16*G*^3 = 0
        x_+ ~ 137.036 (identified with 1/alpha)
        x_- ~ 3.024 (floor gives N_c = 3)
        """
        c = self.g_star

        # Quadratic formula
        a = 1
        b = -16 * c**2
        c_coef = 16 * c**3

        discriminant = b**2 - 4 * a * c_coef
        self.assertGreater(discriminant, 0, "Discriminant must be positive")

        x_plus = (-b + np.sqrt(discriminant)) / (2 * a)
        x_minus = (-b - np.sqrt(discriminant)) / (2 * a)

        # Verify expected values
        self.assertAlmostEqual(x_plus, 137.036, places=2)
        self.assertAlmostEqual(x_minus, 3.024, places=2)

        print(f"\n  Discriminant = {discriminant:.6f}")
        print(f"  x_+ = {x_plus:.10f}")
        print(f"  x_- = {x_minus:.10f} [PASS]")

    def test_vieta_relations(self):
        """
        Verify Vieta's formulas: sum and product of roots.

        x_+ + x_- = 16*G*^2
        x_+ * x_- = 16*G*^3
        """
        c = self.g_star

        # Compute roots
        a = 1
        b = -16 * c**2
        c_coef = 16 * c**3
        discriminant = b**2 - 4 * a * c_coef

        x_plus = (-b + np.sqrt(discriminant)) / (2 * a)
        x_minus = (-b - np.sqrt(discriminant)) / (2 * a)

        # Vieta's formulas
        root_sum = x_plus + x_minus
        root_product = x_plus * x_minus

        expected_sum = 16 * c**2
        expected_product = 16 * c**3

        self.assertAlmostEqual(root_sum, expected_sum, places=10)
        self.assertAlmostEqual(root_product, expected_product, places=10)

        print(f"\n  x_+ + x_- = {root_sum:.6f}")
        print(f"  16*G*^2   = {expected_sum:.6f}")
        print(f"  x_+ * x_- = {root_product:.6f}")
        print(f"  16*G*^3   = {expected_product:.6f} [PASS]")


class TestFineStructureConstant(unittest.TestCase):
    """Test that x_+ matches the experimental fine structure constant."""

    def setUp(self):
        """Compute the predicted 1/alpha."""
        gamma_quarter = gamma(0.25)
        g_star = np.sqrt(2) * gamma_quarter**2 / (2 * np.pi)

        a = 1
        b = -16 * g_star**2
        c = 16 * g_star**3
        discriminant = b**2 - 4 * a * c

        self.x_plus = (-b + np.sqrt(discriminant)) / (2 * a)
        self.x_minus = (-b - np.sqrt(discriminant)) / (2 * a)

    def test_alpha_inverse_value(self):
        """
        Verify x_+ = 1/alpha to high precision.

        Experimental: 1/alpha = 137.035999177 (CODATA 2022)
        FTD derived:  1/alpha = 137.0360...
        """
        alpha_inv_derived = self.x_plus
        alpha_inv_exp = ALPHA_INV_EXP

        error_ppm = abs(alpha_inv_derived - alpha_inv_exp) / alpha_inv_exp * 1e6

        # Should be within 2 ppm
        self.assertLess(error_ppm, 2.0)

        print(f"\n  FTD derived:   1/alpha = {alpha_inv_derived:.10f}")
        print(f"  Experimental:  1/alpha = {alpha_inv_exp:.10f}")
        print(f"  Error: {error_ppm:.2f} ppm [PASS]")

    def test_alpha_value(self):
        """Verify alpha itself."""
        alpha_derived = 1 / self.x_plus
        alpha_exp = 1 / ALPHA_INV_EXP

        error_ppm = abs(alpha_derived - alpha_exp) / alpha_exp * 1e6

        self.assertLess(error_ppm, 2.0)

        print(f"\n  FTD derived:   alpha = {alpha_derived:.12f}")
        print(f"  Experimental:  alpha = {alpha_exp:.12f}")
        print(f"  Error: {error_ppm:.2f} ppm [PASS]")

    def test_x_minus_color_relation(self):
        """
        Verify x_- relates to N_c = 3.

        x_- = 3.024... and floor(x_-) = 3 = number of colors.
        This is used to derive N_gen = 3 (fermion generations).
        """
        x_minus = self.x_minus
        floor_x_minus = int(np.floor(x_minus))

        self.assertEqual(floor_x_minus, 3)
        self.assertAlmostEqual(x_minus, 3.024, places=2)

        print(f"\n  x_- = {x_minus:.10f}")
        print(f"  floor(x_-) = {floor_x_minus} = N_c = N_gen [PASS]")


class TestNapkinDerivation(unittest.TestCase):
    """
    Test that a student can derive these values with a calculator.

    This verifies the pedagogical claim that the derivation is
    reproducible with minimal tools.
    """

    def test_calculator_g_star(self):
        """
        Derive G* using only basic calculator inputs.

        A student needs: sqrt(2), Gamma(1/4), pi
        These are available in scientific calculators or tables.
        """
        # Typical calculator precision (10 digits)
        sqrt_2 = 1.4142135624
        gamma_quarter = 3.6256099082  # From tables
        pi = 3.1415926536

        g_star_calc = sqrt_2 * (gamma_quarter**2) / (2 * pi)

        # Compare to high-precision
        g_star_exact = np.sqrt(2) * gamma(0.25) ** 2 / (2 * np.pi)

        error = abs(g_star_calc - g_star_exact)
        self.assertLess(error, 1e-8)

        print("\n  Calculator inputs:")
        print(f"    sqrt(2)     = {sqrt_2}")
        print(f"    Gamma(1/4)  = {gamma_quarter}")
        print(f"    pi          = {pi}")
        print(f"  Calculator G* = {g_star_calc:.10f}")
        print(f"  Exact G*      = {g_star_exact:.10f}")
        print(f"  Error: {error:.2e} [PASS]")

    def test_calculator_quadratic(self):
        """
        Solve the quadratic using standard formula.
        """
        g = 2.9586751192  # G* from previous calculation

        # Coefficients
        a = 1
        b = -16 * g**2
        c = 16 * g**3

        # Quadratic formula
        disc = b**2 - 4 * a * c
        x_plus = (-b + np.sqrt(disc)) / (2 * a)

        # Should get approximately 137
        self.assertAlmostEqual(x_plus, 137.036, places=2)

        print(f"\n  Using G* = {g}")
        print(f"  Quadratic: x^2 + ({b:.4f})x + ({c:.4f}) = 0")
        print(f"  Discriminant = {disc:.4f}")
        print(f"  x_+ = {x_plus:.6f} ~ 137 [PASS]")


if __name__ == "__main__":
    print("=" * 60)
    print("MASTER QUADRATIC VERIFICATION")
    print("Fine Structure Constant Derivation")
    print("=" * 60)
    unittest.main(verbosity=2)
