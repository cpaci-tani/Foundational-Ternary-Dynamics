#!/usr/bin/env python3
"""
Decisive Tests Runner for Foundational Ternary Dynamics
========================================================

Implements the 5-Phase Verification Protocol from AUDIT_PANEL_RESPONSE.md:
  Phase 1: Infrastructure Verification
  Phase 2: Classical Phenomena
  Phase 3: Quantum Phenomena
  Phase 4: Consistency Tests
  Phase 5: Stress Tests

Runs all verification scripts and generates a comprehensive report.

Success Criteria (from AUDIT_PANEL_RESPONSE.md):
  - Bell S-parameter: > 2.7 (quantum) or <= 2 (classical baseline)
  - Born correlation: > 0.95
  - Energy conservation: < 10^-6
  - Charge conservation: < 10^-8
  - Isotropy deviation: < 1%
  - Alpha accuracy: < 10 ppm

Author: Claude Code
Date: January 31, 2026
Version: 5.11
"""

import sys
import os
import time
from pathlib import Path
from datetime import datetime

# Add paths for imports
SCRIPT_DIR = Path(__file__).parent
ROOT_DIR = SCRIPT_DIR.parent.parent
sys.path.insert(0, str(ROOT_DIR / "scripts"))
sys.path.insert(0, str(ROOT_DIR / "media" / "utils"))

import numpy as np

# =============================================================================
# TEST RESULT TRACKING
# =============================================================================

class TestResult:
    """Container for test result data."""
    def __init__(self, name, passed, value=None, target=None, error=None, details=None):
        self.name = name
        self.passed = passed
        self.value = value
        self.target = target
        self.error = error
        self.details = details or ""
        self.timestamp = datetime.now()

class TestSuite:
    """Collects and reports test results."""
    def __init__(self, name):
        self.name = name
        self.results = []
        self.start_time = None
        self.end_time = None

    def start(self):
        self.start_time = time.time()
        print(f"\n{'='*70}")
        print(f"PHASE: {self.name}")
        print(f"{'='*70}")

    def end(self):
        self.end_time = time.time()
        passed = sum(1 for r in self.results if r.passed)
        total = len(self.results)
        duration = self.end_time - self.start_time
        print(f"\nPhase Complete: {passed}/{total} tests passed ({duration:.2f}s)")

    def add(self, result):
        self.results.append(result)
        status = "[PASS]" if result.passed else "[FAIL]"
        print(f"  {status} {result.name}", end="")
        if result.value is not None:
            if result.target is not None:
                print(f" = {result.value:.6g} (target: {result.target})", end="")
            else:
                print(f" = {result.value:.6g}", end="")
        if result.error is not None:
            print(f" [error: {result.error:.2e}]", end="")
        print()
        if result.details and not result.passed:
            print(f"      Details: {result.details}")

# =============================================================================
# PHASE 1: INFRASTRUCTURE VERIFICATION
# =============================================================================

def run_phase1_infrastructure():
    """Verify basic framework constants and mathematical derivations."""
    suite = TestSuite("1: INFRASTRUCTURE VERIFICATION")
    suite.start()

    # Import constants
    try:
        from constants import (
            G_STAR, X_PLUS, X_MINUS, ALPHA, ALPHA_INV,
            N_c, N_base, b_3, N_eff, Experimental, ppm_error
        )
        suite.add(TestResult("Import constants module", True))
    except ImportError as e:
        suite.add(TestResult("Import constants module", False, details=str(e)))
        suite.end()
        return suite

    # Test G* computation
    from scipy.special import gamma
    g_star_computed = np.sqrt(2) * gamma(0.25)**2 / (2 * np.pi)
    g_star_match = np.isclose(g_star_computed, G_STAR, rtol=1e-10)
    suite.add(TestResult("G* computation", g_star_match, G_STAR, 2.9586751192))

    # Test master quadratic roots
    # x^2 - 16G*^2*x + 16G*^3 = 0
    residual_plus = X_PLUS**2 - 16*G_STAR**2*X_PLUS + 16*G_STAR**3
    residual_minus = X_MINUS**2 - 16*G_STAR**2*X_MINUS + 16*G_STAR**3

    suite.add(TestResult("x+ is root of quadratic", abs(residual_plus) < 1e-8,
                         abs(residual_plus), 0))
    suite.add(TestResult("x- is root of quadratic", abs(residual_minus) < 1e-8,
                         abs(residual_minus), 0))

    # Test Vieta relations
    vieta_sum = abs((X_PLUS + X_MINUS) - 16*G_STAR**2)
    vieta_prod = abs((X_PLUS * X_MINUS) - 16*G_STAR**3)
    suite.add(TestResult("Vieta sum relation", vieta_sum < 1e-8, vieta_sum, 0))
    suite.add(TestResult("Vieta product relation", vieta_prod < 1e-8, vieta_prod, 0))

    # Test alpha accuracy (< 10 ppm)
    alpha_ppm = ppm_error(ALPHA_INV, Experimental.alpha_inv)
    suite.add(TestResult("Alpha accuracy < 10 ppm", alpha_ppm < 10, alpha_ppm, 10))

    # Test N_c floor
    n_c_floor = int(np.floor(X_MINUS))
    suite.add(TestResult("floor(x-) = 3 (N_c)", n_c_floor == 3, n_c_floor, 3))

    # Framework integers consistency
    fib_check = (N_eff == 13) and (b_3 + 2*N_c == N_eff)  # Fibonacci constraint
    suite.add(TestResult("Framework integer constraint", fib_check,
                         details=f"N_eff={N_eff}, b_3+2*N_c={b_3+2*N_c}"))

    suite.end()
    return suite

