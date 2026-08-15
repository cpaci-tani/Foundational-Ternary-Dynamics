# Preregistration: ternary-sector decision repair v3.1

**Date locked:** 2026-08-14
**Status before execution:** `[PREREGISTERED — REPAIR WRAPPER]`
**Parent:** `PREREG_TERNARY_SECTOR_DECISION_v3.md` (lock `e61ff33c`, tag
`preregister-ternary-sector-decision-v3`), first execution 2026-08-14.

## 1. What happened

The locked v3 instrument completed M1 and M2 with recorded verdicts —
**M1 W6 void-hub wheel: BLOCKING-KILL** (charpoly signs not alternating:
indefinite — the disclosed umbrella-inversion expectation) and **M2 ternary
octahedron: NO-STRESS** (coker(R) = 0; isostatic) — then crashed inside
sympy's naive `Matrix.nullspace` on M3's nested radicals (upstream
sympy/python-flint pathology: `minimal_polynomial` → `factorint` →
`OverflowError`). M4 never ran. The same pathology was already exhibited
and circumvented this session on A(6) via the `DomainMatrix`
algebraic-field route (exact rank 24, matching the numeric rank).

## 2. Authorized substitution (exactly one)

`Matrix.nullspace` / `Matrix.rank` are computed via `DomainMatrix` over the
algebraic field (entries `sqrtdenest`/`radsimp`-preprocessed, extension
discovered automatically), falling back to the parent implementation when
conversion fails. **The mathematical definitions are unchanged**; this is a
backend substitution only. No gate logic, tolerance, expectation, menu
cell, or expression under test may change. The parent instrument remains
byte-frozen and is imported intact.

## 3. Pins

| artifact | SHA-256 |
|---|---|
| repair wrapper `native_ternary_sector_decision_v3_1.py` | `011A4CD52F2974A6AB8210F2F3160AE9D09EEEAFC50A1853E6D8B1569AC8E49D` |
| frozen parent instrument | `00E63B797F247BB35036F07FC456AB5F0882D210426ECD1BC0DB8B67CFA8FCC7` |

## 4. Wrapper checks

W01 parent byte-intact; W02 the single substitution applied; W03 the M1/M2
verdicts of the first execution reproduce identically under the repaired
backend. Parent checks C01–C06 inherited unchanged. Any failure →
`[EXECUTION INVALID]`. Outcome taxonomy inherited from the parent prereg.
