# PRE-REGISTRATION — Centered-trace work ledger v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0493`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0492`

## Question

When the centered knot trace replaces the ordinary one-sided branch
derivative, does exact matched-face field work still equal the work assigned to
matter, or is a finite jump/cusp ledger omitted?

## Exact work identity

Let

```text
E_center,a=(E_out,a+E_in,a)/2,
J_a=E_out,a-E_in,a.
```

For a straight segment leaving the knot into incident signs `sigma_a`, the
within-cell field is

```text
E_branch,a=E_center,a+sigma_a J_a/2.
```

If `sigma_a d_a=|d_a|`, exact Whitney current work is

```text
W_field = g q E_branch dot d,
W_center = g q E_center dot d,
W_cusp = W_field-W_center
       = (g q/2) sum_a J_a |d_a|.
```

This is also the exact quadratic matched-face energy loss: choosing the
current-scaled endpoint fields symmetrically about `E_branch`,

```text
E0=E_branch+gK/2,
E1=E_branch-gK/2,
```

gives

```text
[||E1||^2-||E0||^2]/2 = -g<K,E_branch> = -W_field.
```

## Locked tests

Use the FTD-0492 source/bias fixture, both polarities, all relevant translated
and cubic-rotated arms, and tolerance `1e-12`.

1. Build the exact FTD-0478 straight face current from the knot to the
   centered-bias endpoint.
2. Require `W_field-W_center=W_cusp` below `1e-12`.
3. Require the symmetric endpoint-field quadratic-energy identity below
   `1e-12` and exact continuity/Gauss transport below `1e-12`.
4. Require polarity mirror, integer translation, cyclic rotation, and reversed
   segment signs below `1e-12`.
5. With `J=0`, require zero omitted work.
6. Include a source-free jump control `sum_a J_a=0` with unequal component
   displacements. Require nonzero cusp work. This tests that the centered rule
   removes all incident-trace jump work, not only a locally identifiable charge
   self-field.

## Frozen verdicts

- `CENTERED_TRACE_CLOSES_EXISTING_ENERGY_LEDGER` only if the omitted work is
  below `1e-12` in every noncontrol arm.
- `CENTERED_TRACE_LEAVES_EXACT_CUSP_WORK_LEDGER` if the exact formula and field
  energy identity pass with nonzero omitted work.
- `IMPLEMENTATION_INVALID` if continuity, Gauss, energy, symmetry, or formula
  controls fail.

## Consequence

A nonzero cusp ledger cannot be discarded. It must enter matter energy, a
separate dressing/self-energy coordinate, or a modified field/action update.
The source-free control prevents calling the whole term "self-field energy"
without a provenance/decomposition rule. Any repair is a new dynamics cycle.

No production toggle or scenario is authorized.

Run-of-record test-source SHA256:
`6A0757328BF6B95C8691E117F0B3C96DFC1FE109EE2CABC0A3528796AC4E4875`.
