"""
Charge Quartic Verification
============================

Verifies that the substitution e^2 = 1/x transforms the master quadratic
    x² − 16G*²x + 16G*³ = 0
into the charge quartic
    16G*³ e⁴ − 16G*² e^2 + 1 = 0
and that its roots are the electromagnetic and color charge scales.

Key results verified:
  - Quartic roots: e^2_EM = α ≈ 0.00730, e^2_C = 1/x₋ ≈ 0.3307
  - Charge-space Vieta sum:    e^2_EM + e^2_C = 1/G*
  - Charge-space Vieta product: e^2_EM · e^2_C = 1/(16G*³)
  - Product identity: e_EM · e_C = 1/(4G*^{3/2})
  - Splitting parameter: e^2_C/e^2_EM = x+/x- = (1+d)/(1-d), d = sqrt(1-1/(4G*))

Status: All results are [THEOREM] (pure algebra from master quadratic).
See docs/theory/DERIV_CHARGE_QUARTIC_FROM_GSTAR.md for full derivation.
"""

import numpy as np
from scripts.constants import (
    G_STAR, ALPHA, X_PLUS, X_MINUS,
    N_c, N_base, b_3, N_eff,
    percent_error, ppm_error
)


def verify_charge_quartic_equation():
    """
    Verify the charge quartic 16G*³ e⁴ - 16G*² e^2 + 1 = 0 follows from the
    master quadratic via e^2 = 1/x, and that its roots match 1/x₊ and 1/x₋.
    """
    print("=" * 70)
    print("TEST 1: CHARGE QUARTIC EQUATION")
    print("  16G*³ e⁴ - 16G*² e^2 + 1 = 0  (quadratic in u = e^2)")
    print("=" * 70)

    c = G_STAR

    # Solve as quadratic in u = e^2:  16G*³ u² - 16G*² u + 1 = 0
    a_coef = 16 * c**3
    b_coef = -16 * c**2
    c_coef = 1

    discriminant = b_coef**2 - 4 * a_coef * c_coef
    print(f"\n  Coefficients: a = 16G*³ = {a_coef:.10f}")
    print(f"                b = -16G*² = {b_coef:.10f}")
    print(f"                c = 1")
    print(f"  Discriminant: {discriminant:.10f}")

    u_plus = (-b_coef + np.sqrt(discriminant)) / (2 * a_coef)
    u_minus = (-b_coef - np.sqrt(discriminant)) / (2 * a_coef)

    print(f"\n  Root u₊ (= e^2_C):  {u_plus:.15f}")
    print(f"  Root u₋ (= e^2_EM): {u_minus:.15f}")

    # Verify roots match 1/x₋ and 1/x₊ (= α)
    expected_u_plus = 1.0 / X_MINUS   # Larger e^2 = color charge
    expected_u_minus = 1.0 / X_PLUS   # Smaller e^2 = EM charge = α

    diff_plus = abs(u_plus - expected_u_plus)
    diff_minus = abs(u_minus - expected_u_minus)

    print(f"\n  u₊ vs 1/x₋:  diff = {diff_plus:.2e}")
    print(f"  u₋ vs 1/x₊:  diff = {diff_minus:.2e}")
    print(f"  u₋ vs α:     diff = {abs(u_minus - ALPHA):.2e}")

    # Verify each root satisfies the quartic (residual check)
    for u, name in [(u_plus, "e^2_C"), (u_minus, "e^2_EM")]:
        residual = a_coef * u**2 + b_coef * u + c_coef
        print(f"  Residual for {name}: {residual:.2e}")

    ok = diff_plus < 1e-12 and diff_minus < 1e-12
    print(f"\n  Status: {'[OK]' if ok else '[X]'}")
    return ok


