# AUDIT — Staggered current-split compatibility

**Date:** 2026-07-25  
**Identifier:** `FTD-0535`  
**Status:** `[THEOREM — EXACT ENDPOINT CURRENT SPLIT]` +
`[CLOSED NEGATIVE — FROZEN STAGGERED PHASE ORDER AS COMMON ACTION]` +
`[RESOLVED BY FTD-0536 — IMPLICIT ATOMIC ACTION CONSTRUCTIVE]` +
`[OPEN — NEW NONLINEAR ROOT/ENERGY/REVERSAL]`  
**Verdict:**
`EXACT_WORLDLINE_SPLIT_REQUIRES_IMPLICIT_ATOMIC_FIELD_TRANSACTION`  
**Pre-registration:**
[`PREREG_STAGGERED_CURRENT_SPLIT_COMPATIBILITY_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_STAGGERED_CURRENT_SPLIT_COMPATIBILITY_v1.md)  
**Run of record:** `engine/results/ftd_0535/windows_msvc_cpu.json`

## 1. Endpoint-resolved action identity

FTD-0484 does not couple a straight worldline to one instantaneous vector
potential. Its exact spatial action is

```text
S_int=g(<A_0,K^(0)>+<A_1,K^(1)>),
K^(0)+K^(1)=K.
```

The start/end deposits are the analytic `(1-tau)` and `tau` moments of the
same cubical Whitney current. Varying the two connection endpoints therefore
exposes them separately.

For the minimal quadratic electric slab action, the canonical fields obey

```text
E_0=E_slab+K^(0),
E_1=E_slab-K^(1),
E_1-E_0=-K.
```

The frozen FTD-0531 staggered embedding holds `E_0` fixed during its Faraday
substep. Its locked magnetic endpoints can agree with the action only if

```text
C^T K^(0)=0.
```

The exact mismatch is

```text
R_split=-lambda C^T K^(0),
||R_split||^2=lambda^2||C^T K^(0)||^2.
```

## 2. Registered result

The endpoint deposits recombine to the complete current on every arm, with
worst component residual `2.7755575615628914e-17`. All split continuity
identities close below `6.439293542825908e-15`.

Every one of the 240 diagonal roots has a nonzero start deposit and transverse
curl:

```text
0.1670689952078817 <= ||K^(0)||_1 <= 0.4116980400073736,
0.01317635714960842 <= ||C^T K^(0)||^2
                    <= 0.06116216693129017.
```

The resulting locked-order mismatch is

```text
0.06627306428609947 <= ||R_split||
                    <= 0.1427844143353773.
```

Its component and norm identities close below `3.13e-17`; translation,
polarity, and signed-cubic residuals stay below `3.96e-16`.

All 72 symmetric axial controls have zero total, start, and end currents. Their
frozen staggered order is therefore compatible for the same reason FTD-0530
needed no impulse.

## 3. Consequence

This result does not forbid a common action. It forbids interpreting the
frozen sequence

```text
Faraday with E_0, then deposit all K
```

as the exact FTD-0484 action. The action places nonzero current on both temporal
endpoints, and the start deposit changes the same canonical relation used by
Faraday.

The remaining candidate must solve particle endpoints, face field, connection
stage values, `K^(0)`, and `K^(1)` in one implicit atomic transaction. That is
selected new dynamics. It must close energy, Gauss, exact current, gauge
covariance, causality, cubic covariance, and reversal before any default-off
toggle or scenario can be licensed.

FTD-0536 constructs the minimal such action and closes its field equations.
None of the old FTD-0531 scalar roots solves its particle endpoint equations,
so the remaining task is a fresh simultaneous nonlinear root rather than
action existence.

No production code, default, toggle, scenario, force, collision law, phase
order, field ontology, normalization, or tolerance changed.

## 4. Reproducibility

- checks: `7/7 PASS` over `312` Moore-direction arms;
- test SHA256:
  `54AAD8402C3BCF44C11BB3A2E9E3CF44258F266F46041BFB29B6A92E36BB6CA8`;
- header SHA256:
  `E02B0F513011191BC58ED96748257A2414364D8347704B712F28B8ECE1EA59D4`;
- implementation SHA256:
  `76F0D795B0AE98291394F020FB6F7C4B08A9749FBD71A463C0235DF8E4890506`;
- locked preregistration SHA256:
  `26DC878B0284D60FE7F8CBC79CEF00204D47DC1F1E201F7ACD23DF1DCFF49E0F`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