# =============================================================================
# PHASE 2: CLASSICAL PHENOMENA
# =============================================================================

def run_phase2_classical():
    """Test classical physics phenomena."""
    suite = TestSuite("2: CLASSICAL PHENOMENA")
    suite.start()

    # Test Born rule correlation
    try:
        from verify_born_rule import run_born_verification
        correlation = run_born_verification(n_samples=100000)
        suite.add(TestResult("Born rule correlation > 0.95", correlation > 0.95,
                             correlation, 0.95))
    except Exception as e:
        suite.add(TestResult("Born rule verification", False, details=str(e)))

    # Test relativity (time dilation)
    try:
        from verify_relativity import run_relativity_experiment
        passed = run_relativity_experiment()
        suite.add(TestResult("Lorentz factor emergence", passed))
    except Exception as e:
        suite.add(TestResult("Relativity verification", False, details=str(e)))

    suite.end()
    return suite

# =============================================================================
# PHASE 3: QUANTUM PHENOMENA
# =============================================================================

def run_phase3_quantum():
    """Test quantum mechanical predictions."""
    suite = TestSuite("3: QUANTUM PHENOMENA")
    suite.start()

    # Classical Bell test (baseline)
    try:
        from verify_bell_inequality import simulate_bell_experiment
        S_classical = simulate_bell_experiment(n_trials=50000)
        # Classical hidden variable model should give S <= 2
        suite.add(TestResult("Classical Bell bound S <= 2", S_classical <= 2.01,
                             S_classical, 2.0,
                             details="Local realistic model correctly respects bound"))
    except Exception as e:
        suite.add(TestResult("Classical Bell test", False, details=str(e)))

    # Quantum Bell test (if available)
    try:
        # Check if quantum Bell test exists
        quantum_bell_path = ROOT_DIR / "scripts" / "verify_bell_quantum.py"
        if quantum_bell_path.exists():
            from verify_bell_quantum import run_quantum_bell_test
            S_quantum = run_quantum_bell_test()
            suite.add(TestResult("Quantum Bell S > 2.7", S_quantum > 2.7,
                                 S_quantum, 2.7))
        else:
            suite.add(TestResult("Quantum Bell test", False,
                                 details="verify_bell_quantum.py not yet implemented"))
    except ImportError:
        suite.add(TestResult("Quantum Bell test", False,
                             details="Module not available"))
    except Exception as e:
        suite.add(TestResult("Quantum Bell test", False, details=str(e)))

    suite.end()
    return suite

# =============================================================================
# PHASE 4: CONSISTENCY TESTS
# =============================================================================

def run_phase4_consistency():
    """Test internal consistency of the framework."""
    suite = TestSuite("4: CONSISTENCY TESTS")
    suite.start()

    # Import needed modules
    from constants import G_STAR, X_PLUS, X_MINUS, N_c, N_base

    # G* from two lemniscate forms (+6.41 ppm match; corrected 2026-05-01 from 5.45 ppm)
    try:
        from physics_constants import verify_gstar_from_both_curves
        result = verify_gstar_from_both_curves()
        ppm_match = result['discrepancy_ppm'] < 10  # Allow up to 10 ppm
        suite.add(TestResult("Bernoulli-Alpha G* match < 10 ppm", ppm_match,
                             result['discrepancy_ppm'], 10))
    except Exception as e:
        suite.add(TestResult("Lemniscate equivalence", False, details=str(e)))

    # Mandelbrot bridge equation
    try:
        from physics_constants import verify_mandelbrot_bridge
        result = verify_mandelbrot_bridge()
        bridge_ok = result['equals_one']
        suite.add(TestResult("Bridge equation k_c * c_cusp * G* = 1", bridge_ok,
                             result['product'], 1.0))
    except Exception as e:
        suite.add(TestResult("Mandelbrot bridge", False, details=str(e)))

    # j-invariant = 1728 = 12^3 = (N_base * N_c)^3
    j_check = 1728 == (N_base * N_c)**3
    suite.add(TestResult("j-invariant = (N_base * N_c)^3", j_check,
                         (N_base * N_c)**3, 1728))

    # Coefficient 16 = N_base^2 = 2^N_base
    coef_check = (16 == N_base**2) and (16 == 2**N_base)
    suite.add(TestResult("Coefficient 16 dual derivation", coef_check))

    suite.end()
    return suite