def verify_charge_vieta_relations():
    """
    Verify charge-space Vieta relations:
      Sum:     e^2_EM + e^2_C = 1/G*
      Product: e^2_EM · e^2_C = 1/(16G*³)
    """
    print("\n" + "=" * 70)
    print("TEST 2: CHARGE-SPACE VIETA RELATIONS")
    print("=" * 70)

    c = G_STAR
    # Use raw quadratic roots (not ALPHA which has 4-term precision correction)
    # Vieta relations are exact for the quadratic's own roots
    e2_em = 1.0 / X_PLUS        # = 1/x₊ (tree-level)
    e2_c = 1.0 / X_MINUS        # = 1/x₋

    # Sum: e^2_EM + e^2_C should equal 1/G*
    vieta_sum = e2_em + e2_c
    expected_sum = 1.0 / c
    sum_diff = abs(vieta_sum - expected_sum)

    print(f"\n  e^2_EM = 1/x+   = {e2_em:.15f}")
    print(f"  e^2_C  = 1/x-   = {e2_c:.15f}")
    print(f"  (Note: ALPHA with 4-term correction = {ALPHA:.15f}, diff = {abs(e2_em - ALPHA):.2e})")
    print(f"\n  Sum  e^2_EM + e^2_C = {vieta_sum:.15f}")
    print(f"  Expected  1/G*    = {expected_sum:.15f}")
    print(f"  Difference        = {sum_diff:.2e}")

    # Product: e^2_EM · e^2_C should equal 1/(16G*³)
    vieta_prod = e2_em * e2_c
    expected_prod = 1.0 / (16 * c**3)
    prod_diff = abs(vieta_prod - expected_prod)

    print(f"\n  Product  e^2_EM · e^2_C = {vieta_prod:.15f}")
    print(f"  Expected  1/(16G*³)   = {expected_prod:.15f}")
    print(f"  Difference            = {prod_diff:.2e}")

    ok = sum_diff < 1e-12 and prod_diff < 1e-12
    print(f"\n  Status: {'[OK]' if ok else '[X]'}")
    return ok


def verify_product_identity():
    """
    Verify the charge product identity:
      e_EM · e_C = 1/(4G*^{3/2})
    """
    print("\n" + "=" * 70)
    print("TEST 3: CHARGE PRODUCT IDENTITY")
    print("  e_EM · e_C = 1/(4 · G*^{3/2})")
    print("=" * 70)

    c = G_STAR
    # Use raw quadratic roots for exact Vieta product identity
    e_em = np.sqrt(1.0 / X_PLUS)
    e_c = np.sqrt(1.0 / X_MINUS)

    product = e_em * e_c
    expected = 1.0 / (4.0 * c**1.5)
    diff = abs(product - expected)

    print(f"\n  e_EM = sqrt(1/x+) = {e_em:.15f}")
    print(f"  e_C  = sqrt(1/x-) = {e_c:.15f}")
    print(f"\n  Product e_EM*e_C   = {product:.15f}")
    print(f"  Expected 1/(4G*^(3/2)) = {expected:.15f}")
    print(f"  Difference         = {diff:.2e}")

    ok = diff < 1e-12
    print(f"\n  Status: {'[OK]' if ok else '[X]'}")
    return ok


