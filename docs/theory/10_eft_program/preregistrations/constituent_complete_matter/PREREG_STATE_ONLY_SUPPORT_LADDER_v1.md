# FTD-0754C — State-only nested-support projection ladder v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXISTING-CORPUS REPLAY]`  
**Date:** 2026-07-30  
**Parents:** FTD-0754 state-only observer discovery; FTD-0754B boundary-energy
ledger  
**Scope:** analytic and observer-only replay of the already-seen FTD-0753/0754
histories; no new state, perturbation, volume, direction, tick, or FTD-0755
validation datum

## 1. Question

Does the selected finite-support Gauss dressing behave coherently when its
support is treated as an observer resolution scale rather than a material
boundary?

The registered test asks whether the minimum-energy dressings on three nested
cubic supports satisfy the exact Hilbert-space projection law. It does not ask
which radius is the ontic edge of matter and may not promote any radius to that
status.

## 2. Frozen ladder and identity

For the same complete instantaneous state `X` used by FTD-0754, let `b_R` be
the zero-boundary-crossing minimum-energy primitive face field with the
instantaneous pair density as Gauss source on support half-width `R`. Freeze

```text
R in {4,6,8}
```

in that order. No additional radius may be inserted after replay.

For nested supports `R<S`, the zero extension of `b_R` is feasible for the
`S` problem. Both have the same divergence, so `d=b_R-b_S` is divergence-free
on the larger support. Minimum-norm orthogonality gives

\[
\langle b_S,d\rangle=0,
\qquad
U_R=U_S+\tfrac12\lVert d\rVert^2,
\qquad U_R=\tfrac12\lVert b_R\rVert^2.
\]

For the actual primitive electric field `E`, every scale must also satisfy

\[
\tfrac12\lVert E\rVert^2
=U_R+\tfrac12\lVert E-b_R\rVert^2
 +\langle b_R,E-b_R\rangle.
\]

These are exact finite-dimensional identities conditional on the declared
support problems. They imply monotone nonincreasing `U_R`; they do not imply
that `U_R`, the residual, or their cross term is energy intrinsically owned by
matter.

## 3. Frozen corpus

Replay exactly the registered periodic `L=321` face, edge, and body histories
from FTD-0753/0754:

- directions `(0,0,1)`, `(0,1,-1)`, `(1,1,1)`;
- plus-minus pair, separation `1.30`, inward momentum `0.0120`;
- ticks `0..312`, with ladder observations only at
  `{0,80,96,115,160,240,297,312}`;
- the same explicit-rounding ordered WSL2 CUDA field path and unchanged
  selected common action.

Every original scalar row must replay byte-for-byte. The ladder therefore has
exactly `3 arms x 8 ticks x 3 scales = 72` scale rows and `48` adjacent-support
transition rows. Replaying these known histories supplies no FTD-0755
validation evidence.

## 4. Frozen gates

The addendum passes only if:

1. every original scalar row replays exactly;
2. all 72 support preparations are valid, density-contained, Gauss-compatible,
   compact, and zero-boundary-crossing;
3. primitive actual/bound/residual/cross reconstruction is at most `1e-12`
   relative scale on every row;
4. every adjacent support has nonnegative energy drop to `1e-12` relative
   scale;
5. both `|<b_S,b_R-b_S>|` and the Pythagorean residual are at most `1e-12`
   relative scale on all 48 transitions;
6. the existing translation, proper-cubic, and polarity-conjugation unit tests
   pass for the complete ladder scalars;
7. serial/parallel observer evaluation changes no recorded value.

Failure is retained as evidence that the finite-support family is not a
coherent nested projection at the declared tolerance. No radius, tolerance,
solver, history, or verdict may be altered under this identifier after output
is inspected.

## 5. Frozen implementation before replay

- interface candidate:
  `F180DAE14DF62244E9F091F68670EA1EEA192881D87BAE86D43BE633C09CC696`;
- implementation candidate:
  `10BF768DC480C5A0699A18B097E44AC685A27D13BF2C90C95758EC1FF3D3FB2F`;
- unlocked runner candidate:
  `21B96151C5207814C002C6A76AE5BEFD08B9DC4D83081E5AA22E458341287817`.

Registered output is restricted to
`engine/results/ftd_0754_support_ladder/`. The output manifest must say
`held_out_validation_consumed=false` and `dynamics_changed=false`.

## 6. Consequence map

- Pass: the support radius is retained as a coherent resolution scale for
  FTD-0755, and the matter predicate must be support-independent while energy
  ledgers run with scale.
- Fail: FTD-0755 may retain only the single selected radius-four observer and
  must classify support dependence as an unresolved instrument defect; no
  matter boundary or scale-flow claim advances.

Either result leaves M3, particle identity, autonomous motion, charge, poles,
unitarity, and Lorentz recovery open. Production, established CUDA, scenarios,
and ontology remain unchanged.