# =============================================================================
# PHASE 5: CONSERVATION LAWS
# =============================================================================

def run_phase5_conservation():
    """Test conservation laws."""
    suite = TestSuite("5: CONSERVATION & STRESS TESTS")
    suite.start()

    # Check if conservation test exists
    conservation_path = ROOT_DIR / "scripts" / "verification" / "verify_conservation_laws.py"
    if conservation_path.exists():
        try:
            from verify_conservation_laws import run_conservation_tests
            results = run_conservation_tests()
            for name, passed, value, target in results:
                suite.add(TestResult(name, passed, value, target))
        except Exception as e:
            suite.add(TestResult("Conservation tests", False, details=str(e)))
    else:
        suite.add(TestResult("Conservation tests", False,
                             details="verify_conservation_laws.py not yet implemented"))

    # Check if stress tests exist
    stress_path = ROOT_DIR / "scripts" / "verification" / "run_stress_tests.py"
    if stress_path.exists():
        try:
            from run_stress_tests import run_all_stress_tests
            results = run_all_stress_tests()
            for name, passed, value, target in results:
                suite.add(TestResult(name, passed, value, target))
        except Exception as e:
            suite.add(TestResult("Stress tests", False, details=str(e)))
    else:
        suite.add(TestResult("Stress tests", False,
                             details="run_stress_tests.py not yet implemented"))

    suite.end()
    return suite

# =============================================================================
# MAIN RUNNER
# =============================================================================

def generate_report(suites):
    """Generate final test report."""
    print("\n")
    print("=" * 70)
    print("DECISIVE TESTS SUMMARY REPORT")
    print("=" * 70)
    print(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"Framework: Foundational Ternary Dynamics v5.11")
    print()

    total_passed = 0
    total_tests = 0

    for suite in suites:
        passed = sum(1 for r in suite.results if r.passed)
        total = len(suite.results)
        total_passed += passed
        total_tests += total

        status = "PASS" if passed == total else "PARTIAL" if passed > 0 else "FAIL"
        print(f"  {suite.name}: {passed}/{total} [{status}]")

    print()
    print("-" * 70)
    print(f"OVERALL: {total_passed}/{total_tests} tests passed")

    pass_rate = total_passed / total_tests * 100 if total_tests > 0 else 0
    if pass_rate == 100:
        grade = "A+ (ALL TESTS PASS)"
    elif pass_rate >= 90:
        grade = "A (EXCELLENT)"
    elif pass_rate >= 80:
        grade = "B+ (GOOD)"
    elif pass_rate >= 70:
        grade = "B (ACCEPTABLE)"
    elif pass_rate >= 60:
        grade = "C (NEEDS WORK)"
    else:
        grade = "F (CRITICAL ISSUES)"

    print(f"GRADE: {grade} ({pass_rate:.1f}%)")
    print("=" * 70)

    # List failures
    failures = []
    for suite in suites:
        for r in suite.results:
            if not r.passed:
                failures.append((suite.name, r))

    if failures:
        print("\nFAILED TESTS:")
        for suite_name, result in failures:
            print(f"  - [{suite_name}] {result.name}")
            if result.details:
                print(f"    Reason: {result.details}")

    return total_passed, total_tests

def main():
    """Run all decisive tests."""
    print("=" * 70)
    print("FOUNDATIONAL TERNARY DYNAMICS - DECISIVE TESTS")
    print("5-Phase Verification Protocol (AUDIT_PANEL_RESPONSE.md)")
    print("=" * 70)
    print(f"Started: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

    suites = []

    # Run all phases
    suites.append(run_phase1_infrastructure())
    suites.append(run_phase2_classical())
    suites.append(run_phase3_quantum())
    suites.append(run_phase4_consistency())
    suites.append(run_phase5_conservation())

    # Generate report
    passed, total = generate_report(suites)

    # Return exit code
    return 0 if passed == total else 1

if __name__ == "__main__":
    sys.exit(main())
