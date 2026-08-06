# AUDIT — Moore-hop route ambiguity

**Date:** 2026-07-24  
**Identifier:** `FTD-0445`  
**Status:** `[THEOREM — ENDPOINT CONTINUITY DOES NOT SELECT A FACE ROUTE]` + `[MEASURED — EXISTING EXTRACTOR SELECTS X/Y/Z]`  
**Verdict:** `FACE_ROUTING_UNDERDETERMINED`  
**Pre-registration:** [`PREREG_MOORE_HOP_ROUTE_AMBIGUITY_v1.md`](../10_eft_program/preregistrations/PREREG_MOORE_HOP_ROUTE_AMBIGUITY_v1.md)  
**Run of record:** `engine/results/ftd_0445/windows_msvc_cpu_L9.csv`

## 1. Exact route multiplicity

One production movement event may target any of the 26 Moore neighbors. When
that primitive event is represented on the selected oriented-SC-face
continuity complex, its nonzero Cartesian components may be ordered in more
than one way.

The locked all-direction campaign finds exactly:

| Native hop shell | Directed cases | Distinct shortest face routes per case |
|---|---:|---:|
| face / SC | 6 | 1 |
| edge / FCC direction | 12 | 2 |
| corner / BCC direction | 8 | 6 |

There are zero count mismatches. Every route has continuity residual exactly
zero, and the minimum L2 distance between distinct routes is `2`.

Endpoint continuity therefore does not select the face-current history for 20
of the 26 native movement directions.

## 2. Existing extractor is a selection

For a single positive `(1,1,1)` corner hop, the existing
`extract_moore_history_from_snapshots()` output equals the x-then-y-then-z
route exactly. This confirms the documented deterministic routing choice; it
does not derive that order from the movement event.

The corner endpoints are fixed by exchanging x and y. The chosen current is
not: its distance from its swapped image is `2`. Thus the individual selected
route does not respect the endpoint event's x/y stabilizer.

Averaging all six routes restores that swap symmetry and exact continuity, but
the result contains fractional face currents. It represents one primitive hop
as simultaneous fractions on unrealized paths. That is another admissible
selection, not evidence that the event followed six routes.

## 3. Gauss continuity is not an energy principle

Two routes with the same endpoints differ by a divergence-free closed face
current. In the registered background

$$
E_0=0.25(K_{xyz}-K_{yxz}),
$$

the background divergence is exactly zero. Both existing matched updates are
valid and produce the same source relocation, yet their quadratic field
energies are `1.125` and `2.125`.

The energy split is exactly `1`, passing the locked `0.5` gate. Hence Gauss
closure and endpoint continuity do not determine the local field trajectory
or its work in a pre-existing transverse background.

## 4. Ontological consequence

The native movement graph and the selected face-field complex are different
objects:

- native movement has face, edge, and corner links as one-tick primitives;
- the matched field sidecar stores only oriented SC faces;
- projecting an edge/corner link onto SC faces requires a route convention.

The current x/y/z convention is mathematically valid bookkeeping, but it
cannot yet be used as the unique physical spacetime history of the hop.

Three honest continuations remain:

1. Treat all 26 directed Moore links (13 unoriented channels) as primitive
   current carriers.
2. Select an ordered SC-face routing law and accept/control its microscopic
   symmetry breaking.
3. Select a symmetric fractional routing law and justify why a single event
   is distributed over virtual paths.

The frozen ontology supplies no criterion choosing among them.

## 5. Correct claim boundary

FTD-0427/0428 retain their exact selected-complex Gauss and continuity
results. FTD-0445 does not invalidate them. It restricts their interpretation:
the routed current is a selected representation of native Moore movement, not
the uniquely derived local path taken by matter or field momentum.

## 6. Reproducibility

- campaign SHA256: `4191f761dfc971bb415e9ddbc7fb1ba47f8bd2b70c82f66be46e5d2414446594`
- helper SHA256: `fb62681746b69ee232af559a7b9c39a4b65c73de6eb19bbe4b7b5c3bdcee0aec`
- record SHA256: `3506bd2bf1031be47fdb993e60452e61b58401b504f94f0a42b2e47ddec4c9a0`
- compiler: pinned MSVC `14.44.35207`, Release
- execution: CPU algebraic observer using the existing continuity extractor
- result: `FACE_ROUTING_UNDERDETERMINED`

No production dynamics were changed.
