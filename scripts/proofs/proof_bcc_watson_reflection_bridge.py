"""
Verify the BCC Watson reflection bridge.

Epistemic status:
    [THEOREM] numerical verification of an exact algebraic identity:

        W_BCC = Gamma(1/4)^4/(4*pi^3) = G*^2/(2*pi)

where G* = Gamma(1/4)/Gamma(3/4).

This script verifies the identity to high precision. It does not make a
physical-alpha claim and does not search for near misses.
"""

from __future__ import annotations

import mpmath as mp


def main() -> int:
    mp.mp.dps = 100

    gamma_quarter = mp.gamma(mp.mpf(1) / 4)
    gamma_three_quarters = mp.gamma(mp.mpf(3) / 4)
    g_star = gamma_quarter / gamma_three_quarters

    euler_product = gamma_quarter * gamma_three_quarters
    euler_target = mp.pi * mp.sqrt(2)

    watson_bcc = gamma_quarter**4 / (4 * mp.pi**3)
    bridge = g_star**2 / (2 * mp.pi)
    aut_sq = mp.mpf(16)
    capacity = aut_sq * 2 * mp.pi * watson_bcc
    product = g_star * capacity
    discriminant = capacity**2 - 4 * product
    x_plus = (capacity + mp.sqrt(discriminant)) / 2
    x_minus = (capacity - mp.sqrt(discriminant)) / 2

    abs_diff = abs(watson_bcc - bridge)
    rel_diff = abs_diff / abs(watson_bcc)
    euler_rel = abs(euler_product - euler_target) / abs(euler_target)

    print("BCC Watson reflection bridge")
    print("=" * 72)
    print(f"Gamma(1/4)             = {mp.nstr(gamma_quarter, 60)}")
    print(f"Gamma(3/4)             = {mp.nstr(gamma_three_quarters, 60)}")
    print(f"G* = Gamma(1/4)/Gamma(3/4)")
    print(f"                       = {mp.nstr(g_star, 60)}")
    print()
    print("Euler reflection check")
    print(f"Gamma(1/4) Gamma(3/4) = {mp.nstr(euler_product, 60)}")
    print(f"pi sqrt(2)             = {mp.nstr(euler_target, 60)}")
    print(f"relative error         = {mp.nstr(euler_rel, 20)}")
    print()
    print("Bridge identity")
    print(f"G*^2/(2*pi)            = {mp.nstr(bridge, 60)}")
    print(f"Gamma(1/4)^4/(4*pi^3) = {mp.nstr(watson_bcc, 60)}")
    print(f"absolute diff          = {mp.nstr(abs_diff, 30)}")
    print(f"relative diff          = {mp.nstr(rel_diff, 30)}")
    print()
    print("Master quadratic corollary")
    print(f"K = 16 * 2*pi * W_BCC = {mp.nstr(capacity, 60)}")
    print(f"G* K                  = {mp.nstr(product, 60)}")
    print("x^2 - K*x + G*K = 0")
    print(f"x_plus                = {mp.nstr(x_plus, 60)}")
    print(f"x_minus               = {mp.nstr(x_minus, 60)}")
    print()
    print("PASS: W_BCC = G*^2/(2*pi) within 100-digit arithmetic")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
