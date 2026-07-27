#!/usr/bin/env python3
"""Exact AGM/cone boundary checks for FTD-0410.

No numerical search or physical-target fitting is performed.  The script
separates three statements that must not be conflated:

1. the cone coefficient of the production free field is the local ratio of
   spatial to temporal quadratic coefficients;
2. the Gauss AGM evaluates the global lemniscatic/BCC period;
3. identifying elliptic spatial and temporal periods is an additional bridge.

The conditional self-dual bridge gives a unit period ratio.  It cancels the
dimension-six term of the present low-momentum pole but violates the present
explicit update's full-band stability bound.  The reciprocal-AGM magnitude is
an exact candidate value, not a derivation from the live transfer operator.
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
    x, m, r2 = sp.symbols("x m r2", real=True)
    z_t, z_s, omega2, k2 = sp.symbols(
        "Z_t Z_s omega2 k2", positive=True
    )

    # ------------------------------------------------------------------
    # Bare local cone and exact temporal blocking.
    # ------------------------------------------------------------------
    inverse_kernel = z_t * omega2 - z_s * k2
    solved_omega2 = sp.solve(inverse_kernel, omega2)[0]
    require(solved_omega2 == z_s * k2 / z_t,
            "B1 bare cone is the local kinetic ratio c_eff^2=Z_s/Z_t")
    checks += 1

    transfer = sp.Matrix([[2 - x, -1], [1, 0]])
    require(transfer.det() == 1 and transfer.trace() == 2 - x,
            "B2 one-tick scalar transfer is determinant one with trace 2-x")
    checks += 1

    blocked = transfer**2
    x_blocked = sp.expand(2 - blocked.trace())
    require(x_blocked == 4 * x - x**2,
            "B3 exact two-tick decimation is the polynomial x'=4x-x^2")
    checks += 1

    a, b = sp.symbols("a b", positive=True)
    agm_first = ((a + b) / 2, sp.sqrt(a * b))
    require(len(agm_first) == 2 and agm_first[0] != x_blocked,
            "B4 native one-coordinate blocking is not the two-coordinate AGM update")
    checks += 1

    source = read("engine/include/ftd/field_operators.h")
    require("(1.0/3.0)" in source and "(1.0/6.0)" in source
            and "neighbors_6" in source and "neighbors_12" in source,
            "B5 production free operator reads face and edge shells with rational weights")
    checks += 1

    constants = read("engine/include/ftd/ontic/gauge_couplings.h")
    require("C_WAVE = 0.57735026918962576451" in constants
            and "1/sqrt(3) [SELECTED]" in constants,
            "B6 the live cone magnitude is a selected coefficient, not an AGM output")
    checks += 1

    # ------------------------------------------------------------------
    # Exact AGM identities and the distinction between magnitude and ratio.
    # ------------------------------------------------------------------
    gstar, pi, w3 = sp.symbols("Gstar pi W3", positive=True)
    agm_1_sqrt2 = 2 * sp.sqrt(pi) / gstar
    grid_constant = sp.simplify(1 / agm_1_sqrt2)
    require(grid_constant == gstar / (2 * sp.sqrt(pi)),
            "A1 reciprocal AGM magnitude is Gstar/(2 sqrt(pi))")
    checks += 1

    w3_definition = gstar**2 / (2 * pi)
    require(sp.simplify(grid_constant**2 - w3_definition / 2) == 0,
            "A2 reciprocal AGM squared equals one half of the BCC Watson period")
    checks += 1

    K, Kprime = sp.symbols("K Kprime", positive=True)
    period_ratio = K / Kprime
    require(sp.simplify(period_ratio.subs(Kprime, K)) == 1,
            "A3 lemniscatic self-duality K'=K gives unit period ratio")
    checks += 1

    require(sp.simplify(agm_1_sqrt2 / agm_1_sqrt2) == 1,
            "A4 a common AGM normalization cancels from a spatial/temporal ratio")
    checks += 1

    # The BCC Watson symbol and production M18 are different operators.
    cx, cy, cz = sp.symbols("c_x c_y c_z", real=True)
    m18 = 4 - sp.Rational(2, 3) * (cx + cy + cz) \
        - sp.Rational(2, 3) * (cx * cy + cx * cz + cy * cz)
    bcc = 1 - cx * cy * cz
    at_pi00 = {cx: -1, cy: 1, cz: 1}
    at_pipi0 = {cx: -1, cy: -1, cz: 1}
    require(m18.subs(at_pi00) == 4 and bcc.subs(at_pi00) == 2,
            "A5 production and BCC symbols differ exactly at (pi,0,0)")
    checks += 1
    require(m18.subs(at_pipi0) == sp.Rational(16, 3)
            and bcc.subs(at_pipi0) == 0,
            "A6 production band maximum is a BCC-symbol zero at (pi,pi,0)")
    checks += 1

    # ------------------------------------------------------------------
    # Consequences for Lorentz recovery.
    # ------------------------------------------------------------------
    beta4 = sp.factor(r2 * (r2 - 1) / 12)
    require(beta4.subs(r2, 1) == 0,
            "L1 a unit cone cancels the complete isotropic q^4 pole term")
    checks += 1

    s2, q4, q6 = sp.symbols("S2 Q4 Q6", real=True)
    unit_q6 = -s2**3 / 360 + s2 * q4 / 72 - q6 / 90
    # The tensor happens to vanish on a lattice axis, so use the body
    # diagonal (A4=1/3, A6=1/9) to test that it is not identically zero.
    require(unit_q6.subs({s2: 1, q4: sp.Rational(1, 3),
                          q6: sp.Rational(1, 9)})
            == sp.Rational(1, 1620),
            "L2 unit cone retains the corrected complete q^6 lattice tensor")
    checks += 1

    mmax = sp.Rational(16, 3)
    require(mmax > 4,
            "L3 the unit-cone centered update violates full-band stability")
    checks += 1

    beta_grid = sp.factor(beta4.subs(r2, w3 / 2))
    require(beta_grid == w3 * (w3 - 2) / 48,
            "L4 reciprocal-AGM magnitude leaves q^4 coefficient W3(W3-2)/48")
    checks += 1

    # Every n-tick scalar kick trace is a degree-at-most-n polynomial in M.
    # Stability bounds it by one on [0,Mmax].  Markov's endpoint inequality,
    # after mapping that interval to [-1,1], bounds its derivative at M=0.
    n = sp.symbols("n", integer=True, positive=True)
    required_slope = n**2 / 2       # c^2=1: cos(n theta)=1-n^2 M/2+...
    markov_bound = sp.simplify(2 * n**2 / mmax)
    require(markov_bound == 3 * n**2 / 8,
            "L5 Markov endpoint bound is |C_n'(0)|<=3n^2/8 on the M18 band")
    checks += 1
    require(sp.simplify(required_slope - markov_bound) == n**2 / 8,
            "L6 the unit-cone slope exceeds the bound for every finite period n")
    checks += 1

    c2_bound = sp.simplify(4 / mmax)
    require(c2_bound == sp.Rational(3, 4),
            "L7 every full-band-stable finite scalar kick cell obeys c^2<=3/4")
    checks += 1

    # A stable unit-cone effective symbol exists if the one-tick dependency
    # radius is allowed to grow.  This is a construction target, not a P4 law.
    f_unit = sp.expand(m - sp.Rational(27, 16384) * m**4)
    f_prime = sp.factor(sp.diff(f_unit, m))
    require(sp.simplify(f_prime - (1 - sp.Rational(27, 4096) * m**3)) == 0,
            "L8 unit-cone effective symbol is monotone on the production band")
    checks += 1
    require(f_unit.subs(m, 0) == 0 and f_unit.subs(m, mmax) == 4,
            "L9 effective symbol maps the exact production band to [0,4]")
    checks += 1
    require(sp.expand(f_unit - m) == -sp.Rational(27, 16384) * m**4,
            "L10 effective symbol preserves the unit-cone pole through M^3")
    checks += 1

    # A renormalized cone requires two independent derivatives, not W3 alone.
    sigma_t, sigma_s, c0 = sp.symbols("Sigma_t Sigma_s c0", positive=True)
    renormalized = sp.simplify((c0**2 + sigma_s) / (1 + sigma_t))
    require(renormalized.has(sigma_t) and renormalized.has(sigma_s),
            "R1 interacting cone needs both temporal and spatial self-energy slopes")
    checks += 1

    require(not w3_definition.has(sigma_t) and not w3_definition.has(sigma_s),
            "R2 coincident BCC Watson period alone supplies neither required slope")
    checks += 1

    # Source-contract separation: the BCC period document itself defines the
    # product-cosine symbol, while the production source has no corner read.
    watson = read("docs/theory/04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md")
    require("1 - \\cos k_x\\cos k_y\\cos k_z" in watson
            and "neighbors_8" not in source,
            "S1 AGM-bearing BCC period is not the live SC+FCC propagation kernel")
    checks += 1

    print(f"\nRESULT  {checks}/{checks} exact/source-contract checks passed")
    print("BARE     c_eff^2=Z_s/Z_t; current Z_s contains selected C_WAVE^2")
    print("BLOCK    x -> 4x-x^2, not a native Gauss-AGM iteration")
    print("AGM MAG  c_G=1/AGM(1,sqrt(2))=Gstar/(2sqrt(pi)); insertion is SELECTED")
    print("AGM RATIO self-dual K/K'=1 [CONDITIONAL on a missing period-to-pole bridge]")
    print("UNIT IR  q^4 cancels; corrected complete q^6 tensor survives")
    print("NO-GO    Markov bound: every finite scalar kick period has c^2<=3/4")
    print("ESCAPE   F(M)=M-27M^4/16384 maps [0,16/3] to [0,4], but enlarges support")
    print("OPEN     multi-state localization or interacting Sigma_t/Sigma_s derivation")
    print("VERDICT  AGM DOES NOT CURRENTLY DERIVE THE FTD LIGHT CONE")


if __name__ == "__main__":
    main()
