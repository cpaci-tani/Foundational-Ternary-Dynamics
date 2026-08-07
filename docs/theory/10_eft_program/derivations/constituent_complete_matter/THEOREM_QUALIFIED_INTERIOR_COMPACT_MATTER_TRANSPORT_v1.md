# FTD-0608 — Qualified-interior compact matter transport v1

**Status:** `[MEASURED — CLEAN PRE-BOUNDARY EVOLUTION]` +
`[MECHANISM DIAGNOSIS — STRICT ANCHOR ALIAS]` +
`[NUMERICALLY UNRESOLVED — COMPLETE TRANSPORT]`
**Protocol:**
[`PREREG_QUALIFIED_INTERIOR_COMPACT_MATTER_TRANSPORT_v1.md`](../../preregistrations/constituent_complete_matter/PREREG_QUALIFIED_INTERIOR_COMPACT_MATTER_TRANSPORT_v1.md),
prefix SHA-256 `B64BB90EF082EC8E47BE83BA1F9951D7B30C3C5904AE8E4C639B33543020C5E0`
**Production status:** unchanged

## 1. Locked launch state

Phase `15/32` was selected before motion because FTD-0607 identified it as
the lowest-energy member of its five qualified site-interior cores. The
24-start search reproduces that state exactly:

- energy `0.0031781023845096961`, fingerprint residual zero;
- 23/24 terminated starts and four best-energy cluster members;
- chart margin `0.0062729052`;
- gradient infinity norm `9.0895653e-8`;
- minimum tangent eigenvalue `0.0017915509`, with six positive modes;
- direct-field gate `8.2307208e-16`.

Integer-translation covariance of the first step is exact in the stored
record.

## 2. Pre-boundary dynamics

The `v=1/64` arm completes four forward steps; the `v=1/32` arm completes two.
Across all six completed steps, the worst common-action residual is below
`4e-15`, maximum total-energy drift is `2.22e-16`, internal trimer distances
remain near `sqrt(2)`, and no duplicate anchor exists in a stored valid state.

The next solve fails before Newton iteration zero. In both arms:

- the input state is valid and has distinct anchors;
- the solver is attempted but its initial candidate is inadmissible;
- the free-transport predictor produces exactly one duplicate-anchor pair;
- the nominal boundary time satisfies `tick * velocity = 1/16` cell.

Thus the two velocity arms encounter the same site-chart event. This is not a
convergence slowdown: the strict candidate domain rejects the initial
worldline continuation before a residual can be formed.

## 3. Verdict and scope

The locked verdict is

```text
QUALIFIED_INTERIOR_COMPACT_MATTER_NUMERICALLY_UNRESOLVED
```

Solver coverage is incomplete, so the result cannot be promoted to
`COMPACT_TRANSPORT_CLOSED_NEGATIVE`. It proves neither that the underlying
common-action equations lack a continuation nor that compact matter cannot
move. It establishes that the one-record-per-anchor chart cannot represent
this candidate's next local configuration.

The minimal next discriminator is explicit and falsifiable: allow two
distinct constituent records to share one integer chart anchor while keeping
the current, field, action, energy, and inverse equations unchanged. That is
an ontic-extension test, not a reinterpretation of this run.

