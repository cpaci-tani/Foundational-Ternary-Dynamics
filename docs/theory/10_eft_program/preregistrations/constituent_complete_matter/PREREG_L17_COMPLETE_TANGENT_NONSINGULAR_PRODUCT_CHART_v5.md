# FTD-0832 — L=17 complete tangent nonsingular product chart v5

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXECUTION INVALID AT INDEPENDENT REPLAY]`  
**Scope:** replacement of the singular electric-component codec ratio by a
declared direct-product chart norm, with explicit electric harmonic
coordinates  
**Physical question and gates:** inherited unchanged from FTD-0774  
**Production impact:** none  
**Date:** 2026-08-10

## 1. Registered question

FTD-0831 produced a complete, independently replayed execution-invalid
corpus. All `64/64` direct derivative codecs and `24/32` composition codecs
passed. The eight failures were the two composition orders of `p6`, `p7`,
`f_b`, and `p6+f_b`. Each has a legitimate zero electric input component.
The inherited codec divided an internal electric Hodge correction by the
norm of the equally small electric output component, although the complete
tangent direction was nonzero and its full composition residual passed.

This protocol asks:

> Does the locked FTD-0774 endpoint pass its full preflight, and what tangent
> verdict follows, when the electric harmonic is an explicit chart coordinate
> and codec errors are measured in one preregistered nonsingular norm on the
> complete tangent product?

This is a new chart selection. It is not licensed as uniquely forced, and it
is not a repair chosen from a set of denominators after observing which one
passes. Its justification is structural: a linear codec

\[
 C:X_{\rm matter}\oplus X_E\oplus X_B\longrightarrow X
\]

must be bounded on the declared domain norm. A component-relative quotient
is not a norm on this direct sum because it is undefined on the nonzero
subspace `X_matter + X_B` where `X_E=0`.

## 2. Frozen inheritance

Inherit unchanged:

- source commit `93748ac2021e4db5a9b8583cc28493332c716ac0` and all FTD-0774
  parent/compiled-closure hashes;
- the exact orientation-0 `L=17` representative, endpoint options, 16 probes,
  `h_0=2e-6`, `h_1=1e-6`, energy Hessian, positive energy form, root,
  cache, field-control, scale, composition, adjoint, Krylov, cluster,
  qualification, and verdict gates;
- FTD-0829 periodic-source compatibility and semantic 98-record order;
- FTD-0830 stable harmonic reinsertion arithmetic and fail-closed labels;
- FTD-0831 binary64 representability floor for comparing a completed field
  mean with its retained harmonic coordinate;
- the `2e-4` Hodge-correction and reconstruction thresholds and every later
  physical threshold.

No candidate, spectrum, phase, period, `G*` value, or prior pass/fail scalar
may enter the chart definition.

## 3. Chart C6 — explicit electric direct sum

Write the centered electric tangent uniquely as

\[
 \delta E=L(\delta x)+e_T+h_E,                              \tag{C6.1}
\]

where `L(delta x)` is the density-jet longitudinal solution, `e_T` is the
cleaned divergence-free, zero-mean face field, and
`h_E in R^3` is the uniform face harmonic. The `ChartVector` stores `e_T` and
`h_E` separately. Addition, scaling, retraction, the energy metric, and
serialization act on the completed field `e_T+h_E`; therefore C6 changes no
physical endpoint and no raw tangent dimension.

The completed-field mean is still audited against `h_E` with the frozen
FTD-0831 representability floor. The zero-mean transverse summand is audited
independently by its reconstruction and divergence residuals.

## 4. Norm N6 — complete direct-product codec norm

All entries below are dimensionless lattice chart coordinates. For the raw
centered tangent define

\[
 \|v\|_{X}^{2}
 =\|\delta x\|_2^2+\|\delta p\|_2^2
  +\|\delta E\|_2^2+\|\delta B\|_2^2.                     \tag{N6.1}
\]

Let `H_E` be the Hodge-cleaning field and

\[
 R_E=\delta E-\bigl(L(\delta x)+e_T+h_E\bigr).             \tag{N6.2}
\]

The registered codec residuals are

\[
 r_H=\frac{\|H_E\|_2}{\|v\|_X},\qquad
 r_R=\frac{\|R_E\|_2}{\|v\|_X}.                          \tag{N6.3}
\]

If `||v||_X=0`, a residual is exactly zero iff its numerator is exactly zero;
otherwise it is infinity. No numerical floor is inserted into (N6.3).

Retain the locked thresholds

\[
 r_H\le2\times10^{-4},\qquad r_R\le2\times10^{-4}.        \tag{N6.4}
\]

This norm is positive on every nonzero complete tangent, including the
registered zero-electric-component directions. It is a chart-conditioning
norm only. All physical scale, adjoint, composition, positivity, and Krylov
tests continue to use the inherited energy metric `K`.

The producer must serialize `complete_chart_norm`, the two numerators, and
the resulting residuals in each codec detail. The independent verifier must
recompute (N6.1)--(N6.4) from those primitives and must reject any use of the
old electric-component denominator.

## 5. Locked execution order

1. Verify protocol/source/parent closure and runtime options.
2. Verify the exact Hessian, gradient, seed metric, positive energy form, and
   isolated-field control.
3. Execute all 64 direct endpoint codecs, 32 compositions, and the zero
   control under C6/N6.
4. If any inherited preflight gate fails, write the complete invalid corpus
   and stop before Krylov.
5. Only if preflight passes, execute the inherited filtered block-Krylov
   construction and candidate qualification without changing any threshold.
6. Run the independent artifact replay before booking a verdict.

## 6. Source closure and corpus

| role | file |
|---|---|
| shared runner | `engine/tests/test_l17_complete_tangent_candidate.cpp` |
| v5 compile wrapper | `engine/tests/test_l17_complete_tangent_nonsingular_product_chart_v5.cpp` |
| shared test-only codec | `engine/tests/support/connected_moore_tangent_codec.h` |
| independent replay | `scripts/proofs/proof_l17_complete_tangent_candidate.py` |
| v5 replay wrapper | `scripts/proofs/proof_l17_complete_tangent_nonsingular_product_chart_v5.py` |

The final protocol SHA-256 must be embedded in producer and verifier before
execution. Result root:

```text
engine/results/ftd_0832/
```

Stem:

```text
ftd_0832_l17_complete_tangent_nonsingular_product_chart_v5
```

## 7. Outcome map and licensing boundary

- **Outcome A — execution invalid:** any source, schema, replay, chart, or
  inherited gate fails. No tangent or clock verdict.
- **Outcome B — valid preflight, no qualified tangent:** the registered
  endpoint has no qualified tangent in this Krylov/probe construction. This is
  scoped negative evidence for this endpoint/construction, not a universal
  local-clock no-go.
- **Outcome C — qualified tangent:** licenses only a separately preregistered
  localization/nonlinear-continuation campaign. It does not establish a
  bounded autonomous clock.

Finite-period recurrence, repeated phase gates, body-relative localization,
energy/work closure, and held-out orientation robustness remain mandatory
under FTD-0828. No `G*` claim follows from any v5 outcome.

## 8. Stop conditions

- Do not execute before the final protocol hash is embedded in producer and
  verifier.
- Do not change C6, N6, the exact-zero convention, or any threshold after
  execution.
- Do not compare alternative denominators on the new result.
- A producer/verifier disagreement is execution-invalid.
- No near-miss search, phase fit, threshold fit, dimension fit, or
  outcome-conditioned repair is permitted.

## 9. Recorded outcome

The source-pinned producer passed the complete preflight, executed `1920`
derivative evaluations and all four 64-dimensional final Krylov
constructions, found one eligible primary rank-four cluster, and reported
zero qualified candidates with producer verdict
`L17_FIRST_DOUBLET_TANGENT_SOLVE_UNRESOLVED`.

The independent replay did not certify that verdict. After the separately
locked FTD-0833/0834 scope repairs exposed the complete v5 replay, it passed
`94/95` checks and failed `candidate metric rows replay`: the producer stored
the primary-to-sign principal angle as `7.300048299977713e-08`, while the
independent binary-vector/K-metric replay returned `0.0`. The locked
cross-check tolerance was `2e-8`. Both values pass the physical sign-angle
gate `<=1e-6`, and the stored cross-Gram is `-I` to about `1e-15`, but the
preregistered replay equality still fails.

Therefore Outcome A applies. There is no tangent or clock verdict. No replay
tolerance or angle formula is changed after observing the mismatch.