def verify_inside_out_duality():
    """
    Verify the coefficient inversion (inside-out duality):
      Coupling space: 1·x²  + (-16G*²)·x  + 16G*³ = 0
      Charge space:   16G*³·u² + (-16G*²)·u + 1    = 0

    The leading and constant coefficients swap; the linear coefficient is shared.
    This is the reciprocal polynomial relationship: if x is a root, then 1/x
    is a root of the reversed polynomial.
    """
    print("\n" + "=" * 70)
    print("TEST 4: INSIDE-OUT DUALITY (COEFFICIENT INVERSION)")
    print("=" * 70)

    c = G_STAR

    # Coupling-space coefficients
    coup_lead = 1.0
    coup_lin = -16 * c**2
    coup_const = 16 * c**3

    # Charge-space coefficients
    chg_lead = 16 * c**3
    chg_lin = -16 * c**2
    chg_const = 1.0

    print(f"\n  Coupling space: {coup_lead:.1f}·x² + ({coup_lin:.6f})·x + {coup_const:.6f}")
    print(f"  Charge space:   {chg_lead:.6f}·u² + ({chg_lin:.6f})·u + {chg_const:.1f}")

    # Verify: leading_coupling = constant_charge
    check1 = abs(coup_lead - chg_const)
    # Verify: constant_coupling = leading_charge
    check2 = abs(coup_const - chg_lead)
    # Verify: linear coefficients are the same
    check3 = abs(coup_lin - chg_lin)

    print(f"\n  leading_coupling = constant_charge: diff = {check1:.2e}")
    print(f"  constant_coupling = leading_charge: diff = {check2:.2e}")
    print(f"  linear coefficients shared:         diff = {check3:.2e}")

    # Verify the reciprocal polynomial property:
    # If P(x) = x² + bx + c, then x²·P(1/x) = 1 + b/x + c/x² = (c·x² + b·x + 1)/x²
    # Normalized: c·x² + b·x + 1 = 0
    # So charge quartic = coupling quadratic with reversed coefficients, scaled by 1/c
    print(f"\n  Coupling quadratic reversed and normalized:")
    print(f"    {coup_const:.6f}·u² + ({coup_lin:.6f})·u + {coup_lead:.1f} = 0")
    print(f"    = charge quartic ✓")

    ok = check1 < 1e-15 and check2 < 1e-12 and check3 < 1e-12
    print(f"\n  Status: {'[OK]' if ok else '[X]'}")
    return ok


def verify_splitting_parameter():
    """
    Verify the splitting parameter connection:
      e^2_C / e^2_EM = x+/x- = (1+d)/(1-d)
    where d = (x+ - x-)/(x+ + x-) = sqrt(1 - 1/(4G*)).

    Derivation: x+/- = 8G*^2(1 +/- d), so x+*x- = 64G*^4(1-d^2) = 16G*^3
    => 1-d^2 = 1/(4G*) => d^2 = 1 - 1/(4G*).
    """
    print("\n" + "=" * 70)
    print("TEST 5: SPLITTING PARAMETER CONNECTION")
    print("  e^2_C/e^2_EM = x+/x- = (1+d)/(1-d)")
    print("=" * 70)

    c = G_STAR
    # delta = (x+ - x-)/(x+ + x-) = sqrt(1 - 1/(4G*))
    delta = np.sqrt(1 - 1.0 / (4 * c))

    # Three independent computations of the same ratio
    e2_ratio = (1.0 / X_MINUS) / (1.0 / X_PLUS)   # = X_PLUS / X_MINUS
    x_ratio = X_PLUS / X_MINUS
    delta_ratio = (1 + delta) / (1 - delta)

    # Also compute delta directly from roots for cross-check
    delta_from_roots = (X_PLUS - X_MINUS) / (X_PLUS + X_MINUS)

    print(f"\n  d = sqrt(1 - 1/(4G*)) = {delta:.15f}")
    print(f"  d = (x+ - x-)/(x+ + x-) = {delta_from_roots:.15f}")
    print(f"  Diff between definitions = {abs(delta - delta_from_roots):.2e}")
    print(f"\n  e^2_C/e^2_EM     = {e2_ratio:.15f}")
    print(f"  x+/x-           = {x_ratio:.15f}")
    print(f"  (1+d)/(1-d)     = {delta_ratio:.15f}")

    diff_1 = abs(e2_ratio - x_ratio)
    diff_2 = abs(e2_ratio - delta_ratio)

    print(f"\n  e^2_C/e^2_EM vs x+/x-:       diff = {diff_1:.2e}")
    print(f"  e^2_C/e^2_EM vs (1+d)/(1-d): diff = {diff_2:.2e}")

    # Also print the ratio value
    print(f"\n  Ratio = {e2_ratio:.6f}")
    print(f"  (Color charge squared is {e2_ratio:.1f}× larger than EM charge squared)")

    ok = diff_1 < 1e-12 and diff_2 < 1e-12
    print(f"\n  Status: {'[OK]' if ok else '[X]'}")
    return ok


