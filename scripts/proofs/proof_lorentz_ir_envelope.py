#!/usr/bin/env python3
"""Exact FTD-0414 infrared Lorentz-envelope derivation.

This script performs no parameter search and fits no physical target.  It
uses the selected FTD-0411 live flux clock and FTD-0413 free matter clock,
corrects the inherited sixth-order invariant-basis collision, and derives the
leading speed-spread envelope exactly on the direction simplex.
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
    qs = (qx, qy, qz)
    s2 = sum(q**2 for q in qs)
    q4 = sum(q**4 for q in qs)
    q6 = sum(q**6 for q in qs)

    # Correct complete M18 tensor in the unambiguous pure-power basis.
    cs = [sp.cos(eps * q) for q in qs]
    m18 = 4 - sp.Rational(2, 3) * sum(cs) - sp.Rational(2, 3) * (
        cs[0] * cs[1] + cs[0] * cs[2] + cs[1] * cs[2]
    )
    m18_series = sp.series(m18, eps, 0, 8).removeO().expand()
    expected_m18 = (
        eps**2 * s2
        - eps**4 * s2**2 / 12
        + eps**6 * (s2 * q4 / 72 - q6 / 90)
    )
    require(sp.expand(m18_series - expected_m18) == 0,
            "A1 M18 q6 tensor is S2*Q4/72-Q6/90")
    checks += 1
    require((s2 * q4 / 72 - q6 / 90).subs({qx: 1, qy: 0, qz: 0})
            == sp.Rational(1, 360),
            "A2 exact axis M18 coefficient is 1/360, not 1/120")
    checks += 1

    # Selected improved matter Hamiltonian, including the unit-step RK4 phase.
    b = sp.Rational(1, 3)
    r2 = sp.Rational(4, 3)
    kinetic = []
    for mu in range(3):
        nu = (mu + 1) % 3
        rho = (mu + 2) % 3
        transverse = (1 - 2 * b) + b * (cs[nu] + cs[rho])
        kinetic.append(sp.sin(eps * qs[mu]) * transverse)
    wilson = sum(1 - c for c in cs)
    matter_pole = sum(k * k for k in kinetic) + r2 * wilson**2
    matter_series = sp.series(matter_pole, eps, 0, 8).removeO().expand()
    matter_semidiscrete_q6 = s2**3 / 36 + s2 * q4 / 36 - q6 / 15
    require(sp.expand(matter_series.coeff(eps, 4)) == 0,
            "M1 selected matter pole remains q4-free")
    checks += 1
    require(sp.factor(matter_series.coeff(eps, 6)
                      - matter_semidiscrete_q6) == 0,
            "M2 complete semidiscrete matter q6 tensor is exact")
    checks += 1

    # For c_s^2=1/7, unit-step RK4 adds -S2^3/2940 after c_s^2 is factored.
    matter_live_q6 = sp.expand(matter_semidiscrete_q6 - s2**3 / 2940)
    expected_matter_live = (
        sp.Rational(121, 4410) * s2**3
        + s2 * q4 / 36
        - q6 / 15
    )
    require(sp.factor(matter_live_q6 - expected_matter_live) == 0,
            "M3 unit-step RK4 matter coefficient is 121/4410")
    checks += 1

    # Exact FTD-0411 live period-two phase series, now using corrected M18.
    S2, Q4, Q6 = sp.symbols("S2 Q4 Q6", real=True)
    m_ir = S2 - S2**2 / 12 + S2 * Q4 / 72 - Q6 / 90
    u_of_m = m_ir / 7 + m_ir**2 / 84 + sp.Rational(31, 30870) * m_ir**3
    flux_series = sp.series(
        u_of_m.subs({S2: eps**2 * S2,
                     Q4: eps**4 * Q4,
                     Q6: eps**6 * Q6}),
        eps, 0, 8,
    ).removeO().expand()
    flux_factored_q6 = sp.expand(7 * flux_series.coeff(eps, 6))
    expected_flux_live = (
        -sp.Rational(121, 17640) * S2**3
        + S2 * Q4 / 72
        - Q6 / 90
    )
    require(sp.factor(flux_factored_q6 - expected_flux_live) == 0,
            "F1 live BCC-time surrogate has the corrected q6 tensor")
    checks += 1
    require(expected_flux_live.subs({S2: 1, Q4: 1, Q6: 1})
            == -sp.Rational(1, 245),
            "F2 live flux axis coefficient is -1/245")
    checks += 1

    # Direction sphere becomes the compact simplex x_i=n_i^2>=0, sum x_i=1.
    x, y = sp.symbols("x y", real=True)
    z = 1 - x - y
    a4 = x**2 + y**2 + z**2
    a6 = x**3 + y**3 + z**3
    bm = sp.Rational(121, 4410) + a4 / 36 - a6 / 15
    bf = -sp.Rational(121, 17640) + a4 / 72 - a6 / 90
    gap = sp.factor(bm - bf)

    axis = {x: 1, y: 0}
    face = {x: sp.Rational(1, 2), y: sp.Rational(1, 2)}
    body = {x: sp.Rational(1, 3), y: sp.Rational(1, 3)}
    require(sp.factor(bm.subs(axis)) == -sp.Rational(101, 8820)
            and sp.factor(bm.subs(body)) == sp.Rational(155, 5292),
            "E1 matter axis/body coefficients are exact")
    checks += 1
    require(sp.factor(bf.subs(axis)) == -sp.Rational(1, 245)
            and sp.factor(bf.subs(body)) == -sp.Rational(55, 15876),
            "E2 flux axis/body coefficients are exact")
    checks += 1
    require(sp.factor(gap.subs(axis)) == -sp.Rational(13, 1764)
            and sp.factor(gap.subs(face)) == sp.Rational(193, 7056)
            and sp.factor(gap.subs(body)) == sp.Rational(130, 3969),
            "E3 common-cone gap is exact on symmetry directions")
    checks += 1

    # Exhaust the polynomial extrema: interior critical points plus each edge.
    matter_critical = sp.solve([sp.diff(bm, x), sp.diff(bm, y)], [x, y],
                               dict=True)
    require(body in matter_critical,
            "E4 body diagonal is an interior matter extremum")
    checks += 1
    matter_edge_roots = sp.solve(sp.diff(bm.subs(y, 1 - x), x), x)
    require(matter_edge_roots == [sp.Rational(1, 2)],
            "E5 face diagonal is the only non-vertex matter edge extremum")
    checks += 1
    matter_candidates = [bm.subs(axis), bm.subs(face), bm.subs(body)]
    require(min(matter_candidates) == -sp.Rational(101, 8820)
            and max(matter_candidates) == sp.Rational(155, 5292),
            "E6 matter extrema are axis minimum and body maximum")
    checks += 1

    gap_critical = sp.solve([sp.diff(gap, x), sp.diff(gap, y)], [x, y],
                            dict=True)
    gap_edge_roots = sp.solve(sp.diff(gap.subs(y, 1 - x), x), x)
    require(body in gap_critical
            and gap_edge_roots == [sp.Rational(1, 2)],
            "E7 common-gap extrema reduce to axis/face/body candidates")
    checks += 1
    require(max(abs(gap.subs(p)) for p in (axis, face, body))
            == sp.Rational(130, 3969),
            "E8 largest same-direction squared-phase gap is body diagonal")
    checks += 1

    all_speed_spread = sp.factor(
        (sp.Rational(155, 5292) + sp.Rational(101, 8820)) / 2
    )
    common_speed_gap = sp.Rational(1, 2) * sp.Rational(130, 3969)
    require(all_speed_spread == sp.Rational(11, 540),
            "V1 all-sector leading phase-speed spread is 11/540 q^4")
    checks += 1
    require(common_speed_gap == sp.Rational(65, 3969),
            "V2 same-direction matter/flux speed gap is 65/3969 q^4")
    checks += 1

    # Source and documentation contract.
    header = read("engine/include/ftd/lorentz_ir_envelope.h")
    native = read("engine/tests/test_lorentz_ir_envelope.cpp")
    cmake = read("engine/CMakeLists.txt")
    audit = read("docs/theory/07_assessment/lorentz_recovery_causal_structure/AUDIT_LORENTZ_IR_ENVELOPE.md")
    require("LORENTZ_IR_ALL_SPEED_SPREAD_COEFF = 11.0 / 540.0" in header
            and "LORENTZ_IR_COMMON_SPEED_GAP_COEFF = 65.0 / 3969.0" in header,
            "S1 engine header exposes both exact envelope coefficients")
    checks += 1
    require("richardson_b" in native and "exact axis live-flux phase" in native,
            "S2 native gate compares analytic coefficients to exact phases")
    checks += 1
    require("lorentz_ir_envelope" in cmake,
            "S3 native IR-envelope gate is registered")
    checks += 1
    require("FREE-TREE ADEQUACY CONDITIONAL ON CALIBRATION" in audit
            and "11}{540" in audit
            and "radiative generation" in audit,
            "S4 audit makes Planck-calibrated free-tree adequacy conditional")
    checks += 1

    print(f"\n{checks}/{checks} exact/source-contract checks passed")
    print("ENVELOPE  max Delta v/c_s = (11/540) (ka)^4 + O((ka)^6)")
    print("COMMON    max |v_m-v_f|/c_s = (65/3969) (ka)^4 + O((ka)^6)")
    print("VERDICT   IR BOUND DEFINED; FREE-TREE ADEQUACY CONDITIONAL ON CALIBRATION")


if __name__ == "__main__":
    main()
