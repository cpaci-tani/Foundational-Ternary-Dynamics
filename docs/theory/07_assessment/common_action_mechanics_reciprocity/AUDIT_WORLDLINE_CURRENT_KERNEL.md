# AUDIT — Worldline current kernel

**Date:** 2026-07-25  
**Identifier:** `FTD-0502`  
**Status:** `[THEOREM — PERIODIC DIVERGENCE-KERNEL DIMENSION]` +
`[CONSTRUCTIVE — CAUSAL LOCAL CURRENT LOOP]` +
`[CLOSED NEGATIVE — ENDPOINT-MULTISET CURRENT UNIQUENESS]`  
**Verdict:** `WORLDLINE_PATH_IS_REQUIRED_STATE`  
**Pre-registration:**
[`PREREG_WORLDLINE_CURRENT_KERNEL_v1.md`](../10_eft_program/preregistrations/PREREG_WORLDLINE_CURRENT_KERNEL_v1.md)  
**Run of record:** `engine/results/ftd_0502/windows_msvc_cpu.json`

## 1. Continuity fixes a boundary, not a history

On a lattice, site charge is a 0-chain and oriented face current is a 1-chain.
The discrete continuity equation

```text
rho_after-rho_before+div J=0
```

specifies the boundary of `J`. If two currents have the same endpoints, their
difference obeys

```text
div(J_1-J_2)=0.
```

Thus endpoint data determines an affine current class, not a unique transport
history. The missing part is a divergence-free 1-cycle.

This statement does not require distinguishable labels for identical carriers.
The physical object that must be selected is the aggregate oriented 1-chain,
which is invariant under relabeling of constituent worldlines.

## 2. Exact periodic kernel dimension

For `V=L^3` sites, periodic oriented face current has `3V` components. Every
divergence sums to zero, so

```text
rank(div) <= V-1.
```

Conversely, root a spanning tree at one site. For any zero-sum source `b`, let
the current on each parent-child edge equal minus the total `b` in the child
subtree. Direct cancellation gives `div J=b`. Hence divergence maps onto the
entire zero-sum subspace and

```text
rank(div)=V-1,
dim ker(div)=3V-(V-1)=2V+1.
```

The executable implements this routing construction, not a floating matrix-rank
estimate. It closes with zero residual for `L=3,5,17`. At `L=17`:

```text
site dimension             4913
face-current dimension    14739
divergence rank            4912
current-kernel dimension   9827.
```

The dimension formula is periodic-boundary specific, but the local loop below
lies away from the boundary and survives open or environmental boundaries.

## 3. A causal local loop with unchanged matter endpoints

Four positive carriers occupy the corners of a half-site square. Three exact
histories share the same start and end multiset:

```text
static: every carrier remains,
CW:     each carrier advances one square edge,
CCW:    each carrier advances one reverse edge.
```

Every moving segment has length `0.5<C_SPEED`. Exact trilinear deposition and
Whitney face-current integration give

```text
endpoint density difference       0
static current L1                  0
clockwise current L1               1
J_CW+J_CCW residual                0
loop divergence residual           0.
```

The same endpoint manifestation therefore supports at least three physically
distinct transport histories: no circulation, positive circulation, and
negative circulation.

## 4. The cycle changes field evolution

From common zero face field, apply the registered normalized source response

```text
E_after=-0.73 J.
```

The static history leaves zero field. CW and CCW produce opposite face fields
with equal nonzero energy:

```text
U_CW=U_CCW=0.0666125,
Gauss residual=0.
```

Thus the divergence-free branch is not a redundant endpoint labeling. It
changes a dynamical field while leaving charge and Gauss exactly unchanged.
In a reciprocal transaction, the corresponding matter history must supply or
recover that field energy.

## 5. Covariance

The static/CW/CCW construction was repeated under all 48 signed cubic maps and
three integer translations, for 144 transformed triples. Endpoint equality,
current opposition, causal length, continuity, Gauss, and field-energy equality
all close with worst residual zero.

## 6. Ontological consequence

FTD-0501 showed that aggregate shape loses constituent configuration. FTD-0502
now shows that even an unordered endpoint multiset does not determine transport.
A complete transaction needs one of:

1. an oriented worldline 1-chain chosen during the tick;
2. constituent position-momentum data plus an action that uniquely produces
   that 1-chain;
3. the face current itself as an event variable, with its cycle component
   constrained by additional dynamics.

Snapshot differencing cannot supply the cycle component. A minimum-norm current
would select zero for the square example and erase genuine circulation; that is
a new dynamical selection, not a consequence of continuity.

The native-first ontology can therefore be stated more sharply:

```text
manifestation snapshot: site 0-chain,
transport transaction:  oriented spacetime 1-chain,
field response:          coupled face/edge update.
```

The 1-chain may be transient rather than persistent, but it must be generated
atomically with motion. Inferring it after a many-to-one movement update is
mathematically underdetermined.

## 7. Plan consequence and reproducibility

FTD-0502 supports the face-flux mainline while tightening its required matter
interface. `FaceCurrentSegment` is not merely an observer reconstructed from
endpoint density; it represents indispensable event history. No production
branch is authorized until an action selects the multibody 1-chain together
with momenta and fields.

- checks: `9/9 PASS`;
- test SHA256:
  `D283CFB2CF83502A413D922BD750C54DFEC629C2DCE7609FE0C1BEDA10C3841A`;
- header SHA256:
  `8CF8674D2EE3D81E3B5DE74E1DCE3C02A53FBA952AF94F45541D2D807BF755A4`;
- implementation SHA256:
  `20F9A43AF89A7FA3E5C01C719F1408BBBA33C7571AEA4E2570F0FC3D1811F36D`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
