# PRE-REGISTRATION — Moore-hop route ambiguity v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0445`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0444` hop-mechanics underdetermination  
**Engine artifact:** `engine/tests/campaign_moore_hop_route_ambiguity.cpp`  
**Campaign SHA256:** `4191f761dfc971bb415e9ddbc7fb1ba47f8bd2b70c82f66be46e5d2414446594`  
**Helper SHA256:** `fb62681746b69ee232af559a7b9c39a4b65c73de6eb19bbe4b7b5c3bdcee0aec`

## 1. Question

Production treats all 26 Moore neighbors as one-tick destinations. The selected
matched continuity complex stores current only on oriented SC faces and routes
diagonal movement in deterministic x/y/z order. FTD-0445 asks:

> Does endpoint continuity uniquely determine that face route, and is the
> selected x/y/z route covariant under the stabilizer of a corner hop?

## 2. Frozen combinatorics

For every one of 26 displacements, route the nonzero Cartesian components in
all six axis orders and deduplicate identical face-current histories. Expected
unique route counts are:

- face hop: `1! = 1`;
- edge hop: `2! = 2`;
- corner hop: `3! = 6`.

Every route must satisfy exact endpoint continuity. For the positive corner
hop, compare the existing snapshot extractor against the x/y/z route.

## 3. Cubic-stabilizer test

The `(1,1,1)` endpoints are invariant under swapping x and y. Apply that swap
to the canonical x/y/z face current and compare it with the unrotated current.
Then average all six routes and repeat. A symmetric average is admitted only if
it preserves continuity and contains fractional face current; it is a selected
delocalized representation, not a primitive realized path.

## 4. Energy counterexample

Let `K_xyz` and `K_yxz` be two corner routes and choose the registered
divergence-free background

$$
E_0=0.25(K_{xyz}-K_{yxz}).
$$

Apply the existing matched update `E -> E-K` to each route. Because the route
difference has zero divergence, both updates obey the same Gauss endpoint
change. An energy split proves that continuity does not select the mechanical
field update in a pre-existing transverse background.

## 5. Locked gates

- exactly 6 face, 12 edge, and 8 corner cases;
- unique counts `1`, `2`, `6` respectively, with distinct-route L2 separation
  at least `1`;
- continuity and divergence residuals `<=1e-14`;
- current engine extractor equals x/y/z route to `1e-14`;
- x/y/z corner route changes by at least `1` under x/y swap;
- six-route average is swap-invariant and continuous to `1e-14`, with
  fractional face current;
- registered-background energy split at least `0.5`.

## 6. Locked outcomes

- `FACE_ROUTING_UNDERDETERMINED`: every gate passes.
- `ROUTES_ENERGETICALLY_EQUIVALENT_IN_REGISTERED_BACKGROUND`: route and
  symmetry gates pass but the registered energy discriminator does not.
- `PROTOCOL_INVALID`: any other or nonfinite result.

## 7. Interpretation boundary

This campaign audits the selected face-current representation, not the native
movement event. A failure of unique face routing does not invalidate Moore
locality. It means a primitive diagonal link, a selected ordered face route,
or a symmetric fractional routing law is additional structure.

## 8. Banned moves

- No route set, ordering, background, symmetry transform, or gates may change
  after first execution.
- No selected path or fractional average may be called native emergence.
- No production tick changes.
