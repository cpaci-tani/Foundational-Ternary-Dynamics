#!/usr/bin/env python3
"""
FTD self-dual half-shell audit.

This is a fixed structural check, not a numerical search. It compares:

1. The exact elliptic self-dual parameter m = 1/2 that reconstructs G*.
2. The dual-cell half-offset shell whose squared radius is r^2 = 1/2.

The output deliberately separates exact identities from the FTD bridge
hypothesis that these two appearances of one-half are the same datum.
"""

from __future__ import annotations

import math
from dataclasses import dataclass

import mpmath as mp


@dataclass(frozen=True)
class Shell:
    name: str
    count: int
    r2: mp.mpf
    example: str


def nstr(x: mp.mpf, digits: int = 24) -> str:
    return mp.nstr(x, digits)


def dual_cell_shells() -> list[Shell]:
    return [
        Shell("dual face centers", 6, mp.mpf(1) / 4, "(+/-1/2, 0, 0)"),
        Shell("dual edge centers", 12, mp.mpf(1) / 2, "(+/-1/2, +/-1/2, 0)"),
        Shell("dual corners", 8, mp.mpf(3) / 4, "(+/-1/2, +/-1/2, +/-1/2)"),
    ]


def moore_shells() -> list[Shell]:
    return [
        Shell("Moore SC face shell", 6, mp.mpf(1), "(+/-1, 0, 0)"),
        Shell("Moore FCC edge shell", 12, mp.mpf(2), "(+/-1, +/-1, 0)"),
        Shell("Moore BCC corner shell", 8, mp.mpf(3), "(+/-1, +/-1, +/-1)"),
    ]


