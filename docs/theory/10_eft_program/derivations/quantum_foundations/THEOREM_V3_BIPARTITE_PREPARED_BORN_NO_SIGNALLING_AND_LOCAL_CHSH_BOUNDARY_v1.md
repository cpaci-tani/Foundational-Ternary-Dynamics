# V3 bipartite prepared Born no-signalling and local CHSH boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM, CONDITIONAL ON PREPARED SOURCE-COMPLETE LOCAL BANKS —
EXACT BIPARTITE BORN COUNTS AND OPERATIONAL NO-SIGNALLING]** +
**[THEOREM — NATIVE COMMON-SOURCE LOCAL CHSH CEILING TWO]** +
**[BOUNDARY — UNCOUPLED CONTEXT-SPECIFIC PREPARATIONS DO NOT DEFINE ONE CHSH
JOINT MODEL]** + **[OPEN — NATIVE FORMATION, AMPLIFICATION, AND LABORATORY
BELL-CORRELATION RECOVERY]**  
**Single-apparatus parent:**
[`THEOREM_V3_CONTEXTUAL_NEUTRAL_POINTER_BORN_RENEWAL_APPARATUS_v1.md`](THEOREM_V3_CONTEXTUAL_NEUTRAL_POINTER_BORN_RENEWAL_APPARATUS_v1.md)  
**Exact certificate:**
[`proof_v3_bipartite_born_no_signalling_local_ceiling.py`](../../../../../scripts/proofs/proof_v3_bipartite_born_no_signalling_local_ceiling.py)

---

## 1. Finite bipartite history counts

Let a finite source record have sector `lambda` with setting-independent
multiplicity `R_lambda`. At wing A, local setting `a` and outcome `o` have a
prepared v3 residual-history count

\[
 A_{\lambda a}(o)=|Z^A_{\lambda a o}|^2,
\]

and similarly

\[
 B_{\lambda b}(p)=|Z^B_{\lambda b p}|^2.
\]

Every `Z` is the Gaussian-integer readout of finite C4 field records, not an
ontic complex amplitude. The prepared joint cardinality is

\[
 \boxed{
 N_{ab}(o,p)=\sum_\lambda R_\lambda
 A_{\lambda a}(o)B_{\lambda b}(p).}           \tag{1}
\]

The certificate uses nonuniform local weights `(4,1)`, realized exactly by C4
counts `(2,0,0,0)` and `(1,0,0,0)`. Every local source/setting block has total
cardinality five. A deterministic enumeration of source copies and local
history slots reproduces all four context tables exactly, with 100 joint
events per context.

---

## 2. Exact no-signalling condition

Assume **source-complete local cardinality**:

\[
 \sum_o A_{\lambda a}(o)=K_A,
 \qquad
 \sum_p B_{\lambda b}(p)=K_B,                 \tag{2}
\]

where `K_A,K_B` do not depend on the remote setting or source sector. Then

\[
 \sum_p N_{ab}(o,p)
 =K_B\sum_\lambda R_\lambda A_{\lambda a}(o), \tag{3}
\]

and the total is `K_A K_B sum R_lambda`. Hence

\[
 \boxed{
 P_A(o\mid a,b)
 ={\sum_\lambda R_\lambda A_{\lambda a}(o)
   \over K_A\sum_\lambda R_\lambda},}         \tag{4}

\]

which contains no `b`. The B marginal similarly contains no `a`.

The result is stronger than equality after aggregation. Settings change only
the local map from one of five retained history slots to an outcome label.
Because the number and ordering of remote slots are unchanged, the opposite
wing's outcome sequence is identical at every deterministic enumeration index.
The certificate verifies zero pointwise remote-setting dependence across 400
local projections.

This is a finite local-causal no-signalling theorem. It uses no probability
primitive, random draw, singlet, or nonlocal update.

---

## 3. Completeness is a physical requirement

If remote setting changes which source sectors enter the retained sample,
equation (2) fails. For the exact two-sector control

\[
 A_+=(1,0),\qquad
 K_B^{b=0}=(1,1),\qquad
 K_B^{b=1}=(1,2),
\]

