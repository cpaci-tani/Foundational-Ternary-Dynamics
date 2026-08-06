# PRE-REGISTRATION — Cubic hop work response v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0447`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0444` scalar-work underdetermination; informed by `FTD-0445/0446`  
**Engine artifact:** `engine/tests/campaign_cubic_hop_work_response.cpp`  
**Campaign SHA256:** `d0f0331ebe6231ba4bbb68a4dc3b430441cd1996be7174e9b6e4850f46906b28`  
**Helper SHA256:** `25b149f168872f18373e8a77fd771367151fbe7df6940ca9a72d74aa97cad5a1`

## 1. Question

FTD-0444 proved that scalar work alone leaves two transverse force components.
FTD-0447 adds the native cubic symmetry assumption appropriate to an isolated
hop with no other local directional data:

> If the response is an ordinary polar vector depending only on scalar work
> `W` and Moore displacement `d`, does invariance under the full cubic
> stabilizer of `d` force the response to be longitudinal?

## 2. Exact theorem under test

Let `O_h` be the 48 signed coordinate permutations and

$$
H_d=\{g\in O_h:g d=d\}.
$$

A cubic-covariant isolated-hop response must satisfy `gF(d)=F(d)` for every
`g in H_d`. If the fixed subspace of `H_d` is exactly `span(d)`, then imposing

$$
F(d)\cdot d=W
$$

uniquely gives

$$
F(d,W)=\frac{W}{|d|^2}d.
$$

## 3. Frozen exact checks

- enumerate all 48 signed coordinate permutations;
- enumerate all 26 nonzero Moore displacements;
- construct every integer constraint row of `(g-I)F=0` for `g in H_d`;
- prove the constraint rank is exactly `2`, hence fixed dimension `1`;
- verify `d` is in that fixed subspace;
- verify stabilizer sizes `8/4/6` for face/edge/corner orbits;
- use registered integer work `W=6` and verify exact work closure;
- verify response covariance across all `26*48=1248` transformed cases.

All calculations use exact integers.

## 4. Locked outcomes

- `CUBIC_STABILIZER_FIXES_LONGITUDINAL_WORK_RESPONSE`: every exact gate passes.
- `PROTOCOL_INVALID`: any gate fails.

## 5. Interpretation boundary

A positive result upgrades the longitudinal force representative from a free
choice to a theorem only within this assumption package:

- isolated hop;
- response is a polar three-vector;
- only local directional datum is `d`;
- scalar work `W` is already known;
- full cubic covariance is required.

A background field, spin/axial datum, neighboring configuration, memory, or
link-channel state supplies additional tensors and can permit transverse
responses. This theorem does not choose the nonlinear momentum branch, field
recoil support, or a 3-vector versus 13-channel ontology.

## 6. Banned moves

- No group, stabilizer definition, work, direction set, response law, or gate
  may change after first execution.
- No production force replacement follows from this observer-only proof.
- No claim that field recoil is solved.
