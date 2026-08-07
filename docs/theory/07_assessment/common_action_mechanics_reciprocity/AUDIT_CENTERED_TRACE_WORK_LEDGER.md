# AUDIT — Centered-trace work ledger

**Date:** 2026-07-25  
**Identifier:** `FTD-0493`  
**Status:** `[THEOREM — EXACT CUSP-WORK DECOMPOSITION]` +
`[CLOSED NEGATIVE — EXISTING MATTER/FIELD ENERGY CLOSURE]` +
`[COUNTEREXAMPLE — SOURCE-FREE JUMP WORK]`  
**Verdict:** `CENTERED_TRACE_LEAVES_EXACT_CUSP_WORK_LEDGER`  
**Pre-registration:**
[`PREREG_CENTERED_TRACE_WORK_LEDGER_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_CENTERED_TRACE_WORK_LEDGER_v1.md)  
**Run of record:** `engine/results/ftd_0493/windows_msvc_cpu.json`

## 1. Exact omitted-work theorem

Write the two one-sided face traces as

```text
E_out,a=E_center,a+J_a/2,
E_in,a =E_center,a-J_a/2.
```

For a segment entering incident signs `sigma`, the exact cubical Whitney
current samples

```text
E_branch,a=E_center,a+sigma_a J_a/2.
```

Since `sigma_a d_a=|d_a|`,

```text
W_field  = g q E_branch dot d,
W_center = g q E_center dot d,
W_field-W_center
         = (g q/2) sum_a J_a |d_a|.
```

This identity is exact for the registered piecewise-constant incident-cell
fields. The worst formula residual is `3.99e-16`.

## 2. It is real field energy

Let `K` be the exact straight face current and center the endpoint electric
fields around the prescribed midpoint field:

```text
E0=E_mid+gK/2,
E1=E_mid-gK/2.
```

Then direct quadratic expansion gives

```text
(||E1||^2-||E0||^2)/2=-g<K,E_mid>=-W_field.
```

The locked implementation closes this field-energy identity to `3.31e-13`,
relative Gauss transport to `3.13e-16`, exact current continuity to
`6.67e-16`, and forward/reverse work to zero residual. The omitted centered
term is therefore not a display convention; it is part of the exact field
energy transaction.

The largest registered omission is

```text
0.024557002089434718.
```

## 3. It is not uniquely self-field work

The source-free control uses

```text
J=(0.3,-0.3,0),
sum_a J_a=0.
```

With unequal component displacement it still has

```text
W_cusp=-0.0014734201253664564.
```

Thus the centered trace removes all incident-cell jump work, including
divergence-free transverse jump structure. Total `rho` does not identify the
term as a particle's self-field. Calling it self-energy requires a separate
field decomposition/provenance rule, exactly the issue isolated by FTD-0488.

## 4. Consequence

The centered trace cannot close the existing common-action transaction by
itself. One of the following must be added and independently justified:

- a dressing/self-energy degree of freedom carrying `W_cusp`;
- a modified matter dispersion/energy;
- a modified field update/action;
- or a provenance rule that subtracts only a derived self component.

Every option is a new selected dynamics cycle. The compact face-action mobile
branch remains unlicensed; FTD-0481--0483 remain unexecuted.

## 5. Reproducibility

- checks: `8/8 PASS`;
- test SHA256:
  `6A0757328BF6B95C8691E117F0B3C96DFC1FE109EE2CABC0A3528796AC4E4875`;
- header SHA256:
  `30129256FC2108DF72DE09C1228CA3575AF17BC816D73B6F51F0903037EBC6D2`;
- implementation SHA256:
  `5E967A2E52711B496A5F1B07644F3B72CC99ED5CFD10580352A6B354D54AE10D`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state: unchanged.
