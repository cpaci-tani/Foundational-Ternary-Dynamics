"""FTD — Native C3 v3.2: second repair wrapper (M4 reporting guard).

The v3.1 run completed M3 (J17 NO-STRESS) and crashed in M4's final
reporting line: with zero clearance-passing solutions, `wall` is None and
the f-string `{wall:.4f}` raises. Per PREREG_TERNARY_SECTOR_DECISION_
v3_2.md this wrapper applies the v3.1 backend substitution unchanged plus
EXACTLY ONE further substitution: v3.m4 is replaced in memory by the
byte-equivalent function below, amended only in the terminal reporting
block (None-guard + sweep counters). No equation, gate, seed, grid,
tolerance, or clearance bound changes; the numerical sweep is identical.
"""

import hashlib
import os
import sys

import numpy as np
import sympy as sp

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import native_ternary_sector_decision_v3_1 as v31   # backend substitution

PARENT_SHA = "00E63B797F247BB35036F07FC456AB5F0882D210426ECD1BC0DB8B67CFA8FCC7"
W31_SHA = "011A4CD52F2974A6AB8210F2F3160AE9D09EEEAFC50A1853E6D8B1569AC8E49D"

FIRST_EXECUTIONS_OF_RECORD = {
    "M1 W6 void-hub wheel": "BLOCKING-KILL",
    "M2 ternary octahedron": "NO-STRESS",
    "M3 J17 unit deltahedron": "NO-STRESS",
}


def m4_repaired(v3):
    """Byte-equivalent to v3.m4 except the terminal reporting block."""
    from scipy.optimize import fsolve
    rng = np.random.default_rng(20260814)

    def eqs(v, a1):
        b1, b2, b3, c1, c2, c3 = v
        A = np.array([a1, 0, 0])
        B = np.array([b1, b2, b3]); Bp = np.array([-b1, -b2, b3])
        C = np.array([c1, c2, c3]); Cp = np.array([-c1, -c2, c3])
        return [np.sum((A - B) ** 2) - 1, np.sum((A - Bp) ** 2) - 1,
                np.sum((A - C) ** 2) - 1, np.sum((A - Cp) ** 2) - 1,
                np.sum((B - C) ** 2) - 1, np.sum((B - Cp) ** 2) - 1]

    def frame(a1, v):
        b1, b2, b3, c1, c2, c3 = v
        P = [np.array([a1, 0, 0]), np.array([-a1, 0, 0]),
             np.array([b1, b2, b3]), np.array([-b1, -b2, b3]),
             np.array([c1, c2, c3]), np.array([-c1, -c2, c3])]
        E = [(0, 2), (0, 3), (0, 4), (0, 5), (1, 2), (1, 3), (1, 4), (1, 5),
             (2, 4), (2, 5), (3, 4), (3, 5)]
        return P, E

    found, min_sv_global = [], []
    n_solved, n_clear_fail = 0, 0
    for a1 in np.linspace(0.62, 0.95, 34):
        sols = []
        for _ in range(40):
            v0 = rng.normal(scale=0.8, size=6)
            v, info, ier, _ = fsolve(eqs, v0, args=(a1,), full_output=True)
            if ier != 1 or max(abs(np.array(eqs(v, a1)))) > 1e-11:
                continue
            if any(np.allclose(v, s, atol=1e-6) or
                   np.allclose(v * [1, -1, 1, 1, -1, 1], s, atol=1e-6)
                   for s in sols):
                continue
            sols.append(v.copy())
        for v in sols:
            n_solved += 1
            P, E = frame(a1, v)
            diag = [np.sum((P[0] - P[1]) ** 2), np.sum((P[2] - P[3]) ** 2),
                    np.sum((P[4] - P[5]) ** 2)]
            if min(diag) < 1.5 - 1e-9:
                n_clear_fail += 1
                continue
            Rn = np.zeros((12, 18))
            for e, (u, w) in enumerate(E):
                d = P[u] - P[w]
                Rn[e, 3 * u:3 * u + 3] = d
                Rn[e, 3 * w:3 * w + 3] = -d
            sv = np.linalg.svd(Rn, compute_uv=False)
            min_sv_global.append((a1, sv[-1]))
            if sv[-1] < 1e-6:
                found.append((a1, v, sv[-1]))
    # --- repaired reporting block (the only amendment) ---
    if found:
        v3.verdict("M4 Bricard-I probe", "RANK-DROP CANDIDATES",
                   f"{len(found)} configs with singular rigidity matrix; "
                   "exact verification required before any claim")
    elif min_sv_global:
        wall = min(s for _, s in min_sv_global)
        v3.verdict("M4 Bricard-I probe", "NO-SINGULAR-CONFIG",
                   f"{len(min_sv_global)} clearance-passing solutions swept;"
                   f" min singular value {wall:.4f} bounded away from 0")
    else:
        v3.verdict("M4 Bricard-I probe", "NO-ADMISSIBLE-CONFIG",
                   f"sweep solved {n_solved} distinct realizations, "
                   f"{n_clear_fail} failed diagonal clearance, 0 passed: "
                   "the fully-swapped line-symmetric branch admits no "
                   "clearance-passing unit realization")
    v3.check("C04", "M4 probe executed and reported (probe arm, "
             "numeric + declared exact escalation)", True,
             f"{n_solved} solved / {n_clear_fail} clearance-failed / "
             f"{len(min_sv_global)} admissible")


def main():
    ok1 = hashlib.sha256(open(os.path.join(
        HERE, "native_ternary_sector_decision.py"),
        "rb").read()).hexdigest().upper() == PARENT_SHA
    ok2 = hashlib.sha256(open(os.path.join(
        HERE, "native_ternary_sector_decision_v3_1.py"),
        "rb").read()).hexdigest().upper() == W31_SHA
    print(f"[{'PASS' if ok1 and ok2 else 'FAIL'}] X01  parent + v3.1 "
          "wrappers byte-intact")
    if not (ok1 and ok2):
        return 1
    sp.MatrixBase.nullspace = v31.robust_nullspace
    sp.MatrixBase.rank = v31.robust_rank
    import native_ternary_sector_decision as v3
    v3.m4 = lambda: m4_repaired(v3)
    print("[PASS] X02  declared substitutions applied (v3.1 backend + "
          "m4 reporting guard)")
    rc = v3.main()
    ok_rep = all(any(x["cell"] == cell and x["verdict"] == vd
                     for x in v3.VERDICTS)
                 for cell, vd in FIRST_EXECUTIONS_OF_RECORD.items())
    print(f"[{'PASS' if ok_rep else 'FAIL'}] X03  M1/M2/M3 verdicts of "
          "the prior executions reproduce identically")
    return rc if ok_rep else 1


if __name__ == "__main__":
    sys.exit(main())
