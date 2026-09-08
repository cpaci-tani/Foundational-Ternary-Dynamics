# Exact two-token sector on a homogeneous occupied background

Date: 2026-09-05. Law: `phi-v2-staged-candidate-1`.
Status before analysis: **[PREREGISTERED — FINITE PAIR MAP]**.

## Domain and locked criteria

Use the unchanged candidate with s=0, spatially uniform ell, both slots of
every SC/FCC relation carrying the same nonblank A9 code, and exactly two
distinct occupied field slots. Allow equal or opposite polarity. This is an
externally prepared occupied background; no vacuum formation is claimed.
Actual validation uses finite periodic L>=3 probes. Unwrapped relative motion
is an external finite-history chart and must not infer interaction from a
periodic seam return.

Analyze the complete three-layer same-polarity pair tables and all 384 internal
channel maps. Enumerate exact drift labels, preserved layer moments, and
incoming/outgoing displacement multisets. This is finite-map analysis without
numerical fitting or near-miss searches. No many-seed measurement campaign is
part of this contract.

For noninteracting segments, derive relative displacement from the twelve-hop
channel period. Classify equality of asymptotic drift separately from changes
caused by collision. Opposite-polarity pairs never enter a two-token bank and
therefore supply an exact no-collision control. Same-polarity pairs interact
only when they meet at a site during the declared collision stage. Individual
identity ends at that pair vertex unless the law supplies an unambiguous map.

A positive scattering result requires an actual finite-table transition that
changes a declared spatial outgoing observable, its counterfactual collision-
disabled comparison, and agreement with actual charged staged transitions.
A localized pair is not called bound merely because two independent drifts
match or a finite torus recurs. A no-binding result, if available, must state
its exact polarity/preparation sector and prove the restriction; absence of a
pattern in bounded observations is not an impossibility theorem. Interacting
mixed-record matter remains outside any such two-token conclusion.

Results and limitations will be appended after this contract without altering
the acceptance criteria above.

## Exact sector and free relative motion

**[THEOREM — CONDITIONAL PREPARED SECTOR].** Both occupied relation slots rotate
independently of gates and cancel manifestation at every site. Absorption is
disabled. The field count remains exactly two, including collisions. Same-
polarity tokens collide exactly when co-located at input phase 1; opposite-
polarity tokens never trigger a pair collision. Their two complete trajectories
then evolve independently despite any meeting or saved-gate correlation. These
claims follow from the actual stage map, not from superposing a nonlinear law.

For a free channel c, let D(c) be the sum of its three successive pre-update
streaming directions. The actual flag map repeats its direction sequence after
three hops; its phase repeats after twelve. Every component of D is +1 or -1.
After h=3n+r hops, r in {0,1,2}, its displacement is exactly

```text
F_c(h) = n D(c) + sum_(j=0)^(r-1) tangent(U^j c).
relative(h) = relative(0) + F_second(h) - F_first(h).
```

The remainder in each relative coordinate is at most two. If the drifts differ,
some lifted relative coordinate grows by a nonzero multiple of two per three
hops. For every finite localization radius one can therefore give a finite
exit time. A finite torus can wrap this motion; that recurrence is not binding.
If the drifts agree, relative motion is exactly three-hop periodic. This is
ballistic co-motion and needs no interaction. In particular, opposite-polarity
pairs in this preparation have no collision-mediated binding or restoring
response. This is not a no-go for opposite charges in other backgrounds or laws.

## Spatial scattering and exact map census

**[DERIVED — FINITE COLLISION OBSERVABLE].** At layer zero the frozen transition
`(0,32) -> (144,176)` changes the unordered outgoing hop set from opposite x
directions to opposite z directions. The actual staged runtime reproduces that
transition and its following stream on an L=5 probe. The collision-disabled
comparison would stream along x. An actual opposite-polarity preparation with
channels `(0,224)` supplies an independent no-collision control under the same
law. This establishes a spatial interaction at the finite-map level.

