# PRE-REGISTRATION — Discrete interaction-work contract v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0443`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0442` legacy force/Lagrangian contract audit  
**Engine artifact:** `engine/tests/campaign_discrete_interaction_work_contract.cpp`  
**Artifact SHA256:** `9417064ff0ebe2de2a98e1125350bae22ab04621f1d7c3fe095f1e52a620e827`  
**Helper SHA256:** `4ce99516be120486af2bd28cff98dfd6f5e24edb4f27b16d56c1c99b6dd143a1`

## 1. Question

The declared interaction is

$$
L_{int}=G_C\sum_xs_x(\nabla\cdot J)_x.
$$

FTD-0442 found that the helper and production legacy forces disagree with this
term in sign and coupling power. FTD-0443 asks:

> What force/work statement follows exactly from the site-valued action, and
> does the production legacy force implement it?

## 2. Exact finite-site statement

For a charge `q` moved from site `a` to site `b` at fixed field,

$$
\Delta L_{int}=G_Cq[(\nabla\cdot J)_b-(\nabla\cdot J)_a].
$$

This endpoint difference is the registered hop work. It is antisymmetric under
hop reversal and telescopes to zero around a closed loop. The symmetric smooth-
sampling candidate at a site is

$$
F_{sym}=+G_Cq\,\nabla_c(\nabla\cdot J).
$$

This second equation is a diagnostic bridge, not an assertion that continuous
position exists in the frozen ontology.

## 3. Frozen protocol

- periodic `L=17`;
- deterministic analytic vector-flux fixture with nonzero three-axis
  `grad(div J)` at site `(4,5,6)`;
- both mobile signs and all 26 Moore-neighbor hops (`52` cases total);
- a four-link closed loop;
- exact full-action recomputation before/after every hop;
- remainder-only displacement `(0.31,-0.27,0.19)` at unchanged site state;
- one forced-CPU production tick with only `forces` and `strict_validation`
  enabled, selecting the legacy non-Poisson/non-emergent branch.

## 4. Locked gates

- hop action-change residual, reverse-hop residual, closed-loop work, and
  symmetric-gradient residual: each `<=1e-14`;
- remainder-only coupling-action change: `<=1e-15`;
- production output must match its coded `-alpha q grad(div J)` formula to
  relative `1e-12`;
- production matches the action candidate only if relative error `<=1e-12`;
- the registered mismatch signature is cosine `<=-0.999999999999` and
  magnitude ratio equal to `alpha/G_C` within `1e-12`.

## 5. Locked outcomes

- `EXACT_HOP_WORK_PRODUCTION_MATCH`: algebra/remainder gates pass and production
  matches `+G_C q grad(div J)`.
- `EXACT_HOP_WORK_PRODUCTION_MISMATCH`: algebra/remainder gates pass, production
  matches its copied negative formula, and the locked opposite-sign/coupling-
  ratio signature passes.
- `UNCLASSIFIED_PRODUCTION_MISMATCH`: valid mismatch lacking that signature.
- `HOP_IDENTITY_FAILURE`: exact finite-site algebra fails.
- `PROTOCOL_INVALID`: nonfinite output or configuration/copy-formula failure.

## 6. Interpretation boundary

A hop-work theorem does not by itself define continuous sub-voxel acceleration.
If remainder motion is to couple continuously, a source-shape/interpolation
rule must be added and tagged `[SELECTION]` or derived from a deeper ontology.
No production force is changed by this campaign.

## 7. Banned moves

- No coefficient, sign, flux fixture, site, gate, or stencil change after first
  execution.
- No reinterpretation of `alpha` as a single vertex after seeing the result.
- No production tick modification in response to the verdict.
