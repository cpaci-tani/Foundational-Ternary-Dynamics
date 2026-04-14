#!/usr/bin/env python3
"""
run_all_tests.py: Master Test Runner for FTD Verification Suite
================================================================

This script runs all verification tests and produces a comprehensive
summary report suitable for public transparency and peer review.

Usage:
    python tests/run_all_tests.py

    Options:
        --verbose    Show detailed test output
        --summary    Show only summary (default)
        --json       Output results as JSON

Output:
    - Console: Pass/Fail summary with error metrics
    - File: tests/verification_report.txt (detailed results)
"""

import unittest
import sys
import os
import json
from datetime import datetime
from io import StringIO

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

# Import all test modules
from scripts.tests import test_framework_integers
from scripts.tests import test_master_quadratic
from scripts.tests import test_particle_masses
from scripts.tests import test_coupling_constants
from scripts.tests import test_cosmology
from scripts.tests import test_mixing_matrices
from scripts.tests import verify_pedagogy


def create_test_suite():
    """Create a comprehensive test suite from all test modules."""
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()

    # Add all test modules
    suite.addTests(loader.loadTestsFromModule(test_framework_integers))
    suite.addTests(loader.loadTestsFromModule(test_master_quadratic))
    suite.addTests(loader.loadTestsFromModule(test_particle_masses))
    suite.addTests(loader.loadTestsFromModule(test_coupling_constants))
    suite.addTests(loader.loadTestsFromModule(test_cosmology))
    suite.addTests(loader.loadTestsFromModule(test_mixing_matrices))
    suite.addTests(loader.loadTestsFromModule(verify_pedagogy))

    return suite


def run_tests(verbosity=1):
    """Run all tests and return results."""
    suite = create_test_suite()

    # Capture output
    stream = StringIO()
    runner = unittest.TextTestRunner(stream=stream, verbosity=verbosity)
    result = runner.run(suite)

    return result, stream.getvalue()


def print_banner():
    """Print the test suite banner."""
    print("=" * 70)
    print("FOUNDATIONAL TERNARY DYNAMICS - VERIFICATION TEST SUITE")
    print("=" * 70)
    print(f"Date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"Python: {sys.version.split()[0]}")
    print("-" * 70)


def print_summary(result):
    """Print a summary of test results."""
    total = result.testsRun
    failures = len(result.failures)
    errors = len(result.errors)
    skipped = len(result.skipped) if hasattr(result, "skipped") else 0
    passed = total - failures - errors - skipped

    print("\n" + "=" * 70)
    print("TEST SUMMARY")
    print("=" * 70)
    print(f"  Total Tests:  {total}")
    print(f"  Passed:       {passed} ({100*passed/total:.1f}%)" if total > 0 else "  Passed:       0")
    print(f"  Failed:       {failures}")
    print(f"  Errors:       {errors}")
    print(f"  Skipped:      {skipped}")
    print("-" * 70)

    if failures == 0 and errors == 0:
        print("  STATUS: ALL TESTS PASSED")
    else:
        print("  STATUS: SOME TESTS FAILED")

    print("=" * 70)

    return passed, total


def print_key_results():
    """Print key numerical results for quick verification."""
    import numpy as np
    from scipy.special import gamma

    # Compute key values
    gamma_quarter = gamma(0.25)
    g_star = np.sqrt(2) * gamma_quarter**2 / (2 * np.pi)

    c = g_star
    a = 1
    b = -16 * c**2
    c_coef = 16 * c**3
    discriminant = b**2 - 4 * a * c_coef
    x_plus = (-b + np.sqrt(discriminant)) / (2 * a)
    x_minus = (-b - np.sqrt(discriminant)) / (2 * a)

    alpha = 1 / x_plus
    alpha_exp = 1 / 137.035999177

    # Framework integers
    N_c = 3
    N_base = 4
    b_3 = 7
    N_eff = 13

    # Mass calculation
    M_PLANCK = 1.220890e19
    m_e_derived = M_PLANCK * np.sqrt(2 * np.pi) * (N_base**2 / N_c) * alpha**11 * 1000

    print("\n" + "=" * 70)
    print("KEY NUMERICAL RESULTS")
    print("=" * 70)
    print("\nFramework Integers:")
    print(f"  N_c = {N_c}, N_base = {N_base}, b_3 = {b_3}, N_eff = {N_eff}")
    print(f"  Fibonacci check: {b_3} + 2*{N_c} = {b_3 + 2*N_c} = N_eff? {b_3 + 2*N_c == N_eff}")

    print("\nLemniscatic Constant:")
    print(f"  G* = {g_star:.10f}")

    print("\nMaster Quadratic Roots:")
    print(f"  x_+ = {x_plus:.10f}")
    print(f"  x_- = {x_minus:.10f}")

    print("\nFine Structure Constant:")
    print(f"  1/alpha (FTD):  {x_plus:.10f}")
    print("  1/alpha (exp):  137.035999177")
    print(f"  Error: {abs(x_plus - 137.035999177)/137.035999177 * 1e6:.2f} ppm")

    print("\nElectron Mass:")
    print(f"  m_e (FTD): {m_e_derived:.6f} MeV")
    print("  m_e (exp): 0.510999 MeV")
    print(f"  Error: {abs(m_e_derived - 0.510999)/0.510999 * 100:.3f}%")

    print("=" * 70)


def save_report(result, output, filename="tests/verification_report.txt"):
    """Save detailed test report to file."""
    report_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), filename)

    with open(report_path, "w") as f:
        f.write("=" * 70 + "\n")
        f.write("FTD VERIFICATION REPORT\n")
        f.write("=" * 70 + "\n")
        f.write(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        f.write(f"Python Version: {sys.version}\n")
        f.write("-" * 70 + "\n\n")

        f.write("TEST RESULTS\n")
        f.write("-" * 70 + "\n")
        f.write(output)

        if result.failures:
            f.write("\n\nFAILURES:\n")
            for test, traceback in result.failures:
                f.write(f"\n{test}:\n{traceback}\n")

        if result.errors:
            f.write("\n\nERRORS:\n")
            for test, traceback in result.errors:
                f.write(f"\n{test}:\n{traceback}\n")

    print(f"\nDetailed report saved to: {report_path}")


def main():
    """Main entry point."""
    # Parse arguments
    verbose = "--verbose" in sys.argv or "-v" in sys.argv
    json_output = "--json" in sys.argv
    summary_only = "--summary" in sys.argv

    if not json_output:
        print_banner()

    # Run tests
    verbosity = 2 if verbose else 1
    result, output = run_tests(verbosity=verbosity)

    if json_output:
        # Output as JSON
        data = {
            "timestamp": datetime.now().isoformat(),
            "total": result.testsRun,
            "passed": result.testsRun - len(result.failures) - len(result.errors),
            "failures": len(result.failures),
            "errors": len(result.errors),
            "success": len(result.failures) == 0 and len(result.errors) == 0,
        }
        print(json.dumps(data, indent=2))
    else:
        if verbose:
            print(output)

        passed, total = print_summary(result)

        if not summary_only:
            print_key_results()

        # Save report
        save_report(result, output)

    # Exit with appropriate code
    sys.exit(0 if result.wasSuccessful() else 1)


if __name__ == "__main__":
    main()
