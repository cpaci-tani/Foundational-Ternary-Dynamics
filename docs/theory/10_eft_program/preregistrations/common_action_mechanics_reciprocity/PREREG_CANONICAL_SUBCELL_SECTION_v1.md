# PRE-REGISTRATION — Canonical subcell section v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0500`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0497`, `FTD-0498`, `FTD-0499`

## Question

Can the raw inverse defect be removed by selecting one centered canonical
`(site,remainder)` chart for every effective position, while preserving exact
integer-translation covariance, cubic covariance, the exact face observables,
and existing production hop/source/collision behavior?

## Locked canonical section

For each coordinate define

```text
a(x)=floor(x+1/2),
r(x)=x-a(x) in [-1/2,1/2).
```

Apply this componentwise. This is the unique nearest-site chart away from
half-integer tie planes; the displayed half-open convention resolves ties.
Do not fit or change the convention after execution.

## Locked algebraic gates

1. Verify uniqueness, position reproduction, and canonical reversal for a
   translated rational grid.
2. Verify exact integer-translation covariance for shifts in `[-3,3]^3`.
3. Verify all 48 signed cubic permutations away from tie planes.
4. At `x=1/2`, test the exact one-dimensional obstruction. Translation
   covariance and inversion covariance would require

   ```text
   a(-1/2)=a(1/2)-1=-a(1/2),
   2a(1/2)=1,
   ```

   which has no integer solution. Record the selected convention's raw
   inversion mismatch and confirm that physical position still transforms.
5. Compare trilinear polarity and exact face current between the canonical
   chart and every overlapping FTD-0498 chart of the same generic endpoints.

## Locked production discriminators

Use a positive x trajectory from `(site_x=8,remainder_x=0.49)` with
displacement `0.02`:

- the frozen ±1 threshold must retain anchor 8;
- the centered canonical section must select anchor 9;
- both must reproduce physical endpoint `8.51`.

Instantiate those equivalent outputs as otherwise identical production
states. Measure primitive-state L1 difference, exact native
`-G_C grad(s)` source response, and the same incoming-probe collision used by
FTD-0498. No force, tolerance, or ontology change is permitted.

## Frozen verdicts

- `CANONICAL_CHART_PRESERVES_FROZEN_PRODUCTION` only if the section is unique,
  exactly translation/cubic covariant including ties, face-equivalent, and
  production-indistinguishable.
- `CANONICAL_CHART_REQUIRES_RULE_REWRITE` if it repairs raw reversal and keeps
  quotient observables but necessarily breaks an exact tie symmetry or changes
  hop/source/collision behavior.
- `CANONICAL_SECTION_INVALID` if it fails uniqueness, position reproduction,
  translation covariance, or reversal even away from tie planes.

## Scope ceiling

This is observer-only. It does not authorize changing the production threshold,
site-local matter rules, collision algebra, a toggle, or a scenario. A
measure-zero description is not accepted as an exact discrete-covariance proof;
half-cell states are reachable engine states.

## Run-of-record outcome

The pre-registered raw reversal gate failed in `91` locked grid arms, all at
reachable half-cell ties. No off-tie arm failed, and physical position reversed
within `1.12e-16`. This failure is retained in the result record.

## Run-of-record hashes

- test SHA256:
  `A045BF7A69E23A8231C9F66B53AEF65C55D0147CE770DA3C7386FBD060A375AC`;
- header SHA256:
  `8DBA6784C6B0D61B5A78430EB6A5949F215AFCD1C635B67BAF05F2B94595B42F`;
- implementation SHA256:
  `248B0F6309EBF9B0324E61E63592422F7509DC406F611D271E3EC114F35E89FD`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- record: `engine/results/ftd_0500/windows_msvc_cpu.json`.
