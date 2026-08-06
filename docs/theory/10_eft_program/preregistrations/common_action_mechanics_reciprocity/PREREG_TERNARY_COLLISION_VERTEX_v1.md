# PRE-REGISTRATION — Ternary collision vertex v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0504`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0501`, `FTD-0502`, `FTD-0503`

## Question

Which coincident-target events actually require a new collision law: an
identical-carrier crossing between snapshots, or only a multiplicity that must
be written into a ternary snapshot?

## Locked same-sign capacity theorem

For `m>=2` coincident same-sign carriers, the required local manifested charge
is `q=m` or `q=-m`, while one ternary site can store only `{-1,0,+1}`. The
minimum exact charge defect is

```text
min_{s in {-1,0,+1}} |m-s| = m-1.
```

Test `m=2,...,8`. This is a snapshot-capacity statement; a transient event
vertex may have multiple incident worldlines without becoming a stored site.

## Locked interior-crossing construction

Use center `(8.5,8.5,8.5)`, unit direction proportional to `(1,2,3)`, initial
half-separation `a=0.25`, equal causal speed `v=0.40`, and `dt=1`. The carriers
meet at

```text
tau=a/v=0.625
```

and have `0.375` tick remaining. Compare:

1. **pass-through:** each associated carrier retains momentum;
2. **equal-mass elastic bounce:** the carriers exchange momenta at the vertex.

Require that, after quotienting identical labels, both yield exactly the same
unordered endpoint position-momentum multiset and the same aggregate exact
face current. Require energy, total momentum, charge, causality, continuity,
and full time reversal below `1e-12`.

Repeat under all 48 signed cubic maps and the three established integer
translations. The common vertex is transient and must not be written as an
intermediate ternary snapshot.

## Locked tick-boundary discriminator

Repeat with `v=0.25`, so `tau=1`. Both carriers then occupy the common target
at the snapshot boundary and no post-collision time remains. Require the
observer to return `TERNARY_ENDPOINT_OVERLOAD`, not to infer pass-through,
bounce, annihilation, or delayed motion.

## Locked distinguishability ceiling

The identical-crossing quotient applies only when every transported intrinsic
attribute agrees. Record that differing polarity, spin, color, flavor, or any
other physical tag makes pass-through and exchange different attribute
worldlines. `particle_id` alone is not treated as a physical distinction.

## Locked 3D conservation counterfamily

In the center-of-momentum frame, equal-mass elastic outputs

```text
(p',-p'), |p'|=|p|
```

have the same total momentum and relativistic energy for every direction of
`p'`. Register at least the x, y, z, face-diagonal, and body-diagonal outputs
with residual below `1e-12`. This proves conservation alone does not select a
3D scattering angle; a central interaction, impact parameter, or other local
collision mechanism is additional input.

## Frozen verdicts

- `IDENTICAL_INTERIOR_CROSSING_IS_PERMUTATION_GAUGE` if the interior arms are
  exactly equivalent but the boundary overload and 3D angle family remain.
- `IDENTICAL_CROSSING_REQUIRES_NEW_STATE` if pass-through and bounce differ in
  any unlabeled physical/current observable.
- `TERNARY_COLLISION_RULE_DERIVED` only if one mechanism also resolves the
  boundary overload and uniquely selects all registered 3D outputs. The
  present protocol contains no such mechanism.

## Scope ceiling

This is observer-only. It does not authorize collision code, multi-occupancy,
an interaction potential, attribute exchange, annihilation, a toggle, or a
scenario. It classifies the exact frontier left by FTD-0503.

## Run-of-record hashes

- test SHA256:
  `649ECBC66A1FAAD9ED01F71F953965BE9D1668040D41722712DC4A066F55DE1F`;
- header SHA256:
  `E46C42E89D61A9C897E7246E83AA95CD511BC831411C31BA736E1F53E6CE4D97`;
- implementation SHA256:
  `6CA470A788FA209839B0C2A4DB8AFE7350A57FCC2DE54B9FAAB55B87C345DB4A`.
