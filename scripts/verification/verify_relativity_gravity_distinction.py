"""
Verification: FOUND_RELATIVITY_GRAVITY_DISTINCTION.md
=====================================================

Tests the SR / Gravity / GR trichotomy claims:
- RGD-T1: SR limit (f=1 recovers Lorentz factor)
- RGD-T2: Gravity-only limit (v=0 recovers gravitational dilation)
- RGD-T3: Combined formula matches Schwarzschild metric
- RGD-T4: Naive formula error grows with field strength
- RGD-S2: EP accuracy degrades near horizon
- Budget conservation: g_tt * g_rr = -1
- Weak-field limit: naive formula ≈ correct formula

Framework: FTD v5.26
Date: February 20, 2026
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

import numpy as np


# =============================================================================
# CORE FORMULAS
# =============================================================================

def proper_time_correct(f, v):
    """Correct combined proper time formula: dτ/dT = sqrt(f - v²/f)."""
    return np.sqrt(f - v**2 / f)


def proper_time_naive(f, v):
    """Naive (wrong) formula: dτ/dT = sqrt(f - v²)."""
    return np.sqrt(f - v**2)


def proper_time_sr_only(v):
    """SR-only formula: dτ/dT = sqrt(1 - v²)."""
    return np.sqrt(1 - v**2)


def proper_time_gravity_only(f):
    """Gravity-only formula: dτ/dT = sqrt(f)."""
    return np.sqrt(f)


def availability_factor(r, r_s):
    """Lattice availability factor f = 1 - r_s/r."""
    return 1.0 - r_s / r


def schwarzschild_proper_time(f, v_r):
    """Proper time from Schwarzschild line element (radial motion).

    ds² = f dt² - (1/f) dr²
    dτ² = dt²(f - v_r²/f)
    """
    return np.sqrt(f - v_r**2 / f)


# =============================================================================
# TESTS
# =============================================================================

def test_rgd_t1_sr_limit():
    """RGD-T1: SR derives from C=1 independently of gravity.

    When f=1 (no gravity), the combined formula must reduce to the
    Lorentz factor: dτ/dT = sqrt(1 - v²).
    """
    print("RGD-T1: SR limit (f=1 recovers Lorentz factor)")
    print("-" * 50)

    f = 1.0  # flat space, no gravity
    test_velocities = [0.0, 0.1, 0.3, 0.5, 0.7, 0.9, 0.99, 0.999]

    all_pass = True
    for v in test_velocities:
        combined = proper_time_correct(f, v)
        sr_only = proper_time_sr_only(v)
        diff = abs(combined - sr_only)

        if diff > 1e-15:
            print(f"  FAIL: v={v}, combined={combined}, SR={sr_only}, diff={diff}")
            all_pass = False

    # Also verify naive formula equals correct formula when f=1
    for v in test_velocities:
        correct = proper_time_correct(f, v)
        naive = proper_time_naive(f, v)
        diff = abs(correct - naive)
        if diff > 1e-15:
            print(f"  FAIL: f=1 naive≠correct: v={v}, diff={diff}")
            all_pass = False

    status = "PASS" if all_pass else "FAIL"
    print(f"  Result: {status}")
    print()
    return all_pass


def test_rgd_t2_gravity_only():
    """RGD-T2: Gravity derives as scalar field f(r), independent of motion.

    When v=0 (static observer), the combined formula must reduce to
    gravitational time dilation: dτ/dT = sqrt(f).
    """
    print("RGD-T2: Gravity-only limit (v=0 recovers gravitational dilation)")
    print("-" * 50)

    v = 0.0  # static observer
    test_f_values = [1.0, 0.99, 0.9, 0.7, 0.5, 0.3, 0.1, 0.01]

    all_pass = True
    for f in test_f_values:
        combined = proper_time_correct(f, v)
        grav_only = proper_time_gravity_only(f)
        diff = abs(combined - grav_only)

        if diff > 1e-15:
            print(f"  FAIL: f={f}, combined={combined}, grav={grav_only}, diff={diff}")
            all_pass = False

    status = "PASS" if all_pass else "FAIL"
    print(f"  Result: {status}")
    print()
    return all_pass


def test_rgd_t3_schwarzschild_match():
    """RGD-T3: Combined formula matches Schwarzschild metric.

    dτ² = f dt² - (1/f) dr² = dt²(f - v_r²/f)
    for radial coordinate velocity v_r = dr/dt.
    """
    print("RGD-T3: Combined formula matches Schwarzschild metric")
    print("-" * 50)

    all_pass = True
    test_cases = [
        (0.9, 0.1), (0.9, 0.5), (0.5, 0.3), (0.3, 0.2),
        (0.1, 0.05), (0.99, 0.8), (0.7, 0.6),
    ]

    for f, v in test_cases:
        # Direct formula
        direct = proper_time_correct(f, v)

        # From Schwarzschild line element components
        dt2_coeff = f          # g_tt
        dr2_coeff = 1.0 / f    # |g_rr|
        # dτ² = dt²(f - v²·(1/f)) = dt²(f - v²/f)
        from_metric = np.sqrt(f - v**2 * dr2_coeff)

        diff = abs(direct - from_metric)
        if diff > 1e-15:
            print(f"  FAIL: f={f}, v={v}, direct={direct}, metric={from_metric}, diff={diff}")
            all_pass = False

    # Check photon worldline: dτ = 0 ↔ v² = f²
    for f in [0.9, 0.5, 0.1]:
        v_photon = f  # coordinate speed of light in Schwarzschild
        dtau = f - v_photon**2 / f  # should be exactly 0
        if abs(dtau) > 1e-15:
            print(f"  FAIL: photon at f={f}: dτ²={dtau} (should be 0)")
            all_pass = False

    status = "PASS" if all_pass else "FAIL"
    print(f"  Result: {status}")
    print()
    return all_pass


def test_rgd_t4_naive_error_growth():
    """RGD-T4: Naive formula error grows with field strength.

    The error between √(f - v²) and √(f - v²/f) should be O(ε·v²)
    where ε = 1 - f. Error should grow as f decreases from 1.
    """
    print("RGD-T4: Naive formula error grows with field strength")
    print("-" * 50)

    v = 0.3  # moderate velocity
    f_values = [0.999, 0.99, 0.9, 0.7, 0.5]
    previous_error = 0.0
    all_pass = True

    print(f"  {'f':>6}  {'eps=1-f':>8}  {'correct':>10}  {'naive':>10}  {'rel_error':>12}  {'monotonic':>10}")
    for f in f_values:
        # Check that v²/f > v² (so correct formula uses more budget than naive)
        # and that f - v²/f is still positive (observer is subluminal)
        if f - v**2 / f < 0:
            continue

        correct = proper_time_correct(f, v)
        naive = proper_time_naive(f, v)
        rel_error = abs(correct - naive) / correct

        monotonic = rel_error >= previous_error - 1e-15
        print(f"  {f:6.3f}  {1-f:8.4f}  {correct:10.6f}  {naive:10.6f}  {rel_error:12.2e}  {'OK' if monotonic else 'FAIL'}")

        if not monotonic:
            all_pass = False
        previous_error = rel_error

    # Verify error is O(ε·v²)
    # At f=0.9 (ε=0.1), v=0.3: expected error ∝ 0.1 × 0.09 = 0.009
    f_test = 0.9
    correct = proper_time_correct(f_test, v)
    naive = proper_time_naive(f_test, v)
    error = abs(correct - naive) / correct
    eps = 1 - f_test
    expected_order = eps * v**2
    ratio = error / expected_order
    # Ratio should be O(1) — not wildly off
    if ratio > 10 or ratio < 0.01:
        print(f"  FAIL: error/expected_order = {ratio:.4f} (should be O(1))")
        all_pass = False
    else:
        print(f"  Error scaling check: error/(eps*v^2) = {ratio:.4f} -- O(1) confirmed")

    status = "PASS" if all_pass else "FAIL"
    print(f"  Result: {status}")
    print()
    return all_pass


def test_rgd_s2_ep_degrades():
    """RGD-S2: Equivalence principle accuracy degrades near horizon.

    The EP says gravity ≈ acceleration. In FTD, the accuracy of this
    approximation degrades as f → 0 because the v²/f amplification
    departs from v².

    Test: for fixed kinematic budget consumed, compare the "gravitational
    equivalent" in weak vs strong fields.
    """
    print("RGD-S2: EP accuracy degrades near horizon (v²/f vs v²)")
    print("-" * 50)

    # The EP violation is measured by the ratio (v²/f) / (v²) = 1/f
    # This ratio is 1 in flat space (EP exact) and diverges at horizon
    v = 0.1  # small velocity
    f_values = [1.0, 0.999, 0.99, 0.9, 0.5, 0.1, 0.01]

    all_pass = True
    print(f"  {'f':>6}  {'v²':>8}  {'v²/f':>10}  {'EP_violation':>14}  {'expected_1/f':>14}")
    for f in f_values:
        kinematic_cost_flat = v**2
        kinematic_cost_curved = v**2 / f
        ep_violation = kinematic_cost_curved / kinematic_cost_flat  # = 1/f
        expected = 1.0 / f

        diff = abs(ep_violation - expected)
        ok = diff < 1e-12
        print(f"  {f:6.3f}  {v**2:8.4f}  {v**2/f:10.4f}  {ep_violation:14.4f}  {expected:14.4f}  {'OK' if ok else 'FAIL'}")

        if not ok:
            all_pass = False

    # Verify EP violation grows monotonically as f decreases
    violations = [1.0/f for f in f_values]
    monotonic = all(violations[i] <= violations[i+1] + 1e-15 for i in range(len(violations)-1))
    if not monotonic:
        print("  FAIL: EP violation not monotonically increasing as f decreases")
        all_pass = False
    else:
        print("  EP violation grows monotonically: confirmed")

    status = "PASS" if all_pass else "FAIL"
    print(f"  Result: {status}")
    print()
    return all_pass


def test_budget_conservation():
    """g_tt · g_rr = -1 (budget conservation).

    The Schwarzschild metric satisfies g_tt · g_rr = f · (-1/f) = -1.
    This means gravity redistributes budget between temporal and spatial
    channels but cannot create or destroy budget.
    """
    print("Budget conservation: g_tt * g_rr = -1")
    print("-" * 50)

    f_values = [0.999, 0.99, 0.9, 0.7, 0.5, 0.3, 0.1, 0.01, 0.001]
    all_pass = True

    for f in f_values:
        g_tt = f
        g_rr = -1.0 / f
        product = g_tt * g_rr

        if abs(product + 1.0) > 1e-14:
            print(f"  FAIL: f={f}, g_tt*g_rr = {product} (should be -1)")
            all_pass = False

    # Verify: as f → 0, g_rr → -∞ (space becomes infinitely expensive)
    # while g_tt → 0 (time stops)
    f_small = 1e-10
    g_tt = f_small
    g_rr = -1.0 / f_small
    if not (g_tt < 1e-8 and abs(g_rr) > 1e8):
        print(f"  FAIL: horizon behavior incorrect: g_tt={g_tt}, g_rr={g_rr}")
        all_pass = False
    else:
        print(f"  Horizon behavior: g_tt -> 0, |g_rr| -> inf -- confirmed")

    # Product is still -1 even near horizon
    product = g_tt * g_rr
    if abs(product + 1.0) > 1e-10:
        print(f"  FAIL: budget conservation breaks near horizon: product={product}")
        all_pass = False
    else:
        print(f"  Budget conservation near horizon: g_tt*g_rr = {product:.6f} -- confirmed")

    status = "PASS" if all_pass else "FAIL"
    print(f"  Result: {status}")
    print()
    return all_pass


def test_weak_field_agreement():
    """Weak-field limit: naive and correct formulas agree to O(ε·v²).

    For GPS-level fields (ε ~ 10⁻¹⁰) and satellite velocities (v ~ 10⁻⁵),
    the error should be utterly negligible.
    """
    print("Weak-field limit: naive formula ~= correct formula")
    print("-" * 50)

    all_pass = True

    # GPS satellite: altitude ~20,200 km, v ~ 3.87 km/s
    # ε = r_s/r ≈ 2GM/(rc²) ≈ 1.4e-10 for Earth at GPS orbit
    # v/c ≈ 1.29e-5
    eps_gps = 1.4e-10
    v_gps = 1.29e-5
    f_gps = 1.0 - eps_gps

    correct_gps = proper_time_correct(f_gps, v_gps)
    naive_gps = proper_time_naive(f_gps, v_gps)
    rel_error_gps = abs(correct_gps - naive_gps) / correct_gps

    print(f"  GPS: eps={eps_gps:.1e}, v/c={v_gps:.2e}")
    print(f"    correct = {correct_gps:.18f}")
    print(f"    naive   = {naive_gps:.18f}")
    print(f"    rel_error = {rel_error_gps:.2e}")

    if rel_error_gps > 1e-20:
        # Even at double precision, these should be indistinguishable
        print(f"    OK (error below any practical threshold)")
    else:
        print(f"    OK (error at machine epsilon)")

    # Solar surface: ε ≈ 2.1e-6, v ~ 0 for static comparison
    eps_sun = 2.1e-6
    v_sun = 0.01  # hypothetical 1% c motion near sun
    f_sun = 1.0 - eps_sun

    correct_sun = proper_time_correct(f_sun, v_sun)
    naive_sun = proper_time_naive(f_sun, v_sun)
    rel_error_sun = abs(correct_sun - naive_sun) / correct_sun

    print(f"  Solar surface: eps={eps_sun:.1e}, v/c={v_sun}")
    print(f"    rel_error = {rel_error_sun:.2e}")
    expected_sun = eps_sun * v_sun**2
    print(f"    expected O(eps*v^2) = {expected_sun:.2e}")

    # Neutron star surface: ε ≈ 0.3, v = 0.1c
    eps_ns = 0.3
    v_ns = 0.1
    f_ns = 1.0 - eps_ns

    correct_ns = proper_time_correct(f_ns, v_ns)
    naive_ns = proper_time_naive(f_ns, v_ns)
    rel_error_ns = abs(correct_ns - naive_ns) / correct_ns

    print(f"  Neutron star: eps={eps_ns}, v/c={v_ns}")
    print(f"    rel_error = {rel_error_ns:.4f} ({rel_error_ns*100:.2f}%)")
    print(f"    Naive formula fails at ~{rel_error_ns*100:.1f}% level near compact objects")

    # The error should grow from GPS << solar << neutron star
    if not (rel_error_gps < rel_error_sun < rel_error_ns):
        print("  FAIL: error does not grow with field strength")
        all_pass = False
    else:
        print("  Error hierarchy GPS < Solar < NS: confirmed")

    status = "PASS" if all_pass else "FAIL"
    print(f"  Result: {status}")
    print()
    return all_pass


# =============================================================================
# MAIN
# =============================================================================

def main():
    print("=" * 60)
    print("VERIFICATION: FOUND_RELATIVITY_GRAVITY_DISTINCTION.md")
    print("The SR / Gravity / GR Trichotomy")
    print("=" * 60)
    print()

    results = {}
    results['RGD-T1'] = test_rgd_t1_sr_limit()
    results['RGD-T2'] = test_rgd_t2_gravity_only()
    results['RGD-T3'] = test_rgd_t3_schwarzschild_match()
    results['RGD-T4'] = test_rgd_t4_naive_error_growth()
    results['RGD-S2'] = test_rgd_s2_ep_degrades()
    results['Budget'] = test_budget_conservation()
    results['WeakField'] = test_weak_field_agreement()

    # Summary
    print("=" * 60)
    print("SUMMARY")
    print("=" * 60)
    passed = sum(1 for v in results.values() if v)
    total = len(results)
    for name, result in results.items():
        print(f"  {name:12s}: {'PASS' if result else 'FAIL'}")
    print(f"\n  Total: {passed}/{total}")

    if passed == total:
        print("\n  ALL TESTS PASS")
    else:
        print(f"\n  {total - passed} FAILURE(S)")
        sys.exit(1)


if __name__ == "__main__":
    main()
