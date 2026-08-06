# AUDIT — Multibody shape observability

**Date:** 2026-07-25  
**Identifier:** `FTD-0501`  
**Status:** `[THEOREM — CIC MOMENT FACTORIZATION/NONINJECTIVITY]` +
`[CONSTRUCTIVE — EXACT MULTIBODY KERNEL]` +
`[CLOSED NEGATIVE — AGGREGATE SHAPE/CURRENT AS COMPLETE MATTER STATE]`  
**Verdict:** `SHAPE_CURRENT_REQUIRES_WORLDLINE_DECOMPOSITION`  
**Pre-registration:**
[`PREREG_MULTIBODY_SHAPE_OBSERVABILITY_v1.md`](../10_eft_program/preregistrations/PREREG_MULTIBODY_SHAPE_OBSERVABILITY_v1.md)  
**Run of record:** `engine/results/ftd_0501/windows_msvc_cpu.json`

## 1. Exact CIC factorization theorem

Place signed carriers at fractional coordinates `f_i` in one lattice interval.
The two one-dimensional hat weights sum to

```text
rho_0 = sum_i q_i(1-f_i) = Q-M,
rho_1 = sum_i q_i f_i     = M,
Q = sum_i q_i,
M = sum_i q_i f_i.
```

Thus the entire deposited density depends only on total signed charge and one
signed first moment. For `N` equal-sign carriers with fixed `Q=N`, every
continuous family with fixed `sum f_i` lies in the same shape fiber. Before
permutation identifications this fiber has dimension at least `N-1`.

Embedding the construction on a coordinate line with integer transverse
coordinates proves noninjectivity of the full three-dimensional trilinear map.
No claim about generic three-dimensional configurations can restore global
injectivity after an exact one-dimensional kernel exists.

## 2. Exact current histories inherit the same kernel

For paths that remain in the interval, exact discrete continuity makes the
aggregate oriented face current depend on the change of the same moment `M`.
Consequently two time-dependent configurations with equal `Q(t)` and `M(t)`
have identical aggregate charge/current histories even when their constituent
worldlines differ.

The executable constructs every current by summing individually valid
FTD-0478/0484 straight-segment currents, then recomputes aggregate continuity.
The worst residual over every registered arm is exactly zero. The kernel is
therefore not caused by an approximate deposition, chart convention, or failed
continuity solve.

## 3. Same-sign internal separation is invisible

The locked configurations

```text
A+: {8.25, 8.75},
B+: {8.375,8.625}
```

have equal `Q=2` and equal `M`. Their full start density, end density, and face
current under common translation by `0.05` agree exactly. Nevertheless

```text
|x_2-x_1|_A^2-|x_2-x_1|_B^2
=0.50^2-0.25^2
=3/16.
```

Any collision, binding, or internal-energy rule that depends on separation
therefore requires information absent from the aggregate shape/current state.

Both configurations have the same primitive ternary anchor pattern. Their
per-voxel remainders differ by `0.125`; this is exactly the individual
subcell information erased by aggregation.

## 4. A moving neutral composite can be invisible

The neutral configurations

```text
A0: + at 8.35, - at 8.65,
B0: + at 8.45, - at 8.75
```

have equal `Q=0` and equal signed moment `M=-0.30`, but their unsigned centers
differ by `0.10`. Their complete aggregate `rho/current` histories under rigid
translation by `0.05` agree exactly.

More strongly,

```text
aggregate current L1       0
constituent current L1     0.10000000000000142.
```

Because common translation changes `M` by `Q delta`, a neutral pair has
`Delta M=0`; its opposite constituent currents cancel on the available face
degree of freedom while the pair moves inside the cell. The aggregate signed
face complex therefore does not contain the neutral center-of-mass worldline.

The raw ternary anchors again agree while their individual remainders differ by
`0.10`.

## 5. The vacuum kernel

A coincident `(+1,-1)` pair has exactly zero total shape and zero total face
current, identical to vacuum in the aggregate variables. The run of record
finds signature norm zero.

This pair is an algebraic control, not an allowed frozen ternary state. Its
purpose is to show that additive signed shape cannot itself decide whether a
neutral coincidence is absent, present before annihilation, or already reacted.
That distinction must be supplied by a reaction/worldline ontology.

## 6. The kernel is covariant

Both nontrivial kernels were repeated under all 48 signed cubic maps and three
integer translations, giving 288 compared history pairs. The worst difference
remained exactly zero and every aggregate continuity residual remained zero.
The degeneracy is structural, not an axis choice or boundary accident.

## 7. Minimum ontological consequence

The trilinear polarity plus face current is a valid coupling representation
for one already identified carrier. It is not a complete multibody state.
Composite matter requires at least one of:

1. an explicit multiset of signed manifested positions and paired worldlines;
2. enough per-cell multipole data to reconstruct every allowed constituent;
3. an exclusion law restricting each interpolation cell to a single carrier.

Option 1 is a new particle/worldline ontology. Option 2 cannot remain a fixed
finite hierarchy for unbounded occupancy: a finite collection of moments has
continuous kernels once the number of constituent degrees of freedom exceeds
the number of independent moments. Option 3 forbids precisely the nearby
composite configurations the engine must eventually describe and is also a new
dynamical postulate.

Therefore a shape-ontic rewrite cannot consist only of replacing site `s` by
the summed FTD-0478 density. It must preserve a constituent decomposition or
introduce an explicit occupancy bound with collision/reaction semantics.

## 8. Plan consequence and reproducibility

FTD-0501 does not invalidate the isolated one-body coupling sidecar. The
original FTD-0481 cycle deliberately excluded collisions and multiple matter.
It does block promoting that sidecar into a general matter ontology or using
aggregate face current to claim neutral-composite motion.

- checks: `13/13 PASS`;
- test SHA256:
  `CDDAF8B0E272B6D94F1BA29387FA1E27995D907E16D54C8CC5FE571C1B6E2ABA`;
- header SHA256:
  `2B72991B2B5189E957C1811B5AD83391AFEE8CFF14BE007324D7F2825C29B5D4`;
- implementation SHA256:
  `C01C2F7575548D1FEDE7786D78E35EAF6A2657F0C3DD4C75573B720112EBEF7C`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
