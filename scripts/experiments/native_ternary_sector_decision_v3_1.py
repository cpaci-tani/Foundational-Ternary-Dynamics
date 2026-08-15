"""FTD — Native C3 v3.1: repair wrapper for the ternary-sector decision.

The locked v3 instrument (native_ternary_sector_decision.py, byte-frozen)
crashed in M3 at sympy's naive Matrix.nullspace on J17's nested radicals —
an upstream sympy/python-flint pathology (minimal_polynomial -> factorint
overflow), the same one already exhibited and circumvented on A(6) via the
DomainMatrix algebraic-field route. Per PREREG_TERNARY_SECTOR_DECISION_
v3_1.md, this wrapper applies EXACTLY ONE substitution:

  Matrix.nullspace and Matrix.rank are computed via DomainMatrix over the
  algebraic field (entries sqrtdenest/radsimp-preprocessed, extension
  discovered automatically), falling back to the parent implementation
  when conversion fails. The mathematical definitions are unchanged.

No gate logic, tolerance, expectation, menu cell, or expression under test
changes. The parent module is imported byte-intact and its main() re-run;
wrapper checks assert the parent hash, the substitution scope, and that
the M1/M2 verdicts of the first (crashed) execution reproduce identically.
"""

import hashlib
import os
import sys

import sympy as sp
from sympy.polys.matrices import DomainMatrix

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

PARENT = os.path.join(HERE, "native_ternary_sector_decision.py")
PARENT_SHA = "00E63B797F247BB35036F07FC456AB5F0882D210426ECD1BC0DB8B67CFA8FCC7"

FIRST_EXECUTION_OF_RECORD = {
    "M1 W6 void-hub wheel": "BLOCKING-KILL",
    "M2 ternary octahedron": "NO-STRESS",
}


def _prep(M):
    return M.applyfunc(lambda x: sp.radsimp(sp.sqrtdenest(sp.nsimplify(
        sp.simplify(sp.expand_trig(x)), rational=False))))


_orig_nullspace = sp.MatrixBase.nullspace
_orig_rank = sp.MatrixBase.rank


def robust_nullspace(self, *a, **k):
    try:
        dm = DomainMatrix.from_Matrix(_prep(self), extension=True)
        ns = dm.nullspace().to_Matrix()
        return [ns[i, :].T for i in range(ns.rows)]
    except Exception:
        return _orig_nullspace(self, *a, **k)


def robust_rank(self, *a, **k):
    try:
        dm = DomainMatrix.from_Matrix(_prep(self), extension=True)
        return dm.rank()
    except Exception:
        return _orig_rank(self, *a, **k)


def main():
    got = hashlib.sha256(open(PARENT, "rb").read()).hexdigest().upper()
    print(f"[{'PASS' if got == PARENT_SHA else 'FAIL'}] W01  parent "
          f"instrument byte-intact")
    if got != PARENT_SHA:
        return 1
    sp.MatrixBase.nullspace = robust_nullspace
    sp.MatrixBase.rank = robust_rank
    print("[PASS] W02  single declared substitution applied "
          "(DomainMatrix backend for nullspace/rank, fallback preserved)")
    import native_ternary_sector_decision as v3
    rc = v3.main()
    ok_rep = all(any(x["cell"] == cell and x["verdict"] == vd
                     for x in v3.VERDICTS)
                 for cell, vd in FIRST_EXECUTION_OF_RECORD.items())
    print(f"[{'PASS' if ok_rep else 'FAIL'}] W03  M1/M2 verdicts of the "
          f"first execution reproduce identically")
    return rc if ok_rep else 1


if __name__ == "__main__":
    sys.exit(main())
