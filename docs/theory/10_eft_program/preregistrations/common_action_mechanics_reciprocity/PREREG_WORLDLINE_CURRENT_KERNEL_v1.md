# PRE-REGISTRATION — Worldline current kernel v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0502`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0478`, `FTD-0484`, `FTD-0501`

## Question

After retaining an unordered multiset of manifested endpoints, is the exact
oriented face current uniquely determined, or does reciprocal field evolution
still require worldline pairing/path information?

## Locked face-complex theorem

For a connected periodic cubic lattice with `V=L^3` sites, the oriented face
current space has dimension `3V`. The backward divergence maps onto the
`V-1` dimensional zero-sum site subspace. Therefore

```text
rank(div)=V-1,
dim ker(div)=3V-(V-1)=2V+1.
```

Continuity fixes only one affine coset of this kernel. Prove the rank statement
constructively by routing any zero-sum site source along a spanning tree; do
not infer it from numerical matrix rank alone. Register the exact dimensions
for `L=3,5,17`.

## Locked local-loop construction

Inside one cell at `z=8`, use four `q=+1` carriers:

```text
A=(8.25,8.25,8), B=(8.75,8.25,8),
C=(8.75,8.75,8), D=(8.25,8.75,8).
```

Construct three one-tick histories:

```text
static:  A->A, B->B, C->C, D->D,
CW:      A->B, B->C, C->D, D->A,
CCW:     A->D, D->C, C->B, B->A.
```

Every moving segment has length `0.5<C_SPEED`. Require:

1. identical aggregate start and end polarity for all histories;
2. exact aggregate continuity below `1e-12`;
3. zero static current;
4. nonzero CW current and `J_CCW=-J_CW` below `1e-12`;
5. zero divergence of the loop-current difference.

## Locked field-response discriminator

From common zero face field, apply the same normalized source update

```text
E_after = E_before - kappa J,
kappa=0.73.
```

Require static field response zero, CW/CCW responses opposite, relative Gauss
unchanged, and equal nonzero field energies for CW and CCW. This establishes
that the current-kernel branch changes a registered dynamical field and is not
an endpoint relabeling.

## Locked covariance and translation arms

Repeat the static/CW/CCW discriminator under all 48 signed cubic maps about
knot `(8,8,8)` and translations

```text
(-2,+1,0), (0,0,0), (+2,-1,+1)
```

inside `L=17`. Require the same endpoint equality, current opposition,
continuity, causal length, and field-energy equality in all 144 transformed
triples.

## Frozen verdicts

- `ENDPOINT_MULTISET_DETERMINES_FACE_CURRENT` only if all admissible histories
  with the locked endpoints deposit the same current.
- `WORLDLINE_PATH_IS_REQUIRED_STATE` if the exact kernel dimension and local
  loops pass and the common endpoint state produces distinct field updates.
- `LOOP_KERNEL_IS_NUMERICAL_OR_NONCAUSAL` if current opposition, continuity,
  covariance, or causal length fails at `1e-12`.

## Scope ceiling

This is observer-only. It does not authorize particle labels, a matching
algorithm, a new history variable, field evolution, a toggle, or a scenario.
It distinguishes an unordered endpoint ontology from a worldline ontology; it
does not claim distinguishable labels for identical carriers are physical.

## Run-of-record hashes

- test SHA256:
  `D283CFB2CF83502A413D922BD750C54DFEC629C2DCE7609FE0C1BEDA10C3841A`;
- header SHA256:
  `8CF8674D2EE3D81E3B5DE74E1DCE3C02A53FBA952AF94F45541D2D807BF755A4`;
- implementation SHA256:
  `20F9A43AF89A7FA3E5C01C719F1408BBBA33C7571AEA4E2570F0FC3D1811F36D`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- record: `engine/results/ftd_0502/windows_msvc_cpu.json`.
