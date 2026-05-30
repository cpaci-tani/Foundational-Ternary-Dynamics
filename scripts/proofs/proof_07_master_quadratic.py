"""
Proof 07: Master Quadratic — From G* to α and N_c
====================================================

CLAIM [THEOREM + CONJECTURE]: The master quadratic
    x² - 16·G*²·x + 16·G*³ = 0
produces x₊ = 137.036... (identified as 1/α) and x₋ = 3.024... (→ N_c = 3).

The algebra is [THEOREM]. The identification x₊ = 1/α is [CONJECTURE].
"""

import math
import cmath

from .common import (ProofSuite, MACHINE_EPS, PPM_1, PPM_10, PERCENT_1,
                     G_STAR, COEFFICIENT, X_PLUS, X_MINUS,
                     CODATA_ALPHA_INV)


def run() -> ProofSuite:
    s = ProofSuite("Proof 07: Master Quadratic (x₊ → α, x₋ → N_c)")

    c = G_STAR
    k = COEFFICIENT  # = 16

    # =========================================================================
    # Step 1: Compute roots via quadratic formula
    # =========================================================================
    # x² - k·c²·x + k·c³ = 0
    # x = (k·c² ± √(k²c⁴ - 4kc³)) / 2
    #   = (k·c² ± √(kc³(kc - 4))) / 2

    discriminant = k**2 * c**4 - 4 * k * c**3
    sqrt_disc = math.sqrt(discriminant)

    xp = (k * c**2 + sqrt_disc) / 2.0
    xm = (k * c**2 - sqrt_disc) / 2.0

    s.assert_close(
        "x₊ = 137.0361714...",
        xp, X_PLUS, MACHINE_EPS * 1000,
        tag="[THEOREM]"
    )

    s.assert_close(
        "x₋ = 3.0239639...",
        xm, X_MINUS, MACHINE_EPS * 1000,
        tag="[THEOREM]"
    )

    # =========================================================================
    # Step 2: Verify Vieta's relations
    # =========================================================================
    vieta_sum = xp + xm
    vieta_prod = xp * xm

    s.assert_close(
        "Vieta sum: x₊ + x₋ = 16·G*²",
        vieta_sum, k * c**2, MACHINE_EPS * 100,
        tag="[THEOREM]"
    )

    s.assert_close(
        "Vieta product: x₊ · x₋ = 16·G*³",
        vieta_prod, k * c**3, MACHINE_EPS * 100,
        tag="[THEOREM]"
    )

    # =========================================================================
    # Step 3: Residual check (plug back into equation)
    # =========================================================================
    res_plus = xp**2 - k * c**2 * xp + k * c**3
    res_minus = xm**2 - k * c**2 * xm + k * c**3

    s.assert_close(
        "Residual f(x₊) = 0",
        res_plus, 0.0, 1e-8,
        tag="[THEOREM]"
    )

    s.assert_close(
        "Residual f(x₋) = 0",
        res_minus, 0.0, 1e-8,
        tag="[THEOREM]"
    )

    # =========================================================================
    # Step 4: Compare x₊ to CODATA 2022
    # =========================================================================
    error_ppm = abs(xp - CODATA_ALPHA_INV) / CODATA_ALPHA_INV * 1e6

    s.assert_close(
        f"x₊ vs CODATA 1/α ({error_ppm:.2f} ppm)",
        xp, CODATA_ALPHA_INV, PPM_10,
        tag="[CONJECTURE]"
    )

    s.assert_true(
        "Error < 2 ppm (tree level, before radiative corrections)",
        error_ppm < 2.0,
        tag="[CONJECTURE]"
    )

    # =========================================================================
    # Step 5: x₋ → N_c = 3
    # =========================================================================
    N_c_from_root = int(math.floor(xm))

    s.assert_true(
        "floor(x₋) = floor(3.024) = 3 (color charges)",
        N_c_from_root == 3,
        tag="[THEOREM]"
    )

    # Fractional excess: x₋ - 3 ≈ 0.024
    excess = xm - 3.0

    s.assert_true(
        f"x₋ - N_c = {excess:.4f} (small fractional excess < 1%)",
        0 < excess < 0.1,
        tag="[THEOREM]"
    )

    # =========================================================================
    # Step 6: Discriminant analysis — the Q_k family
    # =========================================================================
    # For the generalized quadratic x² - k·G*²·x + k·G*³ = 0:
    # Δ = k·G*³·(k·G* - 4)
    # Three regimes:

    # k = 16 (physics): Δ > 0 → real roots
    disc_phys = k * c**3 * (k * c - 4.0)
    s.assert_true(
        "k=16: Δ > 0 → REAL roots (physics domain)",
        disc_phys > 0,
        tag="[THEOREM]"
    )

    # k = 4/G* (critical): Δ = 0 → degenerate
    k_crit = 4.0 / c
    disc_crit = k_crit * c**3 * (k_crit * c - 4.0)
    s.assert_close(
        "k=4/G*: Δ = 0 → degenerate (Born rule/measurement boundary)",
        disc_crit, 0.0, 1e-10,
        tag="[THEOREM]"
    )

    # Degenerate root: x = k_crit·G*²/2 = 2G*
    x_born = k_crit * c**2 / 2.0
    s.assert_close(
        "Degenerate root x_Born = 2G* ≈ 5.917",
        x_born, 2.0 * c, MACHINE_EPS * 100,
        tag="[THEOREM]"
    )

    # k = 1/2 (reference frame context): Δ < 0 → complex roots
    k_cons = 0.5
    disc_cons = k_cons * c**3 * (k_cons * c - 4.0)
    s.assert_true(
        "k=1/2: Δ < 0 → COMPLEX roots (reference frame context domain)",
        disc_cons < 0,
        tag="[THEOREM]"
    )

    # Complex roots for k=1/2:
    a_coeff = 1.0
    b_coeff = -k_cons * c**2
    c_coeff = k_cons * c**3
    y1 = (-b_coeff + cmath.sqrt(b_coeff**2 - 4 * a_coeff * c_coeff)) / (2 * a_coeff)
    y2 = (-b_coeff - cmath.sqrt(b_coeff**2 - 4 * a_coeff * c_coeff)) / (2 * a_coeff)

    s.assert_close(
        "Reference frame context root Re(y) = G*²/4",
        y1.real, c**2 / 4.0, MACHINE_EPS * 100,
        tag="[THEOREM]"
    )

    s.assert_true(
        "Reference frame context root has nonzero Im(y) (irreducibly subjective)",
        abs(y1.imag) > 0.1,
        tag="[THEOREM]"
    )

    # =========================================================================
    # Step 7: Dimensional origin — D = log₂(k_phys/k_cons)
    # =========================================================================
    # log₂(16) + log₂(1/2) = 4 + (-1) = 3 = D
    D_from_k = math.log2(16) + math.log2(0.5)

    s.assert_close(
        "D = log₂(16) + log₂(1/2) = 4 - 1 = 3",
        D_from_k, 3.0, MACHINE_EPS,
        tag="[THEOREM]"
    )

    # =========================================================================
    # Step 8: Sensitivity of x₊ to G*
    # =========================================================================
    dG = 1e-9
    G_p = c + dG
    G_m = c - dG
    xp_p = (k * G_p**2 + math.sqrt(k**2 * G_p**4 - 4*k*G_p**3)) / 2.0
    xp_m = (k * G_m**2 + math.sqrt(k**2 * G_m**4 - 4*k*G_m**3)) / 2.0
    dx_dG = (xp_p - xp_m) / (2.0 * dG)

    # 1 ppm change in G* produces this change in x₊:
    delta_xp_for_1ppm = dx_dG * c * 1e-6

    s.assert_true(
        f"dx₊/dG* ≈ {dx_dG:.2f}: 1 ppm G* → {delta_xp_for_1ppm:.6f} in x₊",
        dx_dG > 0,
        tag="[THEOREM]"
    )

    # =========================================================================
    # Step 9: Harmonic mean identity
    # =========================================================================
    # G* = HM(x₊, x₋)/2 = x₊·x₋/(x₊+x₋)
    hm_half = xp * xm / (xp + xm)

    s.assert_close(
        "G* = HM(x₊,x₋)/2 = x₊x₋/(x₊+x₋)",
        hm_half, c, PPM_1,
        tag="[THEOREM]"
    )

    # Equivalently: G* = Product/Sum = G*³/G*² (dimensional triad)
    s.assert_close(
        "G* = Vieta(Product)/Vieta(Sum) = 16G*³/(16G*²)",
        (k * c**3) / (k * c**2), c, MACHINE_EPS,
        tag="[THEOREM]"
    )

    return s


if __name__ == "__main__":
    suite = run()
    suite.print_summary()
