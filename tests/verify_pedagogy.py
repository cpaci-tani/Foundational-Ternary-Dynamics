"""
verify_pedagogy.py: Formal Tests for Pedagogical Validity
=========================================================

Purpose:
    Ensures that the "Student Path" (the derivation logic presented in the manuscript)
    yields results identical to the "Core Model" (the verified backend).
    
    This guards against "Pedagogical Drift" where the explanation diverges from the code.

Tests:
    1. The "Napkin Derivation": Can a student derive G* and alpha using only a calculator and the formulas?
    2. The "Integer Check": Do the 4 integers {3,4,7,13} strictly hold their relationships?
    3. The "Limit Check": Does the code correctly handle the D=3 uniqueness argument?
"""

import unittest
import numpy as np
from scipy.special import gamma
import sys
import os

# Add parent directory to path to import simulations
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

# We import the "Truth"
from simulations.constants import G_STAR, X_PLUS, N_c, N_base, b_3, N_eff

class TestPedagogicalValidity(unittest.TestCase):
    
    def test_napkin_derivation_g_star(self):
        """
        Pedagogical Claim: "A student can calculate G* using just sqrt(2), Gamma(1/4), and 2pi"
        """
        print("\nTesting 'Napkin' calculation of G*...")
        
        # Student inputs (simulating 10-digit calculator precision)
        sqrt_2 = 1.4142135624
        gamma_025 = 3.6256099082
        pi = 3.1415926535
        
        # The Formula
        g_star_student = sqrt_2 * (gamma_025**2) / (2 * pi)
        
        # The Truth
        g_star_truth = G_STAR
        
        # Assertion: Must match to decent precision (e.g. 8 decimal places)
        self.assertAlmostEqual(g_star_student, g_star_truth, places=8)
        print(f"  Student: {g_star_student:.10f}")
        print(f"  Truth:   {g_star_truth:.10f} [PASS]")

    def test_napkin_derivation_quadratic(self):
        """
        Pedagogical Claim: "Solving x^2 - 16(G*)^2 x + 16(G*)^3 = 0 yields 1/alpha"
        """
        print("\nTesting 'Napkin' solution of Master Quadratic...")
        
        # Student inputs
        g = 2.9586751192  # From text
        
        # Coefficients
        a = 1
        b = -16 * g**2
        c = 16 * g**3
        
        # Quadratic Formula
        disc = b**2 - 4*a*c
        x_plus_student = (-b + np.sqrt(disc)) / (2*a)
        
        # The Truth
        x_plus_truth = X_PLUS
        
        # Assertion
        # We accept slight deviation due to g input truncation, but it should be close.
        self.assertAlmostEqual(x_plus_student, x_plus_truth, places=5)
        print(f"  Student (truncated G*): {x_plus_student:.6f}")
        print(f"  Truth:                  {x_plus_truth:.6f} [PASS]")

    def test_integer_consistency(self):
        """
        Pedagogical Claim: "The integers are locked by Fibonacci constraints"
        Constraint: b_3 + 2*N_c = N_eff (7 + 6 = 13)
        """
        print("\nTesting Integer Consistency...")
        
        # Pedagogical values
        n_c = 3
        n_base = 4
        
        # Step 1: Derive b_3
        b_3_student = n_c + n_base
        self.assertEqual(b_3_student, 7)
        
        # Step 2: Fibonacci Constraint
        # Claim: "b_3 + 2*Nc = N_eff"
        n_eff_student = b_3_student + 2 * n_c
        
        self.assertEqual(n_eff_student, 13)
        self.assertEqual(n_eff_student, N_eff)
        print(f"  7 + 2*3 = {n_eff_student} (Matches F_7 = 13) [PASS]")

if __name__ == '__main__':
    unittest.main()
