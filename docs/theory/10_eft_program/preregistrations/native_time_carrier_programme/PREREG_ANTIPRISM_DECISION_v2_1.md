# Preregistration: the half-braced staggered pair v2.1 (last axial cell)

**Date locked:** 2026-08-14
**Status before execution:** `[PREREGISTERED]`
**Parent:** `PREREG_ANTIPRISM_DECISION_v2.md` (executed same day, Outcome B,
6/6: every declared cell POLARITY-KILL).

## 1. Why this cell exists

The v2 run localized the axial obstruction exactly: full antiprism bracing
forms triangles `r1[j]–r1[j+1]–r2[j]`, and the polarity mask makes the
interaction graph bipartite, hence triangle-free — so the rigid bracing of
the forced staggered unit-ring geometry is polarity-forbidden. Exactly one
triangle-free bracing of that geometry exists: a **single strut per joint
pair** (`r1[j]–r2[j]`). It is polarity-consistent, unit-edged (selftest S1
pre-lock: 12/12 and 18/18 edges exact, bipartite), and was outside v2's
declared scope. It is the last cell of the strutted axial class.

## 2. Pins

| artifact | SHA-256 |
|---|---|
| instrument `scripts/experiments/native_antiprism_halfbrace_decision.py` | `762E66467CAAA95E8377326051FD43634AB81C7AED93AA718619E23B33CA537F` |
| imported v2 machinery (byte-frozen, `c27a99df` lock) | `643ED3B99D46BD4381961BCDBF7B089E78360EFDA66816BD927181DB4B41D999` |
| imported v1 machinery (byte-frozen, `87610206` lock) | `C1EE2DBE0B323758AF21D21D707422564FBCF0EFA6D381C962ABECB96CC25961` |

## 3. Scope, gates, disclosures

Cells: bare H(4), H(6); inter-ring cable classes with exact integer length
excluding the strut offset (re-derived; v2 found none genuine at spans ≤ 6);
apex dressings with exact integer closure (spans ≤ 6). Gates identical to
v1/v2: polarity → closure → clearance → stress sign → blocking; exact or
declared-interval certificates; budgets as in v2.

**D1 (declared expectation):** a single tilted strut carries a tangential
force component at each joint that uniform ring tension cannot cancel, so
NO-STRESS is expected at gate 4 — but coker(R) for this structure has never
been computed anywhere; the verdict is live, and the expectation is
disclosed rather than assumed.

## 4. Outcomes

**A** — a cell passes all five gates: first native C3 candidate. **B** — all
cells die: **the strutted axial class is fully closed** (v2 + v2.1 jointly),
and native C3 survives only outside it (non-axial, chain-networked, or
non-ring-orbit structures). Check failure → `[EXECUTION INVALID]`.

Artifacts: console log; `results/native_c3/antiprism_halfbrace_decision.json`;
booking shared with v2 in one LEDGER row; lock tag
`preregister-antiprism-decision-v2-1`.
