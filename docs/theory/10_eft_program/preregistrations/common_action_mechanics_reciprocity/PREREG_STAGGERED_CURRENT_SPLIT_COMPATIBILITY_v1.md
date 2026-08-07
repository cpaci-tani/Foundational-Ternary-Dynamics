# PRE-REGISTRATION — Staggered current-split compatibility

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0535`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0484`, `FTD-0530`, `FTD-0531`, `FTD-0534`  
**Scope:** observer-only comparison between the exact temporal endpoint split
of the FTD-0484 worldline action and the frozen Faraday-then-current staggered
ordering used to embed FTD-0531. No production state, default, toggle, scenario,
force, collision law, phase order, field ontology, normalization, or tolerance
change.

## 1. Exact endpoint split

The FTD-0484 spatial coupling on one slab is

```text
S_int=g(<A_0,K^(0)>+<A_1,K^(1)>-lambda<Phi,Q>),
K^(0)+K^(1)=K.
```

Thus variation of `A_0` and `A_1` necessarily exposes `K^(0)` and `K^(1)`
separately. For the minimal quadratic electric slab action

```text
S_E=(beta/(2 lambda))||A_1-A_0||^2+beta S_int/g,
```

the canonical endpoint fields are

```text
E_0=E_slab+K^(0),
E_1=E_slab-K^(1),
E_1-E_0=-K.
```

The frozen FTD-0531 staggered embedding uses `E_0` unchanged during Faraday,
so its magnetic endpoints require `E_slab=E_0` in the transverse sector.
Exact action compatibility therefore requires

```text
C^T K^(0)=0.
```

This is the endpoint-resolved version of FTD-0534; no midpoint approximation
is used.

## 2. Registered arms and gates

Reconstruct both carriers' exact FTD-0484 `spatial_start` and `spatial_end`
deposits on all 312 Moore-direction arms.

1. `K^(0)+K^(1)=K` must hold componentwise below `1e-12`, and each split
   continuity identity inherited from FTD-0484 must close below `1e-12`.
2. On all 240 edge/corner FTD-0531 roots, `||C^T K^(0)||^2` must be positive
   and the locked-Faraday mismatch `lambda C^T K^(0)` must exceed `1e-8`.
3. The component and norm mismatch identities must close below `1e-12`.
4. On all 72 axial controls, both endpoint splits, total current, and mismatch
   must vanish below `1e-12`.
5. Translation, polarity mirror, and signed-cubic magnitudes must agree below
   `1e-12`; inherited endpoint identities remain below `1e-10`.
6. Invalid inputs fail closed.

## 3. Locked verdicts

- If the exact start split is transverse and nonzero on every diagonal arm:
  `EXACT_WORLDLINE_SPLIT_REQUIRES_IMPLICIT_ATOMIC_FIELD_TRANSACTION`.
- If `C^T K^(0)=0` on every diagonal arm:
  `FROZEN_STAGGERED_ORDER_COMPATIBLE_WITH_EXACT_WORLDLINE_SPLIT`.
- If split closure or the mismatch identity fails:
  `STAGGERED_CURRENT_SPLIT_COMPATIBILITY_UNRESOLVED`.

The first verdict would not forbid a common action. It would prove that the
current frozen field phase ordering is not that action: the face field,
worldline, and both endpoint current deposits must be solved in one implicit
atomic transaction. That is selected new dynamics and cannot be inserted into
production without the remaining algebraic, reversal, and energy gates.

## 4. Execution record

Executed 2026-07-25 without changing the locked endpoint split or gates. All
`7/7` checks pass over 312 arms. Every diagonal start deposit has nonzero curl
and obstructs the frozen Faraday phase; all axial endpoint splits vanish.
Locked verdict:

```text
EXACT_WORLDLINE_SPLIT_REQUIRES_IMPLICIT_ATOMIC_FIELD_TRANSACTION
```

Canonical audit:
[`AUDIT_STAGGERED_CURRENT_SPLIT_COMPATIBILITY.md`](../../../07_assessment/common_action_mechanics_reciprocity/AUDIT_STAGGERED_CURRENT_SPLIT_COMPATIBILITY.md).
The SHA256 of this preregistration before this execution annotation was
`26DC878B0284D60FE7F8CBC79CEF00204D47DC1F1E201F7ACD23DF1DCFF49E0F`.
