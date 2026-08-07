# AUDIT — Diagonal endpoint action domain

**Date:** 2026-07-25  
**Identifier:** `FTD-0532`  
**Status:** `[DERIVED + MEASURED — SIMULTANEOUS MULTI-PLANE HOP]` +
`[CLOSED NEGATIVE — COMPACT FTD-0485 COMPOSITION]` +
`[RESOLVED BY FTD-0533 — GLOBAL INTERNAL-KNOT VARIATION]`  
**Verdict:**
`ENERGY_ENDPOINT_CONSTRUCTIVE_COMPACT_COMMON_ACTION_OUT_OF_DOMAIN_AT_HOP`  
**Pre-registration:**
[`PREREG_DIAGONAL_ENDPOINT_ACTION_DOMAIN_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_DIAGONAL_ENDPOINT_ACTION_DOMAIN_v1.md)  
**Run of record:** `engine/results/ftd_0532/windows_msvc_cpu.json`

## 1. Composition question

FTD-0531 constructs a reciprocal scalar matter/field endpoint on every
symmetric edge and corner arm. FTD-0485 derives a full three-component
interaction impulse, but only when each adjacent worldline segment lies in one
closed spatial cell. FTD-0532 asks whether the constructive endpoint is inside
that compact action domain at the actual chart-horizon tick.

For each carrier, let `x_0` be its FTD-0527 bounced preimage and let

```text
x_1=x_0+u d(p_1)
```

be the FTD-0531 endpoint. The signed Moore-normal components have equal
magnitude. Therefore every active coordinate reaches its integer plane at the
same segment parameter

```text
tau_a=(m_a-x_0,a)/(x_1,a-x_0,a).
```

An edge normal has two active coordinates and a corner normal has three. Once
the endpoint overshoots the planes, the compact segment cannot be contained in
one unit cell.

## 2. Registered result

All 240 preregistered arms retain valid FTD-0531 roots. The 144 edge arms cross
exactly two planes per carrier; the 96 corner arms cross exactly three. The
active crossing parameters agree within `2.3536728122053319e-14` and stay in
the open interval

```text
0.4625442015143354 <= tau <= 0.9241598995731553.
```

The original free endpoint already overshoots by at least
`0.005181485540921571`. Coupling shifts the endpoint by
`0.0005468927379653765..0.0008907723858042060` and leaves a minimum final
overshoot `0.005497234210404045`. The field-dependent endpoint solve therefore
does not retreat into the one-cell interior.

## 3. Rejection is purely geometric

Each carrier was supplied a nonzero causal previous segment of length
`0.3017766952966366..0.4040063509461102`, followed by the coupled endpoint.
The auxiliary connection was identically zero and the two slabs joined
exactly.

The decisive control is paired:

- all `480` previous-segment calls with a stationary next segment are accepted
  by FTD-0485;
- all `480` calls with the coupled diagonal next segment are rejected.

The charge, coupling, slab sizes, temporal scales, field values, and connection
join are identical between the two calls. Only the next endpoint changes.
Thus rejection comes from the frozen `lies_in_cell` domain guard, not from a
field singularity, failed gauge join, superluminal control, or algebraic force
failure.

Meanwhile the inherited FTD-0531 continuity, Gauss, work, energy, causality,
and inverse residuals remain at most `6.253350701840299e-13`.

## 4. Correct scope

This result does **not** refute the FTD-0484 worldline interaction or the
FTD-0531 energy transaction. It proves that the current compact FTD-0485
variation cannot be composed with the first constructive diagonal hop.

FTD-0533 corrects the first sentence of the original inference: the multi-cell
action was already present in FTD-0484. Its complete deposited-action gradient
converges uniquely through these **internal** simultaneous knots. What remains
missing is composition of that vector gradient with the FTD-0531 dynamical
field history. FTD-0485/0487 still forbid treating a **varied endpoint** on a
charged threshold as the same problem.

Accordingly the full-vector stationarity gate remains open. FTD-0531 cannot
be promoted to a default-off engine toggle, mobile-matter branch, scenario, or
arbitrary-field collision law.

No production code, default, toggle, scenario, force, collision law, phase
order, field ontology, normalization, or tolerance changed.

## 5. Reproducibility

- checks: `8/8 PASS` over `240` arms and `960` action-domain calls;
- test SHA256:
  `AF0FF799C6629A97BC558E142D25C9C21A2545B755C672308BA9E4D871948679`;
- header SHA256:
  `68BBA3E493CCDB94181164615758FFF55948C05220FE606D72C5155E616D022A`;
- implementation SHA256:
  `AEA3CA8DC106B7847017A833817C10744DAD5243D962A8EA5E53833C5656ED8D`;
- locked preregistration SHA256:
  `CE71957C571052FD5BE4A6A29ADB283D8F0E71D75275E8D7604DB3CBDC83B3F7`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