def main() -> int:
    mp.mp.dps = 80

    gamma_quarter = mp.gamma(mp.mpf(1) / 4)
    gamma_three_quarter = mp.gamma(mp.mpf(3) / 4)
    gamma_half = mp.gamma(mp.mpf(1) / 2)
    pi = mp.pi

    # Equivalent G* definitions used in the repo.
    gstar_ratio = gamma_quarter / gamma_three_quarter
    gstar_gamma_primitive = gamma_quarter**2 / (mp.sqrt(2) * gamma_half**2)

    # mpmath.ellipk takes the parameter m, not the modulus k.
    m = mp.mpf(1) / 2
    k_modulus = mp.sqrt(m)
    k_complement = mp.sqrt(1 - m)
    k_half = mp.ellipk(m)
    k_half_analytic = gamma_quarter**2 / (4 * mp.sqrt(pi))
    gstar_from_k = 2 * mp.sqrt(2) * k_half / mp.sqrt(pi)

    q = mp.e ** (-pi)
    theta3 = mp.jtheta(3, 0, q)
    gstar_from_theta = mp.sqrt(2 * pi) * theta3**2

    print("=" * 72)
    print("FTD SELF-DUAL HALF-SHELL AUDIT")
    print("=" * 72)
    print()
    print("[THEOREM] Elliptic self-dual parameter")
    print(f"  parameter m                 = {nstr(m)}")
    print(f"  modulus k=sqrt(m)           = {nstr(k_modulus)}")
    print(f"  complement k'=sqrt(1-m)     = {nstr(k_complement)}")
    print(f"  |k-k'|                      = {nstr(abs(k_modulus - k_complement), 12)}")
    print()
    print("[THEOREM] K(1/2) reconstructs G*")
    print(f"  K(1/2) numerical            = {nstr(k_half)}")
    print(f"  K(1/2) analytic             = {nstr(k_half_analytic)}")
    print(f"  relative error              = {nstr(abs(k_half - k_half_analytic) / k_half_analytic, 12)}")
    print(f"  G* from K(1/2)              = {nstr(gstar_from_k)}")
    print(f"  G* Gamma(1/4)/Gamma(3/4)    = {nstr(gstar_ratio)}")
    print(f"  G* Gamma-primitive          = {nstr(gstar_gamma_primitive)}")
    print(f"  rel err K-vs-Gamma          = {nstr(abs(gstar_from_k - gstar_ratio) / gstar_ratio, 12)}")
    print()
    print("[THEOREM] Theta self-dual nome reconstructs G*")
    print(f"  q=e^-pi                     = {nstr(q)}")
    print(f"  theta_3(q)                  = {nstr(theta3)}")
    print(f"  sqrt(2pi) theta_3(q)^2      = {nstr(gstar_from_theta)}")
    print(f"  rel err theta-vs-Gamma      = {nstr(abs(gstar_from_theta - gstar_ratio) / gstar_ratio, 12)}")
    print()

    print("[THEOREM] Dual-cell half-offset shells")
    print("  shell                    count   r^2                  r                    t=r/c_FTD")
    print("  " + "-" * 82)
    dual_shells = dual_cell_shells()
    c_ftd = 1 / mp.sqrt(3)
    for shell in dual_shells:
        r = mp.sqrt(shell.r2)
        t = r / c_ftd
        print(
            f"  {shell.name:<24}"
            f"{shell.count:>5}   "
            f"{nstr(shell.r2, 18):<20}"
            f"{nstr(r, 18):<21}"
            f"{nstr(t, 18)}"
        )
    print()
    dual_face, dual_edge, dual_corner = dual_shells
    dual_midpoint_r2 = (dual_face.r2 + dual_corner.r2) / 2
    dual_edge_complement = 1 - dual_edge.r2
    print("[THEOREM] Dual-edge self-complement")
    print(f"  midpoint(face r^2, corner r^2) = {nstr(dual_midpoint_r2)}")
    print(f"  edge r^2                       = {nstr(dual_edge.r2)}")
    print(f"  1 - edge r^2                   = {nstr(dual_edge_complement)}")
    print(f"  midpoint error                 = {nstr(abs(dual_midpoint_r2 - dual_edge.r2), 12)}")
    print(f"  complement error               = {nstr(abs(dual_edge_complement - dual_edge.r2), 12)}")
    print()

    print("[THEOREM] Trilinear half-offset impulse weights")
    print("  A primal center impulse contributes 2^-d to a half-offset point")
    print("  with d nonzero half-offset axes.")
    print("  shell                    count   d   point weight          shell total")
    print("  " + "-" * 78)
    total_linear_weight = mp.mpf(0)
    shell_linear_weights: list[mp.mpf] = []
    for shell, d in zip(dual_shells, [1, 2, 3]):
        point_weight = mp.mpf(1) / (2**d)
        shell_total = shell.count * point_weight
        total_linear_weight += shell_total
        shell_linear_weights.append(shell_total)
        print(
            f"  {shell.name:<24}"
            f"{shell.count:>5}   "
            f"{d:>1}   "
            f"{nstr(point_weight, 18):<22}"
            f"{nstr(shell_total, 18)}"
        )
    print(f"  total shell weight                         {nstr(total_linear_weight, 18)}")
    print(f"  face total - edge total                    {nstr(shell_linear_weights[0] - shell_linear_weights[1], 12)}")
    print()
    print("[THEOREM] Moore neighbor shells, for comparison")
    print("  shell                    count   r^2                  r                    t=r/c_FTD")
    print("  " + "-" * 82)
    for shell in moore_shells():
        r = mp.sqrt(shell.r2)
        t = r / c_ftd
        print(
            f"  {shell.name:<24}"
            f"{shell.count:>5}   "
            f"{nstr(shell.r2, 18):<20}"
            f"{nstr(r, 18):<21}"
            f"{nstr(t, 18)}"
        )
    print()

    print("[SELECTION] Bridge reading")
    print("  The exact m=1/2 elliptic self-dual point and the exact r^2=1/2")
    print("  dual-edge shell are structurally aligned, but this script does not")
    print("  prove they are dynamically identical in FTD.")
    print()
    print("[OPEN] Next dynamical question")
    print("  Does the engine give the dual-edge shell a special role under")
    print("  primal/dual exchange, Gauss closure, or action balance?")

    # Hard fail only if exact identities are numerically broken.
    tolerance = mp.mpf("1e-50")
    checks = [
        abs(k_modulus - k_complement) < tolerance,
        abs(k_half - k_half_analytic) / k_half_analytic < tolerance,
        abs(gstar_from_k - gstar_ratio) / gstar_ratio < tolerance,
        abs(gstar_from_theta - gstar_ratio) / gstar_ratio < tolerance,
        abs(dual_midpoint_r2 - dual_edge.r2) < tolerance,
        abs(dual_edge_complement - dual_edge.r2) < tolerance,
        abs(shell_linear_weights[0] - shell_linear_weights[1]) < tolerance,
        abs(total_linear_weight - 7) < tolerance,
    ]
    return 0 if all(checks) else 1


if __name__ == "__main__":
    raise SystemExit(main())
