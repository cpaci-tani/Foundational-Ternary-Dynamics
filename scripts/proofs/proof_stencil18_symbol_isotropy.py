#!/usr/bin/env python3
"""Exact symbol of the engine's 18-point Laplacian and its cubic anisotropy.

Measured 2026-09-14 by stepping one tick from a delta on the WASM engine
(C_SPEED^2 = 1/3, dt = 1): post-tick magnitudes 1/3 (centre), 1/9 (6 faces),
1/18 (12 edges). The unique zero-sum operator with those weights is

    Lap = -4 d + (1/3) sum_face + (1/6) sum_edge
    Lhat(q) = -4 + (2/3) sum_i cos q_i + (2/3) sum_{i<j} cos q_i cos q_j

Certified here, exactly:
  1. Lhat = -|q|^2 + |q|^4/12 + O(q^6): the sum_i q_i^4 terms cancel, so the
     cubic harmonic H4 = sum q_i^4 - (3/5)|q|^4 has coefficient 0 at 4th order.
  2. The q^6 coefficient on unit directions is -1/360 (axial), -1/240 (face
     diagonal), -11/3240 (body diagonal): anisotropy first enters at q^6.
  3. On an axis Lhat = -2(1 - cos k), so the leapfrog dispersion
     sin^2(w/2) = (C^2/4)(-Lhat) reduces to w = 2 asin(C sin(k/2)), the formula
     engine/tests/test_scenario_behavior.cpp already pins for axial modes.

Scope: the engine's v1 wave law. Not the v3 common-action Phi.
"""
from __future__ import annotations
from fractions import Fraction
from sympy import Rational, cos, expand, series, simplify, sqrt, symbols

qx, qy, qz, t, k = symbols("qx qy qz t k", real=True)
LHAT = (-4 + Rational(2, 3) * (cos(qx) + cos(qy) + cos(qz))
        + Rational(2, 3) * (cos(qx) * cos(qy) + cos(qy) * cos(qz) + cos(qz) * cos(qx)))
SYMBOL_Q6 = {"axial": Fraction(-1, 360), "face": Fraction(-1, 240), "body": Fraction(-11, 3240)}

def _c6_on(direction):
    poly = expand(series(LHAT.subs({qx: t * qx, qy: t * qy, qz: t * qz}), t, 0, 7).removeO())
    c6 = poly.coeff(t, 6)
    n = sqrt(sum(d * d for d in direction))
    return simplify(c6.subs({qx: direction[0] / n, qy: direction[1] / n, qz: direction[2] / n}))

def main() -> None:
    checks = 0
    poly = expand(series(LHAT.subs({qx: t * qx, qy: t * qy, qz: t * qz}), t, 0, 7).removeO())
    r2 = qx**2 + qy**2 + qz**2
    assert expand(poly.coeff(t, 0)) == 0 and expand(poly.coeff(t, 1)) == 0 and expand(poly.coeff(t, 3)) == 0 and expand(poly.coeff(t, 5)) == 0
    assert expand(poly.coeff(t, 2) + r2) == 0; checks += 1
    assert expand(poly.coeff(t, 4) - r2**2 / 12) == 0; checks += 1          # no sum q_i^4 term: H4 coefficient 0
    got = {"axial": _c6_on((1, 0, 0)), "face": _c6_on((1, 1, 0)), "body": _c6_on((1, 1, 1))}
    for key, val in SYMBOL_Q6.items():
        assert got[key] == Rational(val.numerator, val.denominator), (key, got[key]); checks += 1
    assert got["axial"] != got["face"]; checks += 1                           # anisotropy present at q^6
    assert simplify(LHAT.subs({qx: k, qy: 0, qz: 0}) + 2 * (1 - cos(k))) == 0; checks += 1
    print("Lhat = -|q|^2 + |q|^4/12 + O(q^6); H4 coefficient at 4th order = 0")
    print(f"q^6 coefficient on unit directions: axial {got['axial']}, face {got['face']}, body {got['body']}")
    print(f"PASS: stencil18 symbol isotropic through 4th order ({checks} exact checks)")

if __name__ == "__main__":
    main()
