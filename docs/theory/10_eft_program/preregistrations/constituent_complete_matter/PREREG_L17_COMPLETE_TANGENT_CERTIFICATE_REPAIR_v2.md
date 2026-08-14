# FTD-0829 — L=17 complete tangent certificate repair v2

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE SUCCESSOR EXECUTION]`  
**Scope:** target-blind repair of two FTD-0774 execution-certificate defects  
**Physical question and gates:** inherited unchanged from FTD-0774  
**Production impact:** none  
**Date:** 2026-08-10

## 1. Why a successor is required

The first clean reconstruction of the locked FTD-0774 runner completed all
`64/64` signed endpoint probes but returned
`L17_COMPLETE_TANGENT_EXECUTION_INVALID` before Krylov construction. Independent
replay also rejected the preflight derivative ledger. The run therefore has no
physics verdict and cannot be repaired by reinterpreting its artifacts.

Two implementation defects have been isolated:

1. the Hodge source is the periodic discrete divergence `g=D r`, whose exact
   lattice sum vanishes by telescoping, but the generic Poisson helper divided
   its floating mean by `||g||_inf`; when `g` was already near zero this ratio
   became singular as a numerical diagnostic and rejected the chart before the
   registered Hodge correction was attempted;
2. the producer recorded the 98 preflight groups in the registered semantic
   order and then rewrote them in lexicographic metadata order, while the
   independent certificate correctly required the registered semantic order.

These are certificate/instrument failures, not evidence for or against a
complete matter--field tangent clock. FTD-0774 remains preserved as
`[EXECUTION INVALID — NO PHYSICS VERDICT]`. This successor is a new execution,
not a post-hoc amendment of FTD-0774.

## 2. Frozen physical inheritance

Except for the two repairs in sections 3 and 4, inherit every byte-level
parent, representative, selected endpoint option, chart coordinate, mode,
probe, energy form, finite-difference scale, regularity check, cache control,
field control, Krylov construction, cluster rule, held-out qualification gate,
artifact requirement, decision order, and stop condition from
[`PREREG_L17_COMPLETE_TANGENT_CANDIDATE_v1.md`](PREREG_L17_COMPLETE_TANGENT_CANDIDATE_v1.md).

In particular:

- source commit: `93748ac2021e4db5a9b8583cc28493332c716ac0` for
  `engine/include/ftd/` and `engine/src/eft/`;
- `L=17`, orientation `0`, the FTD-0638/0639 representative, and FTD-0640
  modes `6,7`;
- `h_0=2e-6`, `h_1=1e-6`, and `h_E=2e-4`;
- the complete forward/reverse selected common-action endpoint;
- all FTD-0774 numerical thresholds;
- the four ordered FTD-0774 verdicts and their meanings.

No physical threshold is relaxed. No target phase, candidate dimension,
cluster, or outcome from the invalid run may enter either repair.

## 3. Repair R1 — range-aware periodic Hodge compatibility

Let `r` be the zero-harmonic face-field residue already defined in FTD-0774
section 3 and let

\[
 g=D r .
\]

On the periodic `L^3` lattice, with the registered matched-face divergence,

\[
 \sum_x (D r)_x=0                                             \tag{R1}
\]

exactly: every oriented face contribution occurs once with each sign. Thus
`g` is analytically in the zero-mean range required by the periodic Poisson
operator. A nonzero computed mean

\[
 m=\frac1{L^3}\sum_x g_x
\]

is a floating representation error of this exact identity, not an independent
physical source-compatibility observable.

The v2 codec must:

1. form `g` by the unchanged periodic divergence;
2. accumulate `m` in `long double` and record `|m|`;
3. compute the target-blind backward-error ratio

   \[
   \eta_{\rm range}=\frac{|m|}{\max(\|r\|_\infty,10^{-30})};    \tag{R2}
   \]

4. require `eta_range <= 1e-13`;
5. replace `g` by `g-m` before the unchanged source-normalized Poisson solve;
6. retain unchanged the Poisson relative-residual, pre-clean divergence,
   cleaned divergence, Hodge-correction, reconstruction, face-harmonic, and
   edge-harmonic gates from FTD-0774.

The existing artifact column `hodge_source_mean_rel` carries `eta_range` in
this v2 corpus. Its name is retained only to avoid a schema-wide unrelated
change; the protocol-defined denominator is now `max(||r||_inf,1e-30)`.

The generic Poisson route, including the analytic density-tangent source,
retains its original `|m|/||source||_inf <= 1e-13` check. Range-aware
projection is licensed only at the call site where the source is constructed
as the periodic divergence of the recorded residue. A freely supplied source
cannot opt into this repair.

This repair is not a tolerance fit. It replaces an ill-conditioned relative
diagnostic by a backward error referenced to the parent object whose exact
algebraic identity is being represented.

## 4. Repair R2 — semantic preflight ledger order

The 98 derivative bundles must be serialized in exactly this order:

1. for `h` in `(h_0,h_1)`, for probes in the registered 16-probe order, then
   directions `(forward,reverse)`;
2. for columns `0..15`,
   `(reverse_forward,forward_reverse)`;
3. `(zero forward,zero reverse)`;
4. terminal group ID `98`.

The producer must preserve the reservation order for this preflight ledger.
It must not lexicographically resort and renumber the groups. The independent
certificate must reconstruct the same exact 98-key coverage and order.

The post-preflight execution ledger retains its existing deterministic
metadata ordering because FTD-0774 did not impose a stronger semantic order
there; its verifier continues to require zero-based contiguous IDs and exact
forward/reverse coordinate pairs.

## 5. Frozen source and artifact identities

The following files form the v2 test-only execution closure:

| role | file |
|---|---|
| shared complete runner | `engine/tests/test_l17_complete_tangent_candidate.cpp` |
| v2 compile wrapper | `engine/tests/test_l17_complete_tangent_certificate_repair_v2.cpp` |
| shared test-only codec | `engine/tests/support/connected_moore_tangent_codec.h` |
| independent replay implementation | `scripts/proofs/proof_l17_complete_tangent_candidate.py` |
| v2 replay wrapper | `scripts/proofs/proof_l17_complete_tangent_certificate_repair_v2.py` |

The final SHA-256 of this protocol must be embedded in both the producer and
independent certificate before execution. The producer's result manifest must
then hash every file in the table, and the independent certificate must verify
those hashes against its executing checkout. This avoids a circular lock in
which a protocol contains the hash of a runner that itself embeds the protocol
hash, while still making any post-execution source change invalidate replay.

The result root is `engine/results/ftd_0829/`; the stem is
`ftd_0829_l17_complete_tangent_certificate_repair_v2`.

The corpus must contain all FTD-0774 primitive artifacts plus hashes for this
protocol, the shared runner, v2 compile wrapper, shared codec, and embedded
compiled closure. The Python certificate must fail closed on a missing file,
hash mismatch, schema mismatch, ordering mismatch, or unreconstructed scalar.

## 6. Ordered verdict and interpretation

Use the unchanged FTD-0774 ordered verdict map. The JSON identity is
`FTD-0829`, but the physical verdict strings remain the FTD-0774 strings so
their already-defined semantics are not silently forked.

- `L17_COMPLETE_TANGENT_EXECUTION_INVALID`: the repaired execution or
  certificate still fails before a valid solve.
- `L17_FIRST_DOUBLET_TANGENT_SOLVE_UNRESOLVED`: execution is valid but the
  finite construction does not resolve an eligible candidate.
- `L17_FIRST_DOUBLET_LOCKED_CANDIDATES_NOT_QUALIFIED`: candidates resolve but
  none passes every held-out gate.
- `L17_FIRST_DOUBLET_POSITIVE_TANGENT_CANDIDATE_CONSTRUCTIVE`: at least one
  complete-state tangent candidate passes every locked gate.

Only the constructive verdict licenses a fresh, separately preregistered
localization/volume successor. Even a constructive tangent result is not yet a
bounded autonomous clock: localization, nonlinear recurrence, repeated gate
crossings, energy/work closure, and held-out orientation stability remain
required by FTD-0828.

## 7. Stop conditions

- Do not execute until the final protocol SHA-256 is embedded in both producer
  and certificate and both agree with the file on disk.
- Do not modify this protocol after its final SHA-256 is embedded in the
  producer and certificate.
- If R1 fails `eta_range`, preserve the failure; do not rescale its denominator
  or tolerance.
- If any unchanged FTD-0774 gate fails, apply the unchanged ordered verdict.
- If producer and independent certificate disagree, the corpus is
  execution-invalid.
- No numerical near-miss search, phase fitting, candidate-dimension fitting,
  threshold fitting, or outcome-conditioned repair is permitted.
