# FTD-0830 — L=17 complete tangent harmonic-reinsertion repair v3

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE SUCCESSOR EXECUTION]`  
**Scope:** target-blind completion of the FTD-0829 tangent-chart certificate  
**Physical question and gates:** inherited unchanged from FTD-0774  
**Production impact:** none  
**Date:** 2026-08-10

## 1. Registered reason for a third execution

FTD-0829 repaired the singular Hodge compatibility normalization and the
preflight ledger ordering. Its clean execution completed all `64/64` signed
probe endpoints. On the 64 primary probe codecs:

- every finite, Gauss, Hodge-correction, reconstruction, source-compatibility,
  and Poisson-residual gate passed;
- all eight pure uniform-harmonic controls passed;
- the other 56 rows failed only `harmonic_face <= 1e-12`;
- Krylov construction was not entered.

The stored primitive rows show why. The codec first removes the uniform face
mean, then reinserts the separately retained raw harmonic coordinate, and only
then measures the mean. Binary64 rounding in the reinsertion changes a raw
coefficient near zero by a tiny absolute amount but a large relative amount.
The existing helper iterates mean removal before reinsertion, not correction to
the retained coordinate after reinsertion.

The FTD-0829 independent certificate also raised a diagnostic `KeyError` while
formatting a missing-field error for an execution-status row, because that row
has `operation` rather than `probe`. That is a verifier reporting defect; it
does not alter a numerical predicate.

FTD-0829 is therefore preserved as
`[EXECUTION INVALID — NO PHYSICS VERDICT]`. This v3 protocol is locked before
its own execution and does not reinterpret either earlier corpus.

## 2. Frozen inheritance

Inherit all FTD-0774 physical inputs, gates, thresholds, construction rules,
verdicts, and stop conditions through the two FTD-0829 repairs. In particular,
retain:

- the same source commit, parent hashes, `L=17` representative, modes, probes,
  endpoint, chart, and energy form;
- `h_0=2e-6`, `h_1=1e-6`, and `h_E=2e-4`;
- FTD-0829's range-aware Hodge ratio
  `|mean(D r)|/max(||r||_inf,1e-30) <= 1e-13`;
- FTD-0829's exact 98-group semantic serialization order;
- every FTD-0774 face/edge harmonic threshold, including
  `harmonic_face <= 1e-12` and `harmonic_edge <= 1e-12`.

No observed phase, candidate dimension, cluster, or later-stage outcome is
available to this repair because neither prior execution entered Krylov.

## 3. Repair R3 — stable reinsertion of the retained face harmonic

Let `e_0` be the Hodge-cleaned zero-harmonic face field and let
`a=(a_x,a_y,a_z)` be the three raw face means already retained by the locked
codec. Form the initial completed field by

\[
 e^{(0)}_{i,k}=e_{0,i,k}+a_k .                              \tag{R3.1}
\]

For exactly four correction passes, compute the component means in
`long double`,

\[
 \bar e_k^{(j)}=L^{-3}\sum_i e^{(j)}_{i,k},
\]

and update every component uniformly,

\[
 e^{(j+1)}_{i,k}=e^{(j)}_{i,k}+a_k-\bar e_k^{(j)},
 \qquad j=0,1,2,3.                                         \tag{R3.2}
\]

Then measure the unchanged relative reconstruction residual between `a` and
the means of `e^(4)`. The operation acts only on the three explicitly retained
uniform coordinates. It does not alter the zero-harmonic field, source,
Poisson solve, physical endpoint, tolerance, or target. Four passes are frozen
before execution to mirror the already-registered four-pass zero-mean helper.

If any face-harmonic row still exceeds `1e-12`, preserve the failure. Do not
add passes or change the denominator/tolerance after inspection.

## 4. Repair R4 — fail-closed verifier row labeling

The independent certificate's helper for a required numeric field must label
an error row by the first available key in this order:

1. `probe`;
2. `operation`;
3. `record_kind`;
4. `?`.

It must continue to raise the same `CertificateError` for a missing field. No
numerical parsing, predicate, tolerance, ordering rule, or verdict rule changes.

## 5. Source closure and corpus

The v3 test-only source closure is:

| role | file |
|---|---|
| shared complete runner | `engine/tests/test_l17_complete_tangent_candidate.cpp` |
| v3 compile wrapper | `engine/tests/test_l17_complete_tangent_harmonic_reinsertion_repair_v3.cpp` |
| shared test-only codec | `engine/tests/support/connected_moore_tangent_codec.h` |
| independent replay implementation | `scripts/proofs/proof_l17_complete_tangent_candidate.py` |
| v3 replay wrapper | `scripts/proofs/proof_l17_complete_tangent_harmonic_reinsertion_repair_v3.py` |

The final SHA-256 of this protocol must be embedded in producer and verifier
before execution. The result manifest must hash every source above against the
executing checkout. The result root is `engine/results/ftd_0830/` with stem
`ftd_0830_l17_complete_tangent_harmonic_reinsertion_repair_v3`.

## 6. Verdict and licensing boundary

Use the unchanged four FTD-0774 physical verdict strings under JSON identity
`FTD-0830`. Producer and independent replay must agree exactly.

Only `L17_FIRST_DOUBLET_POSITIVE_TANGENT_CANDIDATE_CONSTRUCTIVE` licenses a
new, separately preregistered localization/volume test. It still does not
establish a bounded autonomous clock: nonlinear recurrence, repeated gate
crossings, energy/work closure, and held-out orientation stability remain
required by FTD-0828.

## 7. Stop conditions

- Do not execute before the final protocol hash is embedded in producer and
  verifier.
- Do not change this protocol after hashing.
- Do not alter the four-pass count or any inherited threshold after execution.
- A source/hash/schema/replay disagreement is execution-invalid.
- No near-miss search, phase fit, threshold fit, dimension fit, or
  outcome-conditioned repair is permitted.
