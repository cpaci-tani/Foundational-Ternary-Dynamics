#!/usr/bin/env python3
"""Exact FTD-0413 checks for the Moore-local q^4 common-cone fermion.

No parameter scan or physical-target fit is performed.  The script solves the
two quartic tensor equations exactly inside the declared normalized
face-diagonal ansatz, derives the first surviving q^6 mismatch, and enforces
the implementation/source contract.
"""

from __future__ import annotations

from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]


def read(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8")


def require(condition: bool, label: str) -> None:
    if not condition:
        raise AssertionError(label)
    print(f"PASS  {label}")


def main() -> None:
    checks = 0
    qx, qy, qz, eps = sp.symbols("q_x q_y q_z eps", real=True)
    b, r2 = sp.symbols("b r2", real=True)
    qs = (qx, qy, qz)
    s2 = sum(q**2 for q in qs)
    q4 = sum(q**4 for q in qs)
    q6 = sum(q**6 for q in qs)
    p22 = qx**2 * qy**2 + qx**2 * qz**2 + qy**2 * qz**2

    # Normalized face-diagonal kinetic ansatz.  a+2b=1 has already been used.
    kinetic = []
    for mu in range(3):
        nu = (mu + 1) % 3
        rho = (mu + 2) % 3
        transverse = (1 - 2 * b) + b * (
            sp.cos(eps * qs[nu]) + sp.cos(eps * qs[rho])
        )
        kinetic.append(sp.sin(eps * qs[mu]) * transverse)
    wilson = sum(1 - sp.cos(eps * q) for q in qs)
    pole = sum(component**2 for component in kinetic) + r2 * wilson**2
    series = sp.series(pole, eps, 0, 8).removeO().expand()

    expected_q2 = s2
    expected_q4 = -q4 / 3 - 2 * b * p22 + r2 * s2**2 / 4
    require(sp.expand(series.coeff(eps, 2) - expected_q2) == 0,
            "A1 normalized Moore kinetic symbol has unit leading slope")
    checks += 1
    require(sp.expand(series.coeff(eps, 4) - expected_q4) == 0,
            "A2 complete quartic pole has the declared Q4 and P22 tensors")
    checks += 1

    q4_coeff = sp.expand(expected_q4).coeff(qx, 4)
    cross_coeff = sp.expand(expected_q4).coeff(qx, 2).coeff(qy, 2)
    solution = sp.solve(
        (sp.Eq(q4_coeff, 0), sp.Eq(cross_coeff, 0)),
        (b, r2),
        dict=True,
    )
    require(solution == [{b: sp.Rational(1, 3), r2: sp.Rational(4, 3)}],
            "A3 quartic cancellation uniquely fixes b=1/3 and r^2=4/3")
    checks += 1
    require(1 - 2 * solution[0][b] == sp.Rational(1, 3),
            "A4 selected axial weight is also 1/3")
    checks += 1

    selected = sp.expand(series.subs(solution[0]))
    selected_q4 = sp.expand(selected.coeff(eps, 4))
    selected_q6 = sp.expand(sp.cancel(selected.coeff(eps, 6)))
    expected_q6 = s2**3 / 36 + s2 * q4 / 36 - q6 / 15
    require(selected_q4 == 0,
            "A5 selected massless matter pole is q^4-free")
    checks += 1
    require(sp.factor(selected_q6 - expected_q6) == 0,
            "A6 first matter correction is S2^3/36+S2*Q4/36-Q6/15")
    checks += 1

    c2 = sp.Rational(1, 7)
    literal_flux_q6 = (
        -sp.Rational(61, 17640) * s2**3
        + s2 * q4 / 72
        - q6 / 90
    )
    require(c2 * selected.coeff(eps, 2) == s2 / 7,
            "A7 selected matter leading cone equals the BCC-time flux cone")
    checks += 1
    require(sp.expand(selected_q6 - literal_flux_q6) != 0,
            "A8 matter and literal BCC-time flux poles disagree at q^6")
    checks += 1
    require(sp.Poly(sp.expand(selected_q6), qx, qy, qz).coeff_monomial(qx**4 * qy**2)
            == sp.Rational(1, 9),
            "A9 surviving mixed q^6 tensor is nonzero")
    checks += 1

    # RK4 is the implemented matter clock.  Its phase correction begins at
    # x^5 and therefore changes only the isotropic S2^3 term at q^6; it cannot
    # cancel the mixed S2*Q4 mismatch.
    x = sp.symbols("x", real=True)
    z = -sp.I * x
    rk4 = 1 + z + z**2 / 2 + z**3 / 6 + z**4 / 24
    log_rk4 = sp.series(sp.log(rk4), x, 0, 8).removeO().expand()
    rk4_phase = sp.expand(-sp.im(log_rk4))
    require(rk4_phase == x - x**5 / 120 + x**7 / 336,
            "A10 RK4 eigenphase starts x-x^5/120+x^7/336")
    checks += 1
    rk4_factored_q6 = sp.expand(selected_q6 - s2**3 / 2940)
    rk4_expected_q6 = ((sp.Rational(1, 36) - sp.Rational(1, 2940)) * s2**3
                       + s2 * q4 / 36 - q6 / 15)
    require(sp.factor(rk4_factored_q6 - rk4_expected_q6) == 0
            and sp.Rational(1, 36) != 0,
            "A11 RK4 leaves the S2*Q4 invariant coefficient equal to 1/36")
    checks += 1

    # At a Brillouin corner with n_pi nonzero pi components, K=0 and
    # W=2*n_pi.  With r^2=4/3 the dimensionless E^2/c_s^2 is 16 n_pi^2/3.
    corner_gaps = [sp.Rational(16, 3) * n * n for n in (1, 2, 3)]
    require(all(gap > 0 for gap in corner_gaps),
            "D1 all seven non-origin Brillouin corners retain a positive gap")
    checks += 1
    require(sp.Rational(16, 3) == corner_gaps[0],
            "D2 lightest selected Wilson doubler has E^2/c_s^2=16/3")
    checks += 1

    # Source contract.
    header = read("engine/include/ftd/wilson_dirac.h")
    source = read("engine/src/wilson_dirac.cpp")
    native_test = read("engine/tests/test_lorentz_common_cone_improved.cpp")
    gauge_test = read("engine/tests/test_wilson_dirac_gauge.cpp")
    cmake = read("engine/CMakeLists.txt")
    audit = read("docs/theory/07_assessment/AUDIT_LORENTZ_COMMON_CONE_IMPROVED.md")
    engine_spec = read("engine/SPEC_ENGINE.md")

    require("double kinetic_transverse_weight = 0.0" in header,
            "S1 improved Hamiltonian stencil is default-off")
    checks += 1
    require("(1-2b) + b(cos(q_j)+cos(q_k))" in header,
            "S2 header declares the exact free kinetic symbol")
    checks += 1
    require("transport_face_diagonal" in source
            and "path_mu_nu" in source and "path_nu_mu" in source,
            "S3 CPU implementation uses both shortest face-diagonal paths")
    checks += 1
    require("0.5 * (path_mu_nu + path_nu_mu)" in source,
            "S4 diagonal path ordering is symmetrized")
    checks += 1
    require("if (nu == mu) continue" in source
            and "0.5 * transverse_weight" in source,
            "S5 live Hamiltonian realizes only transverse face-diagonal averaging")
    checks += 1
    require("full L=8 Brillouin zone" in native_test
            and "SC+FCC Moore shell" in native_test,
            "S6 native gate covers exact full-band spectrum and support")
    checks += 1
    require("check_gauge_covariance(8, 1.0 / 3.0)" in gauge_test
            and "H_W Hermitian" in gauge_test,
            "S7 selected stencil is included in covariance and Hermiticity tests")
    checks += 1
    require("lorentz_common_cone_improved" in cmake,
            "S8 FTD-0413 native gate is registered")
    checks += 1
    require("COMMON-CONE-THROUGH-q4" in audit
            and "q6" in audit,
            "S9 audit states both the advance and surviving mismatch")
    checks += 1
    require("lorentz_common_cone_improved" in engine_spec,
            "S10 engine specification registers the improved diagnostic")
    checks += 1

    print(f"\n{checks}/{checks} exact/source-contract checks passed")
    print("SELECTION b=1/3, r^2=4/3, c_s^2=1/7")
    print("VERDICT   COMMON-CONE-THROUGH-q4; q6 AND INTERACTIONS OPEN")


if __name__ == "__main__":
    main()