the retained A+ marginal changes from `1/2` to `1/3`.

This sampled drift does not require a superluminal influence; it is induced by
setting/sector-dependent incompleteness. Therefore an operational
no-signalling claim must audit source-complete trial ownership and retained
records, not merely local equations. In FTD terms, expiry, detector failure,
or setting-dependent admission cannot be hidden in postselection.

---

## 4. Native local CHSH ceiling

If one common source record supplies setting-independent complete local
response values

\[
 A_0(\lambda),A_1(\lambda),B_0(\lambda),B_1(\lambda)
 \in\{-1,+1\},
\]

then for every source state

\[
 S_\lambda=A_0B_0+A_0B_1+A_1B_0-A_1B_1=\pm2. \tag{5}

The certificate exhausts all sixteen deterministic response tables. Every
nonnegative finite mixture consequently obeys

\[
 \boxed{|S|\le2.}                              \tag{6}

\]

This is the native ceiling of a source-complete, measurement-independent,
locally factorized finite-history model. The prepared Gaussian/Born count does
not evade it.

---

## 5. What “four separate contexts” means precisely

Four context-specific preparations can supply four empirical correlations
without supplying any physical equivalence relation that identifies records
across contexts as descendants of one common `lambda`. One may calculate a
CHSH combination from those correlations, but equations (5)--(6) are not
applicable until a common cross-context source coupling is defined.

That is an epistemic-versus-ontic bookkeeping boundary, not a refutation of
Bell's theorem. Nor does it prove that laboratory Bell trials are uncoupled
context-specific preparations. FTD must model the actual source, setting
choice, detector completeness, retained records, and spacetime separation.

If FTD claims one setting-independent common source with local complete
responses, it is committed to (6). If instead it uses context-dependent source
sectors, measurement dependence, incomplete retention, or a nonfactorizable
contextual response, that price must be represented in the finite physical
state and tested rather than introduced through the scientist's later
aggregation.

---

## 6. What is closed and what remains open

Closed on the prepared local sector:

- exact bipartite products of native `|Z|^2` history counts;
- deterministic full enumeration of the finite joint bank;
- pointwise and aggregate operational no-signalling under source completeness;
- the exact incompleteness/postselection boundary; and
- the native source-complete local CHSH ceiling `2`.

Still open:

- formation of the common source bank by homogeneous `Phi`;
- causal splitting/routing of one source emission into one owned paired trial;
- formation and protection of both local apparatuses;
- amplified persistent records, reciprocal backreaction, and expiry/reset;
- overlapping trial traffic and finite-window environmental robustness; and
- recovery of the actual laboratory Bell correlations or an explicit,
  empirically adequate alternative account.

Thus multipartite no-signalling is now exact for the prepared local product
sector. The general physical Born/Bell gate remains open.

The later
[`Phi-v15 transitive scheduler successor`](THEOREM_V3_TRANSITIVE_A2_SOURCE_HISTORY_ODOMETER_BORN_TIME_MEASURE_PHI_v15_CANDIDATE_v1.md)
adds a genuine finite cross-history record: two A2 owners enumerate all
ordered Phi-v13 source-history pairs in one constant-deadline orbit. This
replaces an analyst-supplied reference mixture for that exhaustive sector. It
does not causally split one emission to spacelike apparatuses, implement
setting choices, or evade this theorem's local CHSH ceiling.

The subsequent
[`Phi-v16 opposite-bank convoy successor`](THEOREM_V3_OPPOSITE_BANK_CONVOY_CAUSAL_BIPARTITE_ROUTING_PHI_v16_CANDIDATE_v1.md)
closes the prepared delivery part of that boundary: both banks retain one
common scheduler-origin record and move on opposite one-hop-per-tick routes
with exact endpoint handoff. Setting carriers, spacelike timing, apparatus
backreaction, and the laboratory correlation law remain open; the local CHSH
ceiling is unchanged.

---

## 7. Reproduction

```bash
python scripts/proofs/proof_v3_bipartite_born_no_signalling_local_ceiling.py
```

Expected result: `12/12` checks pass.
