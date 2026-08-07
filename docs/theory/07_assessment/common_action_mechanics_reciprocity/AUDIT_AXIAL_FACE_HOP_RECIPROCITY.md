# AUDIT — Axial face-hop reciprocity

**Date:** 2026-07-25  
**Identifier:** `FTD-0497`  
**Status:** `[THEOREM — RAW THRESHOLD MAP NON-INJECTIVE]` +
`[CONSTRUCTIVE — EXACT AXIAL PHYSICAL HOP]` +
`[CLOSED NEGATIVE — FROZEN RAW-STATE RECIPROCITY]`  
**Verdict:** `AXIAL_HOP_PHYSICAL_QUOTIENT_ONLY`  
**Pre-registration:**
[`PREREG_AXIAL_FACE_HOP_RECIPROCITY_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_AXIAL_FACE_HOP_RECIPROCITY_v1.md)  
**Run of record:** `engine/results/ftd_0497/windows_msvc_cpu.json`

## 1. A real exact face-current hop exists

Restricting the transaction to one principal axis removes the FTD-0480 vector
underdetermination: scalar current work fixes the sole impulse component. For
the exact deposited current `K(d)`, define

```text
E_1=E_0-gK,
E_mid=(E_0+E_1)/2,
E_path=<K,E_mid>/(q d),
p_1-p_0=dt g q E_path,
d=dt c^2(p_0+p_1)/(H_0+H_1).
```

The production dispersion then gives the identity

```text
H_1-H_0
=c^2(p_1+p_0)(p_1-p_0)/(H_1+H_0)
=g q E_path d
=g<K,E_mid>.
```

Meanwhile `E_1=E_0-gK` gives

```text
1/2 ||E_1||^2-1/2 ||E_0||^2=-g<K,E_mid>.
```

No dressing transfer is needed in the axial arm. The registered uniform field
drives a manifested state from remainder `0.85` through the `+1` threshold by
`0.25133299377250545` lattice units. Both polarities, all three axes, and an
integer translation agree.

## 2. The implicit hopping root is unique in the locked arm

When the path crosses at most one integer plane, write its signed face lengths
as `ell_i`. In a uniform pre-field,

```text
E_path=E_0-(gq/2) sum_i ell_i^2/d.
```

Inside either piece and across the joining plane,

```text
|d/dd [sum_i ell_i^2/d]| <= 1.
```

The relativistic displacement satisfies

```text
|dd/dp_1| <= |dt| c^2/E_REST.
```

Therefore

```text
Lip(T) <= g^2 dt^2 c^2/(2 E_REST)
       = 0.17380952380952377 < 1.
```

Three separated initial guesses converge to the same hopping root in at most
15 iterations. This is a Banach certificate for the locked uniform-field
axial arm, not a general nonuniform or multi-axis theorem.

## 3. Every physical transaction identity closes

The run-of-record maxima are

```text
fixed-point residual           6.66e-16
total energy residual          2.22e-16
relative Gauss residual        9.71e-17
current continuity residual    3.33e-16
physical-position inverse      0
polarity-shape inverse          1.11e-16
field inverse                   0
momentum inverse                6.66e-16
causal excess                   0
```

Thus the face representation supports a genuine causal, reciprocal physical
hop. The failure below is entirely in the frozen raw state encoding.

## 4. The production threshold map is non-injective

Let `M_d` be the existing one-axis remainder update. For `0<d<1` and
`1-d<r<1`,

```text
M_d(n,r)=(n+1,r+d-1).
```

Applying the same rule backward gives

```text
M_-d M_d(n,r)
=M_-d(n+1,r+d-1)
=(n+1,r-1),
```

because `r-1` lies strictly between `-1` and `0` and therefore does not cross
the `-1` threshold. This is not `(n,r)`.

Stronger, two distinct raw states have the same output:

```text
M_d(n,r)=M_d(n+1,r-1).
```

The test realizes this collision exactly: the inverse anchor differs by one
site and the inverse remainder differs by exactly `1.0`.

## 5. Physical equivalence does not rescue the frozen ontology

The two raw representations have equal effective position,

```text
n+r=(n+1)+(r-1),
```

and identical FTD-0478 trilinear polarity distributions. They would be gauge
copies in a theory whose observables depended only on that distributed shape.
The frozen engine is not such a theory: primitive ternary manifestation,
collision occupancy, event ordering, and reactions are attached to the anchor
site. The raw states are operationally distinguishable.

Consequently no inverse constructed from the final frozen state alone can
recover which preimage occurred. Exact raw-state reversibility requires at
least one of three ontology changes:

1. quotient adjacent `(site,remainder)` copies and rewrite every site-local
   matter operation in terms of the distributed shape;
2. choose a unique canonical subcell coordinate, changing the existing hop
   threshold convention;
3. retain a hop-sheet/history variable that identifies the raw preimage.

None is authorized by the frozen cycle.

## 6. Verdict and plan consequence

FTD-0497 establishes more than FTD-0496 physically: exact axial face work can
cause a legitimate anchor-changing hop. It simultaneously closes the frozen
reciprocal-mobile-matter candidate negative because the required forward/
reverse raw-state gate is mathematically impossible under the existing
threshold encoding.

No `common_action_face_dynamics` toggle, dashboard scenario, multi-axis
campaign, or infrared claim is licensed. Further work must begin with an
explicitly selected representation repair and a new pre-registration; it
cannot be described as continuation of the frozen ontology.

## 7. Reproducibility

- checks: `13/13 PASS`;
- test SHA256:
  `33DB9450D23F35C6D9D76670997E6EA3DA674CC2601E4E5CA3C60447B6EC83A0`;
- header SHA256:
  `A68DD03779B72B77A1C43077F80B9AB5A71946F44396836E904C1803F623792E`;
- implementation SHA256:
  `33392108DB3DFA8CD83864B44A346E85CBDF0F85C75FD4ACC1DE2F3072E49644`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