The incoming and outgoing sums of the six current-layer channel moments agree
on every one of the 18,336 unordered pair rows in each layer. Both field-token
polarities obey the same local table. Exhaustive counts are:

| Layer | Outgoing hop multiset changes | Free drift multiset changes | Free drift sum changes | Same-flag inputs |
|---|---:|---:|---:|---:|
| 0 | 96 | 480 | 288 | 288 |
| 1 | 288 | 480 | 288 | 288 |
| 2 | 384 | 480 | 288 | 288 |

The drift labels describe hypothetical free propagation after a collision.
They are not conserved physical momentum: the sum explicitly changes in 288
rows per layer. Conserved layer moments likewise acquire no physical momentum
or energy interpretation merely from this census. No target coefficient was
fitted and no empirical physical values were used.

## A repeated-collision co-located pair sector

**[THEOREM — EXACT FINITE SUBSECTOR].** Two distinct same-polarity channels with
the same flag and different phases have a common streaming direction. Every
layer's collision map preserves this same-flag condition in both directions.
After collision, U updates their common flag identically. Thus a co-located
pair in this sector remains co-located by induction, and has one actual pair
collision per four physical microticks.

The finite collision-boundary map is

```text
(ell, {c1,c2}) -> (ell-1 mod 3, {U(out1),U(out2)}),
where {out1,out2}=C_ell({c1,c2}).
```

For positive polarity its domain has 3*288=864 states. Exhaustive traversal
finds 96 orbits of length 3 and 48 of length 12, accounting for all 864 states.
Negative polarity duplicates these maps. Including the homogeneous relation
phase, every orbit returns its complete internal records after 48 microticks,
up to translation of the common field site. During phases 1 through 3 every
saved gate equals one: each endpoint pair contains either zero or both field
tokens at admission time. Admission masks stay zero; phase 0 clears all gates.

The 3-cycle orbits translate by `(+-4,+-4,+-4)` per complete 48-tick period.
The 12-cycle orbits translate by vectors with one zero component and two
components `+-4`. The latter have a genuinely collision-dependent trajectory:
for initial layer zero and channels `(0,1)`, the actual common displacement is
`(-4,0,4)` after 48 microticks, whereas disabling collisions would give
`(-4,4,4)`. The runtime comparison uses all actual stages and exact lifted hops,
including seams. These orbit directions are observer-coordinate quantities,
with no physical speed calibration.

This is a positive repeated-interaction transport result, not evidence for a
bound atom. Without collisions, these same-flag channels also remain co-located.
More sharply, displacing one token by any nonzero periodic lattice offset while
leaving its channel unchanged makes them remain separated by that offset:
their identical flag evolution then gives identical free displacements, so
they never meet and never collide. There is no restoring interaction for this
perturbation. The exact argument applies at every finite future time; an L=3
48-microtick regression checks it directly. No attraction, binding energy,
formation mechanism or perturbation-independent composite species is established.

General same-polarity pairs can separate and meet again. The present finite
subsector census does not classify all their relative histories or rule out
other interacting localization. Mixed field/relation defects and many-token
material recovery remain open.

## Validation and disposition

Implementation: `scripts/phi_v2_lattice/recovery_pair_sector.py`.
Tests: `scripts/tests/phi_v2_lattice/test_recovery_pair_sector.py`.
The first 17 tests passed in 4.54 seconds; one subsequently added regression
checks the proven absence of restoring response after a relative displacement.
The resulting 18-test suite passed in 4.70 seconds. Acceptance criteria above
were not changed. Independent review and its disposition are recorded in the
program ledger; this implementation report does not certify its own review.

The finite-table census exhausts local maps and the declared 864-state quotient.
Actual trajectories are bounded deterministic fixtures, not a random or
multi-seed campaign. APIs expose unordered collision pairs and common pair
positions, not invented individual identities through ambiguous pair vertices.
No source law, collision table, backend or dashboard capability is changed.