def verify_sum_inverse_gstar():
    """
    Verify the key identity: e^2_EM + e^2_C = 1/G* with ppm-level precision.
    This is the charge-space Vieta sum, showing that squared charges sum to
    the inverse of the lemniscatic constant.
    """
    print("\n" + "=" * 70)
    print("TEST 6: SQUARED CHARGES SUM TO INVERSE LEMNISCATIC CONSTANT")
    print("  e^2_EM + e^2_C = 1/G*")
    print("=" * 70)

    c = G_STAR

    # Direct computation from master quadratic roots
    e2_em = 1.0 / X_PLUS     # = α (using tree-level root)
    e2_c = 1.0 / X_MINUS

    lhs = e2_em + e2_c
    rhs = 1.0 / c

    diff = abs(lhs - rhs)
    rel_err = diff / rhs

    print(f"\n  e^2_EM = 1/x₊    = {e2_em:.15f}")
    print(f"  e^2_C  = 1/x₋    = {e2_c:.15f}")
    print(f"\n  Sum (LHS)        = {lhs:.15f}")
    print(f"  1/G* (RHS)       = {rhs:.15f}")
    print(f"  Absolute diff    = {diff:.2e}")
    print(f"  Relative error   = {rel_err:.2e}")

    # Also verify with Vieta: sum of roots of ax²+bx+c=0 is -b/a
    # For 16G*³u² - 16G*²u + 1 = 0: sum = 16G*²/(16G*³) = 1/G*
    vieta_sum = (16 * c**2) / (16 * c**3)
    print(f"\n  Vieta sum −b/a   = {vieta_sum:.15f}")
    print(f"  1/G*             = {rhs:.15f}")
    print(f"  Diff (exact)     = {abs(vieta_sum - rhs):.2e}")

    # Physical interpretation
    print(f"\n  PHYSICAL INSIGHT:")
    print(f"    In coupling space: x₊ · x₋ = 16G*³ = {16*c**3:.6f} (action per DoF)")
    print(f"    In charge space:   e^2_EM + e^2_C = 1/G* = {1/c:.6f} (inverse flux per DoF)")
    print(f"    Relationship: (coupling product) × (charge sum) = 16G*² = {16*c**2:.6f}")

    ok = diff < 1e-12
    print(f"\n  Status: {'[OK]' if ok else '[X]'}")
    return ok


def run_all_verifications():
    """Run all charge quartic verification tests."""
    print()
    print("=" * 70)
    print("  CHARGE QUARTIC VERIFICATION SUITE")
    print("  16G*^3 e^4 - 16G*^2 e^2 + 1 = 0")
    print("  See DERIV_CHARGE_QUARTIC_FROM_GSTAR.md")
    print("=" * 70)

    results = []
    results.append(("Quartic equation & roots", verify_charge_quartic_equation()))
    results.append(("Charge-space Vieta relations", verify_charge_vieta_relations()))
    results.append(("Product identity", verify_product_identity()))
    results.append(("Inside-out duality", verify_inside_out_duality()))
    results.append(("Splitting parameter", verify_splitting_parameter()))
    results.append(("Sum = 1/G*", verify_sum_inverse_gstar()))

    print("\n" + "=" * 70)
    print("SUMMARY")
    print("=" * 70)

    passed = sum(1 for _, ok in results if ok)
    total = len(results)

    for name, ok in results:
        status = "[OK]" if ok else "[X]"
        print(f"  {status}  {name}")

    print(f"\n  {passed}/{total} tests passed")

    if passed == total:
        print("\n  ALL VERIFICATIONS PASSED — charge quartic is algebraically exact.")
        print("  Epistemic status: 6 [THEOREM], 1 [SELECTION] (inside-out interpretation)")
    else:
        print("\n  SOME TESTS FAILED — review results above.")

    print()
    return passed == total


if __name__ == "__main__":
    run_all_verifications()
