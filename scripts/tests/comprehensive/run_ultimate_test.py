#!/usr/bin/env python3
"""
FTD ULTIMATE VERIFICATION -- Unified Runner & Verdict Generator

Runs all 7 tiers of the comprehensive test suite and produces
a scored final verdict.

Usage:
    python tests/comprehensive/run_ultimate_test.py
    # or
    python -m pytest tests/comprehensive/ -v --tb=short
"""

import sys
import os
import subprocess
import re
from datetime import datetime

# Add project root to path
PROJECT_ROOT = os.path.join(os.path.dirname(__file__), "..", "..")
sys.path.insert(0, PROJECT_ROOT)

from scripts.tests.comprehensive.ftd_test_utils import TierResult, compute_verdict, format_verdict_report


def run_pytest_tier(tier_file: str, tier_name: str) -> TierResult:
    """Run a single tier's tests via pytest and parse results."""
    test_path = os.path.join(os.path.dirname(__file__), tier_file)

    result = subprocess.run(
        [sys.executable, "-m", "pytest", test_path, "-v", "--tb=short", "-q"],
        capture_output=True,
        text=True,
        cwd=PROJECT_ROOT,
        timeout=300,
    )

    output = result.stdout + result.stderr

    # Parse pytest summary line: "X passed, Y failed, Z errors"
    passed = 0
    failed = 0
    errors = 0

    summary_match = re.search(r"(\d+) passed", output)
    if summary_match:
        passed = int(summary_match.group(1))

    fail_match = re.search(r"(\d+) failed", output)
    if fail_match:
        failed = int(fail_match.group(1))

    error_match = re.search(r"(\d+) error", output)
    if error_match:
        errors = int(error_match.group(1))

    total = passed + failed + errors
    score = (passed / max(total, 1)) * 100

    # Extract critical failures
    critical = []
    for line in output.split("\n"):
        if "FAILED" in line:
            critical.append(line.strip())

    # Collect interesting output lines (print statements from tests)
    details = []
    for line in output.split("\n"):
        stripped = line.strip()
        if stripped.startswith(("  ", "NOTE:", "VERDICT:", "CONFIRMED:", "WARNING:", "CONCLUSION:")):
            details.append(stripped)

    return TierResult(
        name=tier_name,
        score=score,
        passed=passed,
        failed=failed,
        total=total,
        details=details[:20],  # Limit detail lines
        critical_failures=critical[:5],
    )


def main():
    """Run all tiers and generate the final verdict."""
    print("=" * 72)
    print("  FTD ULTIMATE VERIFICATION SUITE")
    print(f"  Date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print("=" * 72)
    print()

    tiers = [
        ("test_tier1_math.py", "T1_math", "Tier 1: Mathematical Foundations"),
        ("test_tier2_chain.py", "T2_chain", "Tier 2: Derivation Chain Integrity"),
        ("test_tier3_predictions.py", "T3_predictions", "Tier 3: Physics Predictions"),
        ("test_tier4_simulation.py", "T4_simulation", "Tier 4: Simulation Engine"),
        ("test_tier5_gaps.py", "T5_gaps", "Tier 5: Critical Gap Attack"),
        ("test_tier6_novel.py", "T6_novel", "Tier 6: Novel Predictions"),
        ("test_tier7_falsification.py", "T7_falsification", "Tier 7: Falsification"),
    ]

    tier_results = {}

    for filename, key, display_name in tiers:
        print(f"\n{'-' * 72}")
        print(f"  Running {display_name}...")
        print(f"{'-' * 72}")

        try:
            result = run_pytest_tier(filename, display_name)
            tier_results[key] = result
            status = "PASS" if result.failed == 0 else "ISSUES"
            print(f"  Result: {result.passed}/{result.total} passed " f"(Score: {result.score:.0f}/100) [{status}]")
            if result.critical_failures:
                for cf in result.critical_failures[:3]:
                    print(f"    !! {cf}")
        except subprocess.TimeoutExpired:
            tier_results[key] = TierResult(
                name=display_name,
                score=0,
                passed=0,
                failed=1,
                total=1,
                critical_failures=["TIMEOUT: Test suite exceeded 5 minutes"],
            )
            print("  Result: TIMEOUT")
        except Exception as e:
            tier_results[key] = TierResult(
                name=display_name, score=0, passed=0, failed=1, total=1, critical_failures=[f"ERROR: {str(e)[:100]}"]
            )
            print(f"  Result: ERROR -- {e}")

    # Generate final verdict
    print()
    report = format_verdict_report(tier_results)
    print(report)

    # Summary of critical findings
    print("\nCRITICAL FINDINGS:")
    print("-" * 72)

    all_critical = []
    for key, result in tier_results.items():
        all_critical.extend(result.critical_failures)

    if all_critical:
        for i, finding in enumerate(all_critical[:10], 1):
            print(f"  {i}. {finding}")
    else:
        print("  No critical failures detected.")

    total_score, verdict = compute_verdict(tier_results)
    print(f"\nFINAL SCORE: {total_score:.1f}/100 -- {verdict}")

    return 0 if verdict in ("STRONG", "PROMISING") else 1


if __name__ == "__main__":
    sys.exit(main())
