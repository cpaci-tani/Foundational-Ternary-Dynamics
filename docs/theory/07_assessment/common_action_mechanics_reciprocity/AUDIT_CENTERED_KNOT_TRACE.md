# AUDIT — Centered knot trace

**Date:** 2026-07-25  
**Identifier:** `FTD-0492`  
**Status:** `[THEOREM — UNIQUE LOCAL LINEAR CUBIC TRACE]` +
`[CONSTRUCTIVE — SELF-TRACE CANCELLATION]` +
`[CLOSED NEGATIVE — ORDINARY BRANCH-ACTION DERIVATIVE]`  
**Verdict:** `CENTERED_TRACE_UNIQUE_LOCAL_BUT_NOT_BRANCH_ACTION_DERIVATIVE`  
**Pre-registration:**
[`PREREG_CENTERED_KNOT_TRACE_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_CENTERED_KNOT_TRACE_v1.md)  
**Run of record:** `engine/results/ftd_0492/windows_msvc_cpu.json`

## 1. Unique local weak trace

The eight incident cells at a site form one orbit under the cubic group. Any
field-independent linear trace

```text
T(F)=sum_sigma w_sigma F_sigma
```

that is cubic invariant has equal weights. Constant reproduction fixes
`sum w=1`, hence uniquely `w_sigma=1/8`.

For matched face electric variables this reduces exactly to

```text
E_center,a(i)=[E_a(i)+E_a(i-e_a)]/2,
rho(i)=sum_a[E_a(i)-E_a(i-e_a)].
```

The centered trace and Gauss source occupy the sum and difference channels.
All translation, cyclic-rotation, and reflection fixtures close exactly.

## 2. Local self-trace cancellation

For the FTD-0491 radial source plus uniform bias,

```text
E_out,a = E_bias,a+q/6,
E_in,a  = E_bias,a-q/6.
```

Therefore `E_center=E_bias` while `D E=q`. Both polarities close with Gauss
residual `2.22e-16`. This cancellation is local and does not require a global
Hodge solve or per-source provenance.

## 3. It is not the ordinary branch derivative

The centered rule would send zero initial momentum to the endpoint determined
by

```text
p_center=(g q lambda/2)E_bias.
```

The ordinary outgoing-cell action still contains the one-sided Gauss trace and
returns

```text
P0_branch=-(g lambda/12)sigma.
```

The measured component mismatch is

```text
0.03512214137570327,
```

against the exact prediction

```text
g C_SPEED/12 = 0.03512214137570223.
```

The formula residual is `1.04e-15`; the mismatch remains gauge invariant to
`4.16e-17`.

Thus the centered trace is a valid selected generalized/weak derivative, but
not the ordinary derivative of the fixed-history FTD-0484 action. It cannot be
inserted while retaining the claim that one unchanged action generated both
the current and matter endpoint.

## 4. Consequence

The result refines the branch obstruction:

- a unique local deterministic cubic-covariant trace exists;
- it cancels the symmetric knot self-force without source provenance;
- adopting it changes the knot variational rule;
- exact finite work, field-energy exchange, and reversal must be rederived.

The next audit isolates the omitted work. No production toggle, scenario, or
IR claim is licensed.

## 5. Reproducibility

- checks: `10/10 PASS`;
- test SHA256:
  `2302E4A48E7755173DC172477E7AF10DB7326AD2EA89CB9F45D8FB66D1BBEEF3`;
- header SHA256:
  `39397FC93665218DB42A36B627C4FD363B71B9CD065E3D3ED4B9702D5A5283E9`;
- implementation SHA256:
  `49F922E9778F96B33041D2A46DD93F4D3AEB722D2E1CCA96676A32179EFA1976`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state: unchanged.
