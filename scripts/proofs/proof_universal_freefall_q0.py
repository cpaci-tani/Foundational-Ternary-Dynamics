#!/usr/bin/env python3
# -*- coding: ascii -*-
"""
proof_universal_freefall_q0.py -- FTD-1013 / PREREG_UNIVERSAL_FREEFALL_Q0_v1.md

Desk verifier for universal free-fall Q0. Recomputes Euler-Lagrange and
series identities; does not bookkeep author-supplied residuals.

Lock: preregister-universal-freefall-q0-v1
Prefix SHA256: 1F8A8005C05444EA04DC430FFD5CD620304D3C5D1AE3CAE8E0F35829BA215B23
Anchor: anchored-late until the git tag resolves.

No engine, no golden tick, no CODATA target, no near-miss search.
"""

from __future__ import annotations

import sympy as sp

PASS = 0
FAIL = 0
NOTES: list[str] = []


def check(tid: str, desc: str, cond) -> None:
    global PASS, FAIL
    ok = bool(cond)
    if ok:
        PASS += 1
    else:
        FAIL += 1
    print(("PASS " if ok else "FAIL ") + tid + " -- " + desc)


def note(msg: str) -> None:
    NOTES.append(msg)
    print("NOTE " + msg)


def main() -> int:
    print("=== FTD-1013 universal free-fall Q0 ===")
    print("lock prefix SHA256 1F8A8005C05444EA04DC430FFD5CD620304D3C5D1AE3CAE8E0F35829BA215B23")
    print("anchor: anchored-late (git tag not required of this process)")
    print()

    C, E, N, m_i, m_g = sp.symbols("C E N m_i m_g", positive=True)
    t = sp.symbols("t", real=True)
    x = sp.Function("x")
    Lf = sp.Function("L")

    u = sp.diff(x(t), t)
    lat = Lf(x(t))
    s = sp.sqrt((1 - lat**2) - (u / C) ** 2)

    # ------------------------------------------------------------------ V1
    print("--- V1: SR filter forces G(s) = s in class C ---")
    b, s_var = sp.symbols("b s", positive=True)
    # Invert s = sqrt(1-b^2) on (0,1]: b^2 = 1-s^2.
    inversion = sp.simplify(sp.sqrt(1 - (1 - s_var**2)) - s_var)
    ds_db = sp.diff(sp.sqrt(1 - b**2), b)
    check("V1a", "s=sqrt(1-b^2) inverts by b^2=1-s^2, recovering s", inversion == 0)
    check(
        "V1b",
        "ds/db = -b/s is nonzero on (0,1), so the image covers (0,1] locally 1-1",
        sp.simplify(ds_db + b / sp.sqrt(1 - b**2)) == 0,
    )
    # Existence of a solution: G(s)=s. Uniqueness is the inversion V1a+V1b
    # plus the V6 counterexample that a different G fails the filter.

    # ------------------------------------------------------------------ V6
    print("--- V6: G(s)=s^2 fails the SR filter ---")
    s_flat = sp.sqrt(1 - b**2)
    check("V6", "G(s)=s^2 does not equal s_flat", sp.simplify(s_flat**2 - s_flat) != 0)

    # ------------------------------------------------------------------ V2 (vacuous in C; sanity only)
    print("--- V2: overall factor E drops (zero evidential weight) ---")
    L_action = -E * s
    el = sp.diff(sp.diff(L_action, u), t) - sp.diff(L_action, x(t))
    el_over_E = sp.simplify(el / E)
    check("V2", "EL residual / E is independent of E", E not in el_over_E.free_symbols)
    note("V2 is declared vacuous inside class C (LOCK-STD firewall); weight zero.")

    # ------------------------------------------------------------------ V3
    print("--- V3: SPEC 5.4 weak expansion recomputed ---")
    beta, Lat = sp.symbols("beta Lat")
    core = -E * sp.sqrt(1 - beta**2 - Lat**2)
    s3 = sp.expand(core.series(beta, 0, 4).removeO().series(Lat, 0, 4).removeO())
    c0 = sp.simplify(s3.subs({beta: 0, Lat: 0}))
    c_b2 = sp.simplify(s3.coeff(beta**2).subs(Lat, 0))
    c_L2 = sp.simplify(s3.coeff(Lat**2).subs(beta, 0))
    check("V3a", "constant term -E", c0 == -E)
    check("V3b", "beta^2 coefficient E/2", sp.simplify(c_b2 - E / 2) == 0)
    check("V3c", "L^2 coefficient E/2", sp.simplify(c_L2 - E / 2) == 0)

    # ------------------------------------------------------------------ V4 / V5
    print("--- V4/V5: weak EL => a = C^2 L L' and Phi_N = -(C^2/2) L^2 ---")
    u_sym, a_sym, x_sym = sp.symbols("u a x_sym", real=True)
    L_weak = -E + (E / (2 * C**2)) * u_sym**2 + (E / 2) * Lf(x_sym) ** 2
    el_weak = (E / C**2) * a_sym - sp.diff(L_weak, x_sym)
    a_sol = sp.simplify(sp.solve(el_weak, a_sym)[0])
    a_expected = C**2 * Lf(x_sym) * sp.diff(Lf(x_sym), x_sym)
    check("V4", "weak EL gives a = C^2 L dL/dx", sp.simplify(a_sol - a_expected) == 0)

    Phi_N = -(C**2 / 2) * Lf(x_sym) ** 2
    minus_dPhi = sp.simplify(-sp.diff(Phi_N, x_sym))
    check("V5", "a = -d Phi_N / dx with Phi_N = -(C^2/2) L^2", sp.simplify(minus_dPhi - a_expected) == 0)

    print("--- exact EL factorisation (not a V4 substitute) ---")
    p_exact = sp.simplify(sp.diff(L_action, u))
    p_form = E * (u / C**2) / s
    check("X1", "canonical momentum p = E u / (C^2 s)", sp.simplify(p_exact - p_form) == 0)

    # ------------------------------------------------------------------ V7
    print("--- V7: Newtonian split with independent m_i, m_g is mass-dependent ---")
    Phi = sp.Function("Phi")
    a_split = sp.simplify(sp.solve(m_i * a_sym + m_g * sp.diff(Phi(x_sym), x_sym), a_sym)[0])
    check(
        "V7a",
        "split EL is a = -(m_g/m_i) Phi'",
        sp.simplify(a_split + (m_g / m_i) * sp.diff(Phi(x_sym), x_sym)) == 0,
    )
    check(
        "V7b",
        "a_split depends on the independent ratio m_g/m_i",
        m_g in a_split.free_symbols and m_i in a_split.free_symbols,
    )

    # ------------------------------------------------------------------ V8
    print("--- V8: N-extensivity, same weak a ---")
    L_N = -N * E + (N * E / (2 * C**2)) * u_sym**2 + (N * E / 2) * Lf(x_sym) ** 2
    a_N = sp.simplify(sp.solve((N * E / C**2) * a_sym - sp.diff(L_N, x_sym), a_sym)[0])
    check(
        "V8",
        "weak a independent of N",
        sp.simplify(a_N - a_expected) == 0 and N not in a_N.free_symbols,
    )

    # ------------------------------------------------------------------ V9
    print("--- V9: engine F/M remainder is V7-class, not class C ---")
    # Class C after cancelling alpha has no remaining mass parameter in a
    # (V2 + V4). The split a retains m_g/m_i. Therefore the split (and the
    # live F/M integrator that implements it) is not a C-action.
    a_C = a_expected  # from L = -E s, weak
    check(
        "V9a",
        "weak C-action acceleration carries no mass parameter",
        E not in a_C.free_symbols and C in a_C.free_symbols,
    )
    check(
        "V9b",
        "split acceleration is not identical to the C-action acceleration",
        sp.simplify(a_split - a_C) != 0,
    )

    print()
    print("--- classifier (PREREG section 5) ---")
    if FAIL:
        verdict = "CLOSED-NEGATIVE"
        reason = "one or more frozen checks failed"
    else:
        verdict = "FOUND"
        reason = "V1 and V3-V9 passed; V2 recorded as vacuous sanity"
    print("VERDICT " + verdict)
    print("REASON " + reason)
    print("PASS %d  FAIL %d" % (PASS, FAIL))
    if NOTES:
        print("NOTES:")
        for item in NOTES:
            print("  - " + item)
    print()
    print("Tags licensed if FOUND (prereg section 5):")
    print("  [THEOREM given FC-2]  UFF of S = -alpha int d tau inside class C")
    print("  [THEOREM given FC-2]  weak a = -grad Phi_N, Phi_N = -(C^2/2) L^2")
    print("  [SELECTION]           class C / geodesic principle remainder")
    print("  unmoved: FTD-0250, 0349, 0402, 0208, U-8, engine m_i=m_g [IMPOSED]")
    return 0 if FAIL == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
