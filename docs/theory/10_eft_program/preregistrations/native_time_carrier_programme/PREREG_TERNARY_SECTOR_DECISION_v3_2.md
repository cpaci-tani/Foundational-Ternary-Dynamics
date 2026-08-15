# Preregistration: ternary-sector decision repair v3.2 (M4 reporting guard)

**Date locked:** 2026-08-14
**Status before execution:** `[PREREGISTERED — SECOND REPAIR WRAPPER]`
**Parents:** `PREREG_TERNARY_SECTOR_DECISION_v3.md` (lock `e61ff33c`) and
`PREREG_TERNARY_SECTOR_DECISION_v3_1.md` (first repair).

## 1. What happened

The v3.1 run completed **M3 J17: NO-STRESS** (coker(R)=0, the disclosed
convex-deltahedron expectation) and crashed in M4's terminal reporting
line: with zero clearance-passing solutions the `wall` variable is `None`
and `{wall:.4f}` raises. The crash is a reporting defect; it also reveals
the sweep's substantive content — the fully-swapped line-symmetric branch
produced no clearance-passing realization — which the wrapper must record
honestly with counters instead of crashing.

## 2. Authorized substitutions (two, one inherited)

1. (inherited from v3.1, unchanged) DomainMatrix backend for
   `Matrix.nullspace`/`Matrix.rank`, fallback preserved.
2. (new) `v3.m4` is replaced in memory by a byte-equivalent function whose
   **only** amendment is the terminal reporting block: a `None`-guard, a
   third verdict label `NO-ADMISSIBLE-CONFIG`, and sweep counters
   (solved / clearance-failed / admissible). The equation set, seeds, grid,
   tolerances, clearance bounds, and rank-drop threshold are unchanged
   verbatim.

## 3. Pins

| artifact | SHA-256 |
|---|---|
| wrapper `native_ternary_sector_decision_v3_2.py` | `85AC07E4B49BAECC4AC83201F8030293914CE32930A895C3454C0B59BFFC7760` |
| frozen parent instrument | `00E63B797F247BB35036F07FC456AB5F0882D210426ECD1BC0DB8B67CFA8FCC7` |
| frozen v3.1 wrapper | `011A4CD52F2974A6AB8210F2F3160AE9D09EEEAFC50A1853E6D8B1569AC8E49D` |

## 4. Checks and scope note

X01 parents byte-intact; X02 substitutions applied; X03 M1/M2/M3 verdicts
reproduce identically; parent checks C01–C06 inherited. Any failure →
`[EXECUTION INVALID]`.

**Scope note recorded at lock (cuts against the probe's reach):** the M4
ansatz swaps all three antipodal pairs under the C₂ axis; the branch with
one pair fixed on the axis — which contains the regular octahedron and its
line-symmetric deformations — is **not** covered by this probe and remains
open for a future campaign. A NO-ADMISSIBLE-CONFIG verdict therefore
closes only the fully-swapped branch.
