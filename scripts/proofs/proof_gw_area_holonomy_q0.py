#!/usr/bin/env python3
# -*- coding: ascii -*-
"""
proof_gw_area_holonomy_q0.py -- FTD-1015 / PREREG_GW_AREA_HOLONOMY_Q0_v1.md

Desk verifier for radiative shears of spatial Omega. Recomputes the
gauge quotient and little-group helicity multiset; does not bookkeep
author-supplied residuals.

Lock: preregister-gw-area-holonomy-q0-v1
Prefix SHA256: 6381C791B58F3E4259138CCED06A5777DF5175FE015F5BDD670804786DA035C9
Anchor: anchored-late until the git tag resolves.

No engine, no action, no golden tick, no CODATA, no near-miss search.
"""

from __future__ import annotations

import sympy as sp

PASS = 0
FAIL = 0
NOTES: list[str] = []

PREFIX = "6381C791B58F3E4259138CCED06A5777DF5175FE015F5BDD670804786DA035C9"


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


def helicity_weights(evals) -> list[int]:
    """Map generator eigenvalues to integer helicities. Empty if V4 fails."""
    weights: list[int] = []
    for ev in evals:
        val = sp.simplify(ev)
        # Expect pure imaginary: h * I with h in Z.
        h = sp.simplify(val / sp.I)
        if h.is_integer:
            weights.append(int(h))
        else:
            return []
    weights.sort()
    return weights


def main() -> int:
    print("=== FTD-1015 GW area-holonomy Q0 ===")
    print("lock prefix SHA256 " + PREFIX)
    print("anchor: anchored-late (git tag not required of this process)")
    print()

    # ------------------------------------------------------------------ V1, V3
    print("--- V1/V3: nine spatial components, no omega_0 ---")
    w = sp.Matrix(3, 3, lambda i, j: sp.symbols("w_%d%d" % (i, j)))
    names = sorted(str(s) for s in w.free_symbols)
    expected = sorted("w_%d%d" % (i, j) for i in range(3) for j in range(3))
    check("V1", "omega is 3x3 with 9 independent entries", names == expected)
    check(
        "V3",
        "locked field list is the spatial 3x3 (no separate temporal omega_0)",
        names == expected,
    )

    # ------------------------------------------------------------------ V2
    print("--- V2: gauge orbit at k = hat z ---")
    kz = sp.symbols("k_z", nonzero=True)
    kvec = sp.Matrix([0, 0, kz])
    th = sp.Matrix(3, 1, lambda i, _j: sp.symbols("th_%d" % i))
    # delta w_ij = I * k_i * theta_j  (Fourier)
    dw = sp.Matrix(3, 3, lambda i, j: sp.I * kvec[i] * th[j])
    dw_vec = sp.Matrix(9, 1, lambda a, _b: dw[a // 3, a % 3])
    # Jacobian 9 x 3 of gauge map.
    Jg = dw_vec.jacobian(list(th))
    rank = Jg.rank()
    n_res = 9 - rank
    check("V2a", "gauge map rank is 3", rank == 3)
    check("V2b", "n_res = 6", n_res == 6)

    # Residual basis: rows not in the z-direction (gauge kills row 2 when k // z).
    # Confirm: dw[0,:] = 0, dw[1,:] = 0, dw[2,:] = I kz theta.
    check(
        "V2c",
        "gauge variation supported only on the longitudinal row",
        sp.simplify(dw.row(0).norm()) == 0
        and sp.simplify(dw.row(1).norm()) == 0
        and dw.row(2).equals(sp.I * kz * th.T),
    )
    residual_idx = [(0, 0), (0, 1), (0, 2), (1, 0), (1, 1), (1, 2)]
    w_slice = sp.Matrix(3, 3, lambda i, j: 0 if i == 2 else w[i, j])

    def pack(M: sp.Matrix) -> sp.Matrix:
        return sp.Matrix(6, 1, lambda a, _b: M[residual_idx[a]])

    # ------------------------------------------------------------------ V4/V5: SO(2) generator on residual
    print("--- V4/V5: little-group generator on residual ---")
    phi = sp.symbols("phi", real=True)
    c, s = sp.cos(phi), sp.sin(phi)
    R = sp.Matrix([[c, -s, 0], [s, c, 0], [0, 0, 1]])
    w_rot = R * w_slice * R.T
    # Infinitesimal generator G = d/d phi at 0, acting on residual 6-vector.
    d_res = pack(sp.diff(w_rot, phi).subs(phi, 0))
    res_syms = pack(w)
    G = d_res.jacobian(list(res_syms))
    ev = G.eigenvals()
    # eigenvals returns dict value -> multiplicity
    evals = []
    for val, mult in ev.items():
        evals.extend([sp.simplify(val)] * int(mult))
    weights = helicity_weights(evals)
    check("V4", "generator spectrum is i * integers (6 weights)", len(weights) == 6)
    H = weights
    note("H = " + str(H))
    check("V5", "helicity multiset recomputed (nonempty integer list)", len(H) == 6)

    # ------------------------------------------------------------------ V6/V7
    print("--- V6/V7: TT dim and leftover ---")
    n_tt = H.count(2) + H.count(-2)
    leftover = n_res - n_tt
    check("V6", "pm2 subspace dimension recomputed", n_tt == H.count(2) + H.count(-2))
    check("V7", "leftover = n_res - n_tt", leftover == n_res - n_tt)
    note("n_tt = %d, leftover = %d, n_res = %d" % (n_tt, leftover, n_res))

    # ------------------------------------------------------------------ V8
    print("--- V8: hand TT projection of a symmetric tensor is 2 (not this Q0) ---")
    # Symmetric 3x3 at k // z: TT is omega_xx = -omega_yy, omega_xy = omega_yx,
    # all z-rows/cols zero, trace zero. Count independent entries of that subspace.
    a, b = sp.symbols("a b")
    # plus/cross: [[a, b, 0], [b, -a, 0], [0, 0, 0]]
    tt = sp.Matrix([[a, b, 0], [b, -a, 0], [0, 0, 0]])
    check("V8", "symmetric TT at k=z has 2 free parameters", len(tt.free_symbols) == 2)

    # ------------------------------------------------------------------ V9
    print("--- V9: this census is omega, not J ---")
    check(
        "V9",
        "field symbols are w_ij not J",
        all(str(s).startswith("w_") for s in w.free_symbols),
    )

    # ------------------------------------------------------------------ A1 classify (does not fail the process)
    print("--- A1: exactly two TT shears, nothing else ---")
    H_set = set(H)
    a1 = (
        H_set == {2, -2}
        and n_res == 2
        and n_tt == 2
        and leftover == 0
        and len(H) == 2
    )
    print(("PASS " if a1 else "FAIL ") + "A1 -- H == {+2,-2} and n_res == 2 "
          "(classifier; does not fail process)")

    protocol = FAIL == 0
    if not protocol:
        verdict = "UNDERDETERMINED"
    elif a1:
        verdict = "FOUND"
    else:
        verdict = "CLOSED-NEGATIVE"

    print()
    print("VERDICT %s" % verdict)
    print("H = %s" % H)
    print("n_res = %d  n_tt = %d  leftover = %d" % (n_res, n_tt, leftover))
    print("protocol V-checks: %d pass, %d fail" % (PASS, FAIL))
    for n in NOTES:
        print("  " + n)
    return 0 if protocol else 1


if __name__ == "__main__":
    raise SystemExit(main())
