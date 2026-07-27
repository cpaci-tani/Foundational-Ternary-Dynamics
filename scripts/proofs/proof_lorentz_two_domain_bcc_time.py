#!/usr/bin/env python3
"""Exact checks for the FTD-0411 SC+FCC-space/BCC-time construction.

This script performs symbolic series reversion, transfer-matrix algebra, exact
band endpoint checks, and source-contract checks.  It performs no numerical
near-miss search and fits no physical target.
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


def transfer(kappa: sp.Expr, m: sp.Symbol) -> sp.Matrix:
    """One scalar centered-recurrence transfer with kick kappa*M."""
    return sp.Matrix([[2 - kappa * m, -1], [1, 0]])


def main() -> None:
    checks = 0
    theta, u, m, c2 = sp.symbols("theta u m c2", real=True)
    s2, q4, q6, eps = sp.symbols("S2 Q4 Q6 eps", real=True)

    # ------------------------------------------------------------------
    # Selected BCC temporal character and its canonical normalization.
    # ------------------------------------------------------------------
    temporal = sp.Rational(2, 3) * (1 - sp.cos(theta) ** 3)
    temporal_series = sp.series(temporal, theta, 0, 10).removeO().expand()
    expected_temporal = (
        theta**2 - sp.Rational(7, 12) * theta**4
        + sp.Rational(61, 360) * theta**6
        - sp.Rational(547, 20160) * theta**8
    )
    require(sp.expand(temporal_series - expected_temporal) == 0,
            "T1 normalized BCC time series is exact through theta^8")
    checks += 1

    raw_leading = sp.limit((1 - sp.cos(theta) ** 3) / theta**2, theta, 0)
    require(raw_leading == sp.Rational(3, 2),
            "T2 canonical theta^2 normalization uniquely supplies factor 2/3")
    checks += 1

    harmonic = sp.Rational(1, 2) * (1 - sp.cos(theta)) \
        + sp.Rational(1, 6) * (1 - sp.cos(3 * theta))
    require(sp.trigsimp(temporal - harmonic) == 0,
            "T3 BCC time is exactly a fundamental-plus-third-harmonic kernel")
    checks += 1

    # Series reversion in u=theta^2.
    temporal_u = u - sp.Rational(7, 12) * u**2 \
        + sp.Rational(61, 360) * u**3
    x = sp.symbols("x", real=True)
    inverse_u = x + sp.Rational(7, 12) * x**2 \
        + sp.Rational(23, 45) * x**3
    require(sp.series(temporal_u.subs(u, inverse_u), x, 0, 4).removeO()
            .expand() == x,
            "T4 BCC temporal series reversion is x+7x^2/12+23x^3/45")
    checks += 1

    # ------------------------------------------------------------------
    # Production SC+FCC spatial symbol and the cone derivation.
    # ------------------------------------------------------------------
    m_ir = s2 - s2**2 / 12 + s2 * q4 / 72 - q6 / 90
    u_general = inverse_u.subs(x, c2 * m_ir)
    scaled = sp.series(
        u_general.subs({s2: eps**2 * s2,
                        q4: eps**4 * q4,
                        q6: eps**6 * q6}),
        eps, 0, 8,
    ).removeO().expand().subs(eps, 1)
    q4_coefficient = sp.factor(scaled.coeff(s2, 2))
    require(q4_coefficient == c2 * (7 * c2 - 1) / 12,
            "C1 complete q^4 coefficient is c^2(7c^2-1)/12")
    checks += 1

    positive_root = [root for root in sp.solve(7 * c2 - 1, c2)
                     if root.is_positive][0]
    require(positive_root == sp.Rational(1, 7),
            "C2 nonzero q^4-free cone is uniquely c^2=1/7")
    checks += 1

    literal_pole = sp.expand(scaled.subs(c2, sp.Rational(1, 7)))
    expected_literal = (
        s2 / 7
        - sp.Rational(61, 123480) * s2**3
        + s2 * q4 / 504
        - q6 / 630
    )
    require(literal_pole == expected_literal,
            "C3 literal BCC-time pole has the corrected complete q^6 tensor")
    checks += 1

    # Exact spatial symbols distinguish physical M18 from the BCC return
    # character even though both arise in one Moore neighborhood.
    cx, cy, cz = sp.symbols("c_x c_y c_z", real=True)
    m18 = 4 - sp.Rational(2, 3) * (cx + cy + cz) \
        - sp.Rational(2, 3) * (cx * cy + cx * cz + cy * cz)
    bcc = 1 - cx * cy * cz
    require(m18.subs({cx: -1, cy: -1, cz: 1}) == sp.Rational(16, 3)
            and bcc.subs({cx: -1, cy: -1, cz: 1}) == 0,
            "C4 SC+FCC band maximum is exactly a BCC-character zero")
    checks += 1

    # ------------------------------------------------------------------
    # Principal branch and scalar ghost obstruction.
    # ------------------------------------------------------------------
    mmax = sp.Rational(16, 3)
    return_character = 1 - sp.Rational(3, 14) * m
    require(return_character.subs(m, 0) == 1
            and return_character.subs(m, mmax) == -sp.Rational(1, 7),
            "G1 principal BCC cube-root branch is real on the complete band")
    checks += 1

    y, r = sp.symbols("y R", real=True)
    discriminant = sp.discriminant(y**3 - r, y)
    require(discriminant == -27 * r**2,
            "G2 literal scalar BCC clock has one real and two complex y roots")
    checks += 1

    z = sp.symbols("z", complex=True, nonzero=True)
    unit_circle_y = sp.simplify(
        ((sp.exp(sp.I * theta)) + (sp.exp(sp.I * theta)) ** -1) / 2
    )
    require(sp.simplify(unit_circle_y - sp.cos(theta)) == 0,
            "G3 unit-circle transfer roots can represent only real y=cos(theta)")
    checks += 1

    # The obstruction survives every finite number of positive-norm linear
    # auxiliaries rational in the production symbol. Over Q(M), y^3-R(M)
    # is irreducible because R has a simple zero and is therefore not a cube.
    # If a finite rational unitary transfer U had the desired eigenphase, the
    # Hermitian H=(U+U^dagger)/2 would have y as an eigenvalue.  Its
    # characteristic polynomial would then contain the irreducible cubic,
    # including its two non-real roots, contradicting Hermiticity.
    rational_field = sp.QQ.frac_field(m)
    minimal_y = sp.Poly(y**3 - return_character, y, domain=rational_field)
    require(minimal_y.is_irreducible,
            "G4 y^3-(1-3M/14) is irreducible over the rational M18 field")
    checks += 1

    branch_point = sp.Rational(14, 3)
    require(return_character.subs(m, branch_point) == 0
            and sp.diff(return_character, m).subs(m, branch_point)
            == -sp.Rational(3, 14),
            "G5 the BCC return character has a simple zero and cannot be a rational cube")
    checks += 1

    # ------------------------------------------------------------------
    # Stable period-two local surrogate.
    # ------------------------------------------------------------------
    sqrt2 = sp.sqrt(2)
    k0 = (1 + sqrt2) / 7
    k1 = (1 - sqrt2) / 7
    require(sp.simplify(k0 + k1) == sp.Rational(2, 7),
            "L1 selected surrogate kick sum is 2/7")
    checks += 1
    require(sp.simplify(k0 * k1) == -sp.Rational(1, 49),
            "L2 selected surrogate kick product is -1/49")
    checks += 1

    monodromy2 = transfer(k1, m) * transfer(k0, m)
    half_trace2 = sp.factor(sp.trace(monodromy2) / 2)
    expected_half_trace2 = 1 - sp.Rational(2, 7) * m \
        - sp.Rational(1, 98) * m**2
    require(sp.simplify(half_trace2 - expected_half_trace2) == 0,
            "L3 exact period-two half-trace matches the selected kicks")
    checks += 1

    floquet_x = sp.factor((1 - half_trace2) / 2)
    require(sp.simplify(floquet_x - (m / 7 + m**2 / 196)) == 0,
            "L4 exact Floquet pole is sin^2(theta)=M/7+M^2/196")
    checks += 1
    require(floquet_x.subs(m, mmax) == sp.Rational(400, 441),
            "L5 full-band endpoint is exactly 400/441<1")
    checks += 1
    require(sp.diff(floquet_x, m) == m / 98 + sp.Rational(1, 7)
            and sp.diff(floquet_x, m).subs(m, 0) > 0,
            "L6 Floquet X is strictly increasing for M>=0")
    checks += 1

    # arcsin(sqrt(X))^2, expanded algebraically without numerical fitting.
    surrogate_u_m = m / 7 + m**2 / 84 + sp.Rational(31, 30870) * m**3
    # sin^2(sqrt(u)) = u-u^2/3+2u^3/45+... must return X.
    reconstructed_x = surrogate_u_m - surrogate_u_m**2 / 3 \
        + 2 * surrogate_u_m**3 / 45
    require(sp.series(reconstructed_x, m, 0, 4).removeO().expand()
            == sp.expand(m / 7 + m**2 / 196),
            "L7 surrogate phase series is M/7+M^2/84+31M^3/30870")
    checks += 1

    surrogate_spatial = surrogate_u_m.subs(m, m_ir)
    surrogate_pole = sp.series(
        surrogate_spatial.subs({s2: eps**2 * s2,
                                q4: eps**4 * q4,
                                q6: eps**6 * q6}),
        eps, 0, 8,
    ).removeO().expand().subs(eps, 1)
    expected_surrogate = (
        s2 / 7
        - sp.Rational(121, 123480) * s2**3
        + s2 * q4 / 504
        - q6 / 630
    )
    require(surrogate_pole == expected_surrogate,
            "L8 stable surrogate pole has the corrected complete q^6 tensor")
    checks += 1
    require(sp.expand(surrogate_pole - literal_pole)
            == -s2**3 / 2058,
            "L9 surrogate differs from literal BCC time first at isotropic q^6")
    checks += 1

    # ------------------------------------------------------------------
    # Natural period-three deeper match and exact endpoint instability.
    # ------------------------------------------------------------------
    ks3 = [sp.Rational(1, 7), sp.Rational(5, 14), -sp.Rational(1, 14)]
    monodromy3 = sp.eye(2)
    for kick in ks3:
        monodromy3 = transfer(kick, m) * monodromy3
    half_trace3 = sp.expand(sp.trace(monodromy3) / 2)
    target3 = 1 - sp.Rational(9, 14) * m \
        + sp.Rational(3, 196) * m**2 + sp.Rational(5, 2744) * m**3
    require(half_trace3 == target3,
            "P1 real period-three kicks match the BCC temporal germ through M^3")
    checks += 1
    require(half_trace3.subs(m, mmax) == -sp.Rational(15899, 9261),
            "P2 period-three endpoint is exactly -15899/9261")
    checks += 1
    require(half_trace3.subs(m, mmax) < -1,
            "P3 deeper scalar BCC match is unstable on the complete M18 band")
    checks += 1

    # ------------------------------------------------------------------
    # Live source contract: selected, default-off, P4-local, CPU-only.
    # ------------------------------------------------------------------
    header = read("engine/include/ftd/lorentz_bcc_time.h")
    toggles = read("engine/include/ftd/term_toggles.h")
    phase_read = read("engine/src/render_bridge_phases/phase_read.cpp")
    bridge = read("engine/src/render_bridge.cpp")
    cmake = read("engine/CMakeLists.txt")

    require("LORENTZ_BCC_TIME_EFFECTIVE_C2 = 1.0 / 7.0" in header
            and "moore_symbol / 7.0" in header
            and "moore_symbol * moore_symbol / 196.0" in header,
            "S1 header exposes the exact selected surrogate coefficients")
    checks += 1
    require("bool lorentz_bcc_time_floquet = false" in toggles
            and "FTD-0411 SELECTED IR PROTOTYPE" in toggles,
            "S2 engine toggle is explicitly selected and defaults off")
    checks += 1
    require("lorentz_bcc_time_kappa(rb.tick_)" in phase_read
            and "lap_L * cw2" in phase_read and "lap_R * cw2" in phase_read,
            "S3 live single/dual wave paths share the alternating local kick")
    checks += 1
    require("lorentz_bcc_time_floquet is CPU-scoped" in bridge
            and "|| toggles.lorentz_bcc_time_floquet" in bridge,
            "S4 runtime forces CPU and unit-step semantics")
    checks += 1
    require("test_lorentz_bcc_time_floquet" in cmake,
            "S5 native exact/live-wiring gate is registered")
    checks += 1

    print(f"\nRESULT  {checks}/{checks} exact/source-contract checks passed")
    print("SELECTED BCC supplies normalized temporal character (2/3)(1-cos^3 theta)")
    print("SPACE    production SC+FCC supplies M18")
    print("CONE     q^4 cancellation uniquely gives c^2=1/7")
    print("NO-GO    no exact finite-state positive-norm linear localization over Q(M18)")
    print("LOCAL    kicks (1+sqrt(2))/7,(1-sqrt(2))/7; Xmax=400/441")
    print("MISMATCH stable localization differs from literal BCC time at isotropic q^6")
    print("OPEN     nonlinear/constrained BCC clock and common interacting cone")


if __name__ == "__main__":
    main()
